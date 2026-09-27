cmake_minimum_required(VERSION 3.20)

file(GLOB_RECURSE PEACH_SOURCES CONFIGURE_DEPENDS "src/*.c")

add_library(Peach STATIC ${PEACH_SOURCES})

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
)