# UX-ROADMAP-R1 — Post-UX-02 product and visual gap review

Status: **READY FOR OWNER DECISION** (2026-10-21). This is a V0 analysis and
roadmap proposal, not authorization to implement UX-03 or UX-04. The four
checked-in v0.3 PNGs are design references; the UI/UX v0.3 specification is
still **PROPOSED**. [UX-01.R1](UX_01_MODEL_WORKSPACE_CONTRACT.md) remains the
approved composition contract. Frozen releases, campaigns and ADRs are not
amended by this review.

## 1. Executive summary

Recommend **Option A: a bounded Model Workspace refinement before UX-03 Export
UX**. The immediate issue is misleading face-selection feedback on a solid with
a pocket: the owner observed a tonal boundary at the pocket floor level on a
selected lateral face in Shaded With Edges, absent when unselected. It is an
interaction/display finding, not evidence of a defective CAD Solid. A first
slice should reproduce it, identify the owning derived draw pass, correct only
the proven visual cause, and make selected face/feature context and actions
unambiguous. It must preserve topology, picking identity, display-mode semantics
and document transactions. Grouping and search can follow only after their
specific workflow value is demonstrated. The current STL export already works
within its accepted bounds; a full Export workspace would add more surface area
before the Model workflow is as clear as it needs to be.

No current gap is classified **P0** on available evidence. This ranking is a
proposal for owner review, not a claim that all desktop resolutions or DPI
settings were observed. The next executable increment is specified in §11 and
is **not** started by this review.

## 2. Baseline verification and evidence method

- Local `master` HEAD at review entry: `4939f9363a03ef1e77d8dc1ec227033b1fbd81a6`
  (`[UX-02] Freeze Guided Sketch Interaction`); tag
  `ux-02-guided-sketch-interaction` resolves to that commit. The configured
  `github` remote names `farraj1972/micro-solidworks`; the local
  `refs/remotes/github/master` also resolves to this SHA. This review does not
  fetch or independently verify live GitHub state; the owner reported the push.
- Personal SolidWorks v1.2.0, P0, SPEC-01 and SET-01 remain FROZEN. UX-01 is
  FROZEN after V3 928/928 and owner Experience Gate APPROVED WITH CHANGES;
  UX-01.R1 is APPROVED/FROZEN; UX-02 is FROZEN after V3 940/940 and owner full
  Experience Gate PASS. The 71 ADR files all say `Status: ACCEPTED`; K1/K2/K3
  remains OPEN — EVIDENCE REQUIRED.
- Primary design inputs were opened directly: [Model](uiux_v03_pkg/images/UX-REF-01-MODEL.png),
  [Sketch](uiux_v03_pkg/images/UX-REF-02-SKETCH.png),
  [Manufacture](uiux_v03_pkg/images/UX-REF-03-MANUFACTURE.png), and
  [Export](uiux_v03_pkg/images/UX-REF-04-EXPORT.png). Their SHA-256 hashes
  match the [visual registry](uiux_v03_pkg/VISUAL_REFERENCE_REGISTRY_V0_3.md).
  Their pictured Hole/Fillet/Slot/CAM/STEP commands, `.mcad` filename and
  green printability claims do not establish executable capability.
- Actual executable evidence comprises the [UX-01 Model before/after captures](ux01_validation/UX-01-MODEL-EXECUTABLE.png),
  the [UX-02 pre-fix callout capture](ux02_validation/callout-offset-before-fix.png),
  the owner's corrected-build retests, and [UX-01](UX_01_CAD_WORKSPACE_VALIDATION.md)/
  [UX-02](UX_02_GUIDED_SKETCH_VALIDATION.md)/[SET-01](PERSONAL_SOLIDWORKS_SET_01_VALIDATION.md)/
  [v1.2.0](V1_2_0_RELEASE.md) validation records. A post-fix Sketch screenshot,
  selected-pocket-face screenshot and Windows 125/150% comparison are **NOT
  VERIFIED** as archived visual evidence. No runtime or test campaign is
  repeated in this V0 review.
