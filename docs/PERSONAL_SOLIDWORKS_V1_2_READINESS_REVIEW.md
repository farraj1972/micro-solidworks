# V1.2-READY — Product, Geometry, UI/UX & Execution Alignment

Status: **COMPLETE / READY FOR OWNER APPROVAL** (2026-10-20). This is a V0
planning review, not authorization to implement v1.2-C1–C4. The first proposed
functional replacement is a printable, parametric adapter plate, not general
SolidWorks parity or an N2 rating for the whole product.

## 1. Executive Summary

The entry evidence for C1 is sufficient. P0 has already approved the bounded
STL-first product campaign, the R2 repair removed the reference CAD geometry
blocker and its CAD Experience Gate passed, and SET-01 is frozen. The remaining
high-risk unknown is **export-quality derived tessellation and validation**;
that is the work of C1, not evidence that C1 cannot start. The owner can approve
the sequential C1 → C2 → C3 → C4 → FREEZE plan below without another generic
readiness review. Each increment retains its own technical and Experience
checks; a material conflict with an accepted ADR is a hard stop.

No STL export, independent mesh validation, slicer acceptance, portable
Windows package or complete N2 claim is established by this review.

## 2. Source & Baseline Verification

| Source / state | Verified position |
|---|---|
| Git | `master`, clean at entry, HEAD `25c78cc8dcfacdd6bff1e1ca3019c878547a4297`; remote `github` points to `farraj1972/micro-solidworks`. Local `v1.1.0` tag resolves to `774f55c`; local `set-01-application-preferences` tag resolves to `25c78cc`. The owner reports the SET-01 commit/tag published; this task does not push. |
| v1.1.0 | FROZEN, [release evidence](V1_1_0_RELEASE.md); baseline remains historically unchanged. |
| P0 | [Seven approved decisions](PERSONAL_SOLIDWORKS_P0_DECISIONS.md), FROZEN; first N2 is the bounded plate workflow. |
| SPEC-01 | [Traceability baseline](specification/README.md) FROZEN. Its 29 source requirements remain DRAFT / PROPOSED; neither their source priority nor the traceability freeze authorizes implementation. |
| Geometry R2 | [Corrective preflight](PERSONAL_SOLIDWORKS_V1_2_GEOMETRY_PREFLIGHT.md) PASS with owner CAD Experience Gate approved. |
| SET-D01 / SET-01 | FROZEN, [validation](PERSONAL_SOLIDWORKS_SET_01_VALIDATION.md) 918/918 V3 and owner Experience Gate approved. |
| ADRs | 71 local ADR files, 71 with `Status: ACCEPTED`; none amended here. |
| UI/UX v0.3 | [Specification](uiux_v03_pkg/MICRO_SOLIDWORKS_UIUX_SPEC_V0_3.md) PROPOSED — DESIGN REFERENCE; four registered images are design inputs, not product assets or authorization. |
| v1.2-C1–C4 | Proposed detailed plan; **NOT AUTHORIZED FOR IMPLEMENTATION** until the owner approves it. |

The authority order is accepted ADRs and repository contracts, frozen P0 and
roadmap, executable/validation evidence, then the frozen SPEC-01 traceability
record and its DRAFT source. The archived specification is not retroactively
rewritten or upgraded by this review. See [dependency policy](DEPENDENCY_POLICY.md)
and [architecture](ARCHITECTURE.md).

## 3. P0 / SPEC-01 Alignment

P0-D01/D02 require workflow-first, CAD-integrity-first replacement of a real
bounded part task. P0-D03 keeps the internal kernel for **this first N2 only**;
K1/K2/K3 remains **OPEN — EVIDENCE REQUIRED** and is not a C1 entry blocker.
P0-D04 prioritizes validated STL **export**, not import, STEP or CAM. P0-D05
fixes the synthetic plate as the first N2 fixture. P0-D06 approves the campaign
theme while reserving detailed C1–C4 authorization to the owner. P0-D07
requires a new decision gate only for a material architecture conflict.

SPEC-01's `INT-001-EXP` is an approved **product priority** but an unimplemented
capability. [Capability map](specification/CAPABILITY_MAP.md),
[requirements register](specification/REQUIREMENTS_REGISTER.md) and
[traceability ledger](specification/REQUIREMENTS_TRACEABILITY.md) have a
v1.1.0 evidence cut; their STL import, CAM, scripting and broader CAD/CAM
requirements are not approved v1.2 scope. `CAD-001/002/003` and `UX-001/002/003`
provide bounded existing workflow evidence, not proof of every DRAFT clause.

