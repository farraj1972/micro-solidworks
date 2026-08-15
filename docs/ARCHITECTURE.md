# Micro SolidWorks — Initial Architecture

## 1. Objectivo

Definir as fronteiras arquitecturais iniciais do Micro SolidWorks sem antecipar complexidade desnecessária.

A arquitectura deverá permitir evolução incremental e substituição controlada de implementações.

---

## 2. Arquitectura conceptual

Application
    |
    +-- UI
    |
    +-- Interaction
    |
    +-- Document
            |
            +-- Modeling
                    |
                    +-- Topology
                    |
                    +-- Geometry
                            |
                            +-- Math

Rendering consome representações apropriadas do modelo, mas não define o modelo CAD.

Persistence serializa o Document através de uma fronteira própria.

---

## 3. Módulos iniciais

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

Responsabilidades:

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

## 4. Regra de dependências

Orientação desejada:

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

Dependências inversas deverão ser evitadas.

Em particular:

math       X→ UI
geometry   X→ UI
topology   X→ renderer
modeling   X→ renderer
document   X→ concrete UI

---

## 5. Fronteiras substituíveis

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

## 6. Representação CAD vs representação gráfica

Esta separação é obrigatória.

Um Solid não é um mesh gráfico.

Conceptualmente:

CAD Solid
   |
Tessellator
   |
Render Mesh
   |
Renderer
   |
GPU

Alterações no renderer não deverão obrigar a modificar a representação fundamental do Solid.

---

## 7. Estrutura inicial

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