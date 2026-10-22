# UX-04.2 — Model Browser & Context Inspector Refinement

Status: **PASS — TECHNICAL VALIDATION AND OWNER EXPERIENCE GATE APPROVED** (2026-10-22).

## Scope and baseline

The bounded UX-04.2 increment starts from `master` at `c186f92`, after the
approved UX-04.1 gate. It refines only the derived Model Browser and Context
Inspector presentation. The authoritative `FeatureModel`, transient
`EditorContext`, topology identities, CAD operations and persistence contracts
are unchanged. The visual direction is
[`UX-REF-01-MODEL`](uiux_v03_pkg/images/UX-REF-01-MODEL.png), with
[`UX-REF-02-SKETCH`](uiux_v03_pkg/images/UX-REF-02-SKETCH.png) informing the
shared shell. Neither image authorizes its future CAD commands.

## Delivered behavior

- The Browser still derives `Part1 → Reference Geometry / Modeling Features /
  Unconsumed Sketches` from feature history. Consumed Sketches appear once,
  beneath their first direct consumer. F2, double-click, right-click actions,
  selected tint, hidden/editing/dirty/failed/blocked cues and native ImGui
  expansion state remain transient. A shorter history hint replaces the
  duplicate Feature Manager heading.
- Feature icons and status glyphs occupy reserved regions at the right of
  each row. Long names are shortened by whole UTF-8 characters to fit the
  native tree-label region and remain available through the full-name tooltip.
  Selected accent still follows SET-01 Orange/Blue, while hidden and
  diagnostic states have textual cues.
- The Inspector groups applicable content into identity, properties or draft
  parameters, relationships, available actions and diagnostics. Its header
  identifies Feature, Face or Edge. Face support offers Sketch on this face
  only when supported; the Edge view has no invented action. Sketch, Extrude,
  Cut Extrude and Boolean show their existing facts using human names. Extrude
  and Cut Extrude drafts label millimeters and keep Confirm/Cancel next to the
  edited fields, before longer relationship and diagnostic text. Switching
  selection discards an unconfirmed draft, including a Face/Edge pick on the
  feature currently being edited; a plane click now uses the same selection
  reset path. No read-only inspection commits CAD data.
- Feature status remains derived from `FeatureModel`; failed and blocked
  diagnostics use `featureStatusDescription`, including the named upstream
  cause. Rebuild-local Face/Edge targets remain validated against the current
  derived cache by the existing selection reconciliation path.

## Targeted validation

**V2**, because the observable UI joins Browser, Inspector and EditorContext
selection with the existing FeatureModel/viewport contracts. Incremental Debug
build of `micro_solidworks_tests` and `micro_solidworks`: **PASS**. Directed
GoogleTest selection: **32/32 PASS**, covering Browser tree grouping, first
Sketch consumer, visibility/editing/diagnostic derivation, stale Face/Edge
inspection, EditorContext transitions, desktop layout, a long-name context
frame at 1366×768 and 1920×1080, rebuild failure recovery, Cut Extrude
parameters, deletion/Undo, rename/Save/Open and preference/CAD-session
separation, Sketch-panel rendering and the guided Extrude workflow.
After the Browser overlap correction, the 11 directly affected tree and
desktop-layout tests passed again.
`git diff --check`: **PASS**.

The new frame test verifies that switching Feature/Face/Edge/Sketch contexts
and rendering a long name at both desktop sizes causes no UI assertion, creates
no edit/delete request and leaves the model unchanged. It is not a substitute
for visual clipping, mouse interaction or the owner Experience Gate.

No clean configure, full build, full CTest or unrelated Geometry/Math/Rendering
suites were run: no CMake, dependency, geometry or rendering code changed.
Existing GL/ImGui test fixtures validate hidden-window construction and normal
teardown. The owner-visible runtime workflow passed and the window closed
normally.

## Experience Gate and limitations

The first owner pass found icons overlapping feature names in the Browser.
The initial custom-drawn text placement was replaced with native ImGui tree
text and width-aware shortening. The owner confirmed the Browser correction
**PASS**, with normal application exit. The owner then reported **APPROVED;
everything passed** for the full Browser/Inspector/Viewport workflow, editing
and Cancel/Confirm, Undo/Redo, Rebuild, Delete/Undo, Save/Open, Sketch mode and
Orange/Blue preference checks. The reviewed Debug executable was at
`build/Debug/micro_solidworks.exe`, using `out/ux-01-gate.microsw` for a
same-part comparison. The roteiro checks Browser hierarchy, long names,
context and state cues; Feature/Sketch/Face/Edge selection; editing with
Cancel/Confirm; Undo/Redo, Rebuild, Delete/Undo, Save/Open; and SET-01 accents.
The owner requested no automated native application control, so visual
evidence is manual. Decision: **APPROVED**. The owner did not state the exact
screen resolution or Windows scaling used in the final visual run; automated
layout frames covered 1366×768 and 1920×1080, while separate manual
resolution and HiDPI evidence is **NOT VERIFIED**.

Name search and derived Solid Bodies grouping were deliberately deferred: the
current small-part history already has explicit groups, and a filter would
need to preserve parent context, stable expansion and selection without
creating a second model. Interactive View Cube, CAD commands pictured in the
reference, global ribbon redesign and new geometry remain out of scope.
Windows 125%/150% visual scaling: **NOT VERIFIED**.

UX-04.1 and UX-04.2 are complete; UX-04 remains in progress and is not frozen. UX-03
remains planned. No ADR, module ownership, dependency or CAD schema deviation.
