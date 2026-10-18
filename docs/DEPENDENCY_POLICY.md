# Micro SolidWorks — Dependency Policy

## 1. Objectivo

Cada dependência deve preservar o valor educacional, as fronteiras
arquitecturais e a capacidade de evolução do projecto. Uma biblioteca não é
introduzida apenas por conveniência.

## 2. Estratégias

### BUILD

Implementar no projecto quando compreender o conceito é parte central do
objectivo educacional ou quando é necessário controlar directamente as
invariantes do domínio.

### LEARN_THEN_REPLACE

Construir primeiro uma implementação didáctica, estabelecer uma fronteira
adequada e avaliar uma substituição apenas quando existirem requisitos ou
evidências concretas.

### INTEGRATE

Integrar uma biblioteca externa para infraestrutura com baixo valor pedagógico
de implementação própria. A integração deve ser reproduzível e, quando
prático, permanecer atrás de uma fronteira project-owned.

## 3. Critérios para novas dependências

Antes de introduzir uma dependência devem ser avaliados:

1. necessidade no increment autorizado;
2. valor educacional de uma implementação interna;
3. licença e manutenção;
4. impacto arquitectural e direcção das dependências;
5. isolamento de tipos externos;
6. reprodutibilidade e pinning;
7. custo de manutenção e substituição.

Uma nova dependência ou mudança de estratégia arquitectural requer aprovação.

## 4. Dependências aprovadas e implementadas

| Dependência | Estratégia | Fronteira actual |
| --- | --- | --- |
| GoogleTest | INTEGRATE | target de testes separado, descoberto por CTest |
| spdlog | INTEGRATE | `microsw_logging` e API project-owned `Logger` |
| GLFW | INTEGRATE | `microsw_windowing` e `ApplicationWindow` |
| OpenGL | INTEGRATE | OpenGL 3.3 Core; `OpenGLContext`, `ShaderProgram`, `LineRenderer`, `PointRenderer` e pass do `WorkspaceViewport` |
| GLAD | INTEGRATE | `glad_gl_core_33`, usado por Rendering e pelo pass do Viewer; sem tipos GL nas APIs públicas |
| Dear ImGui | INTEGRATE | `microsw_ui`, `ImGuiLayer` e `ApplicationShell` |

As bibliotecas são adquiridas de forma reproduzível pelo CMake. OpenGL é
fornecido pelo driver do sistema; execução e testes GPU exigem um contexto
OpenGL 3.3 funcional.

## 5. Capacidades internas aprovadas

- Math: `BUILD`, implementado em `src/core/math` na baseline B1 FROZEN;
  independente de UI, Viewer, Rendering e bibliotecas gráficas.
- Viewer camera/view/projection math: `BUILD` usando `microsw_math`,
  implementado em B2 conforme D2 (ADR-0010/0011): `OrbitCamera`,
  `viewMatrix`, `perspective`, `orthographic` e `ProjectionState`;
  sem GLM/Eigen.
- Geometry primitives/kernel foundation: `BUILD`, implementada em B3.1–B3.8
  no target project-owned `microsw_geometry`, conforme D3 (ADR-0013–0016).
  Depende apenas de `microsw_math` e da biblioteca standard C++; não depende
  de Viewer, Rendering, UI, Windowing, Logging ou futuras camadas de domínio.
  A aplicação consome Geometry através da Presentation e do Viewer em B4.
- Educational Geometry Kernel: a fundação de valores, queries e métricas
  geométricas já existe; um kernel CAD completo, Topology e Modeling continuam
  por implementar. Preserva-se a estratégia educacional BUILD com possível
  substituição futura controlada de ADR-0006. CGAL, OpenCascade e outros
  kernels externos não estão integrados em B3.
- Geometry presentation/adaptation: `BUILD`, implementada em B4.1–B4.4:
  `microsw_presentation -> microsw_geometry -> microsw_math`; adapters no Viewer
  derivam batches de Point/Segment/Line sem alterar Geometry. Presentation não
  depende de Viewer ou Rendering.
- Entity transform semantics: `BUILD`, frozen in D5.
  `microsw::math::Transform3` is a Math value; Presentation owns the B6 entity
  transform and depends directly on Math while continuing to depend on Geometry.
  The resulting graph remains acyclic.
- Topological model: `BUILD`, frozen in B7 as a project-owned
  `microsw_topology` module depending only on `microsw_geometry` and transitively
  `microsw_math`. Shared indexed records, typed aggregate-local identity and
  oriented uses are specified by accepted ADR-0025–0027. The implemented target
  has no dependency on Presentation, Viewer, Rendering, UI or Application.
- Sketch model and planar circular geometry: `BUILD`, implemented in B8 under D7.
  `Circle2` and `Arc2` extend `microsw_geometry`; the implemented
  `microsw_sketch -> microsw_geometry -> microsw_math` core remains independent
  of Topology and infrastructure. Presentation/Application adapters may consume
  Sketch. ADR-0028–0031 are ACCEPTED and CONFORMANT; `microsw_sketch` has only
  the approved Geometry dependency. Presentation/Application consume Sketch.
