# Micro SolidWorks — Bootstrap Guide

## 1. Purpose

This document defines how Codex must initialize and start development of the **Micro SolidWorks** repository.

The project is currently at:

```text
Decision Gate D0: FROZEN
Baseline B0: FROZEN
Decision Gate D1: FROZEN
Current baseline: B1 — Mathematical Foundation
B1 status: IN PROGRESS
B1.1–B1.8: COMPLETE
Current increment: B1.9 — Documentation & D1 ADR Validation
B1.10 — Baseline Validation: next pending quality gate; requires explicit authorization
B1.FREEZE: PENDING
```

B1 implementation work requires explicit increment authorization.

---

## 2. Required Reading Order

Before changing the repository, read the following documents in this order:

```text
1. AGENTS.md
2. docs/PROJECT_CHARTER.md
3. docs/ARCHITECTURE.md
4. docs/DEPENDENCY_POLICY.md
5. docs/ROADMAP.md
6. docs/adr/ADR-0001-cpp20-and-cmake.md
7. docs/adr/ADR-0002-glfw-windowing.md
8. docs/adr/ADR-0003-opengl-rendering.md
9. docs/adr/ADR-0004-dear-imgui-ui.md
10. docs/adr/ADR-0005-internal-math-library.md
11. docs/adr/ADR-0006-educational-geometry-kernel.md
12. docs/adr/ADR-0007-mathematical-scalar-and-tolerance.md
13. docs/adr/ADR-0008-coordinate-and-unit-conventions.md
14. docs/adr/ADR-0009-matrix-and-transform-conventions.md
```

If some of these documents are not yet present, they belong to the initial repository materialization and must be created from the approved project context before implementation proceeds.

Do not reinterpret or replace the approved decisions contained in them.

---

## 3. Frozen D0 Technology Decisions

The following choices are already approved:

```text
Language:           C++20
Build system:       CMake
Testing:            GoogleTest + CTest
Windowing:          GLFW
Rendering:          OpenGL
UI:                 Dear ImGui

Math:               Internal implementation
Geometry:           Internal implementation
Topology:           Internal implementation
Initial CAD kernel: Educational Geometry Kernel

GLM:                Deferred
Eigen:              Deferred
CGAL:               Deferred
OpenCascade:        Deferred
Qt:                 Not authorized
SDL:                Not authorized
Vulkan:             Not authorized
```

These decisions must not be changed during B0.

---

## 4. B0 Goal

Baseline B0 establishes a stable executable application foundation.

At the end of B0, the repository should provide:

```text
C++20 project
CMake build
CTest integration
GoogleTest
Logging
GLFW window
OpenGL context
Dear ImGui
Minimal application shell
D0 ADRs
Stable build/test/run workflow
```

B0 does not contain CAD functionality.

---

## 5. B0 Increments

The approved sequence is:

```text
B0.1 — Repository & Build Foundation
B0.2 — Test Infrastructure
B0.3 — Logging
B0.4 — GLFW Application Window
B0.5 — OpenGL Bootstrap
B0.6 — Dear ImGui Integration
B0.7 — Application Shell
B0.8 — D0 Documentation & ADRs
B0.9 — Baseline Validation
B0.FREEZE
```

Only one increment should be implemented at a time.

---

# 6. Initial Increment

## B0.1 — Repository & Build Foundation

### Objective

Create the smallest coherent C++20 repository structure that:

* configures successfully with CMake;
* compiles successfully;
* provides an executable entry point;
* establishes the approved module boundaries without speculative implementation.

---

## 7. Required Repository Structure

Target structure:

```text
micro-solidworks/
├── CMakeLists.txt
├── README.md
├── AGENTS.md
├── BOOTSTRAP.md
├── .gitignore
├── cmake/
├── docs/
│   ├── PROJECT_CHARTER.md
│   ├── ARCHITECTURE.md
│   ├── DEPENDENCY_POLICY.md
│   ├── ROADMAP.md
│   └── adr/
├── src/
│   ├── app/
│   ├── core/
│   ├── rendering/
│   ├── interaction/
│   ├── ui/
│   └── persistence/
├── tests/
└── examples/
```

Do not create placeholder classes merely to populate directories.

Empty directories do not need to be tracked unless there is a concrete reason to do so.

---

## 8. B0.1 Implementation Requirements

### 8.1 Root CMake

Create a root `CMakeLists.txt` that:

* defines the project;
* requires an appropriate minimum CMake version;
* explicitly enables C++20;
* disables compiler extensions unless justified;
* creates the initial application target;
* keeps room for later modularization without implementing future modules prematurely.

The build must not depend on IDE-specific configuration.

---

### 8.2 Application Entry Point

Create the minimum application entry point.

Example conceptual scope:

```cpp
int main()
{
    return 0;
}
```

A small project-owned application bootstrap class is acceptable only if it adds genuine structural value to B0.1.

