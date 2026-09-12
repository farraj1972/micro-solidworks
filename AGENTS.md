# Micro SolidWorks — Codex Operating Rules

## 1. Purpose

This repository implements **Micro SolidWorks**, an educational 3D CAD application written in C++.

The project is developed incrementally through stable baselines and small, explicitly scoped increments.

Codex is responsible for implementing approved work while preserving the architectural, educational and stability principles defined by the project.

Codex is not the architectural authority of the project.

---

# 2. Project Priorities

When trade-offs exist, use the following order:

1. Correctness
2. Clarity
3. Learning value
4. Modularity
5. Feature completeness
6. Performance

The project explicitly follows:

> Feature over Performance.

Do not introduce complexity primarily for performance unless the current increment explicitly requires it.

---

# 3. Educational Objective

Micro SolidWorks is not merely an integration project.

A major purpose of the repository is to understand the internal concepts behind:

* 3D mathematics;
* geometry;
* topology;
* rendering;
* tessellation;
* CAD modeling;
* sketches;
* constraints;
* parametric features;
* model rebuild;
* persistence.

Do not automatically replace educational implementations with external libraries.

For each architecturally relevant capability, respect the dependency strategy defined in:

`docs/DEPENDENCY_POLICY.md`

The possible strategies are:

* `BUILD`
* `LEARN_THEN_REPLACE`
* `INTEGRATE`

---

# 4. Architectural Authority

Architecturally significant decisions are controlled by the project owner.

Codex may:

* identify architectural problems;
* analyze alternatives;
* recommend changes;
* propose ADRs;
* provide technical evidence.

Codex MUST NOT autonomously:

* replace an accepted architectural decision;
* change an accepted ADR;
* introduce a conflicting framework or library;
* significantly alter module boundaries;
* change the approved technology stack.

If implementation reveals that an accepted decision is problematic, Codex MUST stop before introducing the architectural deviation and report:

1. the problem;
2. the technical evidence;
3. the affected ADR;
4. the impact;
5. the recommended alternatives;
6. the smallest viable correction.

A new or superseding ADR requires explicit approval.

---

# 5. Decision Gate D0

The following decisions are currently frozen.

## Language

C++20

## Build

CMake

## Testing

GoogleTest + CTest

## Windowing

GLFW

## Rendering

OpenGL

## User Interface

Dear ImGui

## Math

Internal educational implementation.

GLM and Eigen are deferred.

## Geometry

Internal educational implementation.

## Geometry Kernel

Initial kernel is an internal Educational Geometry Kernel.

OpenCascade and CGAL are deferred.

---

# 6. Forbidden Architectural Substitutions

Codex MUST NOT introduce the following without explicit authorization:

* Qt;
* SDL;
* Vulkan;
* DirectX;
* GLM;
* Eigen;
* CGAL;
* OpenCascade;
* alternative CAD kernels;
* alternative UI frameworks;
* alternative rendering engines.

A dependency may not be introduced merely because it makes an implementation easier.

---

# 7. CAD Model vs Rendering Model

This architectural rule is mandatory:

> CAD representation is authoritative. Render representation is derived.

A CAD object must not become equivalent to an OpenGL mesh.

The conceptual pipeline is:

```text
CAD Representation
        |
        v
   Tessellation
        |
        v
   Render Model
        |
        v
     OpenGL
```

Examples of CAD concepts:

* Curve
* Surface
* Face
* Solid

Examples of rendering concepts:

* vertices;
* indices;
* normals;
* triangles;
* lines;
* GPU buffers.

Rendering types MUST NOT leak into the core CAD model.

---

# 8. Dependency Direction

The intended logical dependency direction is:

```text
Application
    |
    +-- UI
    +-- Interaction
    +-- Persistence
            |
            v
         Document
            |
            v
         Modeling
            |
            v
         Topology
            |
            v
         Geometry
            |
            v
           Math
```

Rendering is an infrastructure concern around visualization of derived representations.

The following dependencies are prohibited:

```text
Math       -> UI
Geometry   -> UI
Topology   -> Renderer
Modeling   -> Renderer
Document   -> Dear ImGui
Core       -> OpenGL
Core       -> GLFW
```

---

# 9. Incremental Development

All development must occur through explicitly authorized increments or through
an explicitly approved baseline plan that names and scopes its increment sequence.

Do not implement future roadmap functionality merely because it appears adjacent or convenient.

For every increment:

1. read the requested scope;
2. inspect the existing implementation;
3. identify the smallest coherent change;
4. implement only that change;
5. add or update appropriate tests;
6. run the required validation;
7. report the result.

