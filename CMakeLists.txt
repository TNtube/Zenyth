cmake_minimum_required(VERSION 3.29)
project(Zenyth)

set(CMAKE_CXX_STANDARD 17)

if (NOT EXISTS "${CMAKE_BINARY_DIR}/xrepo.cmake")
    message(STATUS "Downloading xrepo.cmake from https://github.com/xmake-io/xrepo-cmake/")
    file(DOWNLOAD "https://raw.githubusercontent.com/xmake-io/xrepo-cmake/main/xrepo.cmake"
            "${CMAKE_BINARY_DIR}/xrepo.cmake"
            TLS_VERIFY ON)
endif()

include(${CMAKE_BINARY_DIR}/xrepo.cmake)

xrepo_package("directx-headers")
xrepo_package("glm")

include_directories(include)

add_executable(Zenyth WIN32
        src/main.cpp
        include/Window.hpp
        src/Window.cpp
        include/Application.hpp
        src/Application.cpp
)

target_compile_definitions(Zenyth PUBLIC UNICODE _UNICODE)

target_link_libraries(Zenyth PUBLIC d3d12 dxgi)

xrepo_target_packages(Zenyth directx-headers glm)
