# V1.1-C4 — Subtractive Modeling & Sketch-on-Face Validation

## Current result

```text
STATUS: FIX REQUIRED
Technical Gate: PASS
Experience Gate: NOT APPROVED
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

The gate remains not approved because two requested behaviors exceed the frozen
contracts. D18 permits semantic Sketch support only on a single-body Extrude
StartCap/EndCap; it explicitly excludes arbitrary result faces and persistent
topological naming. D11 does not define transactional Feature deletion. The UI
now reports the Cut-face limitation explicitly instead of appearing
unresponsive. Supporting Sketch on a face produced by Cut Extrude and deleting
a Sketch Feature require an approved follow-up decision before implementation.

## Deliberate limits

- exact face-normal/axial Cut Extrude only;
- Blind end condition only;
- Sketch support only on single-body Extrude start/end caps;
- oblique Cut Extrude, conic topology, general exact Boolean, Through All and
  advanced end conditions remain deferred.
