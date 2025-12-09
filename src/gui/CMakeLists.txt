
set(GUI_SOURCES
    main.cpp
    MainWindow.cpp
    SettingsDialog.cpp
)

set(GUI_HEADERS
    MainWindow.h
    SettingsDialog.h
)

add_executable(TwentyAndFiveEyeRest
    ${GUI_SOURCES}
    ${GUI_HEADERS}
)

target_link_libraries(TwentyAndFiveEyeRest
    PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
    core
    platform
    common
)

target_include_directories(TwentyAndFiveEyeRest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Fix for "std::unique_ptr moved" if needed, usually just works with C++14/17
