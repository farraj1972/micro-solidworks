# Personal SolidWorks v1.2 — Windows portable use

This is the bounded v1.2 package, not a general 3D-printing certification.
Extract the archive
to a writable user folder, then run `micro_solidworks.exe`. Windows 10 x64,
OpenGL 3.3-compatible graphics and a supported MSVC/UCRT environment are
required. The archive includes the MSVC redistributable DLLs found by CMake;
Windows system DLLs and UCRT remain provided by Windows.

Use File → New for a part and File → Save/Save As for `.microsw`. Open accepts
native v1/v2 documents; a successful rebuild is required before exporting.
Personal navigation and selection preferences remain in
`%LOCALAPPDATA%\MicroSolidWorks\preferences.json`, not beside the executable.
The portable folder is not a document or settings store.

For STL, finish the current Solid and use File → Export STL.... Confirm the
current result and body, choose Standard or Fine quality, click **Validate
mesh**, then Browse to choose the destination and click **Export**. A result
with several bodies must be exported one selected body at a time. The UI and
writer use **millimetres**, but STL itself has no reliable unit field. Open the
file in an independent slicer, set/confirm millimetres, check X/Y/Z extents,
orientation, through-hole and pocket, and inspect any slicer warnings before
preparing a print. A green mesh result means the supported manifold checks
passed; it is not proof of material/process printability.

The canonical engineering reference plate has an 80 × 50 × 8 mm base, Ø8 through-hole at
(-22, 0) mm and 18 × 14 × 2 mm pocket centred at (16, 0) mm. The supported
edit sequence changes width 80 → 90 mm and pocket depth 2 → 3 mm, then rebuilds
and exports again. Save/Open and Undo/Redo are CAD document operations;
export does not alter the document or create an Undo step. Cancel in the
Export dialog leaves the document and any previous STL unchanged.
The owner's accepted v1.2 STL had a differently positioned pocket. Slicer
preparation and testing on a developer-free Windows machine were waived for
this release, not demonstrated. Verify any part in the intended slicer before
printing it.

Known scope limits: Cut Extrude targeting another Cut Extrude, general STL
import, STEP/3MF/DXF, CAM, unsupported disconnected/overlapping mesh
components and general self-intersection verification are outside this
release. A validation or write failure reports a diagnostic and does not
claim an STL success.
