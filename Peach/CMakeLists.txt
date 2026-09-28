cmake_minimum_required(VERSION 3.20)

file(GLOB_RECURSE PEACH_SOURCES CONFIGURE_DEPENDS "src/*.c")

add_library(Peach STATIC ${PEACH_SOURCES}
        include/Peach/Common.h
        include/Peach/mWindow.h
        include/Peach/Input.h
        include/Peach/Renderer.h
        include/Peach/Camera.h
        include/Peach/Texture.h
        include/Peach/Transform.h
        include/Peach/Shader.h
        include/Peach/Time.h
        include/Peach/Shape.h
        include/Peach/Sprite.h
        include/Peach/Map.h
        include/Peach/Resource.h
        include/Peach/Light.h)

target_include_directories(Peach
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(Peach
        PUBLIC
        glad
        glfw
        cglm
        OpenGL::GL
        PRIVATE
        stb_image
)

if(UNIX AND NOT APPLE)
    target_link_libraries(Peach PUBLIC m)
endif()

set(PEACH_SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders" CACHE PATH "Runtime directory for Peach's batch shaders")
get_filename_component(PEACH_SHADER_DIR_ABSOLUTE "${PEACH_SHADER_DIR}" ABSOLUTE BASE_DIR "${CMAKE_SOURCE_DIR}")
target_compile_definitions(Peach PRIVATE PEACH_SHADER_DIR="${PEACH_SHADER_DIR_ABSOLUTE}")
