# Hero Visual Briefs Workstream

Status date: 2026-06-25

## Scope

Added a checked manifest for review-pending public visual previews. The manifest
does not accept heroes. It records the visual claim, primary grammar, native
evidence expectations, required layer primitives, minimum evidence target and
explicit blockers for each public preview.

## Files

- Manifest: `visual/hero-visual-briefs.manifest.json`
- Gate: `visual/tools/check-hero-visual-briefs.mjs`
- User-facing rule: `docs/visual/hero-visual-briefs.md`

## Current Result

`node visual/tools/check-hero-visual-briefs.mjs` passes against the current
browser regression report.

Current public preview count: `7`.

Record-count blocked previews:

- `condition-monitoring-hero`: `15` records, hero target `500`
- `mixed-record-hero`: `20` records, hero target `2000`
- `cross-space-dependency-hero`: `48` records, hero target `500`
- `dynamics-noise-hero`: `28` records, hero target `500`
- `mapping-dimensionality-hero`: `15` records, hero target `1000`
- `process-curve-external-hero`: `64` records, hero target `500`

`relation-matrix-neighborhood` has `130` records and satisfies the current
matrix-size target of `128`, but remains blocked by human visual composition
and matrix readability acceptance.

## Checks Run

- `node visual/tools/check-hero-visual-briefs.mjs`: passed.

## Effect

Future public preview work cannot rely on a green render check alone. Each
preview has to keep a visible evidence target and blocker state in the manifest
until it is accepted through the separate hero acceptance manifest.
