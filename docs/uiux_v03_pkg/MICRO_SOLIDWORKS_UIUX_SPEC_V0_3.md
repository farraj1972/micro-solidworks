# Micro SolidWorks — UI/UX Specification v0.3

Status: **PROPOSED — READY FOR OWNER REVIEW**.

## 1. Purpose

This document defines a formal UI/UX direction for Micro SolidWorks / Personal SolidWorks, grounded on four canonical visual references:

- UX-REF-01 — Model
- UX-REF-02 — Sketch
- UX-REF-03 — Manufacture
- UX-REF-04 — Export

The objective is to give Codex and future contributors a stable design target that is:

- original and product-owned;
- mouse-first and viewport-first;
- aligned with the approved P0 product strategy;
- compatible with incremental implementation;
- distinct from direct copying of any third-party UI, icons, assets or branding.

## 2. Scope boundary

This specification has **three horizons**:

### 2.1 Immediate horizon (applies to the current product direction)

- overall interaction philosophy;
- workspace shell;
- model browser behavior;
- contextual property inspector;
- viewport HUD;
- status and guidance patterns;
- export-flow UX principles.

### 2.2 Near-term horizon (candidate after v1.2 authorization)

- fuller sketch ergonomics;
- richer in-canvas editing;
- improved feature creation panels;
- export validation workflow;
- systematized command discovery.

### 2.3 Vision horizon (not currently authorized for implementation)

- Manufacture workspace;
- broader interoperability suite;
- deeper CAD→CAM chain;
- future scripting-centric extension surfaces;
- assembly/drawing parity targets.

## 3. Design north star

The product shall present itself as a **premium personal mechanical CAD**. The interface shall feel precise, calm and capable.

Normative character:

1. **Viewport-first** — the graphics area is the primary work surface.
2. **Mouse-first** — normal workflows do not require command IDs or technical text entry beyond legitimate parameter fields.
3. **Context-driven** — selection determines the most relevant actions and inspector content.
4. **Low-friction** — the user should complete common operations with short, discoverable sequences.
5. **Dense but readable** — professional information density without visual clutter.
6. **Direct manipulation** — use in-canvas dimensions, previews, handles and selection feedback whenever practical.
7. **Guided precision** — dimensions and constraints remain exact, but the interface explains state clearly.
8. **Single-product coherence** — Model, Sketch, Export and future Manufacture must feel like one product.
9. **Document safety** — UX must support safe save/open/export flows and clear failure recovery.
10. **Native identity** — do not copy copyrighted icons, branding or exact layouts from other CAD products.

## 4. Canonical application shell

The canonical shell is derived primarily from UX-REF-01 and UX-REF-02.

### 4.1 Global layout

The default desktop layout shall include:

- top title bar with app identity, active document name and command search;
- menu bar (`File`, `Edit`, `View`, `Insert`, `Tools`, `Manufacture`, `Drawing`, `Help` or equivalent approved set);
- workspace ribbon below the menu bar;
- left docked **Model Browser**;
- central **Viewport / Canvas**;
- right docked **Context Inspector**;
- bottom **Status & Guidance Bar**.

### 4.2 Responsiveness

- The viewport shall retain priority under resizing.
- Side panels may collapse, pin or auto-hide in future iterations.
- Minimum target desktop width should preserve simultaneous visibility of tree + viewport + inspector.

## 5. Visual language

### 5.1 Theme

- Default theme: dark graphite / blue-accent premium technical UI.
- Primary accent: luminous blue for active tools, selected tabs and highlighted geometry.
- Positive validation: green.
- Caution: amber/yellow.
- Error: red.
- Neutral text hierarchy: bright white for active labels, subdued gray for secondary text.

### 5.2 Surface hierarchy

1. Background shell.
2. Workspace bars.
3. Dock panels.
4. Viewport.
5. Floating overlays / micro toolbars / callouts.

### 5.3 Typography

- Clean sans-serif UI typography.
- Numeric values and dimensions must be highly legible.
- Dimension labels should be readable directly in the viewport.

## 6. Workspace contracts

## 6.1 Model workspace (UX-REF-01)

Purpose: part modeling, feature editing, model inspection.

Required characteristics:

- feature/model browser at left;
- ribbon emphasizing New/Open/Save/Rebuild and core modeling tools;
- viewport showing shaded model with selection highlighting;
- in-canvas dimensions and preview geometry when relevant;
- right-side contextual property panel bound to the selected command or feature;
- view cube and compact viewport tool strip;
- bottom guidance message for the active command.

For the current product stage, the Model workspace is the primary executable target.

## 6.2 Sketch workspace (UX-REF-02)

Purpose: create and edit 2D sketch geometry, dimensions and constraints.

Required characteristics:

- sketch-specific ribbon with geometry and relation tools;
- orthographic 2D grid canvas with visible X/Y orientation cues;
- direct entity selection and editable dimensions;
- clear indication of selected entities;
- floating micro-toolbar close to selected geometry when appropriate;
- right inspector split between Properties and Constraints;
- synchronized model-browser selection;
- obvious action for exiting sketch mode.

## 6.3 Manufacture workspace (UX-REF-03)

Purpose: future CAD→CAM workflow.

Status: **visionary / future scope**.

Required characteristics when authorized:

