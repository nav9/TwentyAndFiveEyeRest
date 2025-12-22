enable_testing()

add_executable(EyeRestTests
    main_test.cpp
)

target_link_libraries(EyeRestTests PRIVATE
    EyeRestCore
    Boost::unit_test_framework
)

add_test(NAME AllTests COMMAND EyeRestTests)
