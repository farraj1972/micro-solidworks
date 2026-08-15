cmake_minimum_required(VERSION 3.20)

project(MicroSolidWorks LANGUAGES CXX)

add_executable(micro_solidworks
    src/app/main.cpp
)

target_compile_features(micro_solidworks PRIVATE cxx_std_20)
set_target_properties(micro_solidworks PROPERTIES
    CXX_EXTENSIONS OFF
)

