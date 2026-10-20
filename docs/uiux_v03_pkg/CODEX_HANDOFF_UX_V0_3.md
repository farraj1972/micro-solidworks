# Codex Handoff — Micro SolidWorks UI/UX v0.3

Status: **READY FOR CONTEXT TRANSFER**.

## 1. What changed in this revision

This v0.3 handoff corrects a previous issue: the package now uses the **exact four user-uploaded images** as the canonical visual references.

These images are included locally in the package and registered in `VISUAL_REFERENCE_REGISTRY_V0_3.md`.

## 2. Project/product context

Project: **Micro SolidWorks** / working product vision **Personal SolidWorks**.

Current strategic state:

- P0.FREEZE: **PASS / FROZEN**.
- Product strategy frozen; v1.2 remains **READY FOR REVIEW**.
- No implementation authorization has been granted yet for the detailed v1.2 plan.
- Kernel decision K1/K2/K3 remains **OPEN — EVIDENCE REQUIRED**.
- Micro SolidWorks `v1.1.0` remains frozen and authoritative as the current executable baseline.

## 3. Relevant product strategy points

The approved product direction is:

- personal parametric 3D mechanical CAD;
- workflow-first over feature accumulation;
- first bounded N2 workflow: parametric adapter plate for 3D-print preparation;
- native document integrity first;
- validated STL export prioritized before broader interchange;
- no copying of third-party branding, code, UI, icons or assets.

## 4. First bounded workflow reminder

The first N2 target remains a bounded mechanical workflow centered on a synthetic adapter plate:

- create sketch;
- constrain/dimension;
- additive extrude;
- one axial cut for pocket;
- edit/rebuild;
- save/open;
- export STL;
- validate STL in independent consumer.

The uploaded reference images are a **UI/UX design north star**. They do **not** redefine the approved reference part geometry or the authorized campaign scope.

## 5. Canonical visual references

Use these four files as the authoritative UI/UX visual anchors:

1. `images/UX-REF-01-MODEL.png`
2. `images/UX-REF-02-SKETCH.png`
3. `images/UX-REF-03-MANUFACTURE.png`
4. `images/UX-REF-04-EXPORT.png`

Their role is to establish:

- visual hierarchy;
- workspace composition;
- interaction density;
- viewport prominence;
- panel behavior expectations;
- design language continuity.

## 6. Primary interpretation to preserve

The images collectively imply:

- premium dark technical UI;
- wide-screen desktop orientation;
- strong viewport dominance;
- left tree + right inspector layout;
- ribbon/workspace model;
- in-canvas dimensions and callouts;
- compact but readable bottom guidance/status line;
- consistent view/orientation widgets;
- mouse-centered interaction.

## 7. Codex tasking intent

This handoff does **not** instruct Codex to immediately rebuild the UI. The first task should be **analysis and planning**, not blind implementation.

Codex should:

1. inspect the current UI implementation in the repository;
2. compare it to the v0.3 specification and the four uploaded images;
3. produce a gap matrix using these states:
   - `EXISTS`
   - `PARTIAL`
   - `MISSING`
   - `DEFERRED`
4. separate items into:
   - current-scope candidates;
   - future-scope / visionary references;
   - blocked-by-governance items.

## 8. Expected deliverables from Codex (first pass)

- updated documentation aligning repository docs with the v0.3 UX package;
- component inventory and evidence matrix;
- proposed incremental roadmap;
- recommendations for the smallest safe UX alignment slice.

## 9. High-priority component audit list

Codex should specifically audit:

- application shell/chrome;
- top menus;
- workspace ribbon/tabs;
- model browser/tree;
- viewport composition;
- view cube / viewport HUD;
- right-side context inspector;
- in-canvas dimensions/callouts;
- status/guidance bar;
- sketch-mode affordances;
- export workflow affordances.

## 10. Governance rules for Codex

- Do not broaden scope beyond explicit approvals.
- Do not assume Manufacture/CAM is authorized now.
- Do not treat future-looking buttons in the references as mandatory current implementation.
- Preserve ADR boundaries and dependency policy.
- Keep documentation updated if any work is proposed or performed.
- Prefer executable vertical slices and user-visible progress.

## 11. Recommended next action

Create a **UI/UX gap-analysis increment proposal** grounded on this package.

That proposal should identify:

- what can be aligned with current v1.1.0/v1.2 direction without architectural risk;
- what should wait for v1.2 approval;
- what belongs to later roadmaps (interoperability expansion, Manufacture, scripting surfacing, etc.).
