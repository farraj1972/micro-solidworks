# ADR-0017 — Geometry Presentation Boundary

Status: ACCEPTED

## Context

B3 is frozen as model-only value Geometry. B4 must visualize those values
without coupling the educational kernel to Viewer, Rendering or UI concerns.

## Decision

Geometry remains model-only. Presentation derives renderable data outside
Geometry through a project-owned boundary:

```text
Math -> Geometry -> Presentation / Adapter -> Viewer / Rendering
```

This is a conceptual dependency flow and does not require four physical
targets. Geometry must not depend on Presentation, Viewer, Rendering or UI.
Point, Segment, Line and other Geometry values must not acquire color,
visibility, hover, selection, VAO/VBO, shader, render-state or viewer-state
members, nor `render`, `draw` or `toVbo` methods.

B4 shall use a small geometry presentation/collection sufficient for its
vertical slice. It shall reuse existing Viewer and Rendering infrastructure
where appropriate. Geometry model lifetime and presentation lifetime remain
distinct; concrete ownership is increment-scoped beginning with B4.1.

## Rationale

An explicit adapter preserves the frozen kernel and keeps visual policy
replaceable while allowing the Viewer to consume derived representations.

## Consequences

- Presentation may depend on Geometry and produce rendering inputs.
- Selecting, highlighting or drawing an entity never mutates its Geometry value.
- New rendering abstractions remain project-owned and narrowly justified.
- No generic scene graph, hierarchy, ECS or component framework is implied.

## Alternatives Considered

- Rendering methods on Geometry: rejected because they reverse the dependency.
- A generic scene graph or ECS: deferred as unnecessary for the B4 proof.
- Viewer ownership of the Geometry kernel: rejected; observation is consumption.

## Deferred / Non-goals

This ADR creates no target, class or implementation. Hierarchical transforms,
scene nodes, CAD documents, topology and persistence remain deferred.
