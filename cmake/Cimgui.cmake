include(FetchContent)

function(peach_add_cimgui)
    # Allow this new dependency's initial download even with the existing cache offline.
    if(NOT EXISTS "${FETCHCONTENT_BASE_DIR}/cimgui-src/cimgui.cpp")
        set(FETCHCONTENT_FULLY_DISCONNECTED OFF)
    endif()
    FetchContent_Declare(cimgui
        GIT_REPOSITORY https://github.com/cimgui/cimgui.git
        GIT_TAG 1.91.9b
        GIT_SHALLOW TRUE
        GIT_SUBMODULES imgui
        GIT_SUBMODULES_RECURSE FALSE
        UPDATE_DISCONNECTED TRUE
        SOURCE_SUBDIR peach-custom-build
    )
    FetchContent_MakeAvailable(cimgui)

    # Only core ImGui and the two backends used by this application.
    add_library(peach_cimgui STATIC
        "${cimgui_SOURCE_DIR}/cimgui.cpp"
        "${cimgui_SOURCE_DIR}/imgui/imgui.cpp"
        "${cimgui_SOURCE_DIR}/imgui/imgui_draw.cpp"
        "${cimgui_SOURCE_DIR}/imgui/imgui_tables.cpp"
        "${cimgui_SOURCE_DIR}/imgui/imgui_widgets.cpp"
        "${cimgui_SOURCE_DIR}/imgui/imgui_demo.cpp"
        "${cimgui_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.cpp"
        "${cimgui_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.cpp"
    )
    target_compile_features(peach_cimgui PRIVATE cxx_std_11)
    target_include_directories(peach_cimgui PUBLIC
        "${cimgui_SOURCE_DIR}"
        "${cimgui_SOURCE_DIR}/imgui"
    )
    target_compile_definitions(peach_cimgui
        PUBLIC CIMGUI_USE_GLFW CIMGUI_USE_OPENGL3 IMGUI_DISABLE_OBSOLETE_FUNCTIONS
        PRIVATE IMGUI_DISABLE_DEMO_WINDOWS "IMGUI_IMPL_API=extern \"C\""
    )
    target_link_libraries(peach_cimgui PUBLIC glfw OpenGL::GL)
    if(WIN32)
        target_link_libraries(peach_cimgui PRIVATE imm32)
    endif()
endfunction()

peach_add_cimgui()
