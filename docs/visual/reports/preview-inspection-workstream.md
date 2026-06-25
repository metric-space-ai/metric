# Preview/Inspection Workstream

Status date: 2026-06-25

## Slice

This slice expands the shared `RecordPreviewPanel` resolver. It remains an
engine-level preview path driven by `metric.visual.v1` evidence; no public page
or local preview DOM was added.

Implemented:

- record payload-family summaries for string, vector, time-series, histogram,
  image-reference and composed payloads
- exported record properties in selected-record previews
- linked coordinate/view memberships for records that appear in multiple
  exported views
- pair previews with row/column record context and exported record properties
- relation-independent pair property lookup for native pair properties that do
  not carry a `relation_id`
- symmetric relation reverse lookup for native upper-triangle pair values

Native fixtures used by the focused check:

- `docs/examples/assets/mixed-records/metric.visual.json`
- `docs/examples/assets/cross-space-dependency/metric.visual.json`
- `docs/examples/assets/relation-matrix/metric.visual.json`

## Checks Run

```bash
node visual/tools/check-record-preview-payloads.mjs
node visual/tools/check-runtime-picking-preview.mjs
node visual/tools/check-relation-matrix-picker.mjs
node visual/tools/check-views.mjs
node visual/tools/check-single-render-pipeline.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
METRIC_VISUAL_OUT=/tmp/metric-visual-preview-inspection-regression node visual/tools/check-visual-regression-public-examples.mjs
```

All commands passed. The browser regression output was redirected to `/tmp` to
avoid writing repo artifacts outside this workstream's allowed file set.

## Runtime State Keys

The shared runtime state still exposes preview selection through:

- `selectedRecord`
- `selectedRecordId`
- `selectedRecordPreview`
- `selectedPair`
- `selectedPairPreview`
- `selectionPreview`

## Remaining Gaps

- The preview panel is still compact; richer layout polish for very large
  composed records remains future UI work.
- Matrix GPU picking remains optional while the semantic matrix picker is
  sufficient for current native preview scale.
- Public previews remain preview-only until screenshot review accepts each hero
  grammar.
