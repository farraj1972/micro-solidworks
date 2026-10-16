# ADR-0045 - Persistence Boundary and Authoritative State

Status: PROPOSED

## Context

B14 must preserve the parametric model frozen by D11 without making filesystem,
JSON, presentation, or runtime rebuild state authoritative domain concepts.

## Decision

`FeatureModel` is the persisted aggregate and remains the authoritative in-memory
CAD model. B14 does not introduce a runtime `Document` abstraction.

Persistence records ordered feature history, `FeatureId`, feature kind, display
name, authoritative parameters, dependencies, `resultFeatureId`, and
`nextFeatureId`. A Sketch feature additionally records its authoritative
`SketchPlane` origin/X/Y axes, live entities and their exact IDs and geometry,
live constraints and their exact IDs, symbolic types, element references and
driving values, plus both next-ID counters.

Feature state, diagnostics, solved sketches, profiles, solids, Boolean results,
last-known-good caches, topology and visual IDs, presentation geometry,
selection, hover, camera, tools, runtime candidates, and current file path are
not persisted. They are derived or session-local.

The dependency direction is `microsw_persistence -> microsw_features`, with
transitive domain dependencies required for mapping. Features never depends on
Persistence. Application depends on both. Persistence separates FeatureModel to
DTO mapping, DTO to JSON mapping, and JSON to filesystem I/O within the same
module. It returns structured errors; Application maps them to UI messages.

Application/session owns `optional<filesystem::path> currentFilePath`; neither
FeatureModel nor hidden Persistence global state owns it.

## Rationale

Persisting only authoritative inputs lets deterministic rebuild restore derived
outputs and preserves the existing CAD-over-render architecture.

## Consequences

- A saved model may be dirty, failed, or blocked and remain valid user work.
- Topology and visual identity may change after Open.
- Persistence is replaceable and domain modules remain filesystem-independent.

## Alternatives Considered

- Persist derived caches: rejected because they can become stale and duplicate
  authoritative state.
- Introduce Document now: rejected because B14 needs no additional aggregate.
- Put file paths in FeatureModel: rejected as session infrastructure leakage.

## Deferred / Non-goals

Document abstraction, multi-document UI, persistent UI state, selection
persistence, external references, and assemblies are deferred.
