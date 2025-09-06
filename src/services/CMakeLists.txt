add_library(services
    STATIC
        src/impl/infer/ai_service.cpp
        src/impl/camera/camera_service.cpp
        src/impl/music/music_service.cpp
        src/impl/music/playbin-pipeline/playbin_pipeline.cpp
        src/impl/music/custom-pipeline/custom_pipeline.cpp
        src/impl/music/custom-pipeline/source_bin.cpp
        src/impl/music/custom-pipeline/sink_bin.cpp
        src/impl/audio/audio_service.cpp
        src/impl/bluetooth/bluetooth_service.cpp
        src/impl/control/control_service.cpp

        src/adapters/music/music_service_adapter.cpp
        src/adapters/camera/camera_service_adapter.cpp
        src/adapters/bluetooth/bluetooth_service_adapter.cpp
        src/adapters/audio/audio_service_adapter.cpp
)

target_include_directories(services
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${ZMQ_INCLUDE_DIRS}
        ${GST_INCLUDE_DIRS}
        ${DS_ROOT}/sources/includes
        ${PULSEAUDIO_INCLUDE_DIRS}
        ${SDBUS_INCLUDE_DIRS}

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_directories(services
    PUBLIC
        ${DS_ROOT}/lib
        ${DS_ROOT}/lib/aarch64-linux-gnu
)

target_link_libraries(services
    PUBLIC
        ${ZMQ_LIBRARIES}
        spdlog::spdlog
        fmt::fmt
        Threads::Threads
        nlohmann_json::nlohmann_json
        ${GST_LIBRARIES}
        TBB::tbb
        ${GST_APP_LIBRARIES}
        ${DS_ROOT}/lib/libnvdsgst_meta.so
        ${DS_ROOT}/lib/libnvds_meta.so
        config
        ${PULSEAUDIO_LIBRARIES}
        ${SDBUS_LIBRARIES}

    PRIVATE
        common
        config
)
