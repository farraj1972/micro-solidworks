# ADR-0059 — Feature Manager Presentation Hierarchy and Absorbed Sketch Semantics

Status: ACCEPTED

## Context

`FeatureModel` stores authoritative ordered history and explicit dependencies.
Showing those records as a flat technical list exposes implementation structure
instead of the feature-first hierarchy expected from a parametric CAD editor.
The normal Feature Manager must remain human-readable without changing domain
ownership or duplicating an arbitrary dependency DAG.

## Decision

The normal Feature Manager is a derived presentation of `FeatureModel`, not the
authoritative history itself. Modeling features are primary tree nodes. A Sketch
used as the primary input of an Extrude is visually absorbed as that feature's
child. An unconsumed Sketch remains at the Part root. Absorption changes only
presentation hierarchy; the Sketch remains an independently owned FeatureModel
record with its existing identity and dependency semantics.

For a future Sketch referenced by more than one modeling feature, the first
direct consuming modeling feature in authoritative history order is the primary
absorbing consumer. Other consumers expose the dependency through their
properties/context without duplicating the Sketch node. This rule is
deterministic and does not imply ownership. Advanced shared-Sketch interaction
remains outside the corrective campaign.

Boolean features remain primary root-level feature nodes. Their operands are
shown in properties or context rather than copied below the Boolean, so the tree
describes ordered feature history without expanding the dependency DAG.

Normal labels use names such as `Sketch1`, `Extrude1` and `Boolean1`. Internal
FeatureId, SketchEntityId, SketchConstraintId, FaceId, EdgeId and runtime body
identity are diagnostics only. `CLEAN` has no routine label. Dirty, failed,
blocked, hidden and active/editing states use user-facing icons or status cues
with tooltips.

The normal tree supports expand/collapse, single-click selection, supported
double-click editing, context actions and bidirectional viewport selection
synchronization. A visual Part root is allowed. Origin and standard planes are
future UX direction until suitable logical reference objects exist; UI-only
stand-ins must not create domain objects. A future flat diagnostic view may show
raw FeatureModel history separately.

## Rationale

A derived hierarchy preserves the authoritative append-only model while making
the normal editor understandable to a CAD-literate user. One deterministic
absorbing parent prevents duplicated nodes and false aggregate ownership.

## Consequences

- Application/UI derives tree nodes from FeatureModel history and dependencies.
- FeatureId remains the synchronization key but is not displayed by default.
- Tree parenting never changes FeatureModel ownership or rebuild order.
- C2A must validate absorbed and unconsumed Sketch behavior through an
  Experience Gate.

## Alternatives Considered

- Display raw history records: rejected as technical and difficult to operate.
- Reparent Sketch records in FeatureModel: rejected because presentation must
  not change authoritative ownership.
- Duplicate shared Sketch nodes under every consumer: rejected because it makes
  identity and selection ambiguous.
- Keep every shared Sketch at root: deferred in favor of one deterministic
  primary absorber plus explicit secondary references.

## Deferred / Non-goals

Feature reorder, suppression, advanced shared-Sketch workflows, reference
geometry domain objects and a diagnostic flat-tree UI are deferred.
