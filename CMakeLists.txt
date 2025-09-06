cmake_minimum_required(VERSION 3.16)
project(vision_backend-cpp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH "/opt/vision-common/lib")

find_package(PkgConfig REQUIRED)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(fmt REQUIRED)
find_package(Threads REQUIRED)
find_package(vision_common REQUIRED)
find_package(TBB REQUIRED)

pkg_check_modules(ZMQ REQUIRED libzmq)
pkg_check_modules(GST REQUIRED
    gstreamer-1.0
    gstreamer-video-1.0
    gstreamer-plugins-base-1.0
)
pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0)
pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-video-1.0)

add_subdirectory(src)
