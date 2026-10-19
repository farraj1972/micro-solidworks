# V1.2-PLAN-R1 — Reference Geometry Preflight

**R2 corrective result (2026-10-20): PASS WITH LIMITATIONS / EXPERIENCE PENDING.** The R1 failure below is preserved as historical evidence. The exact reference CAD chain, edits, history and `.microsw` v2 roundtrip now pass permanent automated tests. A simple application smoke was confirmed by the owner; the complete interactive N2 Experience Gate remains pending. This correction does not authorize V1.2-C1–C4.

**Result: FIX REQUIRED.** Evidence cut: `master` after SPEC-01.FREEZE (`fec2c9e`), 2026-10-19. The frozen `v1.1.0` executable source and 69 accepted ADRs were not changed. This is a domain/API and targeted automated preflight, **not** an interactive Experience Gate or authorization of V1.2-C1–C4.

## Objective, fixture and reference frames

Test whether the existing CAD stack can make one printable adapter plate using the proposed first-N2 sequence: XY sketch with an 80 × 50 outer rectangle and Ø8 inner circle; Extrude +8; Sketch on the Extrude EndCap; 18 × 14 rectangular pocket; reverse axial Cut Extrude 2; rebuild. No chained Cut is used.

Coordinates are millimetres in the model's local XY frame. The base `SketchPlane` origin is `(0,0,0)`, X `(1,0,0)`, Y `(0,1,0)`, normal +Z. Outer rectangle corners are `(-40,-25)` and `(40,25)`. Hole centre is `(-22,0)`, radius 4. Additive Extrude runs from Z=0 to Z=8. The EndCap sketch retains +X/+Y axes and origin Z=8. Pocket corners are `(7,-7)` and `(25,7)` (centre `(16,0)`); reverse Cut points -Z and, at depth 2, would remove material over Z=[6,8]. The two openings do not overlap. No frozen coordinate contract conflicts with these choices were found.

## Analytical oracles — expected, not observed

| Stage | Analytic volume (mm³) | Expected AABB (mm) |
|---|---:|---|
| Initial finished part | `80×50×8 − π×4²×8 − 18×14×2 = 31,093.8761403405` | X `[-40,40]`, Y `[-25,25]`, Z `[0,8]` |
| Width 90, pocket depth still 2 | `90×50×8 − π×4²×8 − 18×14×2 = 35,093.8761403405` | X `[-45,45]`, Y `[-25,25]`, Z `[0,8]` |
| Width 90, pocket depth 3 | `90×50×8 − π×4²×8 − 18×14×3 = 34,841.8761403405` | X `[-45,45]`, Y `[-25,25]`, Z `[0,8]` |

The request's rounded `31,093.876110` and `34,841.876110` differ by about `0.00003034` mm³ from the stated formula; the formula and values above are the reproducible oracle. On widening, the plate centre remains the origin, while hole `(-22,0)` and pocket `(16,0)` stay fixed. A proposed future acceptance tolerance is ≤0.01 mm on bounds and ≤1 mm³ on independently measured CAD volume, subject to an actual CAD-volume measurement contract; triangulated display volume must be reported separately with a tessellation tolerance. **No final CAD volume or final AABB was observed in this preflight.**

## Procedure and measured results

An ephemeral GoogleTest harness used only existing public `Sketch`, `extractPlanarRegions`, `FeatureModel`, `subtractAxialRegions`, `PersistenceSession` and topology APIs in the incremental `micro_solidworks_tests` target. Its reference test was intentionally failing and was removed after evidence capture; no productive or permanent test code was committed. Direct test command: `build/Debug/micro_solidworks_tests.exe --gtest_filter=V12GeometryPreflight.*`.

