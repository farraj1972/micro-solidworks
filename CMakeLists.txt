cmake_minimum_required(VERSION 3.20)

project(MicroSolidWorks LANGUAGES CXX)

include(CTest)

add_executable(micro_solidworks
    src/app/main.cpp
)

target_compile_features(micro_solidworks PRIVATE cxx_std_20)
set_target_properties(micro_solidworks PROPERTIES
    CXX_EXTENSIONS OFF
)

if(BUILD_TESTING)
    include(FetchContent)

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
    )

    target_compile_features(micro_solidworks_tests PRIVATE cxx_std_20)
    set_target_properties(micro_solidworks_tests PROPERTIES
        CXX_EXTENSIONS OFF
    )
    target_link_libraries(micro_solidworks_tests PRIVATE GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(micro_solidworks_tests)
endif()
