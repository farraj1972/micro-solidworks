# ADR-0066 — Persistence v2 and v1 Migration Semantics

Status: ACCEPTED

## Context

Semantic Sketch support and Cut Extrude add authoritative state and a feature kind. Reusing version 1 is misleading because an MVP v1 reader rejects unknown feature kinds. ADR-0046 remains the historical MVP contract.

## Decision

After C4, saves use `formatVersion: 2`. Persistence has versioned DTO paths: JSON v1 is parsed and validated as v1, migrated in memory and converted to FeatureModel; JSON v2 maps through a v2 DTO. Version conditionals do not enter domain code.

A v1 Sketch migrates to `ExplicitPlane` with its existing SketchPlane; no face relation is invented. Existing Extrude/Boolean semantics, IDs, counters, order and result identity remain. Open never rewrites the source. A later successful Save writes v2 silently. There is no v1 writer or downgrade. Versions above 2 return UNSUPPORTED_VERSION.

V2 Sketch persists `ExplicitPlane` (origin/axes) or `SemanticFace` (source FeatureId/cap role). V2 Cut Extrude persists ID, name, source Sketch, target result, distance and direction. Validation enforces backward dependencies, feature kinds, finite parameters and roles. Derived plane and face/edge/body/visual/topology IDs remain absent.

Roundtrip preserves support, cut parameters, target, IDs and counters. C3 checkpoints include the new values by value; history remains nonpersistent and D17 is unchanged. ADR-0066 supersedes ADR-0046's single-supported-version limitation only for post-MVP evolution.

## Rationale

An explicit version boundary states compatibility honestly and keeps migration outside domain code.

## Consequences

Persistence reads v1/v2 and writes v2. Save of an opened v1 document upgrades it only after successful atomic replacement. Migration and both DTO validators need tests.

## Alternatives Considered

Extending v1 is rejected because old readers reject Cut Extrude. Rewrite on Open and persistence of derived IDs are rejected.

## Deferred / Non-goals

Downgrade, versions beyond v2, persistent history, recovery, multiple documents and derived-state persistence.
