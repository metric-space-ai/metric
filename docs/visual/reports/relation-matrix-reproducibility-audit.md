# Relation Matrix Reproducibility Audit

Date: 2026-06-26
Branch: `codex/visual-engine-evidence-exporters`

## Summary

- Regenerated relation-matrix evidence from
  `examples/engine/relation_matrix_visual_export.cpp`.
- Classified the pre-fix public asset difference as numeric formatting only.
- Updated `docs/examples/assets/relation-matrix/metric.visual.json` from the
  native C++ exporter output so the checked asset is byte-consistent with
  regeneration.
- Kept the exporter stdout/file-output contract unchanged: no arguments emit
  JSON to stdout, and `--export-dir <dir>` writes `<dir>/metric.visual.json`.
- No JavaScript relation values were computed or substituted.

## Difference Classification

Before the asset update, the regenerated JSON and checked public asset had the
same structure and ordering:

- Missing fields: `0`
- Extra fields: `0`
- Type differences: `0`
- Non-numeric value differences: `0`
- Numeric value differences: `11530`
- Equal numeric values: `722`
- Relation pair ordering: unchanged
- Graph edge ordering: unchanged

The public asset used the older six-significant-digit numeric serialization,
while the current native exporter writes round-trip `double` values through
`mtrc::visual` with the classic locale and `max_digits10` precision. The largest
observed absolute numeric delta was `0.00004995710746591442` at
`relations[0].values[4699].value`; the largest observed relative delta was
`0.00000492737178462083` at `relations[0].values[3708].value`.

## Changed Files

- `docs/examples/assets/relation-matrix/metric.visual.json`
- `docs/visual/reports/relation-matrix-reproducibility-audit.md`

## Validation

- `cmake --build build/core --target engine_relation_matrix_visual_export -- -j4`
  - PASS, `Built target engine_relation_matrix_visual_export`
- `build/core/examples/engine/engine_relation_matrix_visual_export > /tmp/relation_matrix.metric.visual.json`
  - PASS, exit `0`
- `node visual/tools/check-visual-document.mjs /tmp/relation_matrix.metric.visual.json`
  - PASS, `ok: true`, `total: 1`, `records: 130`, `relations: 1`,
    `properties: 7`, `diagnostics: 1`
- `build/core/examples/engine/engine_relation_matrix_visual_export --export-dir /var/folders/hf/x8jpryjx6xs8k0kwmkj5spkw0000gn/T/tmp.3VCR7yTLPr/relation`
  - PASS, exit `0`
- `node visual/tools/check-visual-document.mjs /var/folders/hf/x8jpryjx6xs8k0kwmkj5spkw0000gn/T/tmp.3VCR7yTLPr/relation/metric.visual.json`
  - PASS, `ok: true`, `total: 1`, `records: 130`, `relations: 1`,
    `properties: 7`, `diagnostics: 1`
- `cmp -s /tmp/relation_matrix.metric.visual.json docs/examples/assets/relation-matrix/metric.visual.json`
  - PASS, exit `0`
- `cmp -s /tmp/relation_matrix.metric.visual.json /var/folders/hf/x8jpryjx6xs8k0kwmkj5spkw0000gn/T/tmp.3VCR7yTLPr/relation/metric.visual.json`
  - PASS, exit `0`
- `ctest --test-dir build/core -R 'example_engine_relation_matrix_visual_(export|validate)' --output-on-failure`
  - PASS, `2/2` tests passed
