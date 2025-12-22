add_library(EyeRestCore STATIC)

target_sources(EyeRestCore PRIVATE
    Core.cpp
    Settings.cpp
    Filesystem.cpp
    TimeFileManager.cpp
    DefaultTimer/DefaultTimer.cpp
    DefaultTimer/TimerStates.cpp
    operating_system_specific/OSInfoProvider.cpp
)

target_include_directories(EyeRestCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(EyeRestCore PUBLIC
    spdlog::spdlog
    Boost::system
    Boost::filesystem
    nlohmann_json::nlohmann_json
)

# OS specifics
if(UNIX AND NOT APPLE)
    target_sources(EyeRestCore PRIVATE operating_system_specific/LinuxLockDetector.cpp)
elseif(WIN32)
    # target_sources(EyeRestCore PRIVATE operating_system_specific/WindowsLockDetector.cpp)
elseif(APPLE)
    # target_sources(EyeRestCore PRIVATE operating_system_specific/MacOSLockDetector.cpp)
endif()

add_executable(TwentyAndFiveEyeRest main.cpp)
target_link_libraries(TwentyAndFiveEyeRest PRIVATE EyeRestCore CLI11)
