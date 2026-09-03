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
B2 is authorized for planning; implementation requires explicit increment authorization.

The current decision gate is **D2 — 3D Viewer Conventions**, with status
**FROZEN**. Its accepted decisions are recorded in
[`ADR-0010`](docs/adr/ADR-0010-viewer-camera-and-navigation.md),
[`ADR-0011`](docs/adr/ADR-0011-view-and-projection-conventions.md), and
[`ADR-0012`](docs/adr/ADR-0012-viewer-rendering-pipeline.md).
D1 remains FROZEN. Camera and viewer functionality are not implemented yet.

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
