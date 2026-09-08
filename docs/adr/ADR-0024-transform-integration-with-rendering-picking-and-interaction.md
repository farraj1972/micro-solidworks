# ADR-0024 — Transform Integration with Rendering, Picking and Interaction

Status: ACCEPTED

## Context

B6 must update visible geometry and interaction coherently while retaining the
generic Rendering boundary and B4 visual identity.

## Decision

Rendering and picking consume the same transformed world-space semantics:

```text
local Geometry + Transform3
  -> world representation
  -> rendering and screen projection/picking
```

For `Point3`, transform the local point. For `Segment3`, transform both local
endpoints. For `Line3`, transform its origin as a point and its direction as a
direction; after non-uniform scale, normalize and validate the result when
reconstructing the world `Line3`. Only then derive the same finite,
view-dependent Line representation for rendering and picking. Local Geometry
remains unchanged. Ray/Plane transformation may be documented conceptually,
but their visualization remains deferred.

Rendering may use `uModel = Transform3.matrix()`. Different entity transforms
may require per-entity draws in B6; correctness takes priority over batching.
Instancing, transform buffers and ECS are not justified. Rendering remains
generic and receives no `VisualEntityId`, hover/selected state or Geometry
semantic type through a GPU identity path.

Picking must never test untransformed local Geometry when transformed Geometry
is rendered. The B4 screen-space picking tolerance remains unchanged and
distinct from Geometry tolerance.

A transform change preserves `VisualEntityId`. Selection therefore remains
attached to the same entity, hover is recomputed, and highlight follows the
transformed position.

B6 may introduce a minimal Transform editor for the selected entity with XYZ
translation, XYZ rotation displayed in degrees, and XYZ scale. The UI converts
degrees to radians and must reject invalid edits without corrupting state. A
gizmo is not required.

## Rationale

Sharing world-space semantics makes what is rendered equal to what is picked,
while stable visual identity keeps interaction attached to the entity rather
than to obsolete coordinates.

## Consequences

- Per-entity drawing is acceptable for the MVP slice.
- B4 visual-state precedence remains applicable after transforms.
- No rendering optimization is authorized ahead of measured need.

## Alternatives Considered

- Pick local Geometry while rendering transformed data: rejected as incoherent.
- GPU/color identity path: deferred; B4 geometric picking remains authoritative.
- Instancing or transform buffers: deferred as premature optimization.

## Deferred / Non-goals

Gizmos, instancing, transform buffers, acceleration structures, Ray/Plane
visualization, hierarchy, Document, Topology, persistence and undo/redo are not
part of B6.
