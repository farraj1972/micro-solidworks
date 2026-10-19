# CAD → CAM Reference Chain — DRAFT / DEFERRED

Source: [specification v0.1 §6](sources/MICRO_CADCAM_PRODUCT_SPEC_V0_1.md) and [informative handoff §9.6](sources/MICRO_SOLIDWORKS_HANDOFF_ESTRATEGIA_CADCAM_P0_V12_2026-10-19.md). **No CAM implementation or NC production is authorized.** The first conceptual slice is offline, three-axis 2.5D milling, subject to future owner approval and safety review.

```text
CAD Model + revision identity
  → Manufacturing Workspace
  → Setup / work coordinate system (WCS)
  → Stock / fixtures
  → Tool and cutting parameters
  → 2.5D operation (face, pocket, contour, drill)
  → deterministic toolpath
  → preview / stock-removal and collision checks
  → controller-specific postprocessor
  → NC preflight and artifact
  → human confirmation and physical machine procedure
```

**Proposed associativity invariant:** a ManufacturingPlan references a particular CAD revision/fingerprint, not a live mutable mesh. Any relevant CAD edit marks affected derived toolpaths, simulations and NC artifacts `STALE`; regeneration and validation must precede treating them as current. A stale flag must not silently rebind to changed topology. This would require an identity/version contract beyond current [`FeatureModel`](../../src/features/FeatureModel.h) and [`Persistence`](../../src/persistence/Persistence.h), so its ownership and persistence are future Decision Gate topics.

Conceptual entities: `ManufacturingPlan`, `CADReference`, `Setup`, `WCS`, `Stock`, `Fixture`, `ToolLibrary`, `MachiningOperation`, `Toolpath`, `SimulationReport`, `PostprocessorProfile`, `NCArtifact`. None is an existing code aggregate. Early 2.5D geometry would require supported planar pocket/contour recognition, offsets, Z levels, entry/exit and linking moves, stock clipping, deterministic operation order and explicit unsupported-case diagnostics. Neither the current axis-aligned Boolean nor Cut Extrude is a toolpath engine.

Simulation is a diagnostic model, **not proof of machine safety**. Any future NC workflow must identify controller dialect, tools, units, offsets, stock/fixation assumptions and validation fixtures; retain a human review/dry-run/physical setup step. No G-code may be labelled safe merely because a virtual simulation passed. Multi-axis, turning, full machine kinematics and CAM-006 are later horizons, not part of the first conceptual slice or v1.2.
