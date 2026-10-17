# V1.1-C1 — CAD Workspace & Guided Sketching Validation

## Scope and conformance

V1.1-C1 implements the frozen D13 and D14 contracts without changing
ADR-0049–0055. The authoritative model remains `FeatureModel`; regions,
topology caches, body collections and tessellation remain derived. Persistence
continues to use `.microsw` format version 1. No external geometry dependency
was introduced. ADR-0049–0055 are CONFORMANT.

## Delivered behavior

- Resizable CAD workspace with project-owned icon command bar and Model/Edit
  managers, logical mouse selection, hover, context actions and no manual IDs.
- Guided Line/Rectangle/Circle sketching with projection-consistent live previews,
  endpoint/midpoint/grid snapping, horizontal/vertical/coincident inference,
  constraints, driving dimensions and deterministic Cancel/Escape behavior.
- Structured intersections for every D14 Line/Arc/Circle pair and planar-region
  extraction with concavity, holes, nesting, disjoint regions and localized
  invalid-profile diagnostics.
- Exact circular topology, Plane/Cylinder face support, planar inner wires and
  closed shells with through holes. Presentation tessellation remains derived.
- Atomic general extrusion producing one Solid per material region under one
  FeatureId. Curved and multi-body inputs return an explicit unsupported
  diagnostic at the restricted B11 Boolean boundary.
- Active Sketch emphasis, other model geometry hidden during edit, Show/Hide
  actions and deterministic edit/display transitions.

GAP-GEO-002 is CLOSED. GAP-SCENE-002 is PARTIALLY CLOSED — EXECUTABLE V1.1-C1
SLICE; a general scene graph and persistent visibility remain deferred.

## Validation cadence

V1 targeted tests covered intersections, regions and curved topology. V2
covered general extrusion, FeatureModel, Presentation, EditorContext, guided
workflow, Viewer projection and persistence. The final V3 used a fresh
`build-v1.1-c1` configuration.

Technical Gate evidence:

- fresh CMake configure: PASS (initial sandbox network denial retried with
  authorized network access);
- full Debug build: PASS, zero project warnings;
- CTest: 822/822 PASS;
- persistence v1 generalized-Extrude roundtrip: PASS;
- architecture/dependency audit: PASS;
- `git diff --check`: PASS before documentary closure;
- runtime launch and normal shutdown: PASS;
- manual functional workflow and Experience Gate observation: PASS.

## Experience Gate

Status: PASS / APPROVED.

The gate covered workspace coherence, discoverability, mouse-first interaction,
command bar, Model/Edit managers, Sketch creation, snapping/inference,
constraints, dimension editing, general Extrude, invalid-profile diagnostics,
multiple-region predictability and error/recovery clarity. The owner confirmed
the complete workflow, including projection-consistent Line, Rectangle and
Circle previews, general closed-profile extrusion, interaction recovery and
normal application shutdown.

## Freeze

Technical Gate: PASS. Experience Gate: PASS. V1.1-C1 is FROZEN over the
validated executable state with 822/822 tests passing and zero project warnings.

## Stash reconciliation

The preserved C1 stash was reviewed selectively. CAD labels/guidance, projected
circle preview and viewport projection were reused. The line-only eligibility
predicate and obsolete D14 documentation were replaced by the frozen D14
contracts. After all unique useful work was committed, the stash was dropped.

## Deferred scope

Thin Feature, Selected Contours UI, persistent body/region/subshape identity,
splines, ellipses, NURBS, revolve, cut extrude, general Boolean, fillet,
chamfer, shell, draft, undo/redo and a general scene graph remain deferred.
