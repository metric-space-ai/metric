# W5 Mixed-Record Visual Acceptance

Status date: 2026-06-26

## Status

Review-pending visual acceptance candidate. This is not manually accepted as a
public hero.

## Scope

- Example: `visual/examples/mixed-record-hero/index.html`
- Native evidence: `docs/examples/assets/mixed-records/metric.visual.json`
- Screenshot: `visual/output/W5-mixed-record-hero.png`
- Local URL used: `http://127.0.0.1:4173/visual/examples/mixed-record-hero/index.html`
- Mixed-record check files changed: none
- Shared view/glyph/layer files changed: none

The page uses `createMetricVisual()` and `showMixedRecords()` with
`MixedRecordView`. It does not load page-local `evidence.json`, does not
fabricate page-local records, and does not compute relation edges in
JavaScript.

## Evidence Summary

- Native records: 2,000
- Exported record types visible in the view:
  `text_code_record`, `histogram_spectrum_record`,
  `process_curve_record`, `numeric_vitals_record`
- Preview payload families verified through runtime selection:
  `string`, `histogram`, `time_series`, `vector`
- Native relation: `mixed-finite-records-composite-metric`
- Native graph: `mixed-finite-records-knn`
- Cross-type relation edges: 6,000
- Selected screenshot edge: `fleet-0000->fleet-0625:0`
  (`string` payload to `histogram` payload)

## Visual Grammar

- Primary semantic view: `MixedRecordView`
- Primary primitive: `InstancedGlyphLayer`
- Glyph grouping: exported `record_type`
- Relation primitive: `RelationEdgeLayer`
- Relation grammar: `cross-type-relation-edges`
- Supporting context: `mixed-finite-records-family-severity-3d` coordinates,
  ground plane, projection, and family/channel labels
- Preview path: native-evidence `RecordPreviewPanel` resolver for record and
  pair payloads

## Browser Probe

Manual Playwright probe at the local URL:

- Page readiness: `data-metric-mixed-hero="ready"`
- Runtime layers: `GroundPlaneLayer`, `GroundProjectionLayer`,
  `BillboardLabelLayer`, `InstancedGlyphLayer`, `RelationEdgeLayer`
- Runtime graph picking: available, 6,000 graph edges
- GPU edge probe: hit `fleet-0000->fleet-0625:0`
- Record previews sampled one record from each exported type and returned
  payload families `string`, `histogram`, `time_series`, `vector`
- Pair preview selected `fleet-0000` to `fleet-0625`, proving a text-code
  record related to a histogram-spectrum record

## Validation

Commands run after the final candidate update:

```bash
node visual/tools/check-visual-document.mjs docs/examples/assets/mixed-records/metric.visual.json
node visual/tools/check-glyph-record-grammar.mjs
node visual/tools/check-mixed-glyph-geometry.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-grae10-golden.mjs
```

Result:

- visual document schema/evidence gate: pass
- glyph record grammar: pass
- mixed glyph geometry: pass
- hero grammar contract: pass
- public gallery evidence: pass
- browser-backed public example regression: pass (`ok: true`, 8 examples,
  0 failed)
- GRAE10 golden reference: pass

## Remaining Blockers

- Manual project-page review has not accepted this screenshot as a public hero.
- The native evidence provenance still marks `public_hero_ready: false`; this
  report intentionally keeps the candidate review-pending.

Status:

- loads: yes
- renders: yes
- interactive: yes, record and cross-type pair previews are available
- visually accepted: no
- complete: no, review-pending
