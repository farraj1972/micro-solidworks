# V1.1-C5 — Feature Workflow Hardening Prerequisite Review

## Result

```text
STATUS: READY FOR ROADMAP DECISION
Recommended option: A — UX Hardening Only
Decision Gate: NONE
Implementation: NOT STARTED
```

V1.1-C5 should be the final functional campaign of v1.1. Its purpose is to
make the existing Feature Manager a coherent editing and repair surface. It
does not add modeling capability or change the explicit rebuild, ordered
history, dependency or persistence contracts frozen by D11, D12, D13 and D20.

## Repository evidence

`FeatureModel` already owns typed Sketch, Extrude, Cut Extrude and Boolean
records, stable FeatureIds, deterministic display names, backward-only
dependencies, CLEAN/DIRTY/FAILED/BLOCKED_BY_DEPENDENCY state, diagnostics,
transactional explicit rebuild, deletion closure and checkpoints. Persistence
v2 stores feature names and definitions and rebuilds derived state on Open.
Application-owned checkpoints provide atomic Undo/Redo.

The derived Feature Manager already absorbs a primary Sketch below Extrude or
Cut Extrude, preserves unconsumed Sketches at the root, selects by logical
identity, synchronizes with the viewport, exposes Show/Hide and Delete, and
shows compact state cues. Principal planes and generated support descriptions
are human-readable.

The remaining problems are concentrated in interaction consistency and
diagnosis:

- double-click edits a Sketch directly but only opens properties for other
  feature kinds; Extrude and Cut Extrude require a second `Edit parameters`
  action;
- Extrude and Cut Extrude may commit when an input loses focus, before the
  explicit Confirm action, so Cancel is not a uniform discard operation;
- Boolean is inspect-only in the Feature Manager and its normal creation/edit
  workflow is not productized;
- the global rebuild failure message exposes a raw FeatureId, while blocked
  features do not name the failed upstream feature;
- context actions are partly type-specific in the tree, but the Graphics Area
  menu offers generic actions that can be invalid for the selected kind;
- status cues are ASCII tokens and color, rather than distinct type/state icons
  with accessible labels and useful tooltips;
- rename and keyboard rename are absent;
- dependency and provenance text exists for Sketch and Cut Extrude, but is not
  presented consistently for every supported kind;
- Feature Manager expansion is session-local ImGui state and is not explicitly
  stabilized across ordinary edits or rename.

## Current feature workflow audit

The classifications describe the complete user workflow. A domain operation
may be complete while its normal UI remains rough or blocking.

| Capability | SketchFeature | ExtrudeFeature | CutExtrudeFeature | BooleanFeature |
|---|---|---|---|---|
| Create | COMPLETE | COMPLETE | COMPLETE | BLOCKING UX — no coherent normal creation workflow |
| Select | COMPLETE | COMPLETE | COMPLETE | COMPLETE |
| Edit | USABLE BUT ROUGH — direct editing exists, but no session-wide discard | BLOCKING UX — double-click is indirect and focus loss may commit | BLOCKING UX — same Confirm/Cancel inconsistency | INCOMPLETE — inspect-only UI despite domain operation editing support |
| Delete | COMPLETE | COMPLETE | COMPLETE | COMPLETE |
| Visibility | COMPLETE | COMPLETE | COMPLETE | COMPLETE |
| Failure state | USABLE BUT ROUGH | USABLE BUT ROUGH | USABLE BUT ROUGH | USABLE BUT ROUGH |
| Rebuild | COMPLETE | COMPLETE | COMPLETE | COMPLETE |
| Undo/Redo | COMPLETE for committed edits/delete | COMPLETE | COMPLETE | COMPLETE for existing domain transactions |
| Persistence | COMPLETE | COMPLETE | COMPLETE in v2 | COMPLETE |
| Context menu | USABLE BUT ROUGH | USABLE BUT ROUGH | USABLE BUT ROUGH | INCOMPLETE — Inspect/Show/Hide/Delete only |
| Double-click | COMPLETE | INCOMPLETE | INCOMPLETE | INCOMPLETE |

Delete behavior needs no semantic rework. Leaf deletion is directly undoable;
non-leaf deletion identifies the complete dependant closure and requires
confirmation; Undo/Redo restores the exact graph, IDs, order and counter. C5
should only refine wording, keyboard access and post-action selection feedback.

## Proposed C5 scope

### 1. Consistent edit transactions

