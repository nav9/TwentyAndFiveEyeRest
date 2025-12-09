#!/bin/bash

# iRest Dependency Installer

echo "Detecting Operating System..."

if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$NAME
    VER=$VERSION_ID
else
    echo "Unsupported OS or unable to determine distribution."
    exit 1
fi

echo "Detected: $OS $VER"

install_linux_deps() {
    echo "Installing dependencies for Linux..."
    sudo apt-get update
    sudo apt-get install -y build-essential cmake g++ \
                            qt6-base-dev qt6-tools-dev \
                            libboost-all-dev libgl1-mesa-dev valgrind
    echo "Dependencies installed."
}

if [[ "$OS" == *"Ubuntu"* ]] || [[ "$OS" == *"Debian"* ]] || [[ "$OS" == *"Raspbian"* ]]; then
    install_linux_deps
else
    echo "Automatic installation is currently only supported for Ubuntu/Debian based systems."
    echo "Please ensure you have C++, CMake, Qt6, and Boost installed manually."
fi
