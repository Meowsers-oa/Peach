cmake_minimum_required(VERSION 4.3)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

project(peach C)

set(CMAKE_C_STANDARD 11)

include(FetchContent)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
        glad
        GIT_REPOSITORY https://github.com/Dav1dde/glad.git
        GIT_TAG        v0.1.36
)
FetchContent_MakeAvailable(glad)

FetchContent_Declare(
        cglm
        GIT_REPOSITORY https://github.com/recp/cglm.git
        GIT_TAG        v0.9.2
)
FetchContent_MakeAvailable(cglm)

FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG        master
)
FetchContent_MakeAvailable(stb)

# Find system OpenGL
find_package(OpenGL REQUIRED)

set(PEACH_SHADER_DIR "${CMAKE_CURRENT_BINARY_DIR}/assets/shaders")
foreach(shader scene.vert scene.frag shadow.vert shadow.frag)
    configure_file("assets/shaders/${shader}" "${PEACH_SHADER_DIR}/${shader}" COPYONLY)
endforeach()

add_subdirectory(Peach engine)

add_executable(peach main.c)
target_link_libraries(peach PRIVATE peach_engine)

