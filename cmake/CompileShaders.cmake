find_program(DXC_EXECUTABLE dxc
        HINTS
        "$ENV{WindowsSdkVerBinPath}/x64"
        "$ENV{WindowsSdkDir}/bin/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/x64"
)

if(NOT DXC_EXECUTABLE)
    message(FATAL_ERROR "dxc.exe not found. Ensure the Windows SDK is installed.")
endif()

message(STATUS "Found DXC: ${DXC_EXECUTABLE}")

# compile_hlsl_shader(<target> <source> <entry_point> <profile> <output_name>)
# Example:
#   compile_hlsl_shader(Sandbox "${CMAKE_SOURCE_DIR}/assets/shaders/shaders.hlsl" VSMain vs_6_0 shaders_VSMain)

function(compile_hlsl_shader TARGET SHADER_SOURCE ENTRY_POINT PROFILE OUTPUT_NAME)
    set(OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/compiled_shaders/${OUTPUT_NAME}.cso")

    add_custom_command(
        OUTPUT "${OUTPUT_FILE}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/compiled_shaders"
        COMMAND ${DXC_EXECUTABLE}
            -T ${PROFILE}
            -E ${ENTRY_POINT}
            -Fo "${OUTPUT_FILE}"
            "$<$<CONFIG:Debug>:-Zi>"
            "$<$<CONFIG:Debug>:-Od>"
            "$<$<CONFIG:Release>:-O3>"
            "${SHADER_SOURCE}"
        DEPENDS "${SHADER_SOURCE}"
        COMMENT "Compiling HLSL: ${OUTPUT_NAME} (${PROFILE})"
        VERBATIM
    )

    add_custom_target(${OUTPUT_NAME}_shader DEPENDS "${OUTPUT_FILE}")
    add_dependencies(${TARGET} ${OUTPUT_NAME}_shader)
endfunction()
