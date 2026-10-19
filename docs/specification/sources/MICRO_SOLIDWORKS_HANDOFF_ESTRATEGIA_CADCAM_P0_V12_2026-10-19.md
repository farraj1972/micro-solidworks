# HANDOFF INTEGRAL — Micro SolidWorks / Personal SolidWorks / MICRO CAD-CAM

**Data de corte:** 19-10-2026  
**Objetivo:** transportar para outro chat o contexto estratégico, especificação funcional/UX/arquitetura, decisões formalizadas, estado do desenvolvimento e próximas ações sem confundir propostas com autorizações.  
**Estado deste handoff:** documento informativo de transferência, **não** constitui ADR, aprovação de implementação, alteração do roadmap canónico nem commit.

> **Instrução para o novo chat:** ler este documento integralmente; usar o repositório como fonte autoritativa para o estado do código, contratos e decisões congeladas; identificar explicitamente qualquer divergência; não iniciar implementação de v1.2 sem aprovação formal do proprietário. Responder em português de Portugal. Preparar prompts Codex executáveis, com scope, testes, Experience Gate e documentação, quando solicitado.

---

## 1. Identidade e missão

- **Repositório:** https://github.com/farraj1972/micro-solidworks
- **Branch principal real:** `master` (não `main`).
- **Sistema atual:** Micro SolidWorks, aplicação desktop Windows C++20 de CAD mecânico 3D.
- **Visão de produto aprovada em P0:** *Personal SolidWorks* é designação conceptual/provisória; Micro SolidWorks permanece nome técnico/repositório. Pretende-se substituir progressivamente workflows mecânicos pessoais reais, **não** replicar todo o SOLIDWORKS nem usar a sua marca/recursos.
- **Visão paralela ainda em DRAFT:** evolução para **CAD/CAM 3D open source**, desktop-first, mouse-first, paramétrico, interoperável e extensível por scripting. A visão de longo prazo não autoriza antecipar features fora da campanha atual.
- **Princípios de P0:** Product over Education; Workflow over Feature; Build or Integrate Pragmatically; Progressive Functional Replacement; CAD Data Integrity First.
- **Prioridade de produto proposta:** correctness → CAD data integrity → workflow completeness → usability → modularity → maintainability → performance → educational value. **Atenção:** `AGENTS.md` mantém uma hierarquia normativa histórica própria, incluindo Learning Value; P0 não a substitui silenciosamente. Qualquer conflito concreto exige decisão explícita.

## 2. Fontes de verdade e URLs

### Repositório — documentos vinculativos/consolidados

1. Decisões P0: https://github.com/farraj1972/micro-solidworks/blob/master/docs/PERSONAL_SOLIDWORKS_P0_DECISIONS.md
2. Proposta v1.2: https://github.com/farraj1972/micro-solidworks/blob/master/docs/PERSONAL_SOLIDWORKS_V1_2_CAMPAIGN_PROPOSAL.md
3. Roadmap canónico: https://github.com/farraj1972/micro-solidworks/blob/master/docs/ROADMAP.md
4. Revisão de arquitetura de produto P0: `docs/PERSONAL_SOLIDWORKS_PRODUCT_ARCHITECTURE_REVIEW.md`.
5. Matriz de capacidades P0: `docs/PERSONAL_SOLIDWORKS_CAPABILITY_MATRIX.md`.
6. Contratos técnicos: `AGENTS.md`, `docs/DEPENDENCY_POLICY.md`, ADR-0001–0069, documentação de arquitetura, validação e releases no repositório.
7. Pontos de código para investigação: `src/features/FeatureModel.h/.cpp`, `src/presentation/SolidDisplay.h` e as implementações CAD/topology/geometry relevantes.

### Documento de visão transversal separado

`MICRO_CADCAM_PRODUCT_SPEC_V0_1.md` — versão 0.1 criada na conversa, DRAFT, 19-10-2026. Reúne benchmark SOLIDWORKS/CATIA/Fusion/FreeCAD, domínio, arquitetura de referência, UX, interoperabilidade, scripting, CAD→CAM, user stories, roadmap R0–R9 e fontes públicas. **Não está demonstrado que este ficheiro tenha sido incorporado no repositório ou aprovado como documento normativo.** Deve ser transportado como anexo independente, não como revisão tácita do P0.

