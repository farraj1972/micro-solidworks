# V1.2-C3 — Windows Portable Distribution Validation

Status: **FROZEN FOR BOUNDED V1.2 DISTRIBUTION WITH OWNER EXCEPTIONS**.
The portable Release package exists. The owner reports the bounded workflow
passing from an extracted copy on the usual Windows PC. On 2026-10-20 the
owner explicitly waived a developer-free Windows environment for this N2
release decision. That environment has not been tested, so portability to it
remains an unverified limitation, not a demonstrated PASS.

## Package recipe and inventory

CMake's Runtime install component packages only `micro_solidworks.exe`, the
MSVC redistributable DLLs discovered by `InstallRequiredSystemLibraries`,
the [portable guide](PERSONAL_SOLIDWORKS_V1_2_PORTABLE_GUIDE.md), README and
third-party notices/license texts. CPack ZIP now produces the release archive
`out/packages/PersonalSolidWorks-v1.2.0-win64.zip`; generated archives remain
ignored under `out/`. No test binaries, SDKs, sources, installer, updater or
telemetry are included. The archive contains the v1.2 portable guide and
release evidence summary.

Final release ZIP SHA-256:
`6E0815CB97EE887F3BEFA7AC88F1EAFE2BEA3A9D7E946700642A3906595E9F67`.
The final Release EXE SHA-256 is
`36A69A6539E3DA11BA26D15280D1E167176CAA4A8EF2C9FCAC4C268D5C8B0134`.
The preceding C4 candidate ZIP was
`73EEB1AB231FEB190FDC0B55B03E33DF5028146BC2B672A29AA55EC6E93CA303`.
The earlier C3 package inspected for the owner's simple-part smoke was
`7F57CD902241341A8768CF41935D64A2CDEC7A9A254F33C5402D6C53B05E5ABF`.
The final package was extracted to a fresh temporary directory and its EXE
created the GLFW window, then exited normally with code 0 after a native
window-close message. This checks package startup/shutdown, not interactive
modeling or export in that extracted copy. The owner waived a repeat full
Experience run for the exact-rectangle correction.

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
The final Release executable is byte-identical to the updated C4 candidate
executable; only package labeling and documentation changed after its build.

Deferred evidence: open the package on an appropriately isolated Windows 10
x64 machine without development tools, if available; confirm preferences stay
under `%LOCALAPPDATA%`, observe diagnostics and capture normal exit code 0.
Record GPU/OpenGL capability, dependency state and the result of each
operation. The owner waived these checks for the revised bounded N2 decision;
a same-developer-machine launch does not demonstrate independence from
installed developer runtimes. Export of the exact canonical N2 fixture and
independent slicer preparation were also waived for this acceptance, as
recorded in the [N2 decision](PERSONAL_SOLIDWORKS_V1_2_N2_ACCEPTANCE.md).
The bounded C2 export Experience check is owner-reported PASS; the original
integrated C4/N2 workflow was not repeated and was revised by the owner with
the exceptions recorded above.

Validation level: **V2 targeted distribution plus final package smoke**. Release build and archive
generation PASS; package inventory PASS; out-of-tree startup and owner-reported
Save/Open/Export/preferences/close on Windows 10 PASS; clean-environment runtime
on a developer-free machine **NOT VERIFIED / OWNER-WAIVED**. The final ZIP's
startup and normal close returned exit code 0; that does not substitute for
an interactive feature/export retest of the updated UI. Final Debug V3 passed
927/927 tests separately.
