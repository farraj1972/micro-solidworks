# ADR-0068 — Feature Deletion, Dependency Closure and History Semantics

Status: ACCEPTED

## Context

The CAD Feature Manager must delete Sketch, Extrude, Cut Extrude and Boolean
features without leaving dangling dependencies. D11 freezes ordered backward
dependencies and D17 freezes exact authoritative checkpoints, but neither
defines forward deletion. Suppression, hiding and deletion are distinct.

## Decision

Delete computes the selected FeatureId plus its complete transitive dependant
closure from the exact FeatureModel dependency graph. The authoritative change
is atomic: every member is removed or the model is unchanged. A leaf is deleted
directly and remains recoverable with Undo. A non-leaf deletion requires an
explicit confirmation listing every affected feature by display name; accepting
deletes the complete closure. There is no silent cascade, suppression or
partially dangling history.

Normal forward deletion never reuses FeatureIds. `nextFeatureId` remains
unchanged and deleted IDs remain retired. Sketch entity and constraint IDs need
no reassignment. Undo restores the exact pre-delete checkpoint, including all
features, IDs, parameters, dependency references, order, next-ID counter and
resultFeatureId. Redo reapplies the same closure as one logical transaction.

After deletion and deterministic rebuild, `resultFeatureId` becomes the latest
surviving modeled-result feature in history with a valid rebuilt result. If no
Extrude, Cut Extrude or Boolean result survives successfully, it becomes
invalid and the modeled result is empty. No prior cache is selected arbitrarily.

Before commit, Application/Editor integration exits an active deleted Sketch,
cancels its tool/property state and clears hover, selection and derived viewport
selection owned by the closure. Reference Geometry and the Part root are not
FeatureModel records and cannot be deleted.

Persistence writes only surviving features. No deletion tombstone is persisted;
the advanced `nextFeatureId` counter preserves non-reuse. Session history remains
nonpersistent under D17.

## Rationale

Recursive dependant deletion matches normal CAD intent and preserves a coherent
graph. Mandatory confirmation only for a non-leaf closure exposes consequences
without confirmation fatigue for an undoable leaf deletion. Existing checkpoint
semantics already provide exact transactional Undo/Redo.

## Consequences

- FeatureModel needs a dependency-closure query and atomic deletion operation.
- The Feature Manager adds Delete and a named dependant confirmation dialog.
- Deleting a support producer also deletes supported Sketches and their dependants.
- Deleting a face-supported Sketch deletes its Cut Extrude dependant.
- Tests cover all feature kinds, closure, result reassignment, Editor cleanup,
  Undo/Redo and persistence omission.

## Alternatives Considered

Leaf-only deletion is simpler but unnecessarily restrictive. Rejecting non-leaf
deletion makes dependency cleanup manual. Silent recursive deletion hides
destructive scope. Selective dependant retention can create invalid graphs.

## Deferred / Non-goals

Suppression, feature reorder, generic document-object deletion, selective
dependant retention, persistent history, recycle bins and soft deletion.
