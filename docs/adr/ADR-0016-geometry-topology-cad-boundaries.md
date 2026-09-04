# ADR-0016 — Geometry / Topology / CAD Boundaries

Status: ACCEPTED

## Context

B0/B1/B2 are frozen. B3 will introduce geometric values, not a document,
topology model or renderer-owned CAD model. ADR-0006's Educational Geometry
Kernel strategy and the authoritative CAD representation must be preserved.

## Decision

### Dependency and educational strategy

Geometry/core primitives use BUILD, based on internal Math. Dependency
direction is Geometry -> Math; Math never depends on Geometry. In terms of
foundations supplied to future consumers:

```text
Math
  |
  v
Geometry
  |
  v
Topology / Modeling / CAD (future consumers)
```

These arrows describe foundation-to-consumer flow, not reversed dependencies.
Geometry must not depend on Rendering, Viewer, UI, Document, Topology,
Modeling or Persistence.

A future microsw_geometry boundary may consume microsw_math. It is
PLANNED / NOT STARTED, not an existing CMake target. Preserve the internal
Educational Geometry Kernel with possible controlled future replacement
under ADR-0006. No external geometry kernel is introduced in B3.

### Geometry is not topology

```text
Point3 != Vertex
Segment3 != Edge
Plane != Face
```

This remains true even when geometric values coincide. Future topology adds
connectivity, orientation, incidence, ownership and identity. Those concepts
must not be anticipated in B3 primitive values.

### Geometry is not a CAD entity

B3 primitives do not carry IDs, selection state, colors, layer, feature
ownership, document ownership, persistence metadata or render resources.
A geometric primitive is not automatically selectable or persistible.

### Rendering remains derived

Geometry contains no OpenGL, GLAD, VAO, VBO, shader, color or line-width
concerns. Viewer may later consume Geometry through explicit boundaries;
Geometry never depends on Viewer or Rendering.

Preserve the domain pipeline:

```text
CAD representation -> tessellation -> render representation -> OpenGL
```

B2 reference grid/axes remain viewer aids, not CAD entities. They do not
authorize direct rendering ownership inside Geometry or the future CAD model.

## Rationale

Value geometry can be understood, tested and reused without graphical,
document or topological identity. Explicit boundaries preserve educational
implementations and allow controlled replacement when evidence justifies it.

## Consequences

- Future topology and CAD consumers add their own identity/ownership.
- Geometry stays independent of infrastructure and upper domain layers.
- Rendering data is derived, never authoritative CAD representation.
- Geometry implementation requires explicit B3.1 authorization.

## Alternatives Considered

- Geometry as OpenGL meshes/resources: rejected.
- Combining Point/Segment/Plane with Vertex/Edge/Face: rejected.
- Adding IDs or document ownership to primitive values: rejected.
- Integrating CGAL/OpenCascade now: deferred; no B3 dependency authorization.

## Deferred / Non-goals

D3 creates no microsw_geometry target, source, types, topology, CAD entities,
selection, persistence or rendering adapters. It does not start B3.1.
Generic primitive templates and a general intersection subsystem remain
outside initial B3 as specified in ADR-0014.