- Source and governance consulted: repository `AGENTS.md`, `BOOTSTRAP.md`,
  `README.md`, [architecture](ARCHITECTURE.md), [dependency policy](DEPENDENCY_POLICY.md),
  [roadmap](ROADMAP.md), [v0.3 specification](uiux_v03_pkg/MICRO_SOLIDWORKS_UIUX_SPEC_V0_3.md),
  [UX reference model](specification/UX_REFERENCE_MODEL.md),
  [capability map](specification/CAPABILITY_MAP.md), and
  [requirements ledger](specification/REQUIREMENTS_TRACEABILITY.md), plus
  accepted Viewer/Presentation/Sketch/Editor/FeatureModel/Preferences/Persistence
  ADRs. Historical SPEC-01 evidence cuts are not silently rewritten as a
  current-release capability map.

## 3. Delivered UX-01 and UX-02 capability

UX-01 delivered a simultaneous left Model Browser, dominant Graphics Area and
right Context Inspector; Model/Sketch command ribbons with supported actions,
tooltips and disabled states; mode and status guidance; selected feature/face/
edge context and edit diagnostics. Its corrective build made the Windows font
readable, fitted tree icons to their rows and removed Sketch-only controls from
the Model Inspector. Existing CAD creation, edit, rebuild, history, file safety
and STL behavior were retained. An idle Model screenshot cannot prove every
Inspector state, but the corresponding code and owner gate support those
bounded capabilities.

UX-02 added derived callouts for **LineLength, CircleRadius,
HorizontalDistance and VerticalDistance** driving constraints. Canvas hit
testing selects a stable logical constraint, and double click or nearby Edit
opens a transient numeric draft. Confirm validates, solves and commits one
transaction; Cancel/Escape do not mutate geometry. Inspector content,
Undo/Redo, Save/Open, supported plane/face projection and viewport framing
were covered by tests and the owner workflow. The SET-01 Orange/Blue selected
accent is applied without changing Profile A/Orange defaults or `.microsw`
document state. UX-02 does not add draggable geometry, new constraint kinds
or a generic floating toolbar for every entity.

## 4. Updated visual and functional gap matrix

`ALIGNED` means the bounded existing behavior meets the cited design purpose;
`PARTIAL` means a demonstrated capability differs materially;
`MISSING` means a relevant candidate is absent; `DEFERRED` is outside approved
scope; `NOT VERIFIED` means evidence is insufficient. Functional impact and
visual impact are separate. P0 blocks or significantly harms the workflow,
P1 improves productivity/confidence materially, P2 is a meaningful visual
refinement, and P3 is future/unapproved scope. Campaign labels are proposals.

