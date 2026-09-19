# ADR-0033 — Constraint Solver Architecture and Parameterization

Status: PROPOSED

## Context

The canonical B9 relations interact and must solve without order-sensitive
rewrite rules or an industrial symbolic framework. The implementation must stay
educational, deterministic and independent of external solver libraries.

## Decision

B9 introduces a project-owned `microsw_constraints` module with this edge:

```text
microsw_constraints
    -> microsw_sketch
        -> microsw_geometry
            -> microsw_math
```

Sketch contains constraint domain values, identities, references and the
controlled transaction boundary. `microsw_constraints` contains parameter
extraction, residual evaluation, finite-difference Jacobian construction,
Levenberg–Marquardt solving and diagnostics. There is no reverse dependency and
no external solver dependency.

The temporary parameterization is:

```text
Line   = x1, y1, x2, y2
Circle = cx, cy, r
```

Arc is excluded from the initial solver. The parameter vector is derived from
current valid geometry and never becomes authoritative. Reconstructed radii
remain finite, positive and valid under Geometry policy.

The solver minimizes weighted squared residuals. Conceptual residuals are:

```text
Coincident       = dx, dy
Horizontal       = y2 - y1
Vertical         = x2 - x1
Parallel         = cross2(normalized(d1), normalized(d2))
Perpendicular    = dot(normalized(d1), normalized(d2))
Driving dimension = measured - requested
```

Numerically equivalent formulations are permitted when they preserve these
semantics. Direction-dependent constraints reject degenerate candidate lines;
NaN and infinity are invalid.

The Jacobian uses deterministic central finite differences. For parameter
`x_i`, the perturbation derives from a centralized solver policy and scales with
`max(parameterScale_i, abs(x_i))`; it is not a scattered fixed epsilon.
Analytic Jacobians and automatic differentiation are deferred.

Length residuals are divided by a deterministic characteristic length:

```text
L = max(1 mm, current Sketch bounding-box diagonal,
        largest absolute driving length)
```

Directional residuals are already dimensionless. Residual weights are explicit
and centralized in solver policy; the initial weight is one after normalization.
No constraint type may embed an undocumented weight.

Under-constrained Sketches are valid. Solving starts at the current state. A
small, centralized Tikhonov regularization penalizes normalized displacement
from that initial state so unconstrained parameters do not move arbitrarily.
This is solver stabilization, not a visible logical constraint or hidden anchor.

## Rationale

Levenberg–Marquardt handles the small nonlinear system uniformly and avoids
order-dependent special cases. Dense finite differences are sufficient for the
first slice and keep the algorithm inspectable.

## Consequences

- Performance is secondary to deterministic, comprehensible behavior.
- Mixed length and direction equations have an explicit scale policy.
- The initial solver supports lines and circle radius without claiming full
  curve coverage or symbolic DOF analysis.
- A real external-dependency need requires a hard stop and new authorization.

## Alternatives Considered

- Direct rule rewriting: rejected because results become ordering-dependent.
- External solver: rejected for the initial educational implementation.
- Symbolic solver: rejected as premature scope.
- Unscaled mixed residuals: rejected because mm and dimensionless errors are not
  comparable.

## Deferred / Non-goals

Sparse infrastructure, symbolic solving, analytic Jacobians, automatic
differentiation, general DOF analysis, automatic redundancy resolution and Arc
parameterization are deferred.
