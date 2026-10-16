# ADR-0041 - Feature Model Ownership, Identity and History

Status: ACCEPTED

## Context

B12 and B13 need an in-memory history that can represent Sketch, Extrude and
Boolean features without introducing the B14 Document, file or persistence
lifecycle. Application-owned active results from B10 and B11 do not provide
stable feature identity or dependencies.

## Decision

A project-owned `microsw_features` module provides the central in-memory
`FeatureModel` aggregate. It owns feature records, FeatureId allocation,
dependency references, build states, last-successful derived caches and an
explicit result FeatureId. It coordinates dirty propagation and rebuild.

`FeatureId` is strongly typed, FeatureModel-local, stable for the feature
lifetime, nonpersistent in B12/B13, never reused and distinct from Sketch,
constraint, topology and visual IDs. Its invalid/default value is explicit.

The initial type-safe feature value is a variant of:

- `SketchFeature`, owning a Sketch and its SketchPlane;
- `ExtrudeFeature`, owning a prior Sketch FeatureId and positive distance;
- `BooleanFeature`, owning two prior Solid-producing FeatureIds and a
  BooleanOperation.

Each record also has a deterministic display name, kind, state, short
diagnostic and type-appropriate derived cache. Names are presentation metadata,
not identity or dependency keys.

History is append-only insertion order. Reordering, insertion in the middle,
suppression and removal are deferred. Feature parameters, dependencies, source
Sketch state and result FeatureId are authoritative. Solved Sketch snapshots,
Profiles, Solids, Boolean results and presentation geometry are derived.

## Rationale

One aggregate gives identity and dependency validation a single authority while
remaining structurally serializable later. A variant keeps the first feature
set explicit and educational without a dynamic plugin framework.

## Consequences

- FeatureModel supersedes Application ownership of the active modeled result.
- B10/B11 one-shot computation APIs remain reusable.
- IDs are suitable for later serialization, but B14 must define persistence.
- The feature history is distinct from undo/redo history.

## Alternatives Considered

- Call the aggregate Document: rejected because file lifecycle belongs to B14.
- Keep results in Application: rejected because rebuild identity and caches
  would have no domain owner.
- Polymorphic feature hierarchy: deferred because three closed kinds suffice.

## Deferred / Non-goals

Persistence, persistent IDs, file lifecycle, undo/redo, reorder, suppression,
removal, dependency rewiring, additional feature kinds and generic properties
are deferred.
