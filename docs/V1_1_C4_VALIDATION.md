# V1.1-C4 — Subtractive Modeling & Sketch-on-Face Validation

## Current result

```text
STATUS: FIX REQUIRED
Technical Gate: PASS
Experience Gate: NOT APPROVED
D20: FROZEN
```

V1.1-C4 implements the frozen D18, D18-R1 and D19 contracts. A Sketch may
authoritatively reference the start or end cap of a single-body Extrude. Rebuild
derives its plane from that semantic reference, preserves local 2D geometry and
orients the Viewer Normal To the resolved plane without persisting topology IDs.

`CutExtrudeFeature` owns the source Sketch, target result Feature, positive Blind
depth and Forward/Reverse direction. The exact kernel deliberately uses the
authorized axial decomposition:

```text
exact 2D region subtraction
+ axial interval decomposition
+ analytic boundary reconstruction
```

The planar kernel splits and classifies Segment2, Arc2 and Circle2 boundaries,
reconstructs nested material/void loops and preserves circular arcs. The axial
kernel emits Plane/Cylinder faces over shared Segment3/CircularArc3 edges,
removes material-to-material interfaces, validates connected closed 2-manifold
shells and returns zero to many Solids. It covers blind pockets and holes,
Reverse Direction, concave and holed cutting regions, multiple cutter regions,
full removal, split results and multi-body targets with unchanged non-intersected
bodies. Oblique cuts and non-recoverable targets fail explicitly.

The CAD workflow exposes Sketch on an Extrude cap, automatic Normal To, Cut in
the Command Bar, inferred human-readable Profile/Target, Depth, Reverse
Direction, Confirm and Cancel. The Feature Manager presents `CutExtrude1` with
its absorbed `Sketch2`. Creating or editing a Cut is one document-history
transaction; upstream Extrude changes rebuild the supported Sketch and Cut.

Persistence writes `.microsw` format version 2. Version 1 Sketch planes migrate
to ExplicitPlane support, opening alone does not rewrite the file, and the next
Save writes version 2. Semantic face support and Cut Extrude fields roundtrip;
derived topology, body/face identity and editor state remain unpersisted.

## Automated validation

V1 targeted kernel validation passed exact planar and axial scenarios. V2 passed
70/70 directly affected FeatureModel, persistence, history, Presentation,
Feature Manager and interaction tests. Additional axial tests cover concavity,
cutter voids, multiple regions and partial multi-body intersection.

The single final V3 used a fresh build directory:

```text
fresh configure: PASS
full Debug build: PASS
project warnings: 0
full CTest evidence: 879/879 PASS in aggregate
runtime OpenGL tests: PASS
architecture/dependency audit: PASS
git diff --check: PASS
```

The first complete CTest pass exposed three D19 navigation expectations: an old
test still required unwrapped yaw after forward/up became authoritative, and
local orbit lacked the established pole clamp. The implementation and historical
test were corrected. The 876 unaffected tests had passed; all 19 affected orbit
and OpenGL integration tests then passed, including the three original failures.
Unaffected tests were not repeated, following impact-based validation policy.

## Architecture audit

```text
ADR-0064: CONFORMANT
ADR-0065 + D18-R1: CONFORMANT
ADR-0066: CONFORMANT
ADR-0067: CONFORMANT
```

CAD topology remains authoritative and rendering remains derived. The project
adds no external dependency, mesh Boolean, generic B-rep Boolean, persistent
body/face identity or reverse dependency. The implemented dependency direction
remains Features → Modeling/Topology/Geometry/Math, Persistence → Features and
Application/UI → Features/Interaction/Presentation.

## Experience Gate

Manual observation found that entering a face-supported Sketch retained the old
topology selection while the UI had already selected the new Sketch. The next
PropertyManager frame could consequently interpret a Sketch cache as a Solid
cache and terminate the application. The correction clears derived topology
selection at the edit-domain transition, selects the new logical Sketch and
resolves topology inspection from the target's actual owning Feature. A focused
regression test covers the transition.

The follow-up UX correction exposes Front, Top and Right principal planes in the
FeatureManager with `Sketch on Plane` context actions, renames the left tabs to
FeatureManager and PropertyManager, adds a `Views` control and Space shortcut
for the six standard orientations, and makes the Graphics Area context menu
selection-sensitive. Quantitative Presentation coverage confirms that a cap
containing a circular opening retains its complete expected tessellated area.

A second manual review found that an unqualified Sketch command still fell back
to the global XY plane, Extrude Cancel retained the active command, workflow
mode was not prominent and MMB orbit felt inverted to a SolidWorks user. The
fallback was removed: Sketch now requires an explicit principal plane,
supported face or existing Sketch, and Normal To is applied only after that
choice. Cancel returns to Select without committing an Extrude. The Graphics
Area and status bar continuously identify Part editing, Sketch editing and
Extrude/Cut parameter modes. MMB orbit now follows the pointer in the familiar
direct-manipulation direction. The affected interaction and OpenGL navigation
set passed 28/28 tests.

The next manual review exposed a separate projection defect: pointer-to-world
conversion was still hard-coded to global XY while Front or Right sketch
editing was active. The Viewer now intersects the picking ray with the active
SketchPlane, and creation, preview and logical sub-element selection all use
that same plane. Focused SketchPlane and OpenGL Workspace coverage passed 19/19,
including all three principal orientations.

Manual corrective smoke: PASS. The owner confirmed the corrected principal-plane
selection and orientation, active-plane sketch projection, Extrude cancellation,
workflow-state cues and direct-manipulation MMB orbit behavior. The executable
state was unchanged after this observation, so build and automated tests were
not repeated.

The gate remains not approved because two requested behaviors exceed the frozen
contracts. D18 permits semantic Sketch support only on a single-body Extrude
StartCap/EndCap; it explicitly excludes arbitrary result faces and persistent
topological naming. D11 does not define transactional Feature deletion. The UI
now reports the Cut-face limitation explicitly instead of appearing
unresponsive. Supporting Sketch on a face produced by Cut Extrude and deleting
a Sketch Feature require an approved follow-up decision before implementation.

D20 freezes the two missing contracts in accepted ADR-0068–0069: atomic recursive
dependant-closure deletion and narrow generated semantic support for unambiguous
Cut caps plus a single-region, single-body pocket floor. No executable D20 work
has started. V1.1-C4 remains FIX REQUIRED / NEXT ACTIVE and still requires D20
implementation, Experience Gate approval and a final V3 against executable HEAD.

The post-D20 Experience Gate adds these mandatory scenarios:

- delete a leaf Sketch, then Undo and Redo with exact identity restoration;
- delete a Sketch with a Cut Extrude dependant after a dialog names the complete
  deletion closure, then Undo the whole operation in one step;
- verify immediate FeatureManager, selection, active-tool and result updates;
- create a block and blind single-region pocket, select its pocket floor, start
  a Sketch, draw geometry, edit Cut depth and confirm associative movement;
- make the semantic floor disappear or ambiguous and confirm a human-readable
  failure without topology IDs or automatic reattachment.

## Deliberate limits

- exact face-normal/axial Cut Extrude only;
- Blind end condition only;
- Sketch support only on single-body Extrude start/end caps;
- oblique Cut Extrude, conic topology, general exact Boolean, Through All and
  advanced end conditions remain deferred.
