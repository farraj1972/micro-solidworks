# ADR-0058 — Solid Picking, Visibility and View Interaction

Status: ACCEPTED

## Context

C2 requires reliable face and edge interaction for planar, holed, cylindrical
and multi-body results while preserving D13 visibility and Viewer conventions.

## Decision

Picking resolves the screen cursor through derived primitives to the logical
target mapping owned by Presentation/Viewer. Face hits originate from derived
triangles and resolve to Feature, current body context and FaceId. Edge hits may
use derived curve polylines and resolve to the corresponding EdgeId. Triangle
and polyline indices remain private implementation detail.

The nearest visible geometric hit wins. Edges within a six logical-pixel
screen-space tolerance take priority over a face hit, matching the existing B4
picking tolerance, but only when the edge proxy is actually within that
tolerance. This limits excessive edge capture. Face evaluation uses nearest-hit
ray/geometry or equivalent depth-consistent derived picking; it must not select
occluded faces through the solid. Back-facing orientation alone does not reject
a nearest visible face.

Cylindrical triangles resolve to their logical cylindrical Face. Triangulation
of planar caps excludes inner-wire interiors, so a cursor ray through a hole
does not hit the cap Face. Hover uses the same resolution and priority as click,
does not mutate model state, and distinctly pre-highlights face, edge or feature
result.

EditorContext visibility remains authoritative for the session. Hidden features
or derived bodies are neither rendered nor pickable. Visible context geometry
may be dimmed by presentation policy. Right-click on a resolved face or edge may
offer Select, Show owning Feature, Inspect and meaningful owner visibility
actions; it does not create modeling features in C2.

Viewer/session state owns Fit, standard views, heads-up controls and display
mode. Fit frames all visible modeled geometry with a reasonable margin computed
from its bounds. Front, Back, Left, Right, Top, Bottom and Isometric use the
existing right-handed, XY-base, +Z-up convention. Orbit, pan and zoom bindings
remain unchanged. Camera and view state are not persisted in the CAD model.

## Rationale

Logical resolution, visibility filtering and depth coherence let users trust
what they hover and select while keeping Rendering free of CAD ownership.

## Consequences

- Presentation/Viewer maintain primitive-to-logical mappings and nearest-hit
  evaluation.
- Heads-up controls are compact Graphics Area UI over Viewer/session commands.
- C2 needs an Experience Gate covering form readability, discoverability,
  hover clarity, selection confidence, Fit, views, Feature Manager sync,
  context actions and visual hierarchy.

## Alternatives Considered

- Return triangle IDs: rejected because mesh layout is derived.
- Framebuffer ID picking: deferred; it is not required by the initial design.
- Ignore occlusion: rejected because hidden faces would be selected.
- Persist camera/display state in FeatureModel: rejected as non-CAD session state.

## Deferred / Non-goals

Selection acceleration, framebuffer picking, section view, measure, face-based
Sketch creation, Cut Extrude, Hole, fillet, chamfer and assemblies are deferred.
