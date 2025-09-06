add_executable(vision_backend
    src/main.cpp
)

target_include_directories(vision_backend
    PRIVATE
        ${ZMQ_INCLUDE_DIRS}
        ${GST_INCLUDE_DIRS}
)

target_link_libraries(vision_backend
    PRIVATE
        services
        config
        common
)

set(CMAKE_INSTALL_PREFIX "/opt/vision-backend" CACHE PATH "Install path prefix" FORCE)

install(TARGETS vision_backend
    RUNTIME DESTINATION bin
)
