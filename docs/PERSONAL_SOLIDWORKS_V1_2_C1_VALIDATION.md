# V1.2-C1 — Validated CAD Export Mesh

Status: **PASS — targeted V2 validation**. C1 adds no STL writer or
user-facing export command. The current `FeatureModel::resultFeatureId()` and an
explicit body index select one rebuilt `topology::Solid`; historical feature
caches cannot enter this path.

## Contract and ownership

`microsw_export` consumes the authoritative Solid and the existing
Presentation-owned pure CAD tessellation under ADR-0056. It supplies a quality
parameter independent of the default viewport's 24 segments, welds the
derived vertices, reconciles triangle orientation by connected edge incidence,
and validates the resulting indexed mesh. No OpenGL, GPU buffer or Dear ImGui
type enters the contract. The Solid and `.microsw` document remain unchanged.

The first export contract selects **one connected body per mesh/file**. Its
transient source-body index is carried with the derived mesh. A
result with several bodies requires an explicit body choice; disconnected or
overlapping components in one mesh are rejected rather than claimed safe.
This deliberately avoids an unsupported general overlap/self-intersection
guarantee. C2 must make the body choice visible and may write separate files,
but cannot silently combine bodies.

## Quality and validation

The initial quality profile uses 0.02 mm maximum chordal deviation, 0.1 rad
maximum angular step, 1e-8 mm deterministic vertex weld, at most 4096 curve
segments and 200,000 triangles. A circular edge uses the tighter of angular
and chordal requirements; the same CAD edge is sampled identically across
incident faces. Invalid settings, excessive subdivision and triangle limits
fail explicitly. These values are scoped to the canonical N2 fixture and
must remain independently adjustable from display tessellation.

The validator distinguishes fatal status/diagnostic from nonfatal warnings
(none emitted for the supported fixture). It checks finite coordinates,
in-range indices, nondegenerate
triangles, two incident triangles per closed mesh edge, consistent winding,
one connected component, positive enclosed signed volume and AABB. The
builder may reconcile raw per-face winding deterministically; the public
validator rejects an inverted triangle. A conflicting orientation cycle
fails. Outward normals for STL are derived from validated triangle winding
in C2, not copied from viewport shading normals.

For the Ø8 × 8 mm through-hole, 63 facets at the initial angular setting
give approximately 0.6663 mm³ of inscribed-polygon volume difference. The
fixture tests compare export-mesh volume to exact CAD volume within 1 mm³
and AABB coordinates within 1e-8 mm for initial, width-edited and
pocket-depth-edited states. This bounded comparison does not claim general
manufacturability or general self-intersection detection.

## Directed evidence

`ExportMesh.*` covers the N2 holed and pocketed plate, its two mandatory
edits, deterministic repetition, cuboid, cylinder, through-holed plate,
invalid/current result, body choice, disconnected components, open and
degenerate meshes, inverted winding, nonfinite Point3 rejection and resource
limit. `SolidDisplay.*` is rerun only because its tessellation API gained an
optional curve-segment parameter. Earlier R2 exact CAD and Experience Gate
evidence remains valid; C1 does not rerun the full geometry preflight.

## Gate

Validation level: **V2**, because C1 crosses FeatureModel → Presentation
tessellation → export mesh and changes an existing Presentation API. Affected
closure: `microsw_features`, `microsw_presentation`, `microsw_export`, their
directly affected tests. Full CTest, application runtime, slicer, STL I/O and
Windows package are intentionally deferred to their corresponding gates.

Build: incremental Debug `micro_solidworks_tests` **PASS**, zero project
warnings. Directed tests: **17/17 PASS** (`ExportMesh.*`, `SolidDisplay.*`,
`AxialSubtraction.*`, R2 persistence/history fixture). `git diff --check`:
**PASS**. Architecture:
ADR-0056 remains authoritative; no new dependency, renderer ownership or
frozen CAD contract change. C2 entry requires this gate to pass.
