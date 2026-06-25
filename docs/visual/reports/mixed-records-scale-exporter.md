# Mixed Records Scale Exporter Report

Date: 2026-06-25
Branch: `codex/visual-engine-evidence-exporters`

## Changed Files

Edited by this task:

- `examples/engine/mixed_finite_records_fixture.hpp`
- `examples/engine/mixed_finite_records_visual_export.cpp`
- `docs/examples/assets/mixed-records/metric.visual.json`
- `docs/visual/reports/mixed-records-scale-exporter.md`

Pre-existing dirty file in owner scope left unchanged by this task:

- `examples/engine/mixed_finite_records.hpp`

## Final Counts

- Records: `2000`
- Public record types: `4`
- Relations: `1`
- Sparse relation values: `6000`
- Graphs: `1`
- Graph edges: `6000`
- Coordinates: `2`
- Properties: `11`
- Diagnostics: `10`

## Public Record Types And Payload Families

- `text_code_record`: direct `string` payload carrying source code/text.
- `histogram_spectrum_record`: direct `histogram` payload carrying spectrum bins and edges.
- `process_curve_record`: direct `time_series` payload carrying process-curve samples.
- `numeric_vitals_record`: direct `vector` payload carrying named numeric vitals.

Every exported record still has a native `MixedRecord` metric domain in C++ with code, spectrum, curve and vitals fields. The public payload exposes the record's type-specific preview family instead of collapsing all records into one composed preview.

## Native Metric Evidence

- Text/code: `mtrc::Edit<char>`.
- Histogram/spectrum: `mtrc::Wasserstein<double>::on_line`.
- Process curve: `mtrc::TWED<double>`.
- Numeric vitals/vector: `mtrc::Euclidean_standardized<double>`.
- Composite: strictly positive weighted sum using weights `{ code: 0.6, spectrum: 1.0, curve: 0.5, vitals: 1.0 }`.
- Edge evidence: native C++ candidate-neighborhood nearest selection exports sparse cross-record-type edges. Every relation and graph edge carries the composite distance, weighted contribution vector and dominant channel.
- Metric-law diagnostics: native sampled pair/symmetry checks plus deterministic sampled triangle checks. Exported diagnostics report `pair_checks: 34000`, `triangle_checks: 12000`, `triangle_violations: 0`.
- Representative/cluster evidence: native type-seeded representatives and nearest-representative assignments computed in C++ with the composite metric.

## Commands Run

- `cmake --build build/core --target engine_mixed_finite_records_visual_export -- -j4` - passed.
- `build/core/examples/engine/engine_mixed_finite_records_visual_export --export-dir docs/examples/assets/mixed-records` - two early full-matrix/index attempts were interrupted; final bounded native candidate exporter passed and wrote the asset.
- `node visual/tools/check-visual-document.mjs docs/examples/assets/mixed-records/metric.visual.json` - passed.
- `ctest --test-dir build/core -R 'visual_(export|validate)' --output-on-failure` - passed, `14/14` tests.
- `node visual/tools/check-native-hero-evidence-scale.mjs` - exited `1`; mixed-record row is scale-ready with `recordCount: 2000`, `recordTypeCount: 4`, but the manifest still lists stale record-count/type blockers.
- `node visual/tools/check-public-gallery-evidence.mjs` - passed.
- `node visual/tools/check-visual-regression-public-examples.mjs` - passed, `8/8` examples, mixed-record preview `ok: true`.

## Remaining Blocker

The mixed-record native scale gate counts now satisfy the task: `recordCount >= 2000` and `recordTypeCount >= 4`.

Remaining blockers are outside this task's owned files:

- `visual/hero-visual-briefs.manifest.json` still lists `record-count-below-hero-minimum` and `record-type-count-below-hero-minimum` for `mixed-record-hero`, so `check-native-hero-evidence-scale.mjs` reports stale blocker issues.
- `visual-composition-not-human-accepted` remains in the manifest. Browser visual regression passed, but that is not the same as human screenshot acceptance.
