# ADR-0028 — Sketch Coordinate System and Plane Semantics

Status: PROPOSED

## Context

B8 needs editable planar geometry without making world-space Geometry or
Presentation state authoritative. A Geometry `Plane` supplies an origin and
normal, but does not select deterministic local X and Y axes.

## Decision

Sketch geometry is authoritative in local two-dimensional coordinates using
`Point2`, `Vector2`, `Segment2`, `Circle2` and `Arc2`. World-space geometry is
always derived and is never stored as a second authoritative representation.

`SketchPlane` is a project-owned Sketch value whose authoritative state is a
finite `Point3 origin` plus finite, non-degenerate `Vector3 xAxis` and `yAxis`.
Construction normalizes both axes, then requires them to be perpendicular under
`defaultGeometricTolerance`. The stored axes are unit length. The normal is not
stored: it is derived as the normalized `cross(xAxis, yAxis)`, producing a
right-handed orthonormal basis. Invalid construction throws
`std::invalid_argument`.

Conversions have these semantics:

```text
local Point2  -> origin + x * xAxis + y * yAxis
local Vector2 -> x * xAxis + y * yAxis
world Point3  -> Point2(dot(world-origin, xAxis),
                        dot(world-origin, yAxis))
```

World-to-local conversion is orthogonal projection; the input need not lie on
the plane. A Geometry `Plane(origin, normal)` may be derived when needed.
`SketchPlane` remains distinct from `Plane` because it owns the in-plane basis.

The deterministic default is global XY: origin `(0,0,0)`, X axis `+X`, Y axis
`+Y`, derived normal `+Z`. This preserves the frozen coordinate conventions.

## Rationale

An explicit basis makes local/world conversion deterministic and keeps the
editable model compact. Deriving the normal avoids redundant state that could
disagree with the axes.

## Consequences

- Sketch editing operates entirely on local 2D values.
- Camera, rendering and picking cannot become authoritative Sketch geometry.
- A future mapped face plane can construct a SketchPlane only after choosing a
  deterministic in-plane X axis.

## Alternatives Considered

- Store local and world geometry: rejected because the values can diverge.
- Reuse `Plane` alone: rejected because rotation about its normal is undefined.
- Store origin, both axes and normal: rejected as redundant authoritative state.

## Deferred / Non-goals

Face-attached planes, multi-sketch placement, dynamic plane reassignment,
Document ownership and Topology conversion are deferred.
