# ADR-0067 — Viewer Arbitrary Orientation and Normal-To Semantics

Status: ACCEPTED

## Context

The existing orbit camera stores yaw and pitch relative to a fixed world-Z up
direction. It cannot represent an arbitrary planar Sketch frame with predictable
screen X/Y orientation. V1.1-C4 requires deterministic Normal To for supported
StartCap and EndCap Sketches.

## Decision

Viewer-owned camera orientation is one authoritative project-owned right-handed
orthonormal basis. The camera stores forward and up, derives right, and validates
unit length and mutual orthogonality within Math tolerance. Yaw and pitch may
remain derived compatibility queries but are not a second source of truth.

Normal To converts a SketchPlane at the Viewer/Application boundary. It sets
camera forward to `-normal`, screen right to `+xAxis`, and screen up to `+yAxis`.
The result depends only on the SketchPlane basis. StartCap and EndCap therefore
preserve their own frozen local frames without world-up heuristics.

Orbit composes rotations with the current basis. Pan uses camera right/up; zoom
and Fit retain their existing distance semantics. Standard views construct
deterministic bases. No user-facing Roll command is added.

## Rationale

An orthonormal basis is the smallest representation that supports implicit roll,
avoids gimbal singularities at Top/Bottom, and keeps the educational internal
Math policy. It requires no external dependency or CAD-layer coupling.

## Consequences

Viewer and session state own orientation. FeatureModel, Geometry, Topology and
document persistence remain unchanged. Picking and projection consume the same
camera basis and must retain their contracts. Existing orbit, pan, zoom, Fit and
standard-view behavior require focused regression coverage.

## Alternatives

A project-owned quaternion is valid but adds unnecessary representation and
conversion work for this increment. Retaining authoritative yaw/pitch cannot
represent arbitrary roll. A world-up Normal To heuristic is nondeterministic for
Sketch X/Y and is rejected.

## Deferred / Non-goals

Camera roll UI, animated transitions and persisted view state are deferred.
