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
secção ainda não existem em B0.

### Implemented B1 Math Foundation (B1.1–B1.8)

B1 is IN PROGRESS; B0 remains the frozen stable baseline. The implemented
Math module lives in `src/core/math`, under namespace `microsw::math`.
Its project-owned CMake target has only this dependency:

```text
microsw_math -> C++ standard library only
```

It is independent of logging, windowing, rendering and UI, including GLFW,
GLAD, OpenGL, Dear ImGui and spdlog. Math is tested through the existing
`micro_solidworks_tests` target; the B0 application shell remains unchanged.

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

Not implemented or authorized by this increment:

- `Point2`, `Point3`, `Direction3`, `Vector4`;
- generic `Vector<T,N>` and `Matrix<T,R,C>`;
- `Quaternion` and arbitrary-axis rotation;
- matrix inverse and Matrix4 determinant;
- perspective and orthographic projection;
- global geometric modelling tolerance;
- a `Transform` class, `Ray`, `Plane` and `BoundingBox`.

Deferred does not mean rejected forever. Each capability requires a future
explicitly authorized scope. B1.9 consolidates documentation; B1.10 is the
next pending quality gate and requires separate authorization. B1 is not frozen.

---

## 3. Implementação actual em B0

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

## 4. Módulos planeados

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

### rendering

Representação visual.

Responsabilidades futuras:

- render pipeline;
- camera;
- viewport;
- tessellation boundary;
- visual styles;
- grid;
- axes.

O renderer não deverá tornar-se proprietário do modelo CAD.

### interaction

Interacção entre utilizador e viewport.

Responsabilidades futuras:

- picking;
- selection;
- hover;
- manipulators;
- orbit;
- pan;
- zoom.

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
Tessellation e as representações CAD ainda não estão implementadas em B0.

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
