# V1.1-C1 — Reconciled Execution Plan after D14

Status: READY FOR EXECUTION

## Authority and boundary

This plan reconciles the active CAD Workspace & Guided Sketching campaign with
frozen D13 (ADR-0049–0052) and D14 (ADR-0053–0055). MVP 1.0 remains frozen.
General Boolean, Thin Feature, Selected Contours UI, persistent region/body or
subshape naming, splines, ellipses, NURBS, surface/cut extrusion, revolve,
implicit body merge and persistence format changes remain excluded.

The implementation must stop if it needs to change a frozen ADR, persist region
or body identity, change `.microsw` formatVersion, add a generic/external
geometry kernel, generalize Boolean, or cannot robustly satisfy the required
intersection classifications.

## Preserved-work reconciliation

The repository is clean at D14 freeze `a56b15b`. Prior C1 work is preserved in
`stash@{0}` and was inspected without applying or dropping it.

### Reusable as-is

- user-facing `Model` and `Edit` tab language;
- `Model history` and task-oriented supporting text;
- the underlying D13 CAD workspace, Command Bar, EditorContext, logical
  selection, snapping/inference, constraints, dimensions and visibility work
  already committed in C1.1–C1.5.

### Reusable with adaptation

- guided Sketch questions and progressive actions;
- the focused Extrude page with source, depth, validation and Create/Cancel;
- projected circle preview state and world-to-screen projection. This remains
  transient interaction data; future authoritative curve tessellation belongs
  to Presentation/Rendering under ADR-0054;
- status and error text, adapted to structured region diagnostics and affected
  Sketch identities.

### Obsolete after D14

- eligibility inferred from entity count;
- `supportsExtrusionProfile()` as a UI-local line-only predicate;
- assuming one valid Profile or one Solid result;
- generic `closed shape` guidance that does not distinguish open, branched,
  intersecting or ambiguous geometry.

### Conflicts with D14

- disabling Circle Extrude because it is not line geometry;
- `closed outlines made from lines` and `Circle extrusion is not available`
  messages;
- making UI independently decide eligibility instead of consuming the shared
  `PlanarRegionSet` extraction result;
- any polygonal B-rep approximation of Circle/Arc.

The D14 documentation copies inside the stash are superseded by the committed
freeze and must not be restored. Apply the stash only after C1.8–C1.11 expose
the replacement eligibility/result contracts. Resolve in favor of D14, commit
recovered work by coherent increment, and retain the stash until equivalent
work is committed.

## Coordinated streams

Stream A preserves and finishes the CAD UX: workspace, Command Bar, Model/Edit
panels, EditorContext, logical selection, mouse Sketch tools, snapping,
inference, contextual constraints, glyphs, dimensions and display lifecycle.

Stream B supplies only the D14 prerequisites required by the C1 Extrude flow:
the GAP-GEO-002 intersection subset, planar regions, exact circular topology,
general/multiple-body Extrude and derived Presentation.

Stream A may progress independently through C1.7. C1.12 cannot complete until
C1.8–C1.11 provide the shared structured validation and result contracts.

## Increment sequence

### C1.1–C1.7 — Audit and finish the existing CAD UX

Audit committed C1.1–C1.5 rather than reimplementing it. Complete Feature
Manager/viewport synchronization, mouse-first tools, snapping/inference,
contextual constraints/glyphs and graphical dimension editing. Restore only
the D14-neutral stash pieces. Use V1 for pure interaction units and V2 for
workspace-visible changes. Normal workflows accept zero technical IDs.

### C1.8 — Geometry Intersection Foundation (V1)

Add focused structured results for segment/segment, segment/circle,
segment/arc, circle/circle, circle/arc and arc/arc. Classify none, proper,
endpoint, tangent, overlap/coincident and ambiguous/unsupported; include points
and parameters only where consumers require them. Reuse established tolerance.
Do not build a generic computational-geometry framework.

Required tests cover segment crossing/touching; segment-circle 0/1/2;
circle-circle disjoint/tangent/two-point/coincident; arc parameter filtering;
arc-arc classification; and relevant tolerance boundaries.

### C1.9 — Planar Region Extraction (V1)

Derive nonpersistent `PlanarRegionSet` from the solved Sketch. Build exact
Line/Arc/Circle loops; reject open chains, branches, degeneracy,
self-intersection, overlap and ambiguous touching with structured diagnostics
referencing Sketch identities. Establish deterministic containment and nesting:
even depth material, odd depth hole. Normalize boundary orientation after
containment. No persistent region ID or Selected Contours UI.

