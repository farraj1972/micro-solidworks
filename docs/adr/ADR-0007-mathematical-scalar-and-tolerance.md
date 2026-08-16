# ADR-0007 — Mathematical Scalar and Tolerance

Status: ACCEPTED

## Context

The mathematical foundation requires explicit scalar, comparison and failure
conventions before numeric utilities and vector types are implemented.

## Decision

The primary CAD/math scalar is `double`. Rendering may use `float` only at
explicit adaptation boundaries; the Math core must not adopt `float` merely
for OpenGL convenience.

Floating-point comparisons use both absolute and relative tolerance:

```text
|a - b| <= max(
    absoluteTolerance,
    relativeTolerance * max(|a|, |b|)
)
```

Concrete epsilon values are not frozen by this ADR. B1.1 may define them while
preserving this policy.

Numerical comparison tolerance and geometric modelling tolerance are distinct.
Geometric modelling tolerance remains deferred; this ADR does not establish a
global modelling tolerance.

Mathematically invalid operations must fail explicitly. Future examples include
division by an effectively zero scalar, normalization of a zero-length vector,
and inversion of a singular matrix.

## Rationale

`double` provides an appropriate precision baseline for CAD-domain mathematics.
Combined absolute and relative comparison remains meaningful both near zero and
across different numerical scales. Explicit failure prevents invalid results
from being silently propagated.

## Consequences

- Math APIs and tests will use `double` as their default scalar.
- Rendering conversions to `float` must be visible at boundaries.
- B1.1 must define comparison constants and behavior consistent with this ADR.
- Modelling tolerances require a separate future decision based on geometric
  requirements.

## Alternatives Considered

- `float` as the primary scalar: rejected because rendering convenience must
  not determine CAD-domain precision.
- Exact floating-point equality: rejected for general numerical comparison.
- A single absolute epsilon: rejected because it does not scale with magnitude.
- A global geometric tolerance now: deferred until modelling requirements exist.
