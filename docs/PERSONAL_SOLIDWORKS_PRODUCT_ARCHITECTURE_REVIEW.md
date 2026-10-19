# P0 — Personal SolidWorks: Product & Architecture Review

Status: **REVIEW READY, not an implementation authorization**. Evidence cut: Micro SolidWorks `v1.1.0`, commit `774f55c` (2026-10-19); 69/69 accepted ADRs and the already completed 903/903 C5 gate are recorded in the [release freeze](V1_1_0_RELEASE.md). This V0 review did not rerun tests. The [capability matrix](PERSONAL_SOLIDWORKS_CAPABILITY_MATRIX.md), [roadmap proposal](PERSONAL_SOLIDWORKS_ROADMAP_PROPOSAL.md) and [decision package](PERSONAL_SOLIDWORKS_P0_DECISION_PACKAGE.md) carry the detail and pending owner choices.

## 1. Executive Summary

The current application can complete a bounded parametric modeling chain through Sketch, constrained edits, general additive Extrude, axial Cut Extrude, rebuild, Undo/Redo and `.microsw` Save/Open. Its result is still **N1 (assisted workflow)** for a manufactured part because it has no validated export or drawing handoff. The shortest credible N2 target is a single printed prismatic adapter plate whose geometry stays within the supported axial kernel, with verified watertight STL export and a usable Windows distribution. The target and format require owner approval.

The kernel is genuinely useful for planar regions with arcs/circles and planar/cylindrical extrusion faces. It is not a general mechanical B-rep engine: oblique cuts, general Booleans, fillets, revolve, persistent topological naming and exact CAD exchange are absent. Preserve it for a first bounded workflow while running a future isolated external-kernel spike. No dependency or architectural change is approved by this review.

## 2. Strategic Product Vision

**APPROVED vision:** a personal parametric 3D CAD application that progressively replaces specific real mechanical-engineering workflows without an expensive professional license. Product utility, completed workflows, pragmatic build/integrate choices, progressive replacement and CAD data integrity guide future proposals. “Personal SolidWorks” is a working vision, not a branding change or a claim of SolidWorks equivalence. The repository, technical product name, history, frozen baselines, dependency policy and ADRs remain unchanged. A distinct future product name merits trademark review before distribution; no legal conclusion is made here.

The proposed first audience is an individual designing small mechanical replacement/adaptation parts for 3D printing. CNC machinists, drawing-dependent manufacturing and multi-part assemblies are future candidate segments, not the initial N2 claim.

## 3. Current v1.1.0 Capability Inventory

The code-backed [matrix](PERSONAL_SOLIDWORKS_CAPABILITY_MATRIX.md) covers Application/session, UI, interaction, viewer, picking, math, geometry, topology, modeling, sketcher, constraints, feature history, rebuild, persistence, rendering, logging, build/testing and distribution. Authoritative state is [`FeatureModel`](../src/features/FeatureModel.h); [`Persistence`](../src/persistence/Persistence.h) writes the v2 aggregate and reads v1/v2; [`SolidDisplay`](../src/presentation/SolidDisplay.h) derives render triangles from topology. [`EditorContext`](../src/interaction/EditorContext.h) owns transient tools and logical selection. The C5 [Experience Gate](V1_1_C5_VALIDATION.md) establishes the bounded editing UX, not product-wide usability.

[`Sketch`](../src/core/sketch/Sketch.h) holds lines, circles and arcs; [`Intersection2`](../src/core/geometry/Intersection2.h) covers the six supported segment/circle/arc pairs. [`Solid`](../src/core/topology/Solid.h) has oriented vertices/edges/wires/faces/shells, inner wires and Plane/Cylinder face supports. Multiple bodies are a `vector<Solid>` at the feature-cache level, not multiple shells under one Solid. `isValid()` checks the implemented topology contract; it is not proof that every arbitrary CAD surface configuration is sound. Face IDs are regenerated during rebuild; [`SemanticFaceProvenance`](../src/features/FeatureModel.h) supplies restricted cap-role references rather than universal topological naming.

## 4. Reference Mechanical Workflows

Autonomy scale: **N0** demonstration; **N1** assisted workflow; **N2** functional replacement for a delimited job from modeling through required handoff; **N3** preferred personal workflow. The classifications below are *current v1.1.0 assessments*, not claims that reference parts were built or physically tested.

