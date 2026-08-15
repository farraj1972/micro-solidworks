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

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(spdlog)
FetchContent_MakeAvailable(glfw)

add_library(microsw_logging STATIC
    src/app/logging/Logger.cpp
)

target_compile_features(microsw_logging PUBLIC cxx_std_20)
set_target_properties(microsw_logging PROPERTIES
    CXX_EXTENSIONS OFF
)
target_include_directories(microsw_logging PUBLIC src)
target_link_libraries(microsw_logging PRIVATE spdlog::spdlog)

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

add_executable(micro_solidworks
    src/app/main.cpp
)

target_compile_features(micro_solidworks PRIVATE cxx_std_20)
set_target_properties(micro_solidworks PROPERTIES
    CXX_EXTENSIONS OFF
)
target_link_libraries(micro_solidworks PRIVATE
    microsw_logging
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
    )

    target_compile_features(micro_solidworks_tests PRIVATE cxx_std_20)
    set_target_properties(micro_solidworks_tests PROPERTIES
        CXX_EXTENSIONS OFF
    )
    target_link_libraries(micro_solidworks_tests PRIVATE
        GTest::gtest_main
        microsw_logging
    )

    include(GoogleTest)
    gtest_discover_tests(micro_solidworks_tests)
endif()
