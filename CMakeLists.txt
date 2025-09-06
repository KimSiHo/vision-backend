cmake_minimum_required(VERSION 3.16)

set(version_major "0")
set(version_minor "0")
set(version_patch "2")
set(project_version "${version_major}.${version_minor}.${version_patch}")

project(
    vision_backend-cpp
        VERSION ${project_version}
        DESCRIPTION "Vision Backend Project"
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH "/opt/vision-common/lib")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)

find_package(PkgConfig REQUIRED)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(fmt REQUIRED)
find_package(Threads REQUIRED)
find_package(TBB REQUIRED)

pkg_check_modules(SDBUS REQUIRED sdbus-c++)
pkg_check_modules(ZMQ REQUIRED libzmq)
pkg_check_modules(GST REQUIRED
    gstreamer-1.0
    gstreamer-video-1.0
    gstreamer-plugins-base-1.0
)
pkg_check_modules(PULSEAUDIO REQUIRED libpulse)
pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0)
pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-video-1.0)

add_subdirectory(src)

option(PN_BUILD_TESTS "Build tests" ON)
if(PN_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
