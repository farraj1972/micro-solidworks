# ADR-0060 — CAD Display Mode Rendering Semantics

Status: ACCEPTED

## Context

V1.1-C2 introduced derived shaded faces and topology-edge presentation, but the
visual distinction between Shaded and Shaded With Edges is not sufficiently
clear. Display mode must control explicit rendering passes while preserving the
D15 rule that topology and analytic Geometry remain authoritative.

## Decision

Display modes have the following rendering semantics:

- **Shaded:** face fill and lighting are enabled; the global topology-edge
  overlay is disabled.
- **Shaded With Edges:** the same face fill and lighting are enabled and the
  explicit topology-edge overlay is enabled.
- **Wireframe:** face fill is disabled and topology edges/curves are enabled.

The topology-edge overlay is derived only from actual topology edges. Triangle
wireframes, tessellation edges, face-color boundaries and shading artifacts do
not substitute for it. Hovered or selected edges may be drawn temporarily in
Shaded to provide interaction feedback without enabling the global overlay.

Shaded cylinders show a visually continuous lit surface subject only to the
derived tessellation quality. Shaded With Edges adds circular boundaries and a
seam only when the seam is an actual topology edge; it never shows every
tessellation triangle edge. Hole boundaries receive clear topology-edge
contrast in Shaded With Edges and remain geometrically readable through
lighting, without explicit overlay, in Shaded.

The active mode remains Viewer/session state and is not persisted in `.microsw`.
The selector exposes Shaded, Shaded With Edges and Wireframe using a clear active
state, project-owned icon and tooltip. Exact overlay color and line style are
implementation details, but Shaded and Shaded With Edges must be distinguishable
at a glance without inspecting the active button.

## Rationale

Explicit pass semantics make each mode predictable, preserve topology-derived
edge truth and prevent accidental mesh visualization on curved surfaces.

## Consequences

- Viewer selects face and edge passes from the active display mode.
- Rendering continues to consume identity-free derived draw data.
- Edge hover/selection feedback is independent of the global overlay pass.
- C2A requires visual Experience Gate evidence for immediate mode distinction,
  curved surfaces and holes.

## Alternatives Considered

- Always render topology edges: rejected because it makes Shaded equivalent to
  Shaded With Edges.
- Draw tessellation wireframe: rejected because mesh detail is not CAD topology.
- Persist display mode in FeatureModel: rejected because it is session state.

## Deferred / Non-goals

Materials, textures, PBR, section views, hidden-line removal and persistence of
Viewer state are deferred.
