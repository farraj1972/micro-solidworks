# ADR-0055 — General Profiles, Multiple Solids and Extrusion Transactions

Status: ACCEPTED

## Context

SOLIDWORKS-compatible sketch eligibility permits multiple closed and nested
contours. The current `Profile` stores one convex point loop and `extrude`
returns one Solid, so it cannot represent the required input or result.

## Decision

The Modeling input is a value containing an oriented SketchPlane and one or
more selected planar regions. Each region owns one outer loop and zero or more
inner loops of curve uses. Loop nesting and orientation are derived and
validated; Sketch IDs and constraints do not enter Modeling.

Positive Blind extrusion is transactional:

1. extract and validate all selected regions;
2. build exact candidate topology for caps and side surfaces;
3. group disjoint material regions into deterministic result bodies;
4. validate every candidate Shell/Solid;
5. atomically commit the complete feature result.

Failure commits no partial body and preserves last-known-good feature caches.
Feature identity remains stable. Derived topology and visual identities may be
regenerated according to the existing FeatureModel and Presentation contracts.

The feature result may contain multiple solids. Automatic merging with an
existing body is a later Boolean/feature-scope decision and is not inferred.

## Rationale

Regions are the semantic object selected by CAD users. Keeping them ephemeral
and value-based preserves the Sketch-to-Modeling boundary while supporting
disjoint contours and holes without importing editor identity into topology.

## Consequences

- `Profile` and `SolidFeatureCache` require compatible generalization.
- Feature presentation must show all bodies produced by one Extrude feature.
- Persistence schema evolution is required for any new authoritative feature
  parameters, curves or surfaces; derived caches remain unpersisted.
- Existing single rectangular extrusion remains a compatible special case.

## Alternatives Considered

- Create one unrelated Extrude feature per contour: rejected because one user
  command and one feature must rebuild transactionally.
- Boolean-combine tessellated prisms: rejected because it loses exact curved
  topology and changes feature semantics.
- Reject holes and multiple contours: rejected by the SOLIDWORKS-compatible
  eligibility requirement.

## Deferred / Non-goals

Merge Result, Feature Scope, cut extrusion, Thin Feature, surface extrusion,
negative/two-sided depth and non-Blind end conditions remain deferred.

## Final Normative Alignment

ADR-0035–0037 remain historically correct for the MVP. Generalized work uses
`PlanarRegionSet`, `PlanarProfileSet` or an equivalent honest name supporting
multiple material regions, concavity, holes and straight/circular boundaries.
An L-shaped region is valid; convex decomposition may be an implementation aid
but is never authoritative profile state.

Standard Extrude remains linear, finite, greater than modeling tolerance and
along `+SketchPlane.normal`. Its structured result contains status, zero or more
Solids and diagnostics. Successful nonempty results contain at least one Solid.
Disconnected material regions create independent bodies without implicit merge.
Inner boundaries create through-holes in their material region; nesting parity
may make an island inside a hole an independent Solid.

The operation validates all requested regions and candidate topology before
atomic commit. One failing region means no partial authoritative result. Status
coverage includes SUCCESS, INVALID_PROFILE, UNSUPPORTED_GEOMETRY,
AMBIGUOUS_PROFILE, DEGENERATE_PROFILE, TOPOLOGY_FAILURE and NUMERICAL_FAILURE,
with Sketch entity diagnostics. D11 last-known-good and failure semantics remain.

Extrude derived caches may contain multiple Solids while one `FeatureId`
continues to identify the logical feature. Derived bodies receive no persistent
FeatureId. Presentation may derive multiple visual proxies resolving to that
feature; persistent body identity requires a new decision.

B11 Boolean limitations remain unchanged, and curved results can be rejected as
unsupported operands. Persistence format version 1 remains sufficient while
only source Sketch FeatureId and distance are authoritative and full-sketch
regions are derived. D14 does not authorize format version 2; an implementation
need for schema evolution or persisted contour selection is a hard stop for a
new persistence decision.
