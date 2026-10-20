You are working on the `micro-solidworks` repository.

Your task is **not** to begin a broad UI rewrite immediately. Your first task is to perform a **formal UI/UX gap analysis and documentation alignment** based on the uploaded handoff package.

## Context

Use the following package files as authoritative inputs:

- `MICRO_SOLIDWORKS_UIUX_SPEC_V0_3.md`
- `VISUAL_REFERENCE_REGISTRY_V0_3.md`
- `CODEX_HANDOFF_UX_V0_3.md`
- `images/UX-REF-01-MODEL.png`
- `images/UX-REF-02-SKETCH.png`
- `images/UX-REF-03-MANUFACTURE.png`
- `images/UX-REF-04-EXPORT.png`

Project governance state to preserve:

- P0.FREEZE is approved and frozen.
- v1.2 remains READY FOR REVIEW and is **not authorized for implementation**.
- K1/K2/K3 kernel decision remains OPEN — EVIDENCE REQUIRED.
- Current executable baseline remains authoritative.
- No copying of third-party code, icons, branding or assets.

## Goal

Compare the **current executable UI and codebase** against the v0.3 UI/UX specification and the four canonical uploaded images. Produce a component-by-component audit and a safe incremental adoption proposal.

## Required output

1. A documented inventory of UI/UX components in the current codebase.
2. A gap matrix using:
   - `EXISTS`
   - `PARTIAL`
   - `MISSING`
   - `DEFERRED`
3. Evidence for each classification.
4. A proposal for the smallest safe next increment.
5. Clear separation between:
   - current-scope improvements,
   - future/visionary items,
   - governance-blocked items.

## Components to audit

- App shell / chrome
- Menus
- Workspace ribbon
- Model browser
- Viewport / canvas
- View cube / HUD
- Context inspector
- Status / guidance bar
- In-canvas callouts / dimensions
- Sketch-mode affordances
- Export / validation UX
- Future Manufacture-space affordances (document only, do not implement)

## Rules

- Do not start CAM implementation.
- Do not treat the Manufacture reference as authorized scope.
- Do not broaden modeling scope.
- Do not make architectural decisions outside approved governance.
- Update repository documentation for any findings you formalize.
- If you identify ambiguities, record them explicitly instead of guessing.

## Suggested first deliverable

Create a markdown report in the repository documenting the gap analysis and proposing a sequenced UI/UX adoption roadmap.
