# Micro SolidWorks

Micro SolidWorks is an educational project for progressively learning and
implementing the foundations of a small 3D parametric CAD application in C++.
It is not intended to provide compatibility with the commercial SolidWorks
product.

## Current baseline

Development is currently in **B0 — Foundation**. The repository contains only
the initial C++20 and CMake application foundation; CAD functionality is not
implemented yet.

The approved technology foundation is C++20, CMake, GoogleTest with CTest,
GLFW, OpenGL, and Dear ImGui. Dependencies are introduced only in the increment
that requires them.

## Build

```sh
cmake -S . -B build
cmake --build build
```

Project operating rules are defined in [`AGENTS.md`](AGENTS.md). Architectural
and project documentation is available under [`docs/`](docs/).