## 3. Estado formal verificado no remoto

- Commit P0.FREEZE em `master`: **`9405bcebd49826626da20650e41951dcda2e0e9e`**, mensagem `[P0] Freeze Personal SolidWorks product strategy`, **push confirmado** por consulta ao HEAD remoto.
- `P0.FREEZE`: **PASS / P0 FROZEN** (19-10-2026), revisão estratégica documental.
- `v1.1.0`: **FROZEN**, commit-base `774f55c`, tag `v1.1.0`; esta referência deve ser preservada.
- **69/69 ADRs ACCEPTED**, sem ADR novo introduzido por P0.
- `v1.2`: **APPROVED AT CAMPAIGN LEVEL** (âmbito estratégico), mas plano detalhado C1–C4 no estado **READY FOR REVIEW — NOT AUTHORIZED FOR IMPLEMENTATION**.
- Kernel definitivo K1/K2/K3: **OPEN — EVIDENCE REQUIRED**.
- **Nesta conversa** foi emitida **recomendação consultiva** `APPROVED WITH CHANGES` para a proposta v1.2, **não é aprovação formal do proprietário nem estado gravado no repositório**.
- P0.FREEZE alterou documentação; não executou build, CTest, runtime nem gates da v1.2; `git diff --check` passou. Não presumir que isto valida implementação.
- A v1.1.0 mantém nível **N1 para o workflow completo**, apesar de ter funcionalidades paramétricas funcionais; faltam STL validado e distribuição Windows suportada. Não declarar N2 antes do gate completo.

## 4. Decisões P0-D01–D07 (formalmente aprovadas)

| Decisão | Estado | Conteúdo e limites |
|---|---|---|
| P0-D01 Posicionamento | APPROVED | CAD paramétrico pessoal para tarefas mecânicas reais; substituição gradual de workflows, sem pretensão de paridade geral com software comercial. |
| P0-D02 Workflows | APPROVED | Peças prismáticas individuais — placas, flanges, suportes/adaptadores com furos/bolsões axiais. Cadeia Create→Sketch→Constrain/Dimension→Model→Edit→Rebuild→Save/Open→Export→validar produção. Não inclui montagens. |
| P0-D03 Kernel | APPROVED WITH EXPLICIT LIMITATION | Kernel interno Math/Geometry/Topology apenas para primeiro N2; CAD FeatureModel/topology autoritativos, malha derivada. Não iniciar OCCT/CGAL. K1 interno, K2 híbrido e K3 externo continuam abertos; estudar eventual spike isolado principalmente OCCT no futuro. |
| P0-D04 Interoperabilidade | APPROVED | STL validado primeiro; formato nativo `.microsw` preservado. STEP/DXF apenas avaliação futura; STL é malha sem histórico nem unidade fiável. Definir qualidade de malha, orientação, tolerância, escala, múltiplos corpos, repetibilidade e verificação independente. |
| P0-D05 Primeiro N2 | APPROVED | Placa adaptadora paramétrica com furo circular axial, bolsão axial, edição/rebuild, Undo/Redo, Save/Open, STL e slicer independente, sem exigir impressão física. |
| P0-D06 v1.2 | CAMPAIGN LEVEL APPROVED; PLAN PENDING | Planeamento de exportação STL, validação da malha, Release/portable Windows, peça de referência, Experience Gate e guia. Exclui Revolve, Fillet, Chamfer, Hole Wizard, assemblies, Boolean geral, topological naming persistente, STEP/DXF, troca de kernel e CAM toolpaths. |
| P0-D07 Architecture Gates | APPROVED | Abrir gate futuro apenas em escolha material: kernel/representação/dependências, persistent naming, B-rep Boolean geral, STEP exato, reorder/suppression, persistência/ownership. Não abrir gate burocrático só por classe STL ou zip portátil. |

## 5. Níveis de autonomia e evidências

