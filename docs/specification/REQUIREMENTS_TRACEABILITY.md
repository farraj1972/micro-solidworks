# Requirements Traceability Ledger — SPEC-01 FROZEN

Join by stable ID with the [register](REQUIREMENTS_REGISTER.md) and [capability map](CAPABILITY_MAP.md). This ledger is an APPROVED / FROZEN traceability baseline, not approval of the 29 DRAFT requirements. `Source` for every parent is [specification v0.1 §3](sources/MICRO_CADCAM_PRODUCT_SPEC_V0_1.md); the [handoff](sources/MICRO_SOLIDWORKS_HANDOFF_ESTRATEGIA_CADCAM_P0_V12_2026-10-19.md) is informative. `Relevant ADRs` indicate constraints or independently approved bounded capabilities, **not approval of the proposed aggregate requirement**. Tests/Experience evidence refers to the historical frozen release and only the bounded capability stated. `NOT VERIFIED` means no matching implementation/test/Experience claim was established in this audit. R0–R9 are conceptual epochs; only [canonical roadmap](../ROADMAP.md) and [P0 decisions](../PERSONAL_SOLIDWORKS_P0_DECISIONS.md) determine approved scope.

| Requirement → target capability | Architecture contract / relevant ADRs | Code module | Tests / Experience Gate / evidence | Roadmap and dependencies |
|---|---|---|---|---|
| `CAD-001` → native document roundtrip | FeatureModel authority, persistent aggregate IDs and transactional Save/Open; ADR-0041–0048, 0061–0063, 0066 | [`FeatureModel`](../../src/features/FeatureModel.h), [`Persistence`](../../src/persistence/Persistence.h), [`PersistenceSession`](../../src/app/PersistenceSession.h) | [`test_persistence`](../../tests/persistence/test_persistence.cpp), [`test_document_history`](../../tests/app/test_document_history.cpp), [v1.1.0 gate](../V1_1_0_RELEASE.md) | Existing v1.1.0; future general Document would need ownership/schema gate. |
| `CAD-002` → sketch/constraints | Sketch owns local entities, solver separate; ADR-0028–0034 | [`Sketch`](../../src/core/sketch/Sketch.h), [`ConstraintSolver`](../../src/constraints/ConstraintSolver.h) | [`test_sketch`](../../tests/sketch/test_sketch.cpp), [`test_constraint_solver`](../../tests/constraints/test_constraint_solver.cpp); full DOF feedback NOT VERIFIED | Existing partial; R2 conceptual; richer UX deferred. |
| `CAD-003` → Extrude/Cut/Boolean/edit/rebuild | Ordered FeatureModel, atomic rebuild and bounded geometry; ADR-0035–0044, 0053–0055, 0064–0065, 0068–0069 | [`FeatureModel`](../../src/features/FeatureModel.h), [`GeneralExtrusion`](../../src/features/GeneralExtrusion.h), [`AxialSubtraction`](../../src/features/AxialSubtraction.h), [`Boolean`](../../src/modeling/Boolean.h) | [`test_feature_model`](../../tests/features/test_feature_model.cpp), [`test_general_extrusion`](../../tests/modeling/test_general_extrusion.cpp), [`test_axial_subtraction`](../../tests/features/test_axial_subtraction.cpp), [`test_boolean`](../../tests/modeling/test_boolean.cpp), [C5 gate](../V1_1_C5_VALIDATION.md) | Existing bounded; v1.2 preserves it; general Boolean/stacked Cut deferred. |
| `CAD-004` → named parameters/expressions/units | New authoritative parameter ownership/schema would need a gate; existing feature scalar values are not named-expression engine | NOT VERIFIED | NOT VERIFIED | R2 conceptual only; outside v1.2. |
| `CAD-005` → tree↔viewport selection | Editor owns logical transient selection; ADR-0049–0052, 0057–0059 | [`EditorContext`](../../src/interaction/EditorContext.h), [`FeatureTreeModel`](../../src/ui/FeatureTreeModel.h), [`SolidPicker`](../../src/viewer/SolidPicker.h) | [`test_editor_context`](../../tests/interaction/test_editor_context.cpp), [`test_feature_tree_model`](../../tests/ui/test_feature_tree_model.cpp), [C5 gate](../V1_1_C5_VALIDATION.md) | Existing partial; general topology identity deferred. |
| `CAD-006` → Revolve/mirror/pattern/fillet/chamfer/Hole | Internal kernel frozen for first N2 only; future general B-rep decision; ADR-0006, 0025–0027, 0054, 0065 | NOT VERIFIED | NOT VERIFIED | R5 conceptual; no v1.2 authorization. |
| `CAD-007` → multi-body/datums/persistent references | Derived multi-Solid and restricted semantic face support; ADR-0025–0027, 0055, 0057, 0064, 0069 | [`SolidFeatureCache`](../../src/features/FeatureModel.h), [`Solid`](../../src/core/topology/Solid.h), [`ApplicationShell`](../../src/ui/ApplicationShell.h) | [`test_general_extrusion`](../../tests/modeling/test_general_extrusion.cpp), [`test_feature_model`](../../tests/features/test_feature_model.cpp); general naming NOT VERIFIED | Existing partial; R5 conceptual; persistent naming future gate. |
| `CAD-008` → assemblies/mates | No assembly ownership/identity ADR approved | NOT VERIFIED | NOT VERIFIED | R5 conceptual; deferred. |
| `CAD-009` → drawings | No drawing-document contract approved | NOT VERIFIED | NOT VERIFIED | R5/R7 conceptual; deferred. |
| `CAD-010` → advanced surfaces/sweep/loft/sheet/families | General surface/kernel choice OPEN under P0-D03; ADR-0006 currently binding | NOT VERIFIED | NOT VERIFIED | R9 conceptual; deferred. |
| `INT-001` → STL mesh I/O (`EXP`/`IMP`) | CAD authoritative / mesh derived; ADR-0056; P0-D04 export priority | [`SolidDisplay`](../../src/presentation/SolidDisplay.h) **foundation only**; no STL adapter | [`test_solid_display`](../../tests/presentation/test_solid_display.cpp) covers display derivation only; STL writer/reader NOT VERIFIED | `EXP`: [v1.2 proposal C1–C2](../PERSONAL_SOLIDWORKS_V1_2_CAMPAIGN_PROPOSAL.md), not authorized; `IMP`: deferred. |
| `INT-002` → STEP exact B-rep I/O | P0-D03 K1/K2/K3 OPEN; future exchange/schema gate, ADR-0006/0045–0048 | NOT VERIFIED | NOT VERIFIED | R7 conceptual; deferred. |
| `INT-003` → OBJ/3MF | Mesh-derived adapter boundary must be defined; no format ADR | NOT VERIFIED | NOT VERIFIED | R7 conceptual; deferred. |
| `INT-004` → DXF/glTF/IGES | 2D CAD versus visual mesh versus legacy surface representation require separate profiles | NOT VERIFIED | NOT VERIFIED | R3/R7 conceptual; deferred. |
| `INT-005` → preview/import report | Editor transaction and document safety would constrain importer; ADR-0049–0052, 0061–0063 | NOT VERIFIED | NOT VERIFIED | R3 conceptual; deferred; no import authorization in v1.2. |
| `SCR-001` → read-only script runner | Future versioned API/security Decision Gate; P0-D06 excludes | NOT VERIFIED | NOT VERIFIED | R4 conceptual; deferred despite source P0 priority. |
| `SCR-002` → scripted parametric commands | Must preserve FeatureModel transaction/rebuild/persistence; ADR-0041–0048, 0052, 0062 | NOT VERIFIED | NOT VERIFIED | R4 conceptual; deferred. |
| `SCR-003` → script capabilities/quotas | Future threat model and runtime choice required | NOT VERIFIED | NOT VERIFIED | R4 conceptual; deferred. |
| `CAM-001` → associative ManufacturingPlan | Future CAD revision identity and CAM ownership; FeatureModel remains CAD authority, ADR-0041–0048 | NOT VERIFIED | NOT VERIFIED | R6 conceptual; deferred. |
| `CAM-002` → setup/WCS/stock/fixtures/tools | Future manufacturing aggregate and unit/coordinate contract | NOT VERIFIED | NOT VERIFIED | R6 conceptual; deferred. |
| `CAM-003` → offline 2.5D operations | Future geometry/offset/toolpath/safety contract; current Cut is not CAM | NOT VERIFIED | NOT VERIFIED | R6 conceptual; deferred. |
| `CAM-004` → removal/collision simulation | Future simulation validity contract; cannot certify machine safety alone | NOT VERIFIED | NOT VERIFIED | R6 conceptual; deferred. |
| `CAM-005` → NC postprocessors | Future controller profile, human confirmation and NC safety gate | NOT VERIFIED | NOT VERIFIED | R6 conceptual; deferred. |
| `CAM-006` → advanced milling/turning | Future kinematics and safety architecture | NOT VERIFIED | NOT VERIFIED | R8 conceptual; deferred. |
| `UX-001` → mouse/context CAD tasks | D13 editor/product contract; ADR-0049–0052, 0059 | [`ApplicationShell`](../../src/ui/ApplicationShell.h), [`EditorContext`](../../src/interaction/EditorContext.h) | [`test_editor_context`](../../tests/interaction/test_editor_context.cpp), [C5 Experience Gate](../V1_1_C5_VALIDATION.md); future CAM UX NOT VERIFIED | Existing bounded; future visible work needs Experience Gate. |
| `UX-002` → preview/Confirm/Cancel | Editor transaction and session history; ADR-0052, 0062, 0065 | [`ApplicationShell`](../../src/ui/ApplicationShell.h), [`DocumentHistory`](../../src/app/DocumentHistory.h) | [C5 Experience Gate](../V1_1_C5_VALIDATION.md); universal continuous preview NOT VERIFIED | Existing partial; v1.2 export flow needs its own UX gate. |
| `UX-003` → contextual diagnostics | Rebuild FAILED/BLOCKED; ADR-0043, 0052, 0065 | [`FeatureModel`](../../src/features/FeatureModel.h), [`ApplicationShell`](../../src/ui/ApplicationShell.h) | [`test_feature_model`](../../tests/features/test_feature_model.cpp), [C5 gate](../V1_1_C5_VALIDATION.md) | Existing partial; future importer/CAM diagnostics deferred. |
| `UX-004` → task Experience Gate | D13/P0 user-visible quality contract; no new ADR | Gate records in [`V1_1_C5_VALIDATION`](../V1_1_C5_VALIDATION.md) | C5 manual Experience Gate APPROVED; source's all-task/HiDPI matrix NOT VERIFIED | Required for approved visible campaigns; [v1.2 plan](../PERSONAL_SOLIDWORKS_V1_2_CAMPAIGN_PROPOSAL.md) pending. |
| `UX-005` → discoverability/search/onboarding | CAD-first contextual UI under ADR-0049–0052, 0059 | [`ApplicationShell`](../../src/ui/ApplicationShell.h), [`FeatureTreeModel`](../../src/ui/FeatureTreeModel.h) | [C5 gate](../V1_1_C5_VALIDATION.md) bounded; command search/onboarding NOT VERIFIED | R3 conceptual; deferred unless future fixture demonstrates need. |

