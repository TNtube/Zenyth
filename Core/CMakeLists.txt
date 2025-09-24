add_subdirectory(Deps/DirectX-Headers)
add_subdirectory(Deps/DirectXTK12)

include(DownloadDX.cmake)

download_dxc("${CMAKE_SOURCE_DIR}/Core/Deps")

# DXC
include_directories("${DXC_DIR}/inc")

# ImGui

file(GLOB IMGUI_SOURCES
    Deps/imgui/*.cpp
    Deps/imgui/backends/imgui_impl_win32.cpp
    Deps/imgui/backends/imgui_impl_dx12.cpp
)

add_library(ImGui STATIC
    ${IMGUI_SOURCES}
)

target_include_directories(ImGui
    PUBLIC
        ${CMAKE_SOURCE_DIR}/Core/Deps/imgui
)


# Core
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp include/*.hpp)

add_library(Core STATIC
    ${SOURCES}
)

target_compile_features(Core PRIVATE cxx_std_20)

target_include_directories(Core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_precompile_headers(Core PUBLIC include/pch.hpp)

target_link_libraries(Core
    PUBLIC
        Microsoft::DirectX-Headers
        DirectXTK12
        d3d12
        dxgi
        D3DCompiler.lib
        dxguid.lib
        dxcompiler
        ImGui
        "${DXC_DIR}/lib/x64/dxcompiler.lib"
)

target_compile_definitions(Core PUBLIC UNICODE _UNICODE NOMINMAX)