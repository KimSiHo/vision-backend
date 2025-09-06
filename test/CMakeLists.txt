cmake_minimum_required(VERSION 3.14)

include(${CMAKE_SOURCE_DIR}/cmake/modules/FetchGTest.cmake)
enable_testing()

set(CTEST_OUTPUT_ON_FAILURE ON)
set_property(GLOBAL PROPERTY CTEST_CUSTOM_TEST_OUTPUT_TRUNCATION OFF)

add_subdirectory(unit)
add_subdirectory(integration)
