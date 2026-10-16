# B14 — Persistence + MVP Executable Validation

Status: PASS — B14 FROZEN; MVP documentary freeze pending

Validated executable state: `22d6cb9a53035f2856907b75339bf6b34324e46e`

## Implemented scope

- `microsw_persistence -> microsw_features`, with no reverse dependency.
- Vendored, pinned nlohmann/json 3.11.3 single header and MIT license; JSON
  types remain private to Persistence.
- Deterministic human-readable `.microsw` JSON format version 1.
- Authoritative FeatureModel, Sketch entities/constraints, logical IDs and
  explicit next-ID counters; derived caches and runtime identities excluded.
- Validated temporary reconstruction, automatic rebuild, explicit Open/Save
  results, atomic same-directory Save and transactional session replacement.
- Application-owned current path and functional New, Save, Save As and Open UI.

## Incremental evidence

- V1 affected target build: PASS.
- Persistence roundtrip, sparse-ID, dirty-save, invalid-file, unsupported-version,
  failed-rebuild and invalid-path tests: PASS.
- V2 session Save As/Save/New/Open and invalid-Open preservation tests: PASS.
- Incremental Debug application and test target build: PASS.

## Final V3

- Fresh configure in a new build directory: PASS.
- Full Debug build: PASS; project warnings: 0.
- Full CTest: PASS, 793/793 tests.
- Manual runtime MVP smoke: PASS; normal shutdown confirmed.
- New, Save As, New/reset, Open with automatic rebuild, Feature Tree/final
  Solid, upstream edit/Rebuild/Save/reopen, invalid Open preservation and
  unsupported-version preservation: PASS.
- Controlled rebuild-failed fixture: PASS in automated coverage.
- Architecture audit: PASS; JSON is private to Persistence, Features has no
  reverse dependency, and domain modules have no filesystem/JSON errors.
- Dependency audit: PASS; only approved nlohmann/json 3.11.3 was added, with
  vendored MIT license.
- Repository hygiene and `git diff --check`: PASS.

This single V3 is also the Micro SolidWorks MVP executable validation. The
following B14 and MVP freezes are documentary V0 steps while executable HEAD
remains unchanged.

GAP-GEO-002 and GAP-SCENE-002 remain OPEN / outside MVP.
