set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS include/*.hpp src/*.cpp)

add_executable(Sandbox WIN32
    ${SOURCES}
)

target_compile_features(Sandbox PRIVATE cxx_std_20)

target_include_directories(Sandbox PUBLIC
    ${CMAKE_SOURCE_DIR}/Core/include
    ${CMAKE_SOURCE_DIR}/Sandbox/include
)

target_precompile_headers(Sandbox REUSE_FROM Core)


if (CMAKE_GENERATOR MATCHES "Visual Studio")
    # Visual Studio seems to have a different build directory structure
    set(RESOURCES_BUILD_DIR ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/resources)
else()
    set(RESOURCES_BUILD_DIR ${CMAKE_BINARY_DIR}/resources)
endif()

# Copy resources to build directory
add_custom_target(copy_resources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/Sandbox/resources"
    "${RESOURCES_BUILD_DIR}"
    COMMENT "Copying resources to build directory"
)

add_dependencies(Sandbox copy_resources)

target_link_libraries(Sandbox Core)
