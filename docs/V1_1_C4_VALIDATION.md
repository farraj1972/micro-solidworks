# V1.1-C4 — Subtractive Modeling & Sketch-on-Face Validation

## Execution state

`IN PROGRESS`

Existing implementation commits provide semantic Sketch support, persistence
v2 with v1 migration, and the CutExtrude feature contract. The executable
subtraction kernel and product workflow remain under implementation.

## D18-R1 clarification

General oblique subtraction between the current exact extrusion subset and a
cylindrical target can produce elliptical or other conic intersection
boundaries. Current authoritative topology edges support only `Segment3` and
`CircularArc3`.

C4 therefore supports Cut Extrude only when its direction is parallel or
anti-parallel, within the existing angular tolerance, to the recoverable
extrusion axis of every affected target body. StartCap and EndCap face Sketches
meet this condition by construction. Reverse Direction remains supported.

Unsupported orientations and target provenance return `UNSUPPORTED_TARGET`
transactionally. The normal product diagnostic directs the user to create the
Sketch on a supported Extrude start or end face and cut normal to that face.
No conic approximation, mesh Boolean, or tessellation-derived topology is
allowed.

General oblique cuts and conic topology are deferred and require a future
Decision Gate.

## D19 camera decision

ADR-0067 freezes a Viewer-owned arbitrary orthonormal camera basis. Normal To
uses `forward = -SketchPlane.normal`, `right = +SketchPlane.xAxis` and
`up = +SketchPlane.yAxis`. Existing navigation and standard views remain a
compatibility requirement. D19 documentary validation used V0; executable
camera validation follows as part of C4.

## Validation record

- D18-R1 documentation consistency: PASS
- Validation level: V0
- Executable validation: intentionally not run — clarification changes no
  executable or build-system files
- Technical Gate: pending completion of C4
- Experience Gate: pending completion of C4