Scope discipline is mandatory.

An approved baseline plan may authorize its planned increments sequentially;
do not require a separate administrative authorization ritual between them.
Stop for steering when there is a MAJOR/BLOCKER finding, scope expansion, a new
architectural decision, a change to a frozen contract/baseline, an unexpected
external dependency or a destructive/repository-sensitive operation.

## Vertical Slice First

Feature over performance.
Working vertical slice over speculative completeness.
Architecture boundaries over premature abstraction.

Prefer authorized increments with observable functional behavior. Do not finish
subsystems speculatively before they block the next slice. Preserve ADRs and
module boundaries; record canonical gaps/debt and close them when they become
real prerequisites. Canonical roadmap phase != necessarily one historical
implementation baseline. Capabilities may be distributed across authorized
baselines if mapped explicitly, with no dependency inversion or impediment to
future evolution. A canonical gap never automatically reopens a frozen baseline.

---

# 10. No Scope Creep

If an increment is `B0.x`, Codex MUST NOT anticipate functionality belonging to later baselines unless required for the current increment.

Examples currently outside B0:

* Vector2;
* Vector3;
* matrices;
* CAD camera;
* orbit;
* pan;
* zoom;
* 3D grid;
* scene graph;
* geometric primitives;
* topology;
* sketching;
* constraints;
* tessellation engine;
* CAD meshes;
* picking;
* selection;
* persistence;
* feature tree;
* parametric rebuild.

Do not create speculative infrastructure for these capabilities.

---

# 11. Stable Repository Rule

The repository should remain stable after each accepted increment.

An increment is not complete if:

* the project does not configure;
* compilation fails;
* existing tests fail;
* required new tests fail;
* the executable cannot start when startup is part of the increment;
* the implementation leaves knowingly broken intermediate states.

Prefer a smaller complete change over a larger partially completed change.

---

# 12. Impact-Based Validation

Validate according to impact and risk. Do not repeatedly validate unaffected
code without a concrete reason.

```text
Vertical Slice First.
Targeted Validation First.
Full Regression at meaningful integration/baseline gates.
```

The absence of changes in a dependency-independent component is relevant
evidence that its tests need not be repeated. Before testing, determine the
changed modules, changed public contracts, direct consumers, transitive affected
consumers and relevant test suites. Run the smallest test set that gives
meaningful confidence for that changed dependency closure.

## V0 — Documentation / Metadata

Use for non-executable documentation, governance and metadata changes. Review
changed files and documentation/state consistency, then run `git diff --check`
and inspect `git status`. Do not run configure, build, CTest or runtime by
default. Escalate only when documentation participates in executable generation
or configuration, or when a concrete technical reason exists.

## V1 — Targeted Validation

Default for ordinary localized increments. Identify the smallest affected
dependency closure, build affected targets, run directly relevant unit and
contract tests, and run `git diff --check`. Do not run independent historical
suites or perform a clean build by default.

## V2 — Integration Validation

Use when a change crosses layers or completes a vertical slice. Validate the
affected dependency closure with relevant unit and integration tests, plus
runtime validation only for affected observable behavior. V2 does not
automatically imply a full repository regression.

## V3 — Baseline Validation

Reserve V3 for baseline validation/freeze, major dependency or toolchain change,
substantial CMake restructuring, suspected stale-build contamination, or an
explicitly requested full regression. Run clean configure/full build, full
CTest, runtime validation, architecture/dependency audit, repository hygiene
and `git diff --check`.

An incremental build is the default. Do not remove `build/` or repeat
`cmake -S . -B build` when CMake, dependencies and toolchain are unchanged and
the existing configuration is valid. Clean builds require V3 or a concrete
technical reason.

Runtime validation is required when runtime-visible behavior changed,
Viewer/Rendering/UI/App changed materially, cross-layer integration needs it,
or V3 applies. It is not required for docs-only work, an isolated Math value, or
an isolated non-runtime contract covered by sufficient tests.

Validation may escalate V0 -> V1 -> V2 -> V3 only for a concrete reason, such
as wider public API impact, a changed CMake dependency graph, a targeted test
revealing cross-layer regression, or stale/inconsistent incremental output.
Report `Validation escalated from Vx to Vy because: ...`. Do not escalate by
habit. If targeted validation fails, first classify and investigate it as local,
contract, integration or environment failure before broadening the test scope.
An explicit full-regression/V3 request must still be respected.

Do not spend compute, test time, context, or model tokens revalidating unaffected
components without a concrete risk-based reason. Efficiency is part of correct
engineering practice.

