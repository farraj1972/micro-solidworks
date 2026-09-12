# Micro SolidWorks — Architecture

## 1. Objectivo

Definir as fronteiras arquitecturais iniciais do Micro SolidWorks sem antecipar complexidade desnecessária.

A arquitectura deverá permitir evolução incremental e substituição controlada de implementações.

---

## Roadmap realization and Vertical Slice First

Canonical roadmap phase != necessarily one historical implementation baseline.
The authoritative B0–B14 MVP sequence and gap register are in
[ROADMAP.md](ROADMAP.md). Technical B0–B4 remain FROZEN; canonical B0/B1/B2
are satisfied, B3/B4 partially satisfied, B5 satisfied by technical B4, and B6
Transformations is FROZEN. This does not rename or reopen
historical baselines and does not require duplicate B5 implementation.

Feature over performance. Working vertical slice over speculative completeness.
Architecture boundaries over premature abstraction. Prefer observable authorized
slices, preserve ADRs/dependency direction, map early capabilities explicitly
and defer subsystem completeness until it is a real prerequisite. Gaps are
closed through new authorized work while preserving compatibility.

Canonical B3 still lacks Circle and basic intersections. Canonical B4 identity
is partially realized through visual IDs; B6 supplies entity transforms. Explicit visibility
state and lifecycle beyond construction/lookup remain incomplete. B4 FROZEN
therefore describes its accepted technical slice, not full Scene & Object Model.

D5 targets Point3/Segment3/Line3 first: select -> edit transform
-> move/rotate/scale -> updated rendering -> picking follows -> coherent
hover/selection/highlight. Translation, rotation, positive scale, local/world
transforms and minimal editing UI are frozen D5 decisions.
Gizmo, hierarchy, parent-child transforms, undo/redo, persistence, CAD Document
and Topology are not prerequisites for this initial slice. The B7 prerequisite
review is complete; Circle must exist before the B8 slice requiring circles/arcs.

D5 — Transformation Semantics is FROZEN in accepted ADR-0021–0024.

D6 — Topology Ownership, Identity & Orientation is FROZEN.
Accepted ADR-0025–0027 define a `Solid` aggregate owning shared indexed topology,
typed aggregate-local IDs, canonical Edge direction with oriented Edge/Face
uses, exact ID-based connectivity, and a first convex planar closed-manifold
slice. Geometry associations remain explicit and minimal: Vertex owns Point3,
Edge derives Segment3 from Vertex IDs, and Face stores an oriented support Plane
with one outer Wire. The proposed dependency is
`microsw_topology -> microsw_geometry -> microsw_math`; Topology remains
model-space and independent of Presentation transforms. B7 is IMPLEMENTED /
BASELINE CANDIDATE and awaits final validation.

Geometry remains transform-free; local Geometry plus a Presentation-owned
`Transform3` derives world representation. The implemented flat dependency graph is:

```text
Presentation -> Geometry -> Math
Presentation ------------> Math
Viewer -> Presentation
Viewer -> Rendering
```

This remains acyclic. B6.1 provides the Math `Transform3` value. B6.2 gives each
`VisualEntity` an identity transform by default, owned by value and exposed by
`const Transform3& transform() const noexcept` and `setTransform(const Transform3&)`.
Replacing it preserves the visual ID and canonical local Geometry. Existing
`GeometryPresentation::add()` overloads and read-only lookup remain unchanged.
`worldGeometry()` derives Point, Segment and normalized Line values using TRS.
Viewer adapters upload derived world vertices; `uModel` is identity because TRS
has already been applied on CPU. Picking consumes the same world semantics,
including view-derived finite Line endpoints. This accepted ADR-0024 option
keeps Rendering generic and avoids duplicate transform application.

`GeometryPresentation::setTransform()` validates world derivation before committing.
The selected-entity UI edits translation, Euler degrees (converted to radians)
and positive scale. Application preflights the edit against Viewer numeric limits
before commit, then updates navigation, hover, selection and drawing in that order.
Selection identity is retained; hover is recomputed every frame. Numeric failures
are shown in the editor and leave the previous model intact. A float-range
headroom check belongs to Viewer; Math/Geometry do not acquire GPU restrictions.
See [B6_VALIDATION.md](B6_VALIDATION.md) for conformance and validation evidence.

Validation and governance workflow are normative in `AGENTS.md`. Validation is
selected by affected dependency closure: V0 documentation, V1 localized change,
V2 cross-layer integration and V3 baseline/full-regression risk. This workflow
does not alter module boundaries or accepted ADRs.

## 2. Arquitectura planeada

```text
Application
    |
    +-- UI
    +-- Interaction
    +-- Persistence
            |
            v
         Document
            |
            v
         Modeling
            |
            v
         Topology
            |
            v
         Geometry
            |
            v
           Math
```

Rendering consome representações apropriadas do modelo, mas não define o modelo CAD.

Persistence serializa o Document através de uma fronteira própria.

Esta é a direcção lógica planeada. Os módulos de domínio apresentados nesta
secção (Document, Modeling e Topology) continuam por implementar. Geometry já
existe na baseline B3 FROZEN. As secções seguintes distinguem FROZEN
FOUNDATIONS, FROZEN B2/B3 IMPLEMENTATION, B4 FROZEN e PLANNED / DEFERRED.

### FROZEN FOUNDATIONS — B1 Math Foundation (B1.1–B1.8)

B1 is FROZEN; B0 remains a previous frozen stable baseline. The implemented
Math module lives in `src/core/math`, under namespace `microsw::math`.
Its project-owned CMake target has only this dependency:

```text
microsw_math -> C++ standard library only
```

It is independent of logging, windowing, rendering and UI, including GLFW,
GLAD, OpenGL, Dear ImGui and spdlog. Math is tested through the existing
`micro_solidworks_tests` target. B2 extends the application shell with a
viewer without changing the frozen Math foundation.

#### Scalar and numerical comparison

`microsw::math::Scalar` is an alias for `double`. `Tolerance.h` provides
the free functions `almostEqual` and `isNearlyZero`, with defaults:

```cpp
defaultAbsoluteTolerance = 1.0e-12;
defaultRelativeTolerance = 1.0e-12;
```

For finite scalar operands, `almostEqual(a, b)` combines absolute and relative
tolerance as defined in ADR-0007:

```text
|a - b| <= max(absoluteTolerance, relativeTolerance * max(|a|, |b|))
```

`isNearlyZero(value)` uses absolute tolerance only. Negative tolerances throw
`std::invalid_argument`. NaN operands do not compare equal; equal signed
infinities compare equal, but infinities never compare nearly zero.

These are numerical comparison tolerances. They are NOT CAD geometric
modelling tolerance. A global geometric modelling tolerance remains deferred.

#### Vector2 and Vector3

Both concrete types provide zero default construction, construction from
components, read-only component access (`x()`, `y()`, and `z()` for
`Vector3`), addition, subtraction, unary negation, scalar multiplication in
both orders, scalar division, `squaredLength()`, `length()`,
`normalized()`, `dot` and component-wise `almostEqual`.
Normalization returns a new vector. Division by an effectively zero scalar
and normalization of a zero or nearly-zero-length vector throw
`std::domain_error`.

`Vector3` additionally provides `cross`. The executable right-handed
invariant is `cross(X, Y) = Z`, or `X × Y = Z`, for the unit basis.

