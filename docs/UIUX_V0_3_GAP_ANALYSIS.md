# UI/UX v0.3 — Reference inventory and adoption analysis

Status: **DOCUMENTED GAP ANALYSIS / PROPOSED ADOPTION; NO IMPLEMENTATION AUTHORIZATION** (2026-10-20).

The owner designated [`uiux_v03_pkg`](uiux_v03_pkg/README.md) as the visual and interaction target for Micro SolidWorks. The package itself labels its specification **PROPOSED — READY FOR OWNER REVIEW**. This analysis records the target and its gaps without changing that formal status, an accepted ADR, the frozen `v1.1.0` baseline, or the authorization state of v1.2. The four images are design references, not executable screenshots or assets to insert into the application. The source package is preserved verbatim and tracked with this analysis.

## Package and reference integrity

The package contains five Markdown files (`README`, specification, registry, handoff and prompt) and four PNGs. The [registry](uiux_v03_pkg/VISUAL_REFERENCE_REGISTRY_V0_3.md) SHA-256 values were checked against all four local files and match. These are the only primary v0.3 visual references:

| Reference | Design intent | Boundary |
| --- | --- | --- |
| [UX-REF-01 Model](uiux_v03_pkg/images/UX-REF-01-MODEL.png) | Viewport-dominant modeling shell, left browser, right inspector, ribbon, dimensions and view controls. | Illustrated Hole, Revolve, Fillet, etc. are not current feature commitments. |
| [UX-REF-02 Sketch](uiux_v03_pkg/images/UX-REF-02-SKETCH.png) | Orthographic canvas, visible X/Y, in-canvas dimensions, contextual tools and clear Exit Sketch. | Illustrated Slot, Polygon, Trim, Offset, etc. do not authorize geometry or constraint expansion. |
| [UX-REF-03 Manufacture](uiux_v03_pkg/images/UX-REF-03-MANUFACTURE.png) | Future operation tree, stock/toolpath view, inspector and simulation timeline. | Vision only; no CAM, tooling, G-code or postprocessor authorization. |
| [UX-REF-04 Export](uiux_v03_pkg/images/UX-REF-04-EXPORT.png) | CAD-versus-mesh preview, units/destination, validation results and export readiness. | STL is a candidate in the proposed v1.2 campaign; STEP, 3MF, OBJ, DXF, import and repair are not approved. |

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
| Export workflow / DataExchangeNavigator / ValidationChecklist | **DEFERRED** | No STL/export commands are present in `src/ui` or `src/app`. The [v1.2 campaign proposal](PERSONAL_SOLIDWORKS_V1_2_CAMPAIGN_PROPOSAL.md) names STL contract and implementation as C1/C2, both still unauthorized. CAD-to-mesh validation and split preview require their own contract and evidence. |
| Manufacture OperationsTree / SimulationTimeline | **DEFERRED** | No CAM workspace or operation model exists. UX-REF-03 is a long-term consistency reference only. |
| Visual theme and custom icon system | **PARTIAL** | The application has a dark ImGui shell and project-drawn CAD command icons in `ApplicationShell.cpp`. The graphite/blue hierarchy, typography, responsive density and full icon family of the references need an authorized visual pass; do not copy the PNG artwork into product assets. |

## Sequenced adoption proposal

Each item below is a **candidate increment**, not an approved implementation baseline. Preserve Dear ImGui, GLFW, OpenGL and the CAD/render ownership boundary. Validate UI changes with affected builds/tests plus runtime and a user Experience Gate; avoid a routine clean/full regression.

1. **UX-01 — Model/Sketch shell alignment (smallest coherent next slice).** Keep the left Model Browser visible, move contextual properties into a right Inspector, retain a minimum usable viewport, and give Model/Sketch an explicit mode cue. Preserve existing commands, selection, Confirm/Cancel and document safety. Acceptance: the owner can repeat the existing adapter-plate CAD workflow without hidden controls, ambiguous mode or technical IDs. This is a UI layout/workflow proposal requiring explicit increment authorization.
2. **UX-02 — Guided Sketch interaction.** Plan in-canvas dimension callouts, selection feedback, local micro-toolbar and exact inspector synchronization, then implement in bounded slices. Do not infer new sketch primitives or solver semantics from UX-REF-02. Measure discoverability, legibility and editing/Undo behavior on the same mechanical fixture.
3. **UX-03 — Export workflow UX, coupled to authorized v1.2 C1/C2.** Specify format, units, destination, body selection, CAD-versus-derived-mesh preview, validation checklist and actionable errors only after the STL export contract is authorized. The validator's technical result must drive the UI; the mockup's green checklist is not evidence that a mesh is printable.
4. **UX-04 — Future workspace consistency.** Retain Manufacture, broader interchange, scripting, drawings and assemblies as vision/roadmap inputs for later product and architecture review. No CAM implementation or fake enabled commands.

Two **specific compatibility questions** need owner resolution before implementation. Specification §7.3 says middle mouse pans, while the executable uses middle mouse to orbit and Shift+middle mouse to pan in [`OrbitNavigation.cpp`](../src/viewer/OrbitNavigation.cpp) and [`PanZoomNavigation.cpp`](../src/viewer/PanZoomNavigation.cpp). Specification §7.1 calls for blue selected geometry, while frozen B4's [`HighlightColors.h`](../src/viewer/HighlightColors.h) uses orange, as does the solid-target highlight in `WorkspaceViewport.cpp`. Neither existing behavior should be silently changed by a layout increment; first decide whether the new design target supersedes these frozen visual/interaction contracts and document the authorized scope.

Other open design choices from the [specification](uiux_v03_pkg/MICRO_SOLIDWORKS_UIUX_SPEC_V0_3.md) remain open: ribbon density, panel pin/auto-hide, command-search timing, minimum in-canvas editing, floating-toolbar trigger, dark-theme contrast, first STL validation panel and future scripting/manufacture surfaces. UX-01 should decide only the subset needed for its approved scope. Any decision affecting accepted ownership, persistence, module boundaries or dependency policy requires the established Decision Gate process.

The design target may guide future reviews now, but no v1.2 increment, export code, CAM feature, new dependency or architectural change is authorized by tracking this package.
