# UX-01 — CAD Workspace Modernization validation

Status: **FROZEN — TECHNICAL V3 PASS / EXPERIENCE GATE APPROVED WITH CHANGES**.
Entry baseline: frozen Personal SolidWorks v1.2.0 (`b34ca58`), with frozen
v1.1.0, P0, SPEC-01 and SET-01 unchanged. UX-01 is the only authorized UI
campaign in this record. UX-02/UX-03, kernel work and new CAD/CAM capabilities
remain outside scope.

## Canonical visual references

The four checked-in PNGs were opened and examined directly; their SHA-256
values match `uiux_v03_pkg/VISUAL_REFERENCE_REGISTRY_V0_3.md`.

| Reference | UX-01 use | Deliberate limit |
| --- | --- | --- |
| `UX-REF-01-MODEL.png` | Main hierarchy: document, menu, contextual command ribbon, left browser, central viewport, right inspector and guidance. | Only existing CAD commands appear. No Hole Wizard, Fillet, Revolve or other illustrated future features. |
| `UX-REF-02-SKETCH.png` | Distinct Sketch mode, drawing/constraint tools, visible browser and contextual property editing. | In-canvas dimensions, floating toolbar and unimplemented tools remain UX-02 gaps. |
| `UX-REF-03-MANUFACTURE.png` | Future consistency reference. | No Manufacture tab, CAM commands, simulation or toolpaths. |
| `UX-REF-04-EXPORT.png` | Consistency reference for current STL export. | Existing validator-driven STL dialog retained; split CAD/mesh preview and other formats remain outside UX-01. |

The PNG artwork was not copied into the application. Existing project-drawn
icons and ImGui styling implement the supported commands. SET-01 still owns
navigation and selection accent: Profile A/Orange remain defaults; Profile B
and Blue are opt-in.

## Implemented increments

| Increment | Result | Scope |
| --- | --- | --- |
| UX-01.1 | Implemented | Document header, responsive left/center/right layout, status area, explicit Model/Sketch workspace heading, integrated property inspector and dark graphite/blue styling. |
| UX-01.2 | Implemented | Browser remains visible while the Inspector shows selected feature, face, edge, support, source/target, editable parameters and diagnostics; transient draft resets on selection changes. |
| UX-01.3 | Implemented | Model and Sketch ribbons expose existing actions with contextual availability and tooltips. Unsupported illustrated commands remain absent. |
| UX-01.4 | Implemented | Guidance names current mode, next step and result; editing and FAILED/BLOCKED/dirty states have explicit cues. Existing Confirm/Cancel transactions remain authoritative. |
| UX-01.5 | Complete | Desktop layout, design comparison, roadmap traceability, correction and Experience Gate. |

## Visual comparison and intentional deviations

The composition follows the Model/Sketch references in panel order and
contextual hierarchy. The native ImGui main menu precedes the in-app document
header; the images place the document header above the menu. The Model Browser
uses the existing parametric history and principal planes, without inventing
body folders or CAD identities. The Inspector is one scrolling contextual panel
instead of the mockup's full tab family. View controls are HUD buttons and menus,
not an interactive view cube. Sketch dimensions remain in the Inspector rather
than on the canvas. The existing Export STL dialog remains in place. These
deviations preserve frozen behavior and avoid fictitious capabilities.

**Executable screenshots:** the owner supplied a [Model capture before the
legibility correction](ux01_validation/UX-01-MODEL-BEFORE-CORRECTION.png) and a
[Model capture after correction](ux01_validation/UX-01-MODEL-EXECUTABLE.png).
The latter is 1919×1041 pixels; the owner estimated Windows scaling at 100%.
These are captures of the real executable, not the reference images. Computer
Use located the running window, but automatic capture timed out twice, so the
owner captured and assessed it manually. A Sketch-mode executable screenshot
was not supplied; the owner inspected both modes in the Experience Gate.

