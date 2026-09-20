# B10 — Extrusion Baseline Validation

Status: PASS

Validated executable HEAD: `9bbea2a4321a28f54362bc5c000fa3213ce8bbe4`

## V3 evidence

- Clean configure: PASS. The sandboxed configure could not access the declared
  FetchContent repositories; repeating the same configure with network access
  populated the dependencies and passed.
- Full Debug build: PASS.
- Project, dependency and linker warnings: 0.
- Full CTest: PASS, 767/767 tests, 0 failed.
- Runtime: PASS by manual observation. A solved closed rectangle was extruded
  with a positive distance; the wireframe Solid was visible and coherent under
  hover, picking, selection and whole-Solid highlight. Explicit dimension edits,
  solve and manual regeneration updated the modeled result. Invalid extrusion
  input preserved the previous valid Solid. Viewer navigation, projection,
  resize, UI/About and normal shutdown were confirmed.
- Architecture/dependency audit: PASS. `microsw_modeling` depends only on
  `microsw_topology`, which retains the approved Geometry/Math direction. The
  Sketch-to-Profile and active-result integration remain outside Modeling core;
  Presentation derives unique topology edges without reverse dependencies.
- Repository hygiene: PASS. No build output is tracked.
- `git diff --check`: PASS.

## D9 conformance

- ADR-0035: CONFORMANT — Profile is an ephemeral strictly-convex CCW world-space
  boundary on an oriented Plane; Sketch extraction is a separate adapter using
  live line geometry and geometric endpoint tolerance.
- ADR-0036: CONFORMANT — positive one-shot extrusion produces shared manifold
  topology with `2N` vertices, `3N` edges, `N+2` outward faces and one root Shell.
- ADR-0037: CONFORMANT — Application owns one replaceable active Solid; rejected
  regeneration preserves the prior result, while one stable presentation identity
  drives the same wireframe segments through rendering and picking.

GAP-GEO-002 and GAP-SCENE-002 remain OPEN / deferred. No architecture deviation
or B10 baseline finding remains open.
