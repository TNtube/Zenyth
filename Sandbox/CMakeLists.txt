file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS include/*.hpp src/*.cpp)

add_executable(Sandbox WIN32
    ${SOURCES}
)

target_compile_features(Sandbox PRIVATE cxx_std_20)

target_include_directories(Sandbox PUBLIC
    ${CMAKE_SOURCE_DIR}/Core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(Sandbox PRIVATE WORKING_DIR="${CMAKE_CURRENT_SOURCE_DIR}/")

target_precompile_headers(Sandbox REUSE_FROM Core)

add_custom_command(TARGET Sandbox POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DXC_DIR}/bin/x64/dxcompiler.dll"
        "${DXC_DIR}/bin/x64/dxil.dll"
        $<TARGET_FILE_DIR:Sandbox>)

#add_custom_command(TARGET Sandbox POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_directory
#        "${CMAKE_CURRENT_SOURCE_DIR}/resources"
#        "${CMAKE_BINARY_DIR}/Sandbox/resources"
#        COMMENT "Copying resources to build directory"
#)

target_link_libraries(Sandbox Core)
