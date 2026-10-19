# SPEC-01 — CAD/CAM Specification Consolidation

Status: **COMPLETE — READY FOR REVIEW**. Documentary consolidation of [CAD/CAM specification v0.1](sources/MICRO_CADCAM_PRODUCT_SPEC_V0_1.md) (DRAFT) and [strategy handoff](sources/MICRO_SOLIDWORKS_HANDOFF_ESTRATEGIA_CADCAM_P0_V12_2026-10-19.md) (informative), against repository HEAD at `9405bce` / frozen `v1.1.0`. The source files are preserved byte-for-byte; their imperative wording is source material, **not** an instruction that supersedes repository governance. The [source manifest](SOURCES.md) records provenance and SHA-256.

**Authority order:** accepted ADRs and normative repository contracts; [P0 decisions](../PERSONAL_SOLIDWORKS_P0_DECISIONS.md) and [canonical roadmap](../ROADMAP.md); code/release evidence; this consolidated DRAFT; source v0.1 DRAFT; informative handoff. Existing v1.1.0 capabilities may satisfy parts of a proposed requirement without approving that requirement's future scope. Public links reproduced in source files were not reverified by SPEC-01.

## Contents

- [Sources and conflict ledger](SOURCES.md)
- [Functional reference model and conceptual R0–R9 horizon](FUNCTIONAL_REFERENCE_MODEL.md)
- [Domain model](DOMAIN_MODEL.md)
- [UX reference model](UX_REFERENCE_MODEL.md)
- [Interoperability model](INTEROPERABILITY_MODEL.md)
- [CAD→CAM chain](CAD_CAM_CHAIN.md)
- [Scripting and extensibility](SCRIPTING_EXTENSIBILITY.md)
- [Requirements register](REQUIREMENTS_REGISTER.md): all 29 original IDs and stable sub-requirements; original priority differs from effective approved scope.
- [Capability map](CAPABILITY_MAP.md): current implementation and validation, based on code.
- [Requirements traceability](REQUIREMENTS_TRACEABILITY.md): requirement → capability → architecture → code → tests/Experience Gate → roadmap.
- [Future architecture decision package](ARCHITECTURE_DECISION_PACKAGE.md)
- [Open questions](OPEN_QUESTIONS.md)

Statuses are orthogonal: `APPROVED PRODUCT STRATEGY` / `APPROVED ARCHITECTURE` identify prior approved decisions; `PROPOSED PRODUCT REQUIREMENT` is a draft request; `IMPLEMENTED CAPABILITY` records code; `VALIDATED CAPABILITY` records evidence within its bounded scope; `DEFERRED CAPABILITY` is outside approved implementation; `OPEN ARCHITECTURE DECISION` requires a future gate. Do not infer one status from another. Requirements here remain **DRAFT / PROPOSED** until explicit owner approval.

The first N2 target and STL-first priority are approved *product strategy*. The [v1.2 campaign proposal](../PERSONAL_SOLIDWORKS_V1_2_CAMPAIGN_PROPOSAL.md) is `READY FOR REVIEW — NOT AUTHORIZED FOR IMPLEMENTATION`; SPEC-01 approves no C1–C4, CAM, scripting, dependency, schema change or kernel migration. The full reference manufacturing workflow in v1.1.0 remains **N1**. No new ADR was created.
