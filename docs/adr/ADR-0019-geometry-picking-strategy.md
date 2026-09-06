# ADR-0019 — Geometry Picking Strategy

Status: ACCEPTED

## Context

Interactive picking must connect mouse input and the B2 camera/projection to
zero-area mathematical entities without redefining Geometry tolerance.

## Decision

B4's primary strategy is geometric picking. Mouse position, viewport, camera
and projection produce a world-space picking query. The existing Geometry
`Ray3` may be used or adapted when semantically appropriate; a duplicate
geometric Ray type must not be introduced without need.

Point, Segment and Line have zero mathematical area/thickness. Interactive hit
testing therefore uses a distinct picking tolerance derived from screen-space,
preferably pixel-based and converted as required by the view context. Its exact
value is increment-scoped.

```text
modeling tolerance != interaction/picking tolerance
```

The `defaultGeometricTolerance` of 1e-9 mm is not a click tolerance. Geometry
must not gain `isClicked`, `pick`, `hover`, `screenDistance` or pixel-tolerance
APIs. Picking belongs to its consumer/interaction boundary.

## Rationale

Geometric picking is testable and teaches the mapping from screen input to
world queries while preserving model and interaction tolerance semantics.

## Consequences

- Picking consumes camera/projection and Geometry through external adapters.
- Hit policy may account for apparent screen-space proximity.
- Results identify presented entities without mutating them.

## Alternatives Considered

- Color-ID framebuffer picking: deferred; it is not B4's fundamental strategy.
- Reusing modeling tolerance: rejected because visual clickability is contextual.
- Adding screen concepts to Geometry: rejected as boundary leakage.

## Deferred / Non-goals

Framebuffer color picking, acceleration structures, occlusion policy, multiple
hits and production selection acceleration are deferred. No picking code is
introduced by this decision gate.
