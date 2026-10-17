# ADR-0056 — Solid Display, Tessellation and Shading Boundary

Status: ACCEPTED

## Context

V1.1-C1 presents exact solids primarily as derived wire geometry. V1.1-C2 must
make planar, holed and cylindrical forms readable without promoting triangles,
normals, lighting or display settings into authoritative CAD state.

## Decision

Topology and analytic Geometry remain authoritative. Presentation derives a
face mesh from each current topological Face and retains an explicit mapping
from every triangle to its logical face context. Planar faces, including inner
wires, and cylindrical faces are triangulated according to their oriented
support. Hole interiors produce no triangles. Tessellation is deterministic
derived state and may be regenerated without changing the model.

The default display mode is `Shaded With Edges`. `Wireframe` is mandatory and
`Shaded` may be supplied when it is a trivial policy variant. Face rendering
uses flat face shading with simple ambient plus directional lighting. Lighting,
colors, tessellation tolerance and display mode belong to Viewer/Rendering
presentation or session policy and are not persisted in `.microsw`.

Exact Edges independently generate derived line or curve presentation. The
overlay remains readable over filled faces, minimizes z-fighting through an
implementation-level depth/bias policy, and can express normal, hover,
selected, active/context and failed/disabled states. Rendering consumes draw
data and never owns CAD identity.

All derived bodies produced by one Feature are displayed. A runtime body index
or equivalent nonpersistent body-instance discriminator relates face and edge
draw data to the correct derived Solid. It is not a persistent `BodyId`.

## Rationale

This keeps the accepted CAD-authoritative pipeline while supplying the visual
form cues expected from a mechanical CAD workspace.

## Consequences

- Presentation owns topology-to-draw-data mappings and derived tessellation.
- Rendering needs filled-triangle and edge-overlay paths with depth coherence.
- Curved faces use tessellation only for display and picking.
- Changing display tolerance or lighting cannot alter CAD results.

## Alternatives Considered

- Store render meshes as Solid state: rejected because tessellation is derived.
- Add a material/PBR system: rejected as unnecessary for C2 readability.
- Use wireframe alone: rejected because it does not communicate solid form.

## Deferred / Non-goals

Materials, textures, PBR, section views, measurement, general surface types and
persistent body or subshape naming are deferred.
