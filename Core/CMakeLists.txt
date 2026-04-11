# Core
# target_compile_features(Core PRIVATE cxx_std_20)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp include/*.hpp)

add_library(Core STATIC
    ${SOURCES}
)

target_include_directories(Core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_precompile_headers(Core PUBLIC include/pch.hpp)

target_compile_features(Core PRIVATE cxx_std_20)
target_compile_definitions(Core PUBLIC UNICODE _UNICODE NOMINMAX)
