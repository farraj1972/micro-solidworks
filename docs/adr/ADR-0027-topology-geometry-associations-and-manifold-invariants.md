# ADR-0027 — Topology Geometry Associations and Manifold Invariants

Status: PROPOSED

## Context

B7 must associate topology with the frozen educational Geometry kernel without
duplicating derivable values or confusing numerical coincidence with shared
topological identity.

## Decision

The authoritative and derived associations are:

```text
Vertex -> authoritative Point3
Edge   -> authoritative start/end VertexId; derived Segment3
Wire   -> authoritative ordered OrientedEdgeUse sequence
Face   -> authoritative outer WireId and oriented support Plane
Shell  -> authoritative OrientedFaceUse sequence
Solid  -> authoritative ownership plus one root ShellId
```

An Edge rejects equal endpoint IDs. It also rejects distinct endpoints whose
positions are coincident within `defaultGeometricTolerance`, because the first
slice requires a non-degenerate derived `Segment3`. No `Segment3` is stored as
second authoritative endpoint state.

All Face boundary vertices must lie on the stored support Plane within
`defaultGeometricTolerance`. The ordered boundary must satisfy the planar,
convex and winding rules in ADR-0026. Wire records do not duplicate Point3 or
Segment3 geometry, and a Plane remains Geometry rather than becoming a Face.

The initial Shell is closed and 2-manifold. Every Edge is used by exactly two
Face boundary uses, and those two traversals are opposite relative to the
Edge's canonical direction. The oriented Face uses must also be globally
coherent and outward under the convex rule in ADR-0026. Non-manifold input is
rejected. The initial Solid owns exactly one such closed oriented manifold Shell;
multiple shells and cavities are invalid in B7.

`defaultGeometricTolerance = 1e-9` is used only for geometric predicates such
as coincident positions, coplanarity, convexity and numerical orientation.
Connectivity, incidence, closure and sharing use exact typed-ID equality. B7
introduces no topology, sewing or healing tolerance.

The module boundary is:

```text
microsw_topology -> microsw_geometry -> microsw_math
```

Topology is model-space and does not contain Presentation `Transform3` state.
It has no dependency on Presentation, Viewer, Rendering, UI or Application.

## Rationale

Keeping only irreducible geometry authoritative prevents divergent endpoint
representations. Exact IDs express topology, while the existing tolerance is
reserved for the numerical predicates for which it was designed.

## Consequences

- Moving a Vertex in a future editing model would require controlled aggregate
  revalidation; arbitrary mutation is therefore excluded now.
- The first slice supports 8 shared vertices, 12 shared straight edges, 6
  ordered convex planar Faces, one closed manifold Shell and one simple Solid.
- Geometry remains distinct from Topology as required by ADR-0016.

## Alternatives Considered

- Store both endpoint IDs and an authoritative Segment3: rejected because the
  representations can diverge.
- Derive Face orientation without a stored Plane: rejected because the support
  plane and its orientation are required semantics.
- Use coordinate tolerance for connectivity: rejected because coincident
  positions do not imply shared identity.

## Deferred / Non-goals

Circle and curved edges, curved surfaces, holes, non-convex Faces, general
intersections, self-intersection infrastructure, non-manifold topology,
sewing/healing, booleans, mass properties, generic surfaces, multiple shells,
cavities, persistence, serialization, Document and Presentation integration are
deferred.
