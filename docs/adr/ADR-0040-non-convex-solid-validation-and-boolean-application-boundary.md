# ADR-0040 — Non-Convex Solid Validation and Boolean Application Boundary

Status: PROPOSED

## Context

B7 validates outward orientation using one interior reference point, which is a
convex-Solid assumption. Union and Difference of supported boxes can produce a
connected non-convex orthogonal Solid while every individual Face remains a
convex planar rectangle.

## Decision

Solid validity is distinct from Solid convexity. B11 may produce one connected,
closed, outward-oriented, 2-manifold non-convex Shell while retaining all other
ADR-0025–0027 contracts: one aggregate-owned root Shell, topology-local typed
IDs, shared incidence, straight Edges, planar convex Faces, one outer Wire per
Face, no holes and no in-place mutation.

Structural manifold validation is separated from convex-specific outward
orientation validation. Existing B7 builders and convex fixtures retain their
interior-point validation. For a cell-derived Boolean result, each boundary
rectangle receives its outward direction from the occupied cell on its interior
side and the empty/outside cell on its exterior side. Reconstruction preserves
that orientation and final validation confirms closed incidence, opposite
shared-edge traversal and agreement between every effective FaceUse normal and
the derived boundary direction. A single centroid or interior point is not used
to infer orientation for the non-convex result.

This is the only authorized B7 contract extension: convexity becomes a builder
or algorithm precondition rather than a universal Solid invariant. Ownership,
identity, incidence and Face representation are unchanged.

Until Document and Feature Tree exist, Application owns operand A, operand B
and an optional successful Boolean result as values or snapshots. Boolean
execution is one-shot and transactional:

```text
validate operands
-> classify candidate cells
-> reject unsupported result topology
-> reconstruct and validate candidate Solid
-> commit Application result
```

`INVALID_INPUT`, `UNSUPPORTED_CASE` and `NUMERICAL_FAILURE` preserve the
previous successful result. `EMPTY` commits a logical empty result. Presentation
reuses the B10 Solid wireframe, solid-level picking, selection and highlight;
there is no Boolean-specific renderer or scene graph.

## Rationale

Cell-derived normals provide direct local evidence for outward orientation and
avoid applying a convex proof to non-convex topology. Temporary Application
ownership makes the slice observable without introducing persistent CAD or
Document semantics.

## Consequences

- B7 convex behavior and regression fixtures remain valid.
- B11 needs two operand slots plus a result, but no general object collection.
- Future BooleanFeature identity and rebuild dependencies remain B12/B13 work.
- GAP-SCENE-002 remains OPEN because temporary operand/result ownership is not
  a general visibility or lifecycle model.

## Alternatives Considered

- Keep centroid-based validation for all Solids: rejected because it is not a
  valid general proof for non-convex shells.
- Allow non-convex Face wires or holes: rejected because rectangular Face
  decomposition preserves the existing Face contract.
- Introduce Document or a scene graph: rejected as premature.

## Deferred / Non-goals

Inner wires, cavities, through-holes, multiple components, multiple shells,
general non-convex shell classification, face/edge sub-selection, feature
identity/history, automatic rebuild, Document and persistence are deferred.
