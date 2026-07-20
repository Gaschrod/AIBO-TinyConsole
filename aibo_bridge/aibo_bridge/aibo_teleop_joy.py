"""
aibo_teleop_joy.py

PS4 (DualShock 4) joystick teleop for the AIBO bridge, counterpart to aibo_teleop_key.py.

Consumes the standard ROS2 `joy` driver's sensor_msgs/Joy stream and
maps it onto the AIBO's command set -> same two topics as the aibo_teleop_key thus the bridge is untouched:

    /aibo_bridge/command  (std_msgs/String)   one-shot postures
    /cmd_vel              (geometry_msgs/Twist) hold-to-move motion

Control map (PS4 / DualShock 4)
-------------------------------
    Left stick, vertical   push forward  ->  FORWARD   (hold to move)
                           pull back     ->  BACK      (hold to move)
                           released      ->  STOP
    Cross  (X)             GET_UP   (posture, one-shot on press)
    Circle (O)             REST     (posture, one-shot on press)
    Square                 STOP     (explicit zero Twist)

Hold-to-move / deadman
----------------------
No auto-repeat trick as an absence of the stick position is reported.
We keep the latest stick value and re-publish the motion Twist on a fixed timer (`publish_rate`). 


Button / axis indices
---------------------
Indices below = defaults for standard Linux joystick driver +
`joy_node` with a DualShock 4. 
All parameters: if the driver enumerates them differently, run:

    ros2 topic echo /joy

Then, press each button, read off the index, and override the parameter. 
"""

import rclpy
from rclpy.node import Node
from std_msgs.msg import String
from geometry_msgs.msg import Twist
from sensor_msgs.msg import Joy


BANNER = """\
AIBO PS4 joystick teleop
------------------------
  Left stick up   : FORWARD (hold)     Cross  (X) : GET_UP
  Left stick down : BACK    (hold)     Circle (O) : REST
  Release stick   : STOP                Square    : STOP

Motion is hold-to-move: recentre the stick and the robot stops.
"""


class AiboTeleopJoy(Node):

    def __init__(self) -> None:
        super().__init__("aibo_teleop_joy")

        # --- Topics (parameters so they can match a remapped bridge) ----
        self.declare_parameter("command_topic", "/aibo_bridge/command")
        self.declare_parameter("cmd_vel_topic", "/cmd_vel")
        self.declare_parameter("joy_topic", "/joy")

        # --- Motion tuning ----------------------------------------------
        # linear.x magnitude at full stick deflection. Must exceed the
        # bridge's cmd_vel_deadband (default 0.05) to register as motion.
        self.declare_parameter("speed", 0.5)
        # Absorbs wanalogue-stick drift so a resting stick yields a clean STOP.
        self.declare_parameter("deadzone", 0.15)
        # Hz at which the current motion Twist is (re)published, which keeps the
        # bridge's cmd_vel fed while the stick is held.
        self.declare_parameter("publish_rate", 20.0)

        # --- Axis / button map (DS4 via standard joy_node defaults) ------
        self.declare_parameter("axis_linear", 1)      # left stick, vertical
        self.declare_parameter("invert_linear", False)  # push-forward -> FORWARD
        self.declare_parameter("button_get_up", 0)    # Cross  (X)
        self.declare_parameter("button_rest", 1)      # Circle (O)
        self.declare_parameter("button_stop", 3)      # Square

        command_topic = self.get_parameter("command_topic").value
        cmd_vel_topic = self.get_parameter("cmd_vel_topic").value
        joy_topic = self.get_parameter("joy_topic").value

        self.speed = float(self.get_parameter("speed").value)
        self.deadzone = float(self.get_parameter("deadzone").value)
        publish_rate = float(self.get_parameter("publish_rate").value)

        self.axis_linear = int(self.get_parameter("axis_linear").value)
        self.invert_linear = bool(self.get_parameter("invert_linear").value)
        self.button_get_up = int(self.get_parameter("button_get_up").value)
        self.button_rest = int(self.get_parameter("button_rest").value)
        self.button_stop = int(self.get_parameter("button_stop").value)

        self.pub_command = self.create_publisher(String, command_topic, 10)
        self.pub_cmd_vel = self.create_publisher(Twist, cmd_vel_topic, 10)
        self.create_subscription(Joy, joy_topic, self.joy_cb, 10)

        # State
        self.target_linear = 0.0     # latest commanded linear.x
        self.moving = False          # True while a non-zero Twist is active
        self._last_label = None      # last motion label logged (debounce logging)
        self._prev_buttons = []      # previous Joy.buttons for rising-edge detect

        period = 1.0 / publish_rate if publish_rate > 0.0 else 0.05
        self.create_timer(period, self.publish_cb)

        self.get_logger().info(BANNER)

    # ------------------------------------------------------------------
    # Publishing functions  (mirror aibo_teleop_key.py)
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
        # Log only on change so the timer's re-publishing doesn't spam.
        if label != self._last_label:
            self.get_logger().info(f"cmd_vel -> {label}")
            self._last_label = label

    def stop(self) -> None:
        """Force the motion target to zero and publish a stop immediately."""
        self.target_linear = 0.0
        self.send_motion(0.0)

    # ------------------------------------------------------------------
    # Joy subscriber
    # ------------------------------------------------------------------

    def joy_cb(self, msg: Joy) -> None:
        # --- Left-stick vertical -> linear.x --------------------------
        raw = 0.0
        if 0 <= self.axis_linear < len(msg.axes):
            raw = msg.axes[self.axis_linear]

        if abs(raw) < self.deadzone:
            raw = 0.0
        if self.invert_linear:
            raw = -raw
        self.target_linear = raw * self.speed

        # --- Buttons: act on the press edge only ----------------------
        if self.pressed(msg, self.button_stop):
            self.stop()
        if self.pressed(msg, self.button_get_up):
            self.send_command("GET_UP")
        if self.pressed(msg, self.button_rest):
            self.send_command("REST")

        self._prev_buttons = list(msg.buttons)

    def pressed(self, msg: Joy, index: int) -> bool:
        """True on the rising edge (0 -> 1) of the button at `index`."""
        if not (0 <= index < len(msg.buttons)):
            return False
        now = msg.buttons[index]
        was = self._prev_buttons[index] if index < len(self._prev_buttons) else 0
        return now == 1 and was == 0

    # ------------------------------------------------------------------
    # Timer: keep the bridge's cmd_vel deadman fed while the stick is held
    # ------------------------------------------------------------------

    def publish_cb(self) -> None:
        self.send_motion(self.target_linear)


def main(args=None) -> None:
    rclpy.init(args=args)
    node = AiboTeleopJoy()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        # Send STOP so the robot does not keep moving when the node is killed.
        try:
            node.stop()
        except Exception:
            pass
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


main()