add_executable(test_runner
    test_timer.cpp
)

target_link_libraries(test_runner 
    PRIVATE 
    Boost::unit_test_framework
    core
    common
    platform # Although MockOsUtils creates a derived class, we might need platform for base definitions if not header-only
)
