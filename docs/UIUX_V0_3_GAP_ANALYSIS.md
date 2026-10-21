# UI/UX v0.3 — Reference inventory and adoption analysis

Status: **DOCUMENTED COMPLIANCE ANALYSIS / UX-01, UX-01.R1 AND UX-02 FROZEN / UX-ROADMAP-R1 READY FOR OWNER DECISION** (2026-10-21). UX-02 passed V3 940/940 and the full owner Experience Gate; the UI/UX v0.3 source package remains a proposed design reference. The [post-UX-02 review](UX_ROADMAP_R1_POST_UX02_REVIEW.md) gives the current functional/visual/priority matrix and proposes Model refinement before Export, pending owner decision.

The [UX-01.R1 canonical workspace contract](UX_01_MODEL_WORKSPACE_CONTRACT.md) is the detailed design specification. This matrix gives component-level traceability without importing pictured future commands into executable scope. The four canonical PNGs and both actual Model captures were inspected directly. Functional qualifications come from [UX-01 validation](UX_01_CAD_WORKSPACE_VALIDATION.md), [SET-01 validation](PERSONAL_SOLIDWORKS_SET_01_VALIDATION.md) and current UI source. `NOT VERIFIED` means evidence is absent, not that a feature failed.

## UX-01.R1 conformance matrix

States: `ALIGNED`, `PARTIALLY ALIGNED`, `MISALIGNED`, `MISSING`, `DEFERRED`, `NOT VERIFIED`. Priorities: P0 critical regression/workflow; P1 high-value alignment; P2 visual refinement; P3 future capability. Campaigns below are candidates, not implementation authorization.