- **N0** demonstração; **N1** workflow assistido; **N2** substituição funcional para **job concreto e completo**; **N3** workflow pessoal preferido.
- N2 do primeiro workflow só pode ser afirmado depois de: criação/edição/rebuild paramétrico, Undo/Redo, Save/Open nativo, STL watertight validado, escala/unidades/orientação confirmadas num consumidor externo (slicer), preparação de impressão e execução através de pacote Windows suportado sem ambiente de desenvolvimento.
- Uma impressão física é evidência opcional. **CAD solid válido ≠ tesselação válida ≠ STL watertight ≠ aceitação por slicer ≠ peça fisicamente imprimível**.
- Atribuir N2 **apenas à família de peças/workflow validada**, nunca ao produto global.

## 6. Primeira peça N2 — fixture sintética candidata

**Antes do preflight não alegar que a geometria já funciona.** Geometria candidata reproduzível (não é peça real do utilizador):

- Placa: **80 × 50 × 8 mm**, eixos/posição segundo convenção fixada no preflight.
- Furo axial passante: **Ø8 mm**, centro local **(-22, 0) mm**.
- Bolsão cego axial retangular: **18 × 14 mm**, centro **(+16, 0) mm**, profundidade **2 mm**.
- Sketch base: loop exterior retangular **mais loop interior circular**, cujo extrude gera diretamente o furo. Na face adequada: um único **Cut Extrude** cria o bolsão.
- Editar largura **80→90 mm** e profundidade de bolso **2→3 mm**, reconstruir, exercitar Undo/Redo e Save/Open, sem perder furo/bolsão.
- **Limite conhecido:** `FeatureModel::rebuild` exige target `Extrude` para Cut Extrude, rejeitando Cut→Cut encadeado. O fixture evita a segunda subtração. A viabilidade do caminho alternativo necessita prova antes da autorização.

**Oráculos analíticos propostos**, assumindo recortes separados, dimensões e direções corretas:

- Volume inicial: `80×50×8 − π×4²×8 − 18×14×2 = 31 093,876110... mm³`.
- Volume após duas edições: `90×50×8 − π×4²×8 − 18×14×3 = 34 841,876110... mm³`.
- Bounding box esperado com a placa alinhada aos eixos: inicialmente dimensões **80×50×8 mm**; após edição **90×50×8 mm**. A localização e o referencial devem ser concretizados no preflight.
- **Atenção:** estes números são expectativas matemáticas, **não resultados obtidos na aplicação ou no STL**; tolerâncias de comparação serão estabelecidas em C1.
- Fixture adicional: bolsão circular/limite curvo para testar a tesselação; e casos de múltiplos corpos e falhas controladas para o contrato STL.

## 7. Campanha v1.2 detalhada — proposta, não autorização

| Incremento | Finalidade | Aceitação fundamental |
|---|---|---|
| **V1.2-C1** STL Export Contract & Mesh Validation | Fronteira sólido CAD→malha derivada; regras de qualidade, tolerâncias, erros, body selection e repetibilidade. | Fixtures/oráculos, decisão de tesselação por adapter CAD, qualidade mensurável, pré-requisitos sólidos. |
| **V1.2-C2** STL Export Implementation | Exportação de ficheiro pelo utilizador, escrita determinística, feedback/diagnósticos e integração UI; `.microsw` inalterado. | Parser independente, slicer, suporte correto a planar holed/cylindrical faces; falhas não produzem STL parcial. |
| **V1.2-C3** Windows Release Distribution | Build Release reproduzível e pacote portátil com inventário runtime/DLL/assets. | Windows 10 clean-machine: iniciar, Save/Open, Export, fechar fora da árvore de desenvolvimento. |
| **V1.2-C4** Mechanical Reference Workflow & N2 Validation | Guia, cenário completo, evidência Técnica/Experiência/N2. | Workflow mouse-first sem IDs, slicer confirma dimensões e furos/bolsões, preparação de impressão. |
| **V1.2.FREEZE** | Encerramento documental e de release. | Somente depois de C1–C4 aceites com evidência e N2 estritamente delimitado. |

### Especificação de exportação STL a decidir em C1

