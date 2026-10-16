# ADR-0044 - Feature Result Ownership and Presentation Identity

Status: PROPOSED

## Context

B10/B11 Application owners and SolidPresentation provide stable visual identity
for one-shot results. Parametric rebuild needs the feature domain to own outputs
while preserving selection when regenerated topology receives new local IDs.

## Decision

FeatureModel owns all last-successful derived feature caches and an explicit
`resultFeatureId`. The result reference must identify a Solid-producing
feature or a Boolean feature whose valid output may be empty. Appending a
Solid-producing feature makes it the result by default. A trailing Sketch does
not implicitly replace the result.

Application observes FeatureModel rather than owning the authoritative active
Solid or Boolean result. Presentation maintains a stable
`FeatureId -> VisualEntityId` mapping for modeled results. Topology IDs remain
aggregate-local and may change on every rebuild.

Presentation updates only after a successful affected-closure commit. A
successful Solid replaces geometry under the existing visual mapping and keeps
FeatureId-based selection. A failed rebuild retains last-known-good geometry
and selection while the Feature Tree marks failure. A successful empty final
result removes viewport Solid geometry, keeps the feature selectable in the
tree and reports an empty model.

The minimal Feature Tree shows ordered rows with FeatureId, deterministic name,
kind, state, dependencies and diagnostic. Selecting a feature opens its typed
parameter editor. A global Rebuild action reports success, failed feature and
blocked count, or empty final result.

The pure Sketch-to-Profile conversion is extracted from the current mixed
Application adapter into a core integration boundary usable by
`microsw_features`. It owns no Application or Presentation state and has no
UI, Viewer or Rendering dependency. Feature core depends on Constraints,
Sketch, this pure adapter and Modeling; infrastructure depends on Feature core,
never the reverse.

B12 is an internal checkpoint commit with no V3 or tag. After B13 completes,
one V3 validates the combined campaign. One V0 freeze marks both baselines
FROZEN and creates `b12-feature-tree` and
`b13-parametric-rebuild` at the same final freeze commit.

## Rationale

FeatureId is the stable logical in-memory identity available across topology
regeneration. Delaying presentation until commit prevents candidate leakage.

## Consequences

- Existing B10/B11 computational services remain unchanged and reusable.
- Application-specific active owners may be retired only as needed.
- GAP-SCENE-002 remains open: feature output lifecycle is not general scene
  visibility or object lifecycle.
- B14 can later serialize structured feature state but no file schema is set.

## Alternatives Considered

- Use topology IDs for selection: rejected because rebuild regenerates them.
- Infer the result from the final history row: rejected because it may be a
  Sketch and branches are valid.
- Tag B12 at its checkpoint: rejected because rebuild is not yet complete.

## Deferred / Non-goals

General scene lifecycle, visibility, persistence, Document/file lifecycle,
multi-document, assemblies, configurations, reference geometry and additional
feature kinds are deferred.