| Area | Canonical reference | Implemented capability | Observed gap | Functional impact | Visual impact | Priority | Proposed campaign | Evidence |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Header, menu, mode | UX-REF-01/02 | **PARTIAL:** native title, menu, document/dirty header and real Model/Sketch contexts | Mode is a small heading; menu/identity order differs; command search absent | Mode discovery can take extra attention; no proof of failed command | Weaker hierarchy than reference | P1 mode clarity; P3 search | UX-04 later slice | UX-01 actual Model capture; `ApplicationShell::drawCommandBar`; UX-01.R1 §4 |
| Model command grouping | UX-REF-01 | **PARTIAL:** real icon buttons, labels, tooltips and disabled state | Flat ribbon and sparse semantic groups | Prerequisites/actions less discoverable during feature work | Less scanable than reference | P1 | UX-04 after first slice | `ApplicationShell::drawCommandBar`; UX-01 gate |
| Browser/history and search | UX-REF-01/02 | **PARTIAL:** planes, ordered features, absorbed Sketches, right-click/F2 and synchronized selection | Search and derived body grouping absent; no evidence that search is the present blocker | Large-model navigation could cost time; current small-fixture impact unmeasured | Simpler narrative/row rhythm | P1 if fixture proves need; otherwise P2; search P3 pending evidence | UX-04 later proposal | `FeatureTreeModel`; UX-01.R1 §5; Model capture |
| Model Inspector and diagnostics | UX-REF-01 | **PARTIAL:** contextual feature/face/edge facts, draft/Confirm/Cancel, FAILED/BLOCKED | Sparse idle and weak section/field hierarchy; edit/failure screenshots not archived | Can obscure what is selected or why an action is unavailable | Less stable visual hierarchy | P1 | UX-04.1 selected context; later edit polish | `ApplicationShell::drawModelPanel`, feature state branches; UX-01 gate |
| Model face highlight at pocket seam | UX-REF-01 | **PARTIAL:** derived shaded faces, topology-edge overlay, per-face hover/selection | Owner saw a tonal boundary on selected lateral pocket face, only in Shaded With Edges; no archived capture | May suggest false geometry break and reduce picking trust | Visible false discontinuity during selection | P1 investigation/correction | UX-04.1 | UX-02 owner observation; `SolidDisplay::isCoplanarSeam`; `WorkspaceViewport::drawState`; ADR-0056/0060 |
| Viewport Fit and shading quality | UX-REF-01 | **PARTIAL:** shaded/wire modes, Fit, standard views and derived tessellation | Reference shows stronger framing/material depth; fixtures differ, so no measured numeric mismatch | Fit already supports subject discovery | Visual polish remains | P2 | UX-04 later if same-fixture evidence | UX-01 Model capture; `WorkspaceViewport`; UX-01.R1 §7 |
| HUD and View Cube | UX-REF-01/02 | **PARTIAL:** Fit/ISO/Views and X/Y/Z cues | No interactive View Cube | Standard orientation remains available | Reference cube/triad absent | P3 cube; P2 cue polish | Future separately authorized | `ApplicationShell::drawWorkspace`; UX-01.R1 §7 |
| Model status, typography and DPI | UX-REF-01 | **PARTIAL:** readable font, fixed tree icon overlap, mode/result guidance | 125/150% visual behavior **NOT VERIFIED**; status is sparse | Scaling may affect hit targets, not established failure | Lower polish than reference | P1 verification at next visible gate; P2 style | UX-04 gate | UX-01 before/after captures; no owner high-DPI capture |
| Sketch callouts and editing | UX-REF-02 | **ALIGNED within UX-02 scope:** four driving types, direct numeric edit, selected cue, solver/history/Inspector sync | Other kinds intentionally have no numeric label; geometric drag absent | Existing dimension workflow passed; do not reopen as missing | Less dense than mockup's many labels | P3 advanced direct manipulation | Future decision only | UX-02 validation and owner full gate; `SketchDimensionCallout`/UI tests |
| Sketch ribbon and local actions | UX-REF-02 | **PARTIAL:** supported Line/Rectangle/Circle and constraints, Exit Sketch, selected-dimension Edit nearby | Reference has more grouped tools and a multi-action toolbar | Existing tested actions work; extra discovery value unmeasured | Distinct design vocabulary | P2 for grouping; P3 unsupported tools | Later Sketch polish | UX-02 gate; `ApplicationShell` |
| Sketch projection/zoom feedback | UX-REF-02 | **ALIGNED within bounded scope:** normal-to-plane, plane/face projection, framing, pan/zoom, callout alignment | Post-fix screenshot and 125/150% visual captures **NOT VERIFIED** | Owner's tested workflow passed | No comparative pixel evidence | P1 evidence at future gate | UX-04 regression gate | UX-02 tests/owner retest; pre-fix capture is failure evidence only |
| STL command and parameters | UX-REF-04 | **PARTIAL:** File/ribbon Export STL, body, Standard/Fine quality, millimetre convention, Browse, Validate, Export/Cancel | No persistent Export workspace; quality presets do not expose mockup's free tolerance control | Accepted bounded export works; clearer preflight may reduce wrong-body/path mistakes | Modal differs strongly from reference | P1 targeted clarity; P3 full shell | UX-03 after Model | `ApplicationShell::drawExportDialog`; v1.2 C2/release records |
| STL validation/result | UX-REF-04 | **PARTIAL:** real mesh validator and status, export gated on validation and path | No structured checklist/result history; no proof of independent slicer readiness | Better evidence display could improve trust; validator truth must remain bounded | Less explicit than green checklist | P1 | UX-03 | Export validation code/tests; v1.2 N2 waivers |
| CAD versus mesh preview | UX-REF-04 | **MISSING** as UI, **DEFERRED** as current campaign | No split preview of exact export candidate | Helpful for inspection, but adds rendering/state coupling and does not itself prove validity | Major difference from reference | P2 candidate, P3 until scoped | UX-03 later slice, decision first | `ExportMesh`/`StlWriter`; UX-01.R1 §9 |
| Manufacture | UX-REF-03 | **DEFERRED:** no CAM aggregate, toolpath, simulation or postprocessor | Entire pictured workspace is future vision | None for current CAD→STL workflow | Reference-only | P3 | Unnumbered future product/architecture review | SPEC-01 capability map; reference image |

