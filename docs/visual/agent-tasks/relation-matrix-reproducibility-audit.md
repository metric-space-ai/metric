# Relation Matrix Reproducibility Audit

## Owner

This task owns only:

- `examples/engine/relation_matrix_visual_export.cpp`
- optional generated/public asset update:
  `docs/examples/assets/relation-matrix/metric.visual.json`
- optional report update under
  `docs/visual/reports/relation-matrix-reproducibility-audit.md`

Do not edit visual runtime, public pages, other exporters, CMake, or unrelated
dirty files.

## Purpose

The native exporter quality audit found that the relation-matrix exporter
validates, but regenerating the document can differ from the checked public
asset by numeric formatting/precision. That makes public evidence harder to
review and makes future agents tempted to patch around generated output.

Normalize the exporter/public asset contract so regeneration is deterministic
and documented.

## Non-Negotiables

- C++ computes the relation matrix and metric-law diagnostics.
- JavaScript must not compute relation values to compensate for exporter
  output.
- Do not reduce evidence just to make diffs small.
- Do not touch the protected GRAE10 reference.
- Do not revert other agents' work.

## Required Work

1. Regenerate relation-matrix evidence to a temporary path.
2. Compare it with
   `docs/examples/assets/relation-matrix/metric.visual.json`.
3. Determine whether differences are:
   - numeric formatting only,
   - ordering only,
   - actual evidence changes.
4. If the difference is formatting/order only, make the exporter deterministic
   and update the public asset if necessary.
5. If the difference is actual evidence, document the exact cause and do not
   hide it.
6. Keep the default stdout contract intact:
   - no arguments emit JSON to stdout
   - `--export-dir <dir>` writes `<dir>/metric.visual.json`

## Validation

Run all commands that are possible in the current workspace:

```bash
cmake --build build/core --target engine_relation_matrix_visual_export -- -j4

build/core/examples/engine/engine_relation_matrix_visual_export \
  > /tmp/relation_matrix.metric.visual.json
node visual/tools/check-visual-document.mjs \
  /tmp/relation_matrix.metric.visual.json

tmp_dir="$(mktemp -d)"
build/core/examples/engine/engine_relation_matrix_visual_export \
  --export-dir "$tmp_dir/relation"
node visual/tools/check-visual-document.mjs \
  "$tmp_dir/relation/metric.visual.json"

cmp -s /tmp/relation_matrix.metric.visual.json \
  docs/examples/assets/relation-matrix/metric.visual.json || true

ctest --test-dir build/core -R 'example_engine_relation_matrix_visual_(export|validate)' --output-on-failure
```

If `build/core` is unavailable, run `cmake -S . -B build/core` first.

## Done

The task is done when the relation-matrix exporter is deterministic enough for
review, the public asset is either byte-consistent or the remaining difference
is precisely documented, and all relation-matrix visual validation gates pass.
