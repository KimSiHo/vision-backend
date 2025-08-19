cmake_minimum_required(VERSION 3.16)
project(vision_backend-cpp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(ZMQ REQUIRED libzmq)
pkg_check_modules(GST REQUIRED
    gstreamer-1.0
    gstreamer-video-1.0
    gstreamer-plugins-base-1.0
)

find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(fmt REQUIRED)
find_package(Threads REQUIRED)
find_package(vision_common REQUIRED)
find_package(TBB REQUIRED)
pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0)
pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-video-1.0)

message(DEBUG "spdlog_DIR = ${spdlog_DIR}")
message(DEBUG "nlohmann_json_DIR = ${nlohmann_json_DIR}")
message(DEBUG "fmt_DIR = ${fmt_DIR}")
message(DEBUG "fmt_DIR = ${Threads_DIR}")

message(DEBUG "Threads_FOUND = ${Threads_FOUND}")

get_target_property(SPDLOG_LOC spdlog::spdlog LOCATION)
message(STATUS "spdlog::spdlog -> ${SPDLOG_LOC}")

get_target_property(JSON_LOC nlohmann_json::nlohmann_json LOCATION)
message(STATUS "nlohmann_json::nlohmann_json -> ${JSON_LOC}")

get_target_property(FMT_LOC fmt::fmt LOCATION)
message(STATUS "fmt::fmt -> ${FMT_LOC}")

get_target_property(THREADS_INC Threads::Threads INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "Threads::Threads include dirs -> ${THREADS_INC}")

add_executable(vision_backend
    src/main.cpp
    src/camera_service.cpp
    src/ai_service.cpp
    src/control_service.cpp
)



set(SYSROOT ${CMAKE_SYSROOT})
if(NOT SYSROOT AND DEFINED ENV{OECORE_TARGET_SYSROOT})
  set(SYSROOT $ENV{OECORE_TARGET_SYSROOT})
endif()

set(DS_ROOT ${SYSROOT}/opt/nvidia/deepstream/deepstream-7.1)


include_directories(
    ${GST_INCLUDE_DIRS}
    ${DS_ROOT}/sources/includes        # ← 여기!
    ${DS_ROOT}/lib
    ${DS_ROOT}/lib/aarch64-linux-gnu
)

link_directories(
    ${DS_ROOT}/lib/aarch64-linux-gnu   # DS 라이브러리
)

target_compile_definitions(vision_backend PRIVATE SPDLOG_FMT_EXTERNAL=1)

target_include_directories(vision_backend
    PRIVATE
        ${ZMQ_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${GST_INCLUDE_DIRS}
)


target_link_libraries(vision_backend
    PRIVATE
        ${ZMQ_LIBRARIES}
        spdlog::spdlog
        fmt::fmt
        Threads::Threads
        nlohmann_json::nlohmann_json
        ${GST_LIBRARIES}
        vision_common::vision_common
        TBB::tbb
        ${GST_APP_LIBRARIES}
        ${DS_ROOT}/lib/libnvdsgst_meta.so
        ${DS_ROOT}/lib/libnvds_meta.so
)

  #   nvdsgst_meta
     #     nvds_meta
      #    nvbufsurface


# === 설치 규칙 추가 ===
# 실행파일
install(TARGETS vision_backend
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}  # 보통 bin
)

# 공개 헤더(있으면)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vision_backend
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)

