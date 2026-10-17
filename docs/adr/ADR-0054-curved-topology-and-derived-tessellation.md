# ADR-0054 — Exact Curved Topology and Derived Tessellation

Status: ACCEPTED

## Context

A circle or arc profile cannot be extruded exactly with the current topology,
whose Edges are straight segments and whose Faces are planar. ADR-0036
correctly forbids treating a render tessellation as authoritative CAD topology.

## Decision

Topology Edge geometry becomes a project-owned curve use supporting at least
line segments, circular arcs and full circles. Face support geometry supports
at least planes and the cylindrical surfaces produced by circular extrusion.
Loop orientation is represented independently from the curve's authoritative
parameter direction.

Extruding a curve translates the curve to the opposite cap and creates the
corresponding ruled side surface. A full circle produces two circular cap
boundaries and one cylindrical side face without inventing polygonal CAD
edges.

Presentation tessellates topological curves and surfaces deterministically for
rendering and picking. Tessellation tolerance is derived display state and is
never serialized as authoritative model geometry.

## Rationale

Exact curve and surface ownership preserves the mandatory CAD-authoritative,
render-derived pipeline and allows Circle/Arc profiles without contradicting
ADR-0006 or ADR-0036.

## Consequences

- B7 topology validation must understand curved loops and surface uses.
- Presentation and picking share derived tessellation for coherence.
- Persistence must encode the new authoritative curve/surface variants.
- Existing straight-edge solids remain valid and backward compatible.

## Alternatives Considered

- Store a polygonal approximation as CAD topology: rejected because changing
  display tolerance would change the model.
- Special-case a cylinder outside Topology: rejected because it would create a
  second authoritative solid representation.
- Introduce an external geometry kernel: rejected; the dependency strategy
  remains BUILD for this educational capability.

## Deferred / Non-goals

Splines, ellipses, NURBS, cones, general swept surfaces and advanced surface
intersection are deferred until their sketch/modeling features require them.

## Final Normative Alignment

ADR-0035–0037 remain historically valid for the MVP slice. This controlled
post-MVP extension stores exact supported analytic CAD geometry. Edge geometry
supports LineSegment and CircularArc, or equivalent exact variants. A full
circle is split deterministically into two exact semicircular edges with
vertices at parameters 0 and pi in the derived oriented basis, unless later
implementation evidence approves another deterministic exact split.

Face support expands to the exact Plane/Cylinder subset. Extruding a line makes
a planar side face; extruding an arc or circle makes cylindrical side face(s).
A planar Face has one outer Wire and zero or more inner Wires. Inner wires have
opposite orientation to the outer wire on the oriented support. Wires remain
ordered, closed, oriented and exactly connected, and contain exact straight or
circular-arc edges rather than tessellated polylines.

Shell/Solid validation must support nonconvex closed 2-manifold shells, exactly
two compatible opposite uses per edge, valid analytic supports, valid loops and
nondegenerate geometry. Nonzero genus and through-holes are valid when the shell
remains one connected closed 2-manifold.

CircularArc, Cylinder and planar Faces with inner Wires are tessellated only as
derived Presentation/Rendering data. Display tolerance never decides CAD
topology. Picking may use the shared derived tessellation while logical identity
remains independent. D14 does not decide persistent face/edge naming.
