# UI/UX v0.3 — Reference inventory and adoption analysis

Status: **DOCUMENTED COMPLIANCE ANALYSIS / UX-01 FROZEN / UX-01.R1 APPROVED AND FROZEN** (2026-10-21). UX-02 is authorized and in progress; the UI/UX v0.3 source package remains a proposed design reference.

The [UX-01.R1 canonical workspace contract](UX_01_MODEL_WORKSPACE_CONTRACT.md) is the detailed design specification. This matrix gives component-level traceability without importing pictured future commands into executable scope. The four canonical PNGs and both actual Model captures were inspected directly. Functional qualifications come from [UX-01 validation](UX_01_CAD_WORKSPACE_VALIDATION.md), [SET-01 validation](PERSONAL_SOLIDWORKS_SET_01_VALIDATION.md) and current UI source. `NOT VERIFIED` means evidence is absent, not that a feature failed.

## UX-01.R1 conformance matrix

States: `ALIGNED`, `PARTIALLY ALIGNED`, `MISALIGNED`, `MISSING`, `DEFERRED`, `NOT VERIFIED`. Priorities: P0 critical regression/workflow; P1 high-value alignment; P2 visual refinement; P3 future capability. Campaigns below are candidates, not implementation authorization.

| Component | Canonical Reference | Current State | Observed Gap | Target Contract | Priority | Implementation Campaign | Acceptance Evidence |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Application header / document | UX-REF-01/02 | PARTIALLY ALIGNED: native title, document header, dirty state | Menu precedes identity; search is future. | One clear identity/document/dirty hierarchy; no fake search. | P1 | Later Model refinement, number undecided | Model idle/dirty captures; save/open title behavior. |
| Main menu | UX-REF-01/02 | ALIGNED functionally: File/Edit/View/Help | Less visual separation; mockup contains future menus. | Native-like keyboard/file actions; no empty domain menu. | P2 | Later Model refinement | Menu/keyboard smoke at 1366×768. |
| Workspace mode/tabs | UX-REF-01/02 | PARTIALLY ALIGNED: real Model/Sketch contexts | Small heading rather than clear tab tier. | Text-and-shape active context; only implemented modes enabled. | P1 | UX-02 and later Model refinement | Paired Model/Sketch screenshots and entry/exit test. |
| Ribbon | UX-REF-01/02 | PARTIALLY ALIGNED: contextual original icons, labels, tooltips, disabled states | Flat one-row strip, few named command families. | Semantic groups, active/disabled reasons, no clipped commands. | P1 | UX-02 Sketch slice; later Model refinement | 1920/1366 and DPI captures where available; prerequisites test. |
| Browser structure | UX-REF-01/02 | PARTIALLY ALIGNED: planes, ordered features, absorbed sketches, context actions | No body/sketch group or search; simpler hierarchy. | Preserve feature order and single absorbed Sketch node; only derived grouping. | P1 | Later Model refinement, scope decision | Tree/viewport selection, right click, FAILED/BLOCKED. |
| Browser legibility | UX-REF-01/02 | ALIGNED for fixed overlap/font defect | DPI and visual rhythm not verified. | Readable rows, nonoverlapping icons, distinct hover/selected/error. | P0 regression / P2 refinement | UX-02 regression and later refinement | Before/after actual captures; 125/150% NOT VERIFIED. |
| Graphics Area / framing | UX-REF-01 | PARTIALLY ALIGNED: derived solid, picking, camera/Fit | Small model on flat background; different fixture limits comparison. | Dominant pickable viewport, predictable Fit, subtle technical depth. | P2 | Later Model refinement | Same-fixture Fit capture and pick test. |
| Display modes and highlight | UX-REF-01 | ALIGNED functionally: three modes, hover and selection | Static idle image cannot prove all modes. | Preserve Shaded, Shaded With Edges, Wireframe and distinct hover/selection. | P0 regression | Each affected UX gate | Three-mode screenshots/tests; SET-01 evidence. |
| HUD / orientation | UX-REF-01/02 | PARTIALLY ALIGNED: Fit/ISO/Views, X/Y/Z cues | No interactive View Cube or reference triad treatment. | Compact standard views; cube only after authorization. | P2 / P3 cube | Later Model refinement; cube deferred | Standard views/projection checks; no fake cube. |
| Inspector idle/selected | UX-REF-01/02 | PARTIALLY ALIGNED: contextual feature/plane/face/edge UI in source | Actual capture shows sparse idle; weaker section/value hierarchy. | Helpful idle; grouped identity, facts and valid actions when selected. | P1 | UX-02 Sketch; later Model refinement | Idle/feature/face/edge captures; read-only leaves dirty/Undo unchanged. |
| Inspector edit/diagnostic | UX-REF-01/02 | PARTIALLY ALIGNED: drafts, Confirm/Cancel, FAILED/BLOCKED in source | No archived executable edit screenshot. | Stable actions, units, inline validation and actionable diagnostic. | P1 | UX-02 and later Model refinement | Edit/cancel/confirm/Undo and failure captures. |
| Sketch canvas/dimensions | UX-REF-02 | MISSING callouts; executable visual NOT VERIFIED | Reference shows canvas dimensions and constraint marks; current edits in Inspector. | Normal 2D grid/X/Y and real synchronized dimensions when authorized. | P1 | UX-02 NEXT CANDIDATE, NOT STARTED | Archived Sketch capture and edit/solve/invalid/cancel gate. |
| Context micro-toolbar | UX-REF-02 | MISSING visually; context menus exist | No actions beside selected geometry. | Only supported actions; no occluded pick target or advanced manipulation claim. | P1 candidate | UX-02 only if explicitly scoped | Local action/cancel test and screenshot. |
| Status/guidance | UX-REF-01/02 | PARTIALLY ALIGNED: mode, next step, result | Two-line sparse form differs from mockup. | Current action/error before stale success; legible at narrow size. | P2 | UX-02 and later refinement | Model/Sketch/error captures, next-step comprehension. |
| Type/icons/DPI | UX-REF-01/02 | PARTIALLY ALIGNED: corrected Windows font, project icons | Polish and scale differ; 125/150% NOT VERIFIED. | Type/icon/hit-size targets in contract; original/licensed artwork. | P1 | UX-02 regression and later design-system slice | Actual DPI, contrast and focus observations. |
| SET-01 gestures/colors | UX-REF-01/02 | ALIGNED through opt-in Profile B/Blue, defaults A/Orange | Mockup's blue selection/MMB Pan cannot replace accepted defaults. | Accent applies to selected only; hovered distinct; A/B gestures labeled. | P0 contract guard | Every affected UX gate | SET-01 owner gate and affected interaction checks. |
| STL export | UX-REF-04 | PARTIALLY ALIGNED: bounded dialog and real validator | No full Export workspace or split CAD/mesh view. | Preserve validated STL; richer surface only if authorized. | P2 current / P3 broad | Existing UX-03 Export UX, planned | Export/Cancel/validation; future preview evidence if authorized. |
| Manufacture | UX-REF-03 | DEFERRED | CAM operations/simulation are conceptual only. | Shared visual language, no active fake CAM controls. | P3 | UX-04 future-workspace vision | Later architecture/product decision, not current runtime. |

