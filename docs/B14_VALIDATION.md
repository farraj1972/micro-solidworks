# B14 — Persistence + MVP Executable Validation

Status: CANDIDATE — final V3 pending

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

Pending clean configure, full Debug build, full CTest, runtime smoke,
architecture/dependency audit, repository hygiene and `git diff --check`.

GAP-GEO-002 and GAP-SCENE-002 remain OPEN / outside MVP.
