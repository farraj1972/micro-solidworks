# V1.1-C3 — Document Safety, Undo/Redo & Session Workflow Validation

## Result

```text
STATUS: FROZEN
Technical Gate: PASS
Experience Gate: APPROVED
```

V1.1-C3 implements ADR-0061–0063. `DocumentDirty` is session state and remains
independent from feature rebuild state. New, Open, Save and Save As establish
the saved authoritative baseline.

`FeatureModelCheckpoint` contains authoritative Feature and Sketch values,
stable IDs, tombstones, dependencies, result identity and next-ID counters.
Derived rebuild state, diagnostics, topology, Presentation and interaction state
are excluded. `DocumentHistory` owns a linear session-local cursor with 100 undo
steps plus current state, branch truncation and a separately retained saved
checkpoint. Restore always runs deterministic rebuild before Presentation refresh.

Rectangle geometry and inferred constraints are one history entry. Failed
rebuild edits remain dirty and undoable; Redo restores the failing authoritative
edit. Exact counter restoration permits reuse of IDs that existed only in a
discarded future branch.

Save and Save As use the standard Windows file dialog; Open uses the standard
Windows open dialog. There is no technical path input in the Model panel. Dirty
New, Open, File → Exit and native window close share one Save / Don't Save /
Cancel policy. Save failure or file-dialog cancellation preserves the document,
path and history. Open replaces the current session only after loading succeeds.

The title and status bar show `*` for unsaved authoritative changes. Undo and
Redo are available in the Edit menu, compact command controls and Ctrl+Z/Ctrl+Y.

Final V3 evidence:

```text
fresh configure: PASS
full Debug build: PASS
project warnings: 0
CTest: 846/846 PASS
runtime: PASS
architecture/dependency audit: PASS
git diff --check: PASS
```

The manual Experience Gate validated dirty indication, Undo/Redo, native
Save/Open dialogs, dirty New/Open/Exit, native X, all prompt choices, normal
shutdown and Feature Manager/viewport consistency. Decision: **APPROVED**.

Persistence remains `.microsw` format version 1. History, dirty state and all
transient UI/view state remain session-local. ADR-0061, ADR-0062 and ADR-0063
are CONFORMANT. No architectural deviation or external dependency was added.