Tests cover rectangle, circle, mixed arc/line loop, concave L, rectangle with
circular hole, two disjoint regions, outer/hole/island, open, branch, bow-tie,
overlap and tangent ambiguity.

### C1.10 — Exact Curved Topology Extension (V1)

Add the controlled LineSegment/CircularArc Edge and Plane/Cylinder Face support,
deterministic two-semicircle full circles, planar Face inner Wires and validation
for exact connected oriented curved loops. Extend Shell/Solid structural checks
for nonconvexity and nonzero genus while preserving closed connected 2-manifold
and opposite edge-use invariants. Existing polyhedral topology remains a valid
special case.

Tests cover circle topology, shared semicircle vertices, arc orientation,
cylindrical support, annular caps, through-hole genus and curved oriented uses.

### C1.11 — General Multi-Solid Extrusion (V2)

Introduce structured `ExtrudeResult`/body-set semantics. Extrude concave regions,
holes, circles/arcs, nested islands and multiple disjoint material regions.
Build all candidate bodies and commit only after every region and Solid passes.
One failure yields no partial result. Preserve D11 last-known-good behavior.

Evolve only the derived Extrude feature cache to one-or-more Solids under one
FeatureId. Keep formatVersion 1 when source Sketch FeatureId and distance remain
the only authoritative parameters. Return structured unsupported status when a
general/curved result reaches the restricted B11 Boolean subset.

Tests cover concavity, hole, circle/arc, two disjoint solids, nested island,
transaction failure, last-known-good preservation and unsupported Boolean.

### C1.12 — Property Manager Extrude Integration (V2)

Apply and adapt the guided UI stash after C1.9/C1.11 exist. The Property Manager
shows source Sketch, distance, semantic validation, detected region/body count
and user-facing diagnostics. It consumes all valid material regions. It never
asks for region/body/wire/face IDs. Invalid geometry highlights affected Sketch
entities when diagnostics provide them. Open profiles explicitly recommend the
deferred Thin Feature behavior. Circle Extrude must be available.

### C1.13 — Display Lifecycle and UX Cleanup (V2)

Integrate derived tessellation for CircularArc, Cylinder and planar caps with
holes through Presentation/Rendering; keep exact CAD state authoritative.
Picking resolves derived curved Sketch graphics to Sketch identity and result
graphics to FeatureId. Finish visibility, active-edit and context-reference
states. Remove technical IDs and stale line-only language from normal UX.

### C1.14 — Technical and Experience Validation

Run one V3 after all executable commits: fresh configure, full Debug build,
zero project warnings, full CTest, runtime workflows, architecture/dependency
audit, persistence-v1 regression, hygiene and `git diff --check`.

Runtime covers Rectangle plus constraints/dimension/Extrude, concave Extrude,
rectangle with circular hole, Circle curved Extrude and two disjoint regions,
all without IDs. Then run the Experience Gate across discoverability,
mouse-first flow, command/panel coherence, selection, snapping/inference,
constraints, dimensions, Extrude, invalid-profile diagnostics, multi-region
predictability and visual hierarchy.

Freeze only with Technical PASS and Experience APPROVED or acceptable APPROVED
WITH CHANGES. `REWORK REQUIRED` prevents freeze. The documentary freeze commit
is `[V1.1-C1] Freeze CAD Workspace and Guided Sketching` and tag is
`v1.1-c1-cad-workspace-guided-sketching`; do not repeat V3 if executable HEAD
is unchanged.

## Gap and conformance outcomes

Close GAP-GEO-002 only if all D14-required classifications are implemented and
validated; otherwise mark it PARTIALLY CLOSED with the exact remainder.
GAP-SCENE-002 receives executable partial closure for active Sketch, visibility,
context result and deterministic editing lifecycle; a general scene graph stays
deferred. Final audit requires ADR-0049–0055 CONFORMANT.

## Documentation at closure

Create `docs/V1_1_C1_VALIDATION.md` and synchronize AGENTS, BOOTSTRAP,
ARCHITECTURE, DEPENDENCY_POLICY, ROADMAP and materially relevant README content.
Record stash disposition, D14 implementation, both gaps, Technical Gate,
Experience Gate and deferred capabilities.

## Closure

Status: COMPLETE / FROZEN.

The sequence above was completed. The final Technical Gate passed with a fresh
full Debug build, zero project warnings and 822/822 tests. The manual Experience
Gate, including projection-consistent Line, Rectangle and Circle previews,
passed with normal application shutdown. GAP-GEO-002 is CLOSED;
GAP-SCENE-002 is PARTIALLY CLOSED by the executable C1 lifecycle slice.
