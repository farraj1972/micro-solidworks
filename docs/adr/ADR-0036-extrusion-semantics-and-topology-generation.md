# ADR-0036 — Extrusion Semantics and Topology Generation

Status: PROPOSED

## Context

B10 must turn one validated planar polygon into a real shared topological Solid
without bypassing extrusion through the existing cuboid fixture or extending
B7 to curved topology.

## Decision

The initial operation is:

```text
extrude(Profile, distance) -> Solid
```

`distance` must be finite and greater than geometric tolerance. Extrusion is in
`+Profile.supportPlane().normal()`. Negative, symmetric and two-sided forms are
not supported.

For a Profile with `N` vertices, generation produces:

```text
Vertex = 2*N
Edge   = 3*N
Face   = N+2
Shell  = 1
Solid  = 1
```

The bottom vertices are the Profile points and the top vertices are translated
by `distance * normal`. Bottom, top and vertical edges are each created once
and shared by face wires. The bottom outward direction is `-normal`; the top is
`+normal`. Each boundary edge produces one planar convex side quad using its
shared bottom edge, corresponding top edge and two vertical edges.

Oriented Edge and Face uses must preserve ADR-0025–0027: every shell edge is
used exactly twice with opposite traversal and every effective face normal
points outward. Generation follows one transaction:

```text
validate Profile and distance
-> build candidate topology
-> validate Shell and Solid
-> return complete Solid
```

Any failure returns no partial Solid. `makeCuboid()` may serve as an oracle for
the rectangular fixture but is never the extrusion implementation path.

The primary fixture extrudes a solved rectangle of width `W`, height `H` and
distance `D` into 8 vertices, 12 edges, 6 faces, one shell and one Solid with
`W x H x D` bounds.

## Rationale

Positive one-sided extrusion provides the smallest auditable construction that
exercises profile orientation, shared topology and manifold validation. It
reuses B7 invariants without changing them.

## Consequences

- `microsw_modeling` owns Profile, extrusion and modeling validation/errors.
- Extrusion output is a normal Topology Solid value with aggregate-local IDs.
- Circle and Arc extrusion cannot be represented exactly by current straight
  Edge and planar Face topology.

## Alternatives Considered

- Call `makeCuboid()` for rectangles: rejected because it bypasses extrusion.
- Duplicate face-local vertices and edges: rejected because the shell would not
  have shared topology.
- Tessellate curves and claim exact topology: rejected because render
  approximation must not become authoritative CAD representation.

## Deferred / Non-goals

Circle/Arc and curved topology, concave profiles, holes, multiple loops,
negative or two-sided extrusion, taper/draft, up-to-face and Boolean join/cut
are deferred.