| Check | Observed result |
|---|---|
| Base sketch profile | `extractPlanarRegions`: 1 valid region, 1 inner boundary. |
| Base Extrude | Rebuild `Success`; 1 `Solid`, `isValid() == true`, 8 faces, 12 vertices, two cylindrical support faces. EndCap semantic plane at Z=8. Vertex AABB X `[-40,40]`, Y `[-25,25]`, Z `[0,8]`. The through-hole is represented by the inner boundary and cylinder; its full void was not independently volume-measured. |
| Reference rectangular pocket Cut | `FeatureModel::rebuild`: `Failed`, diagnostic `Cut Extrude failed: Incident Faces must traverse a shared Edge oppositely`. No final Cut Solid or pocket-floor provenance was produced. |
| Isolation: no-hole rectangle + same rectangular pocket | Direct `subtractAxialRegions`: `Success`, one Solid. |
| Isolation: holed rectangle + rectangular pocket | Direct `subtractAxialRegions`: `TopologyFailure`, zero solids, same shared-edge-orientation diagnostic. |
| Isolation: holed rectangle + circular pocket | Same `TopologyFailure`, zero solids, same diagnostic. |

The error is raised by shell edge-incidence validation in [`Solid::addShell`](../src/core/topology/Solid.cpp) during blind-cut reconstruction in [`AxialSubtraction`](../src/features/AxialSubtraction.cpp). This is a reproducible failure of the supported one-Cut sequence on a holed base. The precise orientation defect within partition/reconstruction remains **NOT VERIFIED**; no kernel correction was made.

## Parametric, history and persistence state

Width 80→90, pocket depth 2→3, their Undo/Redo, deterministic rebuild of the **finished** part, Save/Close/Open/Rebuild of that part, final volume, pocket floor/walls, outward face orientation and absence of unintended interior surfaces are **NOT VERIFIED**: the initial Cut fails before those steps. The base extrusion AABB and semantic EndCap are observed, but do not prove the completed feature chain. Existing independent contracts for edit/rebuild, history and persistence passed in targeted tests below; they are not a substitute for this fixture's roundtrip.

## Negative cases and targeted validation

The ephemeral harness observed `RegionFailure` / `Invalid axial subtraction input` for an open cutter profile and for zero cut depth. An unavailable semantic face role throws `The sketch support face is unavailable after rebuild.` A second Cut targeting a Cut is accepted as a history entry but rebuild fails with `This Cut Extrude target cannot be reconstructed as a supported axial extrusion`; this limitation remains unchanged. Existing `FeatureModel.FailedClosurePreservesAllLastSuccessfulCachesAndRecovers`, `FeatureModel.CutFailurePreservesLastKnownGoodAndRecovers` and `FeatureModel.DisappearingPocketFloorFailsSketchWithoutReattachment` passed, covering failed dependencies, last-known-good cache and disappearance of a support face. A depth greater than plate thickness was not classified as invalid: it can be a valid through-cut; other impossible-depth geometries are **NOT VERIFIED**.

Validation level: **V2 targeted integration** because the fixture crosses Sketch/Profile, Extrude, topology, Cut and FeatureModel. The existing incremental Debug test target built. The ephemeral fixture test **failed 1/2** as above (isolation/negative-contract test passed). Separately, **21/21 targeted existing tests passed** across `GeneralExtrusion.*`, `AxialSubtraction.*`, selected `FeatureModel` edit/rebuild/semantic-face/failed-closure cases, two persistence v2 cases and one `PersistenceSessionHistory` Cut/Undo/Redo case. Full CTest/V3, Geometry-independent historical suites, Viewer/OpenGL runtime and clean configure/build were intentionally omitted: no production code or build graph changed. No interactive UI run or owner observation was made; Experience Gate is **PENDING OWNER VALIDATION after a corrected CAD candidate exists**.

## Readiness and risks

**V1.2-C1 entry is not ready for this N2 fixture.** The minimum correction is a separately authorized, regression-tested repair of blind axial subtraction's shell orientation/partition for a target with an inner circular void, preserving existing topology and ownership contracts. First reproduce with the exact coordinates above, then verify valid closed outward shell, exact hole/pocket topology, CAD volume and AABB, width/depth edit, Undo/Redo, deterministic rebuild and `.microsw` v2 Save/Open; only then seek owner review of C1–C4. An alternative is an owner-approved fixture/scope revision with equivalent N2 user value. Do not silently remove the through-hole, change the sequence to chained Cuts, claim manual success, or begin STL export/Windows packaging to bypass this failure.

The final K1/K2/K3 kernel choice remains OPEN; this failure alone does not authorize a new dependency or ADR change. Export-quality tessellation, mesh watertightness, slicer scale, Windows portable distribution and human workflow remain future gates, not evidence from this preflight.

