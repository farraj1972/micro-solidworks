# ADR-0008 — Coordinate and Unit Conventions

Status: ACCEPTED

## Context

Vectors, transforms, geometry and rendering must share unambiguous coordinate,
orientation, unit and angle conventions before B1 implementation begins.

## Decision

Use a right-handed coordinate system. The CAD world uses the XY plane as the
horizontal/base plane and Z as vertical, so the future implementation must
satisfy:

```text
X x Y = Z
```

The Math core is unit-agnostic. The initial CAD/document convention is
millimetres; units must not be embedded in Vector2, Vector3, matrices or other
Math types.

Angles are represented internally in radians and presented in the UI in
degrees. Future APIs should avoid semantically ambiguous angular parameters
where that could cause errors.

> Rendering adapts to the mathematical conventions of the CAD domain; the math core is not shaped around OpenGL conventions.

## Rationale

These conventions align the mathematical and CAD domains while keeping Math
reusable and independent of document units, UI presentation and rendering APIs.

## Consequences

- Cross products and future coordinate-frame tests follow right-handed rules.
- CAD/document boundaries will interpret lengths as millimetres initially.
- UI boundaries convert degrees to and from internal radians.
- Rendering performs any required coordinate or scalar adaptation explicitly.
- Point types remain deferred and are not introduced by this decision gate.

## Alternatives Considered

- Left-handed coordinates: not selected for the approved CAD world convention.
- Z as a horizontal axis: rejected in favor of Z vertical.
- Units embedded in Math types: rejected because the Math core is unit-agnostic.
- Degrees internally: rejected because radians are the mathematical convention.
