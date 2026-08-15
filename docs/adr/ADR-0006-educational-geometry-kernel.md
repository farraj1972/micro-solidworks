# ADR-0006 — Educational Geometry Kernel

Status: ACCEPTED

## Context

Geometry and topology are core subjects of the project. Adopting an industrial
kernel immediately would obscure the algorithms and representation choices the
project exists to explore.

## Decision

Build an internal Educational Geometry Kernel incrementally, prioritising
understanding and correctness over industrial robustness or performance.
The strategy is `BUILD -> POSSIBLE REPLACE`.

No geometry-kernel interface or implementation exists in B0. A future
replacement boundary will be introduced only when concrete requirements make
it necessary. OpenCascade and CGAL are deferred.

## Rationale

Constructing the initial kernel provides direct experience with geometry,
topology, tolerances and modelling operations while retaining the possibility
of controlled future evolution.

## Consequences

- Early capabilities will be educational rather than industrial-grade.
- Algorithms and invariants must receive strong tests as they are introduced.
- Replacement remains possible after the project understands the required
  boundary and trade-offs.

## Alternatives Considered

- OpenCascade: comprehensive industrial kernel, deferred because it would hide
  much of the intended learning path.
- CGAL: strong computational-geometry library, deferred for the same reason and
  because it is not a direct replacement for every CAD-kernel responsibility.
