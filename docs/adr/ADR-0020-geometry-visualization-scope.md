# ADR-0020 — Geometry Visualization Scope and Infinite Primitive Representation

Status: ACCEPTED

## Context

B4 needs a bounded vertical slice that proves derived Geometry presentation,
rendering, picking, hover, selection and highlighting.

## Decision

Mandatory B4 visualization covers `Point3`, `Segment3` and `Line3`. `Ray3` and
`Plane` visualization are deferred from mandatory B4 scope.

`Line3` remains mathematically infinite in Geometry. Presentation derives a
finite segment clipped or truncated using view/context information:

```text
Line3 -> view/context-clipped finite visual segment
```

This finite visual representation never changes the Line model. If later
visualized, `Ray3` remains semi-infinite while presentation may clip it, and
Plane remains infinite while presentation may derive a finite quad or grid.

B4 may compose a small demo geometry collection solely to prove:

```text
Geometry -> Presentation -> Viewer rendering -> Picking
         -> Hover -> Selection -> Highlight
```

The collection is not a CAD Document, scene graph, BRep, Sketch, feature tree
or persistent model. Existing `ShaderProgram`, `LineRenderer`,
`WorkspaceViewport`, `OrbitCamera` and view/projection facilities should be
reused where appropriate. Point rendering may add only the minimal project-owned
primitive rendering capability justified when `GL_LINES` is unsuitable.

## Rationale

Three representative primitives prove finite, point-like and infinite visual
cases without expanding B4 into a general scene or CAD architecture.

## Consequences

- Visual clipping is presentation policy, never Geometry state.
- B4 must complete the full interaction cycle for its mandatory types.
- Viewer consumes presentation but does not own the Geometry kernel.

## Alternatives Considered

- Rendering every B3 primitive in B4: rejected as unnecessary scope.
- Truncating Line/Ray or meshing Plane in Geometry: rejected as model mutation.
- A generic scene graph for the demo: rejected as premature infrastructure.

## Deferred / Non-goals

Mandatory Ray3/Plane rendering, generic scene graphs, hierarchical transforms,
ECS, Topology/BRep, Sketching, CAD Modeling, persistence and any functionality
beyond explicitly authorized B4 increments remain deferred.