## 4. Geometry R2 Evidence and Canonical Fixture

R1's `Incident Faces must traverse a shared Edge oppositely` is **historical,
not an active blocker**. R2 traced the failure to orientation loss while
splitting a reversed circular inner boundary in axial subtraction, repaired
directed splitting without relaxing manifold validation, and separately fixed
display-only multi-hole cap triangulation. The latter does **not** qualify a
mesh for STL. Permanent tests include
[`AxialSubtraction.PreservesThroughHoleDuringIndependentBlindPockets`](../tests/features/test_axial_subtraction.cpp),
[`PersistenceSessionHistory.HoledReferencePlateSurvivesEditsHistoryAndVersionTwoRoundtrip`](../tests/app/test_document_history.cpp)
and [`SolidDisplay.MultiHoleTopCapKeepsFullAreaAfterPlateWidthEdit`](../tests/presentation/test_solid_display.cpp).
R2's 78/78 targeted tests and subsequent 41/41 dimension-display tests, plus
the owner-observed CAD gate, establish the bounded CAD chain.

| Fixture state | Exact analytic CAD volume | CAD AABB |
|---|---:|---|
| Initial | 31,093.8761403405 mm³ | X `[-40,40]`, Y `[-25,25]`, Z `[0,8]` mm |
| Width 80 → 90 mm, pocket still 2 mm | 35,093.8761403405 mm³ | X `[-45,45]`, Y `[-25,25]`, Z `[0,8]` mm |
| Width 90 mm, pocket depth 2 → 3 mm | 34,841.8761403405 mm³ | X `[-45,45]`, Y `[-25,25]`, Z `[0,8]` mm |

The initial XY sketch has an 80 × 50 mm outer rectangle and Ø8 mm circular
inner loop centred at `(-22,0)` mm. Extrude +8 mm makes the through-hole. An
EndCap Sketch at Z=8 contains an 18 × 14 mm pocket centred at `(16,0)` mm;
one reverse Cut Extrude removes 2 mm, then 3 mm after edit. Width changes
symmetrically about X=0 while the hole and pocket centres remain fixed.
R2 measured matching volume from **analytic CAD topology**, not triangles,
and matching AABBs; it also covered semantic support, edit/rebuild, Undo/Redo
and `.microsw` v2 Save/Open. CAD volume and future triangulated mesh volume
must be reported separately. The known `Cut Extrude → Cut Extrude` target
limitation remains a future functional gap; this fixture uses only one Cut.

## 5. SET-01 Integration

ADR-0070/0071 and [SET-01 validation](PERSONAL_SOLIDWORKS_SET_01_VALIDATION.md)
freeze per-user Profile A/B navigation, Orange/Blue selection, local persistence
and Apply/OK/Cancel/Restore Defaults. UI/UX v0.3's MMB Pan and blue-selection
differences are **RESOLVED BY CONFIGURABLE PREFERENCES**. They do not reopen
the defaults or the frozen settings decision.

Future v1.2 Experience Gates must show that modeling, Export and diagnostics
work with the available navigation and accent choices. C2/C4 should exercise
both choices on representative workflows and check that export result, units,
mesh validation and document state are independent of preference values; an
exhaustive preference matrix for every export test is unnecessary without a
specific impact. Preferences remain separate from `.microsw`, CAD Undo/Redo
and geometric rebuild.

## 6. UI/UX v0.3 Adoption

The four canonical references are UX-REF-01 Model, UX-REF-02 Sketch,
UX-REF-03 Manufacture and UX-REF-04 Export in the
[visual registry](uiux_v03_pkg/VISUAL_REFERENCE_REGISTRY_V0_3.md). The images
show design intent, not approved commands or the dimensions of the N2 fixture.
The mockup's `.mcad` label does not replace `.microsw` v2. No artwork is copied.

For v1.2, the **mandatory** STL experience is mouse-first, discoverable and
contextual within the existing CAD shell: clear format, selected result/body,
destination, explicit millimetres convention and quality choice, true
validation state and actionable failure recovery in the current dark theme.
UX-REF-04 guides the hierarchy, but C2 need not build its entire Export
Workspace, split preview or unapproved format buttons. A compact modal or
context panel is acceptable if the Experience Gate proves the workflow.

Minimum C2 flow: **File / Export → STL → review active model/result and body
choice → destination → units and quality → validate → export → result or
diagnostic**. The Export button cannot claim “ready for 3D printing” merely
because bytes were written; visible success follows the actual mesh/file
validator. Preview, if offered, must represent the derived export mesh rather
than silently showing the CAD display mesh as an export certificate.

