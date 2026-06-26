# Mixed Records Composition Acceptance

## Summary

Implemented the mixed-record composition upgrade through the reusable
`MixedRecordView` path. The preview still uses only
`docs/examples/assets/mixed-records/metric.visual.json` as application
evidence and still reaches the scene through `showMixedRecords()`.

## Changed Files

- `visual/src/views/MixedRecordView.js`
- `visual/examples/mixed-record-hero/index.html`
- `visual/tools/check-mixed-record-composition.mjs`
- `docs/visual/reports/mixed-records-composition-acceptance.md`

## Implementation Notes

- `MixedRecordView` now owns a deterministic mixed-record type palette and
  default type labels, so exported record types do not collide through hashed
  category colors.
- Direct `MixedRecordView.fromVisualSpace()` calls now normalize
  `pointSize`, `labels: "record_type"`, label offsets and typed ground
  projection options without relying on the public command wrapper.
- The typed glyph descriptor, ground projection descriptor and relation edge
  descriptor now carry explicit mixed-record composition metadata.
- Ground projection colors now follow the same typed record category encoding
  as the glyphs.
- Relation edges still come from the native graph/relation evidence, but now
  encode source/target type-pair color and native relation-value emphasis in
  reusable `RelationEdgeLayer` channels.
- The hero page requests `record_type` labels and the shared type palette
  through `showMixedRecords()`; it does not create a local renderer or
  descriptor factory.

## Checks Run

Passed:

```bash
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-glyph-record-grammar.mjs
node visual/tools/check-mixed-glyph-geometry.mjs
node visual/tools/check-visual-command-api.mjs
node visual/tools/check-views.mjs
node visual/tools/check-mixed-record-composition.mjs
METRIC_VISUAL_EXAMPLES=mixed-record-hero node visual/tools/check-visual-regression-public-examples.mjs
```

Mixed regression result from the successful run:

- `mixed-record-hero` ok: true
- records: 2,000
- record types: 4
- primitives: `GroundPlaneLayer`, `GroundProjectionLayer`,
  `BillboardLabelLayer`, `InstancedGlyphLayer`, `RelationEdgeLayer`
- graph edges: 6,000
- status: `public-preview-only`
- screenshot: `output/visual/check-visual-regression-public-examples/mixed-record-hero.canvas.png`

Blocked by filtered shared regression report:

```bash
node visual/tools/check-hero-visual-briefs.mjs
node visual/tools/check-hero-screenshot-review.mjs
```

`check-hero-visual-briefs.mjs` saw the mixed-only regression report and kept
`mixed-record-hero` green/review-pending, but failed because the report did not
include the other brief-manifest examples.

`check-hero-screenshot-review.mjs` saw the mixed-only regression report and
kept `mixed-record-hero` green/review-pending with screenshots present, but
failed because the report did not include the protected `grae10-metric-engine`
row required by the global review gate.

After the mixed regression completed, another concurrent filtered example run
overwrote the shared `output/visual/check-visual-regression-public-examples/results.json`.
The mixed screenshot artifacts remain present, but the shared JSON path is not
a stable task-local artifact while multiple agents are running filtered visual
checks.

## Remaining Blockers

- `visual-composition-not-human-accepted` remains in force. The preview is not
  marked accepted and the hero acceptance manifest was not edited.
- The global brief/review gates require a full public regression report that
  includes all preview brief rows and protected GRAE10. I did not regenerate a
  full report because this task was constrained to the mixed-record application
  evidence.

## Preview Status

The mixed-record preview remains review-pending.

## Relation-Edge Legibility Update

Date: 2026-06-26

Updated the reusable mixed-record relation edge descriptor path without
changing native evidence, graph topology, record counts or the public page.

Additional descriptor evidence now exposed by `MixedRecordView`:

- `RelationEdgeLayer` channels include `endpointEmphasis` alongside native
  `edgeEmphasis` and `edgeTypePair`.
- `edgeLegibility.rank` declares deterministic
  `native-relation-value-cross-type-rank` from the exported relation value
  emphasis channel.
- `edgeLegibility.laneBundle` declares `type-pair-lanes` with
  `edgeTypePair:sourceId:targetId`, 13 deterministic lanes and geometry-only
  offsets.
- `edgeLegibility.endpointEmphasis` declares
  `cross-type-endpoint-relation-value` and preserves endpoint identity.
- `RelationEdgeLayer.getDiagnostics()` now reports applied alpha, lane and
  emphasis summaries for checker/runtime assertions.

Verification run for this update:

```bash
node visual/tools/check-linked-edge-emphasis.mjs
node visual/tools/check-mixed-glyph-geometry.mjs
node visual/tools/check-mixed-record-composition.mjs
```

All passed. The acceptance blocker remains
`visual-composition-not-human-accepted`.