| Proposed reference part / required handoff | Current level | First blocking step |
|---|---|---|
| Prismatic adapter/plate with axial holes and pockets → 3D print | N1 | No STL export, mesh-quality gate or distributable release; restricted cuts also require fixture validation. |
| Similar plate → CNC/drawing | N1 | No DXF/STEP or dimensioned drawing. |
| Bracket with rib, cuts and blends | N0–N1 | Joining/Boolean shape scope and fillet/chamfer; exact feasibility depends on geometry. |
| Stepped shaft with keyway | N0 | Revolve missing; groove and fillet also missing. |
| Flanged bushing | N0 | Revolve and feature-level hole/fillet absent. |
| Bearing mount / coupling | N0–N1 | General solid operations, patterns and specified fit checks absent. |
| Small bolted assembly | N0 | Component instance, mates, interference and BOM absent. |

For the prism chain `New → Sketch → Constraints/Dimensions → Extrude → Sketch on Face → Cut → Edit → Rebuild → Save/Open → Export`, the first missing *universal* step is Export. Individual complex shapes may fail earlier at axial-cut/topology restrictions. The revolution chain breaks at Revolve. The manufacturing chain breaks at drawing/export; STL is a mesh, STEP is exact CAD exchange, DXF is 2D geometry/drawing exchange, CAM is separate. The assembly chain breaks at the part-instance model. These are not interchangeable outputs.

## 5. Product Gap Analysis

