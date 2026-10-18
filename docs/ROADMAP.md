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

Validation follows the normative impact-based policy in `AGENTS.md`: normal
localized increments use V1, cross-layer vertical slices use V2, and baseline
validation uses V3. A documentary freeze after an unchanged validated executable
uses V0 rather than repeating V3.

## Implementation Mapping

| Fase canónica / scope | Realização funcional | Implementação técnica / gaps |
| --- | --- | --- |
| B0 Foundation | SATISFIED | COMPLETE / FROZEN; nenhum gap material |
| B1 Mathematical Foundation | SATISFIED | COMPLETE / FROZEN; nenhum gap material para o roadmap actual |
| B2 3D Viewer | SATISFIED | COMPLETE / FROZEN |
| B3 Geometric Primitives: point, line, segment, plane, circle, basic intersections | SATISFIED ACROSS FROZEN B3/B8 AND V1.1-C1 | B3 técnica FROZEN supplies primitives/queries; B8 supplies Circle2; V1.1-C1 supplies the required Line/Arc/Circle intersection subset and closes GAP-GEO-002 |
| B4 Scene & Object Model: identity, transforms, visibility, object lifecycle | PARTIALLY SATISFIED | B4 técnica FROZEN: Presentation identity, GeometryPresentation, visual ownership boundary, visualização Point/Segment/Line e o slice de picking/hover/single-selection/highlight |
| B5 Selection & Picking: screen ray, intersections/picking, hover, click selection, selection state, highlighting | REALIZED / SATISFIED BY B4 | FUNCTIONALLY REALIZED BY FROZEN TECHNICAL B4; No duplicate B5 implementation is required |
| B6 Transformations: translate, rotate, scale, local/world coordinates | REALIZED / FROZEN | B6.1–B6.10 COMPLETE; 708/708 testes e runtime PASS; D5 FROZEN |

A realização de B5 usa PickingRay e picking geométrico screen-space existentes;
não implica um motor geral de intersecções nem picking com oclusão perfeita.
As operações matemáticas B1 e a navegação B2 não realizam transforms de entidades B6.

Detalhe do B4 canónico:

| Capability | Mapping |
| --- | --- |
| identity | PARTIALLY REALIZED: identidade visual local, não identidade CAD persistente/global |
| transforms | REALIZED BY frozen B6 |
| visibility | NOT YET REALIZED as explicit entity state |
| object lifecycle | NOT YET REALIZED beyond construction/lookup |

Logo, B4 técnico FROZEN não significa B4 canónico completamente realizado.
Não existe freeze técnico B5 separado implícito neste mapping.

## Canonical Roadmap Gaps

Registo documental simples de Canonical Gaps; não cria tracker ou framework.

| ID | Gap | Blocking now | Expected closure |
| --- | --- | --- | --- |
| GAP-GEO-001 | CLOSED — Circle2 implemented and validated in B8.1 | NO | B8.1 targeted geometry validation PASS |
| GAP-GEO-002 | CLOSED — segment/segment, segment/circle, segment/arc, circle/circle, circle/arc and arc/arc classification implemented and validated in V1.1-C1 | NO | V1.1-C1 C1.8/C1.9 targeted and final V3 validation |
| GAP-SCENE-001 | CLOSED — Entity transforms in frozen B6 | NO | B6.10 PASS / B6.FREEZE |
| GAP-SCENE-002 | PARTIALLY CLOSED — EXECUTABLE V1.1-C1 SLICE: active Sketch, show/hide and deterministic edit/display lifecycle | NO for C1; broader lifecycle remains deferred | General scene graph and persistent visibility remain future scope |

Circle2 is implemented and GAP-GEO-001 is CLOSED. V1.1-C1 implements and
validates the D14-required basic intersections, closing GAP-GEO-002 without
changing the frozen D9/B10 historical slice. Frozen D13 plus the executable C1
editor provide partial closure of GAP-SCENE-002 through explicit visibility
ownership, active editing context and deterministic display lifecycle. General
scene graph and broader object lifecycle remain deferred.

## Delivered Functional Slice — B6

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
B6 está FROZEN. B6.1–B6.10 COMPLETE; freeze V0 após V3 PASS, sem alterações executáveis.

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

