"""
aibo_bridge_node.py
ROS 2 bridge node for the Sony AIBO ERS-7 / TinyConsole.

=== Topics ===

Subscribed
  ~/command   std_msgs/String
      Direct command pass-through.  Any string the AIBO understands:
        GET_UP  REST  FORWARD  BACK  STOP  PING  QUIT
      The node publishes the AIBO's response on ~/response.

  /cmd_vel    geometry_msgs/Twist
      Teleop-compatible velocity stream (from teleop_twist_keyboard,
      a joystick driver, Nav2, etc.).  Only linear.x is used:
        linear.x > +deadband  →  FORWARD
        linear.x < -deadband  →  BACK
        |linear.x| ≤ deadband →  STOP  (only sent on direction change)
      The deadband avoids re-sending the same command on every tick.

Published
  ~/response   std_msgs/String   Plaintext response from the AIBO
  ~/connected  std_msgs/Bool     True while the TCP link is up

=== Parameters ===

  robot_ip          (string)   AIBO IP address           [192.168.1.124]
  robot_port        (int)      TinyConsole TCP port       [7777]
  chacha_key        (string)   32-byte key as 64 hex chars
                               (default matches ConsoleConfig.h)
  cmd_vel_deadband  (double)   |linear.x| threshold m/s  [0.05]
  reconnect_period  (double)   seconds between reconnect attempts [5.0]

=== Notes ===

TinyConsole is half-duplex: the client always sends first, and every
send must be followed by exactly one receive before the next send.
Callbacks therefore serialize through a threading.Lock; only one
AEAD exchange is in-flight at any time.

Motion commands sent via /cmd_vel are debounced: the bridge only
forwards a command when the desired state *changes*, so holding a
joystick forward does not flood the AIBO with repeated FORWARD frames.

The node uses ROS 2's default SingleThreadedExecutor.  The lock is
kept for correctness if someone switches to MultiThreadedExecutor.
"""

import threading
from typing import Optional

import rclpy
from rclpy.node import Node
from std_msgs.msg import Bool, String
from geometry_msgs.msg import Twist

from aibo_bridge.aibo_link import AiboLink

# Default key — matches ConsoleConfig.h / chacha20_console_client.py.
# Override with the 'chacha_key' ROS 2 parameter; never commit a real
# production key to source control.
_DEFAULT_KEY_HEX = (
    "0000000000000000"
    "0000000000000000"
    "0000000000000000"
    "0000000000000000"
)

# Commands that TinyConsole accepts (upper-case, used for validation).
_VALID_COMMANDS = {"GET_UP", "REST", "FORWARD", "BACK", "STOP", "PING", "QUIT"}

# Motion commands that affect the AIBO's movement state (used for debouncing).
_MOTION_COMMANDS = {"FORWARD", "BACK", "STOP"}