- Constraint system: `BUILD`, frozen in D8. Logical constraint values,
  identities and element references belong to Sketch. The project-owned
  dense numeric solver is isolated in `microsw_constraints`, depending on
  `microsw_sketch` without a reverse dependency. ADR-0032–0034 are ACCEPTED;
  no external solver dependency is permitted in B9. The implemented dense,
  project-owned Levenberg–Marquardt target conforms to this boundary.
- Profile and extrusion modeling: `BUILD`, frozen in D9. The approved core
  edge is `microsw_modeling -> microsw_topology -> microsw_geometry ->
  microsw_math`. Modeling owns an ephemeral Profile value and one-shot
  extrusion; it does not depend on Sketch, Constraints, Presentation, Viewer,
  Rendering, UI or Application. A higher integration adapter may consume both
  Sketch and Modeling. B10 implements this project-owned core and its separate
  Application integration/presentation adapters. ADR-0035–0037 are ACCEPTED
  and CONFORMANT; B10 is FROZEN.
- Axis-aligned Boolean modeling: `BUILD`, frozen in D10. Boolean occupancy,
  temporary grid values and topology reconstruction remain inside
  `microsw_modeling`, preserving `microsw_modeling -> microsw_topology ->
  microsw_geometry -> microsw_math`. No BSP, external Boolean kernel or general
  intersection dependency is authorized. ADR-0038–0040 are ACCEPTED; B11 is FROZEN and the accepted D10 contract is CONFORMANT.
- Feature history and transactional rebuild: `BUILD`, frozen in D11.
  ADR-0041–0044 define a project-owned `microsw_features` domain module that
  depends on Constraints, Sketch and Modeling through a pure Sketch-to-Profile
  adapter. No reverse dependency from those core modules is permitted, and
  Feature core must remain independent of Application, Presentation, Viewer,
  Rendering and UI. Ordered history, explicit dependencies, rebuild state and
  derived result caches remain an internal educational implementation; no
  external feature engine or persistence dependency is authorized. D11 is
  FROZEN; ADR-0041–0044 are ACCEPTED and CONFORMANT. B12 and B13 are FROZEN.
  The implemented dependency is
  `microsw_features -> microsw_constraints / microsw_sketch /
  microsw_modeling`, while Presentation, UI and Application consume Features.
- Persistence: `BUILD` project-owned mapping, validation, transaction and
  filesystem boundary with an `INTEGRATE` JSON implementation, frozen in D12.
  ADR-0045–0048 authorize a reproducibly pinned MIT-licensed `nlohmann/json`
  3.11.3 single-header release and MIT license in `third_party/nlohmann`, isolated
  inside `microsw_persistence`. No JSON type
  may cross its public API. The frozen dependency is
  `microsw_persistence -> microsw_features`; Features and lower domain modules
  must not depend on Persistence. D12, B14 and MVP 1.0 are FROZEN.
- Editor interaction and display policy: `BUILD`, frozen in D13. A minimal
  Application-owned editor context consumes Features, Sketch/Constraints,
  Presentation and Viewer mapping without introducing reverse dependencies.
  Snapping/inference, logical selection coordination, visibility policy and
  command transactions remain Application/interaction concerns. Domain modules
  do not depend on Editor/UI, and no new external dependency or editor module is
  authorized by accepted ADR-0049–0052.
- CAD workspace icons and interaction visuals: `BUILD` for V1.1-C1 using
  project-owned simple vector/icon primitives and the existing Dear ImGui/
  Rendering boundaries. No icon library is approved by D13. Any external icon
  dependency must be justified and explicitly authorized before integration;
  proprietary CAD-product assets must not be copied.
- Geometric picking: `BUILD`, implementado em B4.6 no Viewer, com tolerance
  de 6 pixels lógicos, distinta da tolerance geométrica; sem framebuffer picking.
- Hover/single-selection/highlight: `BUILD`, implementados em B4.7–B4.9 no
  Viewer; estados de identidade separados e batches exclusivos por estado visual.
  B4.10 valida a integração; B4.11 documenta-a, sem novas dependências.

`WorkspaceViewport`, navegação, `ReferenceGrid`, `ReferenceAxes`,
`ShaderProgram`, `LineRenderer` e `PointRenderer` são componentes/fronteiras project-owned,
não dependências externas. Rendering não depende de Viewer. As dependências
CMake concretas estão documentadas em `ARCHITECTURE.md`; B4.11 não altera
targets, bibliotecas ou estratégias aprovadas.

## 6. Dependências deferred

GLM, Eigen, CGAL, OpenCascade, Qt, SDL e Vulkan não fazem parte da tecnologia
actual e não podem ser introduzidos sem decisão explícita. `Deferred` não
significa exclusão permanente; significa que uma avaliação futura deve ser
justificada e aprovada no contexto do roadmap.