UX-01 Model/Sketch shell alignment, UX-02 guided Sketch, in-canvas dimension
editing, floating micro-toolbars and the fuller left-browser/right-inspector
layout are **separate proposed work**, not automatic first-N2 prerequisites.
UX-REF-03 Manufacture, simulation/timeline, CAM toolpaths, scripting,
assemblies and STEP/3MF/DXF remain future horizons. Do not surface their
illustrated controls as working commands. The v0.3 specification stays
**PROPOSED — DESIGN REFERENCE**.

## 7. STL Export Contract Readiness — C1 then C2

The accepted ownership direction is:

```text
FeatureModel / analytic topology::Solid (authoritative)
    -> project-owned CAD-facing tessellation/export adapter
    -> derived export mesh
    -> mesh validator
    -> transactional STL writer
    -> independent parser / slicer
```

The existing [`SolidDisplay`](../src/presentation/SolidDisplay.cpp) derives
triangles for viewing under [ADR-0056](adr/ADR-0056-solid-display-tessellation-and-shading-boundary.md).
Its rendering/picking coherence is useful evidence, but export-quality seams,
closedness and limits are **NOT VERIFIED**. C1 must decide whether a CAD-facing
adapter can safely reuse any algorithm; it must never export OpenGL buffers or
infer authoritative topology from a visual mesh.

**C1 contract and validator exit:** fixture-based deterministic tessellation
with explicit chordal and angular quality controls; exact-topology face/wire
coverage including planar inner loops and cylindrical faces; shared seam
vertices welded within an explicit export policy; outward triangle winding
and normals; finite, nondegenerate triangles; two oppositely traversed incident
triangles per closed manifold mesh edge; closed components; coherent positive
signed volume, mesh AABB and resource bounds. Validate a holed plate and
blind pocket, a round-pocket/curved-boundary case, repeated identical export
mesh, multiple components/bodies and negative cases: empty/invalid results,
FAILED/BLOCKED features, NaN/Inf, unsupported overlap, resource exhaustion
and output failure. Decide how self-intersection uncertainty is rejected for
the supported fixture subset; do not claim a general detector without one.
CAD and mesh volume/AABB are separate measurements with agreed comparison
tolerances. Viewport tessellation tolerance is not an export tolerance.

**C1 may settle without a new ADR** (while preserving accepted contracts):
binary versus ASCII STL, initial quality profile, normal computation, active
result/body selection and multi-body policy, empty/invalid rejection, atomic
file strategy, independent parser strategy and supported volume/AABB error
bounds. Determine numeric thresholds from fixtures, not guesses. C1 may
implement a narrow export-mesh builder/validator and test harness needed to
prove these contracts; it must not build the C2 file command or Export UI.
Material ownership, kernel, dependency, persistence or representation changes
require the specific owner decision described under STOP criteria below.

**C2 exit:** user-triggered STL writer and usable Export UI driven by the C1
validator; no partial success after a failed validation/write. STL is unitless,
so UI, guide and independent slicer check must explicitly use millimetres;
do not claim an embedded STL unit. A parser independent of the writer must
read the resulting file and verify triangle count, finitude, winding, bounds
and deterministic output. The slicer/consumer gate is separate from these
technical checks. STL import remains deferred.

## 8. Windows Distribution — C3

Prefer a versioned portable Windows Release package if evidence supports it;
do not start an installer, updater or telemetry. C3 inventories executable,
VC++ runtime/DLLs, GL/GLFW/ImGui requirements and assets, and checks install
layout from CMake. Test from an unpacked directory outside the source/build
tree on an adequately isolated Windows 10 environment without developer
tools. Verify startup, native Save/Open (including v1/v2 read compatibility),
STL export/validation, `%LOCALAPPDATA%` preferences, clean shutdown,
diagnostics for missing resources and safe handling of failed operations.
Record package version and the independent environment. A successful launch
on the developer machine alone does not establish C3.

## 9. First N2 Acceptance — C4

The bounded workflow is Create → Sketch → Dimension/Constrain → Extrude →
Sketch on Face → Cut → Edit → Rebuild → Undo/Redo → Save/Open → STL Export →
Independent Validation → Slicer Preparation → Windows Portable Use. C4 uses
the exact fixture and edited states above, a user guide matching actual UI
commands, and a mechanically CAD-literate owner Experience Gate without
technical IDs or commercial CAD.

