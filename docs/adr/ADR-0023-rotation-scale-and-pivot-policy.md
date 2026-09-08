# ADR-0023 — Rotation, Scale and Pivot Policy

Status: ACCEPTED

## Context

The initial transform editor needs explicit rotation, scale and pivot rules
that are understandable and compatible with the frozen coordinate conventions.

## Decision

Store rotation as Euler XYZ components `(rx, ry, rz)` in radians. Compose:

```text
R = Rz * Ry * Rx
```

With column vectors this applies `Rx`, then `Ry`, then `Rz`. Positive rotations
follow the right-hand rule. The UI presents degrees and converts them to radians
before updating the model. Gimbal lock is an accepted MVP limitation;
quaternions are deferred.

Store positive per-axis scale `(sx, sy, sz)`. Every component must be finite
and greater than zero. Zero, negative, NaN and infinity are invalid and must be
rejected without corrupting existing state. Reflection through negative scale
is deferred.

Rotation and scale operate around local origin `(0, 0, 0)`. B6 stores no pivot.

## Rationale

Euler XYZ state and local-origin pivot provide a small, inspectable educational
model for the first working transform slice. Positive finite scale preserves
invertible orientation without introducing reflection policy.

## Consequences

- `Transform3` validation covers all TRS components; translation and rotation
  must also be finite.
- UI and Math use an explicit degrees/radians boundary.
- The MVP knowingly permits Euler singularities.

## Alternatives Considered

- Degrees in Math: rejected by ADR-0008.
- Quaternion state: deferred until demonstrated requirements justify it.
- Zero or negative scale: rejected for the initial invariant.
- Stored custom pivot: deferred to keep the B6 entity state minimal.

## Deferred / Non-goals

Quaternions, negative scale/reflection, custom/selection/bounding-box pivots,
local coordinate-system pivots, gizmos, drag interaction and snapping remain
outside B6.
