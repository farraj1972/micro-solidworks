# ADR-0026 — Topological Orientation and Oriented Uses

Status: PROPOSED

## Context

Shared edges and faces need a canonical definition and a reversible use in
higher-level topology. Duplicating an edge or face merely to reverse traversal
would break shared identity.

## Decision

Every `Edge` has a canonical direction from its authoritative `start VertexId`
to `end VertexId`. A Wire stores an ordered sequence of `OrientedEdgeUse` values,
each containing an `EdgeId` and `Forward` or `Reversed` orientation. Forward
traverses start-to-end; Reversed traverses end-to-start. An edge is never copied
to reverse it.

The first B7 Wire is closed, has at least three uses, contains no repeated edge
use, and bounds a convex planar polygon made from straight edges. Closure is
exact and topological: the traversal end vertex of each use equals the traversal
start vertex of the next use, including the last-to-first pair.
Convexity validation is sufficient for the authorized boundary and B7 adds no
separate general self-intersection framework.

A planar Face refers to one outer `WireId` and stores a support `Plane`. With the
face observed in the direction of `Plane.normal()`, traversal of the outer Wire
is counter-clockwise. This makes the support-plane normal the canonical Face
normal. B7 Faces have no holes or inner wires.

A Shell stores ordered `OrientedFaceUse` values containing a `FaceId` and
`Forward` or `Reversed` orientation. Forward uses the Face support-plane normal
and outer-Wire traversal as the shell-facing normal and boundary traversal.
Reversed uses the opposite normal and reverses the boundary traversal by
visiting edge uses in reverse order with each orientation flipped. Faces are
not duplicated to invert their shell use.

For the initial convex Solid, every oriented Face use must face outward. A B7
implementation may validate this deterministically using an interior reference
point computed from the cuboid's vertices and require each shell-facing plane
normal to point away from that point. This rule is limited to the authorized
convex slice and is not a general inside/outside algorithm.

## Rationale

Canonical records plus oriented uses preserve sharing and make traversal, Face
normal semantics and shell orientation deterministic.

## Consequences

- The same Edge can be traversed oppositely by adjacent Face boundaries.
- The same Face representation can be used with either shell orientation.
- Wire winding is coupled explicitly to the stored support Plane orientation.
- General solid orientation remains deferred beyond the convex B7 slice.

## Alternatives Considered

- Duplicate reversed Edges or Faces: rejected because it destroys shared
  identity.
- Infer orientation from unordered incidence: rejected because traversal and
  normals would be ambiguous.
- General inside/outside classification: deferred as unnecessary for a cuboid.

## Deferred / Non-goals

Inner wires, holes, non-convex boundaries, curved topology, arbitrary-solid
inside/outside classification, half-edge/DCEL structures and Euler operators are
deferred.