## 7. D14 general-profile strategy

General planar region extraction, the required Line/Arc/Circle intersection and
classification subset, exact curved topology and general extrusion use the
approved `BUILD` strategy and are implemented in V1.1-C1. This preserves their
educational value and the internal Geometry and Topology decisions. The
dependency direction remains:

```text
features/integration -> sketch + modeling
modeling -> topology -> geometry -> math
presentation/rendering -> derived topology tessellation
```

No external geometry kernel or computational-geometry library is authorized.
Tessellation libraries cannot become authoritative CAD representation.
Persistence continues to store feature definitions rather than derived regions,
Solids or tessellation. Any demonstrated need for schema version 2 or a new
dependency is a hard stop and requires a separate decision.

## 8. D15 solid-display and picking strategy

Accepted ADR-0056–0058 freeze BUILD for face tessellation, flat shading, exact-edge
overlay, logical sub-element mapping, nearest-visible-hit picking, Fit and
standard views. No new dependency is proposed. The intended direction remains:

`	ext
topology/geometry -> authoritative CAD state
presentation -> derived face/edge data and logical mappings
viewer -> picking, view/session policy and visibility consumption
rendering -> identity-free draw execution
ui/application -> logical interaction coordination
`

Mesh, GPU and visual IDs must not cross into FeatureModel, Topology or persisted
state. Any request for an external tessellator, renderer or picking framework
requires separate dependency and architectural approval.

V1.1-C2 implements this BUILD strategy with project-owned planar/hole and
cylindrical tessellation, triangle rendering, curve overlay and CPU picking.
No dependency was added. Presentation derives from Topology/Geometry, Viewer
consumes Presentation for picking and view policy, Rendering stays independent
of CAD identity, and Application/UI coordinates logical EditorContext state.

## 9. D16 Feature Manager and display-mode UX semantics

Accepted ADR-0059–0060 require no new dependency. Feature Manager hierarchy remains a
BUILD concern in Application/UI, derived from the existing FeatureModel public
history and dependency contracts. It must not introduce UI dependencies into
Features or reinterpret presentation parenting as domain ownership.

Display-mode correction remains within Viewer/Rendering over existing derived
Presentation data. Viewer chooses face and topology-edge passes; Rendering stays
identity-free. No external tree, scene, tessellation or rendering framework is
authorized for V1.1-C2A.

V1.1-C2A completed this policy without adding dependencies. `FeatureTreeModel`
derives UI nodes from Features, and Viewer consumes the existing Presentation
face/edge data. No reverse dependency or CAD-state ownership change was added.

## 10. D17 document safety and undo/redo

D17 introduces no external dependency and does not change the persisted schema.
A typed authoritative checkpoint belongs at the Features boundary and
must not depend on Application, UI, Persistence or JSON. Application/session may
own a bounded sequence of those checkpoint values, the current position, saved
baseline marker and pending New/Open/Exit action.

Checkpoint restore uses existing Features and deterministic rebuild contracts;
Presentation remains derived. Persistence continues to depend on Features and
stores only the current authoritative model. Undo history, dirty state and
transient editor/display state do not cross the persistence boundary.

V1.1-C3 implements this policy without a new external dependency. Features owns
`FeatureModelCheckpoint`; Application owns `DocumentHistory`, saved state and
destructive-action policy; the GLFW boundary supplies the owner HWND to standard
Windows Open/Save dialogs. Persistence schema version 1 is unchanged.

## 11. D18 Sketch-on-Face and subtractive modeling

Accepted ADR-0064–0066 freeze BUILD. D18-R1 restricts C4 exact subtraction to cutting directions parallel or anti-parallel to recoverable target extrusion axes. Support resolution belongs to Features over Sketch/Modeling; exact axial prismatic subtraction belongs to Modeling/Topology/Geometry; Persistence v2 remains an adapter depending on Features. No external kernel, conic library, Boolean, tessellation or migration dependency is authorized. B11 stays frozen and rendering remains derived. General oblique subtraction and conic topology require a future Decision Gate.

## 12. D19 Viewer orientation

Accepted ADR-0067 keeps arbitrary camera orientation inside Viewer using the
project-owned Math library. The Application/Viewer boundary converts a
SketchPlane into a camera basis. Viewer must not depend on Sketch, and no
external quaternion, camera or scene library is authorized.

## 13. D20 feature deletion and generated-face semantics

Accepted ADR-0068–0069 freeze the existing BUILD strategy. Feature deletion
belongs to FeatureModel with Application/Editor transaction cleanup and uses no
new dependency. Generated-face provenance is derived at the Features over
Modeling/Topology boundary; persistence stores only FeatureId plus semantic role.
No external kernel, persistent FaceId, generic naming framework or schema v3 is
authorized by D20.
