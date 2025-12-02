find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(TwentyAndFiveEyeRest_ui
    QtMainWindow.cpp
    SystemTrayController.cpp
)

target_include_directories(TwentyAndFiveEyeRest_ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(TwentyAndFiveEyeRest_ui PUBLIC Qt6::Widgets TwentyAndFiveEyeRest_common)
