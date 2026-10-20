# Personal SolidWorks v1.2.0 — First Functional Replacement

Status: **FROZEN under the owner's revised bounded N2 acceptance**
(2026-10-20). Annotated release tag: `v1.2.0`.

## Gate evidence

- C1 CAD-derived export mesh and validation: PASS.
- C2 binary STL export, independent parser and bounded owner UI check: PASS.
- C3 portable Windows package: owner-accepted from a same-PC extracted copy.
  Testing on Windows without developer tools and a captured portable exit code
  were expressly waived; they are not verified.
- C4/N2: the owner accepted the identified as-built adapter-plate STL, SHA-256
  `7BDAAA163B88C1D28A97525E39744E3C34C710543FE264C53FB5CCD45B27644E`.
  Independent binary inspection found a finite, closed, consistently wound
  796-triangle mesh, X/Y/Z bounds 80 × 50 × 8 mm, through-hole and blind
  pocket. The pocket differs from the original 18 × 14 mm reference at
  `(16,0)`. The owner waived that exact pocket, complete model/sketch UX,
  independent slicer preparation and clean Windows testing for this release.
- Final technical V3 on the executable code: fresh CMake configure, complete
  Debug build with zero observed project warnings, 927/927 CTest PASS,
  application startup and normal window close with exit code 0. The release
  package was built from the same executable source state. No accepted ADR,
  kernel choice or CAD ownership boundary changed.
- After V3, the package filename and bundled documentation were finalized in
  CMake. Reconfiguration and incremental Debug/Release builds passed; the
  Debug executable SHA-256 remained
  `18300730B82266F5B8B3FE882D72ED49EB8C8063C8A9D81E8E6FEA58C281930B`,
  and the Release executable remained
  `36A69A6539E3DA11BA26D15280D1E167176CAA4A8EF2C9FCAC4C268D5C8B0134`.
  The final ZIP was inventoried, extracted to a fresh temporary directory,
  started and closed normally with exit code 0. Full CTest was not repeated
  because no executable bytes changed.

See the [N2 acceptance record](PERSONAL_SOLIDWORKS_V1_2_N2_ACCEPTANCE.md),
[C1](PERSONAL_SOLIDWORKS_V1_2_C1_VALIDATION.md),
[C2](PERSONAL_SOLIDWORKS_V1_2_C2_VALIDATION.md) and
[C3](PERSONAL_SOLIDWORKS_V1_2_C3_VALIDATION.md) evidence. The original
canonical fixture and strict slicer gate are retained there as historical
engineering criteria; the owner's revision applies only to this accepted
artifact and release.

## Delivered capability and limits

This release adds a validated CAD-derived export mesh, transactional binary
STL export from the current FeatureModel result, a user-facing Export dialog,
and a Windows ZIP distribution. The STL is unitless; the application uses mm.
The accepted file was validated structurally. The owner separately reported
that an exported STL was read by another application; this was not an
independent readback of the exact accepted file. No physical print, slicer
Slice/Prepare result or developer-free Windows run is claimed. Printability
for a particular printer/material/process remains to be checked separately.
STL import, STEP, CAM, general kernel migration and wider UI/UX work remain
outside this release. `INT-001-EXP` is bounded and delivered; `INT-001-IMP`
is absent, so the compound interoperability requirement remains partial.

The v1.1.0 baseline and its decisions remain frozen. The owner will manage
the push to GitHub.
