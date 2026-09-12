# ADR-0029 — Sketch Ownership, Identity and Editing

Status: PROPOSED

## Context

B8 needs stable logical selection while geometry values are edited and visual
representations are regenerated. Existing VisualEntityId and Topology IDs have
different ownership and lifetime semantics.

## Decision

`Sketch` is the B8 aggregate. It owns one `SketchPlane` and an indexed collection
of `SketchEntity` records. Each record combines a `SketchEntityId` with one of:

```text
SketchLine   -> Segment2
SketchCircle -> Circle2
SketchArc    -> Arc2
```

`SketchEntityId` is a strongly typed `std::uint32_t` index. `UINT32_MAX` is the
invalid/default value. IDs are Sketch-local, stable for the entity lifetime,
non-persistent, and unrelated to VisualEntityId, Topology IDs or future CAD
Document identity. IDs are never reused during a Sketch lifetime.

Aggregate APIs validate complete Geometry values before insertion or
replacement. Editing replaces the entity's value atomically and preserves its
ID and entity kind. Line editing replaces its Segment2; Circle editing replaces
center/radius through a valid Circle2; Arc editing replaces its complete valid
Arc2. Failed replacement leaves the previous entity unchanged. Mutable Geometry
references and partially valid setters are prohibited.

B8 supports explicit `remove(SketchEntityId)`. Removal invalidates lookup of that
entity and leaves a tombstone so subsequent insertions never reuse the ID.
Iteration preserves surviving insertion order. Undo/redo is not part of B8.

Rectangle is a creation command, not a stored entity kind. Its first form takes
two corners in Sketch coordinates and creates four independent axis-aligned
SketchLine entities. Coincident corners or a width/height not greater than
`defaultGeometricTolerance` are invalid. The command prevalidates all four
segments and available IDs, then inserts all four or none. Rotated rectangles
are deferred.

## Rationale

Aggregate-local stable IDs allow selection and future constraints to survive
value replacement. Tombstones provide simple non-reuse without introducing
generations, UUIDs or a persistence model.

## Consequences

- B9 can refer to entities and later-defined sub-elements through stable IDs.
- Deletion can leave sparse internal storage; performance is secondary to clear
  lifetime semantics.
- A Rectangle's four lines can later receive coincident, horizontal, vertical
  and dimensional constraints independently.

## Alternatives Considered

- Use VisualEntityId as Sketch identity: rejected because it is a presentation
  proxy.
- Use Topology IDs: rejected because Sketch entities are not topology.
- Mutate fields through exposed references: rejected because invalid transient
  geometry could enter the aggregate.
- Persistent Rectangle entity: rejected because it would complicate the first
  constraint model and duplicate four line boundaries.

## Deferred / Non-goals

Constraints, dimensions, sub-element identity details, solver state, undo/redo,
persistence, Document ownership, construction geometry and generic lifecycle are
deferred.
