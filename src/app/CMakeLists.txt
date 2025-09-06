add_executable(vision_backend
        main.cpp
)

target_compile_definitions(vision_backend
        PRIVATE
        SPDLOG_FMT_EXTERNAL=1
)

target_include_directories(vision_backend
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${ZMQ_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${GST_INCLUDE_DIRS}
)

target_link_libraries(vision_backend
        PRIVATE
        backend_services
)

install(TARGETS vision_backend
        RUNTIME DESTINATION /home/great/workspace/vision-backend/build
        #RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
