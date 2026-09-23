include(FetchContent)

function(peach_add_cimguizmo)
    if(NOT EXISTS "${FETCHCONTENT_BASE_DIR}/cimguizmo-src/cimguizmo.cpp")
        set(FETCHCONTENT_FULLY_DISCONNECTED OFF)
    endif()
    # Matches the ImGui 1.91 API used by cimgui.
    FetchContent_Declare(cimguizmo
        GIT_REPOSITORY https://github.com/cimgui/cimguizmo.git
        GIT_TAG c3ade65dbda9750536a6a26c8d8fe705f0cfdfd1
        GIT_SUBMODULES ImGuizmo
        GIT_SUBMODULES_RECURSE FALSE
        UPDATE_DISCONNECTED TRUE
        SOURCE_SUBDIR peach-custom-build
    )
    FetchContent_MakeAvailable(cimguizmo)
    add_library(peach_cimguizmo STATIC
        "${cimguizmo_SOURCE_DIR}/cimguizmo.cpp"
        "${cimguizmo_SOURCE_DIR}/ImGuizmo/ImGuizmo.cpp"
    )
    target_compile_features(peach_cimguizmo PRIVATE cxx_std_11)
    target_include_directories(peach_cimguizmo PUBLIC "${cimguizmo_SOURCE_DIR}")
    target_link_libraries(peach_cimguizmo PUBLIC peach_cimgui)
endfunction()

peach_add_cimguizmo()
