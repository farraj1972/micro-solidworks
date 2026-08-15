# ADR-0004 — Dear ImGui for UI

Status: ACCEPTED

## Context

The application needs a lightweight desktop UI foundation without placing UI
technology inside the future CAD domain.

## Decision

Use Dear ImGui with its GLFW and OpenGL3 backends. Keep lifecycle and backend
initialization in `ImGuiLayer`, and application UI composition in
`ApplicationShell`, both within `microsw_ui`.

Docking is not adopted in B0. The application shell uses a deterministic fixed
layout while its structure is understood.

## Rationale

Dear ImGui enables rapid, explicit construction of tools-oriented interfaces
and integrates directly with the approved GLFW/OpenGL foundation.

## Consequences

- Dear ImGui remains an external UI-layer concern.
- Future domain modules must not depend on Dear ImGui.
- More complex desktop conventions may require later evaluation.

## Alternatives Considered

- Qt: a broader framework than the initial foundation requires.
- Native UI toolkits: platform-specific and more expensive to maintain.
- Other immediate-mode UIs: no stronger fit for the approved stack was chosen.
