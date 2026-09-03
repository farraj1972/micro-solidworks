# Micro SolidWorks — Roadmap

## Development Rule

Cada baseline é dividida em increments pequenos.

Cada increment deverá:

1. possuir scope explícito;
2. preservar funcionalidades existentes;
3. compilar;
4. executar os testes aplicáveis;
5. manter o executável funcional;
6. evitar alterações fora do scope.

---

# B0 — Foundation

Status: FROZEN

Objectivo:

Estabelecer um projecto C++ compilável, testável e executável.

Estado dos increments:

| Increment | Estado |
| --- | --- |
| B0.1 — Repository & Build Foundation | COMPLETE |
| B0.2 — Test Infrastructure | COMPLETE |
| B0.3 — Logging | COMPLETE |
| B0.4 — GLFW Application Window | COMPLETE |
| B0.5 — OpenGL Bootstrap | COMPLETE |
| B0.6 — Dear ImGui Integration | COMPLETE |
| B0.7 — Application Shell | COMPLETE |
| B0.8 — D0 Documentation & ADRs | COMPLETE |
| B0.9 — Baseline Validation | COMPLETE |
| B0.FREEZE | FROZEN |

Resultados:

- C++20;
- CMake;
- estrutura inicial;
- application bootstrap;
- window;
- logging;
- test framework;
- documentação arquitectural;
- build reproduzível.

Exit criteria:

cmake configure → PASS  
cmake build → PASS  
ctest → PASS  
application launch → PASS

---

# Decision Gate D1 — Mathematical Conventions

Status: FROZEN

The scalar, tolerance, coordinate, unit, angle, vector, matrix and transform
conventions for B1 are recorded in ADR-0007 through ADR-0009.

---

# B1 — Mathematical Foundation

Status: FROZEN

Objectivo:

Construir os fundamentos matemáticos necessários ao motor geométrico.

Capacidades implementadas até B1.8:

- Scalar e numerical comparison tolerance;
- Vector2;
- Vector3;
- Matrix3 e Matrix4;
- affine Transformation Operations;
- testes de integração matemática.

Vector4, Ray, Plane, BoundingBox e as restantes capacidades listadas em
"Deferred after B1" de ARCHITECTURE.md não estão implementadas nem fazem
parte dos increments restantes de B1.

Todos os componentes matemáticos deverão possuir testes.

Estado dos increments:

| Increment | Estado |
| --- | --- |
| B1.1 — Numeric Utilities & Tolerance | COMPLETE |
| B1.2 — Vector2 | COMPLETE |
| B1.3 — Vector3 | COMPLETE |
| B1.4 — Matrix Foundation | COMPLETE |
| B1.5 — Matrix3 | COMPLETE |
| B1.6 — Matrix4 | COMPLETE |
| B1.7 — Transformation Operations | COMPLETE |
| B1.8 — Mathematical Integration Tests | COMPLETE |
| B1.9 — Documentation & D1 ADR Validation | COMPLETE |
| B1.10 — Baseline Validation | COMPLETE |
| B1.FREEZE | FROZEN |

B1 está FROZEN. Tag: `b1-mathematical-foundation`.
B2 permanece NOT STARTED; implementação requer autorização explícita por increment.

---

# Decision Gate D2 — 3D Viewer Conventions

Status: FROZEN

As decisões aprovadas de camera/navigation, view/projection e rendering
estão materializadas em ADR-0010, ADR-0011 e ADR-0012.
B2 está autorizado para planeamento, não para implementação automática.

---

# B2 — 3D Viewer

Status: NOT STARTED

Objectivo:

Criar um viewport tridimensional navegável.

Estado dos increments planeados:

| Increment | Estado |
| --- | --- |
| B2.1 — Camera Mathematical Foundation | PENDING |
| B2.2 — View & Projection Matrices | PENDING |
| B2.3 — Shader Infrastructure | PENDING |
| B2.4 — GPU Buffer / Line Rendering | PENDING |
| B2.5 — Workspace 3D Viewport | PENDING |
| B2.6 — Reference Axes | PENDING |
| B2.7 — XY Grid | PENDING |
| B2.8 — Orbit Navigation | PENDING |
| B2.9 — Pan & Zoom | PENDING |
| B2.10 — Perspective / Orthographic Modes | PENDING |
| B2.11 — Viewer Integration Tests & Runtime Validation | PENDING |
| B2.12 — Documentation & D2 Validation | PENDING |
| B2.13 — Baseline Validation | PENDING |
| B2.FREEZE | PENDING |

