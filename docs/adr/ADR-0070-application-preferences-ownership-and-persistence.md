# ADR-0070 — Application Preferences Ownership and Persistence

Status: PROPOSED

## Context

Personal settings are needed for the UI/UX v0.3 navigation and selection alternatives, while accepted ADR-0045–0047/0066 reserve `.microsw` for authoritative CAD state and ADR-0049/0061–0063 separate session, dirty state and history. A user preference must survive launches without contaminating a document or its Undo/Redo.

## Decision proposed

Application owns one project-owned typed preferences service and its validated immutable current snapshot. It supplies defaults, loads/saves a per-user file, validates candidates and explicitly propagates a successfully applied snapshot to Viewer/Interaction/UI consumers. Consumers do not access files or JSON directly; CAD core has no dependency on the service. Initial fields are typed navigation-profile and selection-accent enums, not public string-key storage.

Preferences are distinct from authoritative Document Settings (not in SET-01) and transient Session State (`EditorContext`, selection, hover, camera, tools, previews, window state). Preference changes never mutate `FeatureModel`, Geometry, Topology or `.microsw`, create CAD history, change document dirty, or trigger geometric rebuild.

For the Windows-first release, store a bounded, versioned `preferences.json` in the user's LocalAppData/MicroSolidWorks directory. Use the existing pinned JSON parser privately where appropriate; no new dependency. Schema v1 has symbolic `navigationProfile` and `selectionAccent` fields. Missing file/fields use defaults; invalid known fields fall back per field with diagnostics; unknown v1 fields are ignored and omitted on the next explicit save. Parse failure uses defaults and retains the original for diagnosis, with a valid last-known-good backup eligible for recovery. Future schema versions use defaults plus warning and are not overwritten absent explicit user-confirmed reset. Future changes use narrow explicit migrations.

An Apply/OK operation validates the whole candidate and writes a complete temporary file alongside the destination, flushes and closes it, installs it with rollback/last-known-good backup, then publishes the new snapshot. Failure leaves the active snapshot unchanged and reports an error. Apply stays open, OK closes only after success. Cancel discards only uncommitted draft edits; Restore Defaults changes only the draft. No application-global event bus or speculative settings framework is introduced.

## Rationale

Typed, Application-owned state prevents settings from becoming document data, keeps Viewer/Core independent of file formats, and makes user changes transactional without CAD rebuild/history side effects. A small versioned file supports future preferences without making an unbounded registry.

## Consequences

- Application startup coordinates load and nonfatal diagnostics; a corrupt settings file cannot block normal CAD startup.
- UI owns the edit draft; a failed save cannot be presented as success.
- Existing `.microsw` format, FeatureModel, PersistenceSession and document safety contracts remain unchanged.
- SET-01 must test missing/partial/corrupt/future files, recovery, rollback, Apply/OK/Cancel/Reset and document dirty/Undo invariants.

## Alternatives considered

- Put preferences in `.microsw` or FeatureModel: rejected because they are per-user rather than CAD-authoritative.
- Put them in `EditorContext`: rejected because that state is intentionally session-local.
- Generic string-key service/event bus: rejected as premature and hard to validate.
- Reuse `microsw_persistence` as the owner of settings: rejected because its accepted boundary is CAD document persistence; private parser reuse does not imply shared ownership.

## Deferred / non-goals

Document Settings, account/cloud sync, roaming configuration, arbitrary keybindings, complex migrations, plugins, telemetry, export/CAM preferences and persistent session layouts are outside SET-01. This ADR requires owner approval before acceptance or implementation.
