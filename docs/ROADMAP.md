# Micro SolidWorks — Roadmap

## Canonical MVP Roadmap

Este é o roadmap funcional oficial até MVP. A numeração não muda com a
organização dos increments ou das baselines técnicas históricas.

| Fase canónica | Scope |
| --- | --- |
| B0 | Foundation |
| B1 | Mathematical Foundation |
| B2 | 3D Viewer |
| B3 | Geometric Primitives |
| B4 | Scene & Object Model |
| B5 | Selection & Picking |
| B6 | Transformations |
| B7 | Topological Model |
| B8 | Sketcher |
| B9 | Constraint System |
| B10 | Extrusion |
| B11 | Boolean Operations |
| B12 | Feature Tree |
| B13 | Parametric Rebuild |
| B14 | Persistence |
| MVP FREEZE | Apenas quando o workflow MVP abaixo estiver funcional |

## Vertical-Slice Development Strategy

**Vertical Slice First**.

```text
Feature over performance.
Working vertical slice over speculative completeness.
Architecture boundaries over premature abstraction.
Canonical roadmap phase != necessarily one historical implementation baseline.
```

Privilegiar increments com comportamento funcional observável; não completar
antecipadamente subsistemas que ainda não bloqueiam o próximo slice. Preservar
ADRs e fronteiras arquitecturais, aceitar distribuição de implementação por
baselines diferente do plano original e registar explicitamente gaps/debt.
Fechar gaps quando se tornarem pré-requisitos reais.

Uma capability pode ser materializada antecipadamente num incremento
explicitamente autorizado se respeitar os ADRs, não inverter dependências,
não impedir evolução futura e ficar claramente mapeada. Exemplo realizado:
a funcionalidade canónica B5 foi implementada na baseline técnica B4.

B0–B4 técnicas permanecem FROZEN. Um gap canónico não reabre automaticamente
uma baseline congelada: a capability será implementada num novo incremento ou
baseline autorizado, preservando compatibilidade. Esta estratégia não substitui
os gates de autorização nem permite trabalho especulativo.

## Implementation Mapping

| Fase canónica / scope | Realização funcional | Implementação técnica / gaps |
| --- | --- | --- |
| B0 Foundation | SATISFIED | COMPLETE / FROZEN; nenhum gap material |
| B1 Mathematical Foundation | SATISFIED | COMPLETE / FROZEN; nenhum gap material para o roadmap actual |
| B2 3D Viewer | SATISFIED | COMPLETE / FROZEN |
| B3 Geometric Primitives: point, line, segment, plane, circle, basic intersections | PARTIALLY SATISFIED | B3 técnica FROZEN: Point2/3, Segment2/3, Line2/3, Ray2/3, Plane, queries, distance/projection e robustness; Circle/basic intersections são DEFERRED GAP — not currently blocking B6 |
| B4 Scene & Object Model: identity, transforms, visibility, object lifecycle | PARTIALLY SATISFIED | B4 técnica FROZEN: Presentation identity, GeometryPresentation, visual ownership boundary, visualização Point/Segment/Line e o slice de picking/hover/single-selection/highlight |
| B5 Selection & Picking: screen ray, intersections/picking, hover, click selection, selection state, highlighting | REALIZED / SATISFIED BY B4 | FUNCTIONALLY REALIZED BY FROZEN TECHNICAL B4; No duplicate B5 implementation is required |
| B6 Transformations: translate, rotate, scale, local/world coordinates | NEXT ACTIVE FUNCTIONAL AREA / NOT STARTED | Não implementada como transformação de entidades; D5 — Transformation Semantics está FROZEN |

A realização de B5 usa PickingRay e picking geométrico screen-space existentes;
não implica um motor geral de intersecções nem picking com oclusão perfeita.
As operações matemáticas B1 e a navegação B2 não realizam transforms de entidades B6.

Detalhe do B4 canónico:

| Capability | Mapping |
| --- | --- |
| identity | PARTIALLY REALIZED: identidade visual local, não identidade CAD persistente/global |
| transforms | NOT YET REALIZED |
| visibility | NOT YET REALIZED as explicit entity state |
| object lifecycle | NOT YET REALIZED beyond construction/lookup |

