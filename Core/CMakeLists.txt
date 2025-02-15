add_subdirectory(Deps/DirectX-Headers)
add_subdirectory(Deps/DirectXTK12)
add_subdirectory(Deps/DirectXTex)


add_library(Core STATIC
        include/Window.hpp
        src/Window.cpp
        include/Application.hpp
        src/Application.cpp
        include/Core.hpp
        include/Vertex.hpp
        include/Buffers.hpp
        src/Buffers.cpp
        include/Texture.hpp
        src/Texture.cpp
        include/Camera.hpp
        src/Camera.cpp
        include/engine/Transform.hpp
        src/engine/Transform.cpp
)

#if(CMAKE_BUILD_TYPE MATCHES Debug)
#    target_compile_definitions(Core PUBLIC _DEBUG=1)
#elseif(CMAKE_BUILD_TYPE MATCHES Release)
#    target_compile_definitions(Core PUBLIC _DEBUG=0)
#endif()

target_include_directories(Core PUBLIC ${CMAKE_SOURCE_DIR}/Core/include)

target_link_libraries(Core
    PUBLIC
        Microsoft::DirectX-Headers
        DirectXTK12
        DirectXTex
)

target_link_libraries(Core PUBLIC d3d12 dxgi D3DCompiler.lib dxguid.lib)

target_compile_definitions(Core PUBLIC UNICODE _UNICODE)