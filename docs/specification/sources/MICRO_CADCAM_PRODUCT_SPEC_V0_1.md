# MICRO CAD/CAM — Especificação funcional, arquitetura de referência, UX e roadmap

**Versão:** 0.1 — Proposta para revisão · **Data:** 2026-10-19 · **Estado:** DRAFT / não substitui ADRs aprovadas

## 0. Mandato e regras inegociáveis

Criar uma aplicação **CAD/CAM 3D open source**, desktop-first, centrada em modelação paramétrica por features, suficientemente extensível para suportar CAD mecânico, documentação técnica, interoperabilidade e produção CNC. O produto deve ser simples de operar com rato, potente sem expor abstrações informáticas aos utilizadores e dotado de automação por scripts como principal via de extensão.

**P0:** (G-01) conceção/implementação original; **não copiar/reutilizar código, interfaces, imagens, ícones, desenhos, ficheiros, modelos, assets, marcas, macros nem algoritmos transcritos de produtos analisados**; documentação pública apenas como referência factual; (G-02) dependências de terceiros não são automaticamente admitidas, mesmo que open source; (G-03) motores CAD/CAM e UI desacoplados; (G-04) UX tem critérios de aceitação próprios e poder de bloquear release; (G-05) todas as alterações de features preservam transações, histórico e integridade do documento; (G-06) todo o CAM emitido necessita simulação, validação, preview e confirmação humana; (G-07) cada entrega gera aplicação executável com regressão e Experience Gate; (G-08) decisões anteriores só mudam mediante ADR explícita.

### Política de licenciamento

**Distinção essencial:** estudar um produto não autoriza usar os seus recursos. A preferência é por **implementação própria**, incluindo geometry kernel educacional, comandos, widgets e design system. Bibliotecas auxiliares open source só podem ser propostas em ADR de exceção quando forem indispensáveis, com inventário de licenças, obrigações de distribuição, segurança e alternativa sem dependência. Licenças MIT/BSD/Apache-2.0 tendem a ser mais simples; LGPL e GPL implicam análise específica e **não são autorizadas por omissão**. Nada na presente especificação obriga a usar OCCT, FreeCAD, OpenCAMLib, Qt ou outro componente. Normas publicamente documentadas são insumo para interoperabilidade, sujeitas às condições efetivamente aplicáveis aos textos normativos.

## 1. Benchmark funcional e modelos de produto

| Dimensão | SOLIDWORKS | CATIA | Autodesk Fusion | FreeCAD | Decisão MICRO |
|---|---|---|---|---|---|
| Modelo de conceção | Feature-based parametric, árvore de construção | Paramétrico associativo, knowledge-based design, ambientes especializados | Paramétrico com timeline e alternativa de edição direta | Paramétrico por objetos/documentos e workbenches | Feature-based parametric como modo normal |
| Organização | Peças, montagens, desenhos; árvore e PropertyManager | Peças, produtos, corpos, superfícies; workbenches e regras | Design + Manufacture; browser e timeline | Document objects, workbenches e propriedades | Documento único com tipos Part, Assembly, Drawing, Manufacturing |
| Sketch 2D | Restrições geométricas/dimensionais | Sketcher parametrizado | Sketch constraints e parâmetros | Sketcher com constraints | Solver 2D incremental e UX gráfica |
| Sólidos | Extrude, revolve, sweep, loft, boolean, fillet, patterns | Part Design + superfícies avançadas | Sólidos, superfícies, mesh, sheet metal | PartDesign/Part, sólidos e booleans | Implementação incremental por complexidade |
| Dependências | Árvore e rebuild | Relações, formulas, conhecimento | Timeline/recompute | DAG e recompute | DAG com dependências explícitas, ordem determinística |
| Extensão | API/macros/ecossistema | Automation/knowledge rules | Scripts e add-ins; APIs para Manufacture | Python e extensões | API versionada e scripting como first-class |
| CAM | Oferta conforme edição/ecossistema | Soluções de manufatura dentro do ecossistema | Manufacture integrado | CAM workbench | Workspace Manufacture associativo, etapas progressivas |
| Filosofia visual | Tree + manipuladores + edição contextual | Workbenches de engenharia e estrutura de produto | Contextual canvas/timeline | Workbenches e painel de propriedades | Identidade gráfica própria; canvas-first |

