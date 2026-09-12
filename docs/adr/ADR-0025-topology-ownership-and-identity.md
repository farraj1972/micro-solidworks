# ADR-0025 — Topology Ownership and Identity

Status: ACCEPTED

## Context

B7 needs shared topological identity for a first programmatic cuboid without
turning Geometry values into topology or introducing Document identity. Adjacent
faces must refer to the same edges, and edges must refer to the same vertices.

## Decision

`Solid` is the initial aggregate root and owns internal indexed collections of
Vertex, Edge, Wire, Face and Shell records. The collections are an implementation
detail of `Solid`, not a separately owned generic topology container. Records
refer to one another through `VertexId`, `EdgeId`, `WireId`, `FaceId` and
`ShellId`; recursive records and duplicated subtopology are prohibited.
The Solid identifies its single root Shell through a `ShellId` into its owned
Shell collection.

Each ID is a distinct, strongly typed, generation-free `std::uint32_t` index.
`UINT32_MAX` is the invalid value produced by default construction; valid IDs
occupy `0` through `UINT32_MAX - 1`. Valid IDs
are collection-local, stable for the aggregate lifetime, non-persistent and not
globally unique. They are unrelated to `VisualEntityId` and to future CAD or
Document identity. An ID from one `Solid`, even with the same numeric value,
cannot be used in another aggregate.

Topology is built through aggregate APIs that validate references and invariants
before insertion. B7 exposes read-only inspection after construction. Records,
IDs, endpoints, uses and collection order cannot be mutated or removed in place.
Future editing requires a separate decision.

A `Vertex` owns an authoritative `Point3`. Distinct vertices may have
geometrically coincident positions; identity and connectivity use `VertexId`
only. B7 performs no implicit merging, sewing or healing.

## Rationale

Shared indexed records make incidence and manifold validation explicit while
keeping the first ownership model small. Append-only construction gives stable
indices without UUIDs, generations or a mutation framework.

## Consequences

- A `Solid` is self-contained and owns one coherent topology graph.
- Cross-aggregate references are invalid.
- Construction order determines local ID values but has no persistent meaning.
- Copying or moving a complete `Solid` preserves its internal ID relationships;
  IDs retained outside the source aggregate do not become cross-aggregate IDs.

## Alternatives Considered

- Recursive ownership or duplicated subtopology: rejected because adjacent
  faces would not share edge identity and incidence would be ambiguous.
- A globally owned topology repository: deferred because B7 needs no Document or
  multi-solid ownership framework.
- UUIDs or generation counters: rejected as unnecessary without persistence,
  deletion or slot reuse.

## Deferred / Non-goals

Persistent identity, Document ownership, cross-solid sharing, deletion, topology
editing, slot reuse, automatic merging, sewing and healing are deferred.