| Component | Canonical Reference | Current State | Observed Gap | Target Contract | Priority | Implementation Campaign | Acceptance Evidence |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Application header / document | UX-REF-01/02 | PARTIALLY ALIGNED: native title, document header, dirty state | Menu precedes identity; search is future. | One clear identity/document/dirty hierarchy; no fake search. | P1 hierarchy / P3 search | Proposed UX-04 later slice | Model idle/dirty captures; save/open title behavior. |
| Main menu | UX-REF-01/02 | ALIGNED functionally: File/Edit/View/Help | Less visual separation; mockup contains future menus. | Native-like keyboard/file actions; no empty domain menu. | P2 | Proposed UX-04 later slice | Menu/keyboard smoke at 1366×768. |
| Workspace mode/tabs | UX-REF-01/02 | PARTIALLY ALIGNED: real Model/Sketch contexts | Small heading rather than clear tab tier. | Text-and-shape active context; only implemented modes enabled. | P1 | Proposed UX-04 later slice; UX-02 behavior frozen | Paired Model/Sketch screenshots and entry/exit test. |
| Ribbon | UX-REF-01/02 | PARTIALLY ALIGNED: contextual original icons, labels, tooltips, disabled states | Flat one-row strip, few named command families. | Semantic groups, active/disabled reasons, no clipped commands. | P1 | Proposed UX-04 after first slice | 1920/1366 and DPI captures where available; prerequisites test. |
| Browser structure | UX-REF-01/02 | PARTIALLY ALIGNED: planes, ordered features, absorbed sketches, context actions | No body/sketch group or search; simpler hierarchy. | Preserve feature order and single absorbed Sketch node; only derived grouping. | P1 if fixture proves navigation friction; otherwise P2 / P3 search | Proposed UX-04 later slice, evidence first | Tree/viewport selection, right click, FAILED/BLOCKED. |
| Browser legibility | UX-REF-01/02 | ALIGNED for fixed overlap/font defect | DPI and visual rhythm not verified. | Readable rows, nonoverlapping icons, distinct hover/selected/error. | P0 regression / P2 refinement | UX-02 regression and later refinement | Before/after actual captures; 125/150% NOT VERIFIED. |
| Graphics Area / framing | UX-REF-01 | PARTIALLY ALIGNED: derived solid, picking, camera/Fit | Small model on flat background; different fixture limits comparison. | Dominant pickable viewport, predictable Fit, subtle technical depth. | P2 | Later Model refinement | Same-fixture Fit capture and pick test. |
| Display modes and highlight | UX-REF-01 | ALIGNED functionally: three modes, hover and selection | Static idle image cannot prove all modes. | Preserve Shaded, Shaded With Edges, Wireframe and distinct hover/selection. | P0 regression | Each affected UX gate | Three-mode screenshots/tests; SET-01 evidence. |
| HUD / orientation | UX-REF-01/02 | PARTIALLY ALIGNED: Fit/ISO/Views, X/Y/Z cues | No interactive View Cube or reference triad treatment. | Compact standard views; cube only after authorization. | P2 / P3 cube | Proposed UX-04 later slice; cube deferred | Standard views/projection checks; no fake cube. |
| Inspector idle/selected | UX-REF-01/02 | PARTIALLY ALIGNED: contextual feature/plane/face/edge UI in source | Actual capture shows sparse idle; weaker section/value hierarchy. | Helpful idle; grouped identity, facts and valid actions when selected. | P1 | Proposed UX-04.1 selected context | Idle/feature/face/edge captures; read-only leaves dirty/Undo unchanged. |
| Selected pocket face shading | UX-REF-01 | PARTIAL: selected face highlight overlays derived shaded triangles; coplanar topology seams are suppressed from edge display | Owner observed a tonal boundary at pocket-floor level only when a lateral face was selected in Shaded With Edges; no archived capture or proven root cause. | May falsely suggest a broken Solid and reduce selection confidence. | P1 | Proposed UX-04.1; Presentation/Viewer hypothesis to verify | UX-02 owner observation; `SolidDisplay::isCoplanarSeam`, `WorkspaceViewport::drawState`, ADR-0056/0060; unselected face did not show it. |
| Inspector edit/diagnostic | UX-REF-01/02 | PARTIALLY ALIGNED: drafts, Confirm/Cancel, FAILED/BLOCKED in source | No archived executable edit screenshot. | Stable actions, units, inline validation and actionable diagnostic. | P1 | Proposed UX-04 later Model refinement | Edit/cancel/confirm/Undo and failure captures. |
| Sketch canvas/dimensions | UX-REF-02 | IMPLEMENTED in UX-02 for four existing driving dimensions; V3 and owner Experience Gate PASS | Other constraint types have no numeric callout; no on-canvas geometric drag manipulation. | Solved geometry and stable constraint IDs drive readable dimensions. | P1 | UX-02 FROZEN | [UX-02 validation](UX_02_GUIDED_SKETCH_VALIDATION.md): projection/edit/Undo tests and owner visual workflow PASS; pre-fix screenshot archived. |
| Context micro-toolbar | UX-REF-02 | IMPLEMENTED in UX-02 for selected dimension Edit; owner workflow PASS | Other selected geometry has only existing applicable actions. | Only supported actions; no occluded pick target or advanced manipulation claim. | P1 candidate | UX-02 FROZEN | Local action/cancel and owner retest PASS; post-fix screenshot not archived. |
| Status/guidance | UX-REF-01/02 | PARTIALLY ALIGNED: mode, next step, result | Two-line sparse form differs from mockup. | Current action/error before stale success; legible at narrow size. | P2 | Proposed UX-04 later slice | Model/Sketch/error captures, next-step comprehension. |
| Type/icons/DPI | UX-REF-01/02 | PARTIALLY ALIGNED: corrected Windows font, project icons | Polish and scale differ; 125/150% NOT VERIFIED. | Type/icon/hit-size targets in contract; original/licensed artwork. | P1 evidence / P2 polish | Proposed UX-04 gate and later design-system slice | Actual DPI, contrast and focus observations. |
| SET-01 gestures/colors | UX-REF-01/02 | ALIGNED through opt-in Profile B/Blue, defaults A/Orange | Mockup's blue selection/MMB Pan cannot replace accepted defaults. | Accent applies to selected only; hovered distinct; A/B gestures labeled. | P0 contract guard | Every affected UX gate | SET-01 owner gate and affected interaction checks. |
| STL export | UX-REF-04 | PARTIALLY ALIGNED: bounded dialog, body/quality/destination and real validator | No structured validation/result display or CAD/mesh preview; the latter needs a separate candidate contract. | Preserve validated STL; make body/settings/validator outcome legible before broader surface work. | P1 honest preflight / P2 preview / P3 broad | UX-03 Export UX, planned after proposed Model refinement | C2/release export tests and owner gate; independent slicer readiness was waived. |
| Manufacture | UX-REF-03 | DEFERRED | CAM operations/simulation are conceptual only. | Shared visual language, no active fake CAM controls. | P3 | Unnumbered future-workspace vision pending owner decision | Later architecture/product decision, not current runtime. |

The existing inventory below is retained as historical functional analysis; where its older `EXISTS/PARTIAL` vocabulary differs, the R1 matrix above governs *visual conformance*. The [post-UX-02 review](UX_ROADMAP_R1_POST_UX02_REVIEW.md) distinguishes functional and visual impact and recommends proposed `UX-04 — Model Workspace Visual & Interaction Refinement` before the already named UX-03 Export UX. The previous unstarted UX-04 future-workspaces placeholder becomes unnumbered vision **only if the owner approves this sequence/name**; neither campaign is implementation-authorized by the review.

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
| Sketch-specific workspace | **PARTIAL — UX-02 slice FROZEN** | Sketch mode has Line/Rectangle/Circle, constraints, previews, grid snap, plane-normal view, distinct ribbon and Exit Sketch, with the browser visible. UX-02 adds derived driving-dimension callouts and transactional numeric editing; it does not add geometric drag manipulation. Owner visual Experience Gate passed. |
| DimensionCallout / direct editing | **IMPLEMENTED — UX-02 FROZEN** | Four existing driving dimensions have derived canvas callouts with stable constraint selection and transactional numeric editing. V3 940/940 and full owner Experience Gate PASS. |
| FloatingMicroToolbar | **PARTIAL — UX-02 FROZEN** | A selected dimension exposes a nearby Edit action; other supported contextual actions remain in the Inspector. No broad floating toolbar is claimed. |
| Export workflow / DataExchangeNavigator / ValidationChecklist | **PARTIAL** | Frozen v1.2.0 includes File → Export STL, body/quality/path selection and validator-driven status. The complete UX-REF-04 workspace, split preview and other formats remain deferred to later work. |
| Manufacture OperationsTree / SimulationTimeline | **DEFERRED** | No CAM workspace or operation model exists. UX-REF-03 is a long-term consistency reference only. |
| Visual theme and custom icon system | **PARTIAL** | The application uses a graphite/blue ImGui palette, an installed Windows UI font and project-drawn CAD icons for supported commands. Owner-supplied before/after runtime captures and Experience Gate APPROVED WITH CHANGES are recorded; the full illustrated icon family is intentionally absent. |

