# ADR-0003 — OpenGL for Rendering

Status: ACCEPTED

## Context

Micro SolidWorks needs an initial graphics API that supports learning the
rendering pipeline and establishing a portable application foundation.

## Decision

Use OpenGL as the initial rendering API. The current implementation requests
OpenGL 3.3 Core Profile, loads functions with GLAD and bootstraps the context in
`OpenGLContext` behind `microsw_rendering`.

Rendering is infrastructure. It must remain separate from the authoritative
CAD representation; a CAD entity must not become an OpenGL mesh.

## Rationale

OpenGL supports direct study of rendering concepts with comparatively low
bootstrap cost, favouring learning and implementation speed for the initial
project stages.

## Consequences

- GLAD and OpenGL calls remain inside the rendering boundary.
- The initial renderer targets the broadly understood 3.3 Core feature set.
- Future CAD data will require an explicit tessellation and render-model path.

## Alternatives Considered

- Vulkan: substantially greater bootstrap complexity.
- DirectX: stronger platform coupling.
- Higher OpenGL versions: not required by the B0 foundation.
- An abstract rendering API: premature before concrete rendering needs exist.
