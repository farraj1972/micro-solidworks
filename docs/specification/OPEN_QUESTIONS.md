# Open Questions and Verification Backlog

Each item is **open**, not an authorization to implement. [SPEC-01 sources](SOURCES.md) are preserved; [P0 decisions](../PERSONAL_SOLIDWORKS_P0_DECISIONS.md) remain authoritative. Prioritize questions by the next owner-reviewed campaign rather than by source v0.1 P0/P1/P2 labels.

| Question | Why it matters | Evidence/decision owner and timing |
|---|---|---|
| Can the approved synthetic plate actually be rebuilt with base Sketch outer rectangle + inner circle → Extrude → face Sketch → one axial pocket Cut, including parameter edits and Save/Open? | [`FeatureModel::rebuild`](../../src/features/FeatureModel.cpp) rejects Cut→Cut target; the proposed path avoids but is not yet proven end-to-end. | Targeted v1.2 geometry preflight **after owner review of the plan**, before C1 authorization; report failure without scope expansion. |
| What CAD-to-export-mesh boundary, tolerances and multi-body rules meet STL quality? | Display triangles are not automatically watertight export mesh. | V1.2-C1 contract proposal/review; check ADR-0056 and P0-D04. |
| Which Windows package/runtime and external slicer will validate N2? | N2 requires use outside the dev tree and independent units/scale verification. | Owner picks receiver; C3/C4 plan review and later gates. |
| What is the correct project-wide license? | No root LICENSE/COPYING/NOTICE was found; open-source aspiration does not establish grant. | Owner/legal review before public licensing/distribution claims. |
| Is the long-term kernel K1, K2 or K3? | General Boolean, blends and STEP may exceed internal kernel economics; hybrid conversion has identity risk. | Separate OCCT-oriented spike and explicit owner Decision Gate; not a blocker for first N2. |
| What do `.mcad`, general Project/Document types and persistent topology identity mean relative to `.microsw` v2/FeatureModel? | Avoid dual authority and broken migrations. | Future workflow and schema gate only if needed; `.microsw` unchanged now. |
| Is scripting a real near-term user need, and should Python or Lua be used? | Source's P0 priority conflicts with approved v1.2 boundaries. | Future user research, threat model/licensing and Decision Gate; no runtime now. |
| Which 2.5D machine/controller and safety envelope would justify CAM? | G-code can move physical machines; simulation is insufficient. | Owner/operator workflow, controller fixtures and safety review before CAM commitment. |
| Are all external benchmark/licensing/standard links in source v0.1 current and legally usable? | SPEC-01 preserved sources but did not reverify third-party claims or standards text. | Primary-source/version/legal research at the relevant future gate. |
| Which parts of UX-002/003/004/005 fail first-use real-user tests? | C5 Experience Gate validates a bounded workflow, not general accessibility/discoverability. | Task study with repeatable desktop scaling/keyboard/error recovery evidence before prioritizing UX expansion. |

No question here changes the status of `v1.1.0` (FROZEN), P0 (FROZEN), v1.2 (READY FOR REVIEW / NOT AUTHORIZED FOR IMPLEMENTATION), or K1/K2/K3 (OPEN — EVIDENCE REQUIRED).
