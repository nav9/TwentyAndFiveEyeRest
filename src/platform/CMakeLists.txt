add_library(platform
    OsUtils_Linux.cpp
)

target_include_directories(platform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(platform PRIVATE common) 
