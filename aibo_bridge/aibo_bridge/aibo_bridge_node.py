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
  robot_ed25519_pubkey (string) 32-byte Ed25519 public key as 64 hex chars. Required: the node won't start without it.
  client_ed25519_seed (string) 32-byte Ed25519 private seed as 64 hex chars.
                               The client's own identity: it signs the handshake
                               transcript so the robot can verify us against its
                               pinned CLIENT_ED25519_PK. Required: the node won't
                               start without it (must never be committed).
  cmd_vel_deadband  (double)   |linear.x| threshold m/s  [0.05]
  reconnect_period  (double)   seconds between reconnect attempts [5.0]

=== Network isolation (self-contained security) ===

With ROS 2's default DDS discovery, any host sharing the ROS domain on that same Wi-Fi
segment could publish to /cmd_vel or ~/command and have the bridge encrypt and forward it.

To keep it self-contained (no actions are required before launching it), the node
limits DDS discovery to the local host only, before anything starts

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

import os
import sys
import threading
from typing import Optional

import rclpy
from rclpy.node import Node
from std_msgs.msg import Bool, String
from geometry_msgs.msg import Twist

from aibo_bridge.aibo_link import AiboLink

# Commands that TinyConsole accepts (upper-case, used for validation).
VALID_COMMANDS = {"GET_UP", "REST", "FORWARD", "BACK", "STOP", "HELP", "INFO", "PING", "QUIT"}

# --------------------------------------------------------------------------
# DDS discovery lock-down.
# --------------------------------------------------------------------------
_ENFORCED_DISCOVERY_ENV = {
    "ROS_AUTOMATIC_DISCOVERY_RANGE": "LOCALHOST",
}


def enforce_localhost_discovery() -> None:
    """
    Constrain DDS discovery to the local host *before* the starts

    Self-contained:
      - no need to `export ROS_LOCALHOST_ONLY=1` manually
      - applies identically whether the node is started with `ros2 run`
        or `ros2 launch` (both call main())
      - if tries to widen discovery, override it and logs it

    Must run before rclpy.init()!! calling it afterwards has no effect
    """
    for key, value in _ENFORCED_DISCOVERY_ENV.items():
        current = os.environ.get(key)
        if current not in (None, "", value):
            print(
                f"[aibo_bridge] Overriding {key}={current!r} -> {value!r} "
                f"to keep AIBO discovery local-only",
                file=sys.stderr,
            )
        os.environ[key] = value