The existing inventory below is retained as historical functional analysis; where its older `EXISTS/PARTIAL` vocabulary differs, the R1 matrix above governs *visual conformance*. The roadmap keeps UX-03 as Export UX and UX-04 as future workspaces. The R1 suggestion of an intervening Model Refinement requires owner sequencing; no automatic renumbering or implementation approval follows.

The owner designated [`uiux_v03_pkg`](uiux_v03_pkg/README.md) as the visual and interaction target for Micro SolidWorks. The package itself labels its specification **PROPOSED — READY FOR OWNER REVIEW**. This analysis records the target and its gaps without changing that formal status, an accepted ADR, the frozen `v1.1.0` baseline, or the authorization state of v1.2. The four images are design references, not executable screenshots or assets to insert into the application. The source package is preserved verbatim and tracked with this analysis.

## Package and reference integrity

The package contains five Markdown files (`README`, specification, registry, handoff and prompt) and four PNGs. The [registry](uiux_v03_pkg/VISUAL_REFERENCE_REGISTRY_V0_3.md) SHA-256 values were checked against all four local files and match. These are the only primary v0.3 visual references:

| Reference | Design intent | Boundary |
| --- | --- | --- |
| [UX-REF-01 Model](uiux_v03_pkg/images/UX-REF-01-MODEL.png) | Viewport-dominant modeling shell, left browser, right inspector, ribbon, dimensions and view controls. | Illustrated Hole, Revolve, Fillet, etc. are not current feature commitments. |
| [UX-REF-02 Sketch](uiux_v03_pkg/images/UX-REF-02-SKETCH.png) | Orthographic canvas, visible X/Y, in-canvas dimensions, contextual tools and clear Exit Sketch. | Illustrated Slot, Polygon, Trim, Offset, etc. do not authorize geometry or constraint expansion. |
| [UX-REF-03 Manufacture](uiux_v03_pkg/images/UX-REF-03-MANUFACTURE.png) | Future operation tree, stock/toolpath view, inspector and simulation timeline. | Vision only; no CAM, tooling, G-code or postprocessor authorization. |
| [UX-REF-04 Export](uiux_v03_pkg/images/UX-REF-04-EXPORT.png) | CAD-versus-mesh preview, units/destination, validation results and export readiness. | STL export was delivered in frozen v1.2.0; STEP, 3MF, OBJ, DXF, import and repair are not approved. |

