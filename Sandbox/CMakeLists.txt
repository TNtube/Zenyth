file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(Sandbox WIN32
        ${SOURCES}
)

target_compile_features(Sandbox PRIVATE cxx_std_23)

target_include_directories(Sandbox
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_precompile_headers(Sandbox REUSE_FROM Core)

target_link_libraries(Sandbox
    PRIVATE Renderer
)

include(${CMAKE_SOURCE_DIR}/cmake/CompileShaders.cmake)

set(SHADER_DIR "${CMAKE_SOURCE_DIR}/assets/shaders")

compile_hlsl_shader(Sandbox "${SHADER_DIR}/shaders.hlsl" VSMain vs_6_0 shaders_VSMain)
compile_hlsl_shader(Sandbox "${SHADER_DIR}/shaders.hlsl" PSMain ps_6_0 shaders_PSMain)

# Copy source assets + compiled shaders to the output directory
add_custom_target(CopyAssets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:Sandbox>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_BINARY_DIR}/compiled_shaders"
        "$<TARGET_FILE_DIR:Sandbox>/assets"
    COMMENT "Copying assets to output directory"
)

add_dependencies(Sandbox CopyAssets)