B6 está FROZEN, após D5 — Transformation Semantics FROZEN.
A revisão de pré-requisitos B7 está concluída; D6 está FROZEN e B7 é a campanha
activa autorizada.
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

Estado: FROZEN; D5 — Transformation Semantics
está FROZEN. O slice MVP-first está implementado sob
autorização explícita.

Capacidades:

- translate;
- rotate;
- scale quando aplicável;
- local/world coordinates;
- transform composition.

---

# B7 — Topological Model

Estado: FROZEN. A prerequisite review found Circle, basic
intersections and visibility non-blocking for the first cuboid slice. D6 —
Topology Ownership, Identity & Orientation is FROZEN in accepted ADR-0025–0027.
The B7 implementation provides the authorized deterministic cuboid vertical
slice. Baseline validation passed with 719/719 tests and runtime smoke.

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
B7, follow frozen D6. The first slice is one programmatic cuboid with
8 shared vertices, 12 shared straight edges, ordered convex planar outer wires,
6 planar faces, one closed manifold shell and one simple solid. Circle, curved
topology, general intersections, visibility and lifecycle remain deferred and
must not be implemented speculatively.

B7.1–B7.10 are COMPLETE in the baseline candidate: `microsw_topology`, typed
aggregate-local IDs, shared Vertex/Edge records, oriented closed Wires, convex
planar Faces, oriented connected 2-manifold Shell validation, one-root-Shell
Solid ownership, deterministic inspection and cuboid construction. ADR-0025,
ADR-0026 and ADR-0027 are CONFORMANT.

B7.11 is COMPLETE; B7.FREEZE is FROZEN.

At the B7 freeze, `GAP-GEO-001`, `GAP-GEO-002` and `GAP-SCENE-002` were OPEN.
B8.1 subsequently closed `GAP-GEO-001`; the other two remain OPEN / deferred.

---

# B8 — Sketcher

Estado: FROZEN — B8.1–B8.12 COMPLETE; B8.FREEZE FROZEN. D7 — Sketch
Coordinates, Ownership & Curve Semantics is FROZEN in accepted ADR-0028–0031.

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

Historical prerequisite: the B3 Circle gap had to close before the Sketcher
slice needing circles/arcs. B8.1 implemented Circle2 and closed GAP-GEO-001.

The implemented slice keeps authoritative geometry in Sketch-local 2D
coordinates, closes `GAP-GEO-001` with validated Circle2, adds Arc2, and derives
a shared world presentation for rendering and picking. It provides global-XY
Line/Rectangle/Circle/Arc tools, stable logical selection, replacement editing
and tombstone deletion. `GAP-GEO-002` and `GAP-SCENE-002` remain OPEN / deferred.
The B8 V3 validation passed a clean configure/full Debug build with zero
warnings, 746/746 CTest tests and runtime smoke. ADR-0028–0031 are CONFORMANT.
Constraints remain B9 scope.

---

# B9 — Constraint System

Estado: FROZEN — B9.1–B9.16 COMPLETE; B9.FREEZE FROZEN. D8 — Constraint
Ownership, References & Solver Semantics is FROZEN in accepted ADR-0032–0034.

Objectivo:

Introduzir relações paramétricas.

Canonical B9 capability list:

- coincident;
- horizontal;
- vertical;
- parallel;
- perpendicular;
- distance;
- radius;
- angle.

Inicialmente poderá ser utilizado um solver educacional simples.

Frozen implemented B9 scope:

- Coincident for supported point references;
- Horizontal and Vertical for lines;
- Parallel and Perpendicular for lines;
- driving HorizontalDistance and VerticalDistance;
- driving LineLength and CircleRadius;
- dense educational LM solver with transactional commit.

General distance, angle dimensions and advanced constraint types remain future
canonical capabilities; they are not part of the frozen technical B9 scope.

Frozen D8 defines Sketch-owned logical constraints with stable local identity
and strong sub-element references. It specifies a project-owned
Levenberg–Marquardt service in `microsw_constraints`, finite-difference
Jacobians, normalized residuals, explicit solver tolerances and atomic
candidate-state commit. The initial scope is endpoint/center coincidence,
line horizontal/vertical/parallel/perpendicular relations, driving horizontal
and vertical distances, line length and circle radius.
The frozen baseline adds stable constraint identities and references,
validated lifecycle and atomic Sketch replacement, normalized residuals,
central finite differences, a dense project-owned Levenberg–Marquardt solver,
under-constrained stabilization, diagnostics, rectangle/circle fixtures and
minimal constraint editing UI. Its V3 passed a clean configure/full Debug build,
zero warnings, 758/758 CTest tests and manual runtime validation.
ADR-0032–0034 are CONFORMANT.

