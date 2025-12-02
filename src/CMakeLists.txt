add_subdirectory(common)
add_subdirectory(defaultTimer)
add_subdirectory(settings)
add_subdirectory(logging)
add_subdirectory(ui)

add_executable(TwentyAndFiveEyeRestApp main.cpp)
target_link_libraries(TwentyAndFiveEyeRestApp
    PRIVATE TwentyAndFiveEyeRest_ui TwentyAndFiveEyeRest_defaultTimer Qt6::Widgets Threads::Threads
)
install(TARGETS TwentyAndFiveEyeRestApp RUNTIME DESTINATION bin)
