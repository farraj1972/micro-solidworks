# Micro SolidWorks

Micro SolidWorks is an educational project for progressively learning and
implementing the foundations of a small 3D parametric CAD application in C++.
It is not intended to provide compatibility with the commercial SolidWorks
product.

## Current stable baseline

**B0 — Foundation** is frozen. The repository provides the executable B0
foundation and application shell; CAD functionality is not implemented yet.

The current baseline is **B1 — Mathematical Foundation**, **IN PROGRESS**.
B1.1–B1.8 are complete. The current increment is
**B1.9 — Documentation & D1 ADR Validation**; the implemented Math foundation
is described in [ARCHITECTURE.md](docs/ARCHITECTURE.md).
B1.10 — Baseline Validation is the next pending quality gate and requires
separate explicit authorization. B1 is not frozen.

The current decision gate is **D1 — Mathematical Conventions**, with status
**FROZEN**. Its accepted decisions are recorded in
[`ADR-0007`](docs/adr/ADR-0007-mathematical-scalar-and-tolerance.md),
[`ADR-0008`](docs/adr/ADR-0008-coordinate-and-unit-conventions.md), and
[`ADR-0009`](docs/adr/ADR-0009-matrix-and-transform-conventions.md).

The approved technology foundation is C++20, CMake, GoogleTest with CTest,
GLFW, OpenGL, and Dear ImGui. Dependencies are introduced only in the increment
that requires them.

## Build

```sh
cmake -S . -B build
cmake --build build
```

Project operating rules are defined in [`AGENTS.md`](AGENTS.md), with bootstrap
guidance in [`BOOTSTRAP.md`](BOOTSTRAP.md). Architectural and project
documentation is available under [`docs/`](docs/), including the accepted
decision records in [`docs/adr/`](docs/adr/).
