# Renderer
# target_compile_features(Core PRIVATE cxx_std_20)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp include/*.hpp)

add_library(Renderer STATIC
    ${SOURCES}
)

target_include_directories(Renderer
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_precompile_headers(Renderer REUSE_FROM Core)

target_link_libraries(Renderer
    PUBLIC Core
    PRIVATE d3d12.lib dxgi.lib d3dcompiler.lib dxguid.lib
)

target_compile_features(Renderer PRIVATE cxx_std_20)