# B9 — Constraint System Baseline Validation

Status: PASS

Validated executable HEAD: `ce51c26f4e47929e36af9c83aa6745f1dfaead90`

## V3 evidence

- Clean configure: PASS. The sandboxed configure could not access the declared
  FetchContent repositories; repeating the same configure with network access
  populated the dependencies and passed.
- Full Debug build: PASS.
- Project, dependency and linker warnings: 0.
- Full CTest: PASS, 758/758 tests, 0 failed.
- Runtime: PASS by manual observation. Rectangle constraints and driving
  width/height edits solved coherently; the CircleRadius dimension edited one
  stable constraint and preserved the circle center; invalid/repeated UI input
  did not create duplicate radius constraints. Entity IDs were visible and the
  selected entity was identified. The resizable/scrollable constraint panel,
  X/Y/Z labels, rendering, picking, selection and normal application behavior
  were confirmed.
- Architecture/dependency audit: PASS. `microsw_constraints` depends on
  `microsw_sketch`, which depends on `microsw_geometry` and `microsw_math`;
  Constraints and Sketch core contain no Presentation, Viewer, Rendering, UI,
  Application or Topology dependency.
- Repository hygiene: PASS. No build output is tracked.
- `git diff --check`: PASS.

## D8 conformance

- ADR-0032: CONFORMANT — Sketch owns stable, local, non-reused constraint IDs
  and strong entity/sub-element references with controlled lifecycle.
- ADR-0033: CONFORMANT — the project-owned dense LM service uses deterministic
  Line/Circle parameterization, normalized residuals and a central finite-
  difference Jacobian outside the Sketch aggregate.
- ADR-0034: CONFORMANT — solver policy and diagnostics are explicit; logical
  satisfaction is distinct from optimization termination; successful candidate
  geometry commits atomically and all failures preserve authoritative geometry.

No architecture deviation or baseline finding remains open.
