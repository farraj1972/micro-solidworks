# ADR-0048 - B14 and MVP Validation/Freeze Strategy

Status: ACCEPTED

## Context

B14 completes executable MVP capability. Validation must cover file semantics
and the full workflow without repeating full regression for every increment or
for documentary freezes over an unchanged executable commit.

## Decision

B14 uses V1 for DTO/schema/parser/writer and domain reconstruction increments,
V2 for Application New/Save/Open, rebuild, and Presentation integration, and one
final V3 for the complete repository. That final B14 V3 is also the MVP
executable validation.

The V3 runtime smoke covers New; Sketch and constraints; Extrude; a second
branch and Boolean; Rebuild; Save As `.microsw`; New/reset; Open with automatic
rebuild; Feature Tree, final Solid, selection, and presentation; upstream edit,
Rebuild, Save, reset and reopen; and normal shutdown. It also verifies invalid
Open, unsupported version, and Save failure preserve the prior session/file.

If V3 passes and executable HEAD is unchanged, B14.FREEZE and MVP.FREEZE are
separate V0 documentary steps without another full regression. The B14 freeze
tag is `b14-persistence`. The MVP documentary freeze creates annotated tag
`mvp-1.0` with message `Micro SolidWorks — MVP 1.0`, over the same validated
executable state.

GAP-GEO-002 and GAP-SCENE-002 remain OPEN / deferred; persistence lifecycle is
not general geometry intersection or scene visibility/lifecycle semantics.

## Rationale

One executable V3 provides the required confidence. Repeating it after only
state documentation changes adds cost without new technical evidence.

## Consequences

- Microincrements use impact-based validation.
- Neither B14 nor MVP is frozen by this proposal.
- Freeze and tag operations still require explicit authorization and PASS state.

## Alternatives Considered

- V3 after every increment: rejected as disproportionate.
- Separate identical B14 and MVP full regressions: rejected when executable HEAD
  is unchanged.

## Deferred / Non-goals

Post-MVP capabilities and all persistence items deferred by ADR-0045–0047 remain
outside B14.
