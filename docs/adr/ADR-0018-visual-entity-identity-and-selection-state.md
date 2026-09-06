# ADR-0018 — Visual Entity Identity and Selection State

Status: ACCEPTED

## Context

B3 Geometry has value semantics and no persistent identity. B4 needs to
associate a presented value with hover, selection and visual appearance.

## Decision

B4 uses a minimal project-owned visual entity identifier external to Geometry.
Its exact representation is decided by B4.1; identity must not be embedded in
Point, Segment, Line or other Geometry values.

Hover and selection are separate interaction states:

- hover is transient pointer/cursor state;
- selection persists until interaction changes it.

The initial MVP permits zero or one selected visual entity. Selection state
belongs to application/presentation/interaction, not Geometry. Selecting or
deselecting must not mutate the geometric value. Normal, hovered and selected
appearance belongs to Presentation/Rendering and is not geometric state.

## Rationale

External identity connects value Geometry to interaction without corrupting
its mathematical semantics or anticipating persistent CAD identity.

## Consequences

- Presentation associates identity, Geometry and visual state.
- Hover does not implicitly define persistent selection.
- Geometry is identical before and after selection changes.
- Highlight colors and policy remain outside Geometry.

## Alternatives Considered

- IDs and selected flags inside Geometry: rejected as presentation leakage.
- Geometry-address identity: not frozen as a durable semantic contract.
- Multi-selection as the initial model: deferred to keep the slice focused.

## Deferred / Non-goals

Multi-selection, selection sets/groups, Ctrl/Shift additive selection,
persistent UUIDs, serialization, database identity and stable cross-session
identity are deferred. This ADR implements no identity type or selection store.