- Double-click Sketch enters Edit Sketch and performs deterministic Normal To.
- Double-click Extrude or Cut Extrude opens its PropertyManager directly in
  edit mode.
- Selecting alone shows read-only properties; Edit is explicit from button,
  context menu or double-click.
- Draft values remain transient until Confirm. Confirm performs one validated
  authoritative edit, explicit rebuild and one Undo step.
- Cancel or Escape discards the complete draft and performs no authoritative
  mutation.
- Sketch editing retains the established per-action Undo model. `Exit Sketch`
  completes the editing mode; C5 must label this clearly rather than imply that
  it is a rollback operation.
- Boolean remains inspectable. If operation editing can be exposed solely
  through the existing `setBooleanOperation` contract, it may use the same
  Confirm/Cancel pattern. Operand rewiring and a new Boolean creation workflow
  are excluded.

### 2. Rename and naming policy

- Add Rename through F2 and the relevant context menu.
- Rename changes displayName only; FeatureId, dependencies, kind and derived
  identity remain unchanged.
- Trim leading/trailing whitespace and reject empty or whitespace-only names.
- Require case-insensitive uniqueness among live features. Human-readable
  diagnostics and relationships must remain unambiguous.
- Keep automatic type names (`Sketch1`, `Extrude1`, `CutExtrude1`, `Boolean1`)
  unique among live records. Retired FeatureIds remain unaffected.
- Rename is one Undo step and is preserved by Save/Open using the existing v2
  displayName field. No schema version change is required.

### 3. Feature Manager and context actions

- Use distinct icons for Sketch, Extrude, Cut Extrude and Boolean.
- Use shape plus accessible tooltip/text for dirty, failed and blocked states;
  do not rely only on color.
- Keep CLEAN silent and show active edit/hidden state without raw enums.
- Limit each context menu to valid actions:
  - Sketch: Edit Sketch, Show/Hide, Delete, Rename;
  - Extrude/Cut Extrude: Edit, Show/Hide, Delete, Rename;
  - Boolean: Edit only if supported, Inspect, Show/Hide, Delete, Rename.
- Apply the same validity filtering to the Graphics Area context menu.
- Preserve expand/collapse through ordinary edits for the session. Use stable
  logical identity for widget state; do not persist expansion in `.microsw`.

### 4. Rebuild state, diagnostics and navigation

- Keep explicit global Rebuild authoritative under ADR-0042.
- Keep Rebuild discoverable in the command bar and context surface; make its
  enabled state and feedback meaningful when no feature is dirty.
- Replace raw FeatureId output with feature display names.
- A failed feature shows a short user-facing cause in its tooltip and
  PropertyManager/diagnostic area.
- A blocked feature names the failed upstream item and explains that it was not
  rebuilt.
- Selecting a failed or blocked node reveals its diagnostic. An Edit action
  opens the selected feature, or the likely upstream source when the selected
  kind itself has no editable cause.
- Present provenance consistently, for example `Profile: Pocket Sketch`,
  `Target: Base`, and `Support: Pocket floor of Pocket`, without internal IDs.
- Boolean diagnostics translate restricted-kernel outcomes into user-facing
  explanations; the Boolean kernel and supported case set do not change.

### 5. Undo/Redo and deletion refinement

- Rename and every confirmed parameter edit are one Undo step.
- Cancel creates no Undo step.
- Existing delete closure remains one Undo step and retains its named
  confirmation.
- After Undo, Redo, Delete or Rebuild, reconcile logical selection, active edit
  mode, PropertyManager draft and viewport selection deterministically.
- Add Delete keyboard handling only when the Feature Manager selection owns
  focus and the same confirmation policy can be preserved.

## Preview classification

Creation guidance already reports whether a Sketch yields valid closed regions,
but Extrude and Cut Extrude do not provide a full solid preview and existing
feature editing does not preview draft parameters. A derived preview would be
valuable, but it crosses Application, Presentation and rendering behavior and
would enlarge C5 beyond feature-history hardening. It is **future UX**. C5 may
improve textual validation and draft feedback without adding modeling or render
preview infrastructure.

## Candidate assessment

### Rename — INCLUDE

High user value and low architectural risk. `displayName` already exists in the
authoritative checkpoint and persistence v2. The work is a controlled domain
setter, validation, UI interaction, Undo/Redo and tests.

### Suppress / Unsuppress — DEFER; REQUIRES DECISION GATE

