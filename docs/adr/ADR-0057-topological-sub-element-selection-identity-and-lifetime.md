# ADR-0057 — Topological Sub-element Selection Identity and Lifetime

Status: PROPOSED

## Context

ADR-0050 requires typed logical selection and forbids using VisualEntityId as
domain identity. C2 adds face and edge selection while current topology has no
persistent naming contract and one Feature may derive multiple Solids.

## Decision

The logical selection alternatives include `Feature`, `TopologyFace` and
`TopologyEdge`. A topological target contains its owning `FeatureId`, a
runtime-only derived body discriminator, and the FaceId or EdgeId valid in that
body's current Solid topology instance. Visual, triangle, polyline and GPU
indices never escape as logical selection.

FaceId and EdgeId are rebuild-local. Feature selection survives rebuild through
FeatureId. Any topology regeneration deterministically clears face or edge
selection before replacing the derived topology. No correspondence is inferred
between old and new indices. Persistent topological naming is explicitly not
claimed.

Without topology regeneration, a valid face or edge target survives orbit,
pan, zoom, viewport resize and Presentation regeneration that refers to the
same Solid topology instance. Deletion, hiding where policy removes the target,
or loss of resolvability clears it deterministically.

C2 may keep a single logical selection. Selecting a face or edge preserves its
owning FeatureId for Feature Manager synchronization and owner highlighting,
but the target remains the selected sub-element. Empty Graphics Area clicks
clear selection unless an active tool defines a different transaction.

## Rationale

Explicit rebuild-local identity provides honest, stable interaction inside one
derived result without creating false persistent naming semantics.

## Consequences

- Multi-body context is always part of a face or edge target.
- Feature Manager may show the owning feature while inspectors retain the exact
  face or edge target.
- Rebuild clearing must be atomic with topology replacement.
- The target model remains extensible to future selection sets.

## Alternatives Considered

- Select by VisualEntityId or mesh index: rejected because both are derived.
- Preserve indices across rebuild: rejected because equality would be accidental.
- Introduce persistent BodyId/topological naming: rejected as outside C2.

## Deferred / Non-goals

Persistent naming, selection persistence across rebuild, general multi-select,
face-based Sketch creation and modeling commands from sub-elements are deferred.
