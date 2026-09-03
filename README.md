# Micro SolidWorks

Micro SolidWorks is an educational project for progressively learning and
implementing the foundations of a small 3D parametric CAD application in C++.
It is not intended to provide compatibility with the commercial SolidWorks
product.

## Stable baselines

**B0 — Foundation** is FROZEN (tag: `b0-foundation`). The repository provides the executable B0
foundation and application shell; CAD functionality is not implemented yet.

The current stable baseline is **B1 — Mathematical Foundation**, **FROZEN**
(tag: `b1-mathematical-foundation`). It contains Scalar/numeric tolerance,
Vector2, Vector3, Matrix3, Matrix4, Transformation Operations and mathematical
integration tests, described in [ARCHITECTURE.md](docs/ARCHITECTURE.md).
B1.1–B1.10 are complete.

Next baseline: **B2 — 3D Viewer**, **NOT STARTED**.
B2 planning / Decision Gate requires explicit authorization.

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
