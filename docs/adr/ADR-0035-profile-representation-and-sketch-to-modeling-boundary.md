# ADR-0035 — Profile Representation and Sketch-to-Modeling Boundary

Status: ACCEPTED

## Context

B10 needs a stable boundary between editable Sketch geometry and modeling
operations. Passing Sketch directly into extrusion would couple Modeling to
Sketch ownership, constraints and solver state. Treating a Topology Wire as the
input would make topology authoritative before modeling validation has passed.

## Decision

`Profile` is an ephemeral, validated Modeling value. It has no persistent ID
and is neither a Sketch, Topology Wire, CAD feature nor Document entity.

Its authoritative state is:

```text
ordered world-space boundary points
oriented support Plane
```

Segments are derived cyclically from adjacent points and are not stored.
The first B10 Profile has one outer loop, at least three vertices, straight
nondegenerate edges and no repeated edge, branch or disconnected component.
Every point lies on the support Plane. The boundary is strictly convex and CCW
when viewed along `supportPlane.normal()`; this contract excludes
self-intersection without requiring a general intersection framework.

Sketch remains the editing domain. An integration adapter outside
`microsw_modeling` consumes a solved Sketch and its SketchPlane, collects live
SketchLine endpoint geometry, matches endpoints with geometric tolerance,
orders exactly one closed loop, rejects ambiguity, transforms the ordered local
points through SketchPlane and constructs Profile. Explicit Coincident
constraints are not required and solver internals are never consulted.

The dependency boundary is:

```text
Application / integration adapter -> microsw_sketch
Application / integration adapter -> microsw_modeling

microsw_modeling -> microsw_topology -> microsw_geometry -> microsw_math
```

`microsw_modeling` has no Sketch or Constraints dependency.

## Rationale

Ordered world points plus an oriented Plane are the minimum unambiguous input
needed by extrusion. They avoid duplicate segment state, preserve an explicit
orientation and prevent Sketch editing or constraint concepts from entering
the Modeling core.

## Consequences

- Profile extraction is independently testable integration logic.
- Modeling consumes only validated geometry and does not own source Sketch.
- Profile has value semantics and can later be wrapped by a feature without
  acquiring feature identity now.
- GAP-GEO-002 remains open because the initial strict-convex contract needs no
  general Segment-Segment intersection.

## Alternatives Considered

- Modeling consumes Sketch directly: rejected because it couples modeling to
  editing and solver ownership.
- Profile stores ordered segments: rejected because shared endpoints would be
  duplicated authoritatively.
- Topology Wire as input: rejected because topology is an extrusion result.
- Constraint-graph connectivity: rejected because geometric closure must not
  depend on a particular constraint history.

## Deferred / Non-goals

Concave and self-intersecting profiles, holes, multiple loops, Circle/Arc
profiles, persistent profile identity, Document ownership and persistence are
deferred.
