# Micro SolidWorks v1.1.0 — Release Freeze

Status: FROZEN (2026-10-19). This is the V0 documentary release step after
V1.1-C5, with no executable change after the final C5 V3 validation.
Annotated release tag: `v1.1.0` (`Micro SolidWorks — v1.1.0`).

## Gate evidence

- V1.1-C1, C2, C2A, C3, C4 and C5: FROZEN.
- ADR-0001–0069: 69/69 ACCEPTED and conformant; no architectural deviation.
- Final C5 Technical Gate: fresh configure, full Debug build, zero project
  warnings observed, 903/903 CTest tests, runtime workflow and normal close.
- C5 Experience Gate: APPROVED by the project owner after the complete
  feature-editing workflow. Native capture timed out; the visual result is
  based on the owner's direct observation.
- C5 freeze commit: `6f6dcff`. Changes between the V3 implementation commit
  `145709d` and this release freeze affect documentation only.
- Release validation: documentation/state consistency, `git diff --check`,
  repository hygiene and clean working tree. V3 was not repeated because the
  executable was unchanged.

See `docs/V1_1_C5_VALIDATION.md` for the detailed C5 gate.

## Delivered product capability

The release provides a CAD-first workspace; guided Sketch creation; constraints
and dimensions; general additive Extrude; solid shading and face/edge
selection; a hierarchical Feature Manager; Undo/Redo and document safety;
Sketch-on-Face; exact axial Cut Extrude within its frozen supported subset;
`.microsw` persistence v2 with v1 migration; feature deletion, editing and
rename; and readable rebuild diagnostics.

This is a bounded personal mechanical-CAD workflow, not a claim of full
SolidWorks compatibility. Suppression, reorder, persistent topological naming,
general exact Boolean, oblique Cut Extrude, advanced end conditions, Hole,
Revolve, Pattern, Mirror, Chamfer and Fillet remain deferred. They are not
authorized implementation work by this release.

## Strategic continuation

The Personal SolidWorks vision is APPROVED from C5 onward: product reliability,
complete bounded workflows, pragmatic future build-or-integrate evaluation and
progressive replacement guide priorities. The existing dependency policy and
accepted ADRs remain binding. V1.2+ has no approved roadmap. The next activity
is `Personal SolidWorks — Product & Architecture Review`, which will identify
priority mechanical-part workflows, assess kernel limitations and
interoperability requirements, and propose a realistic later roadmap. The
review is prepared here, not executed as part of v1.1.0.
