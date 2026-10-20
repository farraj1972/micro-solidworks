# V1.2-C3 — Windows Portable Distribution Validation

Status: **CANDIDATE / PENDING WORKFLOW AND CLEAN-ENVIRONMENT EVIDENCE**.
The portable Release package exists; C3 is not declared PASS yet.

## Package recipe and inventory

CMake's Runtime install component packages only `micro_solidworks.exe`, the
MSVC redistributable DLLs discovered by `InstallRequiredSystemLibraries`,
the [portable guide](PERSONAL_SOLIDWORKS_V1_2_PORTABLE_GUIDE.md), README and
third-party notices/license texts. CPack ZIP produces
`out/packages/PersonalSolidWorks-v1.2.0-candidate-win64.zip`; generated
archives remain ignored under `out/`. No test binaries, SDKs, sources,
installer, updater or telemetry are included. The package label is
**candidate** until all v1.2 gates pass.

Candidate ZIP SHA-256:
`7F57CD902241341A8768CF41935D64A2CDEC7A9A254F33C5402D6C53B05E5ABF`.
This identifies the package inspected here, not a release artifact.

Release `dumpbin /dependents` directly lists `MSVCP140.dll`,
`MSVCP140_2.dll`, `VCRUNTIME140.dll`, `VCRUNTIME140_1.dll`, Windows system
DLLs and Windows UCRT API sets. The four direct MSVC runtime DLLs and their
related CRT companions are present in the ZIP. GLFW, GLAD, Dear ImGui,
spdlog and project libraries are linked statically. The UCRT and OpenGL
system facilities are supplied by Windows; a Windows 10 x64 environment with
OpenGL 3.3 support is required. Preferences remain in
`%LOCALAPPDATA%\MicroSolidWorks\preferences.json`, separate from the
portable folder and `.microsw` files.

## Evidence and limits

An incremental **Release** build of `micro_solidworks` passed with zero
project warnings. CPack initially attempted to install GoogleTest artifacts;
limiting the package to the Runtime component corrected this without adding
test libraries to the archive. The ZIP was inspected and extracted under
`%TEMP%`, outside the repository. The extracted executable launched and its
process reported `Responding: True`. A normal close request on that hidden
technical smoke did not produce a confirmed clean exit; the disposable smoke
process was stopped. This is **startup evidence only**, not a shutdown PASS.

Pending evidence: open the package on an appropriately isolated Windows 10
x64 machine without development tools; create or open a native v1/v2 document,
Save/Open, validate and export the fixture STL, confirm preferences stay under
`%LOCALAPPDATA%`, observe diagnostics and close normally with exit code 0.
Record OS build, GPU/OpenGL capability, package hash, dependency state and
the result of each operation. A same-developer-machine launch does not
substitute for this gate. The Debug C2 owner Experience Gate also remains
pending before the integrated C4 workflow.

Validation level: **V2 targeted distribution**. Release build and archive
generation PASS; package inventory PASS; out-of-tree startup PASS; isolated
Windows 10, Save/Open/Export and normal shutdown **NOT VERIFIED**. Full Debug
CTest and clean V3 are reserved for final baseline validation after C1–C4.
