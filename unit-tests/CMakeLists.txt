enable_testing()

add_executable(unit-tests
        hello_test.cpp)

target_compile_features(unit-tests PRIVATE cxx_std_20)

target_include_directories(unit-tests PRIVATE
        include/unit-tests)

target_link_libraries(unit-tests PRIVATE
        GTest::gtest_main
        rudplib)

include(GoogleTest)
gtest_discover_tests(unit-tests)