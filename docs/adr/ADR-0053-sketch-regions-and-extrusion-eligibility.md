# ADR-0053 — Sketch Regions and Extrusion Eligibility

Status: ACCEPTED

## Context

ADR-0035 limits extrusion input to one strictly convex loop of `SketchLine`
entities. The product requirement now adopts SOLIDWORKS-compatible eligibility
for a normal solid Boss/Base extrusion. A disabled command based on entity count
cannot express those rules or diagnose an invalid sketch.

## Decision

Extrusion eligibility is derived from geometric regions, never from the number
or type of Sketch entities. A region extractor consumes the solved live Sketch
and its SketchPlane and returns deterministic closed planar regions or a
diagnostic identifying the first invalid location.

For a normal solid Boss/Base extrusion:

- one or more disjoint closed contours are valid;
- nested closed contours define alternating material and void regions;
- line, arc and circle entities participate in contours;
- endpoint connections use the project geometric tolerance;
- construction geometry does not contribute to regions;
- dangling geometry may coexist only when explicit contour selection removes
  it from the feature input;
- branches, ambiguous traversal, zero-length entities, duplicate overlapping
  entities and self-intersections are invalid;
- an open contour is not a normal solid profile. It remains reserved for a
  future Thin Feature or surface operation;
- touching or intersecting contours that do not form unambiguous regions are
  invalid.

The whole-sketch default selects all valid closed regions. A future selected
contours value may select a deterministic subset without changing Sketch
ownership. Eligibility and diagnostics are pure and have no UI dependency.

## Rationale

These rules match the observable SOLIDWORKS Boss/Base contour model while
keeping the CAD representation authoritative. They replace UI guesses with one
shared domain result used for command availability, preview and rebuild.

## Consequences

- Circle is a valid closed contour by itself.
- Multiple disjoint contours may create multiple bodies until body merging is
  explicitly implemented.
- Nested contours require cap faces with inner loops.
- General intersection support becomes a prerequisite and closes the relevant
  part of GAP-GEO-002.
- Invalid sketches receive stable diagnostics suitable for highlighting.

## Alternatives Considered

- Keep line-only extrusion: rejected by the product requirement.
- Enable Extrude and fail after confirmation: rejected because availability
  must reflect semantic eligibility.
- Tessellate first and infer regions from render polylines: rejected because
  rendering approximations are not authoritative CAD geometry.

## Deferred / Non-goals

Thin Feature, surface extrude, cut extrude, draft, nonplanar sketches and
SOLIDWORKS end conditions other than positive Blind depth remain separate
increments.

## Final Normative Alignment

ADR-0035–0037 remain the correct historical record of the single, line-only,
strictly-convex, single-Solid MVP slice. ADR-0053–0055 extend and supersede
those initial-slice limitations for authorized post-MVP general-profile work.

`Sketch` remains authoritative for entities and constraints. `PlanarRegionSet`
and its `PlanarRegion` values are derived, recomputable and nonpersistent.
Each material region has one outer boundary and zero or more inner boundaries.
The exact first boundary subset is `SketchLine`, `SketchArc` and `SketchCircle`;
splines, ellipses, Bezier/NURBS and generic curves are excluded.

Containment is evaluated before orientation. Even nesting depth is material and
odd depth is void. Derived outer material boundaries are normalized CCW along
`SketchPlane.normal`; inner hole boundaries are CW. Multiple disjoint depth-0
loops produce independent material regions. A depth-2 island is material again.

Full-sketch extraction rejects branches, self-intersections, degeneracy,
ambiguous tangent or zero-area contacts, overlaps, coincident arcs, duplicate
boundaries and near-coincident geometry that cannot be robustly classified.
Diagnostics contain a problem kind plus affected `SketchEntityId` and, where
meaningful, `SketchElementRef` values. Open contours are ineligible for standard
solid Extrude and report that Thin Feature is required; Thin Feature is deferred.

Selected Contours remains future-compatible but its UI is deferred. Discovery
order is not stable region identity, regions receive no persistent identity,
and stable contour naming requires a separate decision.
