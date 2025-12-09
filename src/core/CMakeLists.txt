add_library(core
    TimerEngine.cpp
    States.cpp
    TimeLogger.cpp
    HistoryAnalyzer.cpp
)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(core PRIVATE common platform)
