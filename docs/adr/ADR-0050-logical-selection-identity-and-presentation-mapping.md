# ADR-0050 - Logical Selection Identity and Presentation Mapping

Status: ACCEPTED

## Context

The MVP has stable model-local feature, sketch entity and constraint identities,
plus Presentation-local visual identity. Guided sketching and synchronized tree
and viewport selection must not use visual IDs or row indices as domain identity.

## Decision

Selection is a typed logical target. Its extensible alternatives include a
Feature, Sketch entity, Sketch sub-element, Sketch constraint and future
Topology face or edge. `FeatureId`, `SketchEntityId`, `SketchConstraintId`,
topological identity and `VisualEntityId` remain distinct and are never
silently substituted.

Sketch sub-elements use `SketchEntityId` plus the existing stable
`SketchElementRef` semantics where applicable, including line start/end,
circle center and future supported arc endpoints. Interaction and Constraints
must not introduce an equivalent parallel reference system.

Presentation owns an explicit mapping between logical targets and
`VisualEntityId` or other picking proxies. `VisualEntityId` remains
Presentation-local, session-local, derived and nonpersistent. Picking resolves
through that mapping and returns a logical target to Application interaction.

Feature Tree selection uses `FeatureId`. Picking a modeled feature result
resolves to its `FeatureId`; selecting either representation synchronizes the
other. While a Sketch is active, picking its geometry resolves to the
appropriate Sketch entity or sub-element.

The Feature Manager presents human-readable display names such as `Sketch1`,
`Extrude1` and `Boolean1`; IDs may appear only in diagnostics. It is the main
visual history and supports selection, highlight, status, visibility and
context actions. The Feature Manager and Graphics Area are two synchronized
views of the same logical model.

Hover resolves through the same logical mapping and pre-highlights what a click
would select without changing authoritative state. The consistent visual-state
vocabulary is normal, hover, selected, active/editing, context/reference,
disabled/hidden and error/failed. Exact colors are Presentation/UI detail.

Logical selection survives Presentation regeneration, successful feature
rebuild, Sketch geometry replacement, viewport resize and navigation while its
target remains valid. It clears deterministically when the target is deleted or
is no longer resolvable.

Sketch sub-element lifetime is defined by `SketchEntityId` and stable element
kind. Topology IDs remain rebuild-local under D11 and cannot provide persistent
face/edge selection across rebuild. Persistent topological naming is not solved
by D13, and face/edge selection is excluded from V1.1-C1.

## Rationale

Logical identity keeps UI workflows stable when visual proxies and topology are
regenerated and allows selection to grow without another identity redesign.

## Consequences

- Numeric IDs may be displayed for diagnostics but are never required normal
  workflow input.
- Picking adapters must expose explicit logical mappings.
- Constraint glyphs, when present, map to `SketchConstraintId`.
- A later face/edge campaign requires a separate topological naming decision.

## Alternatives Considered

- Store only `VisualEntityId`: rejected because it is derived and session-local.
- Use Feature Tree row indices: rejected because ordering is presentation state.
- Preserve raw topology IDs across rebuild: rejected because their accepted
  lifetime is rebuild-local.

## Deferred / Non-goals

General multi-selection/selection sets, persistent selection, face/edge
selection and persistent topological naming are deferred. A small transient set
of logical operands required by one active constraint command remains allowed.
