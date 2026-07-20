"""
aibo_teleop_key.py

Keyboard teleop for the AIBO bridge, in the spirit of
teleop_twist_keyboard but mapping keys to the AIBO's command set.

Key map
-------
    Up arrow     GET_UP   (posture, one-shot)   -> <command_topic> (String)
    Down arrow   REST     (posture, one-shot)   -> <command_topic> (String)
    Left arrow   FORWARD  (hold to move)        -> <cmd_vel_topic> (Twist)
    Right arrow  BACK     (hold to move)        -> <cmd_vel_topic> (Twist)
    Enter        STOP     (explicit)            -> <cmd_vel_topic> (Twist, x=0)
    q / Ctrl-C   quit teleop

Hold-to-move / stop-on-release
------------------------------
A terminal only reports key presses, not releases -> use the OS keyboard auto-repeat: 
While an arrow is held down the motion repeats, and each repeat re-publishes the motion Twist. 
When the key is released, it stops.
If no key press arrives within `key_timeout` seconds, publish a zero Twist to stop the robot.
This is done to avoid the robot continuing to move in case of a crash.

Trade-off: `key_timeout` must be larger than system's __init__ial
auto-repeat delay (0.25-0.5 s) or there will be a brief stop/start
stutter right after pressing. 

Motion goes through /cmd_vel (not the command topic) so the bridge's
existing deadband + debounce handles it and a single STOP is sent on release.
"""

import os
import sys
import select
import threading

import rclpy
from rclpy.node import Node
from std_msgs.msg import String
from geometry_msgs.msg import Twist

import termios
import tty


BANNER = """\
AIBO keyboard teleop
--------------------
  Up      : GET_UP        Left  : FORWARD (hold)
  Down    : REST          Right : BACK    (hold)
  Enter   : STOP          q     : quit

Motion is hold-to-move: release the arrow and the robot stops.
"""

# Final byte of an arrow escape sequence -> logical key.
_ARROWS = {0x41: "UP", 0x42: "DOWN", 0x43: "RIGHT", 0x44: "LEFT"}


class AiboTeleopKey(Node):

    def __init__(self) -> None:
        super().__init__("aibo_teleop_key")

        # Topics (parameters that can match a remapped bridge)
        self.declare_parameter("command_topic", "/aibo_bridge/command")
        self.declare_parameter("cmd_vel_topic", "/cmd_vel")
        # Linear speed magnitude for FORWARD/BACK which must exceed the bridge's
        # cmd_vel_deadband (default 0.05) to register as motion
        self.declare_parameter("speed", 0.5)
        # Seconds without a key before publish STOP
        self.declare_parameter("key_timeout", 0.6)

        command_topic = self.get_parameter("command_topic").value
        cmd_vel_topic = self.get_parameter("cmd_vel_topic").value
        self.speed = float(self.get_parameter("speed").value)
        self.key_timeout = float(self.get_parameter("key_timeout").value)

        self.pub_command = self.create_publisher(String, command_topic, 10)
        self.pub_cmd_vel = self.create_publisher(Twist, cmd_vel_topic, 10)

        self.moving = False        # True while a FORWARD/BACK Twist is active
        self._last_label = None    # last motion label logged (debounce logging)

    # ------------------------------------------------------------------
    # Publishing functions
    # ------------------------------------------------------------------

    def send_command(self, cmd: str) -> None:
        msg = String()
        msg.data = cmd
        self.pub_command.publish(msg)
        self.get_logger().info(f"command -> {cmd}")

    def send_motion(self, linear_x: float) -> None:
        twist = Twist()
        twist.linear.x = linear_x
        self.pub_cmd_vel.publish(twist)
        self.moving = linear_x != 0.0

        if linear_x > 0:
            label = "FORWARD"
        elif linear_x < 0:
            label = "BACK"
        else:
            label = "STOP"
        # Log only on change so auto-repeat doesn't spam the console.
        if label != self._last_label:
            self.get_logger().info(f"cmd_vel -> {label}")
            self._last_label = label

    def stop(self) -> None:
        """Publish a zero Twist once"""
        self.send_motion(0.0)

    # ------------------------------------------------------------------
    # Key loop (runs in the main thread)
    # ------------------------------------------------------------------

    def run(self) -> None:
        print(BANNER)
        fd = sys.stdin.fileno()
        settings = termios.tcgetattr(fd)
        try:
            tty.setraw(fd)
            while rclpy.ok():
                key = self._read_key(fd, self.key_timeout)

                if key is None:
                    # Timeout: no key within key_timeout -> stop if we were moving.
                    if self.moving:
                        self.stop()
                    continue

                if key == "UP":
                    self.send_command("GET_UP")
                elif key == "DOWN":
                    self.send_command("REST")
                elif key == "LEFT":
                    self.send_motion(+self.speed)   # FORWARD
                elif key == "RIGHT":
                    self.send_motion(-self.speed)   # BACK
                elif key == "ENTER":
                    self.stop()
                elif key in ("q", "Q", "CTRL_C"):
                    break
                # any other key: ignore
        finally:
            # Stop the robot before restoring the terminal.
            try:
                self.stop()
            except Exception:
                pass
            termios.tcsetattr(fd, termios.TCSADRAIN, settings)

    @staticmethod
    def _read_key(fd: int, timeout: float):
        """
        Return one logical key: 'UP'/'DOWN'/'LEFT'/'RIGHT'/'ENTER'/'CTRL_C',
        a single character, or None if `timeout` elapsed with no input.

        Reads directly from the fd with os.read so multi-byte arrow escape
        sequences (ESC [ A or ESC O A) arrive intact in one call.
        """
        rlist, _, _ = select.select([fd], [], [], timeout)
        if not rlist:
            return None

        data = os.read(fd, 8)   # whole escape sequence comes atomically
        if not data:
            return None

        if data == b"\x03":
            return "CTRL_C"
        if data in (b"\r", b"\n"):
            return "ENTER"
        if data[:1] == b"\x1b":
            # Arrow: ESC [ X  or  ESC O X  (application-cursor mode).
            if len(data) >= 3 and data[1] in (0x5B, 0x4F):
                return _ARROWS.get(data[2], "ESC")
            return "ESC"
        return data.decode("utf-8", "ignore")


def main(args=None) -> None:
    rclpy.init(args=args)
    node = AiboTeleopKey()

    # Spin in the background so parameters/logging behave normally while the
    # key loop owns the main thread.
    spinner = threading.Thread(target=rclpy.spin, args=(node,), daemon=True)
    spinner.start()

    try:
        node.run()
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()

main()