The selected-pocket-face observation is the only specific new visual defect
reported here. It was not seen in the unselected state, and no topology
validation failure was reported. `SolidDisplay` suppresses explicit coplanar
seam edges, while `WorkspaceViewport` recolors triangles of an individual
selected face target and draws topology edges separately. A boundary between
coplanar face IDs with different selection colors is therefore a **plausible
derived display explanation**, not a proven root cause. Reproduce with the
same saved pocket fixture and inspect face targets, normals and the three
display passes before changing code. If topology/mesh validity fails, stop and
report that distinct defect rather than hide it with shading.

## 5. Workspace-specific findings

**Model.** The A–H shell from UX-01.R1 is present in the correct left/center/
right relationship. Header/menu/mode/ribbon hierarchy, action grouping and
Inspector scan order still differ from UX-REF-01, but visual difference alone
does not justify copying future commands. Current browser history/plane
selection and FeatureManager↔viewport mapping are real; search, derived body
folders and an interactive cube are not. The Model fixture should be used to
compare selected Feature, face, edge, idle and failed rebuild states. The
selected-face boundary gets first attention because it can misrepresent the
solid itself.

**Sketch.** UX-02 closed the previous missing callout/direct-edit gap for four
driving dimensions. Its owner gate exercised line/circle, supported planes,
Confirm/Cancel, solver, Undo/Redo, Save/Open and setting changes. Additional
mockup constraints, circles, slots, handles and multi-action toolbar icons are
not defects in the approved slice. Further work should measure a real task
failure before broadening Sketch tools; high-DPI and post-fix visual capture
remain evidence gaps, not regressions.

**Export.** The `File → Export STL...` path already names the current result,
chooses a derived body, states binary STL and the millimetre convention,
offers two quality presets, a native destination, real Validate mesh, gated
Export and Cancel. The reference's split CAD/mesh presentation, format tiles
and green checklist are not current capability. A next Export slice should
first make validator outputs, active body/quality/path, invalidation after
parameter changes and final write result plainly visible. It must not label a
mesh “ready for slicer” solely because internal manifold checks passed: the
v1.2 N2 slicer check was waived. A split preview is a separate, higher-risk
candidate using the actual export mesh.

**Manufacture.** UX-REF-03 supplies future visual consistency only. No CAM
tree, stock, operation, simulation timeline or G-code button should be
displayed as functional before a separate product/architecture decision.

## 6. Productivity effect on the bounded mechanical-part workflow

