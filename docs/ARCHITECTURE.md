# Micro SolidWorks — Architecture

## 1. Objectivo

Definir as fronteiras arquitecturais iniciais do Micro SolidWorks sem antecipar complexidade desnecessária.

A arquitectura deverá permitir evolução incremental e substituição controlada de implementações.

---

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
secção (Document, Modeling, Topology e Geometry) continuam por implementar.
As secções seguintes distinguem FROZEN FOUNDATIONS, CURRENT B2 IMPLEMENTATION
e PLANNED / DEFERRED.

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
type. `Point3` remains deferred; callers must preserve the explicit
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
in B2's Viewer through `ViewProjection`; the remaining Math concepts above
remain deferred.

### CURRENT B2 IMPLEMENTATION — Viewer (D2 FROZEN)

B2 is IN PROGRESS, not a frozen stable baseline. B2.1–B2.11 are COMPLETE;
B2.12 — Documentation & D2 Validation is CURRENT. B2.13 and B2.FREEZE remain
PENDING and require explicit authorization.

The Viewer observes the CAD world; it does not own CAD representation.
Current composition in `src/app/main.cpp`:

```text
Application
    +-- ApplicationWindow / OpenGLContext / ImGuiLayer
    +-- ApplicationShell
    |     +-- WorkspaceLayout / WorkspaceInput / ProjectionMode
    +-- WorkspaceViewport
          +-- OrbitCamera / OrbitNavigation / PanZoomNavigation
          +-- ProjectionState
          +-- ReferenceGrid / ReferenceAxes (aid generation at construction)
          +-- ShaderProgram / LineRenderer --> OpenGL / GLAD
```

#### Actual CMake boundaries

The table records direct `target_link_libraries` relationships in
`CMakeLists.txt`, not an invented idealized graph. Standard C++ dependencies
are implicit.

| Target | Responsibility | PUBLIC links | PRIVATE links |
| --- | --- | --- | --- |
| `microsw_math` | Internal B1 mathematics | None | None |
| `microsw_logging` | Project-owned `Logger` | None | `spdlog::spdlog` |
| `microsw_windowing` | `ApplicationWindow`, GLFW/window/context lifetime | None | `glfw`, `microsw_logging` |
| `microsw_rendering` | `OpenGLContext`, `ShaderProgram`, `LineRenderer` | `microsw_math` | `glad_gl_core_33`, `microsw_logging`, `microsw_windowing` |
| `microsw_viewer` | Camera, navigation, projection, aids and Workspace pass | `microsw_math` | `microsw_rendering`, `glad_gl_core_33` |
| `microsw_imgui_backend` | Dear ImGui and GLFW/OpenGL3 backends | None | `glfw` |
| `microsw_ui` | `ImGuiLayer` and `ApplicationShell` | None | `glfw`, `microsw_imgui_backend`, `microsw_logging`, `microsw_rendering`, `microsw_windowing` |
| `micro_solidworks` | Application composition | None | `microsw_viewer`, `microsw_logging`, `microsw_rendering`, `microsw_ui`, `microsw_windowing` |
| `micro_solidworks_tests` | GoogleTest/CTest, including real-context tests | None | `GTest::gtest_main`, `microsw_logging`, `microsw_math`, `microsw_viewer`, `microsw_rendering`, `microsw_windowing`, `glad_gl_core_33`, `glfw` |

`glad_gl_core_33` is the generated OpenGL 3.3 Core loader. OpenGL is supplied
by the system driver. Rendering's Windowing dependency supports context
bootstrap; the Viewer uses GLAD privately to coordinate its render pass.
Rendering does not depend on Viewer and has no camera, navigation, grid, axis
or CAD semantics. Math remains independent of every graphical layer.

The shared headers under `src/app` are application contracts, not another
library target. UI does not link to Viewer; no ImGui types cross that boundary.

#### UI and Workspace contracts

`ApplicationShell` owns menus, the Model panel, status bar, About modal and
logical Workspace layout/input snapshot. It does not own a camera, projection
state, render primitives or CAD model.

