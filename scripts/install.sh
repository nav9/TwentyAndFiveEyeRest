#!/bin/bash

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

echo "Checking dependencies..."

# Update package list
if [ -f /etc/debian_version ]; then
    sudo apt-get update
fi

# Install CMake
if ! command_exists cmake; then
    echo "Installing CMake..."
    sudo apt-get install -y cmake
else
    echo "CMake is already installed."
fi

# Install G++
if ! command_exists g++; then
    echo "Installing g++..."
    sudo apt-get install -y g++
    sudo apt-get install -y clangd
else
    echo "g++ is already installed."
fi

# Install Boost
# Check for boost headers or libs
if ! dpkg -s libboost-all-dev >/dev/null 2>&1; then
    echo "Installing Boost..."
    sudo apt-get install -y libboost-all-dev
else
    echo "Boost is already installed."
fi

# Install Ninja (optional but good for speed)
if ! command_exists ninja; then
    echo "Installing Ninja..."
    sudo apt-get install -y ninja-build
else
    echo "Ninja is already installed."
fi

echo "Dependency check complete."
