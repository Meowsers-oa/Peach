cmake_minimum_required(VERSION 3.20)

file(GLOB_RECURSE SANDBOX_SOURCES CONFIGURE_DEPENDS "src/*.c")

add_executable(sandbox ${SANDBOX_SOURCES})

target_link_libraries(sandbox PRIVATE Peach)

target_compile_definitions(sandbox PRIVATE
        PEACH_SANDBOX_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)
