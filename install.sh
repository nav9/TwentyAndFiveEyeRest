#!/usr/bin/env bash
set -e

echo "===== Installing TwentyAndFiveEyeRest build dependencies ====="

OS="$(lsb_release -is 2>/dev/null || uname -s)"
VER="$(lsb_release -rs 2>/dev/null || echo "")"

echo "Detected OS: $OS $VER"

if [[ "$OS" == "Ubuntu" || "$OS" == "Linuxmint" ]]; then
    echo "Updating package lists..."
    sudo add-apt-repository -y universe || true
    sudo apt update

    # Core build tools
    sudo apt install -y \
        build-essential \
        gcc g++ \
        cmake \
        ninja-build \
        pkg-config \
        git

    # Boost full set
    sudo apt install -y libboost-all-dev

    # Valgrind
    sudo apt install -y valgrind

    echo "Installing Qt6 for Ubuntu 22.04 (Jammy)..."

    sudo apt install -y \
        qt6-base-dev \
        qt6-base-dev-tools \
        qt6-tools-dev \
        qt6-tools-dev-tools \
        qt6-l10n-tools \
        qt6-wayland

    # Qt SVG module sometimes missing on Mint; try/catch
    echo "Ubuntu 22.04 uses built-in Qt6 SVG — no qt6-svg-dev package required."


    echo "Linux (Ubuntu 22.04 / Mint) installation completed."

else
    echo "Unsupported OS. Please install dependencies manually."
    exit 1
fi

echo "===== install.sh completed successfully ====="
