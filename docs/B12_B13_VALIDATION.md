# B12/B13 Combined Validation

## Candidate

Executable candidate: `d7c94d4` plus the B12 UI/Application integration in
`9367f9c`. Documentation changes after validation do not alter executable code.

## Functional scope

- `microsw_features` owns append-only typed feature history, stable FeatureId,
  dependencies, state, last-successful caches and resultFeatureId.
- Sketch, Extrude and Boolean features rebuild deterministically in history order.
- Controlled edits propagate DIRTY through transitive dependents.
- Rebuild computes candidates on copies and commits the complete affected
  closure atomically.
- Failure records FAILED/BLOCKED states and preserves last-known-good caches.
- Boolean EMPTY is a successful committed empty result.
- FeaturePresentation retains `FeatureId -> VisualEntityId` identity and updates
  viewport geometry only after commit.
- Feature Tree exposes IDs, names, kinds, states, dependencies, diagnostics,
  cache availability and typed parameters, with an explicit Rebuild action.

## Validation

Validation level: V3 — combined B12/B13 baseline validation.

- Fresh configure: PASS (`cmake --fresh -S . -B build`).
- Full Debug build: PASS.
- Build warnings: 0 project/dependency/linker warnings.
- Full CTest: PASS, 785/785 tests, 0 failed.
- Architecture/dependency audit: PASS.
- ADR-0041: CONFORMANT.
- ADR-0042: CONFORMANT.
- ADR-0043: CONFORMANT.
- ADR-0044: CONFORMANT.
- GAP-GEO-002: OPEN / deferred.
- GAP-SCENE-002: OPEN / deferred.
- Runtime: PENDING manual smoke because the native Computer Use inventory did
  not expose the running application window.

The runtime smoke must confirm the visible Feature Tree and Boolean result,
upstream distance edit and DIRTY state, explicit rebuild, failure/last-known-good
and recovery, selection coherence, normal navigation/projection/UI behavior and
clean shutdown. Freeze and tags remain pending until this smoke passes.