Math types are unit-agnostic. D1 defines XY as the CAD horizontal/base plane
and Z as vertical, millimetres at the future CAD/document boundary, radians
internally and degrees at the UI boundary. Document units and angular UI
conversion are conventions for future consumers, not implemented features.
Rendering must adapt to CAD Math, not the reverse.

#### Matrix3 and Matrix4

Both concrete types use zero-based `matrix(row, column)` access.
An out-of-range row or column throws `std::out_of_range`. Explicit constructor
arguments describe successive mathematical rows, independently of storage.

| Type | Implemented operations |
| --- | --- |
| `Matrix3` | Zero default, construction from 9 scalars, `identity()`, `Matrix3 * Vector3`, `Matrix3 * Matrix3`, `transposed()`, `determinant()`, `almostEqual` |
| `Matrix4` | Zero default, construction from 16 scalars, `identity()`, `Matrix4 * Matrix4`, `transposed()`, `almostEqual` |

Matrices use column vectors and the column-major logical convention of
ADR-0009. Vector application follows `v' = M * v`; multiplication follows
`C(row, column) = sum over k of A(row, k) * B(k, column)`.
The identity preserves supported vector and matrix products on the
applicable sides.

Logical semantics are independent of physical memory layout. Physical
storage is private, not a public contract. Free `almostEqual(matrixA, matrixB)`
compares corresponding elements with the scalar tolerance policy; approximate
equality is not exposed through `operator==`.

`Matrix4` has no `determinant()` or `inverse()`.
`Matrix4 * Vector3` is intentionally NOT provided.

#### Transformation Operations

`Transformations.h` provides free functions returning `Matrix4`:

- `translation(const Vector3& offset)`;
- `scaling(const Vector3& factors)`;
- `rotationX(Scalar radians)`, `rotationY(Scalar radians)`,
  `rotationZ(Scalar radians)`.

Rotations are right-handed and angles are in radians. Scaling accepts
per-axis factors, including zero and negative values. Composition
`T * R * S` applies `S -> R -> T`.

Application explicitly distinguishes two operations returning `Vector3`:

- `transformPoint(matrix, vector)`: input homogeneous `w = 1`;
  includes translation and requires resulting `w'` approximately 1.
- `transformDirection(matrix, vector)`: input homogeneous `w = 0`;
  excludes translation and requires resulting `w'` approximately 0.

These operations are affine only and never perform perspective division.
They check the resulting homogeneous coordinate for the supplied operand
using `almostEqual(w', 1)` or `isNearlyZero(w')`, respectively, and throw
`std::domain_error` if that contract fails. This is not a general validator
of every matrix element or a projective-transform API.

Using a `Vector3` argument for `transformPoint` is a temporary semantic
operation for a position, not a promotion of `Vector3` to a dedicated point
type. B3 now supplies `geometry::Point3`; callers must preserve the explicit
point/direction distinction.

#### B1.8 integration validation

The 18 tests in `tests/math/test_math_integration.cpp` validate relationships
between components, beyond isolated unit behavior. Examples include rotation
preservation of length, dot product and cross orientation; orthonormal
rotated bases; `R^T * R ≈ I` and `R * R^T ≈ I` for pure rotations;
sequential versus composed transforms; Matrix3 application associativity;
and Matrix4 multiplication associativity. They use public APIs and the
existing numerical tolerance policy, without storage assumptions.
The suite through B1.8 contains 97 tests including infrastructure tests.

#### D1 implementation validation

| Accepted ADR | Result | Implementation evidence |
| --- | --- | --- |
| ADR-0007 | CONFORMANT | `Scalar = double`; absolute/relative comparison; explicit invalid-operation failures; geometric tolerance deferred |
| ADR-0008 | CONFORMANT | Unit-agnostic types; `cross(X,Y)=Z`; right-handed rotations in radians; CAD/document and UI conventions preserved for future boundaries |
| ADR-0009 | CONFORMANT | Column-vector application; `T * R * S` order; column-major logical convention independent of memory layout; generic types and Quaternion deferred |

The formal status of all three ADRs remains ACCEPTED; D1 remains FROZEN.
This validation does not introduce document units, UI angle controls or
rendering adapters.

#### Deferred after B1

Not implemented in B1:

- `Point2`, `Point3`, `Direction3`, `Vector4`;
- generic `Vector<T,N>` and `Matrix<T,R,C>`;
- `Quaternion` and arbitrary-axis rotation;
- matrix inverse and Matrix4 determinant;
- perspective and orthographic projection;
- global geometric modelling tolerance;
- a `Transform` class, `Ray`, `Plane` and `BoundingBox`.

Deferred does not mean rejected forever. Each capability requires a future
explicitly authorized scope. B1.9 and B1.10 are complete; B1 is FROZEN.
Perspective/orthographic projection, absent from B1, are now implemented
in B2's Viewer through `ViewProjection`. B3 supplies Point/Ray/Plane in Geometry; the remaining Math concepts above
remain deferred.

### Viewer foundation (B2 FROZEN) and current B4 composition

B2 is FROZEN. B2.1–B2.13 are COMPLETE; B2.FREEZE is FROZEN.
Any subsequent increment, Decision Gate or baseline requires explicit authorization.

The Viewer observes the CAD world; it does not own CAD representation.
Current composition in `src/app/main.cpp`:

```text
Application
    +-- ApplicationWindow / OpenGLContext / ImGuiLayer
    +-- ApplicationShell
    |     +-- WorkspaceLayout / WorkspaceInput / ProjectionMode
    +-- GeometryDemoScene -> GeometryPresentation (owned by main)
    +-- WorkspaceViewport (observes GeometryPresentation)
          +-- OrbitCamera / OrbitNavigation / PanZoomNavigation
          +-- ProjectionState
          +-- ReferenceGrid / ReferenceAxes (aid generation at construction)
          +-- PresentedPoints / PresentedSegments / PresentedLines
          +-- GeometryPicker / HoverState / SelectionState / VisualState
          +-- ShaderProgram / LineRenderer / PointRenderer --> OpenGL / GLAD
```

#### Actual CMake boundaries

The table includes the current B4 Presentation boundary alongside frozen B0–B3
targets and records direct `target_link_libraries` relationships in
`CMakeLists.txt`, not an invented idealized graph. Standard C++ dependencies
are implicit.

| Target | Responsibility | PUBLIC links | PRIVATE links |
| --- | --- | --- | --- |
| `microsw_math` | Internal B1 mathematics | None | None |
| `microsw_geometry` | B3 geometric values, predicates and point metrics | `microsw_math` | None |
| `microsw_presentation` | Visual identity and Geometry value collection | `microsw_geometry` | None |
| `microsw_logging` | Project-owned `Logger` | None | `spdlog::spdlog` |
| `microsw_windowing` | `ApplicationWindow`, GLFW/window/context lifetime | None | `glfw`, `microsw_logging` |
| `microsw_rendering` | `OpenGLContext`, `ShaderProgram`, `LineRenderer`, `PointRenderer` | `microsw_math` | `glad_gl_core_33`, `microsw_logging`, `microsw_windowing` |
| `microsw_viewer` | Camera, adapters, picking, interaction, aids and Workspace pass | `microsw_math` | `microsw_presentation`, `microsw_rendering`, `glad_gl_core_33` |
| `microsw_imgui_backend` | Dear ImGui and GLFW/OpenGL3 backends | None | `glfw` |
| `microsw_ui` | `ImGuiLayer` and `ApplicationShell` | None | `glfw`, `microsw_imgui_backend`, `microsw_logging`, `microsw_rendering`, `microsw_windowing` |
| `micro_solidworks` | Application composition | None | `microsw_viewer`, `microsw_logging`, `microsw_rendering`, `microsw_ui`, `microsw_windowing` |
| `micro_solidworks_tests` | GoogleTest/CTest, including real-context tests | None | `GTest::gtest_main`, `microsw_geometry`, `microsw_logging`, `microsw_math`, `microsw_presentation`, `microsw_viewer`, `microsw_rendering`, `microsw_windowing`, `glad_gl_core_33`, `glfw` |