class AiboBridgeNode(Node):

    def __init__(self) -> None:
        super().__init__("aibo_bridge")

        # ----------------------------------------------------------
        # Parameters
        # ----------------------------------------------------------
        self.declare_parameter("robot_ip",        "192.168.1.124")
        self.declare_parameter("robot_port",       7777)
        self.declare_parameter("chacha_key",       _DEFAULT_KEY_HEX)
        self.declare_parameter("cmd_vel_deadband", 0.05)
        self.declare_parameter("reconnect_period", 5.0)

        robot_ip  = self.get_parameter("robot_ip").value
        robot_port = self.get_parameter("robot_port").value
        key_hex   = self.get_parameter("chacha_key").value
        self._deadband = float(self.get_parameter("cmd_vel_deadband").value)
        reconnect_period = float(self.get_parameter("reconnect_period").value)

        # Parse and validate the key
        try:
            key = bytes.fromhex(key_hex)
        except ValueError as exc:
            self.get_logger().fatal(
                f"chacha_key is not valid hex: {exc}"
            )
            raise
        if len(key) != 32:
            msg = f"chacha_key must decode to 32 bytes, got {len(key)}"
            self.get_logger().fatal(msg)
            raise ValueError(msg)

        # ----------------------------------------------------------
        # Link + synchronisation
        # ----------------------------------------------------------
        self._link = AiboLink(robot_ip, robot_port, key)
        self._lock = threading.Lock()          # serialises AEAD exchanges
        self._last_motion_cmd: Optional[str] = None  # debounce state

        # ----------------------------------------------------------
        # Publishers
        # ----------------------------------------------------------
        self._pub_response  = self.create_publisher(String, "~/response",  10)
        self._pub_connected = self.create_publisher(Bool,   "~/connected", 10)

        # ----------------------------------------------------------
        # Subscribers
        # ----------------------------------------------------------
        self.create_subscription(
            String, "~/command", self._command_cb, 10
        )
        self.create_subscription(
            Twist, "/cmd_vel", self._cmd_vel_cb, 10
        )

        # ----------------------------------------------------------
        # Reconnection timer
        # ----------------------------------------------------------
        self._reconnect_timer = self.create_timer(
            reconnect_period, self._reconnect_cb
        )

        # ----------------------------------------------------------
        # Initial connection attempt
        # ----------------------------------------------------------
        self._try_connect()

    # ==============================================================
    # Connection management
    # ==============================================================

    def _try_connect(self) -> bool:
        """
        Attempt one connection + handshake.  Not holding _lock —
        no command callbacks can succeed while disconnected anyway.
        Returns True on success.
        """
        if self._link.is_connected:
            return True

        robot_ip   = self.get_parameter("robot_ip").value
        robot_port = self.get_parameter("robot_port").value
        self.get_logger().info(
            f"Connecting to AIBO at {robot_ip}:{robot_port} ..."
        )
        try:
            banner = self._link.connect()
            self.get_logger().info(f"Connected — banner: '{banner}'")
            self._publish_connected(True)
            self._last_motion_cmd = None  # reset debounce after reconnect
            return True
        except Exception as exc:
            self.get_logger().warn(f"Connection failed: {exc}")
            self._publish_connected(False)
            return False

    def _reconnect_cb(self) -> None:
        """Timer callback: reconnect if the link is down."""
        if self._link.is_connected:
            self._publish_connected(True)   # keep status fresh
        else:
            self._try_connect()

    def _publish_connected(self, state: bool) -> None:
        msg = Bool()
        msg.data = state
        self._pub_connected.publish(msg)

    # ==============================================================
    # Core send helper
    # ==============================================================

    def _send(self, cmd: str) -> Optional[str]:
        """
        Thread-safe: acquire lock, send cmd, return the AIBO's response
        string, or None if the link is down or an error occurred.
        On any link error the socket is force-closed so the reconnect
        timer can re-establish the connection on its next tick.
        """
        with self._lock:
            if not self._link.is_connected:
                self.get_logger().warn(
                    f"Cannot send '{cmd}': not connected to AIBO"
                )
                return None
            try:
                response = self._link.send_command(cmd)
            except Exception as exc:
                self.get_logger().error(
                    f"Link error while sending '{cmd}': {exc}"
                )
                self._link.close()           # reset state; timer will reconnect
                self._publish_connected(False)
                self._last_motion_cmd = None
                return None

        # Publish and log outside the lock
        msg = String()
        msg.data = response
        self._pub_response.publish(msg)
        self.get_logger().info(f"AIBO ← '{cmd}'  →  '{response}'")
        return response

    # ==============================================================
    # ~/command subscriber  (direct pass-through)
    # ==============================================================

    def _command_cb(self, msg: String) -> None:
        cmd = msg.data.strip().upper()
        if not cmd:
            return

        if cmd not in _VALID_COMMANDS:
            self.get_logger().warn(
                f"Unknown command '{cmd}'. "
                f"Valid commands: {', '.join(sorted(_VALID_COMMANDS))}"
            )
            return

        response = self._send(cmd)

        # If QUIT was acknowledged, close our side of the socket.
        # TinyConsole closes the connection immediately after sending "BYE".
        if cmd == "QUIT" and response is not None:
            with self._lock:
                self._link.close()
            self._publish_connected(False)
            self._last_motion_cmd = None
            self.get_logger().info("Sent QUIT — connection closed by AIBO")

    # ==============================================================
    # /cmd_vel subscriber  (teleop / Nav2 compatible)
    # ==============================================================

    def _cmd_vel_cb(self, msg: Twist) -> None:
        """
        Translate a Twist message into a discrete AIBO motion command.

        Only forward/backward motion is supported (TinyConsole has no
        turn command).  Commands are debounced: we only send when the
        desired state changes, so a held joystick does not spam frames.
        """
        lx = msg.linear.x

        if lx > self._deadband:
            desired = "FORWARD"
        elif lx < -self._deadband:
            desired = "BACK"
        else:
            desired = "STOP"

        if desired == self._last_motion_cmd:
            return  # no change — nothing to do

        self._last_motion_cmd = desired
        self._send(desired)

    # ==============================================================
    # Cleanup
    # ==============================================================

    def destroy_node(self) -> None:
        self.get_logger().info("Shutting down AiboBridgeNode ...")
        with self._lock:
            if self._link.is_connected:
                try:
                    # Park the AIBO safely before we go
                    self._link.send_command("REST")
                except Exception:
                    pass
                self._link.disconnect()
        super().destroy_node()


# ================================================================
# Entry point
# ================================================================

def main(args=None) -> None:
    rclpy.init(args=args)
    node = AiboBridgeNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
