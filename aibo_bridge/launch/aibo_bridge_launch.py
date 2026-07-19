"""
aibo_bridge_launch.py

Launches the AIBO bridge node.

Non-secret arguments (robot IP, port, teleop tuning) stay as normal launch
arguments you can override on the command line. The security-critical key
material (chacha_key, robot_ed25519_pubkey, client_ed25519_seed) is NOT passed
here — it is loaded from a YAML params file so real keys never live in source.

  # default params file (config/aibo_keys.yaml, installed with the package)
  ros2 launch aibo_bridge aibo_bridge_launch.py robot_ip:=192.168.1.124

  # point at a specific (uncommitted) secrets file
  ros2 launch aibo_bridge aibo_bridge_launch.py \
        params_file:=/home/alexis/secrets/aibo_keys.yaml \
        robot_ip:=10.0.0.42 robot_port:=7777

Precedence note:
  parameters=[<yaml>, {<launch-arg overrides>}]
  ROS 2 applies the list in order, so the dict overrides the YAML. The dict
  below deliberately contains ONLY the non-secret params, so the key material
  from the YAML is never clobbered.
"""

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description() -> LaunchDescription:

    default_params_file = os.path.join(
        get_package_share_directory("aibo_bridge"),
        "config",
        "aibo_keys.yaml",
    )

    # ----------------------------------------------------------------
    # Overrideable arguments (non-secret only)
    # ----------------------------------------------------------------
    args = [
        DeclareLaunchArgument(
            "params_file",
            default_value=default_params_file,
            description=(
                "YAML file holding the ChaCha20 key and Ed25519 key material. "
                "Keep the real one out of git; the node fails closed if any "
                "key is empty."
            ),
        ),
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
            # 1) Secrets from the YAML file.
            LaunchConfiguration("params_file"),
            # 2) Non-secret launch args (override the YAML if it also sets them).
            {
                "robot_ip":         LaunchConfiguration("robot_ip"),
                "robot_port":       LaunchConfiguration("robot_port"),
                "cmd_vel_deadband": LaunchConfiguration("cmd_vel_deadband"),
                "reconnect_period": LaunchConfiguration("reconnect_period"),
            },
        ],
    )

    return LaunchDescription(args + [bridge_node])
