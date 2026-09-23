add_library(peach_engine STATIC
    platform/Window.c
    platform/Input.c
    graphics/Camera.c
    graphics/Renderer.c
    graphics/Lighting.c
    graphics/Shader.c
    graphics/Texture.c
)

target_include_directories(peach_engine
    PUBLIC "${PROJECT_SOURCE_DIR}"
    PRIVATE "${stb_SOURCE_DIR}"
)
target_compile_definitions(peach_engine PRIVATE PEACH_SHADER_DIR="${PEACH_SHADER_DIR}")
target_link_libraries(peach_engine PUBLIC glfw glad OpenGL::GL cglm)
if(UNIX AND NOT APPLE)
    target_link_libraries(peach_engine PUBLIC m)
endif()
