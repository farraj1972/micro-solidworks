# UX-02 — Guided Sketch Interaction & Direct Dimension Editing

Status: **FROZEN — technical V3 and owner Experience Gate PASS** (2026-10-21).

UX-01 and the approved UX-01.R1 workspace composition remain frozen. The UI/UX
v0.3 package is a proposed design reference; UX-REF-02-SKETCH is the primary
visual comparator, with UX-REF-01-MODEL checking shell consistency. The new
dimension graphics are derived from solved Sketch geometry and stable
`SketchConstraintId` references. No dimension value is stored in the canvas.

## Delivered slice

- During Sketch editing, existing driving LineLength, CircleRadius,
  HorizontalDistance and VerticalDistance constraints receive millimetre
  callouts, dimension/auxiliary lines and selection/hover cues. Other constraint
  types have no numeric callout. Positions pass through the SketchPlane and the
  same viewport projection as sketch geometry, including supported face planes.
- Clicking a label selects its logical dimension. Double-clicking it, using the
  nearby Edit action or Inspector Edit opens one transient numeric draft.
  Confirm validates and solves a copied model, atomically commits successful
  rebuild and records one Undo entry. Cancel/Escape leave the model unchanged.
- The Inspector reflects the selected dimension, unit, associated entity,
  related-constraint count and an applicable Edit action. The full constraints
  list is collapsed by default. Model mode does not show Sketch controls.
- Undo/Redo retains the active Sketch and selected dimension when they remain
  valid, allowing the geometry and callout to be compared immediately.
- Entering an existing Sketch frames its derived geometry in the current
  Sketch-plane orientation. An off-centre Right-plane line otherwise projects
  entirely outside the viewport, suppressing its dimension callout.
- The display uses the SET-01 selected accent. No Sketch/solver/persistence
  schema, dependency direction, frozen ADR or external dependency changed.

## Experience Gate roteiro

Run `out/ux-02-v3-r3/Debug/micro_solidworks.exe` from the repository. The
owner must decide `APPROVED`, `APPROVED WITH CHANGES` or `REWORK REQUIRED` after
observing the final build. Use two parts because the existing UX-01 fixture's
only driving dimension belongs to an auxiliary line, not its base extrusion.

1. Open `out/ux-01-gate.microsw` with File → Open. Confirm Model Browser left,
   Graphics Area center, Inspector right. Right-click `Sketch3` in the Browser
   and choose `Edit Sketch`. Confirm Sketch mode and `L 30 mm` on the same line,
   without canvas displacement. Compare layout and legibility to the canonical
   `UX-REF-02-SKETCH.png` (functional composition, not pixel identity).
2. Select the line and then click the `L 30 mm` label. The Inspector must switch
   from entity to selected dimension; hover and selected feedback must be
   distinct. Double-click the label or use `Edit dimension`. Set 36 mm, click
   `Cancel dimension`: value, line, dirty state and Undo availability must stay
   as before this draft. Repeat with zero: `Confirm dimension` must be disabled
   with a clear positive-value diagnostic. Use Escape to dismiss that draft.
3. Edit `L 30 mm` to 36 mm and click `Confirm dimension`. The line, label and
   Inspector must update immediately. Ctrl+Z restores 30; Ctrl+Y restores 36.
   `Solve / Rebuild Sketch` must give completion/diagnostic feedback without an
   extra logical edit. Pan/zoom and switch Perspective/Orthographic; label and
   line must stay together. Exit Sketch. Save As
   `out/ux-02-dimension-gate.microsw`, close, reopen it, then Edit Sketch3 and
   verify `L 36 mm` and the line length persisted.
4. Start a fresh document with File → New. Select `Front Plane` in Reference
   Geometry, click Sketch, draw one Line, return to Select, select the line and
   click `Add line length constraint` in the Inspector. Verify a line-length
   callout in the Front plane. Exit Sketch. This is a projection check; no
   extrusion is needed for this open profile.
