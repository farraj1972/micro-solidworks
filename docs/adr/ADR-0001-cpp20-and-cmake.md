# ADR-0001 — C++20 and CMake

Status: ACCEPTED

## Context

Micro SolidWorks requires a portable language and build foundation suitable
for incremental development, explicit module boundaries and automated tests.

## Decision

Use C++20 as the language standard and CMake as the build system. Use
target-based CMake, disable compiler extensions unless justified, and integrate
tests through CTest. The build must not depend on a specific IDE.

## Rationale

C++20 provides the required modern language baseline. CMake supports the
project's target structure, dependency acquisition and common developer tools
across platforms.

## Consequences

- Builds and dependency configuration can be reproduced from the command line.
- Target boundaries make dependency direction explicit.
- Multiple compilers and IDEs can consume the same build description.
- The project accepts some CMake complexity to preserve this portability.

## Alternatives Considered

- IDE-specific project files: rejected because they would make an IDE part of
  the build contract.
- Older C++ standards: rejected because C++20 is the approved common baseline.
- Other build systems: not selected for the initial foundation.
