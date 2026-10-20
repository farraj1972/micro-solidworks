# SET-D01 — Application Settings & Preferences Architecture

Status: **PROPOSED — READY FOR OWNER REVIEW** (2026-10-20). This is a decision proposal, not authorization to implement SET-01 or change the frozen v1.1.0 executable. See [ADR-0070](adr/ADR-0070-application-preferences-ownership-and-persistence.md), [ADR-0071](adr/ADR-0071-configurable-navigation-and-selection-appearance.md) and the [SET-01 campaign proposal](PERSONAL_SOLIDWORKS_SET_01_CAMPAIGN_PROPOSAL.md).

## Objectives and boundary

Give each user stable, typed control over application behavior without turning personal preferences into CAD data. Resolve the two open [UI/UX v0.3 compatibility questions](UIUX_V0_3_GAP_ANALYSIS.md) by offering an alternative to the already validated middle-button mapping and orange selection. Preserve current behavior on first launch and for users who make no change. Future preferences may be added only for real workflows; this gate creates no generic plugin/configuration framework or new dependency.

Three kinds of state remain separate:

| Kind | Owner and lifetime | Examples | Persistence |
| --- | --- | --- | --- |
| **Application Preferences** | Application-owned, per user, across CAD documents and launches | Navigation profile; selected-geometry accent; future interface/accessibility choices | Separate versioned user configuration file. Never `.microsw`. |
| **Document Settings** | Authoritative document contract, only when separately designed and approved | Document units, precision or geometric parameters affecting interpretation/rebuild | Future document schema; **excluded from SET-01**. |
| **Session State** | Application/Editor/Viewer during the current session | Logical selection, hover, active tool, current camera, preview, editing draft, temporary window state | Not persisted by SET-01. |

Changing a preference never mutates `FeatureModel`, Sketch, Geometry or Topology, never changes `.microsw`, never creates a CAD Undo/Redo checkpoint, never marks the CAD document dirty, and never triggers geometric rebuild. It may change only input routing or derived appearance. Document dirty/Undo are governed by ADR-0061/0062; current `PersistenceSession` continues to own `.microsw` Save/Open and document history.

## Ownership, types and propagation

The Application owns one project-owned `PreferencesService` (name indicative). It loads once at startup, provides validated defaults and an immutable current snapshot, validates candidate values, writes user storage, and publishes a successful replacement snapshot to explicit consumers. Initial public values are typed enums, for example `NavigationProfile { CurrentCad, AlternativeCad }` and `SelectionAccent { Orange, Blue }`; no public string-key `get/set` interface. A small schema value groups these options and a schema version. Unknown future categories are not pre-created as an extensible runtime registry.

Application startup owns load and diagnostics; UI owns only the draft and presentation of validation/errors. Viewer/Interaction receive the navigation enum at their boundary; Presentation/Viewer and the Feature Manager receive a resolved selection palette/token. They never read or write the preferences file. A successful Apply explicitly hands the new snapshot to these existing consumers in the Application loop. No global observer/event bus is needed. `EditorContext` remains the owner of transient selection/tool/display state, not of per-user preferences. The service can initially live at the Application boundary; a separate library is justified only if actual reuse and dependency closure require one. No dependency from CAD core to Application/UI is introduced.

## Storage, schema and recovery proposal

On Windows, store `preferences.json` under the current user's **LocalAppData/MicroSolidWorks** directory obtained through a platform boundary (not a hard-coded user path). This is independent of project files and the `.microsw` v2 format. An initial conceptual record is:

```json
{
  "schemaVersion": 1,
  "navigationProfile": "currentCad",
  "selectionAccent": "orange"
}
```

The file is size-bounded; parse and validate every known typed field. The existing pinned JSON parser may be reused privately at the Application storage adapter; its types must not cross the service boundary. This proposes no new third-party dependency and does not expand `microsw_persistence`'s CAD-document authority. Write a complete temporary file in the same directory, flush and close, then replace the target with rollback on failure, retaining one last-known-good backup. The implementation must prove that a failed replacement preserves the previous valid file. Never report Apply success before the new file is safely installed. This is a separate preferences transaction, not CAD Save.

No file on first launch means defaults. A structurally valid v1 file with missing known optional keys uses defaults for those keys; invalid known values use that key's default and produce a visible/nonfatal diagnostic. Unknown v1 keys are ignored and not round-tripped on a later explicit save; no hidden extension state enters the typed API. A parse failure or invalid root uses defaults and retains the unreadable file for diagnosis until an explicit successful Apply/Reset; recovery may read the valid last-known-good backup. A **future schemaVersion** is never silently rewritten: use defaults for that launch, warn that settings could not be loaded, and require an explicit user-confirmed reset before overwriting the future file. Version 1 is the only writer in SET-01. A later schema change adds a narrowly tested, explicit version migration; no generic migration framework now. These rules keep unknown or corrupted preferences from blocking CAD startup or damaging documents.

## Editing transaction and UI

Propose **Edit → Preferences** in the existing shell. Opening copies the current validated snapshot to a draft. Navigation and Appearance are the only configurable sections in SET-01; General, Accessibility, Sketch and Files & Export are conceptual expansion areas and must not show fake enabled controls. The Orange/Blue selector includes a local sample preview of selected geometry; it does not alter the global Viewer before confirmation.

