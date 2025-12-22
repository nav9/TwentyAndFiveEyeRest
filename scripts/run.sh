#!/bin/bash

# Determine the project root directory (assuming script is in scripts/)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"
EXECUTABLE_NAME="TwentyAndFiveEyeRest"

# Move to project root to avoid path issues
cd "$PROJECT_ROOT"

function show_menu() {
    echo "--------------------------------"
    echo "TwentyAndFiveEyeRest Manager"
    echo "--------------------------------"
    echo "1) Run (As Debug. CLI)"
    echo "2) Run (Without any CLI parameters)"        
    echo "3) Configure (CMake)"
    echo "4) Build"
    echo "5) Run Tests"
    echo "6) Clean"
    echo "7) Exit"
    echo "--------------------------------"
}

function configure() {
    mkdir -p "$BUILD_DIR"
    if command -v ninja >/dev/null 2>&1; then
        cmake -B "$BUILD_DIR" -S "$PROJECT_ROOT" -G Ninja
    else
        echo "Ninja not found, using Unix Makefiles..."
        cmake -B "$BUILD_DIR" -S "$PROJECT_ROOT" -G "Unix Makefiles"
    fi
}

function build() {
    if [ ! -d "$BUILD_DIR" ]; then
        echo "Build directory not found. Configuring first..."
        configure
    fi
    cmake --build "$BUILD_DIR" --parallel $(nproc)
}

function run() {
    EXECUTABLE="$BUILD_DIR/bin/$EXECUTABLE_NAME"
    if [ -f "$EXECUTABLE" ]; then
        "$EXECUTABLE" "$@"
    elif [ ! -d "$BUILD_DIR" ]; then
        echo "Build directory not found. Please configure and build first."
    else
        echo "Executable not found at $EXECUTABLE. Please build first."
    fi
}

function run_tests() {
    if [ -d "$BUILD_DIR" ]; then
        cd "$BUILD_DIR" && ctest --output-on-failure
        cd "$PROJECT_ROOT"
    else
        echo "Build directory not found. Please configure and build first."
    fi
}

function clean() {
    rm -rf "$BUILD_DIR"
    echo "Cleaned build directory."
}

# Process command line arguments if present
if [ "$#" -gt 0 ]; then
    case "$1" in
        configure) configure ;;
        build) build ;;
        run) shift; run "$@" ;;
        test) run_tests ;;
        clean) clean ;;
        *) echo "Unknown command: $1"; exit 1 ;;
    esac
    exit 0
fi

# Interactive menu
while true; do
    show_menu
    read -r -p "Select an option: " choice
    case "$choice" in
        1) run -d ;;
        2) 
           read -r -p "Enter args (optional): " args
           run $args 
           ;;        
        3) configure ;;
        4) build ;;
        5) run_tests ;;
        6) clean ;;
        7) exit 0 ;;
        *) echo "Invalid option." ;;
    esac
    echo
done
