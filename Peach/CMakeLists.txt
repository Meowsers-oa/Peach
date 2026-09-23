include(FetchContent)

# Build only the Assimp importers used by the engine.
# Assimp 5.4 predates warnings added by newer Clang versions.
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE) # Disable exporter code (read-only mode)

# Disable ALL ~40 importers by default, then selectively enable only what you need
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)

# Fetch the pinned Assimp version.
FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG        v5.4.3
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(assimp)

# Engine target
add_library(peach_engine STATIC
        core/Frame.c
        platform/Window.c
        platform/Input.c
        graphics/Camera.c
        graphics/PostProcess.c
        graphics/Renderer.c
        graphics/Lighting.c
        graphics/Shader.c
        graphics/Texture.c
        graphics/Mesh.c
        graphics/Mesh.h
        graphics/Ui.cpp
        graphics/Ui.h
)

target_include_directories(peach_engine
        PUBLIC "${PROJECT_SOURCE_DIR}"
        PRIVATE "${stb_SOURCE_DIR}"
)

target_compile_definitions(peach_engine PRIVATE PEACH_SHADER_DIR="${PEACH_SHADER_DIR}")

target_link_libraries(peach_engine
        PUBLIC
        glfw
        glad
        OpenGL::GL
        cglm
        assimp
        PRIVATE peach_cimgui
)

if(UNIX AND NOT APPLE)
    target_link_libraries(peach_engine PUBLIC m)
endif()