| Workflow step | Current support | Remaining friction and useful evidence |
| --- | --- | --- |
| Create Sketch → Dimension | Plane/face entry, supported primitives and UX-02 direct callouts work | Preserve mode/Exit and callout alignment; capture DPI and different plane views at next gate. |
| Extrude → Sketch-on-Face → Cut | Existing contextual actions and valid bounded geometry work | Selected pocket face must not imply a false side-wall break; show exact selected face and owning feature. |
| Edit → Rebuild | Inspector drafts, Undo/Redo and FAILED/BLOCKED exist | Make selected context, disabled reason, draft outcome and recovery scannable without internal IDs. |
| Save/Open | Native dialog, `.microsw` v2, dirty/history safety are frozen | Model UX work must leave source/target and edited dimensions persistent, selection transient. |
| Export STL | C2 validates a CAD-derived body and writes binary STL | Show honest selected body/quality/path, validation and write outcome; a visual mesh preview is not a substitute for validator evidence. |

The biggest near-term leverage is confidence in what part of the model is
selected and what action is currently possible. This also makes later Export
manual validation less ambiguous. It is more defensible than matching the
mockup's command count or materials.

## 7. Roadmap alternatives

`LOW/MEDIUM/HIGH` time-to-value means **speed to a useful owner-visible
improvement** (HIGH is faster). Estimates are relative; no implementation
hours are asserted.

| Criterion | A — Model refinement | B — Export UX | C — mixed productivity slice |
| --- | --- | --- | --- |
| Daily user benefit | **HIGH**: selection truth, selected context and command clarity recur throughout modeling | **MEDIUM**: high at export, limited during editing | **MEDIUM–HIGH** across start/end, but little depth per surface |
| Functional-validation benefit | **HIGH**: selected geometry and failure recovery become easier to judge | **MEDIUM–HIGH**: visible STL preflight/result | **MEDIUM**: two partial gates complicate attribution |
| Estimated effort | **MEDIUM** for a first bounded pass; larger if search/cube included | **MEDIUM–HIGH** if actual mesh preview is included | **HIGH** because two flows and state owners change |
| Regression risk | **MEDIUM**: Viewer/UI selection and display contracts | **MEDIUM** for dialog; **HIGH** for split preview/export-state coherence | **HIGH** across Model, Viewer, export and UI |
| Architectural impact | **LOW** if derived Presentation/Viewer and Application UI only; stop on CAD topology finding | **LOW** for honest status; **MEDIUM** for a shared exact candidate preview | **MEDIUM** from cross-flow coupling and divided ownership |
| Dependencies | Existing FeatureModel, FeatureTreeModel, EditorContext, SolidDisplay, WorkspaceViewport, SET-01 | Existing ExportMesh/StlWriter/validator, UI, persistence session | Both groups; stable cross-screen state contracts needed |
| Time-to-value | **HIGH** with a first face-context slice | **MEDIUM** for preflight; LOW for full reference shell | **LOW–MEDIUM** due to fragmented acceptance |

**Recommendation: A.** It addresses an owner-observed misleading surface and
the incomplete Model context at the center of the whole part workflow, while
keeping the first implementation slice small and testable. B remains important
once users can inspect and edit the correct body confidently. C risks a
shallow UI pass that neither fixes selection trust nor proves better export
outcomes. This recommendation does not reopen UX-01/UX-02 or authorize A.

## 8. Proposed sequence and nomenclature

1. **UX-04 — Model Workspace Visual & Interaction Refinement**: proposed next
   campaign; first executable increment `UX-04.1` below. This is a **proposed
   reassignment** of the prior, unstarted `UX-04 — Future workspaces` vision
   placeholder, not a renumbering of any frozen campaign. Owner approval of
   the name/sequence and bounded executable scope is required.
2. **UX-03 — Export UX**: retain the established identifier and name. After
   Model refinement, prioritize honest STL preflight/diagnostics; evaluate a
   preview separately against the actual export candidate and risk. No new
   format, importer, repair operation or independent printability claim.
3. **Manufacture and later workspaces**: keep as unnumbered vision until a
   product/architecture review approves an actual CAD→CAM workflow. UX-REF-03
   is not an executable commitment.

