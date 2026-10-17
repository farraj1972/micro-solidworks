# ADR-0051 - Scene Visibility and Display Lifecycle

Status: PROPOSED

## Context

`GAP-SCENE-002` records the absence of explicit visibility and object display
lifecycle. v1.1 sketch editing needs deterministic context display without a
generic scene graph or a new CAD ownership model.

## Decision

Visibility is transient editor/session display state keyed by logical feature
or sketch identity. It is not Geometry, Topology or authoritative FeatureModel
state and is not persisted in v1.1. The minimal states are visible, hidden,
active/editing and reference/context; styling remains a derived UI/Presentation
choice.

The active Sketch is visible and emphasized while editing. Its modeled result
is visible as context and may be dimmed. Other Sketches are hidden by default.
Feature results follow deterministic editor display policy, and explicit user
visibility overrides may be held in the session. Selection does not determine
visibility or the active edit object.

Presentation derives visual proxies from the authoritative model plus this
display context. New/Open and invalidated logical targets reconcile display
state deterministically. No scene graph, parent/child transform hierarchy or
general object lifecycle service is introduced.

This decision closes the product-relevant core of `GAP-SCENE-002`: explicit
feature/sketch visibility, active editing context and deterministic display
lifecycle. General scene graph and broader object lifecycle capabilities remain
outside the gap's closure claim.

## Rationale

Display policy belongs with editing and session concerns. Keeping it out of CAD
geometry preserves authoritative model semantics and avoids a premature scene
architecture.

## Consequences

- The persistence schema and FeatureModel remain unchanged.
- Feature and Sketch logical identity key display state.
- V1.1-C1 may implement only the states needed by guided sketch editing.

## Alternatives Considered

- Persist visibility in FeatureModel: rejected for v1.1 because it is currently
  editor/session policy rather than modeling truth.
- Add a generic scene graph: rejected as unnecessary scope.
- Infer visibility from selection: rejected because selection is transient and
  semantically independent.

## Deferred / Non-goals

Persistent visibility, assemblies, configurations, layers, scene hierarchy and
general object lifecycle are deferred.
