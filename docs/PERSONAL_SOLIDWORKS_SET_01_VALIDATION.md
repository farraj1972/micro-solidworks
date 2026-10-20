# SET-01 — Application Settings & Preferences validation

Status: **FROZEN — OWNER EXPERIENCE GATE AND FINAL V3 PASSED** (2026-10-20).
SET-D01 is FROZEN in ADR-0070/0071, with 71/71 ADRs ACCEPTED. This record
covers only SET-01; UI/UX v0.3 remains PROPOSED as a design reference and
V1.2-C1–C4 remain NOT AUTHORIZED.

## Delivered contract

The Application owns a typed `PreferencesService`. Its schema-version-1 JSON
file is `%LOCALAPPDATA%/MicroSolidWorks/preferences.json`, separate from
`.microsw`. Defaults are Profile A (MMB Orbit, Shift+MMB Pan, wheel Zoom) and
Orange selection. Profile B swaps the two MMB gestures; Blue is an opt-in
selected-state accent. Unknown, partial, invalid and future-version files
cannot prevent CAD startup; diagnostics and a confirmed reset path are provided.
Apply writes a complete temporary file and replaces the user file before
publishing to Viewer/UI. Failed persistence leaves active preferences unchanged.

Edit → Preferences exposes Navigation and Appearance, a color sample, Apply,
OK, Cancel and Restore Defaults. Draft changes are isolated until Apply or OK.
The active MMB gesture is cancelled on a profile change and the next press
starts under the new mapping; camera orientation and zoom math are unchanged.
Selected color is routed to primitive, solid, face/edge and Feature Manager
selection without merging hover, error or editing states.

No CAD authority or schema changed: `FeatureModel`, Geometry, Topology,
`.microsw`, document dirty state, CAD Undo/Redo and rebuild remain independent
of user preferences.

## Targeted validation

- V1: incremental preferences/test build and eight `ApplicationPreferences.*`
  tests passed. Cases cover defaults/missing file, save/reload, invalid inputs,
  partial/unknown fields, backup recovery, future schema confirmation,
  unwritable destination and CAD session invariants.
- V2: incremental Application/UI/Viewer/tests build passed. Affected navigation,
  highlight and Viewer tests passed; the final focused run passed 11/11,
  including real OpenGL draw observations for Blue-selected Point, Segment
  and Line. `git diff --check` passed.
- Unaffected historical suites were intentionally deferred to the explicitly
  required final V3 rather than rerun after each increment.

## Owner Experience Gate

**APPROVED.** On 2026-10-20 the owner reported “PASS; todos os cenários
concluídos” for the live application: defaults, Profile B Apply and
Pan/Orbit/Zoom/Normal To, Blue selection distinct from hover for Sketch,
solid, face and edge across Shaded, Shaded With Edges and Wireframe, Cancel,
Restore Defaults, restart/persistence, return to defaults, clean close, and
unchanged CAD dirty/Undo/Redo. Initial process launch was invisible in the
interactive session; launching through the desktop control interface produced
a visible, responding window. Automated screenshot capture timed out, so the
visual result is owner-observed, not machine-image evidence.

## Final V3

- Fresh CMake configure in `build-set01-validation`: PASS. The local sources of
  the already pinned dependencies were reused; no dependency was added.
- Full Debug build of Application and tests: PASS, zero project warnings.
- Full sequential CTest: **918/918 PASS**, zero failures (438.86 seconds).
- Runtime: the clean-build executable opened a visible window and closed via
  Alt+F4; the window and process exited. The owner gate had separately
  verified Preferences and normal close on the same executable source HEAD.
- Architecture/dependency audit: only Application preferences, UI and Viewer
  consumers, CMake wiring and corresponding tests changed. No change to CAD
  authoritative modules or `.microsw`; ADR-0070/0071 remain conformant.
- Repository hygiene: build output is Git-ignored. `git diff --check` and
  final clean-working-tree review complete the documentary freeze.
