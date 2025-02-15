cmake_minimum_required(VERSION 3.29)
set(CMAKE_CXX_STANDARD 20)

# Set this before anything else
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# main project
project(Zenyth)

# add subdirectories
add_subdirectory(Core)
add_subdirectory(Sandbox)

