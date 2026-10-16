# ADR-0043 - Transactional Rebuild, Last-Known-Good and Failure Semantics

Status: ACCEPTED

## Context

Editing an upstream parameter can affect multiple branches. B13 must expose the
attempted failure while preserving the last valid model and must never publish
partial candidate geometry.

## Decision

One Rebuild creates an atomic transaction over the complete affected dirty
closure. Candidate outputs are built in history order. If every affected
feature succeeds, all candidate caches commit together, their states become
CLEAN and diagnostics are cleared. If any feature fails, no candidate cache in
that closure commits.

Authoritative edited parameters remain current after failure. Every feature
retains its last successful derived cache. The failing feature becomes FAILED
with a short cause; dirty downstream dependents become
BLOCKED_BY_DEPENDENCY. Successful candidate work from the failed attempt is
discarded. Unrelated CLEAN features and caches are unchanged. If two affected
branches exist and either fails, neither branch commits candidate output.

SketchFeature rebuild copies its authoritative Sketch and invokes
ConstraintSolver on the copy. The source Sketch retains user edits and is not
mutated merely to produce a derived solution. A successful solved snapshot is
the candidate downstream input. ExtrudeFeature converts that snapshot through
the pure Sketch-to-Profile boundary and extrudes it. BooleanFeature reads two
candidate or committed Solid outputs without mutation.

The feature-layer build outcome distinguishes success with output,
`SUCCESS_EMPTY`, failure with a compact subsystem cause and
`UPSTREAM_BLOCKED`. It does not duplicate every subsystem status enum.
A successful empty Boolean commits an empty cache. Failure preserves the
previous cache; successful empty does not pretend the old Solid is current.

Build status and diagnostic record the latest attempt even when output caches
are rolled back. Same authoritative state, graph and solver policy must produce
the same result and statuses.

## Rationale

Closure-wide atomicity directly satisfies the no-partial-authoritative-rebuild
requirement and extends the transactional behavior proven in B9-B11.

## Consequences

- Last-known-good presentation may remain visible after failure and must be
  visibly identified as such.
- A dirty source Sketch may contain edited constraints while its cache contains
  the last solved snapshot; state makes this distinction explicit.
- Partial successful branch commit is intentionally deferred.

## Alternatives Considered

- Commit each feature as it succeeds: rejected because a downstream failure
  exposes a partially rebuilt history.
- Roll back authoritative parameter edits: rejected because it hides the edit
  the user needs to correct.
- Treat EMPTY as failure: rejected because B11 defines it as valid.

## Deferred / Non-goals

Partial branch commit, automatic retry, rollback history, progress reporting,
parallel evaluation and persistent caches are deferred.
