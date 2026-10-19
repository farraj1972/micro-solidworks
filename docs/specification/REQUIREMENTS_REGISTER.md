# Requirements Register — Source v0.1, DRAFT / PROPOSED

The **29 parent IDs and their original descriptions, acceptance intent and priorities below are preserved verbatim** from [source §3](sources/MICRO_CADCAM_PRODUCT_SPEC_V0_1.md). SPEC-01 traceability is APPROVED / FROZEN; the source and all 29 aggregate requirements remain `DRAFT / PROPOSED` unless independently approved. All have `Source = MICRO_CADCAM_PRODUCT_SPEC_V0_1.md §3`. `Original Priority` is source metadata, **not** approved roadmap priority. The per-ID implementation/validation/scope and open-question fields follow in the second table; architecture, code, dependencies, workflow and evidence fields are keyed by the same ID in [traceability](REQUIREMENTS_TRACEABILITY.md). Together these tables form each requirement record. Evidence absence is `NOT VERIFIED`, never inferred implementation.

| Requirement ID | Original Priority | Original Description | Original Acceptance Intent |
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

## Current assessment per parent ID

`Implementation Status` uses the [capability-map vocabulary](CAPABILITY_MAP.md). `Validation Status` describes the *bounded existing implementation*; it never validates the full proposed requirement unless explicitly said. `Current Scope` is the effective roadmap placement, separate from Original Priority. `Existing` means no new work is authorized by this register. `Deferred` means outside approved v1.2. `Campaign-level` means product target approved but implementation plan pending.

| ID | Implementation Status | Validation Status | Current Scope / Effective Priority | Target capability / workflow | Open question |
|---|---|---|---|---|---|
| CAD-001 | IMPLEMENTED_AND_VALIDATED (bounded native model) | v1.1.0 release and persistence tests | Existing; preserve | Save/Open parametric part | Future multi-document schema? |
| CAD-002 | PARTIALLY_IMPLEMENTED | Sketch/solver tests; full DOF UX NOT VERIFIED | Existing subset; improvements deferred | Constrained sketch | How to expose full DOF/conflict guidance? |
| CAD-003 | PARTIALLY_IMPLEMENTED | Feature/modeling tests and C5 gate for supported subset | Existing subset; general ops deferred | Edit/rebuild plate | General Boolean/stacked cuts need future scope? |
| CAD-004 | NOT_IMPLEMENTED | NOT VERIFIED | Deferred | Named design expressions | Units/expression grammar and ownership? |
| CAD-005 | PARTIALLY_IMPLEMENTED | Viewer/editor/UI tests and C5 gate, bounded | Existing subset | Tree↔viewport selection | General persistent face identity? |
| CAD-006 | NOT_IMPLEMENTED | NOT VERIFIED | Deferred | Broader mechanical part modeling | Kernel and operation order? |
| CAD-007 | PARTIALLY_IMPLEMENTED | Multi-Solid/semantic-face tests, bounded | Existing subset; naming deferred | Multi-body reference | General naming and datum ownership? |
| CAD-008 | NOT_IMPLEMENTED | NOT VERIFIED | Deferred | Assembly | Part instance/mate identity? |
| CAD-009 | NOT_IMPLEMENTED | NOT VERIFIED | Deferred | Technical drawing | Drawing document/revision model? |
| CAD-010 | NOT_IMPLEMENTED | NOT VERIFIED | Deferred | Advanced geometry | Kernel scope/robustness? |
| INT-001 | FOUNDATIONAL_ONLY | Derived-display tests; STL I/O NOT VERIFIED | `EXP`: v1.2 campaign-level priority; `IMP`: deferred | Print handoff / mesh import | Export tessellation, units, body policy? |
| INT-002 | NOT_IMPLEMENTED | NOT VERIFIED | Deferred | Exact CAD exchange | K1/K2/K3 and STEP profile? |
| INT-003 | NOT_IMPLEMENTED | NOT VERIFIED | Deferred | Mesh/additive exchange | Profiles/units/material losses? |
| INT-004 | NOT_IMPLEMENTED | NOT VERIFIED | Deferred | 2D cut / publication / legacy | Which workflow justifies each? |
| INT-005 | NOT_IMPLEMENTED | NOT VERIFIED | Deferred | Safe import preview/report | Import authority and healing? |
| SCR-001 | NOT_IMPLEMENTED | NOT VERIFIED | Deferred despite source P0 | Read-only script API | Python/Lua and threat model? |
| SCR-002 | NOT_IMPLEMENTED | NOT VERIFIED | Deferred | Scripted parametric command | Versioned transaction/schema? |
| SCR-003 | NOT_IMPLEMENTED | NOT VERIFIED | Deferred | Extension security | Enforceable sandbox on Windows? |
| CAM-001 | NOT_IMPLEMENTED | NOT VERIFIED | Deferred | Associative ManufacturingPlan | CAD revision/fingerprint and STALE? |
| CAM-002 | NOT_IMPLEMENTED | NOT VERIFIED | Deferred | Manufacturing setup | WCS/stock/tool ownership and units? |
| CAM-003 | NOT_IMPLEMENTED | NOT VERIFIED | Deferred | Offline 2.5D milling | Geometry/offset robustness and machines? |
| CAM-004 | NOT_IMPLEMENTED | NOT VERIFIED | Deferred | Removal/collision simulation | Validation limits and safety? |
| CAM-005 | NOT_IMPLEMENTED | NOT VERIFIED | Deferred | Controller NC output | First controller/dialect and human gate? |
| CAM-006 | NOT_IMPLEMENTED | NOT VERIFIED | Deferred | Advanced CAM | Machine kinematics/verification? |
| UX-001 | IMPLEMENTED_AND_VALIDATED (bounded CAD workflows) | C5 Experience Gate and interaction tests | Existing; future CAM UX deferred | Mouse-first part editing | Broader task discoverability? |
| UX-002 | PARTIALLY_IMPLEMENTED | C5 edit/Cancel gate; universal preview NOT VERIFIED | Existing subset | Safe feature editing | Shared draft model for new commands? |
| UX-003 | PARTIALLY_IMPLEMENTED | C5 rebuild diagnostics; all operations NOT VERIFIED | Existing subset | Error recovery | Entity-level actionable errors? |
| UX-004 | PARTIALLY_IMPLEMENTED | C5 gate; all-task/HiDPI contract NOT VERIFIED | Required in future visible campaigns | Experience validation | Repeatable accessibility matrix? |
| UX-005 | PARTIALLY_IMPLEMENTED | C5 user gate; search/onboarding NOT VERIFIED | Deferred | First-use CAD discovery | Which tasks fail user study? |

