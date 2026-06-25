# Preview Presentation Workstream

Status date: 2026-06-25

## Files Changed

- `visual/src/interaction/record-preview.js`
- `visual/tools/check-record-preview-presentation.mjs`
- `docs/visual/reports/preview-presentation-workstream.md`

## Payload Families Covered

- Strings/text: bounded text snippets with truncation diagnostics.
- Process curves/time-series: sample count, unit/sample rate fields and sparkline data from exported series.
- Histograms/distributions: bin/edge summaries and compact histogram sparkline data.
- Image-like compact arrays: bounded canvas-ready grayscale/RGB array previews from exported pixel/matrix/value arrays.
- Composed/mixed records: component sections with each component routed through the same payload renderer selection.
- Generic scalar/categorical/vector properties: scalar/category fields, feature summaries and named vector fields.

All preview content is sourced from `metric.visual.v1` records, relations and
properties. JavaScript only renders or summarizes exported evidence for
inspection; it does not compute METRIC distances, neighbors, entropy,
dependence, mappings or dynamics.

Presentation diagnostics remain on the preview data object for gates and runtime
inspection. They are not rendered as user-facing badges in the hover panel.

## Pair Preview Fields Covered

- Row and column record identity: ids, labels, types and exported record properties.
- Relation id, relation name, relation type and exported relation value.
- Optional pair properties from direct pair input and document-level pair properties.
- Symmetric native pair values through reverse lookup when the exported relation declares symmetric storage/metadata.
- Presentation diagnostics for relation value availability, pair-property count, record identity availability and symmetric reverse lookup.

## Checks Run

```bash
node visual/tools/check-record-preview-payloads.mjs
node visual/tools/check-record-preview-presentation.mjs
node visual/tools/check-runtime-picking-preview.mjs
node visual/tools/check-visual-command-api.mjs
node visual/tools/check-views.mjs
node visual/tools/check-single-render-pipeline.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
```

All checks passed.

## Remaining Payload Renderer Gaps

- Rich image assets beyond compact exported arrays still use reference or
  compact-array presentation; palette-aware and multi-frame renderers remain
  future work.
- Tabular/dataframe-like records currently fall back to generic property fields.
- External binary media such as audio/video remains out of scope for this panel.

## Public Hero Status

Public hero status did not change. No public example pages, project pages,
native exporters, renderer layers, relation matrix, glyph/field,
mapping/dynamics code or GRAE10 references were edited.
