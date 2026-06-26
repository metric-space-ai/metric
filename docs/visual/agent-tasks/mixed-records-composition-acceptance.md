# Mixed Records Composition Acceptance

## Purpose

Move the native mixed-record preview closer to accepted-hero quality by making
heterogeneous record structure readable through the reusable typed-glyph and
cross-type relation grammar.

This task must not collapse the example into another point cloud. It must not
invent records, fake payloads, rewrite native evidence in the page, or draw a
custom page-local renderer.

## Owner Scope

Primary write scope:

- `visual/src/views/MixedRecordView.js`
- `visual/examples/mixed-record-hero/index.html`
- optional checker: `visual/tools/check-mixed-record-composition.mjs`
- report: `docs/visual/reports/mixed-records-composition-acceptance.md`

Allowed supporting files only if necessary:

- `visual/src/layers/InstancedGlyphLayer.js`
- `visual/src/layers/RelationEdgeLayer.js`
- `visual/src/views/GroundProjectionView.js`

Do not edit:

- `visual/examples/grae10-metric-engine/**`
- `visual/regression-baselines/grae10-metric-engine.sha256`
- native C++ exporters
- other public hero pages
- `visual/hero-acceptance.manifest.json`

## Current Evidence

Use only:

```text
docs/examples/assets/mixed-records/metric.visual.json
```

The native evidence currently has 2,000 records, 4 record types and explicit
cross-type relation evidence. The open blocker is visual composition, not
record count.

Current screenshot reference:

```text
output/visual/check-visual-regression-public-examples/mixed-record-hero.canvas.png
```

Current issue: the scene must read as heterogeneous finite records with
type-specific structure and cross-type relationships. It must not read as a
generic colored point cloud with labels.

## Required Outcome

Improve `MixedRecordView` and/or reusable layer options so the preview shows:

- distinct visual glyph language per exported record type;
- cross-type relation edges that reveal relation structure without clutter;
- type labels and ground projection that orient the viewer;
- hover/selection previews resolving original payload families through the
  shared preview path;
- no JavaScript metric computation, no generated fake records and no page-local
  descriptor factory;
- a screenshot composition that makes the typed-glyph grammar primary.

Prefer changes in `MixedRecordView` and reusable layer options over page CSS or
ad hoc descriptor mutation.

## Acceptance

Run:

```bash
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-glyph-record-grammar.mjs
node visual/tools/check-mixed-glyph-geometry.mjs
node visual/tools/check-visual-command-api.mjs
node visual/tools/check-views.mjs
node visual/tools/check-hero-visual-briefs.mjs
METRIC_VISUAL_EXAMPLES=mixed-record-hero node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-hero-screenshot-review.mjs
```

If a custom checker is added, it must prove at least:

- selected view kind is `mixed-records`;
- descriptors include `InstancedGlyphLayer`, `RelationEdgeLayer` and
  `GroundProjectionLayer`;
- at least four exported record types are represented;
- relation edges come from native relation evidence;
- no synthetic evidence or page-local render helper is used.

Do not mark the hero accepted. Leave `visual-composition-not-human-accepted`
until a separate human review clears it.
