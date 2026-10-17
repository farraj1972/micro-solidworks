# V1.1-C2 — Solid Visualization & Selection Validation

## Scope

V1.1-C2 adds derived flat-shaded solid faces, topology-derived edge overlays,
logical face/edge hover and selection, Fit, standard CAD views and compact view
controls. Topology and analytic Geometry remain authoritative; triangles,
polylines and runtime body contexts are Presentation/Viewer data only.

## D15 conformance

- ADR-0056: CONFORMANT — planar, holed and cylindrical faces are tessellated in
  Presentation and drawn by identity-free Rendering code.
- ADR-0057: CONFORMANT — logical targets contain FeatureId, rebuild-local body
  context and FaceId or EdgeId; topology selection clears on rebuild.
- ADR-0058: CONFORMANT — nearest visible face picking and six-logical-pixel edge
  priority resolve derived primitives back to logical topology targets.
- Architectural deviations: none.

## Implemented behavior

Shaded With Edges is the default. Wireframe and Shaded are session-only display
modes. Flat normals and simple ambient plus directional lighting make adjacent
faces readable. The edge pass uses exact topology-derived straight and circular
curves independently of face tessellation. Planar triangulation excludes inner
wires; cylinders use deterministic 24-segment derived tessellation with outward
radial normals.

Face and edge hover use distinct pre-highlight feedback; selected state remains
visible after the pointer moves. The owning Feature is synchronized without
collapsing the logical target to feature-only selection. Empty Select clicks
clear selection. Hidden results are not drawn or picked and clear affected
selection. Camera navigation, resize and display-mode changes preserve selection;
topology rebuild clears face/edge targets because runtime body and subshape
identity is intentionally nonpersistent.

Fit uses all currently presented solid triangles. Front, Back, Left, Right, Top,
Bottom and Isometric use deterministic right-handed XYZ orientations. Heads-up
Fit, Isometric and Display controls complement the View menu. In Wireframe,
edges remain primary while predictable ray-based face selection remains available.

## Automated validation

Validation level: V3.

- Fresh configure: PASS (`build-v1.1-c2`). The first sandboxed attempt could not
  fetch declared dependencies; the identical authorized network retry passed.
- Full Debug build: PASS.
- Project/dependency/linker warnings: 0.
- Full CTest: PASS, 831/831.
- Focused affected closure before V3: PASS, 51/51.
- Architecture/dependency audit: PASS; no new external dependency and no reverse
  dependency into authoritative CAD modules.
- Repository hygiene and `git diff --check`: PASS at freeze.

## Runtime and Experience Gate

The final V3 executable launches as a single `Micro SolidWorks` window. Manual
runtime evaluation passed solid readability, face/edge discoverability,
occlusion, curved and holed display/selection, multiple bodies and independent
solid Features, Fit/views, Feature Manager synchronization, visibility and
normal shutdown. Existing solids remain visible as context during New Sketch,
while active Sketch geometry retains picking priority. Experience Gate:
APPROVED.

## Known limitations

Topology selection is deliberately invalid across rebuild and is not persisted.
There is no persistent topological naming, multi-selection, Sketch on Face, Cut,
Hole feature, fillet/chamfer, materials, textures, PBR, section or measurement
tool. Face tessellation and CPU picking target the current Line/Arc/Circle and
Plane/Cylinder feature subset; no general tessellation or acceleration framework
is introduced. SolidWorks-style Selected Contours remains a separate D14
modeling-workflow follow-up; it is not part of the C2 face/edge visualization
and selection contract.
