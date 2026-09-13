# B8 — Sketcher Baseline Validation

Status: PASS

Validated executable HEAD: `522dfb6d4d18bc25c2fdff16efcd48f866ede64f`

## V3 evidence

- Clean configure: PASS. The sandboxed attempt could not invoke Git's HTTPS
  helper for FetchContent; repeating the same configure with network access
  populated the pinned dependencies and passed.
- Full Debug build: PASS.
- Project, dependency and linker warnings: 0.
- Full CTest: PASS, 746/746 tests, 0 failed.
- Runtime: PASS by manual observation. Line, Rectangle, Circle and Arc creation;
  selection, editing, deletion, rendering, picking, hover and highlight;
  orbit, pan, zoom, Perspective/Orthographic, resize, UI/About and normal
  shutdown were confirmed. The process exited normally.
- Automated HiDPI coverage: PASS in the full suite. Manual HiDPI was not
  required by the campaign.
- Architecture/dependency audit: PASS. `microsw_sketch` depends publicly only
  on `microsw_geometry`, which depends on `microsw_math`; Sketch core contains
  no Topology, Presentation, Viewer, Rendering, UI or Application includes.
- Repository hygiene: PASS. No build output is tracked.
- `git diff --check`: PASS.

## D7 conformance

- ADR-0028: CONFORMANT — authoritative local 2D geometry and deterministic
  right-handed SketchPlane conversions.
- ADR-0029: CONFORMANT — Sketch ownership, stable local IDs, atomic replacement,
  tombstone removal, non-reuse and four-line rectangle creation.
- ADR-0030: CONFORMANT — Circle2/Arc2 validity, radians, signed sweep and query
  semantics.
- ADR-0031: CONFORMANT — derived world presentation, distinct stable identity
  mapping and shared curve representation for rendering and picking.

## Gap state

- GAP-GEO-001: CLOSED by implemented and validated Circle2.
- GAP-GEO-002: OPEN / deferred.
- GAP-SCENE-002: OPEN / deferred.

No architecture deviation or baseline finding remains open.
