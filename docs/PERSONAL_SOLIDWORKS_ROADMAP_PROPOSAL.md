# Personal SolidWorks — v1.2+ Roadmap Proposal

**Status: P0 FROZEN. R2 workflow-first APPROVED. v1.2 APPROVED AT CAMPAIGN LEVEL; implementation plan READY FOR REVIEW, NOT AUTHORIZED. v1.3+ PROPOSED — NOT APPROVED.** P0 evidence cut: frozen `v1.1.0` (`774f55c`), 2026-10-19. No increment, dependency, ADR or release after v1.1.0 is authorized by this document. See [approved decisions](PERSONAL_SOLIDWORKS_P0_DECISIONS.md), [v1.2 campaign proposal](PERSONAL_SOLIDWORKS_V1_2_CAMPAIGN_PROPOSAL.md), [review](PERSONAL_SOLIDWORKS_PRODUCT_ARCHITECTURE_REVIEW.md) and [capability matrix](PERSONAL_SOLIDWORKS_CAPABILITY_MATRIX.md).

## Alternatives

| | R1 — Feature-first | R2 — Workflow-first |
|---|---|---|
| Method | Add Revolve, Hole, Fillet, Pattern, etc. in sequence. | Pick one mechanical job and close creation → edit → save/reopen → export/use. |
| Near-term value | Broad menu coverage, uncertain usable output. | Narrower geometry but measurable N2 replacement and shorter expected time to *first* usable job. |
| Main risk | Geometric scope outruns kernel, interoperability and reliability; features remain demonstrations. | Chosen job may be too narrow; explicit follow-on jobs are needed. |
| Architecture exposure | General operations force kernel decisions early and together. | Existing axial slice can be tested first; external-kernel decision remains a controlled gate. |

**Owner decision: R2 APPROVED.** This is a strategic sequencing choice, not a measured time-to-value estimate. Reassess its results after the first end-to-end fixture. A future isolated kernel spike is separate and does not block first N2.

## Proposed sequence and gates

| Milestone | Bounded deliverable and dependencies | Acceptance gate / deferred |
|---|---|---|
| **v1.2 — First Functional Replacement: Printable Mechanical Part (campaign level approved; implementation pending)** | One adapter/fixture plate within supported Sketch → Extrude → axial Cut. Plan STL export and validation, portable Windows Release distribution, reference workflow, guidance and Experience Gate. Depends on a reviewed fixture, units and mesh contract. | Analytic volume/topology and watertight-mesh/roundtrip checks; Save/Open/Undo/Redo; independent slicer acceptance and correct scale; clean-machine release smoke. A physical print is supplementary, not mandatory. N2 only for this part class. Defer general Boolean, Revolve, STEP, drawings, assemblies. |
| **Kernel decision spike (separate, timing before broad geometry)** | Isolated OCCT feasibility versus current kernel using plate + oblique cut + fillet + STEP fixture; test representation, face mapping, migration and Windows packaging. | Architectural Decision Gate; pass/fail evidence and rollback. No change to v1.1.0 or production dependencies during exploratory spike. |
| **v1.3 — Next Interoperable Part Workflow (proposed)** | Select *one* owner-priority job: CNC-cut profile via bounded DXF, or exact CAD handoff via STEP plus broader modeling. Scope follows kernel decision; do not promise both. | Validate output in independent target tool and dimensional/geometric equivalence; document and UX gates. Defer unrelated feature breadth. |
| **v1.4 — Drawing-ready Single Part (conditional)** | Projected views, dimension/tolerance contract and printable technical drawing for a supported part class. Needs stable model views and drawing-document ownership. | Independent drawing review against source CAD; update/rebuild consistency; print/export quality. Only if owner needs drawings. |
| **Later: assembly foundation / personal CAD workstation (conditional)** | Part documents, component instances, placements, mates, interference and BOM, then richer modeling. | Separate architecture and Experience Gates; no release label or date committed here. |

The approved first N2 target is a **printable prismatic adapter plate with a circular axial through-hole and shallow axial pocket**. Its geometry is intended to fit the current axial subtraction contract, subject to fixture validation. The user must dimension/edit it, rebuild, Save/Open, export STL under a known units convention, inspect it in a slicer and prepare printing from the supported package without SolidWorks. STL is not exact CAD exchange. Physical fabrication is optional additional confidence, not a gate requirement.

## Campaign Definition of Done

For each approved future campaign: observable functional capability; authoritative CAD correctness and failure diagnostics; document/Undo/Redo compatibility; discoverable mouse-first workflow; targeted automated validation plus appropriate integration/baseline validation; a real-use Experience Gate. Include export/interoperability only when required for that campaign's job, with an independent receiver check. Record deferred shapes and negative cases; no N2 label when a required handoff is absent.

## Dependencies and decision points

1. The owner has chosen the bounded printed-part workflow and STL handoff. Approve the concrete fixture, units/tolerances and increment plan before v1.2 implementation.
2. Any new external geometric dependency or change to accepted internal-kernel ADRs needs a specific Decision Gate and licensing/distribution review; [`DEPENDENCY_POLICY.md`](DEPENDENCY_POLICY.md) remains in force.
3. Persistent generated-face naming, feature-history reorder/suppression and exact-exchange document semantics need separate gates if chosen work actually depends on them.
4. A Windows Release/packaging gate must cover runtime DLLs, file-version compatibility, failed-save behavior, logs/crashes, rollback and clean-machine smoke.

Risks: tessellation defects can create unusable STL despite valid CAD; axial-cut edge cases may exceed the proposed fixture; a future OCCT migration can invalidate face-dependent edits and file compatibility; schedule/effort is unknown until fixtures/spike are measured. Only the v1.2 campaign-level objective and R2 strategy are approved; detailed implementation remains unapproved.
