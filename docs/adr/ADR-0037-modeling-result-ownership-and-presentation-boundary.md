# ADR-0037 — Modeling Result Ownership and Presentation Boundary

Status: ACCEPTED

## Context

B10 needs observable extrusion behavior before Document, Feature Tree and
Parametric Rebuild exist. Topology and Modeling must remain independent of UI
and rendering while the generated Solid still needs coherent presentation,
picking and failure behavior.

## Decision

B10 extrusion is one-shot generation from the current solved Profile. Editing
Sketch or distance does not automatically rebuild a Solid. The UI may expose a
distance and an explicit `Regenerate` action; this is manual recomputation, not
feature history or a dependency graph.

Until Document exists, Application owns one optional active modeled Solid. A
successful complete extrusion replaces it. Profile extraction or extrusion
failure leaves the previous active Solid and its presentation unchanged and
reports a deterministic error.

The Solid owns only B7 aggregate-local topology identity. B10 adds no CAD
feature identity or persistence. Presentation may own one local visual identity
for the active Solid.

Topology and Modeling do not depend on Presentation. A derived adapter maps:

```text
Solid topology -> world line segments -> one Solid presentation proxy
```

The first representation is wireframe. Multiple derived edge segments share
one logical selectable Solid identity. Picking any visible derived edge selects
that Solid. Face and Edge sub-selection are deferred. Flat shaded faces are
optional future work and do not block B10.

`Profile` remains a Modeling input and `Solid` a Modeling output. A later B12
feature may wrap the one-shot operation, and B13 may add automatic rebuild;
neither concern is owned by the current API.

## Rationale

Application ownership gives the vertical slice a clear lifetime without
inventing Document semantics. A derived wireframe proxy demonstrates the volume
while preserving the CAD-authoritative/render-derived boundary.

## Consequences

- GAP-SCENE-002 remains open and does not block the single active result.
- Regeneration is explicit and transactional at the Application boundary.
- Solid-level picking is sufficient for the initial slice.
- Future Feature Tree work can introduce persistent feature identity without
  changing Topology Solid identity.

## Alternatives Considered

- Solid owns feature/history state: rejected as B12/B13 scope.
- Modeling depends on Presentation: rejected as dependency inversion.
- Edge-level visual identities: rejected because sub-selection is not required.
- Flat shaded renderer as prerequisite: rejected as unnecessary for the first
  observable volume slice.

## Deferred / Non-goals

Document and persistence, feature identity/history, automatic rebuild,
materials, lighting, advanced shading and Face/Edge sub-selection are deferred.
