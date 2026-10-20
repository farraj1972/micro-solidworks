# Visual Reference Registry v0.3

Status: **CANONICAL FOR UI/UX SPECIFICATION v0.3**.

This registry supersedes the earlier visual-reference mapping. The four images in `/images` are the **sole primary visual references** for the current UI/UX specification and Codex handoff package.

## UX-REF-01 — Model

- File: `images/UX-REF-01-MODEL.png`
- SHA-256: `87107d8e8579312aa09f14e8f37c879f2dfb49c63860cc3667563b21b49d5744`
- Workspace: **Model**
- Role: canonical reference for the 3D modeling workspace.
- Primary cues captured:
  - dark premium theme with blue accents;
  - viewport-first composition;
  - top menu + workspace ribbon;
  - left feature/model browser;
  - right context inspector/property panel;
  - in-canvas dimensions and selection highlighting;
  - view cube + compact viewport HUD;
  - bottom status/instruction bar.

## UX-REF-02 — Sketch

- File: `images/UX-REF-02-SKETCH.png`
- SHA-256: `9d09cc27ad7840810280de8f436da590f91e63e365f298c4d665e1f6eff0bc96`
- Workspace: **Sketch**
- Role: canonical reference for the 2D sketching workspace.
- Primary cues captured:
  - sketch-focused ribbon with geometry and constraint tools;
  - orthographic sketch canvas with grid and axes;
  - mouse-first dimension editing and direct selection;
  - floating micro-toolbar near selected entities;
  - right panel divided into Properties and Constraints;
  - model browser synchronized with active sketch.

## UX-REF-03 — Manufacture

- File: `images/UX-REF-03-MANUFACTURE.png`
- SHA-256: `95da8c7ca1d91cfda88bd8d9123122eaf9b20925662e26e432985a75bd0cadaa`
- Workspace: **Manufacture**
- Role: canonical reference for the future CAD→CAM workspace.
- Primary cues captured:
  - manufacturing ribbon with setup, tool, operation and post-process actions;
  - operations tree at left;
  - stock, fixturing, tool and toolpath visualization in viewport;
  - operation inspector on the right;
  - simulation/timeline bar at bottom;
  - machining status and operation diagnostics.

## UX-REF-04 — Export

- File: `images/UX-REF-04-EXPORT.png`
- SHA-256: `11f636e0c34d0e451e69e802cd197a99bda8105f5566b3ab72c6cc85a741a8c7`
- Workspace: **Export / Interoperability**
- Role: canonical reference for STL/export validation workflows.
- Primary cues captured:
  - export-focused ribbon;
  - left-side data exchange navigation;
  - split CAD vs mesh preview in viewport;
  - validation-oriented right panel;
  - explicit STL options, units and destination;
  - machine-readable validation checklist.

## Usage policy

- These images are **reference inputs for design intent**, not bitmaps to be copied into the product.
- Codex may use them to derive layout, hierarchy, interaction patterns and component contracts.
- Codex must **not** infer that all illustrated commands are already approved for implementation in the current campaign.
- Where the images show future-scope capabilities (for example Manufacture), treat them as **visionary references** unless explicitly authorized by product governance.
