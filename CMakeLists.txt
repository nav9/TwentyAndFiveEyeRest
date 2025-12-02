cmake_minimum_required(VERSION 3.18)
project(TwentyAndFiveEyeRest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable Qt automoc / autogen
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# -----------------------------------------------------------------------------
# Do NOT disable Vulkan via QT_FEATURE_vulkan — it causes conflicts with Qt6Gui.
# Instead, disable Vulkan backend in Qt *without modifying Qt features*.
# -----------------------------------------------------------------------------
set(QT_QPA_PLATFORM offscreen CACHE STRING "Disable Vulkan by bypassing GPU backends")
set(QT_DISABLE_VULKAN TRUE CACHE BOOL "Disable Vulkan backend")

# -----------------------------------------------------------------------------
# Qt6 — Widgets only
# -----------------------------------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Threads
find_package(Threads REQUIRED)

# Boost (minimum for Ubuntu 22.04 default = 1.74)
find_package(Boost 1.74 REQUIRED COMPONENTS unit_test_framework)

# Subdirectories
add_subdirectory(src)
add_subdirectory(tests)

# Packaging config (CPack)
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_NAME "TwentyAndFiveEyeRest")
set(CPACK_PACKAGE_VENDOR "TwentyAndFiveEyeRest")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "TwentyAndFiveEyeRest - Eye rest reminder")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.LGPLv3")
include(CPack)