---

# B10 — Extrusion

Estado: FROZEN — B10.1–B10.18 COMPLETE; B10.FREEZE FROZEN.
D9 — Profile, Extrusion & Modeling Boundary is FROZEN in accepted ADR-0035–0037.

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

Implemented first slice: extract one strictly-convex CCW line-only Profile from
a solved Sketch, extrude it by a positive distance into shared manifold topology,
and present/pick/select the resulting Solid as one wireframe object. The
Application owns one active result, preserves it on rejected regeneration and
offers explicit distance editing and manual regeneration. ADR-0035–0037 are
CONFORMANT. Circle/Arc extrusion, curved topology, feature history and automatic
rebuild remain deferred. GAP-GEO-002 and GAP-SCENE-002 remain OPEN / deferred.
The B10 V3 passed clean configure/full Debug build, zero warnings, 767/767
CTest tests and manual runtime validation. See `docs/B10_VALIDATION.md`.

---

# B11 — Boolean Operations

Estado: FROZEN. B11 completed the restricted axis-aligned Boolean vertical slice and passed 775/775 tests plus runtime validation. D10 — Axis-Aligned Boolean
Semantics & Non-Convex Solid Validation is FROZEN in accepted ADR-0038–0040.

Capacidades:

- union;
- difference;
- intersection.

Prioridade inicial:

correctness e compreensão, não robustez industrial.

Proposed first slice: Union, Difference and Intersection of validated
axis-aligned box Solids through coordinate-cell decomposition. A successful
result is one connected, hole-free, single-shell orthogonal Solid; empty is an
explicit result, while touching, near-coincident, disconnected, cavity and
through-hole cases are unsupported. Individual Faces remain convex rectangles,
possibly as multiple coplanar Faces. GAP-GEO-002 and GAP-SCENE-002 remain OPEN.

---

# B12 — Feature Tree

Estado: FROZEN. D11 — Feature Ownership, Dependency Graph &
Transactional Rebuild is FROZEN in accepted ADR-0041–0044.

Objectivo:

Representar a construção histórica do modelo.

Exemplo:

Part
├── Origin
├── Sketch001
├── Extrude001
├── Sketch002
└── Cut001

Approved first slice: a project-owned `FeatureModel` owns an append-only ordered
history of Sketch, Extrude and Boolean feature records. Stable model-local
`FeatureId` values, explicit backward-only dependencies, authoritative feature
parameters and derived result caches keep history separate from Presentation
and infrastructure. B12 is an implementation checkpoint within the combined
B12/B13 campaign; it does not independently require V3, freeze or a tag.

---

# B13 — Parametric Rebuild

Estado: FROZEN. D11 is FROZEN. The combined V3 passed 785/785 tests, zero
warnings and runtime validation. B12 and B13 share the final validated freeze
state.

Objectivo:

Permitir alterar parâmetros anteriores e reconstruir features dependentes.

Conceitos:

- dependency graph;
- dirty state;
- ordered rebuild;
- error propagation;
- feature validity.

Esta é uma das baselines fundamentais para transformar o modelador 3D num pequeno CAD paramétrico.

Approved rebuild semantics: edits mark the affected dependency closure dirty;
an explicit global rebuild evaluates it in deterministic history order. The
whole affected closure commits atomically. A failed attempt preserves
last-known-good derived caches while reporting FAILED and
BLOCKED_BY_DEPENDENCY states. Sketch solving occurs on a copy, and stable
`FeatureId` to `VisualEntityId` mapping is refreshed only after commit.

The campaign closes with one B13 V3 validation followed by a V0 documentary
freeze for both B12 and B13. Both baseline tags point
to that same freeze commit. Persistence, undo/redo, suppression, branching and
general feature types remain deferred.

---

# B14 — Persistence

Estado: FROZEN. D12 — Persistence Schema, Identity &
Transaction Semantics is FROZEN in accepted ADR-0045–0048.

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