Do not introduce GLFW, OpenGL or Dear ImGui yet.

Those belong to later increments.

---

### 8.3 Naming

Use a consistent namespace rooted in the project.

Recommended namespace:

```cpp
microsw
```

or another concise equivalent already approved within the repository.

Do not use `solidworks` as a namespace that could imply compatibility or ownership by Dassault Systèmes.

The executable may still be presented to the user as:

```text
Micro SolidWorks
```

while internal technical naming remains independent.

---

### 8.4 Build Output

The application target should have one stable technical target name.

Recommended examples:

```text
micro_solidworks
```

or:

```text
microsw
```

Choose one and use it consistently.

Avoid multiple aliases in B0.1 without a concrete need.

---

### 8.5 `.gitignore`

At minimum exclude common generated build output:

```text
build/
out/
cmake-build-*/
```

Also exclude IDE-local or operating-system-specific files only where appropriate.

Do not add an excessively broad `.gitignore` copied from unrelated projects.

---

### 8.6 README

Create a concise `README.md` containing:

* project purpose;
* educational nature;
* current baseline;
* technology foundation;
* minimal build commands;
* reference to `AGENTS.md` and `docs/`.

Do not document functionality that does not yet exist.

---

## 9. CMake Quality Requirements

Prefer target-based CMake.

Avoid global compiler configuration when target-specific configuration is appropriate.

Preferred style:

```cmake
target_compile_features(...)
target_include_directories(...)
target_compile_options(...)
```

Avoid legacy global patterns such as unnecessary use of:

```cmake
include_directories(...)
add_definitions(...)
```

Do not add dependency managers or `FetchContent` in B0.1.

External dependencies begin only when required by their respective increments.

---

## 10. Compiler Warnings

Reasonable compiler warnings may be enabled.

They should:

* improve code quality;
* remain portable across the supported compiler set;
* not require suppressing large numbers of warnings.

Do not enable `-Werror` or equivalent globally during B0.1 unless explicitly authorized.

A warning should not automatically make the repository unusable across compilers.

---

## 11. Platform Scope

The initial development environment may be Windows, but repository structure and CMake must not unnecessarily hard-code Windows-specific assumptions.

Do not introduce platform abstraction frameworks during B0.1.

Platform-specific code should only appear when actually needed.

---

## 12. Forbidden Work in B0.1

Do not implement:

```text
GoogleTest integration
CTest tests
logging
GLFW
OpenGL
OpenGL loader
Dear ImGui
camera
rendering loop
viewport
grid
math library
Vector classes
Matrix classes
geometry
topology
scene graph
selection
picking
persistence
feature tree
CAD document model
```

Even if some of these are easy to add.

They belong to later increments.

---

## 13. Validation

After implementation, run from a clean build directory:

```bash
cmake -S . -B build
cmake --build build
```

Both must pass.

Also run:

```bash
git diff --check
```

Expected result:

```text
PASS
```

If the executable can be launched without side effects, optionally validate:

```text
micro_solidworks
```

Expected behavior for B0.1 is simply clean process execution and termination.

---

## 14. Completion Criteria

B0.1 is complete only when all are true:

```text
[PASS] Root project structure established
[PASS] CMake project defined
[PASS] C++20 explicitly required
[PASS] Initial executable target exists
[PASS] Application entry point exists
[PASS] No unauthorized external dependencies
[PASS] Configure succeeds
[PASS] Build succeeds
[PASS] Repository hygiene acceptable
[PASS] git diff --check passes
[PASS] No future B0 functionality implemented prematurely
```

---

## 15. Required Completion Report

When B0.1 is finished, report exactly in this spirit:

```text
B0.1 Result: PASS

Files created:
- ...

Files changed:
- ...

Files removed:
- ...

Implementation:
- C++20 project initialized.
- Root CMake configuration established.
- Initial executable target created.
- No external runtime dependencies introduced.

Tests:
- Not applicable in B0.1.
- Test infrastructure belongs to B0.2.

Build:
- cmake -S . -B build: PASS
- cmake --build build: PASS
- git diff --check: PASS

Runtime validation:
- Executable launches and exits cleanly, if applicable.

Architecture:
- No deviation from D0.
- No future baseline functionality introduced.

Recommended commit message:
`[B0.1] Establish repository and CMake foundation`

Next permitted increment:
`B0.2 — Test Infrastructure`
```

If any requirement cannot be satisfied, use:

```text
B0.1 Result: FIX REQUIRED
```

or:

```text
B0.1 Result: BLOCKED
```

and provide evidence.

---

# 16. Stop Condition

After completing B0.1:

> STOP.

Do not begin B0.2 automatically.

Wait for explicit authorization.

---

# 17. Governing Principle

For this project:

> A small stable increment is more valuable than a larger speculative implementation.

Preserve the learning value, architecture and stability of the repository at every step.
