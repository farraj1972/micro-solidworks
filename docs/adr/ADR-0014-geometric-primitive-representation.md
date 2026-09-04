# ADR-0014 — Geometric Primitive Representation

Status: ACCEPTED

## Context

B3 needs understandable, non-redundant representations whose invariants are
independent of rendering and future topology. D3 freezes these decisions,
not their implementation or every future query signature.

## Decision

Initial primitives are concrete Segment2, Segment3, Line2, Line3, Ray2, Ray3
and Plane, using the point/vector distinction of ADR-0013.

| Primitive | Canonical representation | Domain / invariant |
| --- | --- | --- |
| Segment2 / Segment3 | Endpoints A and B | Approximately coincident endpoints are allowed |
| Line2 / Line3 | Origin + normalized direction | origin + t * direction, t in R; stored direction is unit length |
| Ray2 / Ray3 | Origin + normalized direction | origin + t * direction, t >= 0; stored direction is unit length |
| Plane | Origin + normalized normal, or mathematically equivalent project-owned form | Stored normal is unit length |

A Segment can exist as a zero-length/degenerate geometric value. Do not store
a redundant direction: derive it when needed. A named query such as
isDegenerate must be able to identify degeneracy; operations needing a valid
direction must fail explicitly for a degenerate segment.

Line/Ray zero or near-zero directions and Plane zero or near-zero normals
are invalid construction inputs. Normalize valid inputs during construction
and preserve the unit invariant afterwards; do not retain an unnormalized
direction/normal and normalize ad hoc in every query. Segment normalizes
nothing because it stores only endpoints. Finite-value and error policies
are defined by ADR-0015.

### Representation versus geometric equivalence

Different origins A and B with the same direction D can describe the same
geometric line. Stored representation and geometric equivalence are distinct.
Do not define structural operator== as automatic geometric equivalence;
approximate comparison must be explicit. Future named queries such as
isCoincident, isParallel and contains may express particular relations,
without committing their complete APIs in D3.

### Value semantics and planned queries

Prefer small predictable copyable/movable values, valid on construction,
non-mutating operations and no exposed mutable references. Setters must not
break invariants; absolute const immutability is not mandated.

B3 should focus on representation, invariants, basic queries,
distance/projection and integration. Possible named operations include
length, squaredLength, direction, midpoint, contains, closest point, distance,
projection, isDegenerate and basic parallel/perpendicular queries. This is a
direction for subsequent increments, not a commitment to all exact APIs.

## Rationale

Canonical representations make invariants visible and avoid inconsistent
redundant state. Allowing degenerate segments models a useful value while
keeping mathematically undefined direction-dependent operations explicit.

## Consequences

- Lines and rays are unbounded/bounded parameter domains, not render segments.
- Plane is a geometric value, not a CAD entity or Face.
- Equivalent values need not have identical stored representations.
- Construction checks and named queries must respect ADR-0015.

## Alternatives Considered

- Segment endpoints plus stored direction: rejected as redundant.
- Unnormalized stored Line/Ray direction or Plane normal: rejected.
- Rejecting every zero-length segment: not selected.
- Structural equality as geometric equivalence: rejected.

## Deferred / Non-goals

No generic `Point<N>`, `Line<N>` or `Segment<N>` in B3. Generalization requires
demonstrated benefit. No general intersection framework in the initial core;
an individual future intersection operation requires an explicitly justified
increment. D3 implements no primitives, queries, target or geometry subsystem.