Approved first slice: persist the authoritative `FeatureModel` as deterministic
JSON `.microsw` format version 1. Persistent aggregate-local feature, entity and
constraint IDs and their explicit next-ID counters survive Save/Open; derived
modeling caches, topology/visual identities and session state do not. Save is
atomic and accepts failed or dirty authoritative work. Open validates into a
temporary model, rebuilds deterministically, and replaces the session only after
structural success. A rebuild failure remains a successfully loaded repairable
model. The frozen dependency is
`microsw_persistence -> microsw_features`, never the reverse.

B14 will use targeted V1 increments, V2 application integration, and one final
V3 that also validates the executable MVP workflow. Implementation provides
JSON v1 roundtrip, persistent logical IDs/counters, validated reconstruction,
atomic Save, transactional Open, New/Save/Save As/Open session commands and
automatic rebuild. GAP-GEO-002 and GAP-SCENE-002 remain OPEN / deferred.

The final B14/MVP executable V3 passed a fresh configure, full Debug build with
zero project warnings, 793/793 tests and the complete runtime persistence smoke.
ADR-0045–0048 are CONFORMANT. See `docs/B14_VALIDATION.md`.

---

# MVP FREEZE

Estado: FROZEN — MICRO SOLIDWORKS MVP 1.0.

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

D12 policy: after the final B14 V3, an unchanged executable state allows
B14.FREEZE and MVP.FREEZE to run as V0 documentary steps. Their frozen tags
are `b14-persistence` and annotated `mvp-1.0` with message
`Micro SolidWorks — MVP 1.0`.

Frozen MVP capabilities:

- Foundation and internal Math;
- 3D Viewer;
- Geometry and Presentation/Selection;
- Transformations;
- Topology;
- Sketcher and Constraints;
- Extrusion and Boolean Operations;
- Feature Tree and Parametric Rebuild;
- Persistence.

The B14 final V3 is the MVP executable validation: fresh configure, full Debug
build, zero warnings, 793/793 tests and complete runtime smoke passed. The MVP
freeze changes documentation only and does not repeat V3. GAP-GEO-002 and
GAP-SCENE-002 remain OPEN / outside MVP.

---

# Post-MVP — Micro SolidWorks v1.1

MVP 1.0 remains FROZEN. D13 is FROZEN and ADR-0049–0052 are ACCEPTED. The
initial v1.1 roadmap is:

| Campaign | State | Functional direction |
| --- | --- | --- |
| V1.1-C1 — CAD Workspace & Guided Sketching | FROZEN | CAD interaction shell plus snap/infer Sketch creation, direct logical selection, graphical constraints/dimensions, general multi-Solid extrusion and derived curved presentation |
| V1.1-C2 — Solid Visualization & Selection | FROZEN | Derived shaded solids, topology-edge overlay, logical face/edge selection, multi-body display, Fit/views and visibility validated under ADR-0056–0058 |
| V1.1-C2A — Feature Manager & Display Mode UX Alignment | FROZEN | CAD-first tree hierarchy and immediately distinct Shaded, Shaded With Edges and Wireframe modes |
| V1.1-C3 — Document Safety, Undo/Redo & Session Workflow | FROZEN | Safe authoritative history, native document workflow and unsaved-change protection |
| V1.1-C4 — Subtractive Modeling & Sketch-on-Face | NOT STARTED / NEXT ACTIVE | Frozen D18 contract: associative Extrude-cap Sketch support, exact Cut Extrude and persistence v2 |
| V1.1-C5 — Feature Workflow Hardening | PLANNED / SUBJECT TO REVIEW | Feature interaction and rebuild usability |

V1.1-C1 is graphics-area centric, mouse-first, context-sensitive and icon/tool
driven. Its conceptual workspace is:

```text
+------------------------------------------------------+
| Menu / Application commands                         |
+------------------------------------------------------+
| CAD Command Bar / tabbed tool groups                |
+----------------------+-------------------------------+
| Feature / Property   |                               |
| Manager              |        Graphics Area          |
|                      |                               |
+----------------------+-------------------------------+
| Status / contextual hints                           |
+------------------------------------------------------+
```

The Graphics Area keeps most screen space. The CAD Command Bar supplies the
required General, Sketch, minimal Features and minimal View commands with
project-owned icons/tooltips and state-derived availability. The Feature
Manager uses human-readable names, status and visibility rather than IDs. The
Property/Tool Manager owns parameters, current selections, validation,
Confirm/Cancel and guidance. Context toolbar/menu expose only relevant actions.