**Nota de rigor:** a tabela caracteriza modelos funcionais observáveis, **não** afirma conhecer as arquiteturas internas proprietárias, os respetivos kernels completos nem a implementação exata de seus motores. A arquitetura de microserviços, frameworks internos e mecanismos de persistência dos produtos comerciais não se inferem da UI ou da documentação de produto.

### 1.1 Funcionalidade transversal consolidada

**Fundação:** documentos, unidades, referenciais, seleção, navigation orbit/pan/zoom, histórico Undo/Redo, Save/Open, propriedades, visibilidade, medição, organização da árvore, estados de erro/rebuild.

**CAD comum:** sketches 2D com restrições; planos/eixos; entidades 2D; parametrização por variáveis/equações; modelação sólida por features; transformações; booleans; operações de acabamento; padrões e espelhos; multi-body; referências entre features; montagens e mates; desenhos técnicos 2D; exportação neutra.

**Mais avançado/não universal:** surfacing complexo, chapa, generative design, simulação FEA, MBD/PMI, PDM cloud, CAM multieixo, generative machining, colaboração síncrona. Devem constar no mapa de oportunidades, mas **não** ser confundidos com o núcleo consensual ou MVP.

### 1.2 Arquiteturas de referência — classes de solução

1. **Feature-history CAD:** histórico ordenado de operações com parâmetros, referências e regeneração; preferido para intenção de projeto.
2. **Constraint-based sketch:** sistema de equações e graus de liberdade com solver; separado do kernel 3D.
3. **B-rep solid modeler:** topologia vértice/aresta/face/casca/sólido + curvas/superfícies geométricas; representação alvo para CAD mecânico de precisão.
4. **Mesh modeler:** vértices/faces trianguladas; ótimo para visualização, impressão e importação, mas não equivalente a B-rep editável.
5. **Direct-edit layer:** operações de alteração topológica sem recuperar história; posterior ao core paramétrico.
6. **Associative manufacturing:** setup, stock, ferramenta e operação referenciam uma revisão da geometria, revalidando percursos quando o CAD muda.
7. **Plug-in/script host:** comandos e features definidos através de API estável com lifecycle, acesso ao documento e regras de transação.

## 2. Domínio e arquitectura proposta

### 2.1 Modelo conceptual

`Project` → `Document` → (`Part` | `Assembly` | `Drawing` | `ManufacturingPlan`).

`Part` → `Body[]`, `Sketch[]`, `Datum[]`, `FeatureGraph`, `ParameterTable`.

`Feature` → `FeatureId`, `TypeId`, `Parameters`, `References`, `Dependencies`, `EvaluationStatus`, `OutputShapeIds`, `SchemaVersion`, `Provenance`.

`Sketch` → `Entities`, `Constraints`, `SolverState` (FullyDefined/UnderDefined/OverDefined/Conflicting).

`Assembly` → `ComponentInstance[]` + `MateConstraint[]`.

`ManufacturingPlan` → `CADReference`, `Setup[]`, `Stock`, `Fixtures`, `ToolLibrary`, `Operation[]`, `Toolpath[]`, `SimulationReport`, `Postprocessor`, `NCArtifact`.

**IDs imutáveis:** entidades não devem ser referenciadas apenas pelo índice da face resultante. Definir um serviço de identificação persistente de topologia (*topological naming*), com estratégia de correspondência e diagnóstico explícito de referências ambíguas após rebuild. Qualquer solução inicial limitada deve declarar claramente esse limite.

### 2.2 Componentes e fronteiras