The mockups display a `.mcad` filename, but the current native document is `.microsw` v2. The mockup does not change the accepted persistence contract. The package's `CODEX_PROMPT_UX_V0_3.md` is retained as handoff material; it is not a separate authorization to implement the illustrated capabilities.

## Current executable versus v0.3 components

Classification means `EXISTS` for a usable present capability, `PARTIAL` for a present but materially different or incomplete one, `MISSING` for a target absent in current CAD UI, and `DEFERRED` for a future capability outside the current authorization. UX-01 code, final V3 and owner visual acceptance with changes are recorded in [UX-01 validation](UX_01_CAD_WORKSPACE_VALIDATION.md).

| Component | State | Current evidence and gap |
| --- | --- | --- |
| AppChrome / title / menus / search | **PARTIAL** | Native title and in-app document header reflect file/dirty state; [`ApplicationShell.cpp`](../src/ui/ApplicationShell.cpp) retains File/Edit/View/Help menus. Command search is deferred. |
| WorkspaceRibbon and command discovery | **PARTIAL** | `ApplicationShell::drawCommandBar` distinguishes Model and Sketch, groups existing commands by mode, provides icons/tooltips and disables unavailable actions. Full multi-workspace tabs remain deferred. |
| ModelBrowser / Feature Manager | **PARTIAL** | `drawSketchPanel` and [`FeatureTreeModel`](../src/ui/FeatureTreeModel.cpp) retain left feature history, principal planes, selection, context actions and editing while the right Inspector is visible. Browser search and mockup body folders are absent. |
| ViewportCanvas | **EXISTS** | [`WorkspaceViewport.cpp`](../src/viewer/WorkspaceViewport.cpp) renders CAD-derived solids/sketch context and supports picking, hover, navigation and standard views; `drawWorkspace` reserves the central graphics area. Visual styling and layout need comparison during any authorized implementation. |
| ViewportHUD / view cube | **PARTIAL** | `drawWorkspace` provides X/Y/Z cues and Fit/ISO/Views controls; View menu offers standard views, projection and display modes. No interactive view cube or comparable orientation widget. |
| ContextInspector / Property Manager | **PARTIAL** | Feature/face/edge selection, source/target, Sketch parameters, Confirm/Cancel and diagnostics now appear in a simultaneous right Inspector; its full reference tab family is absent. |
| StatusGuidanceBar | **PARTIAL** | Mode, next step, feedback and failure cues are explicit. Units and richer validation hierarchy remain for later work. |
| Sketch-specific workspace | **PARTIAL** | Sketch mode has Line/Rectangle/Circle, constraints, previews, grid snap, plane-normal view, distinct ribbon and Exit Sketch, with the browser visible. In-canvas dimensions and direct manipulation remain UX-02 gaps. |
| DimensionCallout / direct editing | **MISSING** | Dimensions are edited numerically in `drawSketchPanel`; the 2026-10-20 R2 correction synchronizes the solved Sketch view after application. No editable dimension labels or handles are drawn in the canvas. |
| FloatingMicroToolbar | **MISSING** | Existing right-click/context actions and inspector buttons do not provide a toolbar near selected geometry. |
| Export workflow / DataExchangeNavigator / ValidationChecklist | **PARTIAL** | Frozen v1.2.0 includes File → Export STL, body/quality/path selection and validator-driven status. The complete UX-REF-04 workspace, split preview and other formats remain deferred to later work. |
| Manufacture OperationsTree / SimulationTimeline | **DEFERRED** | No CAM workspace or operation model exists. UX-REF-03 is a long-term consistency reference only. |
| Visual theme and custom icon system | **PARTIAL** | The application uses a graphite/blue ImGui palette, an installed Windows UI font and project-drawn CAD icons for supported commands. Owner-supplied before/after runtime captures and Experience Gate APPROVED WITH CHANGES are recorded; the full illustrated icon family is intentionally absent. |

