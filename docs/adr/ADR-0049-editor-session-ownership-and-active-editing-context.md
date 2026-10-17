# ADR-0049 - Editor Session Ownership and Active Editing Context

Status: PROPOSED

## Context

The frozen MVP has an authoritative `FeatureModel` and an Application-owned
persistence session, but historical one-shot Sketch, Solid and Boolean state
still makes post-MVP editor ownership ambiguous. Guided editing also needs an
active object that is distinct from selection.

## Decision

`FeatureModel` remains the sole authoritative CAD model. The Application-owned
session owns file/session state. Presentation remains derived. Editor
interaction state is transient and nonpersistent.

A minimal `EditorContext`, or an equivalent cohesive Application-owned value,
records the active `FeatureId`, the active Sketch feature while sketch editing,
the current tool/mode, logical selection and display context. Active editing
context and selection are independent: selecting an object does not implicitly
enter or leave edit mode.

`ApplicationShell` is a UI host and orchestrator. It may coordinate tools,
selection, session commands, Presentation refresh and panels, but it does not
own authoritative Sketch geometry, constraints, feature results, Solid
topology or persistent IDs. The first v1.1 campaign may retire demonstrably
redundant legacy/demo active Sketch, Solid and Boolean owners in favor of
`FeatureModel`, `EditorContext`, Presentation and the persistence session. This
is a focused cleanup, not an ApplicationShell rewrite.

Domain modules remain independent of Editor and UI. No new editor module is
required by this decision; one is justified only when cohesive reusable logic
outgrows the Application boundary.

## Rationale

One model source of truth prevents conflicting rebuild, persistence and visual
state. Explicit edit context makes tool lifetime predictable without turning
selection into hidden application state.

## Consequences

- Legacy parallel owners have a defined retirement direction.
- Editor state is reset or reconciled after New/Open and model replacement.
- The MVP persistence schema remains unchanged.
- A generic scene graph, Document aggregate and multi-document model are not
  introduced.

## Alternatives Considered

- Keep one-shot owners beside FeatureModel: rejected because they duplicate
  authoritative modeling state.
- Make selection the active edit object: rejected because inspection and edit
  mode have different lifetimes.
- Put editor state in FeatureModel: rejected because tools and session display
  are not CAD model truth.

## Deferred / Non-goals

Multi-document editing, persistent UI state, assemblies, configurations and a
general editor framework are deferred.
