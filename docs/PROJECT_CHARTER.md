# Micro SolidWorks — Project Charter

## 1. Visão

Micro SolidWorks é um projecto educacional de desenvolvimento de um sistema CAD 3D simplificado em C++.

O objectivo principal não é competir com soluções CAD comerciais, mas compreender, implementar e experimentar progressivamente os paradigmas fundamentais utilizados em sistemas de modelação 3D e CAD paramétrico.

O projecto deverá produzir uma aplicação real, interactiva e utilizável, cuja capacidade evolui incrementalmente ao longo de baselines estáveis.

---

## 2. Objectivos

### 2.1 Objectivo principal

Construir progressivamente um pequeno sistema CAD 3D capaz de:

- representar geometria;
- visualizar modelos tridimensionais;
- criar e editar sketches;
- aplicar constraints e dimensões;
- gerar sólidos a partir de sketches;
- aplicar features de modelação;
- manter uma feature tree;
- reconstruir modelos parametricamente;
- guardar e carregar documentos.

### 2.2 Objectivo educacional

Cada subsistema deverá ser utilizado como oportunidade para compreender os conceitos subjacentes.

Sempre que razoável:

1. compreender o problema;
2. implementar uma solução simples;
3. testar a solução;
4. identificar limitações;
5. abstrair a fronteira;
6. avaliar alternativas;
7. substituir ou optimizar apenas quando necessário.

---

## 3. Princípios

### P1 — Learning First

O valor pedagógico é um requisito de primeira classe.

Uma biblioteca externa não deverá esconder um conceito fundamental que seja importante compreender através da implementação.

### P2 — Feature over Performance

Durante as primeiras fases:

Correctness > Clarity > Learning > Modularity > Features > Performance.

Optimizações serão introduzidas quando existirem evidências ou requisitos concretos que as justifiquem.

### P3 — Stable Baselines

Cada baseline deverá terminar com:

- código compilável;
- testes executáveis;
- aplicação executável;
- funcionalidades anteriores preservadas;
- funcionalidade da baseline demonstrável.

Nenhuma baseline deverá ser considerada concluída com o repositório instável.

### P4 — Incremental Development

As funcionalidades serão implementadas através de pequenos increments verificáveis.

Evitar alterações extensas que introduzam simultaneamente vários conceitos novos.

### P5 — Modular Core

Os principais subsistemas deverão possuir fronteiras arquitecturais claras.

Implementações poderão ser substituídas posteriormente sem obrigar à reescrita global da aplicação.

### P6 — Explicit Dependencies

Dependências entre módulos deverão ser intencionais e unidireccionais.

O domínio geométrico não deverá depender da UI ou do renderer.

### P7 — Understand Before Abstracting

Não criar abstracções prematuramente.

Primeiro compreender o problema através de uma implementação concreta.

### P8 — Abstract Before Optimizing

Quando uma implementação necessitar de substituição ou optimização, estabelecer primeiro uma fronteira arquitectural adequada.

---

## 4. Estratégia Build vs Buy

Para cada nova capacidade será considerada uma das seguintes estratégias:

### A — Build

Implementação integral no projecto.

Preferida quando a funcionalidade possui elevado valor pedagógico.

### B — Learn then Replace

Criar inicialmente uma implementação didáctica simplificada.

Posteriormente poderá ser substituída por uma implementação mais robusta ou biblioteca externa.

### C — Integrate

Utilizar directamente uma biblioteca externa.

Adequado para infraestrutura ou problemas cujo desenvolvimento próprio tenha baixo valor pedagógico.

A decisão deverá ser explícita.

---

## 5. MVP

O MVP deverá permitir o seguinte workflow:

New Document

→ Select Reference Plane  
→ Create Sketch  
→ Draw basic geometry  
→ Apply dimensions/constraints  
→ Finish Sketch  
→ Extrude  
→ Navigate model in 3D  
→ Select faces/edges  
→ Create additional sketch  
→ Extrude or Cut  
→ Edit parameter  
→ Rebuild model  
→ Save  
→ Close  
→ Reopen document

---

## 6. Fora do scope inicial

Não constituem requisitos do MVP:

- compatibilidade integral com SolidWorks;
- assemblies complexos;
- simulation/FEA;
- CAM;
- photorealistic rendering;
- sheet metal;
- advanced surfacing;
- industrial-grade geometry robustness;
- STEP/IGES interoperability;
- collaborative CAD;
- cloud infrastructure.

Estas capacidades poderão ser consideradas depois do MVP.

---

## 7. Critério de sucesso

O projecto será considerado bem sucedido quando permitir compreender e demonstrar, através de código próprio e de uma aplicação funcional, os principais mecanismos de funcionamento de um pequeno CAD paramétrico 3D.