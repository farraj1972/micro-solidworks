# ADR-0061 — Document Dirty State and Saved Baseline Semantics

Status: PROPOSED

## Context

V1.1-C3 must distinguish unsaved authoritative CAD edits from the existing
`FeatureState::Dirty`, which only means that a feature requires rebuild. Rebuild,
Presentation refresh and session display changes do not save user work.

## Decision

The Application-owned persistence session owns a two-state document condition:
`CLEAN` or `DIRTY`. `DocumentDirty` means that the current authoritative
`FeatureModel` checkpoint differs from the last successfully established saved
baseline. It is independent of every feature rebuild state.

A successful Open, Save or Save As marks the current authoritative history
checkpoint as the saved baseline. A successful New establishes a clean,
untitled, empty baseline. Every successful authoritative model transaction moves
to a new checkpoint and normally makes the document dirty. Rebuild alone never
changes the saved-baseline marker or clears document dirty.

The saved baseline is identified by the session history checkpoint identity,
not by serialized JSON, derived caches or a lossy hash. Undo or Redo that moves
the current position exactly to the saved checkpoint makes the document clean;
moving away makes it dirty. If a branch truncation or bounded-history eviction
makes the saved checkpoint unreachable, the document remains dirty until a
successful Save/Save As establishes a new baseline.

Save does not clear undo/redo history. Save As additionally updates the current
file path. Failed or cancelled Save/Save As changes neither file path nor saved
baseline. Document dirty and undo history are session-local and are not written
to `.microsw`.

## Rationale

Checkpoint identity gives deterministic dirty behavior without inventing model
equality or repeatedly serializing JSON. It also makes the common edit, Save,
edit, Undo sequence return naturally to the saved state.

## Consequences

- `DocumentDirty` and `FeatureState::Dirty` have separate names and owners.
- Window title and document commands can expose one reliable dirty marker.
- A failed rebuild after a committed edit remains dirty and undoable.
- The version 1 persistence schema remains unchanged.

## Alternatives Considered

- Feature rebuild state: rejected because rebuild and persistence have different
  meanings.
- Compare serialized JSON after every edit: rejected because persistence is not
  the in-memory history mechanism.
- Increment-only dirty flag: rejected because Undo could not return to clean.

## Deferred / Non-goals

Autosave, crash recovery, persistent history, content-addressed model storage,
multi-document dirty state and cloud synchronization are deferred.
