# ADR-0015 — Geometric Tolerance and Degeneracy

Status: ACCEPTED

## Context

B1's numerical comparison policy is not a geometric modelling policy.
Before B3, the educational kernel needs explicit proximity, finite-value,
degeneracy and failure rules without changing Math globally.

## Decision

### Distinct tolerance policies

B1 retains numeric absolute tolerance = 1e-12 and numeric relative tolerance =
1e-12. Its numerical utilities remain unchanged.

The initial geometric/modeling tolerance is 1e-9 mm. This policy belongs to
the Geometry/kernel layer and represents geometric proximity at the initial
millimetre document/kernel scale. It does not replace numerical tolerance,
must not be injected into Math globally and is not a claim of absolute
physical product accuracy.

Future geometric queries use geometric tolerance for coincidence, containment,
degeneracy, distance approximately zero and parallelism when formulated
geometrically. Internal numerical calculations may still use B1 utilities;
the two policies must not be mixed automatically. Query formulations must
respect units: a length tolerance is not automatically an angular or
dimensionless threshold. Exact query formulations remain increment-scoped.

Geometry uses Scalar and remains mathematically unit-consistent. Do not
embed physical units in Point/Line/etc. or add a unit-conversion subsystem.
The numeric geometric tolerance 1e-9 is interpreted in the initial mm scale.

### Finite values, degeneracy and normalization

Public coordinates and coefficients must be finite. Reject NaN, +inf and
-inf in geometric construction wherever they would create invalid state.

| Type | Validity / degeneracy policy |
| --- | --- |
| Point | Valid whenever coordinates are finite |
| Segment | Zero-length or approximately coincident endpoints allowed; identifiable by a named query such as isDegenerate |
| Line | Zero/near-zero direction invalid at construction |
| Ray | Zero/near-zero direction invalid at construction |
| Plane | Zero/near-zero normal invalid at construction |

Line, Ray and Plane normalize valid inputs at construction; stored
directions/normals remain unit length. Segment stores endpoints, not a
direction, and does not normalize them. Operations requiring a valid segment
direction must fail explicitly when the segment is degenerate.

### Errors and invariant preservation

- Invalid constructor arguments/inputs: std::invalid_argument.
- Operations valid for the type but mathematically undefined for the specific
  state: std::domain_error.
- Overflow or non-representable results: an appropriate specific exception.
- Do not use asserts for normal runtime input validation.
- Do not introduce a custom exception hierarchy.

Prefer valid-on-construction value types and non-mutating operations. No
exposed mutable references or trivial setters may break invariants. Results
must not silently turn valid geometric state into non-finite invalid values.

## Rationale

Separating numerical stability from geometric proximity keeps B1 reusable and
makes the educational kernel's initial assumptions and failure modes explicit.

## Consequences

- Geometric proximity is a kernel policy, not a replacement epsilon for Math.
- A degenerate segment remains representable but not every query is defined.
- Future implementations must validate inputs and preserve normalized state.
- Tests must distinguish invalid construction from undefined operations.

## Alternatives Considered

- Reusing B1 epsilon as modelling tolerance: rejected; the concepts differ.
- A global Math tolerance change: rejected.
- Treating 1e-9 mm as physical accuracy: rejected.
- Assertions or silent fallback values for runtime errors: rejected.

## Deferred / Non-goals

D3 defines policy only: no tolerance constant, validator, type or exception
implementation is added. Exact query APIs, angular criteria, general
intersections and broader unit-management infrastructure are not introduced.
