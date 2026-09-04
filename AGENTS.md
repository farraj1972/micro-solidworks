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

All development must occur through explicitly authorized increments.

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

# 12. Build Requirements

Unless explicitly stated otherwise, validation must include:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

Where applicable, also validate the application executable manually.

Before reporting completion, run:

```bash
git diff --check
```

Expected result:

```text
PASS
```

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

Codex should not commit unless explicitly instructed to do so.

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

Every completed increment must report:

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
B2 — 3D Viewer
STATUS: FROZEN
B2.1–B2.13: COMPLETE
B2.FREEZE: FROZEN
```

Previous stable baselines:

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

Current decision gate:

```text
D3 — Geometric Foundation
STATUS: FROZEN
```

D0, D1 and D2 remain FROZEN. ADR-0013 through ADR-0016 are ACCEPTED.

Next baseline:

```text
B3 — Geometric Primitives
STATUS: NOT STARTED
B3.1–B3.10: PENDING
B3.FREEZE: PENDING
```

B3 implementation may only begin with explicit B3.1 authorization.
Next permitted increment: B3.1 — Point2 / Point3, only after that authorization.

No increment without explicit authorization.
No new Decision Gate without explicit authorization.
No new baseline without explicit authorization.
No baseline freeze without explicit authorization.

The next work item must always be taken from the project owner's explicitly permitted increment.

---

# 32. Core Rule

When uncertain whether a change belongs to the current increment:

> Do less, preserve stability, and report the question rather than silently expanding scope.
