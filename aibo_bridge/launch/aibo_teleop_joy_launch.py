"""
aibo_teleop_joy_launch.py

Brings up PS4 joystick teleop for the AIBO as two cooperating nodes:

  1. joy_node          (package `joy`, off-the-shelf ROS 2 driver)
        Reads the DualShock 4 from the OS and publishes sensor_msgs/Joy
        on /joy. Knows nothing about the AIBO.

  2. aibo_teleop_joy   (this package)
        Subscribes to /joy and maps it onto the AIBO command set,
        publishing to /aibo_bridge/command and /cmd_vel — the same two
        topics the keyboard teleop uses. Knows nothing about gamepads.

The bridge node itself is NOT started here: must still run it with aibo_bridge_launch.py. 
The order does not matter: ROS2 topics decouple the three nodes completely.

Typical use
-----------
  # Terminal 1: the secured bridge to the robot
  ros2 launch aibo_bridge aibo_bridge_launch.py robot_ip:=192.168.1.124

  # Terminal 2: the joystick stack (pair the PS4 pad first, over USB or BT)
  ros2 launch aibo_bridge aibo_teleop_joy_launch.py

Override the pad device or tuning inline, for example:
  ros2 launch aibo_bridge aibo_teleop_joy_launch.py joy_dev:=/dev/input/js1 speed:=0.6

Requires the `joy` package:  sudo apt install ros-$ROS_DISTRO-joy ; jazzy was used to test
"""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description() -> LaunchDescription:

    args = [
        DeclareLaunchArgument(
            # Select the pad by name
            # Get it with:  cat /sys/class/input/js0/device/name
            "joy_device_name",
            default_value="Sony Interactive Entertainment Wireless Controller",
            description="Exact controller name reported by the OS (see cat /sys/class/input/js0/device/name)",
        ),
        DeclareLaunchArgument(
            "joy_deadzone",
            default_value="0.05",
            description="joy_node deadzone (fraction of full scale) below which an axis reads 0",
        ),
        DeclareLaunchArgument(
            "speed",
            default_value="0.5",
            description=(
                "linear.x magnitude at full stick deflection; must exceed the "
                "bridge's cmd_vel_deadband (default 0.05) to register as motion"
            ),
        ),
        DeclareLaunchArgument(
            "deadzone",
            default_value="0.15",
            description="Stick magnitude below which the teleop treats the axis as centred",
        ),
        DeclareLaunchArgument(
            "publish_rate",
            default_value="20.0",
            description="Hz at which the teleop re-publishes /cmd_vel while the stick is held",
        ),
    ]

    # ----------------------------------------------------------------
    # 1) Standard joystick driver -> /joy
    # ----------------------------------------------------------------
    joy_node = Node(
        package="joy",
        executable="joy_node",
        name="joy_node",
        output="screen",
        parameters=[
            {
                # Pin to the controller by name -> stable across reboots/replugs
                "device_name": LaunchConfiguration("joy_device_name"),
                "deadzone": LaunchConfiguration("joy_deadzone"),
                "autorepeat_rate": 20.0,  # keep Joy flowing even when the pad is idle
            }
        ],
    )

    # ----------------------------------------------------------------
    # 2) Joy -> AIBO command set
    # ----------------------------------------------------------------
    teleop_node = Node(
        package="aibo_bridge",
        executable="aibo_teleop_joy",
        name="aibo_teleop_joy",
        output="screen",
        parameters=[
            {
                "speed": LaunchConfiguration("speed"),
                "deadzone": LaunchConfiguration("deadzone"),
                "publish_rate": LaunchConfiguration("publish_rate"),
            }
        ],
    )

    return LaunchDescription(args + [joy_node, teleop_node])
