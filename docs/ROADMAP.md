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

Vector4, Ray, Plane, BoundingBox e os restantes conceitos Math listados em
"Deferred after B1" de ARCHITECTURE.md continuam por implementar.
View/projection math foi materializado no Viewer em B2, sem alterar B1.

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
B2 está FROZEN; cada novo increment requer autorização explícita.

---

# Decision Gate D2 — 3D Viewer Conventions

Status: FROZEN

As decisões aprovadas de camera/navigation, view/projection e rendering
estão materializadas em ADR-0010, ADR-0011 e ADR-0012.
D2 não autoriza implementação automática: cada increment B2 requer autorização.

---

# B2 — 3D Viewer

Status: FROZEN

Objectivo:

Criar um viewport tridimensional navegável.

Estado dos increments:

| Increment | Estado |
| --- | --- |
| B2.1 — Camera Mathematical Foundation | COMPLETE |
| B2.2 — View & Projection Matrices | COMPLETE |
| B2.3 — Shader Infrastructure | COMPLETE |
| B2.4 — GPU Buffer / Line Rendering | COMPLETE |
| B2.5 — Workspace 3D Viewport | COMPLETE |
| B2.6 — Reference Axes | COMPLETE |
| B2.7 — XY Grid | COMPLETE |
| B2.8 — Orbit Navigation | COMPLETE |
| B2.9 — Pan & Zoom | COMPLETE |
| B2.10 — Perspective / Orthographic Modes | COMPLETE |
| B2.11 — Viewer Integration Tests & Runtime Validation | COMPLETE |
| B2.12 — Documentation & D2 Validation | COMPLETE |
| B2.13 — Baseline Validation | COMPLETE |
| B2.FREEZE | FROZEN |

B2.1–B2.13 estão completos. B2.13 validou 274/274
testes e os cenários manuais de runtime, sem findings.
B2 está FROZEN. Tag: `b2-3d-viewer`.
Nenhum increment, Decision Gate, baseline ou freeze posterior está autorizado
automaticamente; cada um requer autorização explícita.

Resultado implementado até B2.11:

Workspace 3D com Perspective/Orthographic, grid XY finito, eixos RGB e
orbit/pan/zoom. Sem representação CAD, picking ou selection.

---

# Decision Gate D3 — Geometric Foundation

Status: FROZEN

ADR-0013–0016 estão ACCEPTED: semântica Point/Vector, representação das
primitivas, geometric/modeling tolerance e degenerescência, e fronteiras
Geometry/Topology/CAD. D3 é documental: não implementa tipos nem targets.
D0/D1/D2 e as baselines B0/B1/B2 permanecem FROZEN.

---

# B3 — Geometric Primitives

Status: NOT STARTED

Objectivo:

Introduzir valores geométricos concretos sobre Math, conforme D3, sem
identidade topológica, entidades CAD, dependências gráficas ou kernel externo.

Estado dos increments:

| Increment | Estado |
| --- | --- |
| B3.1 — Point2 / Point3 | PENDING |
| B3.2 — Segment2 / Segment3 | PENDING |
| B3.3 — Line2 / Line3 | PENDING |
| B3.4 — Ray2 / Ray3 | PENDING |
| B3.5 — Plane | PENDING |
| B3.6 — Primitive Queries | PENDING |
| B3.7 — Distance & Projection Operations | PENDING |
| B3.8 — Geometric Integration Tests | PENDING |
| B3.9 — Documentation & D3 Validation | PENDING |
| B3.10 — Baseline Validation | PENDING |
| B3.FREEZE | PENDING |

Direcção planeada: representação, invariants, basic queries, distance/projection
e integração. Length, squaredLength, direction, midpoint, contains, closest point,
distance, projection, isDegenerate e queries básicas de parallel/perpendicular
são possibilidades; as APIs exactas dependem do incremento autorizado.
Templates genéricos e framework geral de intersections ficam deferred.
Uma intersection pontual futura exige um incremento explicitamente justificado.

B3.1 — Point2 / Point3 só pode começar após autorização explícita.
Nenhum incremento está CURRENT; D3.FREEZE não inicia B3.

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
