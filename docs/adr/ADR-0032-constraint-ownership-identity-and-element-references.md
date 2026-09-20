# ADR-0032 — Constraint Ownership, Identity and Element References

Status: ACCEPTED

## Context

B9 needs constraints that survive replacement-style Sketch editing without
making solver or presentation state authoritative. Entity-level identity alone
cannot identify endpoints, line directions or circle parameters.

## Decision

`Sketch` owns both its entities and its logical constraints. The solver owns
neither. It operates on temporary candidate state derived from a Sketch.

`SketchConstraintId` is a strongly typed `std::uint32_t` index. It is
Sketch-local, stable for the live constraint lifetime, non-persistent and
distinct from every entity, visual and topology identity. Removed constraints
leave tombstones, and IDs are never reused during a Sketch lifetime.

`SketchElementRef` contains a `SketchEntityId` and a `SubElementKind`. It never
contains a pointer or reference into entity storage. B9 supports:

```text
Line.Start
Line.End
Line.Body
Circle.Center
Circle.Radius
```

These references remain stable across value replacement. Arc references are
deferred. A reference must resolve to a live entity of the compatible kind
before a constraint enters the aggregate.

The initial constraint values are:

```text
Coincident(pointRef, pointRef)
Horizontal(lineBodyRef)
Vertical(lineBodyRef)
Parallel(lineBodyRef, lineBodyRef)
Perpendicular(lineBodyRef, lineBodyRef)
HorizontalDistance(pointRef, pointRef, value)
VerticalDistance(pointRef, pointRef, value)
LineLength(lineBodyRef, value)
CircleRadius(circleRadiusRef, value)
```

Dimensions are driving only. Values must be finite; lengths and radii must be
positive where their geometry requires it. `Distance` may be proposed later if
a concrete B9 increment requires it. Angle and reference dimensions are not B9
scope.

The minimum lifecycle is add, inspect, remove, edit a driving value and solve.
An invalid or incompatible reference is `INVALID_INPUT`; it is never retained
as a live logical constraint. Removing an entity referenced by any live
constraint is rejected. The caller must explicitly remove dependent constraints
first. Constraint removal has no implicit entity side effects.

## Rationale

Keeping logical constraints with Sketch makes them part of the parametric model,
while strong logical references preserve B8 identity and editing semantics.
Explicit dependent removal avoids surprising cascade behavior.

## Consequences

- Sketch gains an indexed constraint collection alongside its entity collection.
- Constraint compatibility is validated at the aggregate boundary.
- Presentation identities and memory addresses cannot become constraint identity.
- Undo/redo and persistence can later build on stable logical IDs without being
  designed in B9.

## Alternatives Considered

- Solver-owned constraints: rejected because solver state is derived machinery.
- Direct pointers to geometry: rejected because replacement invalidates them.
- Reuse SketchEntityId: rejected because entity and constraint lifecycles differ.
- Implicitly delete dependent constraints: rejected because it hides side effects.

## Deferred / Non-goals

Point-on-curve, tangent, equal, symmetry, concentric, midpoint, Arc references
and constraints, angle/reference/diameter dimensions, undo/redo, persistence,
Document ownership, snapping and trimming are deferred.
