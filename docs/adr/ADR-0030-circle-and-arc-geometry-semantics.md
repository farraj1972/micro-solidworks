# ADR-0030 — Circle and Arc Geometry Semantics

Status: ACCEPTED

## Context

Canonical B8 requires Circle and Arc geometry. The frozen Geometry kernel has
Point2 and Segment2 but deliberately deferred circular primitives and a general
intersection framework.

## Decision

`Circle2` is a Geometry value storing a finite `Point2 center` and finite radius
strictly greater than `defaultGeometricTolerance`. Invalid construction throws
`std::invalid_argument`. Angles are radians; angle zero is +X and positive angle
is counter-clockwise. Angles are not wrapped on input or in authoritative state.

The minimum API exposes center, radius, `pointAt(angle)`, `closestPoint(point)`,
`distance(point)` and `contains(point, tolerance)`. Contains means distance to
the circumference is at most the supplied finite non-negative tolerance. For a
query at the exact center, closestPoint deterministically returns `pointAt(0)`.

`Arc2` is a Geometry value storing finite center, radius, start angle and signed
sweep angle. Radius follows Circle2 validity. Sweep must satisfy
`0 < abs(sweep) < 2*pi`; zero and full-circle sweeps are rejected so Circle2 and
Arc2 remain distinct. Positive sweep is counter-clockwise and negative sweep is
clockwise. Authoritative angles are retained without implicit wrapping.

Arc2 exposes center, radius, start/sweep angles, start/end points and
`pointAt(t)` for finite `t` in `[0,1]`, where the angle is
`startAngle + t*sweepAngle`. It also supplies on-arc/contains, closest-point and
distance queries needed for picking. Queries may normalize angles internally to
test membership; they do not change stored angles. If the radial direction is
not on the swept interval, the nearer endpoint is closest; equal endpoint
distances choose the start point. A center query also uses this endpoint rule.

The initial center-start-end Arc tool derives radius and start angle from the
start point and chooses the unique positive counter-clockwise sweep from start
to end in `(0,2*pi)`. Coincident angular directions would imply a full circle and
are rejected. Direction choice and three-point Arc creation are deferred.

Circle2 and Arc2 belong to `microsw_geometry` because they are reusable planar
geometric values. Circle3 and Arc3 are not introduced.

## Rationale

Center/radius/angle values are easy to inspect, edit and constrain later. A
signed sweep preserves orientation without a general curve hierarchy.

## Consequences

- GAP-GEO-001 closes only when Circle2 is implemented and validated in B8.
- GAP-GEO-002 remains OPEN: B8 needs no Segment-Segment, Line-Circle or
  Circle-Circle intersection engine.
- Full circles use Circle2 rather than a special Arc2 representation.

## Alternatives Considered

- Start/end/center as redundant authoritative Arc state: rejected because radius
  and sweep can disagree.
- Three-point Arc: deferred because center-start-end matches the chosen value.
- General parametric Curve base: rejected as unnecessary for the first slice.

## Deferred / Non-goals

Circle3, Arc3, intersections, trimming, splines, ellipses, curve hierarchies and
general parameterization are deferred.