- `math-core`: vetores, matrizes, tolerâncias, unidades, cálculos determinísticos;
- `geometry-core`: entidades geométricas, curvas, superfícies e classificação;
- `topology-core`: B-rep, validade manifold, regras de orientação e robustez;
- `constraint-solver`: sketch 2D, DOFs, conflitos e diagnóstico;
- `parametric-engine`: DAG, avaliação incremental, cache, versionamento, erros e transações;
- `document-engine`: serialização versionada, migrações, Undo/Redo, integridade, recovery;
- `selection-and-scene`: picking, highlights, camera, overlays, manipuladores;
- `ui-shell` e `design-system`: workflows, comandos, painéis contextuais, acessibilidade e temas;
- `interop`: import/export por adaptadores e relatório de perdas;
- `scripting-runtime`: scripting isolado, API declarativa, extensões, segurança e testes;
- `cam-core`: stock, setups, ferramentas, estratégias, percursos, verificação;
- `postprocessing`: perfis de controlador, dialetos NC, testes e safety gates;
- `diagnostics`: auditoria de operações, geração de evidências, telemetria local opt-in.

**Política de extensibilidade:** UI invoca um `CommandBus` público; comandos nativos e scripts partilham contrato (`canExecute`, `preview`, `execute`, `undo`, `validate`) e estados. Uma feature scripted deve declarar esquema de parâmetros, dependências, output geométrico, erros, versão e migração; sem acesso irrestrito à memória do motor.

### 2.3 Invariantes

- Rebuild determinístico para o mesmo documento/versão/ambiente; atualização incremental por dependências.
- Separação matemática exata/geométrica versus triangulação de renderização.
- Sem silent failures: falha preserva a última geometria válida onde seguro e assinala **FAILED/BLOCKED**.
- Save/Open/Undo/Redo conservam IDs, parâmetros, links CAD-CAM e integridade.
- Scripts e extensões nunca gravam diretamente estado mutável fora da transação.
- Qualquer CAM calculado sobre revisão obsoleta fica **STALE** e não pode ser pós-processado como válido sem atualização e verificação.

## 3. Requisitos funcionais priorizados

| ID | Prioridade | Requisito | Aceitação essencial |
|---|---|---|---|
| CAD-001 | P0 | Criar/abrir/guardar documento nativo versionado | Round-trip sem perda de parâmetros e referências |
| CAD-002 | P0 | Gestão de sketches e restrições | Feedback visual de DOFs, conflitos e under-defined |
| CAD-003 | P0 | Extrude, Cut, boolean, edit, rename, rebuild | Transações atómicas; Undo/Redo consistente |
| CAD-004 | P0 | Parâmetros nomeados e expressões com unidades | Edição propaga a dependentes com diagnóstico |
| CAD-005 | P0 | Seleção associada entre árvore e viewport | Realce consistente sem estado divergente |
| CAD-006 | P1 | Revolve, mirror, patterns, fillet, chamfer, holes | Parâmetros persistentes e erros compreensíveis |
| CAD-007 | P1 | Multi-body, datums, referências topológicas persistentes | Rebuild identifica e informa quebras |
| CAD-008 | P1 | Montagens, instâncias e mates básicos | Movimentos/DOFs e dependências coerentes |
| CAD-009 | P2 | Desenhos 2D associativos e cotas | Alteração 3D atualiza vistas |
| CAD-010 | P2 | Superfícies, sweep, loft, chapa e famílias configuráveis | Implementação por capacidade e testes |
| INT-001 | P0 | STL import/export de mesh | Unidades explícitas, orientação, validade, tolerância |
| INT-002 | P1 | STEP import/export de B-rep | Via implementação própria ou ADR aprovada; validar fidelidade |
| INT-003 | P1 | OBJ e 3MF por adaptadores próprios | Geometria, unidades e materiais conforme suporte declarado |
| INT-004 | P2 | DXF (2D), glTF/GLB (publicação) e IGES (legado) | Matriz declarada de capacidades e perdas |
| INT-005 | P0 | Preview/import report | Diferenças geométricas, unidades, atributos não suportados |
| SCR-001 | P0 | Console/script runner com API de leitura | Sem acesso oculto às estruturas internas |
| SCR-002 | P1 | Scripts criam comandos paramétricos | Undo/rebuild/save/open/testes equivalentes a comandos nativos |
| SCR-003 | P1 | Sandboxing/capabilities, quotas e consentimento | Scripts não confiáveis bloqueados por defeito |
| CAM-001 | P1 | ManufacturingPlan associativo ao CAD | Mudança de geometria sinaliza percurso inválido |
| CAM-002 | P1 | Setup, WCS, stock, fixtures e ferramentas | Preview visual e validação dimensional |
| CAM-003 | P1 | 2.5D facing, pocketing, contour, drilling | Percursos determinísticos e simuláveis |
| CAM-004 | P1 | Simulação material removido e colisões iniciais | Avisos críticos impedem exportação acidental |
| CAM-005 | P1 | Pós-processadores versionados para G-code | Header, unidades, coordenadas, segurança e testes golden |
| CAM-006 | P2 | 3D finishing, rest machining, 4/5 eixos, turning | Fases autónomas com safety gate dedicado |
| UX-001 | P0 | Manipulação com rato e comandos contextuais | Fluxos primários sem scripting nem console |
| UX-002 | P0 | Preview contínuo com Confirm/Cancel | Cancel não altera modelo nem árvore |
| UX-003 | P0 | Diagnóstico contextual acionável | Erro indica entidade, causa e recuperação |
| UX-004 | P0 | Experience Gate por tarefa/feature | Testes desktop, hi-DPI, teclado e rato |
| UX-005 | P1 | Discoverability, pesquisa de comandos, onboarding | Utilizador encontra operação sem conhecer estrutura técnica |

