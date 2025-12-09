add_library(common
    Logger.cpp
)

target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(common PRIVATE Qt6::Core) # If needed later, otherwise keep minimal