Logo, B4 técnico FROZEN não significa B4 canónico completamente realizado.
Não existe freeze técnico B5 separado implícito neste mapping.

## Canonical Roadmap Gaps

Registo documental simples de Canonical Gaps; não cria tracker ou framework.

| ID | Gap | Blocking now | Expected closure |
| --- | --- | --- | --- |
| GAP-GEO-001 | Circle primitive missing from canonical B3 scope | NO | Before/during B8, antes do slice Sketcher que necessite circles/arcs |
| GAP-GEO-002 | Basic intersections missing from canonical B3 scope | NO | Before first B7/B8 capability that requires them |
| GAP-SCENE-001 | OPEN — Entity transforms missing from canonical B4 scope | YES for next vertical slice | B6 after D5 |
| GAP-SCENE-002 | Explicit visibility/lifecycle object semantics incomplete | NO | When required by Document/Topology/application lifecycle |

Circle e basic intersections são gaps deferred, não bloqueiam actualmente B6
e não serão implementados neste alignment. Cada fecho exige scope autorizado.

## Next Functional Slice — B6, after D5 freeze

```text
Select entity
→ edit transform
→ entity moves/rotates/scales
→ rendering updates
→ picking follows transformed entity
→ hover/selection/highlight remain coherent
```

Este é o primeiro objectivo decidido para Point3, Segment3 e Line3, antes de
qualquer Topology. D5 define: translation, rotation,
positive scale, local/world transform, render/pick coherence,
selection/highlight coherence e minimal transform editing UI.

Não exigir ainda gizmo, hierarchy, parent-child transforms, undo/redo,
persistence, CAD Document ou Topology. Estas semânticas estão FROZEN nos ADRs
ADR-0021–0024, todos ACCEPTED.

**D5 — Transformation Semantics: FROZEN.** ADR-0021–0024 estão ACCEPTED.
B6 permanece NOT STARTED e não pode começar sem autorização explícita.

## Historical Technical Baselines and Canonical Phase Details

As secções B0–B4 seguintes registam as baselines técnicas efectivamente
congeladas, com nomes/commits/tags históricos preservados. B5–B14 conservam a
sequência funcional canónica; a matriz acima determina o estado de realização.

---

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

Status: FROZEN

Objectivo:

Introduzir valores geométricos concretos sobre Math, conforme D3, sem
identidade topológica, entidades CAD, dependências gráficas ou kernel externo.

Estado dos increments:

| Increment | Estado |
| --- | --- |
| B3.1 — Point2 / Point3 | COMPLETE |
| B3.2 — Segment2 / Segment3 | COMPLETE |
| B3.3 — Line2 / Line3 | COMPLETE |
| B3.4 — Ray2 / Ray3 | COMPLETE |
| B3.5 — Plane | COMPLETE |
| B3.6 — Primitive Queries | COMPLETE |
| B3.7 — Distance & Projection Operations | COMPLETE |
| B3.8 — Geometric Integration Tests | COMPLETE |
| B3.9 — Documentation & D3 Validation | COMPLETE |
| B3.10A — Projection Reconstruction Robustness Fix | COMPLETE |
| B3.10 — Baseline Validation | COMPLETE |
| B3.FREEZE | FROZEN |

Implementado em B3.1–B3.8: Point2/3, Segment2/3, Line2/3, Ray2/3 e Plane;
representações canónicas e invariants, queries de pertença e relações entre
primitivas do mesmo tipo, métricas/projecções entre primitiva e Point.
B3.10A corrigiu a robustez da reconstrução de closest-point identificada na
primeira validação B3.10. A repetição de B3.10 validou 549/549 testes, 0 falhas,
runtime smoke com fecho de código 0 e nenhum finding. B3 está FROZEN.

Intersections, primitive equivalence, relações generalizadas entre tipos,
métricas entre primitivas e Topology/CAD continuam não implementados.
Geometry rendering integration foi acrescentada externamente em B4.1–B4.10. Nenhum trabalho futuro é autorizado por este estado.

---

