# W5 Mixed-Record Visual Acceptance

Status date: 2026-06-25

## Status

Review-pending screenshot candidate, not manually accepted as a public hero.

## Scope

- Example: `visual/examples/mixed-record-hero/index.html`
- Native evidence: `docs/examples/assets/mixed-records/metric.visual.json`
- Screenshot target: `visual/output/W5-mixed-record-hero.png`
- Checks updated: `visual/tools/check-mixed-glyph-geometry.mjs`,
  `visual/tools/check-glyph-record-grammar.mjs`

The page uses `createMetricVisual()` and `showMixedRecords()` with
`MixedRecordView`. It does not load page-local `evidence.json`, does not
fabricate page-local records, and does not compute relation edges in JavaScript.

## Evidence Summary

- Native records: 2,000
- Exported record types:
  `text_code_record`, `histogram_spectrum_record`,
  `process_curve_record`, `numeric_vitals_record`
- Payload families: `string`, `histogram`, `time-series`, `vector`
- Native relation: `mixed-finite-records-composite-metric`
- Native cross-type graph: `mixed-finite-records-knn`
- Relation/edge entries used by the view: 6,000

## Visual Grammar

- Primary semantic view: `MixedRecordView`
- Primary primitive: `InstancedGlyphLayer`
- Glyph grouping: exported `record_type`
- Relation primitive: `RelationEdgeLayer`
- Supporting context: `mixed-finite-records-family-severity-3d` coordinates,
  ground plane and projection

## Validation

Commands run:

```bash
node visual/tools/check-mixed-glyph-geometry.mjs
node visual/tools/check-glyph-record-grammar.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
```

Result:

- mixed glyph geometry check: pass
- generic glyph record grammar check: pass
- public gallery evidence gate: pass
- GRAE10 golden reference: pass

The checks verified four native record types, four payload families, typed
glyph geometry/material channels and 6,000 native selectable relation edges.

## Remaining Blockers

- Manual screenshot review has not accepted this as a public hero.
- The view still needs visual review for whether the cross-type relation edges
  are legible enough in the final composition.

Status:

- loads: yes by command/example checks
- renders: yes by descriptor checks
- interactive: shared record and edge preview path available, manual browser
  review still pending
- visually accepted: no
- complete: no, review-pending
