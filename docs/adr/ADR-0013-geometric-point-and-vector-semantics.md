# ADR-0013 — Geometric Point and Vector Semantics

Status: ACCEPTED

## Context

B0, B1 and B2 are frozen. B1 provides internal Vector2/Vector3 mathematics;
D3 defines the geometric foundation before B3 implementation. Positions and
displacements need distinct semantics without changing the frozen Math APIs.

## Decision

Point2 and Point3 are explicit geometric value types, distinct from Vector2
and Vector3. Use the corresponding B1 vector as a mathematical foundation
where appropriate, through composition rather than inheritance.

For matching dimensions, the valid semantic operations are:

| Operation | Result |
| --- | --- |
| Point - Point | Vector |
| Point + Vector | Point |
| Point - Vector | Point |

Do not introduce Point + Point or mathematically ambiguous operator semantics.
Approximate comparison remains explicit; do not introduce approximate
operator==. Representation comparison is not automatically geometric
equivalence (ADR-0014).

Geometry depends on Math; Math never depends on Geometry. Points use Scalar
and preserve D1's coordinate/unit conventions. Physical units are not embedded
in the types; initial document/kernel lengths are interpreted in millimetres.

Point coordinates must be finite: NaN, +inf and -inf are invalid. A point is
valid whenever its coordinates are finite. Invalid construction uses
std::invalid_argument; result failures follow ADR-0015.

Prefer small, predictable, copyable/movable, value-oriented types, valid on
construction, with non-mutating operations by default and no exposed mutable
references. Absolute const immutability is not required, but trivial setters
must not allow invariants to be broken.

Start with concrete 2D/3D types, not generic `Point<N>`. Generalization requires
demonstrated repetition and benefit, not speculative infrastructure.

## Rationale

Explicit position/displacement semantics prevent invalid geometric arithmetic
while reusing the educational Math foundation without reversing dependencies.

## Consequences

- B3 points belong to Geometry, not Math, Topology or the Viewer.
- Existing B1 Vector3-based transformPoint/transformDirection remain unchanged.
- Construction and operations must preserve finite valid values.
- Geometric approximate queries use the explicit ADR-0015 policy.

## Alternatives Considered

- Using Vector as Point: rejected because it conflates position and displacement.
- Point inheriting Vector: rejected; composition/value semantics are preferred.
- Approximate operator==: rejected in favour of explicit comparison.
- Generic dimensional point templates: deferred beyond initial B3.

## Deferred / Non-goals

No point type, operator or target is implemented by D3. Exact APIs and any
transform adapters require separately authorized increments. D3 does not
authorize B3.1, a unit-conversion subsystem or changes to B1.
