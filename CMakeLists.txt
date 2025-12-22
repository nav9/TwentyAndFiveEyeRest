cmake_minimum_required(VERSION 3.20)

project(TwentyAndFiveEyeRest
    VERSION 0.1.0
    DESCRIPTION "A cross-platform eye rest reminder"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable testing
include(CTest)

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# FetchContent for dependencies
include(FetchContent)

# CLI11 - Vendored
add_library(CLI11 INTERFACE)
target_include_directories(CLI11 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/vendor/CLI11)

# spdlog - System
find_package(spdlog REQUIRED)

# nlohmann_json
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(json)

# Boost
find_package(Boost REQUIRED COMPONENTS filesystem system unit_test_framework)

# Add subdirectories
add_subdirectory(src)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
