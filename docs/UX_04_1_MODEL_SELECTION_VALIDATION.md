# UX-04.1 — Model Selection & Context Clarity

Status: **TECHNICAL PASS / EXPERIENCE GATE PENDING** (2026-10-22).

## Baseline and scope

Started from `master` at `628d44f` with a clean working tree. UX-01, UX-01.R1,
UX-02, SET-01 and Personal SolidWorks v1.2.0 remain frozen. This increment
changes only derived solid display, viewer highlight and Model workspace UI.
It does not change CAD topology, modeling, feature identity, persistence or
the accepted ADRs. The canonical visual reference is
[`UX-REF-01-MODEL.png`](uiux_v03_pkg/images/UX-REF-01-MODEL.png); UX-REF-02
informs the related Sketch context. UX-REF-03/04 are future consistency
references, not executable requirements here.

## Rendering finding and reproduction

A directed fixture creates a rectangular base Extrude, a Sketch on its EndCap
and a blind rectangular Cut Extrude. The resulting Solid contains at least two
distinct planar `FaceId`s on the same exterior lateral wall, split at the
pocket-floor height. `SolidDisplay` suppresses the coplanar shared edge, so the
normal Shaded With Edges pass does not draw a false CAD edge there. The previous
Viewer selection pass nevertheless recolored the triangles of only the picked
`FaceId`. The neighboring coplanar patch kept the base shade, creating the
owner-observed tonal boundary only when that face was hovered or selected.
This is a **derived selection-overlay defect**, not evidence of a broken or
open CAD face. Face normals and the underlying Solid were not changed.

The selected and hovered face overlay now follows only visible, real CAD
edges incident to the exact selected face. The coplanar suppressed edge stays
absent; face and edge picking retain their existing rebuild-local typed
targets. Shaded keeps lit faces without global edges; Shaded With Edges keeps
lit faces and real topology edges; Wireframe keeps topology edges without face
fill. Depth testing still occludes hidden geometry. Hover remains cyan;
selection uses the SET-01 Orange/Blue preference. A circular boundary fixture
checks that derived curved edges retain incident face identity.

## Model Browser and Inspector

The Browser shows `Part → Reference Geometry / Modeling Features /
Unconsumed Sketches`. Consumed Sketches stay nested beneath their first direct
consumer, without duplication or persistence changes. Rows have clearer
spacing, a selected-state tint and tooltips for editing, hidden, dirty,
failed and blocked states. Right-click synchronizes logical selection before
showing applicable actions; active Sketch display and deletion cannot be
changed while it is being edited.

The Inspector distinguishes a selected Feature, Face or Edge by name, surface
or curve type and owning Feature, with existing supported actions. A running
command takes precedence over unrelated selection facts. Derived topology
targets are checked against the current Solid cache and discarded if stale;
the Inspector does not dereference an invalid Face or Edge after rebuild,
delete or history restoration. Inspecting does not create a CAD transaction.

## Validation

- **V2, targeted affected closure:** Presentation, Viewer, Interaction, UI,
  Application and directly related FeatureModel/history/persistence contracts.
- Incremental Debug build of `micro_solidworks_tests` and `micro_solidworks`:
  **PASS**, with no project compiler warnings reported.
- Directed tests: pocket coplanar face/edge reproduction; curved boundary
  mapping; display-mode policy; picker; viewport highlight/depth state;
  Browser tree; topology context validity; editor selection/rebuild; feature
  history and document persistence. **61/61** Presentation/Viewer/Interaction
  tests and **43/43** FeatureModel/history/persistence tests passed. After the
  final UI adjustment, the directly affected Browser tests passed **10/10**.
- `git diff --check`: **PASS** (line-ending conversion notices only).
- Runtime GL/ImGui construction and shutdown occurred in the targeted tests;
  visible end-user behavior remains to be confirmed in the owner gate.
- Full CTest, clean configure/build and unrelated geometry suites intentionally
  omitted: no baseline freeze or toolchain/dependency change.

## Experience Gate — owner roteiro

Open a saved mechanical part with a blind pocket in the new Debug build.
Compare against UX-REF-01 using the same viewport fixture where possible:

1. In Model Browser inspect `Part`, the three reference planes, Modeling
   Features, nested consumed Sketches and Unconsumed Sketches. Select Extrude
   and Cut Extrude; confirm Browser tint, state tooltip and Inspector identity.
2. Select a **lateral exterior face at the pocket-floor height**. In Shaded,
   Shaded With Edges and Wireframe compare unselected, hover, selected and
   cleared states. The face remains visually continuous; actual pocket rim
   and floor boundaries stay legible, including under orbit. Try one pocket
   inner face and one real edge. Check that hover and selected are distinct.
3. Confirm the Inspector names the selected Face or Edge type and owning
   Feature, shows only supported actions, and exposes no internal IDs. Select
   a Feature in the Browser after a face: the viewport highlight and Inspector
   should move to the Feature together. Check right-click actions.
4. Edit an Extrude or Cut Extrude distance, Cancel once, then Confirm a valid
   change. Check Rebuild, Undo/Redo, Delete confirmation, Save/Open and return
   to the same selected context. Read-only inspection must not mark the file
   dirty or add an Undo step.
5. Repeat the selection comparison with Orange and Blue in Preferences;
   verify the default can be restored. If available, check 125%/150% Windows
   scaling as a separate visual observation. Close normally.

Record any first failing step, display mode and selected element. Real
screenshots may be stored in `docs/ux04_validation/`; canonical package images
must not be presented as executable evidence. **Do not mark the Experience
Gate approved until the owner reports APPROVED or APPROVED WITH CHANGES.**

## Architecture and remaining limits

CAD Solid and `FeatureModel` remain authoritative; Presentation and Viewer
data stay derived. `EditorContext` owns transient typed selection, and the
Application reconciles it after changed CAD state. No new external
dependency, persistent face identity, mesh-derived CAD edge or framework is
introduced. The UX-04.1 visual slice does not implement Browser search,
body folders, View Cube, new modeling commands, UX-03 Export UX or CAM.