---

# 13. Testing Policy

Tests are part of implementation, not a later activity.

Tests should focus on behavior rather than implementation details.

As the project evolves, the following components are expected to have particularly strong automated coverage:

* math;
* geometry;
* topology;
* modeling;
* constraints;
* document rebuild;
* persistence.

Avoid trivial tests written only to increase test counts.

Do not mock pure mathematical or geometric behavior unnecessarily.

For B6 examples: an isolated `Transform3` increment normally uses V1 with the
Math target, Transform3 tests and directly affected transformation/matrix tests;
Viewer/OpenGL/runtime tests are omitted unless dependency analysis requires
them. A transform-aware rendering/picking vertical slice uses V2 across Math,
Presentation, Viewer and Rendering/Picking with relevant integration tests and
runtime behavior. B6 baseline validation uses V3.

---

# 14. Numerical Code

When math and geometry are introduced in later baselines:

* floating-point comparisons must not rely indiscriminately on exact equality;
* tolerance policy must be explicit;
* degenerate cases should be considered;
* invariants should be documented;
* algorithms should prioritize comprehensibility before optimization.

Do not invent a complex global tolerance system before the roadmap authorizes it.

---

# 15. External Dependencies

Every new external dependency must be justified.

Before introducing one, evaluate:

1. why it is required;
2. whether the current increment genuinely needs it;
3. whether implementing the concept internally has educational value;
4. licensing;
5. maintenance status;
6. architecture impact;
7. replacement cost.

For B0, the admissible dependency categories are limited to:

* GLFW;
* OpenGL loader;
* Dear ImGui;
* GoogleTest;
* lightweight logging library.

Anything else requires explicit authorization.

---

# 16. Dependency Isolation

External libraries should be kept behind appropriate boundaries when practical.

Examples:

```text
GLFW      -> platform/application boundary
OpenGL    -> rendering
Dear ImGui -> UI
Logger implementation -> logging adapter
```

Do not propagate external-library-specific types throughout unrelated modules.

---

# 17. Logging

Logging should be accessed through a project-owned boundary.

Application code should preferably use concepts such as:

```cpp
Logger::info(...);
Logger::warn(...);
Logger::error(...);
```

rather than directly coupling all modules to a logging library.

Do not implement sophisticated asynchronous or distributed logging unless explicitly requested.

---

# 18. Architecture Decision Records

ADRs are stored under:

```text
docs/adr/
```

Each ADR should contain:

* Context
* Decision
* Rationale
* Consequences
* Alternatives
* Status

Approved ADRs use:

```text
Status: ACCEPTED
```

Proposals created for discussion use:

```text
Status: PROPOSED
```

Codex MUST NOT mark a self-proposed architectural decision as `ACCEPTED` without explicit authorization.

Decision Gates are reserved for material architectural choices: module/layer
boundaries, ownership, identity, coordinate/math conventions, persistence
contracts, dependency direction, difficult-to-reverse semantics and changes to
frozen architecture. ADRs are reserved for decisions that are architecturally
significant, long-lived, cross-cutting or expensive to reverse. Do not create a
Decision Gate or ADR for each class, helper, test or reversible implementation
detail.

When an existing decision proposal is explicitly approved, that approval
authorizes its documentary completion: PROPOSED -> ACCEPTED, Decision Gate ->
FROZEN, documentation synchronization and the authorized commit. If executable
HEAD is unchanged, this normally uses V0 and does not repeat full regression.
A documentary Decision Gate alone does not justify V3.

---

# 19. Existing D0 ADRs

The initial D0 ADRs are:

```text
ADR-0001-cpp20-and-cmake.md
ADR-0002-glfw-windowing.md
ADR-0003-opengl-rendering.md
ADR-0004-dear-imgui-ui.md
ADR-0005-internal-math-library.md
ADR-0006-educational-geometry-kernel.md
```

These decisions have already been approved.

When materialized, their status is:

```text
ACCEPTED
```

Codex may refine wording but not alter their substance.

---

# 20. Code Quality

Prefer:

* small classes;
* explicit ownership;
* RAII;
* deterministic resource cleanup;
* const-correctness;
* clear namespaces;
* focused modules;
* descriptive naming;
* minimal hidden global state.

Avoid:

* unnecessary inheritance;
* premature generic frameworks;
* excessive templates without clear value;
* speculative abstractions;
* large manager classes;
* global mutable state;
* macros where language features are sufficient;
* raw resource ownership when RAII can express ownership safely.

---

# 21. Abstraction Policy

Do not create an interface for every class.