5. Start another fresh document. Select `Top Plane`, click Sketch, choose
   Circle, click a center and a radius point, then Select the circle and click
   `Add radius constraint`. Verify `R ... mm` follows the circle. Exit Sketch,
   select its Sketch in the Browser, click Extrude, enter Depth 5 mm and
   `Create extrusion`. Edit the source Sketch, select its radius callout, change
   to a different positive value and Confirm. The circle and callout must
   update immediately; after Exit Sketch the extruded solid must rebuild to
   the new diameter. Undo/Redo must restore/reapply it. Save As
   `out/ux-02-circle-gate.microsw`, close/reopen, and verify the new radius and
   solid persist.
6. On the circle solid, select a supported planar cap, use `Sketch on this
   face`, draw a small circle and add its radius constraint. Verify its callout
   on the face plane; pan/zoom and inspect the label and picking. Exit Sketch
   without creating another feature. Return to Model mode and verify Sketch-only
   controls disappear. Change selection color in Edit → Preferences from Orange
   to Blue, Apply and compare selected callout/geometry to hover; restore the
   original preference afterward. Preferences alone must not dirty the part
   or add Undo history. Close normally.

If a step fails, stop and report that step, the preceding action and visible
diagnostic. A disabled Rebuild button when every feature is already clean is
expected; use `Solve / Rebuild Sketch` for the explicit sketch feedback check.

Report the first failed step and the observed behavior, or confirm every step
passed. State the display resolution and Windows scale if known. A capture of
the final Sketch screen may be stored in `docs/ux02_validation/` as actual
executable evidence; canonical references must be identified separately.

## Evidence and limits

- UX-02.1–02.3 incremental builds and focused unit/contract tests: PASS.
- UX-02.4 incremental app/test build and 20 directly affected tests: PASS.
  Coverage includes dimension identity/association, four numeric types,
  principal/offset plane projection, label hit-testing, transactional edit,
  solver rollback, Undo/Redo and Save/Open contracts.
- Owner retest found a null selection dereference after clicking a dimension;
  the inspector now handles a selected dimension without selected geometry.
  A focused hidden-window regression test passes.
- Owner then found no callout on the off-centre `Sketch3` Right-plane line.
  A viewer regression test reproduces both endpoints outside the viewport
  before framing, and verifies visibility after framing in Perspective and
  Orthographic. Incremental app/test builds and 23 affected tests: PASS.
- The subsequent owner screenshot shows the callout displaced from its line
  by approximately the Model Browser and ribbon offsets. The projected
  callout is Workspace-local; its drawing and hit test must add the Workspace
  window position rather than the main viewport position. A direct ImGui
  vertex-position regression test and six related tests pass. The owner
  retested the corrected build: line/callout alignment and click/edit PASS;
  the application closed normally. The owner's pre-fix executable capture is stored as
  `docs/ux02_validation/callout-offset-before-fix.png`; it is failure evidence,
  not a canonical reference or a post-fix visual claim.
- A selected lateral pocket face shows a tonal boundary only while selected in
  Shaded with Edges. Unselected geometry has no exposed edge there. This is a
  visual face-selection finding, not evidence of a broken Solid; it did not
  block the owner-approved UX-02 workflow. A later Model display refinement
  may evaluate this separately.
- Automated Windows UI capture found the running application but the capture
  service returned `FrameArrived timed out: timed out waiting on channel`.
  This is an evidence collection failure; no visual PASS is inferred from it.
- Final fresh V3 configure/full Debug build and CTest: **PASS, 940/940 tests**.
  The owner opened the same clean-build executable for the focused alignment,
  click/edit and normal-exit retest: PASS.
- The owner completed the full documented roteiro, reported **PASS** and
  confirmed normal application exit. This is the UX-02 Experience Gate
  **APPROVED** decision and includes the functional/visual comparison to
  UX-REF-02 and shell consistency with UX-REF-01. No post-fix screenshot was
  archived; the manual observation is the visual acceptance evidence.
- The validated executable source HEAD was `9e01ea8`. Freeze changes only
  documentation and evidence metadata, so freeze validation is V0.

## Architecture

Authoritative Sketch → Constraint Solver → FeatureModel rebuild → solved Sketch
→ derived Presentation/canvas. EditorContext owns transient logical selection;
ApplicationShell owns the edit draft. The CAD model never receives screen
coordinates, and `.microsw` remains unchanged.
