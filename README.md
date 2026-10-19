# Micro SolidWorks

Micro SolidWorks is a C++ 3D parametric CAD application. From V1.1-C5 onward,
its approved product vision is **Personal SolidWorks**: progressively replace
well-defined personal mechanical-engineering workflows that otherwise require
a commercial CAD licence. Product reliability and complete workflows guide
future prioritization; the educational foundations remain part of the project.
This vision does not claim file or API compatibility with commercial SolidWorks.

## Current release

**Micro SolidWorks v1.1.0 — FROZEN.** The release combines a CAD-first
workspace, guided sketching with constraints and dimensions, general additive
Extrude, solid shading and face/edge selection, Feature Manager hierarchy,
Undo/Redo and document safety, Sketch-on-Face, exact axial Cut Extrude,
persistence v2 with v1 migration, feature deletion, editing, rename and rebuild
diagnostics. Its C5 gate passed 903/903 tests and an approved manual workflow
assessment. See [release validation](docs/V1_1_0_RELEASE.md).

P0 Product & Architecture Review is FROZEN. V1.2 is approved at campaign level,
but its detailed plan is READY FOR REVIEW and implementation is not authorized.
See the [P0 decisions](docs/PERSONAL_SOLIDWORKS_P0_DECISIONS.md),
[v1.2 campaign proposal](docs/PERSONAL_SOLIDWORKS_V1_2_CAMPAIGN_PROPOSAL.md)
and [SPEC-01 CAD/CAM specification](docs/specification/README.md). SPEC-01 is a
DRAFT/PROPOSED reference for future capabilities, not a change to frozen ADRs
or the current implementation roadmap.

## Stable technical baselines

| Baseline | Status | Tag |
| --- | --- | --- |
| B0 — Foundation | FROZEN | `b0-foundation` |
| B1 — Mathematical Foundation | FROZEN | `b1-mathematical-foundation` |
| B2 — 3D Viewer | FROZEN | `b2-3d-viewer` |
| B3 — Geometric Primitives | FROZEN | `b3-geometric-primitives` |
| B4 — Geometry Visualization & Selection | FROZEN | `b4-geometry-visualization-selection` |
| B6 — Transformations | FROZEN | `b6-transformations` |
| B7 — Topological Model | FROZEN | — |
| B8 — Sketcher | FROZEN | `b8-sketcher` |
| B9 — Constraint System | FROZEN | `b9-constraint-system` |
| B10 — Extrusion | FROZEN | `b10-extrusion` |
| B11 — Boolean Operations | FROZEN | `b11-boolean-operations` |
| B12 — Feature Tree | FROZEN | `b12-feature-tree` |
| B13 — Parametric Rebuild | FROZEN | `b13-parametric-rebuild` |
| B14 — Persistence | FROZEN | `b14-persistence` |
| Micro SolidWorks MVP 1.0 | FROZEN | `mvp-1.0` |

B14 is the latest stable baseline and Micro SolidWorks MVP 1.0 is FROZEN.
B3 supplies Point2/3, Segment2/3,
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

## Canonical roadmap and technical baselines

Technical B0–B4 remain FROZEN. Canonical functional progress is distinct:
B0/B1/B2 satisfied; B3/B4 partially satisfied; B5 realized/satisfied by technical
B4; B6 Transformations is FROZEN. No duplicate B5
implementation is required. See [ROADMAP.md](docs/ROADMAP.md) for the fixed
B0–B14 sequence, implementation mapping and Canonical Roadmap Gaps.

**Vertical Slice First**: feature over performance; working vertical slice over
speculative completeness; architecture boundaries over premature abstraction.
Circle is supplied by B8; V1.1-C1 supplies the D14 Line/Arc/Circle intersection
subset, general planar regions and exact curved extrusion. B6 supplies entity
transforms; the C1 editor supplies the first explicit visibility/edit lifecycle.

The B6 slice is select Point/Segment/Line -> edit transform -> updated
rendering/picking -> coherent hover/selection/highlight. D5 — Transformation
Semantics is **FROZEN** in accepted ADR-0021–0024. B6 is **FROZEN** after clean
build, 708/708 tests and runtime PASS. See [B6 validation](docs/B6_VALIDATION.md).

## Controls

| Input | Action |
| --- | --- |
| MMB drag | Orbit |
| Shift + MMB drag | Pan |
| Mouse wheel | Zoom |
| Heads-up Fit | Frame all visible modeled solids |
| Heads-up ISO / View menu | Isometric or Front/Back/Left/Right/Top/Bottom view |
| Heads-up Display / View menu | Shaded With Edges, Wireframe or Shaded |
| Pointer over a solid face/edge | Pre-highlight the logical face or edge |
| Left click solid face/edge | Select it; a nearby edge has priority within 6 logical pixels |
| Pointer over geometry | Automatic hover; suspended during MMB navigation |
| Left click geometry | Select one entity; another click replaces it |
| Left click empty Workspace | Clear selection |
| Selected entity Transform panel | Edit XYZ translation, rotation in degrees and positive scale; valid edits apply immediately |
| View → Projection | Perspective / Orthographic; each preserves its zoom state |
| Help → About | Application information |
| File → Save As | Choose a `.microsw` filename with the native Windows dialog and save atomically |
| File → Save | Save to the current path, or open native Save As for an untitled document |
| File → Open | Choose a `.microsw` file with the native Windows dialog, then load transactionally |
| Ctrl+Z / Ctrl+Y | Undo / Redo the last committed CAD transaction |
| File → New | Protect dirty work, then reset model, path, history and transient interaction state |
| File → Exit or native X | Protect dirty work with Save / Don't Save / Cancel, then close |

The v1.1 CAD workspace adds icon commands and Model/Edit panels. Start a Sketch,
draw Line/Rectangle/Circle geometry with endpoint/midpoint/grid snapping and
horizontal/vertical/coincident inference, then select the Sketch and choose
Extrude. The Edit panel validates the profile, reports region/body count and
accepts a positive depth. Concave profiles, circular/arc boundaries, holes,
nested islands and disjoint regions are supported without internal ID entry.

V1.1-C2 presents every derived solid body with flat-shaded faces and an
independent topology-edge overlay. Face/edge selection synchronizes its owning
Feature without exposing FaceId, EdgeId or runtime body indices. Hidden results
are neither displayed nor pickable; topology rebuild clears sub-element selection.

Re-clicking a selected entity preserves it. Clicks outside the Workspace or
blocked by UI/modal interaction preserve selection. UI does not implement picking.
The About caption retains its historical B0 foundation text.

## Decisions and deferred scope

D0/D1/D2/D3/D4/D5 are **FROZEN**; ADR-0001–0024 are **24/24 ACCEPTED**.
The preceding gate, D4, is documented in [ADR-0017](docs/adr/ADR-0017-geometry-presentation-boundary.md),
[ADR-0018](docs/adr/ADR-0018-visual-entity-identity-and-selection-state.md),
[ADR-0019](docs/adr/ADR-0019-geometry-picking-strategy.md) and
[ADR-0020](docs/adr/ADR-0020-geometry-visualization-scope.md).

Ray3/Plane visualization, multi-selection/box/lasso, framebuffer and exact
occlusion-aware picking, scene graph/ECS, persistent IDs/serialization/Document,
Topology/BRep, Sketching/constraints/dimensions and feature modeling
(Extrude/Revolve/Boolean/history/regeneration) remain deferred.

D5 — Transformation Semantics is frozen in accepted ADR-0021–0024. B6 is
FROZEN. Changes to frozen baseline behavior require
explicit authorization. Canonical B5 functionality is already satisfied by
technical B4.

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
