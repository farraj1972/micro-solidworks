# UI/UX v0.3 — Reference inventory and adoption analysis

Status: **DOCUMENTED GAP ANALYSIS / UX-01 AUTHORIZED AND ACTIVE** (2026-10-20).

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

Classification means `EXISTS` for a usable present capability, `PARTIAL` for a present but materially different or incomplete one, `MISSING` for a target absent in current CAD UI, and `DEFERRED` for a future capability outside the current authorization. Evidence is source-level and from existing validation; no new visual runtime acceptance is claimed here.

| Component | State | Current evidence and gap |
| --- | --- | --- |
| AppChrome / title / menus / search | **PARTIAL** | Native title reflects document and dirty state in [`main.cpp`](../src/app/main.cpp); [`ApplicationShell.cpp`](../src/ui/ApplicationShell.cpp) has File/Edit/View/Help menus. No integrated command search or the v0.3 menu/workspace organization. |
| WorkspaceRibbon and command discovery | **PARTIAL** | The existing icon/tooltip CAD Command Bar in `ApplicationShell::drawCommandBar` exposes core Sketch, Extrude and Cut actions. It is a flat bar with 62-pixel command buttons, not Model/Sketch/Export workspace tabs and grouped ribbon controls. |
| ModelBrowser / Feature Manager | **PARTIAL** | `drawSketchPanel` and [`FeatureTreeModel`](../src/ui/FeatureTreeModel.cpp) provide a left feature tree, principal planes, selection, context actions and editing. Search, the reference's spatial hierarchy and a permanently separate right inspector are absent. |
| ViewportCanvas | **EXISTS** | [`WorkspaceViewport.cpp`](../src/viewer/WorkspaceViewport.cpp) renders CAD-derived solids/sketch context and supports picking, hover, navigation and standard views; `drawWorkspace` reserves the central graphics area. Visual styling and layout need comparison during any authorized implementation. |
| ViewportHUD / view cube | **PARTIAL** | `drawWorkspace` provides X/Y/Z cues and Fit/ISO/Views controls; View menu offers standard views, projection and display modes. No interactive view cube or comparable orientation widget. |
| ContextInspector / Property Manager | **PARTIAL** | `drawSketchPanel` presents feature and Sketch parameters, Confirm/Cancel and diagnostics. It shares the **left** panel with FeatureManager via tabs; UX-REF-01/02 place a selection-driven inspector on the **right**, leaving the browser continuously visible. |
| StatusGuidanceBar | **PARTIAL** | `drawStatusBar` and `workflowState_` show document state and tool guidance. The current messaging is useful but does not yet meet the reference's consistent selection prompts, units and validation-state hierarchy. |
| Sketch-specific workspace | **PARTIAL** | Sketch mode has Line/Rectangle/Circle, constraints, previews, grid snap, plane-normal view and Exit Sketch. Tools remain in the shared bar; properties/constraints are in the left panel. The reference's sketch ribbon, in-canvas dimensions and contextual edit controls are absent. |
| DimensionCallout / direct editing | **MISSING** | Dimensions are edited numerically in `drawSketchPanel`; the 2026-10-20 R2 correction synchronizes the solved Sketch view after application. No editable dimension labels or handles are drawn in the canvas. |
| FloatingMicroToolbar | **MISSING** | Existing right-click/context actions and inspector buttons do not provide a toolbar near selected geometry. |
| Export workflow / DataExchangeNavigator / ValidationChecklist | **PARTIAL** | Frozen v1.2.0 includes File → Export STL, body/quality/path selection and validator-driven status. The complete UX-REF-04 workspace, split preview and other formats remain deferred to later work. |
| Manufacture OperationsTree / SimulationTimeline | **DEFERRED** | No CAM workspace or operation model exists. UX-REF-03 is a long-term consistency reference only. |
| Visual theme and custom icon system | **PARTIAL** | The application has a dark ImGui shell and project-drawn CAD command icons in `ApplicationShell.cpp`. The graphite/blue hierarchy, typography, responsive density and full icon family of the references need an authorized visual pass; do not copy the PNG artwork into product assets. |

## Sequenced adoption proposal

UX-01 is authorized and active; UX-02/UX-03 remain planned, not authorized by this campaign. Preserve Dear ImGui, GLFW, OpenGL and the CAD/render ownership boundary. Use impact-based validation during increments and the explicitly required V3 and owner Experience Gate at UX-01 closure.

1. **UX-01 — Model/Sketch shell alignment (active).** Keep the left Model Browser visible, move contextual properties into a right Inspector, retain a minimum usable viewport, and give Model/Sketch an explicit mode cue. Preserve existing commands, selection, Confirm/Cancel and document safety. Acceptance requires technical and owner Experience Gates.
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
