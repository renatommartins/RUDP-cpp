enable_testing()

add_executable(unit-tests
        hello_test.cpp
        MockTransceiverTest.cpp
        UdpTransceiverTest.cpp
        Rudp.Client.cpp)

target_compile_features(unit-tests PRIVATE cxx_std_23)

target_include_directories(unit-tests PRIVATE
        include/unit-tests)

target_link_libraries(unit-tests PRIVATE
        GTest::gtest_main
        GTest::gmock
        rudplib)

include(GoogleTest)
gtest_discover_tests(unit-tests)