# V1.2-C4 — N2 printable adapter plate acceptance

Status: **PENDING EVIDENCE**. This is an executable owner checklist, not an
Experience Gate or N2 approval. C1 export-mesh tests and C2 binary-STL parser
tests pass; C2 UI observation, C3 isolated Windows workflow and independent
slicer evidence are still required. Do not use this document to infer a release
freeze.

## Candidate and fixture

Use the [candidate portable ZIP](PERSONAL_SOLIDWORKS_V1_2_C3_VALIDATION.md)
from `out/packages/PersonalSolidWorks-v1.2.0-candidate-win64.zip`. Record the
archive SHA-256, Windows version/build, graphics/OpenGL capability and whether
the environment has development tools. Extract outside the repository and
start `micro_solidworks.exe` from the extracted directory. Record startup and
normal exit separately.

The reference part is in **mm**: an XY rectangle from `(-40,-25)` to
`(40,25)`, an inner circle centred at `(-22,0)` with radius `4`, additive
Extrude distance `8`, then a rectangle on the Extrude **EndCap** from
`(7,-7)` to `(25,7)` and a reverse Cut Extrude of depth `2`. The resulting
through-hole is Ø8 and the blind pocket is 18 × 14 × 2. The cut sketch must
attach to the EndCap of the Extrude, not the pocket floor or a historical
solid. The hole is created in the base profile; this avoids an unsupported
Cut→Cut chain.

## Owner workflow

1. File → New; choose the XY reference plane and start a Sketch. Use
   Rectangle and Circle to draw the outer and inner contours. Select the
   relevant sketch geometry and apply driving dimensions/constraints so the
   bounds and Ø8 hole match the coordinates above. Observe that each applied
   dimension changes the sketch immediately. Rebuild the sketch and finish it.
2. Select that Sketch and invoke Extrude. Set distance to `8` and Confirm.
   Check the top and bottom, through-hole, and one resulting body. Select the
   upper **EndCap** and start Sketch on this face.
3. Draw the pocket rectangle at the coordinates above; dimension it 18 × 14.
   Finish the Sketch, invoke Cut Extrude, choose reverse direction `-Z`, set
   depth `2` and Confirm. Check that the pocket is blind and the through-hole
   remains open. Run Rebuild and inspect diagnostics.
4. Edit the base rectangle width from `80` to `90` while keeping it centred:
   X extents become `[-45,45]`; hole centre and pocket centre remain fixed.
   Edit the Cut Extrude depth from `2` to `3`. Rebuild. Exercise Undo then
   Redo on the edits and verify geometry/dimensions follow the history.
5. File → Save As a `.microsw` document; close it and reopen it with File →
   Open. Rebuild. Confirm the final dimensions, hole and pocket remain. Save
   again if needed. No technical feature IDs should be needed in normal use.
6. File → Export STL... on the current result. Confirm the chosen body,
   **mm** convention and Standard/Fine quality. Click **Validate mesh** and
   inspect the result; choose a destination with Browse, then Export. Verify
   the reported success and generated file. Repeat with Cancel and check
   document dirty state, Undo/Redo and previous STL are unchanged. A deliberately
   invalid destination should report a comprehensible error without damaging
   either file.
7. Open the exported STL in an **independent slicer**. Set/confirm mm because
   STL contains no reliable unit declaration. Inspect X/Y/Z extents,
   orientation, through-hole and blind pocket, then prepare a print. Record
   slicer warnings and the prepare/slice result. Physical printing is optional.

For the initial CAD solid the exact volume is `31,093.8761403405 mm³`, with
AABB X `[-40,40]`, Y `[-25,25]`, Z `[0,8]`. After widening, volume is
`35,093.8761403405 mm³` at pocket depth 2; after the final depth edit it is
`34,841.8761403405 mm³`, AABB X `[-45,45]`, Y `[-25,25]`, Z `[0,8]`.
These are **analytic CAD** oracles. The triangulated STL volume differs by
tessellation and uses the [C1 export tolerance](PERSONAL_SOLIDWORKS_V1_2_C1_VALIDATION.md),
not exact equality. A slicer may only display dimensions; do not claim it
measured volume if it did not.

## Evidence record to complete

| Gate | Required observation | Current evidence |
| --- | --- | --- |
| CAD workflow and edits | All steps 1–4, including rebuild and Undo/Redo | R2 automated and owner CAD evidence; integrated portable run pending |
| Native persistence | Save, close, reopen, rebuild final part | R2 automated; integrated portable run pending |
| Export mesh and file | Validation result, STL bytes, AABB/volume, no mutation | C1/C2 automated PASS; UI observation pending |
| Independent slicer | Name/version, OS, STL file/hash, dimensions, hole, pocket, orientation, print preparation, warnings | **PENDING** |
| Windows portable | OS build, no developer runtime dependency, startup, Save/Open/Export, preferences location, diagnostics, exit code | Release ZIP and out-of-tree startup only; **PENDING** |
| Experience Gate | Owner decision: APPROVED / APPROVED WITH CHANGES / REWORK REQUIRED, with findings | **PENDING** |

N2 decision: **PENDING EVIDENCE**. Only the bounded printable adapter-plate
workflow can qualify; a passing STL parser alone does not establish slicer
acceptance, package portability or general CAD capability. C4 PASS and v1.2
freeze require the owner evidence above plus the final V3 on the final
executable state. No physical print is required.