`glad_gl_core_33` is the generated OpenGL 3.3 Core loader. OpenGL is supplied
by the system driver. Rendering's Windowing dependency supports context
bootstrap; the Viewer uses GLAD privately to coordinate its render pass.
Rendering does not depend on Viewer and has no camera, navigation, grid, axis
or CAD semantics. Math remains independent of every graphical layer.

The shared headers under `src/app` are application contracts, not another
library target. UI does not link to Viewer; no ImGui types cross that boundary.
The executable also compiles GeometryDemoScene.cpp. It consumes Presentation
through the static Viewer link closure; it has no direct Presentation link in
target_link_libraries. The test target links Presentation explicitly.

#### UI and Workspace contracts

`ApplicationShell` owns menus, the Model panel, status bar, About modal and
logical Workspace layout/input snapshot. It does not own a camera, projection
state, render primitives or CAD model.

- `WorkspaceLayout`: logical display coordinates, top-left origin relative
  to the main application viewport, plus logical display dimensions.
- `WorkspaceInput`: per-frame pointer/button/modifier/wheel snapshot with
  focus, pointer-validity, Workspace-hover and UI-blocking decisions.
  `leftPressed` is a one-frame left-button edge; optional `projectionRequest` is a one-frame command.
- `ProjectionMode`: shared project-owned enum in `src/app/ProjectionMode.h`,
  with `Perspective` and `Orthographic`. This location permits UI indication
  and requests without a UI-to-Viewer class dependency.

The menu is **View → Projection → Perspective / Orthographic**. Main passes
`workspace.projectionMode()` into `shell.draw(...)` for checked indication;
UI emits a request, consumed by `WorkspaceViewport::updateNavigation`.
The only source of truth is WorkspaceViewport's `ProjectionState`, not a
UI-owned mirror. UI contains no picking/selection algorithm or visual IDs.
About still displays the historical 'Baseline B0 - Foundation' caption; it is
not a current baseline status display. Explicit projection requests apply independently of blocked
pointer navigation, allowing menu commands while menus capture the mouse.

#### Camera, view and projection

`OrbitCamera` stores `target`, `distance`, `yaw` and `pitch`; `position`,
`forward`, `right` and `up` are derived. Defaults are target (0,0,0),
distance 10, yaw -pi/4 and pitch pi/6. Coordinates remain right-handed, XY
base plane, Z vertical, world up +Z; angles are radians.

Forward points toward the target, right = normalize(cross(forward,+Z)),
and up = cross(right,forward). The right-handed view basis is
(right, up, -forward). Yaw is unwrapped. Pitch is clamped to
+/- (pi/2 - 1e-4) to preserve a valid basis near the poles. Non-finite state
inputs are rejected; distance must be positive and finite. Defaults and pole
margin are implementation parameters, not new architectural commitments.

`ViewProjection.h/.cpp` implements pure BUILD mathematics using B1 types:
`viewMatrix(camera)`, `perspective(verticalFov, aspect, near, far)` and
`orthographic(visibleHeight, aspect, near, far)`. View maps eye to origin,
right to +X, up to +Y, forward to -Z and target to (0,0,-distance).
Column-vector semantics remain `v' = M * v`:

```text
clip = Projection * View * Model * position
```

Viewer aids currently use Model = Identity. Both projections use the same
view matrix and positive clipping distances, near > 0 and far > near.
View-space z = -near maps to NDC z = -1; -far maps to +1. Aspect is framebuffer
viewport width / height. Perspective uses vertical FOV; Orthographic uses
visibleWidth = visibleHeight * aspect. Current Workspace parameters are
vertical FOV pi/3, near 0.1 and far 1100 in both modes; they are implementation
parameters, not adaptive clipping or permanent architectural constants.
No matrix inverse or production Vector4 is introduced.

`ProjectionState` owns current mode (default Perspective) and orthographic
`visibleHeight` (default 10, finite and positive). Its `matrix(...)` selects
the existing pure projection functions without duplicating their formulas.
Perspective distance belongs to OrbitCamera; Orthographic visibleHeight
belongs to ProjectionState. These are independent zoom states.
Switching mode preserves target, yaw, pitch, distance and visibleHeight.
It does not reset pose or automatically match apparent scale between modes.

#### Navigation

All sensitivities and bounds below are current implementation/UX parameters,
not Math-core constraints or additional frozen decisions.

| Input | Component and effect |
| --- | --- |
| MMB drag | `OrbitNavigation`: yaw/pitch change; target/distance fixed; 0.005 radians per logical pixel; right increases yaw, up increases pitch |
| Shift+MMB drag | `PanZoomNavigation`: target translates in the camera image plane; derived position follows the same translation; orientation/distance unchanged |
| Mouse wheel | `PanZoomNavigation`: multiplicative zoom of distance in Perspective or visibleHeight in Orthographic, never both |

Pan reference scale is distance * 0.0015 in Perspective and visibleHeight *
0.0015 in Orthographic. Scene motion follows the pointer using camera right/up.
Zoom uses sensitivity 0.15, equivalent to scale * exp(-wheelDelta * 0.15),
evaluated in log space to clamp extreme input safely. Wheel up reduces scale
(zoom in); wheel down increases it. Both navigation ranges are currently
0.1..1000, with distinct distance/height meanings. Orbit works identically
in both modes and does not change either zoom state.

A drag starts only on a fresh MMB press inside the Workspace with UI hover
permission. Its first frame anchors without moving. Captured drags may
continue outside; release, focus loss, invalid pointer or blocked UI/modal
cancels them. Pressing Shift ends orbit; releasing Shift ends pan.
Neither converts implicitly to the other while MMB remains held: a fresh
release/press is required. Wheel always requires Workspace hover permission
and is ignored while blocked, unfocused or pointer-invalid. These are
project-owned input semantics, not an ImGui-internals contract.

#### Direct Workspace render pass and lifetime

`WorkspaceViewport` owns camera, projection and navigation state, its shader
and GPU resources: four aid LineRenderers, two Geometry LineRenderers and one
PointRenderer. The latter three are reused for successive visual-state batches.
ReferenceGrid/ReferenceAxes generate and upload aids at construction; Geometry
batches derive from the observed Presentation per frame. Hover and selection
are Viewer identity states, not CAD ownership or a scene graph.

