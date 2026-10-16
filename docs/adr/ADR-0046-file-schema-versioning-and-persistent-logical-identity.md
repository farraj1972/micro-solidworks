# ADR-0046 - File Schema, Versioning and Persistent Logical Identity

Status: PROPOSED

## Context

B14 needs a human-readable, deterministic format that preserves logical model
identity without exposing C++ layouts or runtime proxy identities.

## Decision

Files use JSON, extension `.microsw`, and root `formatVersion: 1`, containing a
`featureModel` object with `nextFeatureId`, `resultFeatureId`, and history-ordered
`features`. Version values other than 1 return `UNSUPPORTED_VERSION`; B14 has no
migration framework. Unknown extra object members may be ignored, while missing
required members and unknown required type/enum tokens are rejected.

`FeatureId`, `SketchEntityId`, and `SketchConstraintId` are persistent within a
saved FeatureModel, stable across Save/Open, aggregate-local, and neither global
nor UUID-based. Their explicit next-ID counters are persisted and must exceed
every corresponding live ID without overflow. Tombstone records are omitted;
live records plus counters preserve the no-reuse rule.

Feature kinds, Boolean operations, constraint kinds, and sub-element kinds use
stable symbolic tokens, never C++ ordinals. Sketch entities use discriminated
`line`, `circle`, and `arc` records rather than variant layout. Doubles use
round-trip-safe precision equivalent to `max_digits10`; lengths are millimetres
and angles radians.

Load validates version, required fields/types, finite values, unique and valid
IDs, counters, backward dependencies, compatible feature kinds, result identity,
SketchPlane invariants through domain APIs, entity invariants, constraints and
references, and feature parameters. Defensive limits are centralized: 64 MiB
per file, 100,000 features, 250,000 entities per Sketch, and 250,000 constraints
per Sketch. Limits are checked before proportional allocation.

Output ordering is deterministic: feature history order, stable entity and
constraint order, and stable field order where supported.

B14 is authorized to integrate a pinned `nlohmann/json` release under its MIT
license, isolated inside `microsw_persistence`. No JSON types cross the module's
public boundary. The repository currently has no approved JSON implementation;
a custom generic parser has high correctness, Unicode, numeric, and maintenance
risk with little CAD learning value.

## Rationale

Symbolic, versioned records support readable diffs and stable contracts. Explicit
counters retain identity history even when removed objects are not serialized.
A mature parser is safer and smaller in project-owned code than building JSON.

## Consequences

- The JSON dependency must be reproducibly pinned and replaceable.
- Structurally invalid or excessive files are rejected as `INVALID_MODEL` or the
  appropriate parse/version error before replacing the current session.
- Topology IDs and `VisualEntityId` are deliberately absent.

## Alternatives Considered

- Project-owned minimal JSON parser: rejected because the required validation
  still demands a robust general lexical/parser foundation.
- Enum ordinals: rejected as compiler-layout-dependent.
- Recompute counters from maximum live IDs: rejected because it reuses IDs.

## Deferred / Non-goals

Schema migration, multiple supported versions, binary formats, compression,
encryption, and persistence of tombstones are deferred.
