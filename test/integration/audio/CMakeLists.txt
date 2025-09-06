file(GLOB TEST_SOURCES CONFIGURE_DEPENDS *.cpp)

add_executable(test_integration_audio ${TEST_SOURCES})

target_link_libraries(test_integration_audio PRIVATE
        GTest::gtest_main
        services
)

include(GoogleTest)
gtest_discover_tests(test_integration_audio)