## R2 — Corrective geometry closure (2026-10-20)

The permanent `AxialSubtraction.PreservesThroughHoleDuringIndependentBlindPockets` reproduction failed before the repair with `Incident Faces must traverse a shared Edge oppositely`. Instrumentation localized the invalid shared edge to edge 14, from `(-18,0,6)` to `(-26,0,6)`, on the existing through-hole at the Z=6 axial slab interface. Its incident cylindrical faces 5 and 16, with outer wires 6 and 17, both recorded `Reversed` traversal. The error was not a duplicate pocket floor or a failure of the manifold validator. In `PlanarRegionBoolean`, a reversed circular inner boundary lost its direction when split because `Circle2` itself carries no orientation. The resulting lower and upper cylindrical segments then traversed their shared edge in the same direction.

The correction splits exact region curves first, then reverses both the sequence and direction of pieces for a reversed boundary. `flatten`, `sample` and `partitionPlanarRegions` now use the same directed pieces. Exact CAD curves, topology validation and the CAD/render separation remain intact. A second, derived-display defect appeared after the width edit: bridging two top-cap holes in insertion order omitted much of the cap triangulation. `SolidDisplay` now bridges projected holes in deterministic increasing rightmost-X order. This affects visualization only; the CAD Solid remained valid. The permanent display regression checks cap and pocket-floor triangle areas for both widths. View tessellation has not been qualified for STL export.

| R2 check | Observed automated result |
|---|---|
| Base Extrude and Cut | Complete `FeatureModel::rebuild` succeeds; one valid closed Solid retains the analytic through-hole and blind pocket. EndCap and PocketFloor semantic faces resolve. |
| Topology | Every edge has exactly two incident faces with opposite traversal; cylindrical through-hole faces and planar faces with inner wires remain present. No material-material internal face at the pocket floor. Existing no-hole/pocket, circular-pocket, split and multibody regressions pass. |
| Initial CAD volume | Independent shell integration over exact horizontal planar wire curves: `31,093.876140` mm³ observed, versus `31,093.8761403405` mm³ analytic; assertion tolerance `10⁻⁶` mm³. |
| Initial CAD AABB | X `[-40,40]`, Y `[-25,25]`, Z `[0,8]`, matching oracle. |
| Width 80→90, depth 2 | `35,093.876140` mm³; X `[-45,45]`, Y `[-25,25]`, Z `[0,8]`. Hole and pocket keep their fixed local centres. |
| Width 90, depth 2→3 | `34,841.876140` mm³ observed, versus `34,841.8761403405` mm³ analytic; same edited AABB, floor Z=5. |
| History and persistence | Width/depth edits, Undo/Redo, `.microsw` v2 Save/Open and deterministic rebuild preserve geometry, feature dependencies and semantic support. |

The volume measurement uses the authoritative topology's analytic segments and circular arcs, not display triangles. The `10⁻⁶` mm³ assertion tolerance is much tighter than the proposed 1 mm³ product tolerance and accommodates floating-point integration. The permanent tests are `AxialSubtraction.PreservesThroughHoleDuringIndependentBlindPockets`, `PersistenceSessionHistory.HoledReferencePlateSurvivesEditsHistoryAndVersionTwoRoundtrip` and `SolidDisplay.MultiHoleTopCapKeepsFullAreaAfterPlateWidthEdit`.

Validation used incremental Debug builds of `micro_solidworks_tests` and `micro_solidworks`. Focused V1/V2 regression across planar regions, axial subtraction, general extrusion, FeatureModel, persistence/session history and SolidDisplay passed **78/78**. Full CTest/V3, unrelated historical suites and clean configure/build were intentionally omitted under impact-based validation. The owner confirmed the directed application smoke passed and closed normally after native capture timed out twice. The complete N2 interaction/Experience Gate was **not** performed, and STL/export mesh quality, slicer and packaging remain unverified. P0, SPEC-01, `v1.1.0`, all 69 accepted ADRs and the open K1/K2/K3 kernel decision are unchanged. The technical geometry blocker to proposed C1 is corrected; owner review remains required, and V1.2-C1–C4 remain unauthorized.
