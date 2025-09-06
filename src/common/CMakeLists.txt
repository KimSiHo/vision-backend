add_library(common
    STATIC
        src/zmq/pub_socket.cpp
        src/zmq/rep_socket.cpp
)

target_include_directories(common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${ZMQ_INCLUDE_DIRS}
)

target_link_libraries(common
    PUBLIC
        ${ZMQ_LIBRARIES}
        nlohmann_json::nlohmann_json
        spdlog::spdlog
)