The first capture exposed a tiny built-in pixel font, feature icons taller than
their tree rows, and Sketch controls displayed in the Model inspector. The
corrective change loads the installed Windows UI font (with a fallback), draws
tree icons at row size and renders Sketch-specific controls only in Sketch mode.
The owner confirmed all three defects corrected in the second build. Remaining
conceptual/design differences are acknowledged for later formalization, without
altering the approved UX-01 scope or authorizing UX-02/UX-03.

## Validation

Affected dependency closure: UI/Application and their existing editor,
selection, viewport, persistence-session and preferences contracts. No Math,
Geometry, Topology, Modeling, CAD kernel or external dependency changed.

- UX-01.1 incremental Debug app/test build: PASS. Relevant 63 tests across
  preferences, tree, editor, guided workflow, persistence session, viewport
  and selection: PASS. New layout test confirms usable viewport at 1366×768
  and 1920×1080 using a hidden window.
- UX-01.2 incremental build and 15 directly relevant tests: PASS.
- UX-01.3 incremental app build and contextual availability/layout tests: PASS.
- UX-01.4 incremental app build: PASS; transaction semantics unchanged and
  previously affected history tests passed.
- UX-01.5 first V3: **PASS**. Fresh configure and full Debug build from
  `out/ux-01-v3-build` using the already pinned local dependency sources;
  928/928 CTest PASS; zero compiler warnings; the executable created a main
  window and exited normally with code 0 after a close request. Build and
  test logs are in ignored `out/ux-01-v3-build.log` and
  `out/ux-01-v3-ctest.log`.
- Architecture/dependency audit: only UI, a UI layout test and documentation
  changed relative to v1.2.0. CMake, external dependencies, core CAD modules,
  persistence contracts and ADRs are unchanged. `git diff --check` PASS.
- Windows 125%/150% visual scaling: pending owner observation if available;
  the layout test checks logical desktop sizes, not OS DPI rendering.
- Corrective UI change after the first V3: affected app/test incremental build
  PASS; seven directly relevant UI, feature-tree and guided-workflow tests
  PASS; `git diff --check` PASS.
- Final V3 after the correction: fresh configure and full Debug build in
  `out/ux-01-v3-r2` PASS with no project compiler warnings; full CTest
  **928/928 PASS** in 323.46 seconds. The owner opened that clean-build
  executable and closed it normally through File → Exit. A separate automated
  hidden-window close probe did not exit after `CloseMainWindow`; its test
  processes were stopped and it is not used as evidence of normal shutdown.
  Normal interactive shutdown is supported by the owner's direct observation.

## Experience Gate

The owner completed the 26-step roteiro in guided blocks: steps 1–4, 5–9,
10–14, 15–16, 17–21, 22–23 and 24–25 were all reported PASS. The Save/Open
fixture and binary STL exist under ignored `out/`. The first visual review was
`REWORK REQUIRED`; the owner supplied the screenshot above. After the
legibility/context correction the owner confirmed the three observed defects
fixed and issued **APPROVED WITH CHANGES** for Model and Sketch. The stated
condition is that remaining conceptual and design differences will be
formalized later; none was specified as a remaining UX-01 blocking change.
The owner reported normal close of both the corrected incremental build and
the clean final V3 executable. This Experience decision does not approve any
new command, architecture or future campaign. The technical and experience
gates are complete. The remaining conceptual/design differences will be
formalized separately; no new UX-01 blocking requirement was specified.

## Architecture and roadmap

FeatureModel remains the CAD authority; EditorContext owns transient selection
and command state; ApplicationShell owns UI presentation/drafts; viewport and
STL mesh remain derived. ADR-0001–0071, native `.microsw`, SET-01 and the
dependency policy are unchanged. UX-02/UX-03 are planned but unauthorized;
advanced modeling, exchange, drawing, assembly, scripting and CAM remain
tracked in the SPEC-01 horizon. K1/K2/K3 is still open.

## Freeze

The final executable source is the corrective UX-01.5 commit `ab2eafa`.
Only documentation/state metadata changes after that validated source; V0
consistency review, `git diff --check` and Git status complete the freeze.
The local annotated tag is `ux-01-cad-workspace-modernization`. No push is
performed; the owner manages GitHub publication.