## 4. Conceito UI/UX (first-class citizen)

### 4.1 Princípios de interação

**Canvas-first, direct manipulation, progressive disclosure, proximity of controls, reversibility, visual continuity, immediate feedback, predictable selection, consistent command grammar e error recovery.** A UI deve revelar apenas propriedades relevantes para o contexto; facilitar operações por rato; continuar totalmente acessível por teclado, sem obrigar o utilizador a navegar por APIs ou estruturas técnicas. Não copiar organização visual concreta ou identidade de qualquer concorrente.

### 4.2 Information architecture

- **Header discreto:** documento, save/status, Undo/Redo, comandos globais, pesquisa.
- **Workspace switcher:** Design / Assembly / Drawing / Manufacture (somente quando funcional).
- **Explorer esquerdo:** elementos do documento, features, referências, corpos e erros, com seleção sincronizada.
- **Canvas central dominante:** grelha/triad, view cube original, gizmos, dimensões editáveis no local, selection preview.
- **Inspector contextual direito ou popover próximo do cursor:** parâmetros da operação, mensagens de validação, assistentes de seleção.
- **Status bottom:** modo, unidades, estado de rebuild, seleção, ajuda curta; timeline apenas quando melhorar a compreensão.
- **Command palette e pie/context menu:** comandos dependentes de seleção; comandos frequentes a 1–2 ações.

### 4.3 Mouse interaction contract

- Clique seleciona; Ctrl/Shift adiciona/remove conforme padrão consistente; clique vazio limpa; duplo clique edita.
- Roda zoom ancorado no cursor; pan com botão configurável; orbit por arrasto com botão configurável; fit por comando único.
- Hover mostra pré-seleção; hit testing discrimina face/edge/vertex/body; seleção ambígua abre lista de candidatos.
- Comando lançado após seleção mantém seleção; comando iniciado sem seleção guia o utilizador para escolher entidades.
- Drag handle altera dimensão com preview; valor exato pode ser digitado no mesmo contexto, sem diálogo bloqueante.
- Escape cancela pré-visualização, nunca executa; Confirm é explícito para transformações destrutivas.
- Undo/Redo têm feedback imediato; badges apenas para estados críticos, sem poluição visual.

