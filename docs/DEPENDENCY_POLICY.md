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
| OpenGL | INTEGRATE | OpenGL 3.3 Core; `OpenGLContext`, `ShaderProgram`, `LineRenderer` e pass do `WorkspaceViewport` |
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
  O executável principal ainda não consome Geometry.
- Educational Geometry Kernel: a fundação de valores, queries e métricas
  geométricas já existe; um kernel CAD completo, Topology e Modeling continuam
  por implementar. Preserva-se a estratégia educacional BUILD com possível
  substituição futura controlada de ADR-0006. CGAL, OpenCascade e outros
  kernels externos não estão integrados em B3.

`WorkspaceViewport`, navegação, `ReferenceGrid`, `ReferenceAxes`,
`ShaderProgram` e `LineRenderer` são componentes/fronteiras project-owned,
não dependências externas. Rendering não depende de Viewer. As dependências
CMake concretas estão documentadas em `ARCHITECTURE.md`; B2.12 não altera
targets, bibliotecas ou estratégias aprovadas.

## 6. Dependências deferred

GLM, Eigen, CGAL, OpenCascade, Qt, SDL e Vulkan não fazem parte da tecnologia
actual e não podem ser introduzidos sem decisão explícita. `Deferred` não
significa exclusão permanente; significa que uma avaliação futura deve ser
justificada e aprovada no contexto do roadmap.
