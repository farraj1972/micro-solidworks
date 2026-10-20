# SET-01 — Application Preferences First Slice

Status: **PROPOSED — READY FOR REVIEW; NOT AUTHORIZED FOR IMPLEMENTATION**. The [SET-D01 proposal](PERSONAL_SOLIDWORKS_SETTINGS_PREFERENCES_PROPOSAL.md) and proposed [ADR-0070](adr/ADR-0070-application-preferences-ownership-and-persistence.md) / [ADR-0071](adr/ADR-0071-configurable-navigation-and-selection-appearance.md) must be reviewed and frozen before this campaign can be authorized. This plan changes neither v1.1.0 nor the proposed v1.2-C1–C4 state.

## Goal and bounded workflow

A user opens Edit → Preferences, selects either existing or alternative CAD navigation and orange or blue selection, previews the accent, applies or cancels, and sees the choice retained after restart. The same CAD document remains clean and its model/history unchanged. First launch remains Profile A plus Orange.

| Proposed step | Scope | Targeted evidence |
| --- | --- | --- |
| **SET-01.1 — Typed Preferences and User Storage** | Application-owned schema/defaults/validation, LocalAppData file, atomic write/backup/recovery, explicit immutable snapshot. No UI/Viewer behavior yet. | Unit/contract tests for absent, partial, unknown, invalid, corrupt and future-version files; rollback; no `.microsw`/CAD-history mutation. V1 affected Application/storage closure. |
| **SET-01.2 — Navigation and Selection Integration** | Route Profile A/B through existing Orbit/Pan/Zoom; apply semantic Orange/Blue selected palette consistently in Viewer/Presentation-facing UI. Avoid camera-algorithm duplication and new CAD identity. | Viewer/Interaction and presentation integration tests, both projections, drag switch, picking, Normal To/views, sketch/solid/face/edge and display modes. V2 affected closure and runtime smoke. |
| **SET-01.3 — Preferences UI and Experience Gate** | Edit → Preferences with draft, local accent preview, Apply/OK/Cancel/Restore Defaults, validation and persistence error feedback. Show only implemented Navigation/Appearance options. | UI/application tests for transactional semantics, persistence/restart, dirty/Undo invariants, accessibility cues; owner Experience Gate on a real edited part. V2 targeted integration plus runtime. |
| **SET-01.FREEZE** | Review evidence, accepted ADR conformity, documentation/roadmap and release decision. | V3 only if designated as a baseline freeze or concrete integration risk warrants it; otherwise use the required gate evidence and V0 documentary closure under `AGENTS.md`. |

The split keeps storage, routed behavior and user workflow reviewable without creating a micro-increment for each enum or color. Before each increment, derive the changed dependency closure and run affected tests/builds only. UI/Viewer changes require runtime and Experience Gate evidence; full regression is reserved for a justified baseline/integration gate. Do not automatically rebuild independent Geometry/Topology/Modeling suites.

## Experience Gate

With an existing mechanical part open, the user can find Preferences without internal IDs, identify the active mapping and accent, switch A ↔ B, orbit/pan/zoom and keep camera/picking/selection coherent, use Normal To and standard views, distinguish selected from hovered and FAILED/BLOCKED in viewport and Feature Manager, test all display modes, Apply and restart, then Cancel an unapplied edit and Restore Defaults deliberately. CAD dirty and Undo/Redo must remain unchanged. An active drag cannot change action mid-gesture. A storage failure must be explained without claiming Apply succeeded. Keyboard and non-color selected-state feedback must be usable. Record observable PASS/failures and any unavailable accessibility scenario.

## Dependencies and exclusions

SET-01 depends on owner approval/freeze of SET-D01 and explicit campaign authorization. UX-01 may later consume this preference snapshot but need not wait for an unrelated full UI restructuring. SET-01 is cross-cutting and is **not an automatic blocker** for the proposed v1.2 first-N2 workflow or STL export; only a demonstrated workflow dependency can change that order. V1.2-C1–C4 and UX-01 remain separately unauthorized. Kernel K1/K2/K3 remains open.

Excluded: arbitrary bindings, macros, complex themes, advanced font scaling, per-workspace profiles, cloud/accounts/telemetry, export preferences, CAM configuration, Document Settings, session persistence, CAD schema edits and new geometry/modeling features. No new external dependency, architecture substitution or accepted ADR change is implied by this proposal.