- Fonte autoritativa: `FeatureModel` + `topology::Solid`, nunca buffers OpenGL/presentation GPU.
- Malha derivada reprodutível de faces planares com wires interiores, contornos circulares e faces cilíndricas; seam vertices compartilhados, tampas corretas, winding para fora, normais calculadas.
- Triângulos finitos, não degenerados; arestas com incidência correta, componentes fechadas/manifold; volume assinado positivo em orientação definida; sem auto-interseções relevantes dentro do âmbito.
- Tolerâncias geométricas/angulares/chord, limite de triângulos e critérios empíricos de aceitação; **não** usar automaticamente tolerâncias de viewport.
- Seleção correta do resultado atual, não exportar corpos históricos escondidos; definir contrato para múltiplos corpos e rejeição/aceitação de desconexos/sobrepostos.
- Binário STL é candidato, mas decisão deve ser registada; **STL não codifica unidades de forma robusta** — usar mm na UI/documentação e verificar no consumidor.
- Casos negativos: sólido vazio/inválido, feature FAILED/BLOCKED, NaN/Inf, limite de tesselação, malha aberta/nonmanifold, seleções ambíguas, erros de I/O; preferir exportação transacional.

### Gates de v1.2

1. **Technical Gate:** geometria/volume/bounds, rebuild e Undo/Redo, roundtrip `.microsw`, STL em parser independente, qualidade/normais/escala, integração/regressão, clean-machine smoke.
2. **Experience Gate:** utilizador com literacia CAD encontra sequência, escolhe sketch/face, compreende corte/edição/exportação e recupera erros usando GUI/rato, sem IDs técnicos.
3. **N2 Gate:** consumidor/slicer independente, nome e versão, escalas e orientação, geometria identificável, preparação impressão no pacote suportado. Impressão física opcional.
4. **Governance Gate:** documentação/ADRs/dependências intactas ou alteração aprovada, evidência reprodutível e commit no fecho; push é ato separado sob controlo do utilizador.

## 8. Resultado da revisão consultiva nesta conversa

Foi feita leitura remota das decisões P0, proposta v1.2 e roadmap na branch `master`, e confirmado HEAD `9405bce...`. Recomendação **APPROVED WITH CHANGES**, ainda **não formalmente aprovada pelo proprietário**.

**Condições recomendadas antes da implementação:**

1. Preflight geométrico dedicado **antes de C1** com perfil exterior + círculo interior → extrusão → face sketch → único cut axial → rebuild com mudanças e persistência; documentar PASS/FAIL e bloquear expansão de scope em caso de falha.
2. Fixar matematicamente oráculos de bounding box/volume (valores da §6) e relações geométricas do fixture, com pre/post-edit.
3. Formalizar em C1 critérios de qualidade, tolerâncias e multibody/determinismo, parser independente e slicer, antes de C2.

**Não confundir:** esta recomendação consultiva com uma aprovação explícita `APPROVED`/`APPROVED WITH CHANGES` lançada em P0 ou no roadmap. Até decisão do proprietário, estado autoritativo da proposta permanece **READY FOR REVIEW — NOT AUTHORIZED FOR IMPLEMENTATION**.

**Próximo prompt Codex indicado:** revisão documental corretiva da proposta + preflight técnico estritamente delimitado, **não executar C1–C4**. Se houver necessidade de alterar ADR/ownership do kernel ou fixture, STOP e retornar para decisão do proprietário. Idealmente não alterar baseline congelada; fixar evidência e preparar plano revisado para aprovação.

---

## 9. Visão futura CAD/CAM — proposta transversal DRAFT (não P0 aprovado)

A discussão paralela especifica uma plataforma original **CAD/CAM mecânica 3D open source** que sintetiza funcionalidades comuns de SOLIDWORKS, CATIA, Autodesk Fusion e FreeCAD. Estudar documentação pública, sem copiar código, design, marcas, ícones, imagens, modelos ou assets dos produtos. Analisar funções observáveis; **não alegar conhecimento das arquiteturas internas proprietárias**.

### 9.1 Benchmark conceptual