Introduce abstractions where there is a real boundary or plausible replacement point.

Likely future architectural boundaries include:

* renderer;
* tessellator;
* geometry kernel;
* boolean engine;
* constraint solver;
* persistence serializer;
* selection acceleration.

Follow:

> Understand before abstracting.

and:

> Abstract before optimizing.

---

# 22. Error Handling

Do not silently swallow initialization or runtime failures.

Failures that prevent application startup should:

* be logged;
* produce a meaningful error;
* terminate cleanly.

Avoid broad catch-all handling that hides defects.

---

# 23. Resource Management

C++ resources must follow RAII where appropriate.

Examples include:

* GLFW lifetime;
* windows;
* OpenGL resources;
* UI contexts;
* files;
* future CAD resources.

Creation and destruction order must be explicit when dependencies exist.

---

# 24. Documentation

Documentation must remain synchronized with architecturally meaningful implementation changes.

Do not rewrite large documentation sections for cosmetic reasons during a small implementation increment.

Update only documentation affected by the current scope.

---

# 25. Comments

Comments should explain:

* why;
* invariants;
* mathematical reasoning;
* unusual constraints;
* non-obvious ownership;
* architectural decisions.

Do not add comments that merely restate obvious code.

---

# 26. Formatting and Repository Hygiene

Do not modify unrelated files.

Do not perform mass formatting unless explicitly requested.

Do not introduce generated artifacts into source control unless the repository intentionally tracks them.

Build directories must remain outside tracked source.

Typical examples:

```text
build/
out/
cmake-build-*/
```

should not be committed.

---

# 27. Git Discipline

Codex should not commit unless the prompt or an approved plan authorizes it. A
PASS increment may be committed under that authorization without a separate
permission cycle. Tags remain reserved for baseline/release freezes.

When reporting a completed increment, propose a commit message.

Recommended format:

```text
[Bx.y] Short imperative description
```

Example:

```text
[B0.4] Add GLFW application window
```

For baseline closure:

```text
[B0] Establish Micro SolidWorks foundation
```

---

# 28. Increment Completion Report

Reports must be proportional. A normal increment reports Result, Files,
Behavior, Validation, architecture deviations/findings and Commit. Do not repeat
the full project history. Reserve extensive reports for Decision Gates, V3
baseline validation, baseline freeze and major findings.

V1/V2 reports include:

```text
Validation level: V1 | V2
Changed dependency closure: ...
Tests run: ...
Tests intentionally not run: ... — reason
Build: affected targets PASS | FAIL
Escalation: none | reason
```

V0 reports include:

```text
Validation level: V0
Files reviewed: ...
git diff --check: PASS | FAIL
Executable validation: intentionally not run — no executable impact
```

The established detailed completion shape remains available when scope or risk
requires it:

```text
Increment:
Result: PASS | FIX REQUIRED | BLOCKED

Files created:
- ...

Files changed:
- ...

Files removed:
- ...

Implementation:
- ...

Tests:
- ...

Build:
- ...

Runtime validation:
- ...

Architecture:
- No deviation
```

If there is a deviation or issue:

```text
Architecture:
- Deviation detected
- Affected ADR: ADR-XXXX
- Authorization required before proceeding
```

Also provide:

```text
Recommended commit message:
`[...] ...`

Next permitted increment:
`Bx.y ...`
```

---

# 29. Failure Reporting

If an increment cannot be completed correctly, do not hide the failure.

Use:

```text
Result: FIX REQUIRED
```

or:

```text
Result: BLOCKED
```

Describe:

* what failed;
* evidence;
* root cause if known;
* affected files;
* tests/build status;
* minimum recommended correction.

A failing test that reveals an existing defect is a valid and useful result.

Do not modify architecture outside the authorized scope merely to make tests green.

---

# 30. Baseline Freeze

A baseline may only be considered frozen after all required increments have passed.

The normal lifecycle is small V1 increments, V2 vertical integration increments,
V3 baseline validation, then baseline freeze. If V3 passed and only documentary
state changes occur before freeze, do not repeat V3: use V0, confirm the
validated executable HEAD is unchanged, then commit/tag as authorized. If the
executable HEAD changed, V3 is required again.

For B0, baseline validation requires at minimum:

```text
[PASS] Repository structure established
[PASS] C++20 enforced
[PASS] CMake configure
[PASS] CMake build
[PASS] GoogleTest integrated
[PASS] CTest passes
[PASS] Logging operational
[PASS] GLFW window operational
[PASS] OpenGL initialized
[PASS] Dear ImGui operational
[PASS] Application shell visible
[PASS] Clean shutdown
[PASS] D0 ADRs materialized
[PASS] Foundational documentation present
[PASS] No unauthorized CAD functionality
[PASS] git diff --check
```

