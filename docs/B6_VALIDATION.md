# B6 — Transformations validation

Status: FROZEN. B6.1–B6.10 COMPLETE; B6.FREEZE FROZEN.

## Implemented scope

B6.1–B6.2 supply Transform3 and Presentation value ownership. B6.3 derives
world Point/Segment/Line without changing local Geometry. B6.4–B6.5 share those
world values between CPU draw adapters and picking. B6.6–B6.8 provide the
selected-entity editor, atomic edits, interaction coherence and real OpenGL
integration coverage. No hierarchy, pivot, quaternion, Document, persistence,
Topology, Circle or general intersections are introduced.

## D5 conformance

| ADR | Result | Implementation evidence |
| --- | --- | --- |
| 0021 | CONFORMANT | VisualEntity owns Transform3 by value; local PresentedGeometry and ID are preserved; Geometry has no transform state |
| 0022 | CONFORMANT | Transform3 derives T * R * S; worldGeometry uses point/direction operations; no authoritative matrix or hierarchy |
| 0023 | CONFORMANT | Rz * Ry * Rx, radians in Math, degrees in UI, finite positive per-axis scale, local-origin pivot, atomic rejection |
| 0024 | CONFORMANT | Shared world derivation for drawing/picking; Line normalized before finite view extent; existing selection ID and highlight precedence |

Rendering uses CPU-transformed world vertices with identity uModel, an option
allowed by ADR-0024 (uModel = TRS was a preference, not a requirement). Renderer
types and shaders remain generic and do not know visual IDs or selection.

## Development validation

- B6.3 V1: incremental Presentation/test build; 14 focused tests PASS.
- B6.4–B6.5 V2: 49 relevant Presentation/adapter/picking/OpenGL tests PASS.
- B6.6–B6.8 V2: 29 interaction/integration tests PASS before UI smoke correction.
- Manual editor smoke initially found an ImGui assertion: InputScalar does not
  support EnterReturnsTrue. Local UI correction removes that flag and applies
  valid edits immediately. The real selected-entity editor draw test passes for
  Point, Segment and Line (1/1). Corrected manual smoke passed with exit code 0.
  This local implementation finding is CLOSED; no open architectural findings.
- Native capture failed twice with SetIsBorderRequired / 0x80004002; manual
  runtime observations are required. This is an environment limitation.

## B6.10 baseline validation

PASS at source HEAD `4e53fd40268c9e4b0ccfb3935b4c047fb77c16bc`:

- Clean configure into previously absent `build/b6-validation`: PASS. Existing
  pinned dependency sources reused, with fresh build artifacts and configuration.
- Full Debug build: PASS; zero warnings/errors in the complete build log.
- Full CTest: 708/708 PASS, 0 failed (474.67 seconds, two test processes).
- Manual clean-build runtime: PASS for Point/Segment/Line TRS, picking, hover,
  selection/highlight, grid/axes, navigation, projections, resize and UI/About.
  Normal shutdown returned 0. Manual HiDPI was not explicitly confirmed;
  automated HiDPI scenarios passed in the full suite.
- Architecture/dependency audit: PASS. No source changes to Geometry, Rendering
  or accepted ADRs; no new external dependencies. UI -> Presentation and
  Presentation -> Geometry -> Math remain acyclic; Viewer keeps GPU range checks.
- Repository hygiene and `git diff --check`: PASS; generated files stay in build/.

Local evidence: `build/b6-v3-configure.log`, `build/b6-v3-build.log`,
`build/b6-v3-ctest.log`, `build/b6-v3-runtime.log` and `build/b6-v3-runtime.exit`.

## Freeze

V0 only: documentation/state review, unchanged executable source versus the
validated HEAD, diff check and clean committed tree. No repeated V3 after these
documentation-only changes. Tag: `b6-transformations`.
B7 is not started; it requires prerequisite review and explicit authorization.
