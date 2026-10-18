# ADR-0065 — Exact Subtractive Modeling and Targeting Semantics

Status: ACCEPTED

## Context

C4 must create exact pockets and cylindrical holes from closed D14 profiles. B11 accepts one axis-aligned linear Solid per operand and at most one connected hole-free result, so it cannot implement C4.

## Decision

Add `CutExtrudeFeature` with source Sketch `FeatureId`, explicit upstream target-result `FeatureId`, finite distance above modeling tolerance, and direction (`AlongSupportNormal` or `OppositeSupportNormal`). UI infers the target from support/current result; the committed feature stores it explicitly.

The cutter reuses D14 exact region/extrusion semantics, including concavity, holes, line/circle/arc boundaries, nesting parity and multiple regions. Open profiles remain Thin Feature scope. C4 supports Blind and Reverse Direction; Through All and other end conditions are deferred.

Implement a project-owned exact regularized prismatic difference, separate from B11. It operates on supported extrusion-derived analytic solids, constructs planar arrangements of line/circular-arc boundaries at axial event slabs, classifies material by region parity, reconstructs shared Plane/Cylinder topology, finds connected components and validates every Solid. It is not a generic NURBS B-rep Boolean. Unsupported geometry returns `UNSUPPORTED_TARGET`.

Every body in the target result whose exact volume intersects the cutter is processed; untouched bodies remain. One body may yield zero, one or many bodies. Full removal is successful omission and all removed is successful empty. No intersection is `NO_INTERSECTION` and fails rebuild. All candidates commit atomically or D11 last-known-good data remains.

Statuses include `SUCCESS`, `NO_INTERSECTION`, `INVALID_PROFILE`, `UNSUPPORTED_TARGET`, `TOPOLOGY_FAILURE` and `NUMERICAL_FAILURE`. UI diagnostics use logical features/Sketch elements, never raw topology IDs. Preview is transient and Confirm is one transaction.

## Rationale

The analytic algorithm teaches exact CAD subtraction and supports hole/pocket workflows without pretending B11 or render meshes are a general kernel. Explicit result targeting avoids persistent BodyId.

## Consequences

Cut results contain 0..N Solids under one FeatureId; cutting-region holes preserve material by D14 parity; B11 remains unchanged; Feature Manager absorbs the consumed Sketch.

## Alternatives Considered

Implicit B11 expansion, mesh Boolean and visual masking are rejected. Persistent BodyId and exactly-one-body targeting are unnecessary restrictions.

## Deferred / Non-goals

Generic Union/Intersection or B-rep/NURBS Boolean, Through All, other end conditions, draft, Thin Feature and selected-body scope.
