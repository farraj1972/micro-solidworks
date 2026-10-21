# Capability Map — v1.1.0 Evidence Cut

This APPROVED / FROZEN SPEC-01 map classifies **implementation**, not approval of the [DRAFT requirements](REQUIREMENTS_REGISTER.md). Evidence is frozen `v1.1.0` at `774f55c`, the [P0 code audit](../PERSONAL_SOLIDWORKS_CAPABILITY_MATRIX.md) and [v1.1.0 gate](../V1_1_0_RELEASE.md), not a new SPEC-01 build. Status vocabulary: `IMPLEMENTED_AND_VALIDATED` (bounded code and relevant tests/gate), `IMPLEMENTED_NOT_VALIDATED`, `PARTIALLY_IMPLEMENTED`, `FOUNDATIONAL_ONLY`, `NOT_IMPLEMENTED`, `UNKNOWN`. “Validated” never implies complete commercial-CAD parity or fulfillment of every clause of a broader draft requirement.

| Status | Parent IDs (29 total, each exactly once) | Evidence and limit |
|---|---|---|
| IMPLEMENTED_AND_VALIDATED | `CAD-001`, `UX-001` | Native `.microsw` v2/v1 migration, Save/Open and mouse-first bounded CAD workflow have code/tests and release/Experience evidence. |
| IMPLEMENTED_NOT_VALIDATED | *(none)* | No unsupported positive claim needed. |
| PARTIALLY_IMPLEMENTED | `CAD-002`, `CAD-003`, `CAD-005`, `CAD-007`, `UX-002`, `UX-003`, `UX-004`, `UX-005` | Solver/sketch feedback, bounded Boolean/Cut, selection, multi-Solid/semantic face roles, selected preview/diagnostics/Experience Gate exist; full source requirement coverage is unverified. |
| FOUNDATIONAL_ONLY | `INT-001` | [`SolidDisplay`](../../src/presentation/SolidDisplay.h) derives triangles for viewing; no STL writer/reader or export-quality contract. |
| NOT_IMPLEMENTED | `CAD-004`, `CAD-006`, `CAD-008`, `CAD-009`, `CAD-010`, `INT-002`, `INT-003`, `INT-004`, `INT-005`, `SCR-001`, `SCR-002`, `SCR-003`, `CAM-001`, `CAM-002`, `CAM-003`, `CAM-004`, `CAM-005`, `CAM-006` | No delivered named-expression system, broad modeling/assembly/drawing, listed exchange adapters, script host or CAM/NC domain found. |
| UNKNOWN | *(none)* | If future evidence is insufficient, use UNKNOWN rather than guessing. |

## Compound capability distinctions

| Stable child or group | Current capability | Planning status |
|---|---|---|
| `CAD-003-EXT` | Supported general additive extrusion including holed/curved planar profiles, tested. | Existing validated bounded capability. |
| `CAD-003-CUT` | Supported axial subtraction from an Extrude result; stacked Cut→Cut target fails in [`FeatureModel::rebuild`](../../src/features/FeatureModel.cpp). | Existing partial; [v1.2 synthetic fixture](../PERSONAL_SOLIDWORKS_V1_2_CAMPAIGN_PROPOSAL.md) must preflight hole-in-base-profile + one pocket cut. |
| `CAD-003-BOOL` | Axis-aligned box/cell Boolean subset, not general B-rep Boolean. | Existing partial; general Boolean deferred. |
| `CAD-003-EDIT`, `CAD-003-RENAME`, `CAD-003-REBUILD` | Edit/rename/rebuild and Undo/Redo in bounded C5 workflow. | Existing validated bounded capability. |
| `CAD-006-*` / `CAD-010-*` | No Revolve, mirror, pattern, fillet, chamfer, Hole Wizard, sweep/loft/sheet metal/families. | Deferred. |
| `CAD-007-MB` / `CAD-007-DAT` / `CAD-007-TOP` | Derived multi-Solid results; principal reference planes in UI; restricted semantic cap roles only. | MB partial; DAT foundational; TOP general persistent naming deferred. |
| `INT-001-EXP` / `INT-001-IMP` | Neither STL export nor import delivered. | **Export is approved product priority** but implementation plan unapproved; import deferred. |
| `INT-002-*`, `INT-003-*`, `INT-004-*` | No STEP, OBJ, 3MF, DXF, glTF/GLB or IGES adapter. | Deferred. |
| `CAM-002-*`, `CAM-003-*`, `CAM-004-*`, `CAM-006-*` | No CAM aggregate/toolpath/controller implementation. | DRAFT/deferred, including first conceptual offline 2.5D 3-axis slice. |
| `UX-005-CONTEXT`, `UX-005-SEARCH`, `UX-005-ONBOARD` | Bounded contextual UI exists; command search and onboarding are unverified/absent. | Context partial; search/onboarding deferred. |

The classification is tied to the parent ID. A compound parent may be `PARTIALLY_IMPLEMENTED` even when one child is validated and another is absent. The [traceability ledger](REQUIREMENTS_TRACEABILITY.md) gives concrete modules, ADRs, tests and roadmap mapping; unverified links are marked `NOT VERIFIED`.

## Current v1.2 candidate addendum (does not revise the frozen v1.1.0 cut)

`INT-001-EXP` now has a validated CAD-derived export mesh and a binary STL
writer with independent parser tests in [C1](../PERSONAL_SOLIDWORKS_V1_2_C1_VALIDATION.md)
and [C2](../PERSONAL_SOLIDWORKS_V1_2_C2_VALIDATION.md). The bounded export UI
has owner-reported PASS on a non-N2 part. The [C3 portable candidate](../PERSONAL_SOLIDWORKS_V1_2_C3_VALIDATION.md)
has passed same-PC Save/Open/Export but not a clean-environment test. The owner
subsequently [accepted one as-built N2 STL with explicit exceptions](../PERSONAL_SOLIDWORKS_V1_2_N2_ACCEPTANCE.md):
the pocket differs from the original fixture, and independent slicer and clean
Windows checks were waived rather than passed. Final V3 passed 927/927 tests
and normal startup/shutdown; v1.2.0 is frozen under that revised bounded gate.
`INT-001-IMP` remains not implemented.
Thus the compound `INT-001` is at most **partially implemented** in the
current candidate; the historical table above continues to describe v1.1.0.

## UX-01 current addendum (historical SPEC-01 cut unchanged)

Frozen v1.2.0 delivered bounded STL export; UX-01 is a separately frozen
Model/Sketch workspace modernization. The left Model Browser, right Context
Inspector, contextual ribbon and status guidance are implemented in
[`ApplicationShell`](../../src/ui/ApplicationShell.cpp). These add evidence for
the bounded `CAD-005` and `UX-001`–`UX-005` interaction children, **not** full
approval or fulfillment of their DRAFT parent requirements. Targeted UI,
selection, editor, session and preferences tests pass; UX-01's final V3
(928/928) and owner Experience Gate (APPROVED WITH CHANGES) are tracked in
[`UX_01_CAD_WORKSPACE_VALIDATION`](../UX_01_CAD_WORKSPACE_VALIDATION.md).
Command search, in-canvas dimensions, floating toolbar and richer Export UX
remain UX-02/UX-03 plans. All deferred modeling, STEP/DXF/3MF, drawings,
assemblies, scripting and CAM capability records above remain preserved.