B2.1 requer autorização explícita. D2 não implementa camera, viewer ou render aids.

Resultado:

viewport 3D interactivo.

---

# B3 — Geometric Primitives

Objectivo:

Introduzir as primeiras entidades geométricas.

Possíveis capacidades:

- Point;
- Line;
- Segment;
- Plane;
- Circle;
- basic intersections;
- tolerance handling.

---

# B4 — Scene & Object Model

Objectivo:

Permitir representar múltiplos objectos numa cena/documento inicial.

Capacidades:

- object identity;
- transforms;
- scene registration;
- visibility;
- basic object lifecycle.

---

# B5 — Selection & Picking

Objectivo:

Permitir identificar entidades através do viewport.

Capacidades:

- screen ray;
- ray/object intersection;
- hover;
- click selection;
- selection state;
- visual highlighting.

---

# B6 — Transformations

Capacidades:

- translate;
- rotate;
- scale quando aplicável;
- local/world coordinates;
- transform composition.

---

# B7 — Topological Model

Objectivo:

Introduzir representação topológica independente da geometria.

Capacidades:

Vertex  
Edge  
Wire  
Face  
Shell  
Solid

Esta baseline estabelece a fundação para modelação sólida.

---

# B8 — Sketcher

Objectivo:

Criar geometria bidimensional sobre planos de referência.

Capacidades previstas:

- sketch plane;
- line;
- rectangle;
- circle;
- arc;
- selection;
- editing;
- dimensions iniciais.

---

# B9 — Constraint System

Objectivo:

Introduzir relações paramétricas.

Constraints possíveis:

- coincident;
- horizontal;
- vertical;
- parallel;
- perpendicular;
- distance;
- radius;
- angle.

Inicialmente poderá ser utilizado um solver educacional simples.

---

# B10 — Extrusion

Objectivo:

Gerar sólidos a partir de profiles fechados.

Workflow:

Sketch
 ↓
Profile
 ↓
Extrude
 ↓
Solid

---

# B11 — Boolean Operations

Capacidades:

- union;
- difference;
- intersection.

Prioridade inicial:

correctness e compreensão, não robustez industrial.

---

# B12 — Feature Tree

Objectivo:

Representar a construção histórica do modelo.

Exemplo:

Part
├── Origin
├── Sketch001
├── Extrude001
├── Sketch002
└── Cut001

---

# B13 — Parametric Rebuild

Objectivo:

Permitir alterar parâmetros anteriores e reconstruir features dependentes.

Conceitos:

- dependency graph;
- dirty state;
- ordered rebuild;
- error propagation;
- feature validity.

Esta é uma das baselines fundamentais para transformar o modelador 3D num pequeno CAD paramétrico.

---

# B14 — Persistence

Objectivo:

Guardar e recuperar documentos.

Capacidades:

- New;
- Save;
- Save As;
- Open;
- document version;
- serialization;
- validation.

---

# MVP FREEZE

O MVP deverá permitir:

New Document
→ Sketch
→ Geometry
→ Constraints
→ Extrude
→ Select
→ Second Sketch
→ Extrude/Cut
→ Edit Parameter
→ Rebuild
→ Save
→ Open

O repositório será marcado com uma baseline/tag MVP estável.

---

# Post-MVP

Possíveis linhas de evolução:

B15 — Revolve  
B16 — Fillet / Chamfer  
B17 — Advanced Sketch Constraints  
B18 — Improved Boolean Engine  
B19 — Curves / Bezier  
B20 — B-Splines / NURBS  
B21 — Advanced Surfaces  
B22 — STEP / IGES  
B23 — Assemblies  
B24 — Measurements  
B25 — Technical Drawing  
B26 — Performance / Spatial Acceleration  
B27 — Alternative Geometry Kernel

A sequência pós-MVP não está congelada e deverá ser decidida com base na evolução do projecto.