**Architecture/capability distinction:** accepted ADRs constrain existing code; they do not automatically approve new aggregate types, dependencies or file formats. The release's 903/903 test result validates its bounded executable state, not DRAFT CAM/scripting/interop requirements. The `INT-001-EXP` handoff is an approved product priority only; C1–C4 are not yet implementation-authorized. The child IDs in the [register](REQUIREMENTS_REGISTER.md#stable-child-ids-for-compound-requirements) inherit parent source/approval but retain their own scope distinction.

## Current v1.2 candidate addendum (historical SPEC-01 ledger preserved)

The owner subsequently authorized C1–C4. `INT-001-EXP` now maps to
[`ExportMesh`](../../src/export/ExportMesh.h), [`StlWriter`](../../src/export/StlWriter.h)
and the File → Export STL UI in [`ApplicationShell`](../../src/ui/ApplicationShell.cpp).
The [C1 mesh tests](../../tests/export/test_export_mesh.cpp) and
[C2 independent STL parse tests](../../tests/export/test_stl_writer.cpp)
pass in targeted validation. The bounded export UI has owner-reported PASS
for a non-N2 part. The
[portable package](../PERSONAL_SOLIDWORKS_V1_2_C3_VALIDATION.md) passed
same-PC Save/Open/Export; a clean Windows environment was not tested. The
owner [accepted an as-built N2 STL with explicit exceptions](../PERSONAL_SOLIDWORKS_V1_2_N2_ACCEPTANCE.md),
waiving the original pocket position and independent slicer check for this
release. Final V3 passed 927/927 tests and normal startup/shutdown;
v1.2.0 is frozen under that revised bounded gate. `INT-001-IMP` is still
absent. These later implementation
facts do not retroactively alter the frozen v1.1.0 evidence cut above.

