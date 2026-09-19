# ADR-0034 — Solver Convergence, Transaction and Failure Semantics

Status: PROPOSED

## Context

A numeric solve must not partially corrupt authoritative Sketch geometry. Math
comparison tolerance, Geometry modelling tolerance and solver convergence have
different meanings and cannot be reused interchangeably.

## Decision

Solver policy centrally owns separate finite, positive settings for
`stepTolerance`, `costReductionTolerance`,
`residualSatisfactionTolerance`, `finiteDifferenceStep`, `maxIterations` and
Levenberg–Marquardt damping. Initial normalized defaults are:

```text
stepTolerance                 = 1e-10
costReductionTolerance        = 1e-12
residualSatisfactionTolerance = 1e-8
finiteDifferenceStep          = sqrt(machine epsilon) times parameter scale
maxIterations                 = 100
initial damping               = 1e-3
damping adjustment factor     = 10
initial-state regularization  = 1e-8
```

These are solver defaults, not Math or Geometry constants. They may be tuned
with focused numerical evidence without changing the ownership and transaction
architecture. Convergence alone is insufficient: the final normalized
constraint residual must also satisfy `residualSatisfactionTolerance`.

The public result is:

```text
SOLVED
UNSATISFIED
INVALID_INPUT
NUMERICAL_FAILURE
```

An optional `likelyUnderConstrained` diagnostic may use an estimated Jacobian
rank. B9 does not promise complete DOF classification and does not formally
distinguish over-constrained from conflicting systems.

Every solve follows one transaction:

```text
snapshot current Sketch geometry
-> build temporary parameters
-> solve temporary state
-> reconstruct every candidate Geometry value
-> validate all candidates
-> verify all constraint residuals
-> atomically commit all replacements
```

Sketch provides a controlled multi-entity replacement API that validates all
IDs, entity kinds and geometry values before changing any record, then commits
all or none. It does not expose arbitrary mutable geometry.

For `UNSATISFIED`, `INVALID_INPUT` or `NUMERICAL_FAILURE`, authoritative geometry
is unchanged. `SketchEntityId`, `SketchConstraintId`, selection and references
remain unchanged. Successful replacement preserves entity IDs and kinds;
Presentation regenerates from Sketch and retains its stable identity mapping.
The solver has no Presentation, Viewer or UI dependency.

The primary acceptance fixture is four independent lines with four endpoint
coincidences, two horizontal constraints, two vertical constraints and driving
horizontal and vertical distances. Editing either dimension must keep the
rectangle closed and satisfy all relations while regularization determines the
least displacement from the initial state. A second focused fixture changes a
circle radius while preserving its center.

The minimal UI exposes a constraint list and type, add/remove operations,
editable driving values, and solve status/diagnostics. Sophisticated glyphs and
constraint picking are deferred.

## Rationale

Separate solver tolerances make convergence meaningful, while candidate-state
solving and atomic replacement preserve the established valid-on-construction
Sketch model. Explicit results avoid claiming diagnoses the first solver cannot
reliably make.

## Consequences

- A failed solve is observationally non-mutating for authoritative geometry.
- B9 requires a multi-entity Sketch transaction API.
- Presentation refresh happens only after a successful commit.
- Deterministic fixtures can validate both constrained and under-constrained
  behavior.

## Alternatives Considered

- Mutate geometry during iterations: rejected because failure could corrupt it.
- Reuse `1e-12` Math or `1e-9 mm` Geometry tolerance: rejected because solver
  criteria are normalized algorithmic thresholds.
- Report formal over-constraint categories immediately: rejected because the
  initial numeric solver cannot guarantee that classification.

## Deferred / Non-goals

Formal DOF visualization, complete over/constrained classification, advanced
glyphs, constraint picking, undo/redo, persistence, Document, feature rebuild
and future modeling phases are deferred.
