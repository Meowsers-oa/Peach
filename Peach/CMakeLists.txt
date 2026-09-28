cmake_minimum_required(VERSION 3.20)

file(GLOB_RECURSE PEACH_SOURCES CONFIGURE_DEPENDS "src/*.c")

add_library(Peach STATIC ${PEACH_SOURCES}
        include/Peach/Common.h
        src/mWindow.c
        include/Peach/mWindow.h
        include/Peach/Input.h
        include/Peach/Renderer.h
        include/Peach/Camera.h
        include/Peach/Texture.h)

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
        assimp
        OpenGL::GL
        PRIVATE
        stb_image
)

if(UNIX AND NOT APPLE)
    target_link_libraries(Peach PUBLIC m)
endif()