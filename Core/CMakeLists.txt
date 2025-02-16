add_subdirectory(Deps/DirectX-Headers)
add_subdirectory(Deps/DirectXTK12)
add_subdirectory(Deps/DirectXTex)


file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp include/*.hpp)

add_library(Core STATIC
        ${SOURCES}
        include/Win32Application.hpp
        src/Win32Application.cpp
)

target_include_directories(Core PUBLIC ${CMAKE_SOURCE_DIR}/Core/include)

target_precompile_headers(Core PUBLIC include/pch.hpp)

target_link_libraries(Core
    PUBLIC
        Microsoft::DirectX-Headers
        DirectXTK12
        DirectXTex
)

target_link_libraries(Core PUBLIC d3d12 dxgi D3DCompiler.lib dxguid.lib)

target_compile_definitions(Core PUBLIC UNICODE _UNICODE)