| Referência | Padrões observáveis úteis | Não assumir |
|---|---|---|
| SOLIDWORKS | FeatureManager, sketch dimensional, features paramétricas, peças, assemblies, drawings, interação contextual | Internals exatos, artefactos/UI reutilizáveis |
| CATIA | Associatividade paramétrica, workflows especializados, knowledge-based design, produtos/superfícies | Kernel interno e organização técnica não documentada |
| Autodesk Fusion | Timeline, design→manufacture integrado, preview/edição contextual, API/automação | APIs licenciadas como dependência automaticamente aceite |
| FreeCAD | Object/document model paramétrico, workbenches, Python scripting, CAD/CAM modular | Reutilização de recursos/código sem avaliação e autorização |

**Núcleo consensual de produto:** sketch constraints, dimensions/parameters, features, regeneração, geometria/topologia, documentos, selection/picking, visualização, unidades, Undo/Redo, Save/Open, assemblies/desenhos como progressão posterior, formatos de intercâmbio. Recursos avançados (simulação, sheet metal, surfacing sofisticado, FEA, MBD, colaboração cloud, CAM multieixo) não são núcleo universal/MVP.

### 9.2 Arquitectura-alvo conceptual (não implementação autorizada)

- **Parametric feature-history** e grafo de dependências com rebuild determinístico.
- **Sketch solver** separado do kernel 3D.
- **CAD B-rep** exato e topologia coerente para operações mecânicas; **mesh** derivada ou importada, sem confundir com B-rep.
- **Document/transaction/history** versionado, garantia de integridade, IDs persistentes onde viável, diagnóstico de referências quebradas.
- **Rendering/selection/UI** desacoplados da geometria autoritativa.
- **Interop adapters** com perfis de formato, validação e relatório de perdas.
- **Scripting host**/API pública versionada, comandos e features declarativos, isolamento/capabilities, migrações e segurança.
- **Manufacture domain** associativo a revisão CAD, stock, setup, WCS, fixação, ferramenta, operações, toolpath, simulação, postprocessador e artefacto NC.
- **Command model unificado** para extensões e comandos nativos, transações, preview/cancel/undo/validate; scripts não mutam o core arbitrariamente.

**Modelo de domínio aspiracional:** `Project → Document → Part/Assembly/Drawing/ManufacturingPlan`; `Part → Sketches + Bodies + Datums + Parameters + FeatureGraph`; `ManufacturingPlan → CADReference + Setups + Tools + Operations + Toolpaths + Simulation + Postprocessor + NC`. Estes tipos são especificação conceptual; não declarar que já existem no código.

### 9.3 Regras de conceção/licença

- **Regra do utilizador:** não reutilizar fontes, assets ou recursos de produtos CAD existentes; usar fontes públicas apenas para levantamento de factos, comportamentos, normas e padrões genéricos. Implementação/branding/UI próprios.
- Preferência por desenvolvimento interno; bibliotecas open source não entram automaticamente. Exceções através de ADR e análise de licença, redistribuição, segurança e Windows. Evitar packages comerciais/licenças restritivas. Não confundir documentação de um protocolo com licença para copiar a sua implementação.
- Kernel K1/K2/K3 continua aberto; OCCT/CGAL/OpenCAMLib servem como casos de estudo **sem autorização de integração**. Python vs Lua também permanece aberto.
- Uma eventual licença open source do próprio projeto ainda deve ser decidida explicitamente, consultando estado do repositório. A existência de repositório público não equivale à concessão de licença open source.

### 9.4 UX como first-class citizen

**Obrigatório no produto-alvo:** desktop-first, graphics canvas dominante; mouse-first em tarefas normais; seleção direta de faces/arestas/vértices/corpos; hover/prehighlight e feedback; manipuladores 3D; preview antes de commit; escape/cancel seguro; seleção árvore↔canvas; Property/Feature Manager contextuais; comandos descobríveis; modos CAD/Assembly/Drawing/Manufacture coerentes; mensagens em linguagem de engenharia, **sem expor IDs internos**; ícones e design system originais; acessibilidade, escalas desktop e suporte a vários tamanhos. Scripts ampliam, não substituem, GUI.

**Conceito estrutural:**