### 4.4 Design system original

Tokens semânticos para spacing, tipografia, camadas, densidade, contraste, seleção, preselection, error, warning e sucesso. Ícones vetoriais **criados de raiz** com grelha, espessura e cantos consistentes; suporte dark/light, hi-DPI, escalabilidade, tooltips e motion reduzido. Para desktop: painel de propriedades responsivo, modo compacto e layouts persistidos por utilizador. Sem dependência de assets comerciais nem clonagem gráfica.

### 4.5 UX metrics e gates

- 100% dos fluxos P0 executáveis via GUI/rato.
- 100% dos comandos com status visível, preview ou indicador de execução e reversibilidade definida.
- Nenhuma seleção ambígua executa ação irreversível automaticamente.
- Testes de primeira utilização por cenário com recolha de sucesso, erro, número de ações e tempo (metas quantitativas após baseline empírica).
- Experience Gate: criação, edição, cancel, erros, recuperação, import, CAM, display 1920×1080 e 390×844 apenas quando existir alvo mobile; no desktop validar também 125/150/200% escala e navegação teclado.

## 5. Interoperabilidade: perfis e limitações

| Formato | Papel | Representação | Meta / fase | Advertências |
|---|---|---|---|---|
| Nativo MICRO (`.mcad`, nome provisório) | Fonte autoritativa de design | DAG/params/topologia/versões | P0 | Schema próprio e migrações; publicar especificação |
| STL | Mesh de fabricação/partilha | Triângulos | P0 | Sem histórico CAD; STL não define unidade robustamente |
| STEP AP203/AP214/AP242 | Transferência industrial de geometria precisa | B-rep, assemblies e metadados conforme perfil | P1+ | Nem todo export conserva histórico de features; AP242 é vasto |
| OBJ | Mesh de intercâmbio | Faces/atributos | P1 | Pode ter malhas abertas |
| 3MF | Fabricação aditiva | Mesh + unidades/metadados | P1 | Suportar subconjunto explícito de versão/extensões |
| DXF | Sketch/desenho 2D | Curvas e entidades | P2 | Cobertura por entidades; vigiar unidades |
| glTF/GLB | Preview/publicação 3D | Mesh, cenas e materiais | P2 | Não é CAD B-rep |
| IGES | Geometria legacy | Curvas/superfícies | P2 | Topologia e healing podem ser frágeis |
| G-code/NC | Output máquina | Movimentos e funções | CAM P1 | Dialeto, ferramenta, WCS, M-codes específicos |

**Import sequence:** deteção → preflight segurança/tamanho → unidade → leitura em modelo intermédio → validação/healing não destrutivo → preview comparativo → confirmação → criação de `ImportedBody` com proveniência → relatório. **Export sequence:** escolher formato/perfil/tolerância/unidades → verificar suporte → gerar → reimportar para round-trip de conformidade → relatório de perdas. Não prometer conversão de STL em sólido paramétrico sem reconstrução, nem recuperabilidade automática da história de STEP.

## 6. CAD → CAM: contrato funcional

**Fluxo de utilizador:** selecionar corpo/revisão CAD → abrir Manufacture → criar Setup → definir orientação e zero peça (WCS) → definir stock, fixação e ferramenta → escolher estratégia → parâmetros de corte → gerar percurso → visualizar trajetória e simular remoção/collision check → comparar stock residual → escolher pós-processador → preflight NC → exportar programa → revisão humana externa/na máquina.

**Primeiro âmbito CAM:** fresagem 2.5D em 3 eixos e G-code para controladores selecionados em modo offline. Maquinagem 3D de superfícies, turning, 4/5 eixos, sondagem, compensações complexas, cinemática e simulação completa de máquina são incrementos posteriores. Adaptadores de LinuxCNC ou controladores ISO-like só após fixture de testes e revisão de segurança. A simulação digital não garante segurança real: confirmação física de setups, offsets, ferramenta, fixação, dry run e procedimentos do fabricante continuam necessários.

