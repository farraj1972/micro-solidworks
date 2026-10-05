# ADR-0038 — Boolean Operation Scope and Result Semantics

Status: PROPOSED

## Context

B11 needs a useful first Boolean slice without claiming general BRep support.
Even Boolean operations between boxes can produce empty, disconnected or
non-convex results, so failure and result semantics must be explicit.

## Decision

The first B11 operands are finite, closed, outward-oriented, single-shell
manifold Solids whose topology and geometry describe axis-aligned rectangular
boxes. Each axis extent must be strictly greater than
`defaultGeometricTolerance`. A generic Solid is never silently treated as a
supported operand.

B11 supports Union, Difference and Intersection within this subset. `SUCCESS`
requires one connected, hole-free, single-shell orthogonal result. No operation
returns multiple Solids. The operation contract returns `BooleanResult` with one
of these states:

```text
SUCCESS + Solid
EMPTY
INVALID_INPUT
UNSUPPORTED_CASE
NUMERICAL_FAILURE
```

`EMPTY` is a valid result rather than an error. Disjoint intersection and
difference where B fully contains A are empty. Disjoint union is unsupported
because it needs multiple components. `A - B` with clearly disjoint B succeeds
with a newly constructed result equivalent to A. Invalid operands are distinct
from supported inputs whose result topology is outside the initial subset, and
unsupported geometry is distinct from numerical failure.

Nontrivial operations require strict volumetric overlap. Point-, edge- and
face-only contact, near-coincident boundary planes and overlap or separation
within geometric tolerance are unsupported. Inputs are immutable and candidate
failure never changes either operand or a previous Application result.

## Rationale

The explicit result type prevents empty geometry and scope limits from becoming
exceptions or misleading failures. Axis-aligned boxes provide deterministic
fixtures for all three operations while keeping the first kernel auditable.

## Consequences

- B11 demonstrates all three Boolean operations with deliberately narrow input
  and result contracts.
- Results own new topology-local identities; operand IDs are not preserved.
- The API can later be wrapped by a Boolean feature without owning feature
  identity or history now.

## Alternatives Considered

- General manifold Solid operands: rejected because current Geometry and
  Topology lack the required classification and splitting capabilities.
- Intersection only: rejected because the approved B11 baseline must establish
  coherent semantics for Union, Difference and Intersection.
- Exceptions for empty results: rejected because emptiness is a normal Boolean
  outcome.

## Deferred / Non-goals

Oriented boxes, convex prisms, arbitrary polyhedra, curved topology, multiple
components, multiple shells, cavities, holes, touching-only cases, near-
coincident cases, feature history, automatic rebuild, Document and persistence
are deferred.