```text
┌──────────────────────────────────────────────────────────────────────┐
│ File / Edit / View / Workspace / Help                                │
├──────────────────────────────────────────────────────────────────────┤
│ Command groups contextuais: Design | Sketch | Assembly | Manufacture │
├─────────────────────┬────────────────────────────────────────────────┤
│ Feature Tree        │                                                │
│ + contextual        │         Graphics Canvas 3D                     │
│ Property Inspector  │    Pick / Preview / Gizmos / Diagnostics       │
│                     │                                                │
├─────────────────────┴────────────────────────────────────────────────┤
│ Barra de estado | Seleção | Unidade | Operação | Diagnóstico         │
└──────────────────────────────────────────────────────────────────────┘
```

**Experience Gate bloqueia release** mesmo com CTest GREEN. Medir por cenário: sucesso sem interfaces técnicas, clareza de comandos, recuperação de erro, seleção, visualização, entendimento de efeitos de features e integridade do documento.

### 9.5 Matriz de interoperabilidade (visão alvo, sujeita a feasibility/licença)

| Formato | Uso principal | Nota |
|---|---|---|
| `.microsw` | Documento CAD nativo existente | Preservar, versionar e migrar; nunca substituir silenciosamente pelo `.mcad` conceptual da spec v0.1. |
| STL | Exportação 3D printable; potencial futura importação mesh | Malha sem histórico; unidade não robustamente embebida; primeiro alvo real de v1.2 é **exportação**. |
| STEP AP203/AP214/AP242 | Intercâmbio de geometria CAD exata/B-rep e assemblies conforme perfil | Elevado esforço; avaliar futuro kernel/spike; geralmente não transfere história paramétrica completa. |
| 3MF | Fabricação aditiva com unidades/metadados | Posterior; perfil/versionamento de extensões. |
| DXF | Perfis/desenhos 2D para fabrico | Posterior; unidades/entidades cuidadosamente validadas. |
| OBJ | Intercâmbio de mesh | Posterior; malha pode não ser fechada. |
| glTF/GLB | Visualização e cenas 3D | Não equivale a geometria paramétrica precisa. |
| IGES | Legacy CAD curves/surfaces | Posterior; pode exigir healing. |
| G-code/NC | Saída de operações CAM | Dialetos e controladores específicos; segurança e validação humana. |

Workflow de importação alvo: identificação → leitura segura → unidades → validação/healing documentado → preview → confirmar → ImportedBody + origem/perdas. Exportação: perfil/tolerâncias/unidades → validação → escrever transacionalmente → reimportar/verificar → relatório. Não prometer transformar STL em feature history.

### 9.6 Cadeia CAD→CAM desejada

`Modelo CAD (revisão) → Manufacture → Setup/WCS → Stock/Fixture → Tools → Estratégias 2.5D (pocket/contour/drill) → Toolpaths → Preview/simulação de remoção/collision check → Postprocessador por máquina → G-code → preflight/revisão humana → operação física controlada`.

- Primeiro recorte de CAM: **fresagem 2.5D, 3 eixos, offline**; 3D surfacing, turning, multieixo e simulação de máquina apenas depois.
- Associatividade obrigatória: revisão CAD alterada invalida toolpath/NC (**STALE**); não emitir NC como válido sem regeneração/verificação.
- CAM não pode ser considerado seguro só por simulação: offsets, fixações, ferramentas, dry run e procedimentos na máquina devem ser confirmados.
- Tecnologias sem packages comerciais: algoritmos próprios para offsets, pocket/contour, níveis Z, ordem de passes, entradas/rampas, paths; pesquisar abordagens open source sem incorporar código não autorizado.

### 9.7 Scripting e extensibilidade

- Scripting como **mecanismo preferencial para novas extensões**; API estável, independente da linguagem e compatível com modelo de comando/transactions/Undo/Redo.
- Candidatos: **Python embebido vs Lua**, sujeitos a gate formal por distribuição Windows, licença, sandbox, desempenho aceitável e ergonomia.
- Feature scripted define parâmetros, referências/dependências, tipo/versão/schema, preview, execução determinística, outputs, diagnósticos e migrações; suportar Save/Open e Undo/Redo.
- Capabilities explícitas para filesystem/rede/processos, limites de recursos/cancelamento, sem acesso arbitrário à memória interna.
- Scripting **não é pré-requisito de v1.2**, é direção futura proposta.

