file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(ModelGallery WIN32
        ${SOURCES}
)

target_compile_features(ModelGallery PRIVATE cxx_std_20)

target_include_directories(ModelGallery PUBLIC
        ${CMAKE_SOURCE_DIR}/Core/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_precompile_headers(ModelGallery REUSE_FROM Core)

target_compile_definitions(ModelGallery PRIVATE WORKING_DIR="${CMAKE_CURRENT_SOURCE_DIR}/")

add_custom_command(TARGET ModelGallery POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DXC_DIR}/bin/x64/dxcompiler.dll"
        "${DXC_DIR}/bin/x64/dxil.dll"
        $<TARGET_FILE_DIR:ModelGallery>
)

#add_custom_command(TARGET ModelGallery PRE_LINK
#        COMMAND ${CMAKE_COMMAND} -E copy_directory
#        "${CMAKE_CURRENT_SOURCE_DIR}/resources"
#        "${CMAKE_BINARY_DIR}/ModelGallery/resources"
#        COMMENT "Copying resources to build directory"
#
#)

target_link_libraries(ModelGallery Core)
