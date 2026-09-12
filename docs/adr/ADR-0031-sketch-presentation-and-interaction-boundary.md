# ADR-0031 — Sketch Presentation and Interaction Boundary

Status: PROPOSED

## Context

B8 is visual and interactive, but its local 2D values and logical identity must
remain independent of Presentation, Viewer, Rendering and UI infrastructure.

## Decision

The core dependency is:

```text
microsw_sketch -> microsw_geometry -> microsw_math
```

Sketch core has no dependency on Topology, Presentation, Viewer, Rendering, UI
or Application. Adapters and Application may depend on Sketch. SketchLine is not
a Topology Edge, and a closed Sketch profile is not a Wire or Face. B8 performs
no automatic Topology conversion; future modeling owns that conversion.

Presentation derives world geometry from local Sketch values plus SketchPlane.
SketchLine becomes a world segment. Circle2 and Arc2 are tessellated into
view-appropriate world polylines outside Geometry and Sketch. Tessellated points
are transient derived data and are never stored as authoritative curve geometry.

Rendering and picking consume the same derived line/polyline representation for
each frame or regeneration. Visual resolution is Viewer-specific. Picking uses
the displayed segments and retains entity-level identity, satisfying “what the
user sees is what the user can pick” without circular intersection APIs.

Authoritative interaction identity in Sketch mode is SketchEntityId.
VisualEntityId remains a presentation-local proxy. The active Sketch adapter
maintains a one-to-one mapping between them for each surviving entity; assigning
a proxy once and retaining it across regeneration makes the mapping deterministic
during that active presentation lifetime. Removal retires the proxy. Hover and
selection resolve proxy hits back to SketchEntityId, so geometry replacement or
retessellation does not change logical selection.

B8 interaction includes one active Sketch, Line/Rectangle/Circle/Arc creation,
single-entity hover/selection, atomic basic editing and delete. Existing viewport,
input gating, highlight and single-selection behavior should be reused through
the adapter. Grid/endpoint snapping and general scene lifecycle are not required.

B8 stores no constraint state. Its stable SketchEntityId and value replacement
allow B9 to associate constraints with entities and later-defined sub-elements
or parameters, without predesigning a solver.

## Rationale

An adapter preserves the domain boundary while reusing the proven interaction
pipeline. A shared tessellation for draw and pick prevents visible and pickable
curves from diverging.

## Consequences

- Curve tessellation quality can evolve without changing Sketch files or IDs.
- Presentation regeneration preserves logical selection.
- Presentation may depend on Sketch, while the reverse dependency is forbidden.
- Topology remains independent and unchanged.

## Alternatives Considered

- Store world or tessellated geometry in Sketch: rejected as duplicate derived
  state.
- Reuse VisualEntityId as domain identity: rejected because presentation
  lifetimes differ from Sketch lifetimes.
- Require analytic curve picking immediately: not selected; shared tessellation
  is sufficient for the MVP.

## Deferred / Non-goals

Snapping, constraints, dimensions, solver, topology conversion, extrusion,
Document/persistence, multi-sketch lifecycle and topology-specific presentation
are deferred.
