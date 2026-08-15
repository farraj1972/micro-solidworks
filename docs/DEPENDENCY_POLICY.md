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
| GLAD | INTEGRATE | `microsw_rendering` e `OpenGLContext` |
| Dear ImGui | INTEGRATE | `microsw_ui`, `ImGuiLayer` e `ApplicationShell` |

Estas dependências são adquiridas de forma reproduzível pelo CMake, sem exigir
instalação global manual.

## 5. Capacidades internas aprovadas

- Math: `BUILD`, ainda não implementado em B0.
- Educational Geometry Kernel: `BUILD` com possível substituição futura,
  ainda não implementado em B0.

## 6. Dependências deferred

GLM, Eigen, CGAL, OpenCascade, Qt, SDL e Vulkan não fazem parte da tecnologia
actual e não podem ser introduzidos sem decisão explícita. `Deferred` não
significa exclusão permanente; significa que uma avaliação futura deve ser
justificada e aprovada no contexto do roadmap.
