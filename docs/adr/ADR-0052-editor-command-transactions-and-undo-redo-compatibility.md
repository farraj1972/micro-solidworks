# ADR-0052 - Editor Command Transactions and Undo/Redo Compatibility

Status: ACCEPTED

## Context

Guided interaction adds multi-step tools and previews. UI code must not leak
partial mutation into the authoritative model or make later undo/redo
impractical.

## Decision

Each meaningful user action is one atomic model edit. The Application boundary
coordinates `UI action -> command/application operation -> validated domain
operation -> committed model edit`. Domain APIs remain type-safe, but UI panels
do not spread ad hoc authoritative mutations.

Creating or replacing Sketch geometry, adding a constraint, editing a driving
value, changing extrusion distance and changing a Boolean operation are each
logical transaction boundaries. Rebuild occurs after a committed edit, never
during partial input or preview. Failed validation leaves authoritative state
unchanged.

The first campaign need not introduce a generic command framework. Its design
must preserve atomic operations, stable logical identity, absence of hidden
mutation outside the transaction boundary and compatibility with either future
snapshot or inverse-command undo/redo. D13 does not choose between those
strategies.

Snapping and inference belong to the interaction/tool layer. They inspect the
active Sketch, SketchPlane and screen/world mapping without mutating the Sketch
until commit. Temporary inference is a preview/suggestion, not solver truth.
Acceptance may create an explicit supported constraint as part of the same
atomic edit.

Constraint creation follows logical element selection, compatibility
validation and one committed constraint addition. Constraint glyphs are
derived Presentation/UI state mapped to `SketchConstraintId`. Solver results
retain their accepted `SOLVED`, `UNSATISFIED`, `INVALID_INPUT` and
`NUMERICAL_FAILURE` semantics, with likely under-constrained guidance where the
solver can support it, and are translated into comprehensible UI diagnostics.
Contextual tool guidance is transient UI state.

Multi-step tools expose explicit Confirm and Cancel semantics. Confirm, or an
equivalent Enter action, commits exactly one validated model edit. Cancel, or
Escape, discards all candidates; repeated Escape may exit the active tool and
return to Select. No partial mutation survives cancellation.

Mouse-first defaults are left-click for selection, point picking and tool-step
acceptance; right-click for a relevant context menu or compact context toolbar;
wheel for the existing compatible zoom behavior; and the frozen middle-button
Viewer conventions for orbit/pan. A relevant selection may expose only
semantically valid contextual commands. Command availability derives from
active Sketch, logical selection and domain validity so incompatible flows are
disabled before entry where practical.

The Property/Tool Manager owns command parameters, current selections,
validation, Confirm and Cancel. Property-oriented workflows are preferred over
sequences of modal dialogs. Constraint glyphs and driving dimensions are
graphical derived objects: glyphs map to `SketchConstraintId`, while selecting
or safely double-clicking a dimension opens numeric editing and commits through
the same transaction boundary. Active tools provide cursor/status/hint feedback.

The architecture permits future right-drag radial gestures and safe Sketch
dragging, but V1.1-C1 implements neither constrained drag solving nor gestures.

## Rationale

Explicit transaction boundaries prevent partial edits and establish the
minimum contract required for later undo/redo without over-designing it now.

## Consequences

- Preview state remains outside authoritative domain state.
- Accepted inference can create an explicit constraint; suggestion alone cannot.
- V1.1-C1 implements compatibility with undo/redo, not undo/redo itself.
- Existing domain validation and solver result contracts remain authoritative.

## Alternatives Considered

- Mutate on every widget/input event: rejected because partial states can leak
  into rebuild and persistence.
- Make inference implicit solver truth: rejected because it hides model intent.
- Select an undo strategy now: deferred because neither strategy is required to
  define safe first-campaign edits.

## Deferred / Non-goals

Undo/redo implementation, command history persistence, macros, general
transaction framework, mouse gestures, constrained direct drag and new solver
semantics are deferred.
