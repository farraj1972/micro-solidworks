# UX-04-REVIEW — Model Workspace Completion Assessment

Status: **READY FOR OWNER DECISION — recommendation only; UX-04 remains IN PROGRESS / NOT FROZEN** (2026-10-23).

## Executive summary and baseline

Recommend **B — proceed to a final UX-04 validation and freeze**, without an additional executable UX-04.3. UX-04.1 and UX-04.2 corrected the demonstrated Model selection and context problems, and the owner approved both Experience Gates. No remaining P0 or demonstrated P1 blocker was found in the tested mechanical-part workflow. The remaining differences from the canonical Model image are visual refinement, scale-evidence gaps, or future capabilities. This review does not authorize or perform the freeze.

The reviewed executable HEAD is `b710f48` on `master`, with a clean entry working tree. Personal SolidWorks v1.2.0, P0, SPEC-01, SET-01, UX-01, UX-01.R1 and UX-02 remain FROZEN. UX-04.1 and UX-04.2 are COMPLETE; UX-04 is NOT FROZEN; UX-03 Export UX is PLANNED / NOT AUTHORIZED. All 71 tracked ADRs are ACCEPTED. Kernel K1/K2/K3 stays OPEN. No accepted CAD ownership, identity, persistence, renderer boundary, dependency or UI-framework decision is changed here.

## Closure evidence for UX-04.1 and UX-04.2

| Slice | Delivered and observed | Evidence and limit |
| --- | --- | --- |
| [UX-04.1](UX_04_1_MODEL_SELECTION_VALIDATION.md) | Exact face/edge picks keep their logical identity while derived connected coplanar-face and straight-edge highlight regions remove misleading visual breaks. Pocket-floor edges draw over filled faces with depth-aware rendering. Shaded, Shaded With Edges and Wireframe preserve their distinct semantics; hover and selected colors remain distinct. Browser/viewport/Inspector selection, edit/rebuild, Save/Open and both SET-01 accents passed the owner gate. | Targeted V2 build/tests, including picker and framebuffer regressions; owner Experience Gate **APPROVED** and normal exit. Earlier failed candidates and the corrected overlay/picking defects are closed, not current gaps. No full V3 was run for this slice. |
| [UX-04.2](UX_04_2_BROWSER_INSPECTOR_VALIDATION.md) | Browser derives Part, reference planes, ordered Modeling Features, unconsumed Sketches and each absorbed Sketch once under its first consumer. Name shortening with full tooltip, reserved icon/status space, tree expansion, states, feature context and right-click actions are readable. Inspector groups identity, properties, relationships, applicable actions and diagnostics; it keeps Confirm/Cancel beside editable parameters and names failed/blocked causes. Logical selection survives or is reconciled after rebuild as appropriate. | Incremental V2 build, 32/32 directed tests and 11/11 recheck after the icon/text overlap fix. Owner retested the overlap **PASS**, then approved the complete context/edit/history/Save/Open/Sketch/preferences gate and closed normally. The overlap is resolved; no new evidence reopens it. |

The reference workflow `Open Part → Select Feature → Inspect → Edit → Confirm/Cancel → Select Face/Edge → Sketch-on-Face → Rebuild → Undo/Redo → Save/Open` was covered across the two approved owner gates using a pocketed mechanical part. The record supports that workflow on the tested build; it does not claim exhaustive usability or pixel conformity.

## Visual conformance and remaining gaps

The four canonical PNGs were opened directly and their SHA-256 values match the [v0.3 visual registry](uiux_v03_pkg/VISUAL_REFERENCE_REGISTRY_V0_3.md). [UX-REF-01-MODEL](uiux_v03_pkg/images/UX-REF-01-MODEL.png) and the [UX-01.R1 contract](UX_01_MODEL_WORKSPACE_CONTRACT.md) govern Model composition: document/menu/command area, left Browser, dominant Graphics Area, right Inspector and status guidance. [UX-REF-02-SKETCH](uiux_v03_pkg/images/UX-REF-02-SKETCH.png) informs the common shell and already-frozen UX-02 behavior. [UX-REF-03-MANUFACTURE](uiux_v03_pkg/images/UX-REF-03-MANUFACTURE.png) is future CAM vision only. [UX-REF-04-EXPORT](uiux_v03_pkg/images/UX-REF-04-EXPORT.png) informs a separate future Export UX campaign; its split preview, broader formats and illustrated checks are not current product behavior. None of the PNGs authorizes the pictured future CAD/CAM commands, artwork copying or a pixel-perfect layout.

