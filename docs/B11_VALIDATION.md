# B11 — Boolean Operations Baseline Validation

Status: PASS

## Scope

B11 implements the D10 axis-aligned box Boolean subset: Intersection, Union and
Difference through deterministic coordinate-cell occupancy and reconstruction
of new shared manifold topology. It does not claim general BRep Boolean support.

## Validation

Validation level: V3

- Clean configure: PASS. The first attempt could not access GitHub; the successful
  clean configure reused the repository's already pinned dependency source trees.
- Full Debug build: PASS.
- Project warnings: 0.
- Full CTest: PASS, 775/775.
- Targeted B7/B10 regression: PASS.
- Boolean core and vertical-slice tests: PASS.
- Runtime: PASS, manually confirmed on the final executable.
- Architecture and dependency audit: PASS.
- Repository hygiene and git diff --check: PASS.

Runtime covered deterministic operands, Intersection, non-convex Union,
non-convex Difference, visible selectable wireframe results, hover/selection/
highlight, navigation, projections, grid/axes, UI/About, touching-only
UNSUPPORTED_CASE, preservation of the preceding successful result and normal
shutdown.

## Conformance

- ADR-0038: CONFORMANT.
- ADR-0039: CONFORMANT.
- ADR-0040: CONFORMANT.
- GAP-GEO-002 remains OPEN / deferred.
- GAP-SCENE-002 remains OPEN / deferred.

No external dependency, general intersection framework, general BRep splitting,
curved topology, inner wire, cavity, through-hole, multiple result component,
Document or Feature Tree was introduced.

