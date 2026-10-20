# ADR-0071 — Configurable Navigation and Selection Appearance

Status: ACCEPTED

## Context

ADR-0010's initial MMB Orbit / Shift+MMB Pan mapping permits future configuration. The UI/UX v0.3 proposal instead depicts MMB Pan / Shift+MMB Orbit and blue selected geometry, while the validated executable and B4 highlight palette use orange. ADR-0052/0058 refer to existing middle-button conventions; alternatives require an explicit decision rather than a silent global switch.

## Decision

Two typed navigation profiles are available. **A (default)** routes MMB drag to Orbit, Shift+MMB drag to Pan and wheel to Zoom. **B** routes MMB drag to Pan, Shift+MMB drag to Orbit and wheel to Zoom. The profile selects existing Viewer/Interaction actions; it does not duplicate navigation algorithms or alter camera, projection, picking, Normal To or standard-view mathematics. When a confirmed profile change occurs during a captured drag, cancel that gesture safely and use the new routing only on the next MMB press. Preserve current camera pose.

One typed selected-geometry accent has **Orange (default)** and **Blue**. It colors selected logical targets and synchronized Feature Manager indication across sketch, solid, face and edge presentation and all display modes. It does not modify hover, active/editing, context/reference, normal, disabled/hidden, warning or error/FAILED/BLOCKED semantics. Selected remains distinct from and takes precedence over hover on the same target. Identity, picking and visibility never derive from the accent. Use semantic palette tokens and a non-color selection cue; validate contrast and distinguishability on dark background, Shaded, Shaded With Edges and Wireframe. The dialog may preview the draft accent locally before applying it; global Viewer preview is not required.

Application's preferences snapshot is passed explicitly to Viewer/Interaction and presentation-facing UI. Existing profile and orange remain the first-launch behavior. This is an approved opt-in extension of ADR-0010 and the current-input references in ADR-0052/0058; their default behavior remains unchanged.

## Rationale

The two UI/UX v0.3 compatibility conflicts can be addressed without silently changing validated workflows. Routing and palette policies are separate from camera math, selection identity and CAD state.

## Consequences

- Test both mappings, modifiers, wheel in both projection modes, active-drag profile switch, camera continuity, picking/selection, Normal To and standard views.
- Test selected/hover precedence and palette consistency for Feature Manager, sketch/solid/face/edge and each display mode, including accessibility cues and FAILED/BLOCKED distinction.
- Preferences changes produce no model rebuild, document dirty change or CAD Undo/Redo entry.

## Alternatives considered

- Change defaults to v0.3 behavior: rejected because it silently regresses frozen usage.
- Duplicate Orbit/Pan implementations by profile: rejected because input mapping is the only variation.
- Treat a single global accent as hover/tool/error color too: rejected because those are distinct semantic states.

## Deferred / non-goals

Arbitrary mouse/key remapping, theme system, advanced color picker, high-contrast mode and Viewer algorithm changes are outside SET-01.
