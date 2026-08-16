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

### Planned B1 Math Foundation

Decision Gate D1 freezes the following conventions for the future Math layer:

```text
Scalar:             double
Coordinate system:  right-handed
World orientation:  XY horizontal/base plane; Z vertical
Angles:             radians internally
Vectors:            column-vector convention
Transforms:         v' = M * v
```

The Math core will remain unit-agnostic, while the initial CAD/document unit
convention is millimetres. Rendering must adapt to these CAD-domain
mathematical conventions rather than shaping the Math core around OpenGL.

Math implementation has not started. These statements describe the approved
B1 conventions, not types or APIs that currently exist.

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

Exemplos futuros:

- Vector2
- Vector3
- Vector4
- Matrix3
- Matrix4
- Quaternion
- Transform
- Ray
- Plane
- BoundingBox

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