class AiboBridgeNode(Node):

    def __init__(self) -> None:
        super().__init__("aibo_bridge")

        # ----------------------------------------------------------
        # Parameters
        # ----------------------------------------------------------
        self.declare_parameter("robot_ip", "192.168.1.124")
        self.declare_parameter("robot_port", 7777)
        self.declare_parameter("chacha_key", "")
        self.declare_parameter("robot_ed25519_pubkey", "")
        self.declare_parameter("client_ed25519_seed", "")
        self.declare_parameter("cmd_vel_deadband", 0.05)
        self.declare_parameter("reconnect_period", 5.0)

        robot_ip  = self.get_parameter("robot_ip").value
        robot_port = self.get_parameter("robot_port").value
        key_hex   = self.get_parameter("chacha_key").value
        pubkey_hex = self.get_parameter("robot_ed25519_pubkey").value
        client_seed_hex = self.get_parameter("client_ed25519_seed").value
        self.deadband = float(self.get_parameter("cmd_vel_deadband").value)
        reconnect_period = float(self.get_parameter("reconnect_period").value)

        # Confirm, for the operator and the record, that discovery is local-only.
        self.get_logger().info(
            "DDS discovery constrained to local host "
            f"(ROS_AUTOMATIC_DISCOVERY_RANGE="
            f"{os.environ.get('ROS_AUTOMATIC_DISCOVERY_RANGE')}, "
            f"ROS_LOCALHOST_ONLY={os.environ.get('ROS_LOCALHOST_ONLY')})"
        )

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

        # Parse and validate the robot's pinned Ed25519 identity key.
        # Deliberately no usable default here: this is an authentication security
        # control, not a convenience.  The node will refuse to start without it.
        if not pubkey_hex:
            msg = (
                "robot_ed25519_pubkey is not set. Run the generator "
                "once (offline, on this machine) to "
                "generate the robot's identity key pair, flash "
                "ROBOT_ED25519_SK into ConsoleConfig.h, and pass the "
                "printed public-key hex here via this launch parameter."
            )
            self.get_logger().fatal(msg)
            raise ValueError(msg)
        try:
            robot_pubkey = bytes.fromhex(pubkey_hex)
        except ValueError as exc:
            self.get_logger().fatal(
                f"robot_ed25519_pubkey is not valid hex: {exc}"
            )
            raise
        if len(robot_pubkey) != 32:
            msg = f"robot_ed25519_pubkey must decode to 32 bytes, got {len(robot_pubkey)}"
            self.get_logger().fatal(msg)
            raise ValueError(msg)

        # Parse and validate the client's own Ed25519 secret seed.
        # Private half of the client's identity, used to sign the handshake so the robot can
        # authenticate against its pinned CLIENT_ED25519_PK.
        if not client_seed_hex:
            msg = (
                "client_ed25519_seed is not set. Run the generator with "
                "--role client once to generate the client's identity key pair,"
                "flash CLIENT_ED25519_PK into ConsoleConfig.h, "
                "and pass the printed secret seed hex here via "
                "this launch parameter. Never commit it."
            )
            self.get_logger().fatal(msg)
            raise ValueError(msg)
        try:
            client_seed = bytes.fromhex(client_seed_hex)
        except ValueError as exc:
            self.get_logger().fatal(
                f"client_ed25519_seed is not valid hex: {exc}"
            )
            raise
        if len(client_seed) != 32:
            msg = f"client_ed25519_seed must decode to 32 bytes, got {len(client_seed)}"
            self.get_logger().fatal(msg)
            raise ValueError(msg)

        # ----------------------------------------------------------
        # Link + synchronisation
        # ----------------------------------------------------------
        self.link = AiboLink(robot_ip, robot_port, key, robot_pubkey, client_seed)
        self.lock = threading.Lock()          # serialises AEAD exchanges
        self.last_motion_cmd: Optional[str] = None  # debounce state

        # ----------------------------------------------------------
        # Publishers
        # ----------------------------------------------------------
        self.pub_response  = self.create_publisher(String, "~/response",  10)
        self.pub_connected = self.create_publisher(Bool,   "~/connected", 10)

        # ----------------------------------------------------------
        # Subscribers
        # ----------------------------------------------------------
        self.create_subscription(
            String, "~/command", self.command_cb, 10
        )
        self.create_subscription(
            Twist, "/cmd_vel", self.cmd_vel_cb, 10
        )

        self.declare_parameter("motion_timeout", 1.0) # seconds without a /cmd_vel before sending STOP
        self.motion_timeout = float(self.get_parameter("motion_timeout").value)
        self.last_cmd_vel_time = self.get_clock().now()
        self.create_timer(0.2, self.motion_timeout_cb)  # check for timeout every 100ms

        # ----------------------------------------------------------
        # Reconnection timer
        # ----------------------------------------------------------
        self.reconnect_timer = self.create_timer(
            reconnect_period, self.reconnect_cb
        )

        # ----------------------------------------------------------
        # __init__ial connection attempt
        # ----------------------------------------------------------
        self.try_connect()

    # ==============================================================
    # Connection management
    # ==============================================================

    def try_connect(self) -> bool:
        """
        Attempt one connection + handshake.  Not holding _lock —
        no command callbacks can succeed while disconnected anyway.
        Returns True on success.
        """
        if self.link.is_connected:
            return True

        robot_ip   = self.get_parameter("robot_ip").value
        robot_port = self.get_parameter("robot_port").value
        self.get_logger().info(
            f"Connecting to AIBO at {robot_ip}:{robot_port} ..."
        )
        try:
            banner = self.link.connect()
            self.get_logger().info(f"Connected — banner: '{banner}'")
            self.publish_connected(True)
            self.last_motion_cmd = None  # reset debounce after reconnect
            return True
        except Exception as exc:
            self.get_logger().warn(f"Connection failed: {exc}")
            self.publish_connected(False)
            return False

    def reconnect_cb(self) -> None:
        """Timer callback: reconnect if the link is down."""
        if self.link.is_connected:
            self.publish_connected(True)   # keep status fresh
        else:
            self.try_connect()

    def publish_connected(self, state: bool) -> None:
        msg = Bool()
        msg.data = state
        self.pub_connected.publish(msg)

    # ==============================================================
    # Core send helper
    # ==============================================================

    def send(self, cmd: str) -> Optional[str]:
        """
        Thread-safe: acquire lock, send cmd, return the AIBO's response
        string, or None if the link is down or an error occurred.
        On any link error the socket is force-closed so the reconnect
        timer can re-establish the connection on its next tick.
        """
        with self.lock:
            if not self.link.is_connected:
                self.get_logger().warn(
                    f"Cannot send '{cmd}': not connected to AIBO"
                )
                return None
            try:
                response = self.link.send_command(cmd)
            except Exception as exc:
                self.get_logger().error(
                    f"Link error while sending '{cmd}': {exc}"
                )
                self.link.close()           # reset state; timer will reconnect
                self.publish_connected(False)
                self.last_motion_cmd = None
                return None

        # Publish and log outside the lock
        msg = String()
        msg.data = response
        self.pub_response.publish(msg)
        self.get_logger().info(f"AIBO ← '{cmd}'  →  '{response}'")
        return response

    # ==============================================================
    # ~/command subscriber  (direct pass-through)
    # ==============================================================

    def command_cb(self, msg: String) -> None:
        cmd = msg.data.strip().upper()
        if not cmd:
            return

        if cmd not in VALID_COMMANDS:
            self.get_logger().warn(
                f"Unknown command '{cmd}'. "
                f"Valid commands: {', '.join(sorted(VALID_COMMANDS))}"
            )
            return

        response = self.send(cmd)

        # If QUIT was acknowledged, close our side of the socket.
        # TinyConsole closes the connection immediately after sending "BYE".
        if cmd == "QUIT" and response is not None:
            with self.lock:
                self.link.close()
            self.publish_connected(False)
            self.last_motion_cmd = None
            self.get_logger().info("Sent QUIT — connection closed by AIBO")

    # ==============================================================
    # /cmd_vel subscriber  (teleop / Nav2 compatible)
    # ==============================================================

    def cmd_vel_cb(self, msg: Twist) -> None:
        """
        Translate a Twist message into a discrete AIBO motion command.

        Only forward/backward motion is supported (TinyConsole has no
        turn command).  Commands are debounced: we only send when the
        desired state changes, so a held joystick does not spam frames.
        """
        self.last_cmd_vel_time = self.get_clock().now()

        lx = msg.linear.x

        if lx > self.deadband:
            desired = "FORWARD"
        elif lx < -self.deadband:
            desired = "BACK"
        else:
            desired = "STOP"

        if desired == self.last_motion_cmd:
            return  # no change — nothing to do

        self.last_motion_cmd = desired
        self.send(desired)

    # ==============================================================
    # Cleanup
    # ==============================================================

    def destroy_node(self) -> None:
        self.get_logger().info("Shutting down AiboBridgeNode ...")
        with self.lock:
            if self.link.is_connected:
                try:
                    # Park the AIBO safely before we go
                    self.link.send_command("REST")
                except Exception:
                    pass
                self.link.disconnect()
        super().destroy_node()

    def motion_timeout_cb(self) -> None:
        if self.last_motion_cmd in ("FORWARD", "BACK"):
            elapsed = (self.get_clock().now() - self.last_cmd_vel_time).nanoseconds / 1e9
            if elapsed > self.motion_timeout:
                self.get_logger().warn("cmd_vel timeout — sending STOP (deadman)")
                self.last_motion_cmd = "STOP"
                self.send("STOP")



# ================================================================
# Entry point
# ================================================================

def main(args=None) -> None:
    # Lock DDS discovery to the local host BEFORE the middleware starts.
    # Must precede rclpy.init(); afterwards it has no effect.
    enforce_localhost_discovery()

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