- `WorkspaceLayout`: logical display coordinates, top-left origin relative
  to the main application viewport, plus logical display dimensions.
- `WorkspaceInput`: per-frame pointer/button/modifier/wheel snapshot with
  focus, pointer-validity, Workspace-hover and UI-blocking decisions.
  Its optional `projectionRequest` is a one-frame command.
- `ProjectionMode`: shared project-owned enum in `src/app/ProjectionMode.h`,
  with `Perspective` and `Orthographic`. This location permits UI indication
  and requests without a UI-to-Viewer class dependency.

The menu is **View → Projection → Perspective / Orthographic**. Main passes
`workspace.projectionMode()` into `shell.draw(...)` for checked indication;
UI emits a request, consumed by `WorkspaceViewport::updateNavigation`.
The only source of truth is WorkspaceViewport's `ProjectionState`, not a
UI-owned mirror. Explicit projection requests apply independently of blocked
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
and four LineRenderer batches. ReferenceGrid/ReferenceAxes generate CPU aid
vertices during construction; the GPU batches persist. There is no CAD model,
geometry ownership, selection, picking or scene graph.

Main clears the frame, composes Dear ImGui's UI, updates navigation, renders
the Workspace, renders the ImGui chrome over it, then swaps buffers.
Dear ImGui reserves a Workspace with no background covering the 3D pass.
`framebufferRect()` clips logical bounds to the display, scales to framebuffer
pixels (including HiDPI and independent horizontal/vertical scale), rounds
inward and converts top-left to OpenGL bottom-left coordinates. The result
drives `glViewport`/`glScissor`; invalid, empty or offscreen rectangles are
no-ops. Resize recomputes aspect/projection, not camera pose or aid geometry.
Offscreen framebuffer / texture-backed viewport: NOT IMPLEMENTED / DEFERRED.

The pass clears only Workspace color/depth, enables scissor and depth testing
with GL_LESS, enables color/depth writes and disables blending. Its local RAII
guard restores viewport, scissor box, depth function, program, VAO,
scissor/depth/blend enables, write masks and clear values, including exception
paths. This is not a global state manager or render graph.

OpenGLContext loads/validates OpenGL through ApplicationWindow and supplies
whole-frame background clearing. GPU objects require a current compatible
context and loaded GLAD throughout their lifetime. Main destroys Workspace
GPU resources before ImGui and before the GLFW window/context.

#### ShaderProgram and LineRenderer

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

#### Testing and D2 validation

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
- CAD/domain: geometry, topology, scene graph, CAD tessellation, selection,
  picking, ray casting, entity hover/highlighting. Workspace hover used for
  input gating is not CAD hover.
- Math: production Vector4, generic Vector/Matrix, Matrix4 determinant,
  matrix inverse, Quaternion, arbitrary-axis rotation, global geometric
  modelling tolerance, Point/Direction types, Ray, Plane and BoundingBox.
  Local homogeneous helpers in tests do not add a production Vector4.

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

Representação das relações topológicas.

Exemplos:

- Vertex
- Edge
- Wire
- Face
- Shell
- Solid

Geometry e Topology deverão permanecer conceitos distintos.

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

### viewer (CURRENT B2)

Observa o mundo CAD e gere camera, viewport/navigation state, orbit, pan,
zoom e render aids. Camera usa `microsw_math`; desenho usa Rendering.
Não possui geometria CAD, topology ou scene/domain ownership.

### rendering

Representação visual.

Actualmente fornece OpenGLContext, ShaderProgram e LineRenderer. O Viewer
coordena viewport/scissor, camera e composição de grid/axes com essa infraestrutura.
Tessellation de entidades CAD e estilos visuais adicionais continuam planeados.

O renderer não deverá tornar-se proprietário do modelo CAD.

### interaction

Interacção entre utilizador e viewport.

Responsabilidades futuras:

- picking;
- selection;
- hover;
- manipulators;

O encaminhamento actual de orbit/pan/zoom usa contratos em app, produzidos
pela UI e consumidos pelo Viewer; não existe um módulo interaction separado.

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