Main clears the frame, composes the UI/input snapshot, updates navigation and
projection, updates hover, updates selection, renders the Workspace, renders
ImGui chrome over it, then swaps buffers. Interaction uses the updated view
of the same frame, including simultaneous zoom or projection requests.
Dear ImGui reserves a Workspace with no background covering the 3D pass.
`framebufferRect()` clips logical bounds to the display, scales to framebuffer
pixels (including HiDPI and independent horizontal/vertical scale), rounds
inward and converts top-left to OpenGL bottom-left coordinates. The result
drives `glViewport`/`glScissor`; invalid, empty or offscreen rectangles are
no-ops. Resize recomputes aspect/projection, not camera pose or aid geometry.
Offscreen framebuffer / texture-backed viewport: NOT IMPLEMENTED / DEFERRED.

The pass clears only Workspace color/depth, enables scissor and depth testing
with GL_LESS for normal geometry and GL_LEQUAL for hovered/selected batches, enables color/depth writes and disables blending. Its local RAII
guard restores viewport, scissor box, depth function, program, VAO,
scissor/depth/blend enables, write masks and clear values, including exception
paths. This is not a global state manager or render graph.

OpenGLContext loads/validates OpenGL through ApplicationWindow and supplies
whole-frame background clearing. GPU objects require a current compatible
context and loaded GLAD throughout their lifetime. Main destroys Workspace
GPU resources before ImGui and before the GLFW window/context.

#### ShaderProgram, LineRenderer and PointRenderer

`ShaderProgram` is a project-owned PImpl, move-only RAII wrapper. It compiles
vertex/fragment source strings, links, reports failures with driver logs,
releases intermediate shader objects and exposes explicit `bind()`,
`setMatrix4` and `setVector3`. Viewer shaders are embedded GLSL 330 Core
unlit shaders; file loading, caching and hot reload are absent.
Uniform upload requires the program bound and a valid active uniform.

The public API has no GLuint, GLenum or GL headers. Math Scalar remains
double. Upload validates finite float-representable values and converts
explicitly at the rendering boundary. Matrix upload reads
`Matrix4(row,column)`, builds column-major `GLfloat[16]` and calls
`glUniformMatrix4fv(..., GL_FALSE, ...)`; it never assumes Math storage layout.
Vector3 uniforms provide a uniform color per batch, not per-vertex colors.

`LineRenderer` is also PImpl, move-only RAII, owning VAO/VBO. It accepts CPU
`Vector3` pairs through `setVertices`, validates them, converts double to float
and uploads GL_ARRAY_BUFFER with GL_DYNAMIC_DRAW. Attribute location 0 is
three tightly packed GL_FLOAT position components; `draw()` uses GL_LINES.
There is no EBO, indexed rendering or mesh semantics. Shader, color, camera,
projection and depth policy are caller responsibilities; LineRenderer has no
grid or axis knowledge. Empty draws are no-ops. Construction/upload preserve
the bindings they touch; the enclosing Workspace pass restores its draw state.

`PointRenderer` follows the same move-only VAO/VBO RAII boundary and explicit
Scalar/double-to-float conversion. It draws GL_POINTS with fixed MVP size 5.0,
restoring the previous point size. LineRenderer serves grid, axes, Segments
and finite Line visualizations. Neither renderer nor ShaderProgram knows
VisualEntityId, GeometryPresentation, hover, selection or VisualState semantics.

#### Reference aids

`ReferenceGrid` is finite, uniform and static in XY at Z=0. Defaults
halfExtent=10 and spacing=1 produce 40 segments / 80 vertices. Central x=0
and y=0 lines are omitted so ReferenceAxes owns the visual origin orientation.
The neutral batch color is currently (0.35,0.35,0.38). Generation and upload
occur once per Workspace construction, not during navigation or resize.
It is not infinite, adaptive or CAD geometry.

`ReferenceAxes` supplies three positive semi-axes sharing the origin:
origin→+X, origin→+Y, origin→+Z, current length 3. Colors are X red, Y green,
Z blue. There are no negative axes, arrowheads or labels.
Grid, X, Y and Z draw in that order in the same depth pass, not as an
artificial overlay. Both aids are visual references, not sketch entities,
construction geometry or domain scene entities.

#### Historical B2 testing and D2 validation

B2.11's validated snapshot is 274 tests, 274 PASS, 0 FAIL; it is not a
permanent test-count promise. Coverage combines pure math/state, camera,
projection, navigation, Workspace and real OpenGL-context rendering tests
in `tests/viewer` and `tests/rendering`, plus manual application validation.

`test_viewer_integration.cpp` adds 13 integration tests covering camera/view,
orbit/pan, both zoom modes, independent state across switching, NDC depth,
grid/axes upload and shader/line drawing, Workspace/HiDPI/resize, empty
surfaces, gesture transitions, UI/focus/pointer blocking and extreme pitch/zoom.
It uses production components and real OpenGL, not mocks or pixel comparisons.
B2.11 manual validation confirmed both projections/navigation, switching,
grid/axes, About/UI isolation, resize/maximize/minimize/restore and both close
paths. B2.12 changes documentation only.

| Accepted ADR | Audit against current implementation | Evidence |
| --- | --- | --- |
| ADR-0010 | CONFORMANT | OrbitCamera +Z basis, OrbitNavigation, PanZoomNavigation, Workspace input gating, distance/height zoom, no CAD ownership or picking |
| ADR-0011 | CONFORMANT | Internal ViewProjection, ProjectionState, column-vector P*V*M shader pipeline, vertical FOV/visibleHeight and NDC depth tests; inverse deferred |
| ADR-0012 | CONFORMANT | ApplicationWindow OpenGL 3.3 Core, GLSL 330 Core, encapsulated VAO/VBO/programs, direct viewport/scissor/depth pass, XY grid/RGB axes, no offscreen target or scene graph |

ADR-0001 through ADR-0012 exist: 12/12 retain Status: ACCEPTED, unchanged.
D0/D1/D2 remain FROZEN. Their historical contexts describe the decisions at
acceptance, not today's implementation status. PROJECT_CHARTER remains
consistent and requires no change.

#### PLANNED / DEFERRED — not implemented

- Viewer: offscreen framebuffer/texture-backed viewport; infinite/adaptive
  grid, major/minor styling, fading, labels and snap; negative reference axes,
  arrows and labels; orientation/view cube; standard Top/Front/Right/Isometric
  commands; camera reset/home, Fit View/Zoom Extents, animation, inertial
  navigation and configurable input bindings.
- Rendering: shader file loading, hot reload/cache; mesh/triangle renderer,
  EBO/indexed rendering and per-vertex colors for viewer content; lighting,
  materials, PBR and scene textures. Dear ImGui's own backend drawing/font
  resources do not constitute these Viewer capabilities.
- CAD/domain: topology, scene graph, CAD tessellation and persistent CAD
  entity identity. B4 implements visual-entity picking, hover, single-selection
  and highlighting of B3 values through Presentation; this is not CAD ownership.
- Math: production Vector4, generic Vector/Matrix, Matrix4 determinant,
  matrix inverse, Quaternion, arbitrary-axis rotation, global geometric
  modelling tolerance inside Math, dedicated Direction types and BoundingBox.
  Point, Ray and Plane now exist in Geometry, not Math; B1 stays unchanged.
  Local homogeneous helpers in Viewer/tests do not introduce a public Math Vector4.

No B1 point/direction API is changed. Future CAD representation remains
authoritative and will pass through tessellation to derived render
representation and Rendering/OpenGL. Aids generate render primitives directly
only because they are not CAD representation.