### 9.8 User stories representativas para aprofundar

- **US-001** sketch→extrude→save exclusivamente por GUI.
- **US-002** alterar dimensão driving e rebuild das features dependentes.
- **US-003** pré-visualizar alteração e cancelar sem dirty/history parcial.
- **US-004** identificar/recuperar face/aresta cuja referência deixou de resolver (futuro persistent naming).
- **US-005** importar STL como malha e exigir decisão explícita sobre unidades (futuro; não v1.2).
- **US-006** exportar STEP e comparar geometria no CAD recetor (futuro).
- **US-007** instalar feature scripted com transação e persistência (futuro).
- **US-008** configurar operação CAM 2.5D, stock/ferramenta/WCS, simular, gerar NC controlado (futuro).
- **US-009** marcar CAM `STALE` após alterar modelo CAD (futuro).
- **US-010** descobrir ações sobre a face via menu contextual (UX transversal).
- **US-011** inserir/mate de duas peças (futuro).
- **US-012** implementar pós-processador de controlador com testes/fixtures NC (futuro).

Template obrigatório para histórias: ID, persona, valor, pré-condições, percurso mouse-first, modelo de domínio, contratos, edge cases, Given/When/Then, testes unit/integration/E2E, Experience Gate, evidência e licenças.

### 9.9 Roadmap **conceptual** de produto CAD/CAM (não substitui roadmap canónico)

- **R0** alinhamento de produto, estudos, domínio, UX/design system, ADRs, benchmark, fontes.
- **R1** fundações CAD; **R2** modelação paramétrica viável com rebuild/features.
- **R3** UX + interoperabilidade base; **R4** scripting/extension API.
- **R5** mecânica avançada: multi-body, datums, fillet/pattern, assemblies/desenhos básicos.
- **R6** cadeia CAM 2.5D com setup/toolpath/simulação/G-code.
- **R7** formatos industriais STEP, DXF, 3MF/glTF conforme capacidade.
- **R8** CAM 3D/turning e validação mais avançada.
- **R9** superfícies/chapa/MBD/ecossistema de extensões.

**Importante:** R0–R9 são épocas de capacidades-alvo, não números de commits, prazos nem autorização de trabalho. O roadmap canónico efetivo (`B0–B14`, MVP, v1.1, P0, v1.2) tem precedência.

### 9.10 Decisões futuras ainda OPEN

- Kernel K1/K2/K3, incluindo custo de B-rep e STEP.
- Persistent topological naming, Boolean geral, sketches com geometrias avançadas.
- Formato nativo futuro e versionamento; compatibilidade obrigatória `.microsw`.
- Política final de licenciamento open source e contribuição de terceiros.
- Scripting Python/Lua, gestão de permissões e migrações.
- Perfis STEP, G-code e primeiros controladores CAM.
- Protótipos UX originais e resultados de testes com utilizadores.
- Inventário de dependências aprovadas/exceções: consultar ADR/política reais.

## 10. Padrão de execução com Codex/agentes

1. **Antes de qualquer tarefa:** confirmar branch, HEAD, worktree, baseline congelada, fontes de verdade e decisão/ADR aplicável.
2. Definir **objetivo verificável, entry criteria, in scope, out of scope**, dependências e plano de evidência.
3. Priorizar **vertical slice executável**; preservar arquitetura e comportamento existente; não ampliar scope silenciosamente.
4. Criar/testar integração com GUI para todas as features do utilizador; não aceitar feature de API isolada como concluída.
5. Executar unit/integration/CTest, build Debug/Release quando aplicável, smoke runtime, casos negativos e Experience Gate com evidência.
6. Documentar **depois de cada commit**; fechar a tarefa com commit; push apenas após indicação/ação do proprietário. Sem repetidas rondas desnecessárias.
7. Gate de arquitetura apenas se material; identificar ADRs impactadas, opções, evidências e decisão solicitada.
8. Se execução requer função não autorizada ou reproduz defeito fora do âmbito: **STOP / solicitar decisão**, não implementar por iniciativa própria.
9. Recolher fontes públicas com URL/data e nunca importar recursos proibidos.
10. Em relatórios, separar **fatos testados, expectativas matemáticas, proposta de decisão, aprovação formal**.

