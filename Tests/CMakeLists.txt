file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(Tests ${TEST_SOURCES})

target_include_directories(Tests
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Tests PRIVATE Core)

target_compile_features(Tests PRIVATE cxx_std_23)

# Register with CTest so `ctest` works from the build directory
enable_testing()
add_test(NAME ZenythMathTests COMMAND Tests)