Suppression is authoritative state, not visibility. It requires decisions for
dependency propagation, resultFeatureId, derived caches, rebuild status,
Undo/Redo, Save/Open and persistence compatibility. Adding it would change the
D11/D12 contracts and requires a dedicated gate before implementation.

### Reorder — DEFER; REQUIRES DECISION GATE

D11 freezes append-only insertion order and backward-only dependencies. Reorder
would need dependency-valid movement rules, result semantics, absorbed Sketch
presentation rules, Undo/Redo and persisted order changes. It directly changes
a frozen architectural contract and is unsuitable for C5.

### Automatic rebuild — DEFER; REQUIRES DECISION GATE if reconsidered

Explicit rebuild remains authoritative. C5 may make confirmed edits invoke the
existing explicit application transaction as they do today, but must not adopt
a continuous or implicit rebuild policy.

## Options

| Option | User Value | Risk | Persistence Impact | Architecture Impact | Time-to-demo |
|---|---|---|---|---|---|
| A — UX Hardening Only | HIGH | LOW | LOW — existing displayName v2 field | LOW | LOW |
| B — UX + Suppression | HIGH | HIGH | HIGH — new authoritative persisted state | HIGH | MEDIUM |
| C — UX + Suppression + Reorder | HIGH | HIGH | HIGH — state and history order | HIGH | HIGH |

## Recommendation

Choose **Option A**. It resolves the observed product gaps while preserving all
frozen contracts. Rename, coherent Edit/Confirm/Cancel, relevant context menus,
human diagnostics and predictable selection/rebuild feedback turn the tree into
an editing center without mixing a new feature-state model into the v1.1 close.

```text
Decision Gate: NONE
```

Suppression or reorder must not be added opportunistically. Either requires a
future Decision Gate with the topics listed above.

## C5 exclusions

- suppression/unsuppression;
- feature reorder or drag history;
- automatic/continuous rebuild and partial rebuild;
- feature folders, configurations and roll-back bar;
- full solid previews;
- operand rewiring or a new Boolean workflow;
- pattern, mirror or any new modeling feature;
- persistence v3;
- arbitrary dependency graph visualization;
- persistent Feature Manager expansion state.

## Experience Gate

The C5 Experience Gate asks:

> Can a CAD-literate user understand, edit and repair the feature history
> without knowing internal implementation details?

Evidence must cover:

- discoverable double-click and context editing for Sketch, Extrude and Cut
  Extrude;
- consistent PropertyManager drafts, Confirm, Cancel and Escape;
- rename by context action and F2, stable identity, Undo/Redo and Save/Open;
- readable modeling narrative using user names and absorbed Sketches;
- relevant actions only for each selected kind;
- silent CLEAN state and distinct dirty, failed and blocked cues;
- failure cause and upstream blocker shown without raw IDs or enums;
- explicit Rebuild availability, success/failure feedback and repair flow;
- safe leaf and dependency-closure deletion with confident Undo/Redo;
- stable selection, active edit and expansion through ordinary actions;
- human-readable Profile, Target and Support relationships;
- Boolean limitations explained without kernel status tokens.

## v1.1 release strategy

V1.1-C5 should be the final v1.1 functional campaign. After its increments:

1. run one final C5 V3 over the complete affected executable state;
2. approve the C5 Experience Gate;
3. perform a V0 v1.1 release freeze if executable HEAD is unchanged;
4. create annotated tag `v1.1.0` with message
   `Micro SolidWorks — v1.1.0`.

Release criteria are:

- CAD-first workspace and guided Sketch creation/editing;
- solid visualization and face/edge selection;
- safe New/Open/Save/Save As and dirty-exit handling;
- deterministic Undo/Redo;
- additive and subtractive modeling with Sketch-on-Face;
- readable, editable and diagnosable feature history;
- predictable explicit rebuild and repair;
- rename/delete/Undo/Redo/Save/Open coherence;
- persistence v2 compatibility;
- final Technical and Experience Gates passed.

## Post-v1.1 deferred outlook

Likely later candidates are Hole, Revolve, Pattern, Mirror, Chamfer, Fillet,
Suppression, dependency-valid Reorder, persistent topological naming and general
Booleans. This review does not order or design those campaigns.

## Validation plan for the future campaign

C5 implementation should use impact-based increments and avoid rebuilding or
testing unaffected modules. The final C5 gate is V3 because it closes v1.1.
This prerequisite review itself is V0 and changes documentation only.