## 11. Fontes públicas de referência já identificadas para a spec v0.1

- SOLIDWORKS: https://www.solidworks.com/product/solidworks-design
- SOLIDWORKS FeatureManager: https://help.solidworks.com/2026/english/SolidWorks/sldworks/c_FeatureManager_Design_Tree_Conventions.htm
- CATIA Knowledge: https://learn.3ds.com/player?id=38320&lpId=225864
- Autodesk Fusion design history: https://help.autodesk.com/view/fusion360/ENU/?contextId=DESIGN_HISTORY
- Autodesk Fusion API: https://help.autodesk.com/cloudhelp/ENU/Fusion-360-API/files/UserManualIndex_UM.htm
- FreeCAD repo/documentação: https://github.com/FreeCAD/FreeCAD ; https://github.com/FreeCAD/FreeCAD-documentation
- STEP AP242: https://www.ap242.org/ap242-standard.html
- 3MF: https://3mf.io/spec/
- glTF: https://www.khronos.org/gltf/
- OpenCAMLib (estudo, não integração): https://github.com/aewallin/opencamlib

Fontes precisam de confirmação por versão/data em investigações posteriores; não equivalem a aprovação de bibliotecas nem a conhecimento de arquitetura proprietária.

## 12. Próximo passo recomendado para o novo chat

**Estado inicial:** HEAD `9405bceb` pushed, P0 FROZEN, v1.2 proposta em review, kernel definitivo open. **Não iniciar implementação**.

**Primeiro trabalho de decisão:**

1. Fazer review da proposta v1.2 contra P0-D01–D07, ADR-0064–0069, política de dependências e FeatureModel/geometry/topology reais.
2. Preparar prompt **V1.2-PLAN-R1 / GEOMETRY PREFLIGHT** estritamente pré-implementação: verificar fixture, expected volume/bounds, documentação de limitações e conformidade sem alargar v1.2.
3. Receber resultado; se PASS, apresentar plano C1–C4 corrigido e pedir aprovação explícita do proprietário; se FAIL, alternativas com impacto e scope para deliberação.
4. Só após essa aprovação formal, gerar prompt para **V1.2-C1**.
5. **Em linha paralela e sem bloquear v1.2**, evoluir DRAFT CAD/CAM v0.1 para `Functional Reference Model`, `UX Reference Model` e `Architecture Decision Package`, depois mapear requisitos por `CAPABILITY_MAP` para backlog futuro.

### Resumo para colar como instrução no primeiro turno do novo chat

> Este chat continua o projeto `farraj1972/micro-solidworks`, branch `master`. O P0.FREEZE foi pushed no commit `9405bcebd49826626da20650e41951dcda2e0e9e`; `v1.1.0` está FROZEN em `774f55c`, 69 ADRs ACCEPTED. A estratégia R2 workflow-first e o primeiro N2 para placa adaptadora com STL validado estão aprovados, mas a proposta v1.2 C1–C4 permanece READY FOR REVIEW e **não** autorizada para implementação. A recomendação de revisão nesta conversa foi consultivamente APPROVED WITH CHANGES (preflight geométrico, volume/bounds analíticos, contrato STL rigoroso), **sem aprovação formal**. K1/K2/K3 continua OPEN. Há ainda uma especificação CAD/CAM 3D open-source v0.1 em DRAFT, com UX mouse-first como first-class, modelo feature-based paramétrico, interoperabilidade STEP/STL/3MF/DXF e cadeia CAD→CAM 2.5D, scripting-first, identidade e código originais. Não confundir visão de longo prazo com v1.2. Verificar documentos canónicos no remoto; preparar o próximo prompt de revisão/preflight, sem iniciar C1–C4 sem aprovação explícita.

---

**Fim do handoff.**
