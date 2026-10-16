# ADR-0047 - Transactional Save/Open and Rebuild Semantics

Status: PROPOSED

## Context

Persistence must protect both the current session and an existing destination
file while allowing users to save authoritative edits that do not rebuild.

## Decision

Save is allowed for CLEAN, DIRTY, FAILED, and BLOCKED models and serializes only
authoritative state. It maps FeatureModel to DTO, serializes completely, writes
a temporary file in the destination directory, flushes and closes it, then uses
`std::filesystem` to replace the destination atomically under the supported
Windows workflow. A failure before replacement leaves the original intact.
Save results distinguish at least `SUCCESS`, `IO_ERROR`,
`SERIALIZATION_ERROR`, and `INVALID_PATH`.

`Save As(path)` changes `currentFilePath` only after success. `Save()` requires
a session path and otherwise returns a clear state for the UI to invoke Save As.

Open reads and parses a temporary DTO, validates and reconstructs a temporary
FeatureModel, then runs a deterministic rebuild because caches are not stored.
Parse, schema, or domain reconstruction failure leaves the current session
unchanged. Once structural load succeeds, the temporary model replaces the
session even when rebuild fails. Results distinguish at least
`SUCCESS_REBUILT`, `SUCCESS_REBUILD_FAILED`, `IO_ERROR`, `PARSE_ERROR`,
`UNSUPPORTED_VERSION`, and `INVALID_MODEL`.

Successful structural Open replaces FeatureModel, sets the path, derives
Presentation from any successful rebuild result, and clears selection, hover,
and transient tool state. Failed rebuild has no persisted last-known-good
viewport cache. New installs an empty FeatureModel and clears path,
Presentation, selection, and transient tools. B14 adds no unsaved-work prompt.

Save/Open semantic roundtrip preserves persistent logical IDs and counters,
kinds, names, parameters, dependencies, result identity, Sketch geometry, and
constraints. Rebuild must produce the same semantic modeling result; topology
IDs, visual IDs, cache addresses, and runtime state need not match.

## Rationale

Structural validity and current rebuildability are separate. Transactional
replacement prevents corrupt input or failed writes from destroying valid work.

## Consequences

- Users can reopen and repair models saved in a failed rebuild state.
- Open rebuild failure is a successful loaded-model condition, not file damage.
- Application coordinates session replacement and transient state clearing.

## Alternatives Considered

- Save only CLEAN models: rejected because failed authoritative edits are work.
- Treat rebuild failure as Open failure: rejected because it blocks repair.
- Write directly to destination: rejected because partial writes lose files.

## Deferred / Non-goals

Autosave, recovery files, recent files, unsaved-change prompts, locking,
concurrent editing, and cloud synchronization are deferred.
