cmake_minimum_required(VERSION 3.16)

project(TwentyAndFiveEyeRest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)
find_package(Boost REQUIRED COMPONENTS unit_test_framework)

# Common functionality
add_subdirectory(src/common)
add_subdirectory(src/core)
add_subdirectory(src/platform)
add_subdirectory(src/defaultTimer)
add_subdirectory(src/gui)

enable_testing()
add_subdirectory(tests)