The [gap matrix](PERSONAL_SOLIDWORKS_CAPABILITY_MATRIX.md#workflow-gap-matrix) covers all requested operations and priorities. Proposed blocker severity reflects practical workflow impact, not measured implementation cost.

| Group | Severity | Blocking effect |
|---|---|---|
| Interoperability | CRITICAL | No STL/STEP/DXF handoff; modeled parts cannot enter an external production chain. |
| Reliability/distribution | CRITICAL | No supported installed/portable release, recovery policy or export quality gate; development success is not field readiness. |
| Modeling / geometry-topology | HIGH | Axial-only cut, restricted Boolean, no revolve/fillet/chamfer; many everyday mechanical shapes are unavailable. |
| Parametric history | HIGH | Generated-face references are narrow; no general naming, suppression or reorder for complex edits. |
| Drawings | HIGH for CNC; MEDIUM for first printed-part target | No drawing views, tolerances or dimensioned manufacturing sheet. |
| UX | MEDIUM | C5 improved editing/diagnostics, but measurement, clearer remaining tool flows and sustained long-session evidence are missing. |

## 6. Geometry Kernel Assessment

[`PlanarRegion`](../src/features/PlanarRegion.h) builds regions from supported lines/arcs/circles with outer loops and holes. [`GeneralExtrusion`](../src/features/GeneralExtrusion.h) produces one or more valid Solids for selected regions, preserving analytic planar/cylindrical topology; concave outlines and multiple regions are supported within validation rules. This is a stronger base than polygon-only extrusion.

[`AxialSubtraction`](../src/features/AxialSubtraction.cpp) accepts cutter direction aligned or opposed to the target extrusion axis. Its explicit failure branches reject oblique direction, wholly internal blind intervals and partial-depth removal of an entire target cross-section. Supported blind/reverse pockets, through holes and splits are not a general face/face intersection engine; multi-body output is possible as `vector<Solid>`, subject to its supported target/regional cases. [`modeling::booleanOperation`](../src/modeling/Boolean.h) is an older axis-aligned box/cell operation with union, difference and intersection status reporting; its connected, hole-free orthogonal result limit is separate from C4 axial subtraction. Calling both “general Boolean” would misstate the source.

The structural gap is the absence of arbitrary analytic surface intersection, robust tolerance/healing and face correspondence across modeling operations. Adding an isolated feature command cannot solve those. Missing commands such as Hole or Pattern can sometimes be built on the current kernel, but Revolve, fillets and broad STEP interoperability imply deeper geometry and identity work. Tessellation in [`SolidDisplay`](../src/presentation/SolidDisplay.cpp) creates derived mesh for view/pick; its triangle quality and watertightness have not been certified as export contracts.

## 7. Build vs Integrate Options

| Criterion | K1: internal kernel | K2: selective hybrid | K3: external B-rep under project boundary |
|---|---|---|---|
| Functional reach / robustness | Best for current planar/axial subset; general Boolean, blends, surface intersection and STEP require large new work. | A specialized mesher or algorithm can help a narrow gap; cross-kernel geometry operations risk conversion loss. | Potential broad analytic modeling and exchange; still needs proof on our fixtures, tolerances, face references and licensing. |
| Correctness and identity | One authoritative representation, but all algorithms ours. | Highest risk of two authoritative shapes and unstable conversion/topological mapping. | One external geometric authority could simplify operations, but CAD FeatureModel identity and generated-face naming still ours. |
| Effort / migration | Low initial cost, potentially very high cumulative geometric cost. | Medium per integration, high system-wide coherence burden. | High up-front adapter, document migration and distribution effort. |
| Learning / modularity | Strongest educational value; existing modular boundaries. | Retains selective learning; adapters must be explicit. | Preserves product-owned Application/UI/interaction/FeatureModel/persistence; lower algorithm-learning value. |
| Interoperability / licensing / maintenance | STEP work largely ours; no new license. | Package-specific licenses and version matrix. | OCCT offers candidate exchange stack; license, DLLs and long-term API versioning require review. |

**Recommendation for owner consideration:** use K1 only for the bounded first printed-part milestone; authorize a *separate*, isolated K3 feasibility spike before choosing the long-term kernel. Do not adopt K2 for arbitrary modeling operations without a single-authority representation and stable conversion/identity proof. This is not a decision to migrate or to add OCCT. The existing [dependency policy](DEPENDENCY_POLICY.md) remains binding.

Official candidate evidence (checked 2026-10-19; source pages may evolve): [OCCT overview](https://occt3d.com/dev/doc/overview/html/index.html), [OCCT repository/readme](https://github.com/Open-Cascade-SAS/OCCT/blob/master/README.md), [OCCT build guide](https://github.com/Open-Cascade-SAS/OCCT/blob/master/dox/build/build_occt/building_occt.md) describe a C++ B-rep/modeling/data-exchange platform, CMake/Windows build and LGPL-2.1 with special exception. The exact release, enabled toolkit set, transitive licenses, binary footprint and MSVC/C++20 build must be pinned/verified in a future spike; no version compatibility is claimed here. [CGAL packages](https://doc.cgal.org/6.1/Manual/packages.html), [license](https://doc.cgal.org/6.1/Manual/license.html) and [Windows/CMake guide](https://doc.cgal.org/6.1.2/Manual/general_intro.html) show computational geometry packages with package-specific GPL/LGPL or commercial terms. CGAL is not a drop-in feature/history B-rep CAD kernel or STEP stack. [earcut.hpp](https://github.com/mapbox/earcut.hpp) is a specialized 2D polygon-with-holes tessellator with [ISC license](https://github.com/mapbox/earcut.hpp/blob/master/LICENSE) and CMake/MSVC support; its own documentation explicitly does not guarantee correctness for arbitrary invalid polygons. It cannot replace CAD topology or validate an STL by itself. No candidate was built or integrated in P0.

Proposed future spike, subject to a kernel Decision Gate: preserve `v1.1.0` untouched; create an isolated branch/prototype with a project-owned geometry adapter. Roundtrip a prismatic plate with circular through-hole and blind pocket; test a nonaxial cut, fillet and STEP export/import using one chosen OCCT release. Compare volume, face continuity, manifoldness, edit/rebuild face mapping, file size, Windows release deployment and v1/v2 document reconstruction. Reject if geometry varies outside agreed tolerances, semantic references cannot be reconciled, license/dependency review fails or build/runtime packaging is impractical. Roll back by discarding the isolated prototype; no document-format rewrite or accepted-ADR change in the experiment.

## 8. CAD Interoperability Strategy

**STL first for the proposed printed-part N2:** tessellate the *authoritative* Solid, validate units, orientation, watertightness, nondegenerate triangles and tolerance versus analytic boundaries, then write mesh. STL loses curves, parameters, units metadata conventions and editability; it is a manufacturing mesh handoff, not an exact CAD document. **STEP later:** exact B-rep exchange is high value for CNC/other CAD, but current topology/surface subset and lack of general healing/naming make a trustworthy broad writer/reader a major architectural effort; prioritize after kernel decision. **DXF:** bounded 2D sketch/profile export could support laser/CNC cutting; layer, arc, units, constraints-not-exported and drawing mapping need a contract. **IGES:** secondary legacy exchange; defer. **3MF:** evaluate after STL for richer additive-manufacturing metadata. No import path is currently delivered. Do not claim CAD interoperability from STL alone.

## 9. UX and Productivity Priorities

Keep Graphics Area, Command Bar, Feature Manager and contextual Property Manager from [`ApplicationShell`](../src/ui/ApplicationShell.cpp); retain direct picks and human names. The next bounded workflow needs explicit mode/selection guidance, dimension and distance/angle/radius measurement, visible export validation, clear operation failure recovery and one-click orientation/fit. Bounding box is a useful export preflight. Section view, mass properties, reference/construction geometry, trim/offset, patterns and mirror should enter only when a chosen fixture shows a concrete need. C5 editing success does not imply first-use discoverability or long-session productivity; measure those with a repeatable Experience Gate.

## 10. Reliability and Distribution

Product targets proposed for first N2: no lost valid document on failed edit/save; deterministic rebuild for persisted fixtures; actionable diagnostic for unsupported geometry; Undo/Redo across edits; successful reopen after save; watertight export with recorded tolerance and unit choice; no crash across a sustained fixture session. Add fixture-based topology/property checks and image comparisons only where they detect observable regressions. Existing [`Persistence`](../src/persistence/Persistence.cpp) uses bounded parse and temporary-file replacement, but that does not provide autosave, crash recovery or backup history. A future Windows Release gate needs reproducible Release build, dependency/DLL inventory, signing/packaging choice (portable zip or installer), logging/crash collection with privacy policy, user settings, document version/upgrade compatibility and clean-machine smoke. None is implemented or authorized by P0.

## 11. Proposed v1.2+ Roadmap

The [separate roadmap](PERSONAL_SOLIDWORKS_ROADMAP_PROPOSAL.md) compares R1 feature-first and R2 workflow-first and recommends R2. Indicative, unapproved ordering: v1.2 bounded printable prismatic part and STL/release quality; v1.3 kernel decision plus exact CAD interoperability or selected DXF workflow; v1.4 drawing-ready single-part workflow; later assemblies only after part/product foundations. Names, sequencing and scope require owner approval. A future campaign is done only when functional operation, authoritative CAD correctness, document lifecycle, usable interaction, automated evidence and real-use Experience Gate all pass, plus exchange/export when needed by its job.

## 12. First Functional Replacement Milestone

**Proposed N2:** one 3D-printable rectangular adapter plate, with a circular through-hole and one shallow blind pocket on a planar face, modeled from sketch dimensions, additive extrusion and supported axial cut. The user can change width/height/thickness, hole diameter and pocket depth, rebuild without silent corruption, Save/Open, export a watertight STL at explicit millimetre scale, inspect the result in an independent slicer and print/use the piece without opening SolidWorks. Acceptance needs automated fixtures for analytic volume/topology and mesh manifoldness; a real-user end-to-end Experience Gate must confirm the print handoff. This does **not** claim arbitrary pocket layouts, CAM, STEP, toleranced drawing, material certification or all 3D printers. Owner should confirm this target before v1.2 scope is frozen.

## 13. Architecture Decision Requirements

Keep ADR-0001–0069 accepted and unchanged. A kernel replacement would revisit the internal-kernel/dependency contract (notably [ADR-0006](adr/ADR-0006-educational-geometry-kernel.md) and later topology/modeling decisions), define which representation is authoritative, and consider D18/D20 face support and [`FeatureModel`](../src/features/FeatureModel.h) mapping. Exact STEP import/export would need a data-exchange and document-ownership gate (D12 persistence remains a distinct native format). General topological naming, document identity, suppression/reorder and assembly ownership warrant gates only when chosen work requires them. Propose amending dependency policy in a future, owner-approved decision to allow product-value `INTEGRATE` with license, maintenance, migration and distribution review; do not amend it here.

## 14. Risks and Open Questions

**CRITICAL:** absence of manufacturing handoff and deployable/recoverable product; **HIGH:** kernel coverage, generated-face stability and numeric robustness; **MEDIUM:** first-use UX, cross-version document migrations, mesh accuracy and licensing overhead. Complexity ratings are qualitative. Open decisions: Which user's jobs and machines define N2? Is the first handoff print STL or CNC DXF/STEP? What tolerance/unit/mesh quality is acceptable? Is an external kernel feasible with v1/v2 saved documents and face-based history? What field failure rate and distribution mode are acceptable? The [decision package](PERSONAL_SOLIDWORKS_P0_DECISION_PACKAGE.md) makes these explicit.

## 15. Recommendations

Approve a **workflow target**, not a long feature list. Retain the frozen release and current dependency policy. Test the first printed-part fixture against the current axial contract before committing to v1.2; if it falls outside the contract, revise the target or seek a separate Decision Gate. Gate STL and packaging on independent output validation. Run a kernel feasibility spike before promising broad STEP/revolve/fillet scope. Review the proposal with the owner; only then authorize increments and any new ADRs. P0 itself changes documentation only.