Only then may the project declare:

```text
BASELINE B0 — FOUNDATION
STATUS: FROZEN
```

---

# 31. Current Development Boundary

Current latest stable baseline:

```text
B6 — Transformations
STATUS: FROZEN
B6.1–B6.10: COMPLETE
B6.FREEZE: FROZEN
```

Current baseline candidate:

```text
B7 — Topological Model
STATUS: IMPLEMENTED / BASELINE CANDIDATE
B7.1–B7.10: COMPLETE
```

Previous stable baseline:

```text
B4 — Geometry Visualization & Selection
STATUS: FROZEN
B4.1–B4.12: COMPLETE
B4.FREEZE: FROZEN
```

Previous stable baseline:

```text
B3 — Geometric Primitives
STATUS: FROZEN
B3.1–B3.9: COMPLETE
B3.10A: COMPLETE
B3.10: COMPLETE
B3.FREEZE: FROZEN
```

Previous stable baselines:

```text
B2 — 3D Viewer
STATUS: FROZEN
B2.1–B2.13: COMPLETE
B2.FREEZE: FROZEN
```

```text
B1 — Mathematical Foundation
STATUS: FROZEN
B1.1–B1.10: COMPLETE
B1.FREEZE: FROZEN
```

```text
B0 — Foundation
STATUS: FROZEN
```

Latest decision gate:

```text
D6 — Topology Ownership, Identity & Orientation
STATUS: FROZEN
```

D0, D1, D2 and D3 remain FROZEN. ADR-0001 through ADR-0020 are ACCEPTED.
ADR-0021 through ADR-0024 are ACCEPTED. D5 — Transformation Semantics is
FROZEN. The complete inventory is 27/27 ADRs ACCEPTED. D6 — Topology Ownership,
Identity & Orientation is FROZEN in ADR-0025–0027.

B3.10A corrected closest-point reconstruction robustness identified by the
first B3.10 validation. The repeated B3.10 validated 549/549 tests and runtime
smoke with no findings. B3 is FROZEN.

Frozen B4 validation:

```text
B4 — Geometry Visualization & Selection
STATUS: FROZEN
B4.1–B4.12: COMPLETE
B4.FREEZE: FROZEN
```

B4.12 passed clean configure/Debug build and 684/684 tests, with zero project,
dependency or linker warnings. Runtime passed; native X and File -> Exit
returned 0. HiDPI automated validation passed; manual B4.12 HiDPI was unavailable.
The B4.12 MINOR documentary state finding is CLOSED by B4.FREEZE.
ADR-0017–0020 remain ACCEPTED / CONFORMANT; ADR-0001–0024 are 24/24 ACCEPTED.

B4 is FROZEN. Changes to frozen B4 behavior require explicit authorization.
Canonical functional progress (see docs/ROADMAP.md): B0/B1/B2 satisfied;
B3/B4 partially satisfied; B5 REALIZED / SATISFIED BY technical B4. No duplicate
B5 implementation is required. Technical B0–B4 remain FROZEN.
B6 Transformations is FROZEN after B6.10: clean configure/full Debug build,
708/708 CTest tests, manual runtime PASS and normal exit code 0. Build warnings: 0.
ADR-0021–0024 are CONFORMANT. Manual HiDPI was not explicitly confirmed;
automated HiDPI coverage passed. See docs/B6_VALIDATION.md.
Entity transforms close GAP-SCENE-001. Circle/intersections and visibility/lifecycle
remain deferred. The B7 prerequisite review is complete: these gaps do not block
the first cuboid slice. Accepted ADR-0025–0027 freeze D6 ownership, identity,
orientation, geometry association and manifold semantics. B7.1–B7.10 implement
the conformant Topology vertical slice; final V3 and freeze remain. Frozen B6
changes require explicit authorization.

No increment without explicit authorization.
No new Decision Gate without explicit authorization.
No new baseline without explicit authorization.
No baseline freeze without explicit authorization.

The next work item must always be taken from the project owner's explicitly permitted increment.

---

# 32. Core Rule

When uncertain whether a change belongs to the current increment:

> Do less, preserve stability, and report the question rather than silently expanding scope.

Repository governance and accepted ADRs are authoritative. Future prompts
should reference them instead of restating them. Prefer compact prompts covering
increment, scope, expected behavior, affected layers, validation level,
acceptance criteria, stop conditions and commit policy.
