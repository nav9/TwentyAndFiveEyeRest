add_library(TwentyAndFiveEyeRest_settings
    SettingsManager.cpp
    SettingsSearch.cpp
)
target_include_directories(TwentyAndFiveEyeRest_settings PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(TwentyAndFiveEyeRest_settings PUBLIC TwentyAndFiveEyeRest_common)