# Decision Gate D4 — Geometry Visualization & Selection

Status: FROZEN

ADR-0017–0020 estão ACCEPTED. D4 congela a boundary de apresentação derivada,
identidade visual externa à Geometry, hover distinto de single-selection,
picking geométrico com tolerance de interacção screen-space e o scope visual
obrigatório Point3/Segment3/Line3. Geometry permanece model-only.

---

# Technical B4 — Geometry Visualization & Selection

Status: FROZEN

Objectivo:

Provar o vertical slice Geometry → Presentation → Viewer rendering → Picking
→ Hover → Selection → Highlight, sem alterar a baseline Geometry congelada.

Estado dos increments:

| Increment | Estado |
| --- | --- |
| B4.1 — Geometry Presentation Model | COMPLETE |
| B4.2 — Point Rendering | COMPLETE |
| B4.3 — Segment Rendering | COMPLETE |
| B4.4 — Line Visualization & View Clipping | COMPLETE |
| B4.5 — Geometry Demo Scene | COMPLETE |
| B4.6 — Picking Foundation | COMPLETE |
| B4.7 — Hover State | COMPLETE |
| B4.8 — Single Selection | COMPLETE |
| B4.9 — Selection Highlighting | COMPLETE |
| B4.10 — Viewer / Geometry Integration Tests | COMPLETE |
| B4.11 — Documentation & D4 Validation | COMPLETE |
| B4.12 — Baseline Validation | COMPLETE |
| B4.FREEZE | FROZEN |

B4.1–B4.12 estão COMPLETE; B4.FREEZE e B4 estão FROZEN. B4 é a baseline
estável mais recente, com apresentação/visualização Point3/Segment3/Line3,
picking geométrico, hover, single-selection e highlight. B4.12 passou clean
configure/Debug build e 684/684 testes, 0 falhas e zero warnings de projecto,
dependências ou linker. Runtime PASS: X nativo -> 0 e File -> Exit -> 0.
HiDPI automatizado PASS; validação manual B4.12 indisponível/não repetida.
O finding MINOR de estado documental está CLOSED por B4.FREEZE.

Próxima área funcional canónica: B6, após D5 — Transformation Semantics FROZEN.
B6 permanece NOT STARTED.
Alterações ao comportamento B4 congelado exigem autorização explícita.
Ray3/Plane visualization, framebuffer e exact occlusion-aware picking,
multi-selection/selection box/lasso, scene graph/ECS, identidade persistente,
serialization/Document, Topology/BRep, Sketching/constraints/dimensions e
feature modeling (Extrude/Revolve/Boolean/history/regeneration) continuam deferred.
As secções B5+ abaixo mantêm a sequência canónica, não autorizam execução;
B5 já está satisfeito pela baseline técnica B4 descrita acima.

---

# B5 — Selection & Picking

Estado canónico: REALIZED / SATISFIED BY B4.
No duplicate B5 implementation is required. A descrição abaixo é o scope
funcional satisfeito, não um plano de implementação duplicada.

Objectivo:

Permitir identificar entidades através do viewport.

Capacidades:

- screen ray;
- intersections/picking (realizado por picking geométrico screen-space);
- hover;
- click selection;
- selection state;
- visual highlighting.

---

# B6 — Transformations

Estado: NEXT ACTIVE FUNCTIONAL AREA / NOT STARTED; D5 — Transformation Semantics
está FROZEN. O slice MVP-first está definido acima, mas a sua execução requer
autorização explícita.

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

Esta baseline estabelece a fundação para modelação sólida. Antes de iniciar
B7, executar prerequisite review: basic intersections, additional geometric
predicates e possivelmente Circle, conforme o caminho Topology/Sketch.
Não implementar estes pré-requisitos antecipadamente neste alignment.

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

B3 Circle gap MUST be closed before the Sketcher slice that needs circles/arcs.
Circle torna-se obrigatório antes/durante B8, em trabalho explicitamente autorizado.

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

MVP FREEZE apenas quando todo este workflow estiver funcional e validado,
mediante autorização explícita. Só então o repositório poderá ser marcado
com uma baseline/tag MVP estável.

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
