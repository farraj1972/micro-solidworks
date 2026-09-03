# ADR-0011 — View and Projection Conventions

Status: ACCEPTED

## Context

B2 needs explicit view/projection mathematics consistent with the frozen B1
foundation and D1 conventions. These operations are planned, not present in B1.

## Decision

### Projection modes

The Viewer shall support Perspective and Orthographic modes. Both need not
be implemented in the first increment.

Perspective uses vertical field of view, aspect = viewport width / viewport
height, near > 0 and far > near. Concrete FOV and clipping values are parameters,
not architectural constants. Internal angles remain radians.

Orthographic projection uses `visibleHeight` and
`visibleWidth = visibleHeight * aspectRatio`. Its zoom changes visible
scale / height, as defined in ADR-0010.

### View and mathematical strategy

The camera is described conceptually by eye, target and worldUp = +Z.
Construct the view matrix internally using `microsw_math`.
Use strategy BUILD for view, perspective and orthographic matrices.
Do not introduce GLM; the educational implementation remains internal.

### Matrix pipeline

Preserve D1 column vectors, `v' = M * v`, and logical semantics independent
of physical memory layout. The rendering pipeline is:

```text
clip = Projection * View * Model * position
```

Here position is conceptually homogeneous; this equation does not introduce
a new CPU vector type or multiplication API. Initial viewer aids use
Model = Identity.

Matrix inverse remains DEFERRED; the basic viewer does not require it.
B1 point/direction semantics remain unchanged: do not introduce
`Matrix4 * Vector3` or a `Point3` type merely for the viewer.
The existing affine `transformPoint` / `transformDirection` contracts
are not redefined as projective operations.

## Rationale

Explicit view/projection conventions prevent composition-order and unit
ambiguity. Building the mathematics internally preserves learning value and
the dependency boundary established by ADR-0005 and D1.

## Consequences

- Projection parameters must respect the selected mode's semantics.
- Navigation changes distance or visible height, rather than normal zoom via FOV.
- Rendering adapters must preserve column-vector semantics regardless of upload layout.
- Implementation is deferred to explicitly authorized B2 increments.
- Inverse, generic math abstractions and Quaternion remain deferred.

## Alternatives Considered

- GLM-based view/projection: deferred to preserve the BUILD strategy.
- Row-vector composition: rejected because it conflicts with D1.
- General matrix inversion for a basic view matrix: unnecessary for this scope.
- Introducing Point3 or Matrix4 * Vector3 for projection convenience: not selected.