Record **separate** evidence for (1) valid analytic CAD Solid, (2) valid
derived mesh, (3) correctly written and independently parsed STL, (4) slicer
acceptance at intended millimetre scale/orientation with hole and pocket,
(5) portable Windows use, and (6) approved user experience. Record slicer
name/version and result. A physical print is optional supplementary evidence.
N2 attaches only to this task and cannot be declared before every mandatory
gate passes; it never implies product-wide N2.

## 10. Proposed Execution and Entry Classification

| Stage | Entry | Deliverable / exit |
|---|---|---|
| **C1 — STL Export Contract & Mesh Validation** | Owner authorization of this detailed plan; P0, R2 CAD gate and ADR bounds already satisfied. | CAD-facing derived export-mesh contract, quality controls, validator, fixture/negative oracles and technical choices above. No Export UI. |
| **C2 — STL Export Implementation & UX** | C1 contract/validator GREEN. | Transactional STL writing, independent parse and a discoverable File / Export workflow informed by UX-REF-04. |
| **C3 — Windows Portable Release Distribution** | C2 executable and export UX stable. | Versioned Release portable package, isolated Windows 10 test and user-location/compatibility checks. |
| **C4 — Mechanical Workflow & N2 Acceptance** | C1–C3 accepted. | Full owner Technical, Experience and independent slicer/N2 gates for the plate. |
| **V1.2.FREEZE** | Every required gate PASS, no blocker. | Documentary/release freeze with exact N2 claim and limitations. |

Entry evidence classification: **satisfied** — P0 product authorization,
SPEC-01 traceability, frozen v1.1.0/SET-01, 71 accepted ADRs, R2 repaired
topology, CAD fixture metrics and owner CAD Experience Gate. **Partial** —
derived display tessellation exists, but its suitability for export is
unverified. **Pending by design** — owner approval of C1–C4, C1 mesh contract,
C2 writer/UX, C3 package and C4 independent N2 evidence. **Non-blocking** —
future kernel K1/K2/K3, stacked Cut, UX-01/UX-02, general topology naming,
CAM and broader interchange. No further generic prerequisite review is needed
before owner approval of C1.

## 11. Risk Register, Open Decisions and STOP Criteria

| Risk | Severity | C1–C4 mitigation / evidence |
|---|---|---|
| Display tessellation reused as if export-ready; open seams or missing cap material | **MAJOR** | C1 builds/tests a CAD-facing export mesh and validates seam incidence, closed components, winding and fixture mesh volume/AABB independently of display. Reuse only after proof. |
| Unitless STL silently interpreted at wrong scale; false “ready” status | **MAJOR** | Explicit mm UI/guide, validator-driven status, independent parser and slicer size/orientation check. |
| Package runs only in development tree | **MAJOR** | C3 Release portable inventory and isolated Win10 smoke including Save/Open/Export/preferences. |
| Multiple bodies or overlaps produce ambiguous output | **MEDIUM** | C1 chooses explicit body policy; reject unsupported ambiguity with structured diagnostic. |
| Export UI hides selection, quality or failures | **MEDIUM** | C2 guided flow and owner Experience Gate based on UX-REF-04, without fictitious validation states. |
| Cut→Cut limitation | **FUTURE GAP, NOT A FIXTURE BLOCKER** | Keep the one-Cut fixture; address stacked cuts only in a separately authorized campaign. |

Open **implementation decisions for C1** are format, quality/tolerance profile,
normal policy, multi-body selection, invalid/empty handling, atomic write,
independent parser, and measurable mesh-vs-CAD volume/bounds limits. They are
not grounds for a generic pre-C1 review; C1 records fixture evidence before
fixing thresholds. K1/K2/K3 remains an open future **architecture** decision,
not an implicit approval for an external kernel.

**HARD STOP only** for a material accepted-ADR change, unauthorized external
dependency, geometry the internal kernel cannot represent for this fixture,
CAD/Rendering ownership inversion, document-integrity violation, structural
incompatibility with first N2, or a real architecture/security blocker. Report
the exact evidence, affected ADR, impact and smallest correction before such a
change. Reversible UI or implementation choices are C1/C2 work, not STOPs.

## 12. Final Recommendation

**READY FOR OWNER APPROVAL.** Approve the bounded sequential C1–C4 execution
plan and authorize C1 within its stated boundary. The prior R1 geometry issue
is closed by R2; no further generic planning gate is needed. C1's exit
evidence, rather than this review, decides whether export-quality tessellation
is adequate to proceed to C2. Until explicit owner authorization, C1–C4 remain
**PROPOSED / NOT AUTHORIZED FOR IMPLEMENTATION**. No code, STL, package, CAM,
scripting or UX-01 implementation is performed in V1.2-READY.