---

## 3. FROZEN FOUNDATIONS — Implementação original da baseline B0

```text
Application
    |
    +-> Windowing -> GLFW
    +-> Rendering -> GLAD/OpenGL 3.3 Core
    +-> UI -> Dear ImGui
```

As fronteiras concretas são targets CMake project-owned:

- `microsw_logging`: `Logger`, com spdlog isolado na implementação;
- `microsw_windowing`: `ApplicationWindow`, com GLFW isolado;
- `microsw_rendering`: `OpenGLContext`, responsável pelo bootstrap OpenGL;
- `microsw_ui`: `ImGuiLayer` para lifecycle e `ApplicationShell` para composição.

O executável `micro_solidworks` compõe estas fronteiras. GoogleTest e CTest
fornecem a infraestrutura de testes. B0 não contém math, geometry, topology,
modeling, document, interaction ou persistence implementados.

---

## 4. Módulos actuais e planeados

### app

Responsável por:

- bootstrap;
- composição da aplicação;
- lifecycle;
- wiring dos componentes.

### core/math

Matemática independente do domínio CAD.

Implementado em B1.1–B1.8: Scalar, Tolerance, Vector2, Vector3, Matrix3,
Matrix4 e Transformation Operations, conforme a secção Math acima.
Os conceitos ainda não implementados estão separados em "Deferred after B1".

### core/geometry

#### FROZEN B3 IMPLEMENTATION — Geometric Primitives (D3 FROZEN)

B3 is FROZEN. B3.1–B3.9, the corrective B3.10A and the repeated B3.10 are
COMPLETE; B3.FREEZE is FROZEN. B3.10A corrected closest-point reconstruction
robustness identified during the first B3.10 validation.

Geometry is implemented in `src/core/geometry`, namespace
`microsw::geometry`, using the project-owned `microsw_geometry` target.
Its real dependency graph is:

```text
microsw_geometry
    |
    v
microsw_math
    |
    v
C++ standard library
```

Math does not depend on Geometry. Geometry does not depend on Viewer,
Rendering, UI, Windowing or Logging. B4 now consumes Geometry through
microsw_presentation and Viewer adapters; the application owns the demo
Presentation. Grid and axes remain independent viewer aids.

The conceptual foundation-to-consumer flow is Math -> Geometry -> future
Topology / Modeling / CAD. These are not reversed dependency arrows.
Topology, Modeling and CAD entities remain NOT IMPLEMENTED.

Geometry currently supplies finite positions, canonical geometric primitives,
predicates, point-to-primitive metric/projection operations and a geometric
tolerance policy. Types are small project-owned, copyable/movable values,
valid on construction, with read-only invariant-bearing state. There is no
inheritance between primitives and no stored query cache.

##### Point2 / Point3

Point2 stores x/y Scalars; Point3 stores x/y/z Scalars. Default construction
is the origin. Coordinates must be finite. Points are distinct from B1
Vector2/Vector3; no implicit conversion or inheritance conflates positions
with displacements.

| Operation | Result |
| --- | --- |
| Point - Point | Vector |
| Point + Vector | Point |
| Vector + Point | Point |
| Point - Vector | Point |

There is no Point + Point, Point * Scalar, Point / Scalar or approximate
operator==. `areCoincident(a,b,tolerance)` compares Euclidean separation
against a finite non-negative geometric tolerance, without relative tolerance.
Zero tolerance requires identical coordinates.

##### Primitive representations and operations

| Types | Stored state / domain | Implemented operations |
| --- | --- | --- |
| Segment2 / Segment3 | Endpoints A and B only; parameter [0,1] | a, b, length, squaredLength, midpoint, direction, isDegenerate, pointAt, contains, closestPoint, distance, isParallel, isPerpendicular |
| Line2 / Line3 | Origin + unit direction; parameter in R | origin, direction, pointAt, contains, closestPoint, distance, isParallel, isPerpendicular |
| Ray2 / Ray3 | Origin + unit direction; parameter >= 0 | origin, direction, pointAt, contains, closestPoint, distance, isParallel, isPerpendicular |
| Plane (3D only) | Origin + unit normal | origin, normal, contains, closestPoint, distance, signedDistance, isParallel, isPerpendicular |

Line/Ray directions and Plane normals are normalized at construction.
Their input Euclidean magnitude must exceed 1e-9; zero, near-zero and
non-finite inputs are rejected. These types have no invalid default state.

Segment permits coincident or approximately coincident endpoints.
`isDegenerate` uses endpoint coincidence; `direction` uses the default
geometric tolerance and throws domain_error for degeneracy. Same-type
parallel/perpendicular relations consequently also reject a degenerate operand.
Midpoint and convex `pointAt(t)` preserve Point semantics and avoid naive
overflow in A+B or B-A. For exactly equal endpoints, every valid pointAt
returns A. For distinct but approximately coincident endpoints, pointAt
still interpolates the endpoints; it does not snap them to A.
The metric degeneracy policy uses the default geometric tolerance:
closestPoint returns A and distance remains the Euclidean distance to A.

For a non-degenerate Segment, closestPoint clamps the projected parameter
to [0,1]. Line projection has no domain clamp. Ray projection clamps a
negative projected parameter exactly to its origin, not according to geometric
tolerance. Thus a point 5e-10 behind a Ray origin can satisfy default contains
but still have its closest point at the origin and a non-zero distance.

Representation is not equivalence: different origins can represent the same
Line; +D/-D preserve the same line point set with opposite parameter
orientations. Opposite Ray directions describe different half-lines.
Plane is conceptually dot(P-origin,normal)=0. Plane(O,N) and Plane(O,-N)
have the same point set with opposite orientation: signedDistance changes
sign, distance remains its absolute value and closestPoint is geometrically
unchanged. No primitive-equivalence API is implemented.

##### Predicates, metrics and tolerance

`pointAt`, `contains` and relation queries derive from canonical state.
Contains tests support-line/plane residuals and, where applicable, domain
bounds. Segment contains uses its supplied tolerance to decide degeneracy;
its metric operations use the default degeneracy policy described above.

Same-type `isParallel` and `isPerpendicular` exist for Segment, Line, Ray
and Plane (matching dimensions). They compare dimensionless unit-vector
cross/dot residuals using the geometric default as their explicit threshold;
this is not a general conversion of length tolerance into angular units.
There is no cross-type relation matrix (Line-Ray, Plane-Line, etc.).

```text
Geometry defaultGeometricTolerance = 1e-9
Math defaultAbsoluteTolerance      = 1e-12
Math defaultRelativeTolerance      = 1e-12
```

Geometric tolerance is not numeric tolerance. The initial Geometry/kernel
length scale is interpreted in millimetres at the future document boundary;
1e-9 mm is a kernel policy, not a physical-accuracy guarantee. Types do not
embed units. Math's numeric tolerances are unchanged.

Predicates areCoincident, isDegenerate, contains, isParallel and
isPerpendicular accept finite non-negative tolerance; zero uses the exact
computed residual (not approximate equality). Floating-point generation of a
diagonal point may leave a residual, so zero-tolerance containment is not a
promise of symbolic exact arithmetic.