- operation-oriented ribbon;
- manufacturing setup/operation tree;
- viewport capable of showing stock, workholding, tool, toolpath and WCS;
- operation parameter inspector;
- bottom simulation controls and machining timeline;
- validation/status messaging.

This reference shall influence long-term architecture and UX consistency, but does not authorize CAM implementation now.

## 6.4 Export workspace (UX-REF-04)

Purpose: export and validate geometry for exchange and fabrication.

Required characteristics:

- command set for export/import/validation;
- left navigation for data exchange tasks;
- viewport capable of comparing exact CAD and derived mesh representations;
- right validation panel with file format, settings, units and destination;
- explicit machine-readable validation checklist;
- status line indicating export readiness.

For near-term product use, this workspace strongly informs STL export UX.

## 7. Core interaction model

### 7.1 Selection

- Hover pre-highlight shall precede selection when feasible.
- Selected geometry must receive a visually strong blue highlight.
- Tree selection and viewport selection shall remain synchronized.
- Active selection should drive inspector content.

### 7.2 Command lifecycle

Every user-facing command should follow this pattern where appropriate:

1. Invoke command.
2. Show guidance in status bar.
3. Highlight acceptable selection targets.
4. Show preview.
5. Allow parameter refinement in inspector and/or in-canvas controls.
6. Confirm or cancel.
7. Return to a stable post-command state.

### 7.3 Mouse semantics

- left click: select / place / confirm pick step;
- drag: manipulate view or geometry depending on mode and context;
- scroll wheel: zoom;
- middle mouse: pan;
- right click: context actions;
- `Esc`: cancel active sub-step or clear transient state.

### 7.4 In-canvas feedback

- dimensions, callouts, arrows and preview outlines are preferred for geometric operations;
- overlays should remain legible but non-obstructive;
- floating micro-toolbars should be compact and contextual.

## 8. Component contracts

The following conceptual components are authoritative UX building blocks:

1. **AppChrome** — title bar, menus, search.
2. **WorkspaceRibbon** — workspace tabs and command groups.
3. **ModelBrowser** — tree of model entities/features/operations.
4. **ViewportCanvas** — 3D or 2D work area.
5. **ContextInspector** — right-side property/parameter panel.
6. **ViewportHUD** — view cube, compact view tools, in-view toggles.
7. **StatusGuidanceBar** — bottom instructions and model state.
8. **FloatingMicroToolbar** — small nearby actions for selected entities.
9. **DimensionCallout** — in-canvas editable measurement labels.
10. **ValidationChecklist** — machine-readable readiness results.
11. **DataExchangeNavigator** — export/import task navigation.
12. **OperationsTree** — future CAM operation hierarchy.
13. **SimulationTimeline** — future CAM playback and legend.

## 9. Mapping to current product governance

### 9.1 What is in-scope for the current product direction

- formalize the shell and workspace philosophy;
- align the existing CAD UI with viewport-first, mouse-first principles;
- improve model browser semantics;
- improve contextual inspector behavior;
- inform STL export UX;
- define future consistency targets.

### 9.2 What remains outside current authorization

- CAM implementation;
- generalized data-exchange implementation beyond approved scope;
- unapproved modeling commands;
- kernel changes or architectural decisions not yet accepted.

## 10. Experience Gates

Any executable UX-facing increment should be reviewed against:

1. **Discoverability** — can a mechanically CAD-literate user find the required commands?
2. **Workflow continuity** — does the sequence feel coherent without technical IDs?
3. **Selection confidence** — is it obvious what is selected and what can be picked next?
4. **Parameter clarity** — are parameter fields and units understandable?
5. **State clarity** — is it obvious when the user is in Sketch, Model or another mode?
6. **Error recovery** — do invalid states provide actionable feedback?
7. **Visual coherence** — does the implemented result remain faithful to the visual north star?

## 11. Open UX questions

1. How much ribbon density is appropriate for the current ImGui shell?
2. What should auto-hide/pinning behavior be for side panels?
3. Should command search be implemented early or deferred?
4. What is the minimum acceptable in-canvas dimension editability for v1.x?
5. When should floating micro-toolbars appear?
6. How should dark-theme accessibility contrast be measured and tested?
7. What is the minimum export validation panel for first STL release?
8. Which manufacture-space elements should be represented architecturally before implementation?
9. What is the right balance between exact inspector control and direct manipulation?
10. How should future scripting entry points surface in the UI?

## 12. Codex implementation guidance

Codex should treat this document as a **design contract**, not as blanket authorization to build all depicted functions.

Codex shall:

- compare the current executable UI to this specification;
- identify `EXISTS`, `PARTIAL`, `MISSING` and `DEFERRED` components;
- preserve current product governance and campaign boundaries;
- propose incremental adoption steps;
- avoid direct copying of any third-party visual assets.

## 13. Recommended next Codex task

Before any UI rewrite, perform a **documented gap analysis** between the current executable UI and:

- `MICRO_SOLIDWORKS_UIUX_SPEC_V0_3.md`
- `VISUAL_REFERENCE_REGISTRY_V0_3.md`
- the four canonical image references in `/images`

Expected output:

- a component-by-component audit;
- screenshot/evidence mapping where possible;
- a proposed implementation roadmap divided into:
  - shell alignment,
  - model workspace improvements,
  - sketch workflow improvements,
  - export workflow UX,
  - future/visionary items.