Minimum C1 commands are New, Open, Save, Save As, Select and Rebuild; Line,
Rectangle, Circle, Dimension, Horizontal, Vertical and Coincident; and Extrude.
Existing Viewer navigation remains authoritative. Fit, standard views and
display-mode heads-up controls are prepared conceptually and added in C1 only
where low-risk and necessary.

V1.1-C1 target workflow:

```text
Launch → New
→ Sketch command → active Sketch mode
→ Rectangle in Graphics Area with visible snap/inference
→ mouse-select lines/endpoints
→ Horizontal/Vertical/Coincident from context or Command Bar
→ inspect glyphs and graphically edit a driving dimension
→ exit Sketch
→ select Sketch in Graphics Area or Feature Manager
→ Extrude → Property/Tool Manager → distance → Confirm
→ inspect result and synchronized Feature Manager selection
→ use relevant right-click context actions
→ Save
```

Normal operation never requires manual internal ID entry. Existing controls
that expose Feature, entity, constraint or result IDs must be classified during
C1 as `REMOVE FROM NORMAL UX`, `MOVE TO DEBUG/DIAGNOSTICS` or
`KEEP BECAUSE USER-FACING`. Line, Rectangle, Circle, Select, Dimension,
Horizontal, Vertical, Coincident, Extrude and Rebuild are direct tools. Hover
pre-highlights the next logical pick; active tools provide cursor/status hints;
Escape cancels transient steps and returns toward Select without partial commit.
Grid is a visual reference and optional snap source.

V1.1-C1 explicitly excludes trim, intersections beyond the D14-required subset, point-on-curve,
tangent/equal/concentric and arc constraints, face/edge selection, flat
shading, undo/redo implementation, persistent visibility and new modeling
features beyond general Boss/Base extrusion. Endpoint/midpoint/grid snapping and horizontal/vertical/
coincident inference are interaction aids; temporary inference is not solver
truth, while an accepted inference may create an explicit supported constraint.
Mouse gestures, constrained dragging and detailed grid configuration remain
deferred. The architecture stays ready for right-drag radial shortcuts and C2
heads-up Fit/standard-view/display controls. C1 prepares C2 flat shaded faces,
edge overlay and face hover/selection without implementing them.

The CAD interaction direction uses familiar mechanical-CAD command grouping,
context sensitivity, tree/graphics synchronization, property-based workflows
and mouse-centric selection. It does not copy trademarks, copyrighted artwork,
proprietary icons or a pixel-perfect third-party product layout. Micro
SolidWorks retains its own visual identity.

Campaigns retain V0/V1/V2/V3 governance. A prerequisite review is used only
when needed, a Decision Gate only for material architecture, and one V3 closes
each campaign. Historical B0–B14 numbering is unchanged. Longer-term Revolve,
Fillet/Chamfer, advanced curves/surfaces, exchange, assemblies, drawings,
acceleration and alternative-kernel work remain uncommitted future options.

Every user-facing v1.1 campaign also requires an Experience Gate before freeze.
The gate asks whether a mechanically CAD-literate user can discover and complete
the intended workflow without understanding implementation IDs. It evaluates
discoverability, mouse-first flow, command clarity, selection feedback, context
sensitivity, Feature Manager coherence, parameter editing and error/recovery
clarity. Technical V0/V1/V2/V3 validation and the Experience Gate are both
required evidence; neither substitutes for the other. V1.1-C1 must pass this
gate before its campaign freeze.

## D14 — General Planar Profiles & Curved Extrusion Semantics

```text
STATUS: FROZEN
TRIGGER: V1.1-C1 Experience Gate revealed that Circle and other valid closed
         Sketch regions cannot enter the frozen line-only B10 extrusion path.
```

The product requirement is SOLIDWORKS-compatible sketch eligibility for normal
solid Boss/Base extrusion. The accepted contract is recorded in ADR-0053–0055:

- eligibility derives from closed planar regions rather than entity count;
- multiple disjoint and nested closed contours are valid;
- Line, Arc and Circle boundaries are authoritative curves;
- open contours require a future Thin Feature or surface operation;
- branches, ambiguity, degeneracy and self-intersection produce localized
  diagnostics;