Metric APIs follow one public convention: `operation(primitive, point)`.
Only point-to-Segment/Line/Ray/Plane distance and closestPoint are provided,
plus Plane signedDistance. No reverse overload set, public projection
parameter, public Point-Point distance, or primitive-primitive metric exists.
Metrics have no tolerance parameter and do not snap small results to zero.
For an applicable perpendicular offset of 5e-10, contains may be true while
distance remains approximately 5e-10. The Segment degeneracy policy is an
explicit domain rule, not general metric-output snapping.

##### Numerical support and failures

`GeometryQuerySupport` is an internal implementation detail shared by
primitive implementations, not a public conceptual query API or new Scalar.
Binary-scaled intermediates support robust offsets, dot/cross residuals,
norms, projection and metric reconstruction where ordinary intermediate
differences or products could overflow. Reconstruction avoids selected
extreme-origin cancellation cases; this is not arbitrary-precision arithmetic
or a guarantee against all floating-point loss. It does not depend on a
wider native long double on MSVC.

Public geometric state stays finite. A representable closest point can still
exist when distance overflows, and a finite perpendicular distance can exist
when the closest point is unrepresentable; these operations are checked
independently where supported.

| Failure | Exception / examples |
| --- | --- |
| Invalid input | std::invalid_argument: non-finite coordinates, directions/normals or parameters; invalid tolerance; near-zero Line/Ray direction or Plane normal |
| Undefined operation | std::domain_error: degenerate Segment direction/relations; pointAt outside Segment/Ray domain |
| Non-representable result | std::overflow_error: point arithmetic, lengths, pointAt or metric/projection result overflow |

No silent inf/NaN is returned as valid geometric state or metric output.
Queries and operations do not mutate their operands.

##### Geometry / Topology / CAD / Rendering boundaries

Point3 != Vertex; Segment3 != Edge; Plane != Face. Line/Ray are not CAD
construction entities merely because they are geometric values.
Geometry has no topological connectivity, incidence, adjacency, loops,
shells or topological ownership. Primitive direction/normal orientation is
not a topological orientation model.

Geometry has no IDs, names, selection state, document/feature ownership,
history, constraints or persistence metadata. Public APIs expose no OpenGL,
GLAD, GLFW, Dear ImGui, spdlog, VAO, VBO, Shader, Color, LineWidth or Render/Draw
methods. Future CAD representation remains authoritative, flowing through
tessellation to derived render representations; Geometry owns no GPU resources.

Not implemented: general or individual Line-Line/Ray-Ray/Segment-Segment/
Plane-* intersections; primitive equivalence (sameLine/samePlane/sameRay/
sameSegment); cross-type relation matrices; primitive-primitive distances or
closest points; Vertex/Edge/Face/Wire/Shell/Solid, Topology/BRep;
Document/Sketch/Feature/constraints, extrude/revolve/booleans, CAD sub-selection,
persistent CAD IDs/history. Geometry rendering integration now exists externally
in B4; it does not change the B3 model. The B3 freeze authorizes no future work.

##### Historical B3 testing and D3 validation

B3.10's final validated snapshot is 549 tests, 549 PASS, 0 FAIL, with no
findings. It includes primitive/unit, invariant, tolerance, query,
metric/projection, B3.10A reconstruction regression and extreme-coordinate
tests. The 31 tests in
`tests/geometry/test_geometry_integration.cpp` cross Point/Vector semantics,
Segment degeneracy, Line projection and origin invariance, Ray domain and
predicate-vs-metric distinctions, Plane signed orientation and tangent-origin
invariance, parallel/perpendicular relations, extreme coordinates, overflow
contracts and 2D/3D consistency. Runtime smoke validates startup, a visible
unchanged B2 viewer, normal close and exit code 0, not rendering of Geometry.

| Accepted ADR | B3 implementation audit | Evidence |
| --- | --- | --- |
| ADR-0013 | CONFORMANT | Point2/3 finite Scalar state, distinct Point/Vector arithmetic, areCoincident, no inheritance or ambiguous Point operators |
| ADR-0014 | CONFORMANT | Endpoint-only Segment; origin + normalized direction/normal for Line/Ray/Plane; derived queries without redundant state; no equivalence/intersections |
| ADR-0015 | CONFORMANT | Separate 1e-9 geometric and 1e-12 numeric policies, finite validation, explicit degeneracy/exceptions, no metric-output snapping |
| ADR-0016 | CONFORMANT | Geometry -> Math only; no topology/CAD identity or rendering ownership; application/Viewer do not consume Geometry |

ADR-0001 through ADR-0016 remain 16/16 ACCEPTED, unchanged. D0/D1/D2/D3
remain FROZEN. B3 is FROZEN. PROJECT_CHARTER remains consistent and unchanged.

### B4 — Geometry Visualization & Selection (FROZEN, D4 FROZEN)

B4.1–B4.12 are COMPLETE; B4.FREEZE is FROZEN. B4 is the latest stable
baseline. B0/B1/B2/B3 and D0/D1/D2/D3/D4 remain FROZEN.
Next functional area: canonical B6 after frozen D5 — Transformation Semantics.
B6 is NOT STARTED.
The B4 boundaries and interaction contracts below are frozen; changes require
explicit authorization.

#### Presentation, identity and ownership

`microsw_presentation` contains `VisualEntityId`, `PresentedGeometry`,
`VisualEntity` and `GeometryPresentation` under `src/presentation`.
`PresentedGeometry` is exactly `variant<Point3, Segment3, Line3>`.
A VisualEntity owns an external visual identity plus a Geometry value;
it is not a CAD entity. GeometryPresentation is an insertion-ordered value
collection with add/find/read-only access, not a CAD Document or scene graph.
It contains no rendering resources, colors or interaction state.

VisualEntityId is a strong `uint64_t` value type. Zero is reserved/invalid;
explicit zero construction throws. The process-local generator starts at 1,
increases monotonically within its collection/generator, never reuses generated
IDs during that generator's lifetime and throws after exhaustion. It is not
thread-safe. Identity is non-persistent, not globally unique and not cross-session;
separate collections and copied values do not gain global uniqueness.

`main.cpp` owns the GeometryPresentation returned by `createGeometryDemoScene()`.
WorkspaceViewport observes it through a const pointer and must be destroyed
before it. VisualEntity owns its Geometry value. Interaction IDs are valid
for the lifetime of the observed presentation. There is no remove/update,
orphan reconciliation, event system or persistence lifecycle.

The actual target dependency graph (`A -> B` means A consumes B) is:

```text
microsw_presentation -> microsw_geometry -> microsw_math
microsw_viewer -> microsw_presentation
              -> microsw_rendering -> microsw_math
              -> microsw_math
```

The complete infrastructure links are in the CMake table above. There are no
cycles or reverse links: Geometry !-> Presentation; Presentation !-> Viewer;
Presentation !-> Rendering; Rendering !-> Presentation. Rendering is generic.
Geometry remains model-only: Point3/Segment3/Line3 have no visual identity,
color, hover, selection, GPU state, render methods or screen/pixel semantics.
Point3 != Vertex, Segment3 != Edge and Plane != Face remain true (ADR-0016).

The following is data/responsibility flow, not reverse target dependencies:

```text
Geometry values -> Presentation -> Viewer adaptation / interaction / orchestration
                                      -> Rendering -> OpenGL
Pointer + current view -> Picking -> Hover / eligible click Selection
                                      -> VisualState -> Highlight batches
```