- **Apply:** validate draft, persist atomically, publish the snapshot to consumers and keep the dialog open. On validation/I/O failure, leave active settings unchanged, preserve the draft and show a useful error.
- **OK:** same transaction as Apply, then close only on success.
- **Cancel:** discard only unapplied draft changes; a previous successful Apply remains active and saved.
- **Restore Defaults:** replace the draft with current schema defaults; an Apply/OK is still needed to persist and activate them.

The editing dialog must remain usable if user storage is unavailable; it must not falsely imply a saved preference. A reset of an unreadable/future file requires a clear confirmation before replacement.

## Navigation and appearance contracts

| Profile | MMB drag | Shift+MMB drag | Wheel |
| --- | --- | --- | --- |
| **A — Current CAD Navigation (default)** | Orbit | Pan | Zoom |
| **B — Alternative CAD Navigation** | Pan | Orbit | Zoom |

`WorkspaceViewport` currently calls `OrbitNavigation::handle` and `PanZoomNavigation::handle`; those handlers currently encode Shift routing. SET-01 should route the typed profile at the Viewer/Interaction input boundary to the *existing* orbit and pan/zoom operations, without duplicating camera algorithms, changing `OrbitCamera` orientation or changing zoom/projection mathematics. A profile change during a captured MMB drag cancels that gesture safely, clears its capture, and takes effect only on the next press. It does not reset the camera. Normal To, standard views, picking and selection keep their contracts.

`SelectionAccent` is **Orange (default)** or **Blue**. The choice affects selected logical geometry and its synchronized selected indication in the Feature Manager, including supported sketch entities, solids, faces and edges across Shaded, Shaded With Edges and Wireframe. It does not recolor hover/preselection, active editing, context/reference, warning, error/FAILED/BLOCKED, disabled/hidden or normal geometry. Selection continues to override hover on the same target; picking identity and lifetime never depend on color. Use semantic palette tokens at UI/Presentation/Viewer boundaries so no literal color is scattered across render paths. The current `HighlightColors.h` orange and solid highlight remain the first-launch reference. Exact blue RGB and contrast thresholds require visual validation on dark background and all three display modes before acceptance; the owner should not approve a palette that hides topology edges or resembles an error state. Also show a non-color selection cue (for example outline/line weight/tree selected state) and distinct icons/text for failure states; color alone cannot carry meaning. Full high-contrast themes, light/dark themes and advanced font scaling are deferred.

## Architecture impact and risks

| Accepted contract | SET-D01 interpretation |
| --- | --- |
| ADR-0010, ADR-0011, ADR-0067 (Viewer/camera) | ADR-0010 explicitly permits future configurable mappings. Defaults and orbit/pan/zoom mathematics remain; arbitrary orientation, Normal To and projection unaffected. |
| ADR-0018, ADR-0050, ADR-0057, ADR-0058 (selection/picking) | Identity, lifetime, hover/selected precedence, picking and Feature Manager synchronization remain. Only selected-state presentation color becomes a user preference. |
| ADR-0049, ADR-0051, ADR-0052 (EditorContext/workflow) | Transient tool/selection/display ownership and atomic CAD transactions stay intact. ADR-0052's reference to frozen MMB conventions is preserved as the default, with an explicit owner-approved alternative only if ADR-0071 is accepted. |
| ADR-0060 (display modes) | Shaded/edge overlay semantics remain; temporary selected edge emphasis remains allowed in Shaded. Palette must work in each mode. |
| ADR-0045–0047, ADR-0066 (document persistence) | `.microsw` authoritative payload and v1/v2 migration remain unchanged. User preferences use another file and owner. Existing JSON parser is private infrastructure only. |
| ADR-0061–0063 (dirty, Undo, exit) | Preference Apply is not a CAD transaction and does not trigger document dirty, undo history or unsaved-document prompts. Existing New/Open/Exit safety remains. |

The main risks are a profile switch during drag, inconsistent selection palette across render paths, accidental coupling between preferences and document Save/Undo, file replacement failure, and a future-version file being overwritten. SET-01 needs focused tests for each and a user Experience Gate. If implementation cannot keep the accepted boundaries, stop for owner review rather than silently broadening ADR-0070/0071. No accepted ADR is amended in this proposal.

Future Viewer, Sketch, UI, Export or CAD/CAM preferences may be added as new typed fields only when a real workflow needs them. Settings that change model interpretation or rebuild are Document Settings and require their own architecture/schema decision. SET-01 does not include arbitrary binding maps, macros, complex themes, font scaling, workspace profiles, cloud sync, accounts, telemetry, STL export preferences or CAM configuration.

## Decisions requested from the owner

1. Approve or revise the Application-owned typed service, Windows per-user file/recovery policy and strict separation from document/session state (ADR-0070).
2. Approve or revise Profile B and Blue as opt-in alternatives while preserving Profile A and Orange defaults, including cancellation of an active drag on profile change (ADR-0071).
3. Decide whether to authorize the bounded [SET-01 implementation campaign](PERSONAL_SOLIDWORKS_SET_01_CAMPAIGN_PROPOSAL.md) after this gate is frozen. Until then it remains proposed, and UX-01 plus V1.2-C1–C4 remain separately unauthorized.
