# V1.2-C2 — STL Export Implementation & UX Validation

Status: **TECHNICAL PASS / EXPERIENCE PENDING**. The File → Export STL flow is
implemented in the Debug candidate. Owner observation of the dialog and its
Cancel/Validate/Export behavior is pending. C2 is not declared fully accepted
until that evidence arrives.

## Behavior

The Application selects `FeatureModel::resultFeatureId()` and an explicit
current body. C1 rebuilds and validates a derived indexed mesh before binary
STL writing. The writer derives normals from validated outward triangle
winding, serializes little-endian binary STL, writes a sibling temporary file,
checks the expected byte count, then replaces the destination with a
write-through Windows move. Failed validation or write cannot report success;
the previous destination is preserved where the filesystem permits. No
`.microsw` content, CAD dirty flag or Undo/Redo entry is changed by export.

The UI exposes File → Export STL..., current document/result, body, binary STL,
explicit millimetre convention, Standard/Fine mesh quality, Browse, Validate
mesh, Export, Cancel and actionable status. A green validation state follows
the actual C1 validator. The destination uses a native Windows Save dialog;
the STL file remains unitless, so the user must confirm mm scale in a slicer.
The full UX-REF-04 workspace and other file formats remain outside scope.

## Directed technical evidence

Incremental Debug build of `micro_solidworks` and `micro_solidworks_tests`:
**PASS**, zero project warnings after closing an older running application
instance that initially blocked linker output. `ExportMesh.*`, `StlWriter.*`,
`SolidDisplay.*` and the R2 persistence/history contract: **13/13 PASS**.
The test-only independent binary parser checks the exact 84+50N byte layout,
triangle count, normals, finite coordinates, AABB and signed volume of both a
box and the canonical holed/pocketed plate. Repeated writes are byte-identical;
an invalid mesh preserves an existing destination. The parser does not share
the production writer's byte-encoding routine.

Validation level: **V2** for Application/UI, native dialog, export I/O and
FeatureModel/Presentation integration. Full CTest and clean build are reserved
for the final baseline V3. Slicer acceptance and isolated Windows Release
package are C4/C3 evidence respectively. The automated Windows capture failed
with `FrameArrived timed out` after window activation failed; manual owner UI
observation was requested. This is an environment limitation, not a UI PASS.

## Experience check

Open a valid rebuilt Solid, then File → Export STL...; confirm document/result,
body, binary STL, mm and quality. Validate, choose a `.stl` path, Export, and
inspect the result/diagnostics. Cancel a second attempt and confirm native
document dirty/Undo/Redo and any prior STL are unchanged. Test no current
Solid and an invalid destination. Close normally. Record the first failure
before declaring Experience PASS.