The numerical order is intentionally nonsequential: identifiers preserve the
already established `UX-03 — Export UX`; proposed execution order follows
workflow impact. If the owner prefers Export first, UX-03 can proceed under a
separate approved bounded plan without changing UX-04's proposed scope.

## 9. Architecture and risk assessment

FeatureModel and Geometry/Topology stay authoritative; Presentation and
Rendering are derived. EditorContext owns transient typed selection; the UI
may display facts and draft commands but must not authoritatively store CAD
faces or geometry. SET-01 preferences remain Application-owned and independent
of document dirty/history. `.microsw` v2, Undo/Redo, the actual STL export,
Dear ImGui/GLFW/OpenGL and all 71 accepted ADRs remain binding. No new external
dependency, CAD schema, mesh-authoritative model, persistent topological
naming, or accepted-ADR change is recommended here.

The main technical risk is mistaking an intentional per-face selected accent,
an actual exposed edge, a coplanar topology seam and a tessellation artifact
for one another. ADR-0056/0057/0060 require topology-derived edges and
rebuild-local sub-element IDs. A correction must not suppress real pocket
edges, show triangle wireframes, modify Solid validity or silently make face
selection a group of unrelated CAD faces. If the observed boundary instead
comes from invalid triangulation or Solid geometry, stop UX-04.1 and submit a
separate technical finding with fixture and affected ADRs.

## 10. Open owner decisions

**Subsequent resolution:** the owner authorized the bounded UX-04.1 Model
selection/context increment on 2026-10-21. The review below remains the
historical proposal; [UX-04.1 validation](UX_04_1_MODEL_SELECTION_VALIDATION.md)
tracks execution and the pending Experience Gate. UX-03 and later UX-04 slices
remain outside that authorization.

1. Approve or reject **Option A before UX-03**, including proposed use of
   `UX-04` for Model refinement and moving the old UX-04 future-workspace
   placeholder to unnumbered vision.
2. Decide whether the first Model gate needs 125%/150% Windows scaling on an
   available machine; if unavailable, record the omission rather than a pass.
3. Defer Browser search/body grouping, interactive View Cube, full Export
   workspace, CAD/mesh split preview and any new CAD/CAM functionality unless
   later evidence and a separately scoped approval justify them.

No architecture Decision Gate is needed for the documentary recommendation.
A material change to selection identity, CAD/mesh ownership, persistence,
module dependency or an accepted ADR would require the owner's separate
architectural approval before implementation.

## 11. Next executable increment proposal — UX-04.1

**Name:** `UX-04.1 — Model Selection and Context Clarity` (**AUTHORIZED;
implementation and Experience Gate tracked in
[UX-04.1 validation](UX_04_1_MODEL_SELECTION_VALIDATION.md)**).

**Objective:** on the saved extrude-plus-pocket fixture, selecting a side face
must show truthful continuous surface feedback and unmistakably identify the
selected face/owning feature and valid next actions, without changing CAD
state or selection identity.

**Scope:** reproduce and classify the pocket-floor tonal boundary in Shaded,
Shaded With Edges and Wireframe with no selection, hover, selected face and
selected feature. Inspect derived coplanar face/edge data and current
selection pass. Correct only a proven Presentation/Viewer highlight or edge
overlay cause. In the existing Model Inspector, group selected face/feature
identity, source/target and valid actions with clear enabled/disabled reasons;
retain one current command/result message. Keep the Browser selection cue in
sync. Use existing project-owned icons and existing commands. No new toolbar
family, search field or View Cube in this first slice.

**Excluded:** Geometry/Topology changes, CAD face merging, new feature types,
persistent naming, new selection sets, shaded material system, mesh export
changes, Sketch solver/geometry, CAM, new dependencies and UI framework change.

