import os
from glob import glob
from setuptools import find_packages, setup

package_name = "aibo_bridge"

setup(
    name=package_name,
    version="1.0",
    packages=find_packages(exclude=["test"]),
    data_files=[
        # Required by ament to find the package
        (
            "share/ament_index/resource_index/packages",
            [f"resource/{package_name}"],
        ),
        # Install package.xml
        (f"share/{package_name}", ["package.xml"]),
        (os.path.join("share", package_name, "config"), glob("config/*.yaml")),
        # Install launch files
        (
            os.path.join("share", package_name, "launch"),
            glob("launch/*_launch.py"),
        ),
    ],
    install_requires=["setuptools"],
    zip_safe=True,
    maintainer="Alexis Regardin",
    maintainer_email="alexis.regardin@ulb.be",
    description="ROS 2 bridge for the Sony AIBO ERS-7 TinyConsole",
    license="MIT",
    tests_require=["pytest"],
    entry_points={
        "console_scripts": [
            # ros2 run aibo_bridge aibo_bridge_node
            "aibo_bridge_node = aibo_bridge.aibo_bridge_node:main",
        ],
    },
)
