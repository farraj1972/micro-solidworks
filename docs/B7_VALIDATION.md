# B7 — Topological Model Validation

Status: PASS

Validated executable commit: `18b2f0d`

## Scope

B7 adds the project-owned `microsw_topology` module and the first educational
topological Solid: shared indexed Vertex and Edge records, oriented closed
Wires, convex planar Faces, an oriented connected closed 2-manifold Shell and
one root Shell per Solid. The deterministic validation cuboid contains 8
Vertices, 12 Edges, 6 Wires, 6 Faces and 1 Shell.

## Validation

- Level: V3 — Baseline Validation.
- Clean configure: PASS in a new `build-b7-validation` directory.
- Full Debug build: PASS.
- Project/dependency/linker warnings: 0.
- Full CTest: 719/719 PASS, 0 failed.
- Runtime smoke: PASS. The clean-build application initialized GLFW, OpenGL
  3.3 and Dear ImGui, displayed its existing workspace, remained stable and was
  closed on the user's instruction. Topology has no runtime UI integration.
- Architecture audit: PASS.
- Dependency audit: PASS. The implemented edge is
  `microsw_topology -> microsw_geometry -> microsw_math`; no Presentation,
  Viewer, Rendering, UI or Application dependency enters Topology.
- Repository hygiene: PASS after removing the temporary validation build.
- `git diff --check`: PASS.

The first clean configure attempt inside the restricted sandbox could not use
Git's HTTPS remote helper while populating pinned `spdlog`. Repeating the same
configuration with authorized external access succeeded. This was an
environment-only retry and did not require a source or dependency change.

## D6 Conformance

- ADR-0025 — CONFORMANT.
- ADR-0026 — CONFORMANT.
- ADR-0027 — CONFORMANT.

`GAP-GEO-001`, `GAP-GEO-002` and `GAP-SCENE-002` remain OPEN / deferred.

## Result

B7 baseline validation: PASS. The executable tree did not change after this V3;
the remaining freeze synchronization is documentation-only and uses V0.
