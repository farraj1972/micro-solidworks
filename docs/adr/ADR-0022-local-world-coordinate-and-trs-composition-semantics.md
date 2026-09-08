# ADR-0022 — Local/World Coordinate and TRS Composition Semantics

Status: ACCEPTED

## Context

B6 requires one unambiguous mapping from local presented Geometry to the world
representation used by rendering and picking.

## Decision

An entity's Geometry is expressed in local space. With no hierarchy, parent
space equals world space. World values are derived by the entity transform.

Preserve ADR-0009 column-vector semantics:

```text
v' = M * v
M = T * R * S
```

The conceptual application order is Scale, then Rotation, then Translation.
Translation is a finite `Vector3`, interpreted as model-space millimetres at
the current application/document boundary while Math remains unit-agnostic.

Points use homogeneous `w = 1`; directions use `w = 0`, so translation does
not affect directions. `Transform3` stores TRS as authoritative semantic state;
its `Matrix4` is derived. TRS and an authoritative matrix must not be stored in
parallel.

B6 is flat: `one entity -> one local transform -> world`.

## Rationale

A single local-to-world and composition convention prevents render/pick drift
and preserves the mathematical rules frozen by ADR-0008 and ADR-0009.

## Consequences

- Rendering and picking derive world data from the same entity transform.
- Translation never changes a transformed direction.
- No parent or world matrix is authoritative entity state in B6.

## Alternatives Considered

- Row vectors or a different composition order: rejected by ADR-0009.
- Authoritative matrix plus TRS: rejected as redundant divergent state.
- Parent/child composition: deferred until hierarchy is required.

## Deferred / Non-goals

Hierarchy, parent/child transforms, assemblies, transform trees and scene nodes
are outside B6.
