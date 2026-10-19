# Personal SolidWorks — v1.2+ Roadmap Proposal

**Status: PROPOSED — NOT APPROVED.** P0 review at frozen `v1.1.0` (`774f55c`), 2026-10-19. No increment, dependency, ADR or release after v1.1.0 is authorized by this document. Evidence: [product/architecture review](PERSONAL_SOLIDWORKS_PRODUCT_ARCHITECTURE_REVIEW.md), [capability matrix](PERSONAL_SOLIDWORKS_CAPABILITY_MATRIX.md), [owner decision package](PERSONAL_SOLIDWORKS_P0_DECISION_PACKAGE.md).

## Alternatives

| | R1 — Feature-first | R2 — Workflow-first |
|---|---|---|
| Method | Add Revolve, Hole, Fillet, Pattern, etc. in sequence. | Pick one mechanical job and close creation → edit → save/reopen → export/use. |
| Near-term value | Broad menu coverage, uncertain usable output. | Narrower geometry but measurable N2 replacement and shorter expected time to *first* usable job. |
| Main risk | Geometric scope outruns kernel, interoperability and reliability; features remain demonstrations. | Chosen job may be too narrow; explicit follow-on jobs are needed. |
| Architecture exposure | General operations force kernel decisions early and together. | Existing axial slice can be tested first; external-kernel decision remains a controlled gate. |

**Recommendation: R2**, subject to owner approval. This is a sequencing judgment, not a measured estimate. Reassess after the first end-to-end fixture and an isolated kernel spike.

## Proposed sequence and gates

| Milestone | Bounded deliverable and dependencies | Acceptance gate / deferred |
|---|---|---|
| **v1.2 — Practical Printed Part (proposed)** | One adapter/fixture plate within supported Sketch → Extrude → axial Cut. Add only workflow-critical editing/measurement, STL export from validated derived mesh, document/recovery hardening and Windows Release distribution. Depends on an agreed fixture, units, mesh tolerance and distribution choice. | Automated analytic volume/topology and watertight-mesh/roundtrip checks; Save/Open/Undo/Redo; slicer import and actual user print/use; clean-machine release smoke. N2 only for this part class. Defer general Boolean, Revolve, STEP, drawings, assemblies. |
| **Kernel decision spike (separate, timing before broad geometry)** | Isolated OCCT feasibility versus current kernel using plate + oblique cut + fillet + STEP fixture; test representation, face mapping, migration and Windows packaging. | Architectural Decision Gate; pass/fail evidence and rollback. No change to v1.1.0 or production dependencies during exploratory spike. |
| **v1.3 — Next Interoperable Part Workflow (proposed)** | Select *one* owner-priority job: CNC-cut profile via bounded DXF, or exact CAD handoff via STEP plus broader modeling. Scope follows kernel decision; do not promise both. | Validate output in independent target tool and dimensional/geometric equivalence; document and UX gates. Defer unrelated feature breadth. |
| **v1.4 — Drawing-ready Single Part (conditional)** | Projected views, dimension/tolerance contract and printable technical drawing for a supported part class. Needs stable model views and drawing-document ownership. | Independent drawing review against source CAD; update/rebuild consistency; print/export quality. Only if owner needs drawings. |
| **Later: assembly foundation / personal CAD workstation (conditional)** | Part documents, component instances, placements, mates, interference and BOM, then richer modeling. | Separate architecture and Experience Gates; no release label or date committed here. |

The proposed first N2 is a **3D-printed prismatic adapter plate with a circular through-hole and shallow blind pocket**. Its geometry deliberately fits the current axial subtraction contract. The user must be able to dimension/edit it, rebuild, Save/Open, export STL at known units, inspect in a slicer and produce/use the physical part without SolidWorks. STL is not exact CAD exchange. If the owner instead prioritizes CNC or supplier STEP handoff, v1.2 scope and gate must change before approval.

## Campaign Definition of Done

For each approved future campaign: observable functional capability; authoritative CAD correctness and failure diagnostics; document/Undo/Redo compatibility; discoverable mouse-first workflow; targeted automated validation plus appropriate integration/baseline validation; a real-use Experience Gate. Include export/interoperability only when required for that campaign's job, with an independent receiver check. Record deferred shapes and negative cases; no N2 label when a required handoff is absent.

## Dependencies and decision points

1. Owner chooses target users, job, handoff and measurable success criteria before v1.2 scope.
2. Any new external geometric dependency or change to accepted internal-kernel ADRs needs a specific Decision Gate and licensing/distribution review; [`DEPENDENCY_POLICY.md`](DEPENDENCY_POLICY.md) remains in force.
3. Persistent generated-face naming, feature-history reorder/suppression and exact-exchange document semantics need separate gates if chosen work actually depends on them.
4. A Windows Release/packaging gate must cover runtime DLLs, file-version compatibility, failed-save behavior, logs/crashes, rollback and clean-machine smoke.

Risks: tessellation defects can create unprintable STL despite valid CAD; axial-cut edge cases may exceed the proposed fixture; OCCT migration can invalidate face-dependent edits and file compatibility; schedule/effort is unknown until fixtures/spike are measured. None of the above is an approved implementation plan.
