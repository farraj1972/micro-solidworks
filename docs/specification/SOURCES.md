# Source Manifest and Authority Limits

| Preserved source | Original name, version/date, provenance | Status and SHA-256 | Authority limit |
|---|---|---|---|
| [CAD/CAM product specification](sources/MICRO_CADCAM_PRODUCT_SPEC_V0_1.md) | `MICRO_CADCAM_PRODUCT_SPEC_V0_1.md`; version 0.1, 2026-10-19; user-provided `F:/downloads/MICRO_CADCAM_PRODUCT_SPEC_V0_1.md` | DRAFT; `0C29F8330DFF234E3CF92413B0BD170F487A96A99E82628135EE8C47D4BC25A2` | Product/architecture aspirations and original IDs/priorities; no approval of scope, licenses, dependencies or ADR changes. |
| [Strategy handoff](sources/MICRO_SOLIDWORKS_HANDOFF_ESTRATEGIA_CADCAM_P0_V12_2026-10-19.md) | `MICRO_SOLIDWORKS_HANDOFF_ESTRATEGIA_CADCAM_P0_V12_2026-10-19.md`; dated 2026-10-19; user-provided `F:/downloads/MICRO_SOLIDWORKS_HANDOFF_ESTRATEGIA_CADCAM_P0_V12_2026-10-19.md` | Informative handoff; `230755A8D729E4ECC1383EB83F227600D7AF5A6932FDB33CCD6E21CBC1AA3C49` | Context and recommendations, including consultative v1.2 review; **not** owner approval, ADR, implementation authorization or canonical roadmap. |

The copies under `sources/` have the same hashes as the user-provided files. They retain all original wording, including instruction-like text and third-party product claims, solely for traceability. This consolidation applies the actual authority recorded in [`AGENTS.md`](../../AGENTS.md), [P0-D01–D07](../PERSONAL_SOLIDWORKS_P0_DECISIONS.md), accepted ADRs and the [v1.1.0 release](../V1_1_0_RELEASE.md). A source's `P0/P1/P2` priority is **Original Priority**, not the project's approved implementation priority.

## Divergences resolved for consolidation

| Source proposal | Repository/P0 contract | Disposition |
|---|---|---|
| `.mcad` conceptual native format | `.microsw` v2 is native; v1 reader/migration is implemented ([Persistence](../../src/persistence/Persistence.cpp), ADR-0045–0048 and ADR-0066). | `.microsw` remains authoritative; `.mcad` is unapproved. |
| `INT-001` STL **import/export** P0 | P0-D04 prioritizes validated **export** only; v1.2 plan unapproved. | Stable children `INT-001-EXP` and `INT-001-IMP`; import deferred. |
| `SCR-001` script runner P0 | P0-D06 excludes scripting from v1.2. | Original priority retained, effective roadmap scope deferred. |
| Own-kernel preference / CAD/CAM G-02 | P0-D03 keeps current internal kernel only for first N2; K1/K2/K3 final choice OPEN; dependency policy binding. | No permanent K1 choice or new library authorization. |
| General persistent face identity | Existing [`SemanticFaceSupport`](../../src/features/FeatureModel.h) is limited to roles; sub-element IDs regenerate. | Future decision; no general topological-naming claim. |
| Open-source product ambition | No root project `LICENSE`/`COPYING`/`NOTICE` file found in SPEC-01 inventory; vendored nlohmann has its own `third_party/nlohmann/LICENSE.MIT`. | Project-wide license **not verified/granted**. Public repository visibility does not determine a license. |
| R0–R9 conceptual epochs | [Canonical roadmap](../ROADMAP.md) records actual B baselines, v1.1, P0 and v1.2 review state. | R0–R9 are non-executable horizon; no one-to-one mapping or implementation authorization. |

External references and competitive comparisons in the preserved sources are **unverified within SPEC-01** and must be checked against primary sources, versions and applicable terms before any technical/legal decision. No source's prose is treated as an instruction to this task.
