#!/bin/bash

PROJECT_DIR="TwentyAndFiveEyeRest"
BUILD_DIR="$PROJECT_DIR/build"
EXECUTABLE_NAME="TwentyAndFiveEyeRest"

show_menu() {
    echo "=========================================="
    echo "    TwentyAndFiveEyeRest Manager"
    echo "=========================================="
    echo "1. Build"
    echo "2. Clean and Build"
    echo "3. Run Tests"
    echo "4. Run Program"
    echo "5. Check for Memory Leaks (Valgrind)"
    echo "6. Generate Installer"
    echo "7. Exit"
    echo "=========================================="
    echo -n "Enter your choice: "
}

build_project() {
    if [ ! -d "$BUILD_DIR" ]; then
        mkdir -p "$BUILD_DIR"
    fi
    cd "$BUILD_DIR"
    cmake ..
    if make -j$(nproc); then
        echo "Build Successful."
        # Versioning executable
        # Assuming exe is in src/gui or root bin? CMake output location varies.
        # Let's find it.
        # Usually it's in the subfolder where add_executable was called.
        # Check src/gui or if we add a binary output dir in cmake.
        
        # For now, let's copy from specific location if found
        if [ -f "src/gui/$EXECUTABLE_NAME" ]; then
             cp src/gui/$EXECUTABLE_NAME ../$EXECUTABLE_NAME
             cp src/gui/$EXECUTABLE_NAME ../$EXECUTABLE_NAME-$(date +%Y%m%d%H%M%S)
        fi
    else
        echo "Build Failed."
    fi
    cd ../.. 
}

clean_build() {
    rm -rf "$BUILD_DIR"
    build_project
}

run_tests() {
    if [ -f "$BUILD_DIR/tests/test_runner" ]; then
        "$BUILD_DIR/tests/test_runner"
    else
        echo "Tests not built or found. Try building first."
    fi
}

run_program() {
    if [ -f "$PROJECT_DIR/$EXECUTABLE_NAME" ]; then
        ./$PROJECT_DIR/$EXECUTABLE_NAME
    elif [ -f "$BUILD_DIR/src/gui/$EXECUTABLE_NAME" ]; then
        ./$BUILD_DIR/src/gui/$EXECUTABLE_NAME
    else
        echo "Executable not found. Build first."
    fi
}

check_leaks() {
    if [ -f "$PROJECT_DIR/$EXECUTABLE_NAME" ]; then
        valgrind --leak-check=full --track-origins=yes ./$PROJECT_DIR/$EXECUTABLE_NAME
    elif [ -f "$BUILD_DIR/src/gui/$EXECUTABLE_NAME" ]; then
         valgrind --leak-check=full --track-origins=yes ./$BUILD_DIR/src/gui/$EXECUTABLE_NAME
    else
        echo "Executable not found. Build first."
    fi
}

generate_installer() {
     echo "Installer generation implementation to be done."
}

while true; do
    show_menu
    read choice
    case $choice in
        1) build_project ;;
        2) clean_build ;;
        3) run_tests ;;
        4) run_program ;;
        5) check_leaks ;;
        6) generate_installer ;;
        7) exit 0 ;;
        *) echo "Invalid choice. Please try again." ;;
    esac
    echo -e "\nPress Enter to continue..."
    read
done
