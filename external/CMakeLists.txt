cmake_minimum_required(VERSION 3.20)

include(FetchContent)

# GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4
)

# cglm
set(CGLM_SHARED OFF CACHE BOOL "" FORCE)
set(CGLM_STATIC ON CACHE BOOL "" FORCE)
set(CGLM_USE_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        cglm
        GIT_REPOSITORY https://github.com/recp/cglm.git
        GIT_TAG        v0.9.4
)

FetchContent_MakeAvailable(glfw cglm)

# Local Glad Library
add_library(glad STATIC glad/src/glad.c)
target_include_directories(glad PUBLIC glad/include)
# Local stb_image library
add_library(stb_image STATIC stb/src/stb_image.c)
target_include_directories(stb_image PUBLIC stb/include)
