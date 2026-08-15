# ADR-0005 — Internal Math Library

Status: ACCEPTED

## Context

Vector, matrix and spatial mathematics are central to the educational purpose
of Micro SolidWorks. Delegating them immediately would hide concepts the
project intends to study.

## Decision

Use strategy `BUILD` for the initial math library. Expected future concepts
include Vector2, Vector3, matrices, transforms, rays, planes and bounding boxes.
None of these classes is implemented in B0.

GLM and Eigen are deferred and require explicit future evaluation.

## Rationale

An internal implementation makes numerical assumptions, invariants and
algorithms visible and testable, maximising learning value.

## Consequences

- The project owns correctness and testing of its mathematical foundations.
- Initial development may be slower than integrating a mature library.
- A future replacement must preserve a deliberate architectural boundary.

## Alternatives Considered

- GLM: mature and rendering-oriented, but deferred to preserve learning value.
- Eigen: powerful general linear algebra, but broader than initial needs and
  likewise deferred.
