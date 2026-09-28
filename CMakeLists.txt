cmake_minimum_required(VERSION 3.20)
project(peach LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Find system OpenGL at the root level so imported targets (OpenGL::GL) are global
find_package(OpenGL REQUIRED)

add_subdirectory(external)
add_subdirectory(Peach)
add_subdirectory(sandbox)
option(PEACH_BUILD_TESTS "Build renderer tests (requires a graphical OpenGL session)" OFF)
if(PEACH_BUILD_TESTS)
    enable_testing()
    add_executable(renderer_tests tests/Renderer.c)
    target_link_libraries(renderer_tests PRIVATE Peach)
    target_compile_definitions(renderer_tests PRIVATE
            PEACH_TEST_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/sandbox/assets"
            PEACH_TEST_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Peach/shaders"
            PEACH_TEST_SHADER_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/shaders"
    )
    add_test(NAME renderer COMMAND renderer_tests)
    add_executable(time_tests tests/Time.c)
    target_link_libraries(time_tests PRIVATE Peach)
    add_test(NAME time COMMAND time_tests)
endif()
