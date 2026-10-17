# ADR-0063 — Unsaved Changes, New/Open/Exit and Session Safety

Status: ACCEPTED

## Context

New, Open and Exit can destroy unsaved authoritative work. C3 needs one
predictable policy that integrates dirty state, persistence failures and the
single-document Application session.

## Decision

When the document is dirty, New, Open and Exit first present `Save`,
`Don't Save` and `Cancel` for the current document. Clean documents proceed
without this prompt.

- Save attempts Save, or Save As for an untitled document. Only success permits
  the pending destructive action. Failure or cancellation preserves the current
  session and cancels the pending action.
- Don't Save explicitly authorizes the pending replacement or exit without
  writing current changes.
- Cancel performs no model, file-path, history or window-lifecycle change.

New replaces the session only after prompt resolution, clears history and creates
a clean untitled empty baseline. Open resolves unsaved work before requesting or
loading the target. The target is parsed, validated and rebuilt transactionally;
the current session remains intact until structural Open success. If target Open
fails, the current session remains present even after Don't Save because no
replacement occurred. A successful Open replaces model/path, clears history and
is clean; a structurally successful Open with rebuild failure follows the frozen
D12 result contract and establishes the loaded authoritative file as the clean
baseline while surfacing rebuild attention.

Exit closes only after prompt resolution and, when Save was chosen, successful
persistence. The native window close event enters exactly the same pending Exit
workflow and cannot bypass the dirty guard. Cancel or Save failure keeps the
application running. Successful
Save/Save As updates the saved baseline as defined by ADR-0061; Save As also
updates the current path. Save does not clear history.

The window title shows a human-readable file name, or an untitled Part name,
with a visible dirty marker. Undo and Redo appear in the Edit menu and a compact
General/Edit command group, are disabled when unavailable, and support `Ctrl+Z`
and `Ctrl+Y` on the current Windows-first application. Platform-standard
equivalents may be added without changing semantics.

Escape owns cancellation/unwinding of transient uncommitted tool state. Ctrl+Z
operates only on committed authoritative history; after restoration the
Application clears invalid hover and topology selection, clears or reconciles
tool operands and active command state, and retains only UI state that remains
valid. Face/edge targets clear whenever topology regenerates, as required by
D15.

## Rationale

One pending-action state machine prevents ad hoc prompt ordering and ensures a
failed persistence or Open operation cannot silently destroy the current model.
The policy matches familiar desktop CAD behavior without introducing multiple
documents or modal workflows for ordinary edits.

## Consequences

- Session replacement and exit become explicit Application transactions.
- Persistence remains transactional and schema version 1 remains sufficient.
- UI command availability derives from history state.
- C3 requires tests for every prompt outcome and failure path.

## Alternatives Considered

- Auto-discard on New/Open/Exit: rejected because it loses work.
- Prompt after choosing/loading the target: rejected because it complicates
  ownership and can obscure which document is being resolved.
- Persist editor/session state: rejected by D13 and unnecessary for safety.

## Deferred / Non-goals

Autosave, crash recovery, recent files, multi-document editing, persistent undo
history and cloud recovery are deferred.