#### Viewer adapters and finite Line representation

`PresentedPoints`, `PresentedSegments` and `PresentedLines` derive renderable
Vector3 batches from Presentation, preserving insertion order within each batch:

| Geometry value | Derived vertices |
| --- | --- |
| Point3 | One position |
| Segment3 | A/B pair, including coincident endpoints |
| Line3 | Finite view-derived pair |

Line3 remains mathematically infinite. LinePresentationContext contains the
current view center and positive finite visible scale. The adapter finds the
support point nearest the view center, then generates endpoints at plus/minus
three times visibleScale along the unit Line direction. Workspace supplies
camera.target() and `visibleHeight * max(1, rasterAspect)`, where visibleHeight
is `2 * camera.distance() * tan(verticalFov / 2)` in Perspective and the
ProjectionState visible height in Orthographic. Batches regenerate every frame.
This is view-derived truncation, not exact frustum clipping of the infinite Line;
OpenGL subsequently clips its finite representation to the viewing frustum.
Picking uses that same finite pair and clips projected segments before division.

#### Temporary application demo

GeometryDemoScene is a deterministic B4 composition in `src/app/demo`, with
3 Point3, 3 Segment3 and 2 Line3, in that insertion order. The adapters produce
3 point vertices, 6 segment vertices and 4 line vertices for a valid context.
It is not a Document, scene graph, CAD model or persistent data. No style
metadata or selection flags are stored in the demo or Presentation.

#### Picking implementation and limits

PickingContext holds camera/projection, dimensions, pointer coordinates,
clipping parameters and interaction tolerance. Its coordinates are Workspace-local
logical units: top-left origin, +X right, +Y down. WorkspaceViewport::pick takes
main-viewport logical coordinates and maps them to the effective clipped,
inward-rounded raster viewport using the actual framebuffer size. Nonuniform
HiDPI scaling uses the raster aspect for projection while distances stay logical.

PickingRay's `makePickingRay` reuses Geometry Ray3. Perspective rays originate
at camera position and point through the cursor; Orthographic directions are
parallel, with origins translated in the camera image plane. It uses camera
basis vectors and projection parameters; no Matrix4 inverse was introduced.
This helper is available independently: the current GeometryPicker hit algorithm
projects world geometry to screen, rather than intersecting those rays.

GeometryPicker provides `projectWorldToScreen` and `pickGeometry`. Point hits use
mouse-to-projected-point distance. Segment hits use distance to the projected,
frustum-clipped A/B segment. Line hits use its current finite visual pair.
Clipping precedes perspective division. PickHit stores a VisualEntityId and
screenDistance in logical pixels; it is a query result, not persistent state.

`defaultPickingTolerancePixels = 6.0` logical pixels, independent of zoom/DPI.
The three tolerance responsibilities stay separate:

- Math: numerical comparisons (default absolute/relative 1e-12).
- Geometry: geometric/model predicate tolerance (default 1e-9).
- Picking: interaction/perceptual tolerance (6 logical pixels).

Picking tolerance != geometric tolerance. Ranking uses smaller screen-space
distance, camera-space depth for numerical distance ties (within 1e-7 pixels),
then stable insertion order for numerically equal depths. There is no primitive
priority. Perspective segment depth uses reciprocal-depth interpolation.

Exact occlusion/depth-buffer-aware picking is not implemented. Depth tie-breaking
does not make this visibility-perfect: candidates are not checked against the
rendered depth buffer. Framebuffer/color-ID picking and glReadPixels picking
remain deferred. Empty/default Workspaces safely return no hit; invalid query
parameters fail explicitly rather than hiding numerical errors.

#### Hover and single selection

HoverState stores only an optional VisualEntityId. It is transient, recomputed
from the pointer and current view. No hit, outside Workspace, blocked UI/modal,
invalid focus/pointer or a degenerate viewport clears hover. MMB orbit/pan
suspends it; release or view changes recompute it from the current query.

SelectionState independently stores an optional VisualEntityId, initially none,
with zero-or-one semantics. An eligible leftPressed edge inside the effective
Workspace selects a hit or clears selection on empty space. Re-clicking the same
entity preserves it; clicking another replaces it. Outside/blocked/invalid input
and simultaneous MMB preserve selection. There is no toggle or additive modifier.

Selection survives cursor movement, hover changes, orbit, pan, zoom, projection
switch, resize and minimize. An offscreen selected entity remains selected even
when its highlight is not visible. Hover is transient; selection persists until
an eligible selection action changes it. They may reference different entities
simultaneously. Neither stores Geometry, PickHit or a pointer to a Geometry value.

Frame ordering is UI/input snapshot -> navigation/projection -> hover -> selection
-> render. Selection queries the existing picker at the press location using the
updated view; it does not require a cached hovered ID. The UI supplies mouse
position, leftPressed, MMB/Shift/wheel, focus, pointer validity, capture/blocking,
projection request and Workspace rectangle; picking and selection logic stay out
of Dear ImGui/ApplicationShell.

#### Visual state, batching and depth

VisualState and visualStateFor live in the Viewer. Resolution is exclusive:
`Selected > Hovered > Normal`; hover on the selected entity cannot replace its
selected appearance. HighlightColors.h centralizes the fixed B4 implementation
policy, not a style/material/theme framework:

| State/type | RGB |
| --- | --- |
| Normal Point | (1.0, 0.85, 0.2) |
| Normal Segment | (0.8, 0.8, 0.85) |
| Normal Line | (0.2, 0.75, 0.85) |
| Hovered, all three types | (0.65, 1.0, 1.0) |
| Selected, all three types | (1.0, 0.4, 0.05) |

VisualStateFilter partitions entities on the CPU. Each entity belongs to exactly
one Normal/Hovered/Selected batch. An absent adapter filter retains its original
all-entities contract. Workspace reuses three generic GPU buffers for successive
state batches; IDs never reach the GPU. No entity is duplicated into an overlay.

Actual draw order is grid -> normal Lines -> X/Y/Z axes -> normal Segments ->
normal Points -> hovered Lines/Segments/Points -> selected Lines/Segments/Points
-> UI. Normal drawing uses GL_LESS. Highlight drawing uses GL_LEQUAL so an
equal-depth fragment can replace the same geometry location by draw order;
nearer geometry still occludes farther highlighted geometry. There is no x-ray,
always-on-top, depth bias or depth clear between state batches. The local
Workspace pass guard restores its affected OpenGL state, including GL_DEPTH_FUNC,
viewport/scissor, depth/blend enables, masks, current program/VAO and clear state.
Without hover or selection, the B4.8 normal palette and geometry order remain.

#### B4 testing and D4 validation

Frozen validated snapshot from B4.12: **684/684 PASS, 0 FAIL**. Validation layers
include Presentation unit tests, Viewer adapter tests, real OpenGL renderer tests,
picking/ray tests, HoverState and Workspace hover tests, SelectionState and
Workspace selection tests, highlight batching, Workspace tests and the 14
end-to-end tests in `tests/viewer/test_geometry_viewer_integration.cpp`.

