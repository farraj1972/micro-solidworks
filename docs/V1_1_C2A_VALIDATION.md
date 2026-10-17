# V1.1-C2A — Feature Manager & Display Mode UX Alignment Validation

## Scope and D16 conformance

V1.1-C2A implements the accepted ADR-0059–0060 contract. The Feature Manager is
a derived CAD narrative over `FeatureModel`; it does not own model state or
change identity, dependency or rebuild order. Solid display modes are explicit
Viewer passes over Presentation-derived faces and topology edges.

- ADR-0059: CONFORMANT — `Part1` is the root, Extrude absorbs its direct source
  Sketch, unconsumed Sketches stay at the root, shared Sketches use the first
  direct Extrude consumer, and Boolean remains a root modeling feature.
- ADR-0060: CONFORMANT — Shaded draws faces, Shaded With Edges adds topology
  edges, and Wireframe draws topology edges without face fill.
- Architectural deviations: none.

## Feature Manager

`FeatureTreeModel` derives stable ordered nodes with human-readable names,
feature kind, rebuild state, display state and active-edit state. CLEAN remains
visually quiet; hidden, active, dirty, failed and blocked states use contextual
cues. Single click synchronizes logical selection with the Graphics Area.
Double click enters Sketch editing or opens Extrude properties where supported.
Context actions expose Edit, Extrude, Show/Hide and Boolean inspection without
requiring internal IDs.

## Display modes and corrective finding

The initial Experience Gate found that Shaded still showed solid edges whenever
the owning feature was selected. The root cause was the legacy wireframe visual
proxy: its normal state was suppressed, but its selected state still redrew the
complete solid wireframe after the explicit face pass.

The correction suppresses that proxy in every visual state. It remains only an
identity mapping; explicit solid passes exclusively draw faces, topology edges
and face/edge hover or selection. Shaded now contains lit faces without the
global overlay, Shaded With Edges adds actual topology edges, and Wireframe has
topology edges without fill. Curved faces do not expose tessellation triangles;
real circular seams and hole boundaries remain topology edges.

## Technical Gate

Validation level: V3, followed by targeted V1 validation of the Experience Gate
correction.

- Fresh configure: PASS (`build-v1.1-c2a-final`).
- Full Debug build: PASS.
- Project/dependency/linker warnings: 0.
- Full CTest: PASS, 837/837.
- Pre-V3 affected closure: PASS, 66/66.
- Corrective incremental application/test build: PASS.
- Corrective Viewer/display/picking closure: PASS, 20/20.
- `git diff --check`: PASS.
- Architecture/dependency audit: PASS; no new dependency or reverse dependency.

The full V3 was not repeated after the localized Viewer correction. The changed
dependency closure was rebuilt and its 20 directly affected tests passed, in
accordance with impact-based validation.

## Runtime and Experience Gate

Manual runtime validation covered the Feature Manager narrative, absorbed and
unconsumed Sketches, selection and viewport synchronization, context and
double-click interactions, box/cylinder/hole/multi-body display, the three
display modes, topology-only edge overlays, curved surfaces, hover/selection,
visibility and normal shutdown.

The first display-mode observation required correction. The repeated manual
gate confirmed Shaded without global edges, Shaded With Edges with topology
edges, Wireframe without face fill, and correct hover/selection feedback.
Experience Gate: APPROVED.

## Result

V1.1-C2A is FROZEN. V1.1-C3 — Document Safety, Undo/Redo & Session Workflow is
the next active campaign.
