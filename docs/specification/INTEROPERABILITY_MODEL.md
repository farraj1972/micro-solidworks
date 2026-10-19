# Interoperability Model — DRAFT with Approved STL Priority

Source: [specification v0.1 §5](sources/MICRO_CADCAM_PRODUCT_SPEC_V0_1.md), resolved by [P0-D04](../PERSONAL_SOLIDWORKS_P0_DECISIONS.md). Format entries are intended contracts, not implemented adapters unless explicitly stated.

| Format | Target role and representation | Current / approved scope | Required limits or evidence |
|---|---|---|---|
| `.microsw` | Native authoritative parametric FeatureModel document. | **Implemented and validated** v2 writer/v1 reader in [`Persistence`](../../src/persistence/Persistence.h); preserve it. | Roundtrip authoritative IDs/parameters, transactional Open/Save and rebuild. No `.mcad` migration approved. |
| STL | Derived triangle mesh for 3D-print handoff. | Export **approved product priority**, not implemented; detailed v1.2 plan pending. Import deferred. | Closed oriented nondegenerate mesh, tessellation tolerance, multi-body policy, explicit external units convention and independent parser/slicer. STL is not exact CAD or parametric history. |
| STEP | Exact CAD/B-rep exchange by stated AP/profile. | Not implemented; future architectural/kernel decision. | Define supported entities/units/assembly fidelity and roundtrip loss; no claim that feature history transfers. |
| DXF | 2D curves, profiles/drawings for fabrication. | Not implemented; evaluate after first N2 as job requires. | Entity/layer/unit subset, arc preservation and independent receiver validation. |
| 3MF | Additive-manufacturing mesh plus explicit metadata/units. | Not implemented, deferred. | Version/extension profile and external receiver checks. |
| OBJ | Mesh exchange. | Not implemented, deferred. | Open-mesh/material limitations and import classification. |
| glTF/GLB | Visualization/scene publication mesh. | Not implemented, deferred. | Never imply B-rep/parametric CAD interchange. |
| IGES | Legacy curve/surface exchange. | Not implemented, deferred. | Healing and partial topology limitations need scope. |
| G-code/NC | Controller-specific CAM output, not CAD exchange. | Not implemented; CAM DRAFT only. | Controller dialect, WCS/tool/stock and human verification before machine use. |

The source's `.mcad` is conceptual only. The source's `INT-001` and original P0 priority cover *both* STL directions; stable children `INT-001-EXP` and `INT-001-IMP` separate approved-priority export from deferred import. An import preview/report (`INT-005`) is also deferred. Future adapters should declare entity support, units, tolerances, loss report, failure semantics and independent receiver tests. Imported mesh must not silently become editable exact CAD topology; exported mesh must be derived from authoritative CAD, not from a rendering GPU buffer. No third-party format library is authorized by this document.
