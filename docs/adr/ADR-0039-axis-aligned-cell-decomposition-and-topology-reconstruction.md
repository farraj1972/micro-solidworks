# ADR-0039 — Axis-Aligned Cell Decomposition and Topology Reconstruction

Status: ACCEPTED

## Context

A general BSP or BRep face-splitting kernel would exceed the first B11 slice.
The supported operands nevertheless need one algorithmic path rather than
fixture-specific topology construction.

## Decision

B11 uses axis-aligned coordinate-cell decomposition. It collects the sorted
unique X, Y and Z boundary coordinates of both operands, forms the finite grid
cells, classifies each cell as inside A and/or B, and applies these occupancy
rules:

```text
Intersection: insideA && insideB
Union:        insideA || insideB
Difference:   insideA && !insideB
```

Only occupied-to-empty or occupied-to-outside interfaces become result
boundaries; internal interfaces between occupied cells are removed.

Temporary values such as `AxisAlignedBox`, `BooleanGrid`, cells and boundary
rectangles remain implementation details of `microsw_modeling`. They do not
become public Geometry primitives. Coordinate equality and ordering use
`defaultGeometricTolerance`. Grid coordinate deduplication never snaps or
mutates operand geometry; an ambiguous near-coincident partition is reported
as `UNSUPPORTED_CASE`.

Boundary rectangles are reconstructed into new Topology. Geometrically equal
result vertices share one VertexId and identical boundary segments share one
EdgeId. Every emitted Face remains a convex axis-aligned rectangle with one
outer Wire. Multiple coplanar Faces are permitted and need not be merged. Each
final manifold Edge has exactly two opposite Face traversals.

Before reconstruction, face-adjacent cell flood fill requires exactly one
connected occupied component. Empty cells connected to the unbounded exterior
are distinguished from enclosed empty components; a cavity is unsupported.
Tunnel/through-hole detection may be conservative, but any detected tunnel,
cavity, disconnected component or non-manifold boundary produces
`UNSUPPORTED_CASE`, never partial topology.

The dependency direction remains:

```text
microsw_modeling -> microsw_topology -> microsw_geometry -> microsw_math
```

GAP-GEO-002 remains OPEN. B11 implements only interval and axis-plane cell
classification, coordinate deduplication and boundary-cell extraction. It does
not provide general Segment-Plane, Plane-Plane, Segment-Face, Face-Face or
polygon intersection.

## Rationale

Cell occupancy makes all three operations explicit, avoids coplanar polygon
special cases and cleanly separates geometric classification from topology
reconstruction. It is educational and replaceable without pretending to be a
general BRep Boolean engine.

## Consequences

- Correctness takes precedence over minimizing the number of coplanar Faces.
- Sewing and healing are not used to repair incorrect reconstruction.
- Fixture validation can inspect occupancy, bounds, connectivity and manifold
  topology without introducing generic mass properties.

## Alternatives Considered

- Half-space clipping: useful for convex intersection but insufficient for
  non-convex Union and Difference results.
- BSP/CSG polygon clipping: deferred because it substantially widens numerical
  and polygon-processing scope.
- General BRep face splitting: deferred until the topology and geometry kernel
  intentionally support it.
- Exact fixture-specific builders: rejected because they would not establish a
  coherent Boolean algorithm.

## Deferred / Non-goals

General intersections, generic planar polygons, BSP/CSG, general face splitting,
sewing, healing, coplanar face merging and mass properties are deferred.
