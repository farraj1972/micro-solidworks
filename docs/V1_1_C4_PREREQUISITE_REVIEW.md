# V1.1-C4 — Subtractive Modeling & Sketch-on-Face Prerequisite Review

## Result

```text
Status: DECISION REQUIRED
Implementation: NOT STARTED
Validation: V0
```

The product objective is renamed from **General Profiles & Cut Extrude** to
**Subtractive Modeling & Sketch-on-Face**. Sketch-on-Face is a required product
prerequisite for the intended Cut Extrude workflow. A global-plane-only cut
would expose an unnecessarily technical workflow and would not satisfy the
requested mechanically familiar CAD experience.

## Current capability assessment

- `SketchPlane` already provides authoritative local 2D coordinates and a
  deterministic right-handed local/world mapping.
- D14 extracts exact planar regions containing line, circle and arc boundaries,
  concavity, holes, nesting parity and multiple disjoint material regions.
- General extrusion produces exact planar/cylindrical topology and may return
  multiple `Solid` values under one `FeatureId`.
- Feature rebuild is ordered, backward-dependent and atomic; failed features
  preserve last-known-good caches and block their downstream closure.
- C3 checkpoints already capture all authoritative `FeatureData`. Once the new
  values enter that variant, creation and editing participate naturally in
  Undo/Redo without changing D17.
- Presentation exposes rebuild-local face selection as Feature, derived body
  discriminator and `FaceId`. D15 explicitly forbids treating these values as
  persistent topology names.
- B11 Boolean accepts one axis-aligned, linear Solid per operand and returns at
  most one Solid or empty. It rejects curved and multibody operands and cannot
  represent a split subtraction result.

## Sketch-on-Face support reference

### Options

**A — Plane snapshot.** Persisting only origin and axes is simple and stable,
but the Sketch does not follow its source after an upstream edit. This is useful
as an explicit detached-plane operation, not as the meaning of Sketch-on-Face.

**B — Semantic generated-face reference.** Persist the producer `FeatureId` and
a generated-face role. This preserves associativity without persisting
`FaceId`. It is viable for a deliberately restricted set of generated faces.

**C — Semantic reference plus geometric fallback.** A plane snapshot may aid a
diagnostic or recovery UI, but it must not silently become authoritative when
resolution fails. Silent fallback could attach the Sketch to stale geometry and
conceal a broken model. As automatic behavior this option is rejected.

### Recommended first semantic reference

Use Option B and initially support only the `StartCap` and `EndCap` of an
Extrude that resolves to exactly one body. The authoritative support is
conceptually:

```text
ExtrudeCapSupport
  producerFeatureId
  role: StartCap | EndCap
```

The selected rebuild-local face is converted to this value when the Sketch is
created; the selection object is never retained. A cap on a multibody Extrude
is ambiguous because D14 regions and derived bodies have no persistent identity.
Supporting those caps requires a later stable region/body semantic. Side faces
also require persistent boundary semantics and remain deferred.

Current face classes are classified as follows:

| Face class | C4 support | Reason |
|---|---|---|
| Single-body Extrude planar start/end cap | YES | Stable producer and cap role |
| Extrude planar side | DEFERRED | Requires stable source-boundary naming |
| Cylindrical side | OUT | Nonplanar Sketch support |
| Boolean-generated planar/orthogonal face | DEFERRED | No stable generation provenance |
| Other arbitrary face | OUT | General persistent topological naming is unresolved |

For an Extrude cap, derive the support origin and axes from the producer's
source `SketchPlane`, not from tessellation, `FaceId` order or camera state. The
end cap uses the source origin translated by the extrusion distance; the start
cap uses the source origin. The selected face outward orientation determines
the Sketch normal. Retain or reverse the source in-plane axes deterministically
so `cross(xAxis, yAxis) == normal`. This makes the frame stable when the source
dimensions or extrusion distance change.

If the producer fails, ceases to be a compatible single-body Extrude or cannot
resolve the requested cap role, the face-based Sketch becomes `FAILED` with a
dangling-support diagnostic and downstream Cut Extrude becomes
`BLOCKED_BY_DEPENDENCY`. It must never reattach to another face automatically.
The Property Manager should show a name such as `Support: End face of Extrude1`,
without topology IDs. `Normal To active Sketch` is a high-value C4 interaction
and uses the resolved Sketch frame without adding model semantics.

## Cut Extrude

### Authoritative feature

The minimum feature value should contain:

```text
CutExtrudeFeature
  sourceSketchFeatureId
  targetResultFeatureId
  distance
  direction: AlongSupportNormal | OppositeSupportNormal
```

`targetResultFeatureId` identifies the earlier result context, not a persistent
body. On rebuild, the cut applies transactionally to every body in that result
whose exact volume intersects the cutting volume. This avoids inventing
`BodyId`, remains deterministic, and matches the current multibody result model.
The UI can infer this target from the selected support face/current result and
need not request a technical target ID.

Behavior is explicit:

- no intersected body: feature `FAILED`, no authoritative partial result;
- one or more intersected bodies: replace each with all exact subtraction
  components and preserve untouched bodies;
