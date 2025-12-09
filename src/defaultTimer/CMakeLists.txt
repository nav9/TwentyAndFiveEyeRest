add_library(defaultTimer
    DefaultTimer.cpp
)

target_include_directories(defaultTimer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(defaultTimer PRIVATE core common platform)
