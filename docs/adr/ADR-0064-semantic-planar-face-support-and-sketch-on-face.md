# ADR-0064 — Semantic Planar Face Support and Sketch-on-Face

Status: PROPOSED

## Context

V1.1-C4 requires an associative Sketch on a modeled face. D15 makes `FaceId`, derived body context and visual identity rebuild-local, so none can become a persistent Sketch dependency. General persistent topological naming is outside C4.

## Decision

Sketch support becomes an authoritative discriminated value: `ExplicitPlane` owns a `SketchPlane`; `SemanticFace` owns a `PlanarFaceSupportRef` containing a source `FeatureId` and `StartCap` or `EndCap` role. C4 accepts semantic support only from a single-body Extrude. Side, cylindrical, Boolean-generated, arbitrary and ambiguous multibody faces are unsupported.

At command time the rebuild-local face is converted to the semantic reference and discarded. Rebuild resolves the source first and derives the requested cap. `StartCap` is the source profile plane. `EndCap` is translated by positive extrusion distance along the source Sketch normal. The frame inherits source X; Y is chosen so `cross(xAxis, yAxis)` equals the outward cap normal. Start uses the opposite source normal; End uses the source orientation. Face ordering, tessellation and camera do not participate.

Local Sketch geometry remains unchanged when the cap moves. If the source is missing, failed, blocked, not a single-body Extrude or lacks the role, the Sketch becomes FAILED with a structured diagnostic and dependants become BLOCKED_BY_DEPENDENCY. There is no similar-face search or automatic reattachment. A redundant resolved plane is never authoritative fallback.

The workflow is select a valid cap then Sketch. Invalid faces disable the command or show an explanation. A new face Sketch automatically orients Normal To, while later navigation remains available. Properties show `Start/End face of ExtrudeN`, never topology IDs.

## Rationale

Known generation semantics provide associativity without claiming general topological naming. The inherited frame is stable under source dimension, position and distance edits.

## Consequences

- Sketch gains a dependency on its semantic support producer.
- Its derived SketchPlane resolves before constraints and regions.
- Persistence stores the support discriminator and reference.

## Alternatives Considered

Plane snapshot loses associativity; raw FaceId/body index is rebuild-local; geometric fallback can silently attach incorrectly; general persistent naming is disproportionate to C4.

## Deferred / Non-goals

Side, Boolean-generated, arbitrary planar and curved faces; multibody cap identity; persistent Face/Edge/Body identity; manual reattach.
