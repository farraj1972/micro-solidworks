cmake_minimum_required(VERSION 3.20)

project(MicroSolidWorks LANGUAGES CXX)

include(CTest)
include(FetchContent)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG 6fa36017cfd5731d617e1a934f0e5ea9c4445b13
)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG a74efa0d5628b74adc0426af4c5710e287fa7c2c
)

FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG 73db193f853e2ee079bf3ca8a64aa2eaf6459043
    SOURCE_SUBDIR cmake
)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG f5befd2d29e66809cd1110a152e375a7f1981f06
)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(spdlog)
FetchContent_MakeAvailable(glfw)
FetchContent_MakeAvailable(glad)
FetchContent_MakeAvailable(imgui)

glad_add_library(glad_gl_core_33 REPRODUCIBLE LOADER API gl:core=3.3)
if(MSVC)
    target_compile_options(
        glad_gl_core_33
        PRIVATE "$<$<CONFIG:Debug>:/MDd>" "$<$<NOT:$<CONFIG:Debug>>:/MD>"
    )
endif()

add_library(microsw_imgui_backend STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_compile_features(microsw_imgui_backend PUBLIC cxx_std_20)
set_target_properties(microsw_imgui_backend PROPERTIES
    CXX_EXTENSIONS OFF
)
target_compile_definitions(microsw_imgui_backend PRIVATE GLFW_INCLUDE_NONE)
target_include_directories(microsw_imgui_backend PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(microsw_imgui_backend PRIVATE glfw)

add_library(microsw_logging STATIC
    src/app/logging/Logger.cpp
)

target_compile_features(microsw_logging PUBLIC cxx_std_20)
set_target_properties(microsw_logging PROPERTIES
    CXX_EXTENSIONS OFF
)
target_include_directories(microsw_logging PUBLIC src)
target_link_libraries(microsw_logging PRIVATE spdlog::spdlog)

add_library(microsw_math STATIC
    src/core/math/Matrix3.cpp
    src/core/math/Matrix4.cpp
    src/core/math/Tolerance.cpp
    src/core/math/Transformations.cpp
    src/core/math/Vector2.cpp
    src/core/math/Vector3.cpp
)

target_compile_features(microsw_math PUBLIC cxx_std_20)
set_target_properties(microsw_math PROPERTIES
    CXX_EXTENSIONS OFF
)
target_include_directories(microsw_math PUBLIC src)

add_library(microsw_viewer STATIC
    src/viewer/OrbitCamera.cpp
    src/viewer/OrbitNavigation.cpp
    src/viewer/PanZoomNavigation.cpp
    src/viewer/ProjectionState.cpp
    src/viewer/ReferenceAxes.cpp
    src/viewer/ReferenceGrid.cpp
    src/viewer/ViewProjection.cpp
    src/viewer/WorkspaceViewport.cpp
)

target_compile_features(microsw_viewer PUBLIC cxx_std_20)
set_target_properties(microsw_viewer PROPERTIES
    CXX_EXTENSIONS OFF
)
target_include_directories(microsw_viewer PUBLIC src)
target_link_libraries(microsw_viewer PUBLIC microsw_math)

add_library(microsw_windowing STATIC
    src/app/window/ApplicationWindow.cpp
)

target_compile_features(microsw_windowing PUBLIC cxx_std_20)
set_target_properties(microsw_windowing PROPERTIES
    CXX_EXTENSIONS OFF
)
target_include_directories(microsw_windowing PUBLIC src)
target_link_libraries(microsw_windowing PRIVATE
    glfw
    microsw_logging
)

add_library(microsw_rendering STATIC
    src/rendering/LineRenderer.cpp
    src/rendering/OpenGLContext.cpp
    src/rendering/ShaderProgram.cpp
)

target_compile_features(microsw_rendering PUBLIC cxx_std_20)
set_target_properties(microsw_rendering PROPERTIES
    CXX_EXTENSIONS OFF
)
target_include_directories(microsw_rendering PUBLIC src)
target_link_libraries(microsw_rendering PUBLIC microsw_math)
target_link_libraries(microsw_rendering PRIVATE
    glad_gl_core_33
    microsw_logging
    microsw_windowing
)

# Viewport integration depends on rendering, never the reverse.
target_link_libraries(microsw_viewer PRIVATE microsw_rendering glad_gl_core_33)

add_library(microsw_ui STATIC
    src/ui/ApplicationShell.cpp
    src/ui/ImGuiLayer.cpp
)

target_compile_features(microsw_ui PUBLIC cxx_std_20)
set_target_properties(microsw_ui PROPERTIES
    CXX_EXTENSIONS OFF
)
target_include_directories(microsw_ui PUBLIC src)
target_link_libraries(microsw_ui PRIVATE
    glfw
    microsw_imgui_backend
    microsw_logging
    microsw_rendering
    microsw_windowing
)

add_executable(micro_solidworks
    src/app/main.cpp
)

target_compile_features(micro_solidworks PRIVATE cxx_std_20)
set_target_properties(micro_solidworks PROPERTIES
    CXX_EXTENSIONS OFF
)
target_link_libraries(micro_solidworks PRIVATE
    microsw_viewer
    microsw_logging
    microsw_rendering
    microsw_ui
    microsw_windowing
)

if(BUILD_TESTING)
    if(POLICY CMP0135)
        cmake_policy(SET CMP0135 NEW)
    endif()

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        URL_HASH SHA256=1f357c27ca988c3f7c6b4bf68a9395005ac6761f034046e9dde0896e3aba00e4
    )

    if(MSVC)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif()

    FetchContent_MakeAvailable(googletest)

    add_executable(micro_solidworks_tests
        tests/test_infrastructure.cpp
        tests/test_logging.cpp
        tests/math/test_math_integration.cpp
        tests/math/test_matrix3.cpp
        tests/math/test_matrix4.cpp
        tests/math/test_tolerance.cpp
        tests/math/test_transformations.cpp
        tests/math/test_vector2.cpp
        tests/math/test_vector3.cpp
        tests/viewer/test_orbit_camera.cpp
        tests/viewer/test_orbit_navigation.cpp
        tests/viewer/test_pan_zoom_navigation.cpp
        tests/viewer/test_projection_state.cpp
        tests/viewer/test_reference_axes.cpp
        tests/viewer/test_reference_grid.cpp
        tests/viewer/test_view_projection.cpp
        tests/viewer/test_workspace_viewport.cpp
        tests/rendering/test_shader_program.cpp
        tests/rendering/test_line_renderer.cpp
    )

    target_compile_features(micro_solidworks_tests PRIVATE cxx_std_20)
    set_target_properties(micro_solidworks_tests PROPERTIES
        CXX_EXTENSIONS OFF
    )
    target_link_libraries(micro_solidworks_tests PRIVATE
        GTest::gtest_main
        microsw_logging
        microsw_math
        microsw_viewer
        microsw_rendering
        microsw_windowing
        glad_gl_core_33
        glfw
    )

    include(GoogleTest)
    gtest_discover_tests(micro_solidworks_tests)
endif()
