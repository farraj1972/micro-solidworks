# ADR-0002 — GLFW for Windowing

Status: ACCEPTED

## Context

The application needs portable window creation, event processing and OpenGL
context integration without adopting a complete application framework.

## Decision

Use GLFW for windowing, input/event integration and OpenGL context creation.
Isolate it behind the project-owned windowing boundary. `ApplicationWindow` is
the current implementation, and public APIs must not expose `GLFWwindow*`.

## Rationale

GLFW is focused, portable and matches the needs of the OpenGL foundation while
leaving application and future CAD architecture under project control.

## Consequences

- GLFW-specific code remains in windowing and the necessary Dear ImGui backend
  integration.
- Application code uses `ApplicationWindow` for lifecycle and close requests.
- Native access required by an integration boundary remains opaque.

## Alternatives Considered

- SDL: broader than required for the initial foundation.
- Qt: a larger UI/application framework than D0 requires.
- Native platform APIs: greater platform-specific complexity.

These alternatives are not permanently excluded, but changing the accepted
decision requires explicit architectural approval.