## Sequenced adoption proposal

UX-01 is FROZEN after V3 and owner Experience Gate APPROVED WITH CHANGES. UX-02 is FROZEN after V3 940/940 and owner Experience Gate APPROVED. UX-ROADMAP-R1 proposes Option A: bounded Model refinement as UX-04 before UX-03 Export UX, pending owner decision. Both are unapproved executable campaigns. Preserve Dear ImGui, GLFW, OpenGL and the CAD/render ownership boundary.

1. **UX-01 — Model/Sketch shell alignment (frozen).** The left Model Browser stays visible beside the right Inspector and usable viewport, with distinct Model/Sketch commands and mode cues. Existing commands, selection, Confirm/Cancel and document safety passed the technical and owner Experience Gates.
2. **UX-02 — Guided Sketch interaction (frozen).** Bounded callout, selection, editing and contextual-action slices passed final V3 940/940 and the full owner Experience Gate. Do not infer new sketch primitives or solver semantics from UX-REF-02. The documented fixtures cover discoverability, legibility and editing/Undo behavior.
3. **UX-04 — Model Workspace Visual & Interaction Refinement (proposed first).** Investigate selected pocket-face shading and improve selected Model context in a small derived-display/UI slice. Browser/ribbon/HUD polish needs separately evidenced scope. Owner naming/scope approval is outstanding.
4. **UX-03 — Export workflow UX (planned later).** The bounded C1/C2 STL flow is frozen in v1.2.0. Honest validation/results should precede a higher-risk exact-candidate CAD/mesh preview; both require separate authorization. The mockup's green checklist is not evidence that a mesh is printable.
5. **Future workspaces (unnumbered vision pending owner sequencing).** Retain Manufacture, broader interchange, scripting, drawings and assemblies as product/architecture inputs. No CAM implementation or fake enabled commands.

Two **specific compatibility questions** are resolved by the FROZEN executable [SET-01 preferences slice](PERSONAL_SOLIDWORKS_SET_01_CAMPAIGN_PROPOSAL.md). Specification §7.3's middle-button Pan is available as opt-in Profile B; Profile A keeps the existing middle-button Orbit and Shift+middle-button Pan defaults. Specification §7.1's blue selection is available as an opt-in accent; Orange remains the default. The owner Experience Gate validated both alternatives, their persistence and the original defaults; final V3 passed 918/918 tests. [ADR-0070](adr/ADR-0070-application-preferences-ownership-and-persistence.md) and [ADR-0071](adr/ADR-0071-configurable-navigation-and-selection-appearance.md) are **ACCEPTED**. The package specification remains **PROPOSED — DESIGN REFERENCE**, and UX-01 must not silently change these defaults.

Other open design choices from the [specification](uiux_v03_pkg/MICRO_SOLIDWORKS_UIUX_SPEC_V0_3.md) remain open: ribbon density, panel pin/auto-hide, command-search timing, minimum in-canvas editing, floating-toolbar trigger, dark-theme contrast, first STL validation panel and future scripting/manufacture surfaces. UX-01 should decide only the subset needed for its approved scope. Any decision affecting accepted ownership, persistence, module boundaries or dependency policy requires the established Decision Gate process.

The design target guides UX-01 within its explicit scope. It does not authorize CAM, new geometry, a new dependency or an architectural change.

## V1.2-READY adoption ruling

The historical [readiness review](PERSONAL_SOLIDWORKS_V1_2_READINESS_REVIEW.md) keeps UI/UX
v0.3 **PROPOSED — DESIGN REFERENCE**. Its two specific MMB/selection conflicts
are **RESOLVED BY CONFIGURABLE PREFERENCES** in frozen SET-01 (Profile B and
Blue opt-in; Profile A and Orange defaults). Frozen v1.2.0 delivered the
bounded STL export. The owner subsequently authorized UX-01 Model/Sketch shell
work; UX-02 in-canvas improvements were a separate campaign and are now
FROZEN as recorded above. UX-REF-03 Manufacture and
its toolpaths/timeline remain visionary; no simulated control is presented as
an available product command.
