add_library(EyeRestCore STATIC)

target_sources(EyeRestCore PRIVATE
    Core.cpp
    Settings.cpp
    Filesystem.cpp
    TimeFileManager.cpp
    DefaultTimer/DefaultTimer.cpp
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
if(LINUX)
    # Add Linux specific sources if any
    # target_sources(EyeRestCore PRIVATE operating_system_specific/linux/LinuxImpl.cpp)
elseif(WIN32)
    # target_sources(EyeRestCore PRIVATE operating_system_specific/windows/WindowsImpl.cpp)
elseif(APPLE)
    # target_sources(EyeRestCore PRIVATE operating_system_specific/macos/MacOSImpl.cpp)
endif()

add_executable(TwentyAndFiveEyeRest main.cpp)
target_link_libraries(TwentyAndFiveEyeRest PRIVATE EyeRestCore CLI11)
