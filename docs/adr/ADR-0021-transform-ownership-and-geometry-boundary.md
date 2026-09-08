# ADR-0021 — Transform Ownership and Geometry Boundary

Status: ACCEPTED

## Context

Canonical B6 needs entity transforms without changing the frozen Geometry
kernel into an object, scene or rendering model. B4 already associates a
presentation-only `VisualEntityId` with immutable presented Geometry values.

## Decision

Geometry remains transform-free and canonical. `Point3`, `Segment3`, `Line3`,
`Ray3` and `Plane` do not acquire transform, identity, ownership, hierarchy,
selection or rendering state. Geometry values presented by an entity are
local-space values; their world representation is derived externally.

A future project-owned mathematical value type `microsw::math::Transform3`
stores translation, rotation and scale semantics and derives a `Matrix4`. It
owns no identity, CAD meaning, selection or GPU state. D5 proposes the type and
location but does not implement it.

For the B6 vertical slice, Presentation owns one `Transform3` per
`VisualEntity`, or an equivalent representation preserving this boundary:

```text
VisualEntity
  VisualEntityId
  PresentedGeometry     // local-space canonical value
  Transform3
```

Controlled mutation may expose operations such as `setTranslation`,
`setRotation` and `setScale`, without mutable references. A valid transform
change preserves both `VisualEntityId` and `PresentedGeometry`.

`VisualEntity` remains a presentation entity. `VisualEntityId` remains
collection-scoped, non-CAD and non-persistent. This MVP ownership may migrate
only through a future Decision Gate when a CAD/Document model exists.

## Rationale

External transform ownership preserves Geometry value semantics and the
Geometry -> Math dependency while enabling the earliest complete interactive
transform slice.

## Consequences

- Presentation may consume both Geometry and Math without creating a cycle.
- Moving an entity does not rewrite its local Geometry coordinates.
- B6 does not introduce a CAD entity, persistent identity or Document model.
- Transform inputs must be validated before authoritative state changes.

## Alternatives Considered

- Store transforms in Geometry: rejected because it mixes value geometry with
  object and presentation state.
- Rewrite local coordinates on every edit: rejected because local Geometry
  would cease to be canonical.
- Introduce a CAD entity or scene graph now: deferred as premature for B6.

## Deferred / Non-goals

CAD entities, persistent IDs, Document ownership, hierarchy, scene graphs,
ECS, serialization and undo/redo are not authorized by this proposal.
