# V1.2-PLAN-R1 — Reference Geometry Preflight

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
