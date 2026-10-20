# UX-01 — CAD Workspace Modernization validation

Status: **TECHNICAL V3 PASS / EXPERIENCE GATE PENDING**.
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
| UX-01.5 | In validation | Desktop layout, design comparison, roadmap traceability and Experience Gate. |

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

**Executable screenshots:** pending. The Windows Computer Use service located
the running `Micro SolidWorks — Untitled` window, but both capture attempts
failed (`FrameArrived timed out` and `window capture timed out`). No generated
or reference image is represented as a screenshot of the executable. The owner
must provide manual visual evidence/assessment of Model and Sketch composition
for Experience closure.

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
- UX-01.5 final V3: **PASS**. Fresh configure and full Debug build from
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

## Experience Gate

**Pending owner decision.** Use the 26-step roteiro in the UX-01 campaign,
covering New, plane selection, Sketch, rectangle/dimension, Extrude,
Sketch-on-Face, Cut, edit/Cancel/Confirm, Undo/Redo, Save/Open, Export STL,
Preferences and normal close. Evaluate the Model/Sketch visual match and
browser/inspector simultaneity as well as workflow behavior. Record exactly
`APPROVED`, `APPROVED WITH CHANGES` or `REWORK REQUIRED`, including untested
steps and observations. No freeze commit/tag is permitted until technical V3
passes and the owner approves the Experience Gate.

## Architecture and roadmap

FeatureModel remains the CAD authority; EditorContext owns transient selection
and command state; ApplicationShell owns UI presentation/drafts; viewport and
STL mesh remain derived. ADR-0001–0071, native `.microsw`, SET-01 and the
dependency policy are unchanged. UX-02/UX-03 are planned but unauthorized;
advanced modeling, exchange, drawing, assembly, scripting and CAM remain
tracked in the SPEC-01 horizon. K1/K2/K3 is still open.
