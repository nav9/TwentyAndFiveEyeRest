add_library(TwentyAndFiveEyeRest_defaultTimer
    DefaultTimer.cpp
    DefaultTimerState.cpp
)

target_include_directories(TwentyAndFiveEyeRest_defaultTimer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(TwentyAndFiveEyeRest_defaultTimer PUBLIC TwentyAndFiveEyeRest_common Threads::Threads)
