cmake_minimum_required(VERSION 4.3)

# Allow older subprojects to configure on modern CMake
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

project(peach C)

set(CMAKE_C_STANDARD 11)

include(FetchContent)

# 1. Fetch GLFW
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# 2. Fetch GLAD (it creates its own 'glad' target automatically)
FetchContent_Declare(
        glad
        GIT_REPOSITORY https://github.com/Dav1dde/glad.git
        GIT_TAG        v0.1.36
)
FetchContent_MakeAvailable(glad)

# 3. Fetch cglm (OpenGL Mathematics for C)
FetchContent_Declare(
        cglm
        GIT_REPOSITORY https://github.com/recp/cglm.git
        GIT_TAG        v0.9.2
)
FetchContent_MakeAvailable(cglm)

# 4. Fetch stb (stb_image, etc.)
FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG        master
)
FetchContent_MakeAvailable(stb)

# Find system OpenGL
find_package(OpenGL REQUIRED)

add_executable(peach main.c
        Peach/Peach.h
        Peach/Window.c
        Peach/Window.h
        Peach/structs.h
        Peach/Input.c
        Peach/Input.h
        Peach/Shader.c
        Peach/Shader.h
        Peach/utils.h
        Peach/Renderer.c
        Peach/Renderer.h
        Peach/Texture.c
        Peach/Texture.h
        Peach/Camera.c
        Peach/Camera.h)

target_include_directories(peach PRIVATE
        ${stb_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link GLFW, GLAD, OpenGL, and cglm
target_link_libraries(peach PRIVATE
        glfw
        glad
        OpenGL::GL
        cglm
)