- a fully removed body: omit it successfully;
- all bodies fully removed: successful empty result;
- a split body: return every connected component as a separate derived Solid;
- any unsupported or invalid candidate: fail the entire feature transaction.

The first slice supports Blind depth and Reverse Direction. `Through All` is
valuable but is deferred until finite-volume subtraction is robust; emulating it
with an arbitrary large distance is not an acceptable semantic definition.
Other end conditions, draft, two directions, Thin Feature and open profiles are
outside C4.

The cutting volume should accept every exact closed D14 profile already valid
for normal Extrude: lines, circles and arcs, concavity, holes, nested parity and
multiple regions. A hole in the cutting region preserves its material island.
This is the coherent target contract; staged implementation may first prove the
circle-hole and rectangular-pocket cases but must not freeze a competing profile
model.

### Kernel gap

Cut cannot be implemented honestly by the current B11 Boolean. B11 is limited
to one axis-aligned linear Solid per input, cell decomposition and a single
connected hole-free result. It cannot subtract general D14 prisms, preserve
cylindrical surfaces, create a through-hole, handle multibody inputs or return
multiple disconnected outputs.

C4 therefore requires a deliberate educational-kernel extension. The gate must
choose between a general exact Boolean extension and a narrower exact
prismatic-subtraction algorithm that preserves analytic plane/cylinder support
and returns zero or more validated Solids. Rendering-only removal, tessellation
subtraction and hidden-volume presentation are prohibited.

Required user-facing diagnostics include invalid profile, no target
intersection, unsupported target configuration, lost support face and
unsupported result topology. Preview remains derived and transient; Confirm is
the only authoritative mutation.

## Persistence and history

Adding face support and `CutExtrudeFeature` changes authoritative Feature data.
Although JSON v1 readers ignore unknown object members, they reject an unknown
feature-kind token. An old v1 reader therefore cannot safely consume the new
model. Keeping `formatVersion: 1` would misrepresent compatibility.

The recommended decision is format version 2 with deterministic migration of
v1 documents into the unchanged global-plane Sketch and existing feature
semantics. Derived face/body/topology IDs remain absent. The Decision Gate must
freeze the exact support DTO, Cut Extrude DTO, validation and migration policy.

D17 requires no semantic change. Checkpoints, dirty tracking and bounded history
already store the authoritative Feature variant by value. Create Sketch on Face,
create Cut Extrude and edit cut depth each become normal atomic editor
transactions.

## Candidate campaign options

### Option A — Minimal Cut

Single-body Extrude cap support, Blind cut and line/circle/arc profiles. This
delivers a hole and pocket quickly, but either restricts D14 regions artificially
or leaves near-term kernel and persistence rework.

### Option B — General Subtractive Modeling

Restricted semantic cap support combined with all closed D14 profiles,
automatic intersection targeting across one or more result bodies, exact
zero-to-many subtraction results and Blind/Reverse Direction. Through All is
deferred. This is comprehensive at the product boundary while keeping general
topological naming and arbitrary-face support outside scope.

### Option C — Split campaign

C4A introduces associative Sketch-on-Face and Normal To; C4B later introduces
general Cut Extrude. This reduces implementation coupling but leaves an interim
feature with limited standalone value and delays the target workflow.

| Option | User value | Technical risk | Naming pressure | Kernel pressure | Persistence impact | Time-to-demo | Future leverage |
|---|---|---|---|---|---|---|---|
| A — Minimal Cut | MEDIUM | MEDIUM | LOW | MEDIUM | HIGH | LOW | MEDIUM |
| B — General subtractive modeling | HIGH | HIGH | MEDIUM | HIGH | HIGH | HIGH | HIGH |
| C — Split campaign | MEDIUM | MEDIUM | MEDIUM | HIGH | HIGH | MEDIUM | HIGH |

## Recommendation

Choose Option B as one coordinated C4 campaign, implemented through small V1/V2
increments after a Decision Gate. Restrict associative support to semantic caps
of a single-body Extrude; this avoids claiming general persistent topological
naming while satisfying the key upstream-edit scenario. Use automatic
intersection targeting over the identified earlier result context and allow
zero-to-many exact output bodies transactionally. Defer Through All, side-face
support, Boolean-generated support and arbitrary planar faces.

The target Experience workflow remains: create base Extrude, select its planar
cap, create/dimension a circle Sketch, Cut Extrude it into a cylindrical hole,
Undo/Redo it, then edit the base and observe the supported Sketch and cut rebuild
associatively.

## Required Decision Gate

A new gate is required before implementation. It must freeze:

1. restricted semantic Extrude-cap support identity and resolution/failure;
2. deterministic face-based Sketch coordinate-frame derivation;
3. exact prismatic subtraction boundary and zero-to-many result semantics;
4. automatic target-result and multibody intersection policy;
5. Cut depth/direction and empty/split-result semantics;
6. persistence format v2 and v1 migration contract.

The roadmap retains **V1.1-C5 — Feature Workflow Hardening** after C4.

## Deferred

General persistent topological naming, persistent BodyId, arbitrary planar or
curved face Sketch support, side-face boundary naming, Boolean-generated support,
Through All, advanced end conditions, Thin Feature, open-profile cuts, draft,
surface cuts and rendering approximations are excluded.