| Area / responsible component | State | Priority | Evidence, remaining difference and productivity impact |
| --- | --- | --- | --- |
| Model face/edge feedback — Presentation/WorkspaceViewport | **ALIGNED** in bounded scope | P0 guard, closed | Corrected pocket wall and floor-edge display/picking passed framebuffer fixtures and owner retests across three display modes. A new regression would reopen this, but none is evidenced. |
| Browser hierarchy, absorbed Sketches, statuses and long names — FeatureTreeModel/ApplicationShell | **ALIGNED** in bounded scope | P1, closed | Owner approved hierarchy, tooltip/spacing correction, context actions and synchronized selection. The previously observed icon/name overlap is resolved. |
| Inspector identity, properties, relationships, diagnostics and edits — ApplicationShell/EditorContext | **ALIGNED** in bounded scope | P1, closed | Owner approved Feature/Sketch/Face/Edge contexts and Cancel/Confirm/Undo/Redo/Rebuild. Inspection alone must stay read-only. A current final-build screenshot of an error/draft state is not archived; this is an evidence limit, not a demonstrated failure. |
| Document header and Model/Sketch hierarchy — ApplicationShell | **PARTIAL** | P2 | Native menu precedes in-app document header; current filename/dirty marker and text mode heading work, but the reference has a stronger identity/mode tier. Source shows the real title, dirty `*` with tooltip and explicit MODEL/SKETCH heading. Owner completed the workflow without a mode-discovery failure after UX-04.2. |
| Command ribbon — ApplicationShell | **PARTIAL** | P2 | Original icons, visible labels, tooltips, active/disabled state and supported-only commands work. One flat strip and simple separators lack the reference's semantic grouping and rhythm. Disabled tooltips explain prerequisites. No measured extra action or blocked workflow remains in the owner gate. |
| Browser name search and body folders — ApplicationShell/FeatureTreeModel | **DEFERRED** | P3; reassess on larger histories | Search is absent and current folders are derived history groups, not persistent Bodies. The tested part remained navigable; no large-tree time/error evidence establishes a P1 need. A later filter must preserve parents, expansion and logical selection without a second CAD model. |
| Graphics Area scale, contrast, grid and model framing — WorkspaceViewport/ApplicationShell | **PARTIAL** | P2 | Derived solids, Fit, projection, display modes, grid/axes, selection and pick feedback work. The reference shows a larger/more staged model and richer depth; its geometry and camera differ from the actual pocket fixture, so this is not evidence of a Fit defect. Same-fixture captures would be needed to claim one. |
| HUD/orientation — ApplicationShell/WorkspaceViewport | **PARTIAL** | P2 cue polish; P3 interactive cube | Fit/ISO/Views, six standard views, display selector and X/Y/Z cues are real. Their compact text treatment is less expressive than the reference triad/cube. An interactive View Cube would add navigation behavior and needs separate authorization; a dead cube must not be drawn. |
| Typography, clipping and Windows scale — ApplicationShell/UI layout | **NOT VERIFIED** at 125%/150% | Evidence risk; P1 only if failure reproduced | UX-01 corrected the major font issue and UX-04.2 corrected row overlap. Directed frame tests covered 1366×768 and 1920×1080, while owner final resolution/scaling were not recorded. Those tests cannot prove actual Windows 125% or 150% readability, clipping, hit targets or scroll reach. No confirmed HiDPI defect exists. Record this omission explicitly; test available scales during final gate where practical. |
| Status/guidance — ApplicationShell | **PARTIAL** | P2 | Mode, next step and result are visible, but the sparse two-line treatment differs from the reference. The owner completed the named workflow without reporting a guidance blocker. |
| Export and Manufacture references | **DEFERRED** | P3 relative to UX-04 | Frozen STL export remains valid; richer result/preview belongs to UX-03. Manufacture remains unapproved vision. |

**P0:** none open. **P1:** none demonstrated after the approved owner gates. **P2:** header/mode/ribbon visual hierarchy, viewport staging, HUD/status treatment and visual scale polish. **P3:** search pending a larger-tree need, interactive View Cube, derived body presentation and future Export/Manufacture capabilities. **NOT VERIFIED:** manual 125%/150% Windows scaling, exact screen scaling/resolution of the final UX-04.2 owner run, and final-state screenshots for every Inspector state. These are evidence limits, not passed checks or reported defects. A reproducible clipped control, missing required action or failed owner workflow at supported scale would become P1 and invalidate the direct-freeze recommendation.