**Algoritmos próprios na fase 2.5D:** identificação de pockets/contornos planar, offsets planares, níveis Z, ordenação de segmentos, rampas/entradas, ligações e passes, clipping em stock; pós-processamento com AST/IR de movimentos e perfis de controlador. Pesquisa sobre OpenCAMLib pode orientar a taxonomia de operações; **não incorporar código sem exceção formal à política G-01/G-02**.

## 7. Scripting-first extensibility

Linguagem proposta para avaliação: Python embebido e isolado, ou Lua com runtime pequeno; **D2-SCRIPT-GATE** decidirá a tecnologia por licenciamento, segurança, portabilidade Windows, ergonomia e custo de embedding. `Script API` independente da linguagem; schemas declarativos serializáveis, versionados e documentados. Principais interfaces: `Document`, `Sketch`, `Feature`, `Body`, `Selection`, `Parameters`, `Transaction`, `CommandRegistry`, `Preview`, `CamJob`, `Toolpath` (CAM após maturidade). Eventos após commit; recompute sem efeitos laterais; permissão explícita para ficheiros/network/processos; quota de execução e cancelamento; bundles de extensões com manifesto, assinatura opcional, testes e compatibilidade. **Scripts não são um substituto da experiência gráfica; scripts ampliam a funcionalidade, enquanto a GUI continua a superfície primária.**

## 8. User stories e testes de aceitação (semente para agentes)

**US-001 — Criar peça por rato (P0).** Como utilizador iniciante quero criar um sketch num plano, desenhar retângulo, cotar, extrudir e guardar, sem tocar numa interface técnica. *Given* novo documento, *when* concluo Sketch→Extrude→Save, *then* o modelo mostra sólido editável e árvore correta; reabrir preserva relações; Undo/Redo funciona.

**US-002 — Alteração paramétrica (P0).** Como projetista quero editar uma cota de sketch e obter atualização da extrusão dependente. *Then*: geometria e medição atualizam; em conflito a UI assinala FAILED/BLOCKED sem corrupção.

**US-003 — Pré-visualização segura (P0).** Como utilizador quero arrastar profundidade da extrusão, visualizar resultado e cancelar. *Then*: Cancel preserva documento, Undo stack e dirty flag originais.

**US-004 — Reparar referências (P1).** Como projetista quero saber qual aresta/face perdeu associação após modificar uma feature anterior. *Then*: diagnóstico identifica feature e candidatos; não faz rebinding silencioso.

**US-005 — Importar STL (P0).** Como projetista quero importar uma malha para inspecionar e medir. *Then*: sistema identifica mesh, unidade desconhecida a confirmar, geometria não paramétrica e relatório de qualidade.

**US-006 — Trocar STEP (P1).** Como projetista quero exportar peça e reimportar num CAD diferente. *Then*: volume, bounding box, unidades e orientação respeitam tolerâncias publicadas; não se promete manter árvore paramétrica.

**US-007 — Extensão por script (P1).** Como contributor quero instalar feature personalizada com inputs e preview e que funcione como feature nativa. *Then*: parâmetros sobrevivem a Save/Open, há transação, Undo, diagnóstico e testes.

**US-008 — CAM 2.5D (P1).** Como operador CNC quero selecionar face, criar stock, operação pocket, ferramenta e G-code pós-processado. *Then*: percurso, WCS, offset, simulação e bloqueios de segurança são auditáveis.

**US-009 — Associatividade CAD/CAM (P1).** Como operador quero que uma alteração de diâmetro na peça invalide a operação CAM anterior. *Then*: `STALE`, regeneração requerida, sem export NC como válido antes de revalidar.

**US-010 — Descoberta por contexto (P0).** Como principiante quero selecionar uma face e encontrar operações relevantes no menu contextual. *Then*: opções aplicáveis aparecem; comandos indisponíveis justificam porquê; sem formulários técnicos irrelevantes.

