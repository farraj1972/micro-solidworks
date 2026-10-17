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
records the active `FeatureId` and Sketch, active command/tool and tool step,
hover logical target, logical selection target/set, temporary snapping and
inference candidates, display state and Property/Tool Manager editing context.
All are transient. Active editing context and selection are independent:
selecting an object does not implicitly enter or leave edit mode.

The primary interaction surface is the Graphics Area and the mouse is the
primary pointing device. Command discovery is through context-sensitive icon
tools; the Feature Manager is the primary view of model structure; the
Property/Tool Manager is the primary parameter and multi-step tool editor.
Internal logical or numeric IDs are diagnostic only and normal CAD operations
must not require their knowledge or manual entry.

The conceptual workspace contains compact application commands, a CAD Command
Bar with logical Sketch, Features, View and later Evaluate/Inspect groups, a
left manager area switching between Feature Manager and Property/Tool Manager,
a dominant Graphics Area, contextual tools and status hints. Exact layout and
styling remain adjustable. Familiar mechanical-CAD interaction patterns may be
used, but Micro SolidWorks retains its own visual identity: no third-party
trademarks, proprietary artwork/icons or pixel-perfect reproduction is copied.

`ApplicationShell` is a UI host and orchestrator. It may coordinate tools,
selection, session commands, Presentation refresh and panels, but it does not
own authoritative Sketch geometry, constraints, feature results, Solid
topology or persistent IDs. The first v1.1 campaign may retire demonstrably
redundant legacy/demo active Sketch, Solid and Boolean owners in favor of
`FeatureModel`, `EditorContext`, Presentation and the persistence session. This
is a focused cleanup, not an ApplicationShell rewrite.

V1.1-C1 may reorganize `ApplicationShell` around Command Bar, manager views,
Graphics Area, context actions and status feedback. Cohesive controllers or
views may be extracted when responsibility is clear, avoiding further
monolithic growth without introducing a framework rewrite.

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
- The Graphics Area receives workspace priority; controls remain compact and
  CAD-oriented rather than dashboard/card-oriented.

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