## Stable child IDs for compound requirements

Child IDs refine, **never renumber or replace**, their original parent. Their acceptance intent inherits the parent and needs a separate future refinement before implementation. All remain DRAFT / PROPOSED even where an existing bounded capability is validated. The [traceability table](REQUIREMENTS_TRACEABILITY.md) maps parents; child mapping is below and in [capability map](CAPABILITY_MAP.md).

| Parent | Stable children and split | Effective status |
|---|---|---|
| CAD-002 | `CAD-002-SKETCH` sketch entities; `CAD-002-CONSTRAINT` geometric/dimensional constraints | Both partially implemented. |
| CAD-003 | `CAD-003-EXT` additive extrude; `CAD-003-CUT` axial cut; `CAD-003-BOOL` Boolean; `CAD-003-EDIT` edit; `CAD-003-RENAME` rename; `CAD-003-REBUILD` rebuild | Existing bounded; broader semantics deferred. |
| CAD-006 | `CAD-006-REV` revolve; `CAD-006-MIR` mirror; `CAD-006-PAT` pattern; `CAD-006-FIL` fillet; `CAD-006-CHA` chamfer; `CAD-006-HOL` hole feature | All deferred; no Hole Wizard approved. |
| CAD-007 | `CAD-007-MB` multi-body; `CAD-007-DAT` datums; `CAD-007-TOP` persistent topology references | MB bounded; DAT foundational principal planes; TOP deferred. |
| CAD-008 | `CAD-008-ASM` assembly document; `CAD-008-INST` component instances; `CAD-008-MATE` mates | All deferred. |
| CAD-009 | `CAD-009-VIEW` associated drawing views; `CAD-009-DIM` drawing dimensions | Both deferred. |
| CAD-010 | `CAD-010-SUR` surfaces; `CAD-010-SWP` sweep; `CAD-010-LOF` loft; `CAD-010-SHT` sheet metal; `CAD-010-FAM` families | All deferred. |
| INT-001 | `INT-001-EXP` STL export; `INT-001-IMP` STL import | EXP approved product priority / v1.2 plan pending; IMP deferred. |
| INT-002 | `INT-002-EXP` STEP export; `INT-002-IMP` STEP import | Both deferred. |
| INT-003 | `INT-003-OBJ` OBJ; `INT-003-3MF` 3MF | Both deferred. |
| INT-004 | `INT-004-DXF` DXF; `INT-004-GLTF` glTF/GLB; `INT-004-IGES` IGES | All deferred. |
| CAM-002 | `CAM-002-SET` setup; `CAM-002-WCS` coordinates; `CAM-002-STK` stock; `CAM-002-FIX` fixtures; `CAM-002-TOOL` tools | All deferred. |
| CAM-003 | `CAM-003-FACE` facing; `CAM-003-POCK` pocketing; `CAM-003-CONT` contour; `CAM-003-DRILL` drilling | All deferred. |
| CAM-004 | `CAM-004-REMOVE` stock-removal simulation; `CAM-004-COLLISION` collision checks | Both deferred. |
| CAM-006 | `CAM-006-3D` finishing; `CAM-006-REST` rest machining; `CAM-006-MULTI` 4/5 axes; `CAM-006-TURN` turning | All deferred. |
| UX-005 | `UX-005-CONTEXT` contextual discovery; `UX-005-SEARCH` command search; `UX-005-ONBOARD` onboarding | CONTEXT bounded; SEARCH/ONBOARD deferred. |

No child is a new approved increment. Future revisions may add child acceptance details without changing the parent ID or its original wording.