## Decision matrix

LOW/MEDIUM/HIGH compare relative benefit, cost and risk for the *currently evidenced* Model workflow. For effort, risk and time, LOW is preferable.

| Criterion | A — UX-04.3 | B — direct freeze | C — bounded UX-04.3 then freeze |
| --- | --- | --- | --- |
| Productivity benefit | LOW: current workflow passed; polish may improve scanning | HIGH: unlocks next workflow campaign without reopening passed work | LOW unless a concrete new P1 is reproduced |
| CAD validation benefit | LOW: little new behavioral evidence from styling alone | HIGH: final baseline validation and preserved owner gate provide a clear handoff | MEDIUM only if a reproduced P1 is corrected and retested |
| Effort | HIGH if header, ribbon, search, HUD and DPI are bundled | LOW for review/freeze work; V3 still required | MEDIUM for one evidenced defect plus V3 |
| Risk | MEDIUM: UI regression and growing scope | LOW for unchanged executable, conditional on final checks | MEDIUM: limited executable regression surface |
| Architecture impact | LOW if strictly visual; MEDIUM if search/cube invent identity/navigation contracts | LOW: no contract change | LOW only with bounded, approved behavior |
| Time until freeze | HIGH | LOW | MEDIUM |

**Recommendation: B.** It is supported by the approved, end-to-end Model workflow and the lack of an evidenced current P1. Option A would conflate several P2/P3 improvements. Option C is the fallback if final checks or owner review reproduce a concrete P1; it is not pre-authorized by this document.

## Proposed next action, risks and acceptance

The next action is a **separately authorized UX-04 final validation/freeze**, not UX-04.3 implementation. Its executable scope is zero unless validation exposes a defect. Preserve current CAD identities, FeatureModel authority, EditorContext ownership, `.microsw` persistence, STL, SET-01 defaults and the frozen UX-01/UX-02 contracts.

1. Confirm executable HEAD has not changed since the approved UX-04.2 gate. Perform final **V3**, because this is the meaningful UX-04 campaign freeze and no UX-04 V3 has yet occurred: fresh configure/full build, full CTest, runtime smoke of the final build, architecture/dependency audit, repository hygiene and `git diff --check`. Do not interpret this V0 review as that validation.
2. Reuse the owner's UX-04.1/04.2 Experience approvals for the unchanged behavior; perform a short final clean-build smoke of the reference workflow and normal exit. Record actual display resolution and Windows scale. If 125%/150% is available, check Browser rows, ribbon labels/tooltips, Inspector actions/scroll and viewport HUD at 1366×768 and 1920×1080 where feasible. If unavailable, mark **NOT VERIFIED** rather than PASS. A newly observed functional P1 requires a bounded correction and new technical/owner retest before freeze.
3. Only after technical V3 and owner closure pass, update UX-04 status to FROZEN and record accepted P2/P3 debt plus HiDPI evidence limitations. Commit the validated state and add a UX-04 tag **only if separately authorized for the freeze**. If executable HEAD changes after V3, rerun affected validation and, when required by the baseline rule, V3. A docs-only freeze after unchanged executable HEAD uses V0 and does not repeat V3.

Principal risk: manual HiDPI or a larger-tree workflow could reveal a productivity defect that targeted tests did not cover. The response is to classify the concrete observation, correct only its affected closure under a newly approved scope, and retain UX-04 as NOT FROZEN until revalidated. Missing screenshots alone do not establish a functional failure. A request for richer command grouping or appearance after freeze can be tracked as future P2 design work without revising an accepted ADR.

UX-03 Export UX should start **after UX-04 freeze**, under its own authorization and Experience Gate. It should use UX-REF-04 to improve the real STL validator/result flow while preserving the frozen v1.2.0 export contract. CAD/mesh split preview, additional formats, printability claims and Manufacture remain outside this recommendation.

## Review boundary

This V0 assessment inspects the documented tests/gates, the actual UI/Viewer/Presentation/Interaction implementation, the canonical PNGs and the ADR inventory. It does not run the app, perform a new visual gate, change code, create an ADR, freeze UX-04, start UX-03 or alter any frozen baseline. [The v0.3 gap matrix](UIUX_V0_3_GAP_ANALYSIS.md) and [roadmap](ROADMAP.md) carry the decision status until the owner chooses the next action.