## Sequenced adoption proposal

UX-01 is FROZEN after V3 and owner Experience Gate APPROVED WITH CHANGES; UX-02/UX-03 remain planned, not authorized by this campaign. Preserve Dear ImGui, GLFW, OpenGL and the CAD/render ownership boundary. The owner will formalize remaining conceptual and design differences separately; that statement does not itself expand the next campaign.

1. **UX-01 — Model/Sketch shell alignment (frozen).** The left Model Browser stays visible beside the right Inspector and usable viewport, with distinct Model/Sketch commands and mode cues. Existing commands, selection, Confirm/Cancel and document safety passed the technical and owner Experience Gates.
2. **UX-02 — Guided Sketch interaction.** Plan in-canvas dimension callouts, selection feedback, local micro-toolbar and exact inspector synchronization, then implement in bounded slices. Do not infer new sketch primitives or solver semantics from UX-REF-02. Measure discoverability, legibility and editing/Undo behavior on the same mechanical fixture.
3. **UX-03 — Export workflow UX.** The bounded C1/C2 STL flow is frozen in v1.2.0. A full Export Workspace, richer checklist or split CAD/mesh preview requires separate authorization; the mockup's green checklist is not evidence that a mesh is printable.
4. **UX-04 — Future workspace consistency.** Retain Manufacture, broader interchange, scripting, drawings and assemblies as vision/roadmap inputs for later product and architecture review. No CAM implementation or fake enabled commands.

Two **specific compatibility questions** are resolved by the FROZEN executable [SET-01 preferences slice](PERSONAL_SOLIDWORKS_SET_01_CAMPAIGN_PROPOSAL.md). Specification §7.3's middle-button Pan is available as opt-in Profile B; Profile A keeps the existing middle-button Orbit and Shift+middle-button Pan defaults. Specification §7.1's blue selection is available as an opt-in accent; Orange remains the default. The owner Experience Gate validated both alternatives, their persistence and the original defaults; final V3 passed 918/918 tests. [ADR-0070](adr/ADR-0070-application-preferences-ownership-and-persistence.md) and [ADR-0071](adr/ADR-0071-configurable-navigation-and-selection-appearance.md) are **ACCEPTED**. The package specification remains **PROPOSED — DESIGN REFERENCE**, and UX-01 must not silently change these defaults.

Other open design choices from the [specification](uiux_v03_pkg/MICRO_SOLIDWORKS_UIUX_SPEC_V0_3.md) remain open: ribbon density, panel pin/auto-hide, command-search timing, minimum in-canvas editing, floating-toolbar trigger, dark-theme contrast, first STL validation panel and future scripting/manufacture surfaces. UX-01 should decide only the subset needed for its approved scope. Any decision affecting accepted ownership, persistence, module boundaries or dependency policy requires the established Decision Gate process.

The design target guides UX-01 within its explicit scope. It does not authorize CAM, new geometry, a new dependency or an architectural change.

## V1.2-READY adoption ruling

The [readiness review](PERSONAL_SOLIDWORKS_V1_2_READINESS_REVIEW.md) keeps UI/UX
v0.3 **PROPOSED — DESIGN REFERENCE**. Its two specific MMB/selection conflicts
are **RESOLVED BY CONFIGURABLE PREFERENCES** in frozen SET-01 (Profile B and
Blue opt-in; Profile A and Orange defaults). Frozen v1.2.0 delivered the
bounded STL export. The owner subsequently authorized UX-01 Model/Sketch shell
work; UX-02 in-canvas improvements remain separate. UX-REF-03 Manufacture and
its toolpaths/timeline remain visionary; no simulated control is presented as
an available product command.