**US-011 — Montagem (P1).** Como projetista quero inserir duas instâncias e relacioná-las por mate. *Then*: transformações ficam determinadas dentro dos DOFs e persistem em documento.

**US-012 — Pós-processador de controlador (P1).** Como contributor quero adicionar perfil NC usando extensão testável. *Then*: produção de output validada por fixtures/golden files; perfis desconhecidos não geram ficheiros não verificados por defeito.

**Formato obrigatório de cada história futura:** `ID`, ator, objetivo, valor, precondições, fluxo mouse-first, entidades de domínio, contratos API, casos limite, critérios Given/When/Then, testes unit/integration/E2E, Experience Gate, evidência e riscos/licença.

## 9. Roadmap de produto (independente das baselines B0/B1/v1.1 da implementação)

| Época | Resultado verificável | Gate |
|---|---|---|
| R0 — Alignment | Requisitos, domínio, design system, modelo de arquitetura, ADRs, benchmark, jornadas e protótipo de UX | Product/UX/License Gate |
| R1 — CAD foundation | Documentos, seleção, sketches, primitives, feature graph, transações, Save/Open, Undo/Redo | Native CAD slice + Experience Gate |
| R2 — CAD parametric viable | Constraint solver, parâmetros e expressões, rebuild incremental, extrude/cut/boolean/revolve, diagnósticos | Parametric Integrity Gate |
| R3 — Usabilidade e interoperabilidade | UX avançada, manipuladores, tree/inspector, import/export STL, primeiros formats neutros; STEP feasibility spike | Interop Gate + User Test |
| R4 — Extensibilidade | API versionada, scripts e feature plugins, capabilities/sandbox, migrações | Extension/Threat Model Gate |
| R5 — CAD mecânico generalista | multi-body, datums, fillet/chamfer/pattern, assemblies básicas, desenho 2D inicial | Mechanical Design Gate |
| R6 — CAM 2.5D vertical slice | Setup/stock/tools/face/pocket/contour/drill, preview, simulação básica, G-code seguro | Manufacturing Safety Gate |
| R7 — Interoperabilidade industrial | STEP B-rep e assemblies por perfis incrementais, 3MF, glTF, DXF, validação round-trip | Conformance Gate |
| R8 — CAM evoluído | 3D surfacing/rest machining, verificação avançada, perfis de controladores, turning em fase própria | Machining Validation Gate |
| R9 — Especialização e comunidade | superfícies complexas, sheet metal, MBD, bibliotecas e marketplace de extensões original | Community/UX Gate |

**Mapeamento para implementação existente:** não reiniciar a aplicação nem reimplementar o já validado. Criar `CAPABILITY-MAP.md`: cada requisito → funcionalidades já entregues (v1.1.0 / 903 testes), parcial, gap, dependência, ADR e próxima baseline. Planeamento das novas tarefas deve partir deste gap, sem quebrar a baseline congelada.

### Critérios transversais de conclusão de cada incremento

1. Executável Debug instalável/iniciável e sem regressões.
2. Testes unit/integration, replay determinístico, Save/Open e Undo/Redo quando aplicáveis.
3. Experience Gate real por rato, incluindo erros e Cancel, com aceite explícito.
4. Documentação, ADRs e inventário de licenças revistos **depois de cada commit**.
5. Commit ao fecho, evidência de `git diff --check`, build, CTest; push sob controle do responsável.
6. Uma feature só conta como concluída quando integrada no fluxo do utilizador, não apenas numa API.

## 10. Gates e decisões ainda não aprovadas