## UX-01 current trace (historical ledger unchanged)

The owner authorized UX-01 after frozen v1.2.0. The bounded `CAD-005` and
`UX-001`–`UX-005` interaction work maps to
[`ApplicationShell`](../../src/ui/ApplicationShell.cpp),
[`FeatureTreeModel`](../../src/ui/FeatureTreeModel.cpp),
[`EditorContext`](../../src/interaction/EditorContext.cpp), and
[`WorkspaceViewport`](../../src/viewer/WorkspaceViewport.cpp) under existing
ADR-0049–0052 and frozen SET-01 preferences. No new authoritative CAD state or
dependency direction was introduced. The left browser/right inspector, Model/
Sketch ribbon and guidance are recorded in
[`UX-01 validation`](../UX_01_CAD_WORKSPACE_VALIDATION.md), with targeted
layout, tree, editor, viewport, history and preference tests. Final V3 passed
928/928, and the owner Experience Gate is APPROVED WITH CHANGES, with the
remaining conceptual/design differences to be formalized later. The full
DRAFT requirements, UX-02/UX-03 and CAM horizon remain
unchanged; this addendum does not approve them.

## UX-04.2 current trace (historical ledger unchanged)

The owner-approved [UX-04.2 gate](../UX_04_2_BROWSER_INSPECTOR_VALIDATION.md)
adds bounded `CAD-005`, `UX-001`–`UX-003` and `UX-005-CONTEXT` evidence:
[`ApplicationShell`](../../src/ui/ApplicationShell.cpp) derives the Browser and
contextual Inspector from existing FeatureModel and EditorContext state;
[`FeatureTreeModel`](../../src/ui/FeatureTreeModel.cpp) preserves feature order,
single absorbed-Sketch placement and human-named FAILED/BLOCKED diagnostics.
Directed [UI tests](../../tests/ui/test_transform_editor.cpp),
[tree/selection tests](../../tests/ui/test_feature_tree_model.cpp),
EditorContext, history and persistence contracts passed targeted validation.
This does not fulfill search/onboarding, create persistent topology identity,
approve later UX-04/UX-03 work or revise the frozen source requirement status.
