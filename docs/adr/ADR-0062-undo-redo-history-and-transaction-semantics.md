# ADR-0062 — Undo/Redo History and Transaction Semantics

Status: ACCEPTED

## Context

D13 already requires one atomic authoritative edit per meaningful user action.
Sketch creation, solver-related edits and dependency rebuilds make inverse
commands difficult to implement and verify correctly in the current model.

## Decision

V1.1-C3 uses bounded, linear, session-local authoritative checkpoints at D13
transaction boundaries. A checkpoint contains the complete authoritative
`FeatureModel` state needed to restore history order, feature data, dependencies,
result identity, `FeatureId`, `SketchEntityId`, `SketchConstraintId`, tombstone
positions and every next-ID counter. It excludes rebuild states, diagnostics,
solved Sketches, topology, Solids, Presentation data and other derived caches.

Features exposes or supports a project-owned typed checkpoint capture/restore
boundary. C3 must not serialize `FeatureModel` through JSON for each operation,
and must not make Persistence or UI types dependencies of Features. Restoring a
checkpoint is followed by deterministic rebuild and Presentation refresh.

One successful logical user transaction creates one Undo step. This includes a
Line, Circle or complete Rectangle creation; accepted inference and constraints
created by that same gesture; one explicit constraint; one confirmed dimension
edit; one confirmed Extrude parameter edit; and each feature creation or future
deletion transaction. Rectangle remains one step despite creating multiple
entities and constraints. Preview, partial Property Manager input and Cancel
create no entry.

An authoritative edit that commits but later fails rebuild remains the current
history entry. Undo restores the preceding authoritative checkpoint and rebuilds
it without relying on stale Presentation or last-known-good caches. Redo restores
the succeeding checkpoint equivalently. After Undo, a new committed edit
discards the Redo branch.

The initial policy retains at most 100 undoable transactions plus the current
checkpoint. Oldest undoable checkpoints are evicted first; this count is an
adjustable implementation policy, not file format. The saved authoritative
checkpoint is retained separately when its history entry leaves this window, so
capacity eviction cannot corrupt dirty-state evaluation. New and successful
Open clear the previous document history and establish a new initial checkpoint.
Save and Save As retain history.

Restoring a historical checkpoint restores its exact next-ID counters. A new
edit on a branch may therefore allocate an ID that existed only in the discarded
future branch. IDs are never reused inside the active linear authoritative
history; mutually exclusive discarded branches do not require global identity.

Camera, hover, selection, display mode, panels, Fit, standard views and D13
visibility/display policy are transient session state and do not participate in
model Undo/Redo. Reconciliation after restore clears invalid topology targets
and refreshes model-derived UI state.

## Rationale

The model is small, copyable by value and already has atomic transaction
boundaries. Typed authoritative checkpoints prioritize correctness and make
multi-entity and solver edits reversible without a large inverse-command
framework. Excluding caches preserves D11's authoritative/derived boundary.

## Consequences

- Undo/Redo is deterministic and compatible with failed rebuild recovery.
- History memory is bounded and no schema v2 is needed.
- Feature and Sketch identity is restored exactly for each historical state.
- Undo history is lost on application restart.

## Alternatives Considered

- Inverse commands: rejected for C3 because compound Sketch/constraint and
  dependency edits have greater inversion risk.
- Hybrid commands plus checkpoints: deferred until model size or performance
  provides evidence that snapshots are inadequate.
- Unlimited snapshots: rejected because session memory must be bounded.
- JSON round trips: rejected as slower, filesystem-format-coupled and unnecessary.

## Deferred / Non-goals

Branching undo trees, persistent history, macros, history compaction by memory
estimate, collaborative history and generic command frameworks are deferred.
