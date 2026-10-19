# V1.1-C5 — Feature Workflow Hardening Validation

Status: FROZEN. Technical V3 and Experience Gate APPROVED; no blocking finding.

## Scope

V1.1-C5 hardens the existing Feature Manager workflow under approved Option A.
Sketch, Extrude and Cut Extrude editing use their existing authoritative
FeatureModel operations. Extrude and Cut Extrude drafts commit only on Confirm;
Cancel discards them. Double-click and relevant context actions open the
corresponding editor. Boolean remains inspect-only, with readable operation
and operand names. No operand rewiring or new Boolean creation workflow is
included.

Feature rename changes the authoritative `displayName` without changing
FeatureId, dependencies, geometry or derived topology. Names are trimmed,
non-empty and unique among live features under case-insensitive comparison.
Automatic names avoid current live-name collisions. Rename is one session
checkpoint and roundtrips in persistence v2 without a schema change.

The derived Feature Manager uses stable feature identity for expansion state,
type icons, distinct status symbols and user-facing tooltips. Failed and
blocked features expose a cause and an upstream feature where applicable.
Explicit Rebuild is available when features need attention and reports a named
failure with blocked count. Deleted features still follow D20 closure,
confirmation and Undo/Redo semantics. Logical feature selection is retained
after an edit or a checkpoint restore when that feature survives; topology
sub-selection is cleared on rebuild.

## Technical Gate

```text
Validation level: V3 final baseline gate
Fresh configure: PASS (Visual Studio 17 2022; pinned existing dependency sources)
Full Debug build: PASS (clean-first build; resumed after pausing test compilation for the visual gate)
Project warnings: 0 observed
Full CTest: PASS (903/903)
Runtime: PASS (manual complete workflow and normal close)
Architecture and dependency audit: PASS (FeatureModel remains authoritative; 69/69 ADRs ACCEPTED; no new dependency)
Repository hygiene: PASS (build directory ignored; no generated files tracked)
git diff --check: PASS
```

Targeted V2 evidence before the final gate: 59/59 relevant FeatureModel,
FeatureTreeModel, DocumentHistory, PersistenceSessionHistory and Persistence
tests passed. Tests cover rename validation, collision avoidance, Undo/Redo,
Save/Open, blocked dependency wording and existing delete/Cut/Sketch-on-Face
contracts.

## Experience Gate

APPROVED by the project owner on 2026-10-19 using the C5 executable from the
fresh Debug build. The owner reported that every step in the visual workflow
passed and that the application was closed normally. Native window capture
timed out, so the interface result is based on this direct manual observation.

The checklist was: create/open a part; rename a Sketch and Extrude through context and F2;
double-click an absorbed Sketch and edit geometry; double-click Extrude,
change distance, Cancel and confirm unchanged model; repeat with Confirm and
verify one Undo/Redo step; inspect a failed feature and blocked dependant;
run Rebuild and read its feedback; inspect relevant context actions; delete a
leaf and a dependency closure, including confirmation, Cancel, Undo and Redo;
Save/Open and confirm names; edit a Cut Extrude and verify target/profile
relationships. A CAD-literate user should be able to edit and repair a
parametric mechanical part without technical internal commands or IDs.

## Architecture and deferred work

FeatureModel remains authoritative. UI drafts and expansion are session-local.
The explicit rebuild policy, checkpoints, D20 deletion semantics, persistence
v2, dependency direction and all accepted ADRs remain unchanged. Suppression
and reorder are deferred and require a future Decision Gate. Auto rebuild,
partial rebuild, solid preview, new modeling features, persistence v3 and
kernel or technology migration remain outside C5.

The Personal SolidWorks product vision is APPROVED from C5 onward. It does
not expand this campaign. V1.2+ requires a separate Product & Architecture
Review. Earlier baselines retain their historical purpose and contracts.
