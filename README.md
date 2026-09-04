# Micro SolidWorks

Micro SolidWorks is an educational project for progressively learning and
implementing the foundations of a small 3D parametric CAD application in C++.
It is not intended to provide compatibility with the commercial SolidWorks
product.

## Stable baselines

**B0 — Foundation** is FROZEN (tag: `b0-foundation`). The repository provides the executable B0
foundation and application shell; CAD functionality is not implemented yet.

**B1 — Mathematical Foundation** is **FROZEN**
(tag: `b1-mathematical-foundation`). It contains Scalar/numeric tolerance,
Vector2, Vector3, Matrix3, Matrix4, Transformation Operations and mathematical
integration tests, described in [ARCHITECTURE.md](docs/ARCHITECTURE.md).
B1.1–B1.10 are complete.

## Latest stable baseline

The current latest stable baseline is **B2 — 3D Viewer**, **STATUS: FROZEN**
(tag: `b2-3d-viewer`). B2.1–B2.13 are COMPLETE; B2.FREEZE is FROZEN.

Stable capabilities: a HiDPI-aware directly rendered 3D Workspace, Perspective and
Orthographic modes, a finite XY reference grid, RGB reference axes, and
Orbit / Pan / Zoom navigation. These are viewer aids, not CAD entities;
CAD modelling has not started; picking and selection are not implemented.

Controls: MMB drag orbits, Shift+MMB pans, and the mouse wheel zooms.
Use **View → Projection → Perspective / Orthographic** to change mode.
Each mode preserves its independent zoom state.

The viewer decision gate **D2 — 3D Viewer Conventions** remains
**FROZEN**. Its accepted decisions are recorded in
[`ADR-0010`](docs/adr/ADR-0010-viewer-camera-and-navigation.md),
[`ADR-0011`](docs/adr/ADR-0011-view-and-projection-conventions.md), and
[`ADR-0012`](docs/adr/ADR-0012-viewer-rendering-pipeline.md).
D0 and D1 remain FROZEN.

The current decision gate is **D3 — Geometric Foundation**, **FROZEN**.
Its accepted decisions are
[`ADR-0013`](docs/adr/ADR-0013-geometric-point-and-vector-semantics.md),
[`ADR-0014`](docs/adr/ADR-0014-geometric-primitive-representation.md),
[`ADR-0015`](docs/adr/ADR-0015-geometric-tolerance-and-degeneracy.md) and
[`ADR-0016`](docs/adr/ADR-0016-geometry-topology-cad-boundaries.md).

## Current baseline

**B3 — Geometric Primitives** is **IN PROGRESS**.
B3.1–B3.8 are COMPLETE / ACCEPTED. The current increment is
**B3.9 — Documentation & D3 Validation**.
B3.10 — Baseline Validation and B3.FREEZE remain PENDING and each requires
explicit authorization. B3 is not frozen.

The project-owned `microsw_geometry` library provides Point2/3, Segment2/3,
Line2/3, Ray2/3 and Plane, with geometric predicates and point-to-primitive
distance/projection operations. It depends only on internal Math and the C++
standard library. Geometry is tested but is not yet consumed by the application
executable or rendered by the Viewer. Topology, CAD modelling, intersections,
primitive equivalence and primitive-to-primitive metrics are not implemented.
See [ARCHITECTURE.md](docs/ARCHITECTURE.md) for APIs, tolerance and boundaries.

Every subsequent increment, Decision Gate, baseline or freeze requires
explicit authorization.

The approved technology foundation is C++20, CMake, GoogleTest with CTest,
GLFW, OpenGL, and Dear ImGui. Dependencies are introduced only in the increment
that requires them.

## Build

```sh
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

B3.8 validation snapshot: **533 tests, 533 PASS, 0 FAIL**, including 31 geometric
integration tests, plus application runtime smoke (startup, visible viewer,
normal close, exit code 0). B2.13's frozen snapshot was 274/274 tests PASS.
Tests include real OpenGL contexts and require a working graphics environment;
these counts are snapshots, not fixed future totals.

Project operating rules are defined in [`AGENTS.md`](AGENTS.md), with bootstrap
guidance in [`BOOTSTRAP.md`](BOOTSTRAP.md). Architectural and project
documentation is available under [`docs/`](docs/), including the accepted
decision records in [`docs/adr/`](docs/adr/).
