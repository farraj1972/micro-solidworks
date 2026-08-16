# ADR-0009 — Matrix and Transform Conventions

Status: ACCEPTED

## Context

Matrix multiplication, vector application and transform composition become
ambiguous unless their semantic conventions are frozen before implementation.

## Decision

Use column vectors and apply transformations as:

```text
v' = M * v
```

Composition follows the corresponding right-to-left application order. For:

```text
M = T * R * S
```

`S` is applied first, `R` second and `T` last.

Adopt a column-major logical matrix convention.

> Mathematical semantics must not depend on physical memory layout.

A future storage-layout change must not alter `M * v` or the semantic order of
composition.

Quaternion, generic `Matrix<T, N, M>` and generic `Vector<T, N>` abstractions
remain deferred. Quaternion should be reconsidered only when rotation or camera
requirements demonstrate a concrete need.

## Rationale

A single explicit convention prevents transposition and composition-order
errors across Math, geometry and rendering. Separating semantics from storage
also preserves implementation freedom without changing public behavior.

## Consequences

- Future matrix and transform tests must verify column-vector semantics.
- Transform composition reads left-to-right but applies right-to-left.
- Rendering adapters must respect the Math convention even when an API boundary
  has different memory or upload requirements.
- No generic matrix/vector framework or quaternion is authorized by this ADR.

## Alternatives Considered

- Row vectors with `v' = v * M`: not selected.
- Composition semantics tied to physical array order: rejected because storage
  is an implementation detail.
- Generic templates and Quaternion in the initial Math increment: deferred until
  concrete requirements justify their complexity.