**Affected closure:** `microsw_presentation` and `microsw_viewer` only if the
reproduction locates the visual cause there; `microsw_ui` for selected-context
layout; direct UI/Viewer selection tests and Application integration consumers.
No changed core CAD target is presumed. Preserve ADR-0049/0050, 0056–0060,
0062/0063, 0070/0071 and `.microsw` v2.

**Automated tests (V2 targeted integration):** one saved/generated pocket
fixture checks Solid validity separately from derived display data; selected
face remains the same typed logical target, while visible boundary styling
respects coplanar seam suppression and retains actual pocket rim edges. Test
Shaded, Shaded With Edges and Wireframe pass policy; hover/selected precedence
in Orange and Blue; Browser↔viewport↔Inspector mapping; read-only inspection
does not dirty the document, add Undo, change rebuild result or alter STL
candidate. Incremental build only of the affected closure and directly
relevant tests, followed by `git diff --check`. Runtime validation covers the
changed visual behavior. Do not run full CTest or clean build unless a concrete
regression or baseline gate requires V3.

**Experience Gate:** the owner opens the same saved plate with a pocket,
compares unselected/hover/selected lateral faces, pocket wall/floor/rim and
selected feature in all three modes, both SET-01 accents, orbit/pan/zoom and
Perspective/Orthographic. Inspector must clearly name the target/owner and
valid action; no apparent false side-wall break may appear on selection.
Cancel/Confirm an existing feature edit, Undo/Redo, Save/Open and STL export
smoke must remain coherent. Inspect at 1920×1080 and 1366×768 logical layout;
record Windows scaling if available. The user should complete this without
internal IDs. Normal exit is confirmed.

**Evidence:** retain the reproducible `.microsw` fixture or deterministic
construction steps, before/after **actual executable** captures with mode,
selection, resolution and scaling labels, targeted test log, build result and
owner gate decision. Reference PNGs remain separately labeled design intent.

**STOP:** pause and report if the reproduction shows invalid CAD topology,
requires a change to an accepted ADR/selection identity/file format, requires
an external dependency, or cannot be corrected within the derived display
and bounded Inspector scope. No cosmetic mask of a geometry failure.

**Suggested commits after owner authorization:**
`[UX-04.1] Reproduce pocket face selection display` (tests/fixture), then
`[UX-04.1] Clarify Model face highlight and context` (minimal correction and
bounded UI), then a separate documentation/validation commit if the gate
passes. No implementation or commit in this V0 review belongs to UX-04.1.

## 12. Review acceptance and traceability

This review creates no new requirement ID or change to the frozen SPEC-01
register. Its current-capability clarification for bounded `CAD-005`,
`UX-001`–`UX-005` and `INT-001-EXP` is recorded here and in the live v0.3 gap
analysis. [CAPABILITY_MAP](specification/CAPABILITY_MAP.md) and
[REQUIREMENTS_TRACEABILITY](specification/REQUIREMENTS_TRACEABILITY.md)
deliberately retain their historical v1.1.0 evidence cuts and later dated
addenda; changing their frozen tables would misstate source chronology.

Acceptance requires: all four canonical references inspected; implemented
UX-01/02 behavior credited; Model/Sketch/Export evaluated; Manufacture kept
future-only; A/B/C compared; the observed shading finding classified without
assuming CAD corruption; a bounded next increment and STOP criteria supplied;
roadmap and gap matrix synchronized; no executable changes or new ADRs;
V0 consistency, reference-hash, ADR-inventory, `git diff --check` and clean
working tree after the dedicated documentation commit.

**V0 review result:** all four local reference hashes match the registry; the
ADR inventory is contiguous 0001–0071 with 71/71 `ACCEPTED`; links in the three
changed Markdown files resolve; frozen campaign/release states remain intact;
only this review, the roadmap and the v0.3 gap analysis changed. `git diff
--check` passed. Configure, build, CTest and runtime were intentionally not
run because this review changes no executable, build-system or dependency
file. The dedicated commit and clean-tree state are verified separately in
the task completion report.
