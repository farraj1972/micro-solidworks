# V1.2-C3 — Windows Portable Distribution Validation

Status: **CANDIDATE / CLEAN-ENVIRONMENT EVIDENCE PENDING**.
The portable Release package exists. The owner reports the bounded workflow
passing from an extracted copy on the usual Windows PC, but no machine free of
development tools has been tested; C3 is not declared fully PASS yet.

## Package recipe and inventory

CMake's Runtime install component packages only `micro_solidworks.exe`, the
MSVC redistributable DLLs discovered by `InstallRequiredSystemLibraries`,
the [portable guide](PERSONAL_SOLIDWORKS_V1_2_PORTABLE_GUIDE.md), README and
third-party notices/license texts. CPack ZIP produces
`out/packages/PersonalSolidWorks-v1.2.0-candidate-win64.zip`; generated
archives remain ignored under `out/`. No test binaries, SDKs, sources,
installer, updater or telemetry are included. The package label is
**candidate** until all v1.2 gates pass.

Current candidate ZIP SHA-256 after the C4 exact-rectangle correction:
`73EEB1AB231FEB190FDC0B55B03E33DF5028146BC2B672A29AA55EC6E93CA303`.
The earlier C3 package inspected for the owner's simple-part smoke was
`7F57CD902241341A8768CF41935D64A2CDEC7A9A254F33C5402D6C53B05E5ABF`.
Neither hash identifies a frozen release artifact. The updated package needs
its own runtime/Experience check.

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

Subsequently, the owner ran the copy in `F:\downloads` and reported
Save/Close/Open, preferences, STL export/readback, Cancel preserving document
dirty/Undo/Redo and normal window close. The host registry reports Windows 10
Pro 22H2, build 19045.6466. That earlier extracted executable SHA-256 equalled
the then-current Release build executable SHA-256
`90FF471E4B868759B0DCB195A2EC8D54DD0359C6A4B91A113D286A23BE7E5CB1`.
A process was briefly visible after the owner's close report and was absent
on the next check; its exit code was not captured. This is functional
same-machine portable evidence, not clean-environment dependency proof.
The updated C4 candidate executable SHA-256 is
`36A69A6539E3DA11BA26D15280D1E167176CAA4A8EF2C9FCAC4C268D5C8B0134`.

Pending evidence: open the package on an appropriately isolated Windows 10
x64 machine without development tools, if available; validate and export the
**N2 fixture** STL, confirm preferences stay under `%LOCALAPPDATA%`, observe
diagnostics and capture normal exit code 0. Record GPU/OpenGL capability,
dependency state and the result of each operation. A same-developer-machine
launch does not demonstrate independence from installed developer runtimes.
The bounded C2 export Experience check is now owner-reported PASS; the
integrated C4/N2 workflow remains separate.

Validation level: **V2 targeted distribution**. Release build and archive
generation PASS; package inventory PASS; out-of-tree startup and owner-reported
Save/Open/Export/preferences/close on Windows 10 PASS; clean-environment runtime
and exit code **NOT VERIFIED**. Full Debug
CTest and clean V3 are reserved for final baseline validation after C1–C4.