- curved topology and surfaces remain exact CAD state, with tessellation
  derived only for presentation and picking;
- multi-region extrusion commits all result bodies transactionally.

D14 preserves ADR-0035–0037 as the correct historical record of the MVP slice.
Accepted ADR-0053–0055 extend and supersede their line-only, strictly-convex,
single-Solid limitations for authorized post-MVP general-profile work.

```text
ADR-0001–0055: 55/55 ACCEPTED
general profiles: IMPLEMENTED IN V1.1-C1
curved topology: IMPLEMENTED FOR LINE/CIRCULAR ARC + PLANE/CYLINDER
multiple solids: IMPLEMENTED AS DERIVED FEATURE OUTPUT
GAP-GEO-002: CLOSED
GAP-SCENE-002: PARTIALLY CLOSED — EXECUTABLE V1.1-C1 SLICE
V1.1-C1: FROZEN
```

V1.1-C1 completed the required foundation sequence and its guided Extrude
workflow is accepted for the frozen C1 scope:

```text
Geometry intersections/classification
-> Planar region extraction
-> exact curved topology
-> general/multi-body extrusion
-> Presentation integration
-> CAD Extrude workflow acceptance
```

The implementation must cover the required robust classification subset for
segment/segment, segment/circle, segment/arc, circle/circle, circle/arc and
arc/arc, including crossings, endpoint and tangent contacts, and
coincident/overlap classification. GAP-GEO-002 closes only after implementation
and validation. Thin Feature, Selected Contours UI, persistent region naming,
advanced curves, surface/revolve/cut features, implicit body merge, general
Booleans and persistent body/subshape naming remain deferred.

## D15 — Solid Visualization & Sub-element Selection Semantics

`	ext
STATUS: FROZEN
ADR-0056–0058: ACCEPTED
ADR inventory: 58/58 ACCEPTED
V1.1-C1: FROZEN
V1.1-C2: FROZEN
`

D15 freezes Shaded With Edges as the default solid display, Wireframe as
the mandatory alternate mode, flat shading with basic ambient/directional
lighting, and independent exact-edge overlay. Topology and analytic geometry
remain authoritative; planar, holed and cylindrical face meshes remain derived.

Logical face and edge targets carry owning FeatureId, rebuild-local body context
and current FaceId/EdgeId. They survive navigation and same-topology Presentation
regeneration, but clear deterministically on topology regeneration. Feature
selection continues to survive rebuild. Persistent body and topological naming remain explicitly deferred.

Picking resolves derived triangles/curves to logical targets, gives an edge
within six logical pixels priority over a face, and selects only the nearest
visible geometric hit. Cylindrical faces resolve as logical faces and hole
interiors produce no cap hit. Hidden results are neither rendered nor pickable.
Fit and standard views belong to Viewer/session state and preserve the existing
right-handed XY-base, +Z-up convention and navigation bindings.

C2 passed an Experience Gate covering solid-form readability, face/edge
discoverability, hover clarity, selection confidence, standard-view discovery,
Fit usefulness, Feature Manager synchronization, context actions and visual
hierarchy. The final V3 passed a fresh configure, full Debug build with zero
warnings and 831/831 CTest tests; runtime and normal shutdown were confirmed.

C2 implements flat-shaded faces, exact-edge overlay, simple lighting,
Shaded With Edges/Wireframe display modes, face/edge hover and single logical
selection, owning-feature synchronization, visibility integration, multi-body
and cylindrical-face presentation/picking, Fit, standard views and compact
heads-up controls. Persistent topological naming, persistent bodies,
multi-selection unless UX evidence requires it, face-based Sketch creation,
Cut Extrude, Hole, fillet, chamfer, materials, textures, PBR, section view,
measure, assemblies and selection persistence across rebuild are excluded.
Selected Contours remains a separate modeling-workflow follow-up under D14 and
does not change the frozen C2 solid visualization and sub-element selection gate.

## D16 — Feature Manager Hierarchy & Display Mode UX Semantics

```text
STATUS: FROZEN
ADR-0059–0060: ACCEPTED
ADR inventory: 60/60 ACCEPTED
V1.1-C2A: FROZEN
V1.1-C3: FROZEN
V1.1-C4: NOT STARTED / NEXT ACTIVE
```

