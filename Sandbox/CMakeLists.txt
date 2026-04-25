file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(Sandbox WIN32
        ${SOURCES}
)

target_compile_features(Sandbox PRIVATE cxx_std_20)

target_include_directories(Sandbox
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_precompile_headers(Sandbox REUSE_FROM Core)

target_link_libraries(Sandbox
    PRIVATE Renderer
)