# Micro SolidWorks

Micro SolidWorks is an educational project for progressively learning and
implementing the foundations of a small 3D parametric CAD application in C++.
It is not intended to provide compatibility with the commercial SolidWorks
product.

## Stable baselines

| Baseline | Status | Tag |
| --- | --- | --- |
| B0 — Foundation | FROZEN | `b0-foundation` |
| B1 — Mathematical Foundation | FROZEN | `b1-mathematical-foundation` |
| B2 — 3D Viewer | FROZEN | `b2-3d-viewer` |
| B3 — Geometric Primitives | FROZEN | `b3-geometric-primitives` |
| B4 — Geometry Visualization & Selection | FROZEN | — |

B4 is the latest stable baseline. B3 supplies Point2/3, Segment2/3,
Line2/3, Ray2/3 and Plane, queries, metrics and geometric tolerance over internal
Math. Geometry remains model-only, independent of graphics and UI.
Point3 is not a Vertex, Segment3 is not an Edge, and Plane is not a Face.

## Frozen B4 — Geometry Visualization & Selection

**Status: FROZEN**.

- B4.1–B4.12: COMPLETE.
- B4.FREEZE: FROZEN.
- B4.12 MINOR documentary state finding: CLOSED by B4.FREEZE.

The application now demonstrates a deterministic temporary collection of
3 Point3, 3 Segment3 and 2 Line3, with geometric picking, automatic hover,
single selection and visual highlighting. Existing capabilities include a
HiDPI-aware 3D Workspace, finite XY grid, RGB axes, orbit/pan/zoom and
Perspective/Orthographic projection.

Normal points are yellow, segments light gray and lines cyan. Hover is light
cyan; selection is orange and takes precedence over hover. Selection persists
through cursor movement, navigation, projection changes and resize/minimize.
Hover and selection can reference different entities simultaneously.

Line3 remains mathematically infinite; the Viewer derives a finite segment
from the current view center and visible scale. Picking uses this same finite
representation and a 6 logical-pixel tolerance. Exact occlusion/depth-buffer-aware
picking is not implemented; highlight remains subject to normal depth occlusion.

The application owns GeometryPresentation; WorkspaceViewport observes it.
Visual IDs are process-local, collection-scoped and non-persistent. This demo
is not a CAD Document, scene graph or persistent model. Details of ownership,
dependencies, adapters and interaction are in [ARCHITECTURE.md](docs/ARCHITECTURE.md).

## Controls

| Input | Action |
| --- | --- |
| MMB drag | Orbit |
| Shift + MMB drag | Pan |
| Mouse wheel | Zoom |
| Pointer over geometry | Automatic hover; suspended during MMB navigation |
| Left click geometry | Select one entity; another click replaces it |
| Left click empty Workspace | Clear selection |
| View → Projection | Perspective / Orthographic; each preserves its zoom state |
| Help → About | Application information |
| File → Exit or native X | Close the application |

Re-clicking a selected entity preserves it. Clicks outside the Workspace or
blocked by UI/modal interaction preserve selection. UI does not implement picking.
The About caption retains its historical B0 foundation text.

## Decisions and deferred scope

D0/D1/D2/D3/D4 remain **FROZEN**; ADR-0001–0020 remain **20/20 ACCEPTED**.
The latest gate, D4, is documented in [ADR-0017](docs/adr/ADR-0017-geometry-presentation-boundary.md),
[ADR-0018](docs/adr/ADR-0018-visual-entity-identity-and-selection-state.md),
[ADR-0019](docs/adr/ADR-0019-geometry-picking-strategy.md) and
[ADR-0020](docs/adr/ADR-0020-geometry-visualization-scope.md).

Ray3/Plane visualization, multi-selection/box/lasso, framebuffer and exact
occlusion-aware picking, scene graph/ECS, persistent IDs/serialization/Document,
Topology/BRep, Sketching/constraints/dimensions and feature modeling
(Extrude/Revolve/Boolean/history/regeneration) remain deferred.

No next increment, Decision Gate or baseline has been authorized. Changes to
frozen B4 behavior require explicit authorization. B5 has not started.

## Build and validation

The approved stack remains C++20, CMake, GoogleTest/CTest, GLFW, OpenGL and
Dear ImGui, with internal Math and Geometry; B4 introduces no external dependency.

```sh
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
git diff --check
```

Freeze validation snapshot (B4.12): **684/684 PASS, 0 FAIL**. Coverage includes
Presentation, adapters, renderers, picking, hover, selection, highlight batching
and 14 Viewer/Geometry integration tests with real OpenGL and no pixel assertions.
Runtime smoke passed with native X and File → Exit both returning 0. HiDPI was
validated automatically; manual B4.12 HiDPI validation was unavailable/not
repeated. Frozen historical snapshots remain B3.10: 549/549 and B2.13: 274/274.

B4.12 clean configure and Debug build passed with zero project, dependency
or linker warnings. ADR-0017–0020 are ACCEPTED / CONFORMANT. Freeze changes
only documentation; production, tests, CMake, dependencies and ADRs are unchanged.

Tests require a working OpenGL 3.3 graphics environment. Counts are validation
snapshots, not permanent totals. See [AGENTS.md](AGENTS.md), [BOOTSTRAP.md](BOOTSTRAP.md),
[ROADMAP.md](docs/ROADMAP.md) and [DEPENDENCY_POLICY.md](docs/DEPENDENCY_POLICY.md)
for governance, increment state and dependency strategy.
