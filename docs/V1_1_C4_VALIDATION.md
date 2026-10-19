# V1.1-C4 — Subtractive Modeling & Sketch-on-Face Validation

## Final result

```text
STATUS: FROZEN
Technical Gate: PASS
Experience Gate: APPROVED
D18 / D18-R1 / D19 / D20: FROZEN / CONFORMANT
```

V1.1-C4 implements semantic Sketch support on generated planar faces, exact
axial Cut Extrude, persistence v2, arbitrary Normal To orientation, feature
deletion and the approved CAD interaction corrections.

`CutExtrudeFeature` owns its source Sketch, target result Feature, positive
Blind depth and Forward/Reverse direction. The project-owned exact kernel uses
2D analytic region subtraction plus axial interval decomposition and rebuilds
Plane/Cylinder topology without deriving CAD state from tessellation. It
supports blind pockets and holes, concave and holed cutters, multiple cutter
regions, full removal, split results and multi-body targets. Unsupported
oblique/conic cases fail transactionally.

Generated-face Sketch support remains semantic. Single-body Extrude start/end
caps, inherited Cut caps and an unambiguous single-region pocket floor resolve
to derived SketchPlanes. Raw body and topology IDs remain rebuild-local.
Missing or ambiguous roles fail without geometric reattachment.

Feature deletion removes the selected Feature and its complete transitive
dependant closure atomically. Non-leaf deletion names the affected Features for
confirmation. Deleted IDs remain retired; Undo/Redo restores or removes the
exact closure, and persistence retains the advanced next-ID counter.

Persistence writes `.microsw` version 2 and migrates version 1 without rewriting
on Open. Authoritative semantic support and Cut parameters roundtrip; derived
topology, Presentation and editor/session state remain unpersisted.

## Validation

Validation level: V3.

```text
fresh configure: PASS
full Debug build: PASS
project warnings: 0
full CTest: 899/899 PASS
runtime startup: PASS
native close: PASS
exit code: 0
architecture/dependency audit: PASS
git diff --check: PASS
```

The first full CTest execution passed 897/899. The two failures were historical
tests that still required the pre-correction MMB orbit sign, while the approved
application behavior follows the documented direct-manipulation direction.
Only those expectations changed. Their focused rerun passed 2/2, followed by
the final complete 899/899 CTest pass. No executable implementation changed
between the approved Experience Gate and the final runtime smoke.

The fresh configure initially could not download the already-approved spdlog
source because sandbox network access was blocked. Repeating the same configure
with authorized network access succeeded; no dependency or version changed.

## Experience Gate

Experience Gate: APPROVED by the project owner.

The manual campaign covered:

- principal-plane and generated-face Sketch creation with deterministic Normal To;
- projection-consistent Line, Rectangle and Circle creation and preview;
- exact Blind Cut Extrude, Reverse Direction and associative depth changes;
- pocket-floor Sketch support and readable failure for missing/ambiguous roles;
- leaf and dependant-closure deletion with atomic Undo/Redo;
- human-readable FeatureManager selection and contextual commands;
- persistent sketch dimensions without competing geometry/constraint inputs;
- subdued visible Sketch references with Show/Hide;
- dependant solids hidden while their owning Sketch is edited;
- inactive Extrude/Cut parameters after command completion;
- display, picking, navigation, direct-manipulation MMB orbit and clean exit.

## Architecture audit

```text
ADR-0064: CONFORMANT
ADR-0065 + D18-R1: CONFORMANT
ADR-0066: CONFORMANT
ADR-0067: CONFORMANT
ADR-0068: CONFORMANT
ADR-0069: CONFORMANT
```

CAD and FeatureModel state remain authoritative; Presentation and rendering
remain derived. Dependency direction remains Features → Modeling/Topology/
Geometry/Math, Persistence → Features, and Application/UI → Features/
Interaction/Presentation. No external kernel, generic naming framework,
persistent FaceId/BodyId, mesh Boolean or schema v3 was introduced.

## Deliberate limits

- exact face-normal/axial Cut Extrude and Blind end condition only;
- generated support limited to the accepted unambiguous semantic roles;
- oblique Cut Extrude, conic intersection topology, Through All, advanced end
  conditions, general persistent topological naming and general scene graph
  remain deferred.

V1.1-C4 is FROZEN. The next roadmap item is V1.1-C5 — Feature Workflow
Hardening, subject to prerequisite review and explicit authorization.