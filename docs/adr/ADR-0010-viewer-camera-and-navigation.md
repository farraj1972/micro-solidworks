# ADR-0010 — Viewer Camera and Navigation

Status: ACCEPTED

## Context

B0 provides the application shell and B1 provides internal mathematics.
D2 freezes viewer conventions before B2 implementation. The Viewer observes
the CAD world; it neither defines nor owns the authoritative CAD representation.

## Decision

### Responsibility and coordinates

The Viewer manages camera, viewport/navigation state, orbit, pan and zoom.
CAD geometry ownership, topology, scene/domain ownership, selection and picking
are outside this boundary.

Preserve D1: right-handed coordinates, XY horizontal/base plane, Z vertical,
and world up = +Z. The viewer adapts to the mathematical domain.

### Camera and default view

Use an initial orbit camera with conceptual state `target`, `distance`,
`yaw` and `pitch`. Camera position is derived. These are conceptual
responsibilities, not classes or APIs implemented by this gate.

The default view must show the origin and XY grid with Z visually upward.
Exact initial position and distance are parameters, not frozen constants.

### Navigation

- Orbit changes yaw/pitch while keeping target and distance fixed.
- Limit pitch with a small margin from both poles to avoid a view direction
  parallel to the +Z world-up axis. No exact limit such as 89 degrees is frozen.
- Pan moves in the camera image plane: horizontal motion follows camera right
  and vertical motion follows camera up. Camera and target move together.
- Perspective zoom changes camera distance.
- Orthographic zoom changes visible scale / visible height.
- Field of view is not the normal zoom mechanism.

Initial input defaults:

| Input | Action |
| --- | --- |
| Middle Mouse Drag | Orbit |
| Shift + Middle Mouse Drag | Pan |
| Mouse Wheel | Zoom |

These mappings may become configurable in the future.

## Rationale

An orbit model provides understandable navigation around a subject while
preserving the approved CAD coordinate system. Separating pan, orbit and zoom
keeps their effects explicit and avoids conflating navigation with lens changes.

## Consequences

- Camera/navigation remain viewer state, not CAD ownership.
- Pitch limiting must preserve a valid camera basis near the poles.
- Default view parameters and input tuning remain implementation parameters.
- B2 implementation requires explicit increment authorization.
- Projection semantics are defined in ADR-0011; rendering in ADR-0012.

## Alternatives Considered

- Free-flight camera: not selected for the initial orbit-oriented viewer.
- FOV-based normal zoom: not selected; distance/visible height have distinct semantics.
- Adapting CAD axes to rendering conventions: rejected; D1 remains authoritative.