- **D-PRODUCT-01:** abrangência CAD mecânico generalista vs CAD/CAM industrial orientado 3 eixos; proposta: mecânico generalista com CAM progressivo.
- **D-ARCH-01:** formato nativo e versionamento do grafo.
- **D-GEOM-01:** crescimento do kernel educacional versus exceção futura para kernel open source. Não substituir sem decisão explícita.
- **D-INTEROP-01:** implementação própria de STEP e âmbito de AP242; risco técnico elevado.
- **D-SCRIPT-01:** Python vs Lua e modelo de sandbox/capabilities.
- **D-LICENSE-01:** licença do próprio projeto, compatibilidade de dependências e política de contribuição.
- **D-CAM-01:** pós-processadores iniciais, WCS e conjunto inicial de máquinas-controladores.
- **D-UX-01:** protótipo interativo testado, guidelines de mouse mappings e acessibilidade.

## 11. Pacote para agentes de desenvolvimento

Manter os seguintes artefactos textuais fonte de verdade: `PRODUCT_VISION.md`, `FUNCTIONAL_SPEC.md`, `DOMAIN_MODEL.md`, `ARCHITECTURE.md`, `UX_CONCEPT.md`, `INTEROPERABILITY_MATRIX.md`, `CAD_CAM_CHAIN.md`, `SCRIPTING_EXTENSIONS.md`, `USER_STORIES.md`, `ROADMAP.md`, `ADRS/`, `DEPENDENCY_POLICY.md`, `CAPABILITY_MAP.md`, `TEST_AND_EXPERIENCE_GATES.md` e `SOURCES.md`.

**Agente investigador:** reúne fontes públicas e regista URL, data, evidência e limites; nunca transfere assets/código. **Agente de produto:** normaliza features e critérios. **Agente UX:** protótipo original e testes de tarefa. **Agente arquiteto:** define contratos e ADR. **Agente implementação:** uma slice executável. **Agente verificação:** testes, licenças, Experience Gate e relatórios de risco. Agentes devem sinalizar conflitos com decisões frozen, não alterá-las silenciosamente.

**Template de prompt para cada baseline:** `Contexto e baseline exata → objetivo verificável → in scope/out of scope → requisitos e user stories → contratos e invariantes → UI/UX mouse-first → exceções de dependências proibidas por defeito → plano de testes → Experience Gate → atualização pós-commit da documentação → critérios STOP/GO → evidência e relatório de handoff`.

## 12. Fontes públicas verificadas (consulta de 2026-10-19)

- SOLIDWORKS — capabilities: https://www.solidworks.com/product/solidworks-design
- SOLIDWORKS — FeatureManager: https://help.solidworks.com/2026/english/SolidWorks/sldworks/c_FeatureManager_Design_Tree_Conventions.htm
- CATIA Knowledge Fundamentals: https://learn.3ds.com/player?id=38320&lpId=225864
- Autodesk Fusion — Modeling modes: https://help.autodesk.com/view/fusion360/ENU/?contextId=DESIGN_HISTORY
- Autodesk Fusion — API: https://help.autodesk.com/cloudhelp/ENU/Fusion-360-API/files/UserManualIndex_UM.htm
- Autodesk Fusion — imported history limitations: https://www.autodesk.com/support/technical/article/caas/sfdcarticles/sfdcarticles/Imported-files-do-not-contain-timeline-features-in-Fusion-360.html
- FreeCAD — official repository: https://github.com/FreeCAD/FreeCAD
- FreeCAD — scripting: https://github.com/FreeCAD/FreeCAD-documentation/blob/main/wiki/Getting_started.md
- STEP AP242 working group: https://www.ap242.org/ap242-standard.html
- 3MF specification: https://3mf.io/spec/
- Khronos glTF: https://www.khronos.org/gltf/
- OpenCAMLib — algorithms and license (reference only): https://github.com/aewallin/opencamlib

**Lacunas de investigação prioritárias:** documentação CATIA mais detalhada por plataforma/versão; normas STEP implementáveis legalmente e testes públicos de conformidade; licenciamento de eventual biblioteca de parsing; referências CAM/LinuxCNC/postprocessors; pesquisa UX com utilizadores; análise de *topological naming* e robustez numérica. A investigação factual não dispensa diligência jurídica de licenças nem validação técnica.
