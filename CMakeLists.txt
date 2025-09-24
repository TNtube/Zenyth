cmake_minimum_required(VERSION 3.29)

# main project
project(Zenyth)

set(DXC_DIR "${CMAKE_SOURCE_DIR}/Core/Deps/dxc")

# add subdirectories
add_subdirectory(Core)
add_subdirectory(Sandbox)
add_subdirectory(ModelGallery)

