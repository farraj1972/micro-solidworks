# ADR-0042 - Feature Dependencies, Dirty State and Rebuild Ordering

Status: ACCEPTED

## Context

A Boolean consumes two prior results, so the domain is not a strict tree.
B12 still needs an ordered history UI, and B13 needs deterministic dependency
and rebuild behavior without a general scheduler.

## Decision

FeatureModel contains an authoritative ordered history plus an explicit
dependency DAG. A feature may reference only compatible, earlier FeatureIds.
This backward-only rule prevents cycles by construction and makes history order
a valid deterministic topological order while still permitting branches.

New features start `DIRTY`. The complete state set is:

- `CLEAN`;
- `DIRTY`;
- `FAILED`;
- `BLOCKED_BY_DEPENDENCY`.

A controlled edit validates the local parameter, preserves FeatureId, marks the
changed feature DIRTY and marks every transitive dependent DIRTY. Unrelated
features remain unchanged. Reverse dependencies may be derived by a
deterministic history scan; they need not be authoritative duplicated state.

Edits do not rebuild automatically. One explicit global Rebuild command
processes all dirty features in history order. CLEAN dependency caches may be
reused. A dirty feature runs only after every dependency has a valid candidate
or committed output. A dependent of a failed feature is not executed and is
reported BLOCKED_BY_DEPENDENCY.

Initial editable parameters are Sketch geometry, constraints and driving
dimensions; Extrude distance; and Boolean operation. Operand rewiring is
deferred after creation.

## Rationale

Backward references provide DAG semantics, branching and deterministic rebuild
without cycle detection or topological sorting machinery. Explicit rebuild
separates authoritative edits from computation and makes failure observable.

## Consequences

- The Feature Tree UI is an ordered history view over a DAG, not a claim that
  the domain is a strict tree.
- Dirty propagation covers every affected branch.
- Suppression and partial/local rebuild commands are not required.

## Alternatives Considered

- Strict parent-child tree: rejected because Boolean has two inputs.
- Arbitrary graph with cycle detection: deferred as unnecessary complexity.
- Automatic rebuild after every edit: rejected for the initial explicit and
  diagnosable workflow.

## Deferred / Non-goals

Forward dependencies, graph rewiring, reorder, suppression, partial rebuild,
parallel rebuild, lazy rebuild and advanced scheduling are deferred.
