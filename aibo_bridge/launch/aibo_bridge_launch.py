"""
aibo_bridge.launch.py

Launches the AIBO bridge node with all parameters exposed so you can
override them from the command line without editing source:

  ros2 launch aibo_bridge aibo_bridge.launch.py robot_ip:=192.168.1.124
  ros2 launch aibo_bridge aibo_bridge.launch.py robot_ip:=10.0.0.42 robot_port:=7777
"""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description() -> LaunchDescription:

    # ----------------------------------------------------------------
    # Declare overrideable arguments
    # ----------------------------------------------------------------
    args = [
        DeclareLaunchArgument(
            "robot_ip",
            default_value="192.168.1.124",
            description="IP address of the AIBO ERS-7",
        ),
        DeclareLaunchArgument(
            "robot_port",
            default_value="7777",
            description="TinyConsole TCP port",
        ),
        DeclareLaunchArgument(
            "chacha_key",
            # Default matches ConsoleConfig.h — override in production.
            default_value=(
                "0000000000000000"
                "0000000000000000"
                "0000000000000000"
                "0000000000000000"
            ),
            description="32-byte ChaCha20 key as 64 hex characters",
        ),
        DeclareLaunchArgument(
            "robot_ed25519_pubkey",
            default_value="",
            description=(
                "32-byte Ed25519 public key as 64 hex characters. "
                "The node fails if this is left empty."
            ),
        ),
        DeclareLaunchArgument(
            "cmd_vel_deadband",
            default_value="0.05",
            description=(
                "Minimum |linear.x| (m/s) before a FORWARD/BACK command "
                "is issued; values below this threshold map to STOP"
            ),
        ),
        DeclareLaunchArgument(
            "reconnect_period",
            default_value="5.0",
            description="Seconds between reconnection attempts when the link is down",
        ),
    ]

    # ----------------------------------------------------------------
    # Bridge node
    # ----------------------------------------------------------------
    bridge_node = Node(
        package="aibo_bridge",
        executable="aibo_bridge_node",
        name="aibo_bridge",
        output="screen",
        parameters=[
            {
                "robot_ip":        LaunchConfiguration("robot_ip"),
                "robot_port":      LaunchConfiguration("robot_port"),
                "chacha_key":      LaunchConfiguration("chacha_key"),
                "robot_ed25519_pubkey": LaunchConfiguration("robot_ed25519_pubkey"),
                "cmd_vel_deadband": LaunchConfiguration("cmd_vel_deadband"),
                "reconnect_period": LaunchConfiguration("reconnect_period"),
            }
        ],
    )

    return LaunchDescription(args + [bridge_node])
