add_library(backend_services
        ai_service.cpp
        camera_service.cpp
        control_service.cpp
)

set(DS_ROOT ${SYSROOT}/opt/nvidia/deepstream/deepstream)
message(STATUS "DS_ROOT = ${DS_ROOT}")

target_include_directories(backend_services
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${ZMQ_INCLUDE_DIRS}
        ${GST_INCLUDE_DIRS}
        ${DS_ROOT}/sources/includes
)

target_link_directories(backend_services
        PUBLIC
        ${DS_ROOT}/lib
        ${DS_ROOT}/lib/aarch64-linux-gnu
)

target_link_libraries(backend_services
        PUBLIC
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
