file(GLOB TEST_SOURCES CONFIGURE_DEPENDS *.cpp)

add_executable(test_hello ${TEST_SOURCES})

target_link_libraries(test_hello
    PRIVATE
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_hello)
