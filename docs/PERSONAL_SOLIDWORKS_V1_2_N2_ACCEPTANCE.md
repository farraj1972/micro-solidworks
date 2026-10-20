# V1.2-C4 — N2 printable adapter plate acceptance

Status: **FROZEN UNDER REVISED N2 OWNER ACCEPTANCE WITH EXCEPTIONS**.
On 2026-10-20 the owner explicitly revised this acceptance gate to accept the
validated `F:\downloads\export.stl` as the bounded adapter-plate artifact.
The owner accepted its actual pocket placement and expressly waived the
independent slicer/print-preparation and developer-free Windows environment
checks for this release. Broader model/sketch UI/UX maturation is deferred.
These are acceptance exceptions, not test results: the original exact N2
fixture remains the reproducible engineering oracle below, and no general
printability or clean-machine portability claim follows from this decision.
Final V3 on the executable state passed 927/927 tests, and both the clean
Debug executable and the extracted Release ZIP exited normally with code 0.

## Owner-approved acceptance revision (2026-10-20)

The accepted artifact has SHA-256
`7BDAAA163B88C1D28A97525E39744E3C34C710543FE264C53FB5CCD45B27644E`,
39,884 bytes and 796 triangles. Independent inspection of the binary STL
found finite, nondegenerate triangles, matching file length, closed edge
incidence, consistent winding and positive signed mesh volume (about
`30,910.119 mm³`). Its AABB is X `[-40,40]`, Y `[-25,25]`, Z `[0,8]` mm. The
through-hole and a blind rectangular pocket are present. The pocket floor is
at Z=6 mm and the measured pocket bounds are X `[-0.952690,20.102491]`, Y
`[-2.098002,14.238203]` mm. These mesh measurements do not establish exact
CAD dimensions. The owner previously confirmed that an exported STL could be
read in another application and retracted an initial invalid-file report.
This file predates the later exact-rectangle UI correction in commit
`401a415`; that correction did not change the STL writer/export mesh contract.
The revised owner acceptance is based on the identified file and existing
export contract, not an observed run of the corrected rectangle UI.

For this one revised N2 decision, acceptance uses the actual bounded plate,
through-hole and blind pocket plus the validated CAD-derived STL, prior R2
CAD/edit/persistence contract evidence and same-PC portable Export/Save/Open
evidence. The owner's approval overrides the original requirement that the
pocket exactly match the canonical 18 × 14 mm pocket centered at `(16,0)`.
The owner also dispenses with completing this exact part interactively without
UX assistance, slicer Slice/Prepare evidence, and a developer-free Windows
machine. Those unperformed checks remain explicit product limitations.
No physical print was made or claimed. This is **N2 for this accepted bounded
artifact only**, not a certification of the original exact fixture or the
whole CAD application.

## Release package and reference fixture

Use the [release portable ZIP](PERSONAL_SOLIDWORKS_V1_2_C3_VALIDATION.md)
from `out/packages/PersonalSolidWorks-v1.2.0-win64.zip`. Record the
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
| CAD workflow and edits | All steps 1–4, including rebuild and Undo/Redo | R2 automated and owner CAD evidence PASS; full interactive run for the accepted as-built part waived by owner |
| Native persistence | Save, close, reopen, rebuild final part | R2 automated and same-PC portable Save/Open owner report PASS; exact exported part reopen unverified |
| Export mesh and file | Validation result, STL bytes, AABB/volume, no mutation | Accepted artifact independently inspected as closed/wound/finite with correct plate AABB; C1/C2 tests and bounded UI Export PASS |
| Independent slicer | Name/version, OS, STL file/hash, dimensions, hole, pocket, orientation, print preparation, warnings | **WAIVED BY OWNER** for this release; not run for the accepted artifact |
| Windows portable | OS build, no developer runtime dependency, startup, Save/Open/Export, preferences location, diagnostics, exit code | Same Windows 10 PC workflow owner-reported PASS; final ZIP startup/exit code 0 technical smoke PASS; developer-free environment **waived**, not verified |
| Experience Gate | Owner decision: APPROVED / APPROVED WITH CHANGES / REWORK REQUIRED, with findings | **APPROVED WITH CHANGES BY OWNER** for this bounded artifact; model/sketch UX maturation deferred |

Revised N2 owner decision: **APPROVED WITH EXCEPTIONS** for the identified STL
artifact. The former strict N2 gate would remain unproven; the owner explicitly
changed its acceptance criteria for this release. Final V3 passed and v1.2 is
frozen under that revised decision. Do not infer slicer acceptance, clean
package portability, exact canonical pocket dimensions or general CAD
capability from this approval.

## Experience finding and smallest next correction

The owner could export and read a simpler part but could not complete the
specified N2 plate easily, citing insufficient UI robustness and intuition,
and identified **dimensions and constraints** as the first blocking stage,
without a specific crash or error message. The owner now treats the validated
STL as sufficient for the bounded export gate, while leaving this N2 workflow
limitation as future UX work. The exact N2 dimensions were restated for a
further attempt; the resulting pocket still differed from the fixture.
Code inspection shows that the
reference-plane choice, sketch drawing, dimension editing, face-supported
sketch, Cut direction and export live in separate contextual controls; the
Rectangle tool creates independent sketch lines and the property panel edits
selected geometry or constraints rather than exposing the whole N2 task as a
single guided sequence. These are implementation observations, not a claim
that any one control is the sole blocker.

The bounded C4 correction addresses the concrete pocket-placement failure
without implementing the separately excluded UX-02 in-canvas dimension system.
It leaves the existing Application/UI/EditorContext ownership and Sketch
geometry contract unchanged. Retest from New Part through N2 STL and slicer;
do not declare the Experience Gate approved from source changes alone.

### Exact rectangle entry candidate

The owner's repeat export in `F:\downloads\export.stl` (SHA-256
`7BDAAA163B88C1D28A97525E39744E3C34C710543FE264C53FB5CCD45B27644E`)
contains a closed, consistently wound 796-triangle mesh of an 80 × 50 × 8 mm
plate. The through-hole is present. The pocket floor is at Z=6 but its four
corners bound X `[-0.952690,20.102491]` and Y `[-2.098002,14.238203]`,
area about `343.962 mm²`; the target is X `[7,25]`, Y `[-7,7]`, area `252 mm²`.
This isolates the observed mismatch to interactive pocket placement rather
than STL byte integrity.

The correction adds **Rectangle by dimensions (mm)** while editing a Sketch
with the Rectangle tool. Center X/Y and Width/Height create the existing four
independent `SketchLine` entities via `Sketch::addRectangle`; invalid inputs
are rejected. No new geometry type, constraint, solver or persistence contract
was added. The N2 pocket inputs are Center `(16,0)`, Width `18`, Height `14`.
The targeted Debug build and 7 Sketch Tools/vertical-slice tests passed;
Release build and ZIP generation passed. The final ZIP started and closed with
exit code 0 after extraction. The owner waived the separate interactive
Experience retest of this exact-rectangle UI; it is not recorded as tested.
The release ZIP SHA-256 is in
[C3 validation](PERSONAL_SOLIDWORKS_V1_2_C3_VALIDATION.md).