B4.10 covers Point/Segment/Line vertical slices and ID continuity; Geometry and
Presentation immutability (count, IDs, payload and order); screen-space tolerance;
finite Line draw/pick coherence; hover/selection independence and precedence;
exclusive batching; orbit/pan/zoom/projections; clipped viewport/HiDPI/resize and
minimize/restore; UI blocking; default/empty Presentation; and OpenGL state
restoration. Its scoped draw observer forwards real driver calls and checks
vertices/colors/depth state without pixel assertions. Runtime confirmed visible
geometry/highlights, navigation, projections, resize and UI/About; native X and
File -> Exit each returned 0. HiDPI was covered automatically, not repeated
manually during B4.10. Test counts are snapshots, not permanent totals.

| Accepted ADR | B4.11 audit result | Source/CMake/test evidence |
| --- | --- | --- |
| ADR-0017 | CONFORMANT | Geometry -> Math; Presentation -> Geometry; Viewer consumes Presentation; no reverse dependencies or visual state in Geometry |
| ADR-0018 | CONFORMANT | External VisualEntityId; separate optional-ID HoverState/SelectionState; zero-or-one selection; highlight resolved in Viewer; immutability tests |
| ADR-0019 | CONFORMANT | GeometryPicker projection/clipping and 6 logical-pixel tolerance; separate Geometry tolerance; no framebuffer/color-ID/depth read picking |
| ADR-0020 | CONFORMANT | Point/Segment/Line adapters and real draws; finite Line representation derived externally; Ray/Plane visualization absent |

ADR-0001 through ADR-0027 are **27/27 ACCEPTED**. ADR-0001–0024 remain unchanged.
Geometry/Presentation leakage searches found none; Rendering knows no visual
identity or interaction semantics, UI only supplies input, and the actual target
graph is acyclic. PROJECT_CHARTER remains consistent. Earlier ADR contexts and
B0–B3 validation tables are historical snapshots; D4 authorizes the explicitly
scoped B4 slice without rewriting their accepted decisions.

B4.12 baseline validation PASS: clean configure/Debug build, 684/684 tests,
0 failed and zero project, dependency or linker warnings. Runtime passed with
ordered shutdown: native X -> 0 and File -> Exit -> 0. Automated HiDPI passed;
manual B4.12 HiDPI was unavailable/not repeated. The MINOR documentary state
synchronization finding is CLOSED by B4.FREEZE.

#### Deferred after the frozen B4 slice

Ray3/Plane visualization; generic scene graph/ECS; multi-selection (including
Ctrl/Shift additive selection), selection box and lasso; framebuffer picking and exact occlusion-aware picking; persistent
visual identity, serialization and Document; Topology/BRep; Sketching, constraints
and dimensions; feature modeling, Extrude, Revolve, Boolean, history/regeneration
remain deferred. B4.FREEZE authorizes no new feature, increment, baseline
or Decision Gate.

### Longer-term Geometry direction (deferred)

Representação matemática de entidades geométricas.

Exemplos:

- Point
- Line
- Segment
- Plane
- Circle
- Arc
- Curve
- Surface

Evolução possível:

- Bezier;
- B-Spline;
- NURBS.

### core/topology

Representação das relações topológicas. D6 is FROZEN; B7 is IMPLEMENTED /
BASELINE CANDIDATE. Accepted ADR-0025–0027 are normative for the implemented
ownership, identity, orientation, Geometry associations and manifold invariants.

Exemplos:

- Vertex
- Edge
- Wire
- Face
- Shell
- Solid

Geometry e Topology deverão permanecer conceitos distintos.

The implemented dependency boundary is
`microsw_topology -> microsw_geometry -> microsw_math`, with no dependency on
Presentation, Viewer, Rendering, UI or Application. The B7 Solid owns
shared indexed Vertex/Edge/Wire/Face/Shell records and exactly one closed,
oriented 2-manifold Shell. Construction is append-only through validated APIs;
inspection is read-only. Edge geometry is derived as Segment3 from shared
Vertex positions. The deterministic cuboid contains 8 Vertices, 12 Edges,
6 Wires, 6 Faces and one Shell. ADR-0025–0027 are CONFORMANT.

### core/modeling

Operações que criam ou modificam modelos.

Exemplos futuros:

- primitives;
- extrusion;
- revolution;
- boolean operations;
- fillet;
- chamfer.

### core/document

Modelo do documento CAD.

Responsabilidades futuras:

- entities;
- features;
- parameters;
- dependencies;
- feature tree;
- rebuild.

### viewer (B2 foundation, B4 FROZEN)

Observa o mundo CAD e gere camera, viewport/navigation state, orbit, pan,
zoom e render aids. B4 acrescenta adapters, picking, hover, single-selection e highlight. Camera usa `microsw_math`; desenho usa Rendering.
Não possui geometria CAD, topology ou scene/domain ownership.

### rendering

Representação visual.

Actualmente fornece OpenGLContext, ShaderProgram, LineRenderer e PointRenderer. O Viewer
coordena viewport/scissor, camera e composição de grid/axes com essa infraestrutura.
Tessellation de entidades CAD e estilos visuais adicionais continuam planeados.

O renderer não deverá tornar-se proprietário do modelo CAD.

### interaction

Interacção entre utilizador e viewport.

Picking, hover e single-selection estão implementados no Viewer. Manipulators
continuam deferred. Input usa contratos em app, produzidos pela UI e consumidos
pelo Viewer; não existe um módulo interaction separado.

### ui

Interface gráfica da aplicação.

Não deverá conter algoritmos geométricos ou regras de modelação.

### persistence

Persistência de documentos.

Deverá depender de contratos do modelo e não da representação gráfica.

---

## 5. Regra de dependências

Orientação planeada:

```text
app
 ↓
ui / interaction / persistence
 ↓
document / modeling
 ↓
topology
 ↓
geometry
 ↓
math
```

Dependências inversas deverão ser evitadas.

Em particular:

math       X→ UI
geometry   X→ UI
topology   X→ renderer
modeling   X→ renderer
document   X→ concrete UI

---

## 6. Fronteiras substituíveis

São candidatos naturais a interfaces arquitecturais:

- renderer;
- tessellator;
- boolean engine;
- geometry kernel;
- constraint solver;
- persistence serializer;
- selection acceleration.

Interfaces apenas deverão ser introduzidas quando a fronteira for necessária.

Não criar interfaces automaticamente para todas as classes.

---

## 7. Representação CAD vs representação gráfica

Esta separação é obrigatória.

> CAD representation is authoritative. Render representation is derived.

Uma entidade CAD não é um mesh gráfico. O pipeline planeado é:

```text
CAD Representation
        |
        v
   Tessellation
        |
        v
   Render Model
        |
        v
      OpenGL
```

Alterações no renderer não deverão obrigar a modificar a representação CAD.
Tessellation e as representações CAD continuam por implementar.

Em B2, grid/axes podem gerar render primitives directamente: são viewer aids,
não representação CAD. Esta excepção conceptual não permite tratar futuras
entidades CAD como meshes nem contornar o pipeline de tessellation do domínio.

---

## 8. Estrutura planeada

micro-solidworks/
├── CMakeLists.txt
├── README.md
├── docs/
├── src/
│   ├── app/
│   ├── core/
│   │   ├── math/
│   │   ├── geometry/
│   │   ├── topology/
│   │   ├── modeling/
│   │   └── document/
│   ├── rendering/
│   ├── interaction/
│   ├── ui/
│   └── persistence/
├── tests/
└── examples/

Esta estrutura poderá evoluir. Não deverá ser expandida preventivamente sem necessidade.