Frozen D16 defines a CAD-first Feature Manager derived from FeatureModel rather than a
raw record list. Modeling features are primary nodes; an Extrude visually
absorbs its primary Sketch and unconsumed Sketches remain at the Part root. The
first direct modeling consumer in history order is the future deterministic
absorbing parent for a shared Sketch, while secondary dependencies remain in
properties/context. Boolean operands are not duplicated into a dependency DAG.
Normal labels and status cues remain human-readable and internal IDs are
diagnostic only.

Frozen D16 defines explicit rendering-pass semantics. Shaded draws lit faces
without global topology edges. Shaded With Edges adds the topology-derived edge
overlay. Wireframe draws topology edges without face fill. Transient edge hover
or selection may remain visible in Shaded. Curved surfaces never expose
tessellation triangle edges, and hole/circular boundaries receive clear overlay
contrast only where the active mode enables it.

The corrective V1.1-C2A campaign covers hierarchy derivation, absorbed and
unconsumed Sketch presentation, labels/status cleanup, tree interaction,
display-mode passes and visual differentiation plus an Experience Gate. It
excludes Undo/Redo, Document Safety, Cut Extrude, Sketch on Face, persistent
topological naming, feature reorder, suppression, advanced shared-Sketch
semantics, reference geometry objects and materials/PBR. C2A is FROZEN after a
fresh configure, full Debug build with zero warnings, 837/837 tests and an
approved manual Experience Gate. C3 is FROZEN under the D17 contract.

## D17 — Document Safety, Undo/Redo & Session History Semantics

```text
STATUS: FROZEN
ADR-0061–0063: ACCEPTED
ADR inventory: 63/63 ACCEPTED
V1.1-C2A: FROZEN
V1.1-C3: FROZEN
V1.1-C4: NOT STARTED / NEXT ACTIVE
```

D17 freezes Application-owned document dirty and saved-baseline state distinct
from feature rebuild dirtiness. Successful New/Open/Save/Save As establish a
saved checkpoint; rebuild does not clear dirty. Undo or Redo reaching the saved
checkpoint is clean, while branch truncation or eviction of that checkpoint
keeps the document dirty until the next successful save.

Undo/Redo uses a bounded linear history of typed authoritative FeatureModel
checkpoints at D13 transaction boundaries. Checkpoints preserve model-local IDs,
tombstones, counters, dependencies, parameters and history order, while derived
caches and Presentation remain excluded and are rebuilt after restore. One
logical user transaction is one step; compound Rectangle creation and accepted
inference remain atomic. Failed rebuilds remain undoable, new edits after Undo
discard Redo, and Save retains history. The initial limit is 100 undoable
transactions plus the current checkpoint. The saved authoritative checkpoint
is retained separately from that bounded window so eviction cannot corrupt
dirty-state evaluation.

Dirty New/Open/Exit use Save / Don't Save / Cancel. Save failure or cancellation
preserves the current session. Open replaces the session only after the target
loads transactionally; New/Open clear history. Undo history, dirty state,
selection, visibility and view state are not persisted. Autosave, crash recovery,
recent files, persistent history and multi-document editing remain deferred.

V1.1-C3 implements and validates these frozen decisions with typed authoritative
checkpoints, bounded Undo/Redo, native Windows file dialogs and one reusable
unsaved-changes policy. Its final V3 passed 846/846 tests and the manual
Experience Gate was APPROVED. The V1.1-C4 prerequisite review renames the next
campaign to Subtractive Modeling & Sketch-on-Face and finds a Decision Gate is
required for semantic cap support, exact subtraction, target-result semantics
and persistence evolution. V1.1-C5 remains Feature Workflow Hardening.

## D18 — Sketch-on-Face, Exact Subtraction & Persistence v2 Semantics

```text
STATUS: FROZEN
ADR-0064–0066: ACCEPTED
ADR inventory: 66/66 ACCEPTED
V1.1-C4: NOT STARTED / NEXT ACTIVE
```

D18 freezes Option B: semantic StartCap/EndCap support on single-body Extrudes, associative Sketch planes, exact Blind subtraction over all intersected bodies in an explicit target result, 0..N transactional outputs, and persistence v2 with v1 migration. Through All, general topological naming and arbitrary face support remain outside C4.
