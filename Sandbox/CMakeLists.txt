set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(Sandbox WIN32
        main.cpp
)

target_include_directories(Sandbox PUBLIC ${CMAKE_SOURCE_DIR}/Core/include)
target_include_directories(Sandbox PUBLIC ${CMAKE_SOURCE_DIR}/Sandbox)

# Copy resources to build directory
add_custom_target(copy_resources ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Sandbox/resources
        ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/resources
        COMMENT "Copying resources to build directory"
)

add_dependencies(Sandbox copy_resources)

target_link_libraries(Sandbox Core)