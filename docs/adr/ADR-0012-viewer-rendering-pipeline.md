# ADR-0012 — Viewer Rendering Pipeline

Status: ACCEPTED

## Context

B0 establishes OpenGL and Dear ImGui, but does not implement a 3D viewer.
B2 needs a small rendering path for navigation aids without creating CAD
entities or adopting a scene graph.

## Decision

### Graphics and resources

Continue with OpenGL 3.3 Core and GLAD under D0; adopt GLSL 330 Core for
viewer shaders. Initial rendering is unlit line rendering, without lighting,
materials, PBR, geometry shaders or compute shaders.

Encapsulate GPU resources in project-owned abstractions. Expected resources
are shader program, VAO and VBO. Public project APIs must not leak GLuint.
This decision does not create resource classes or shaders.

### Workspace integration

Use the existing ApplicationShell Workspace region, not a second window.
Initially render directly into that region with appropriate viewport and
scissor settings.

Offscreen framebuffer rendering remains DEFERRED. Do not introduce a
framebuffer texture / ImGui Image path in this phase.

### Depth and aids

Use GL_DEPTH_TEST for the 3D viewer. Grid and axes normally participate in
depth testing.

The initial grid lies in the XY plane at Z = 0. Reference axes identify the
origin and X, Y, Z directions. Both are render aids, never CAD entities.

Axis colours are X = red, Y = green, Z = blue. This convention belongs only
to rendering/UI and does not alter mathematical or CAD semantics.

Grid/axes may generate render primitives directly because they are viewer
aids. This is not an exception for CAD entities, whose pipeline remains:

```text
CAD representation -> tessellation -> render representation -> OpenGL
```

Do not implement an infinite adaptive grid in this gate.

### Deferred scope

Picking, hover, selection and highlighting remain deferred to B5 —
Selection & Picking. Do not introduce a scene graph in B2 just to draw grid
and axes. CAD geometry, topology and scene/domain ownership remain outside
the Viewer.

## Rationale

Unlit lines and direct Workspace rendering keep the first viewer small and
educational. GPU isolation preserves replaceable boundaries, while separating
render aids from CAD entities protects the authoritative domain model.

## Consequences

- Future Workspace integration must coordinate viewport/scissor and UI drawing.
- Resource ownership and cleanup must follow project RAII rules.
- Depth testing applies to aids without promoting them to domain geometry.
- Offscreen rendering and richer rendering techniques require later scope.
- B2 remains NOT STARTED until an implementation increment is authorized.

## Alternatives Considered

- Offscreen framebuffer plus ImGui Image: deferred in favour of direct rendering.
- A separate viewer window: not selected; reuse ApplicationShell Workspace.
- Lit/material rendering: outside the initial unlit line scope.
- Scene graph solely for grid/axes: unnecessary for initial aids.
