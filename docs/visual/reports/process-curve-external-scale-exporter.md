# External Process-Curve Scale Exporter Report

Date: 2026-06-26
Branch: `main`

Status: source-scale blocker cleared. The external UCR process-curve visual
evidence now contains more than 500 real source windows in the native
source-source metric relation. It remains review-pending for visual composition.

## Task

Task contract:

```text
docs/visual/agent-tasks/process-curve-external-scale-exporter.md
```

The target was to scale `process-curve-external-hero` to at least 500 real
source windows while keeping aligned metric, vector baseline, nearest-neighbor
winner, margin and mismatch computations in native C++.

## Changed Files

- `examples/engine/assets/process_curve_power_demand_gallery.csv`
- `examples/engine/assets/process_curve_internal_bleeding_gallery.csv`
- `examples/engine/assets/process_curve_power_demand_gallery_license.md`
- `examples/engine/assets/process_curve_internal_bleeding_gallery_license.md`
- `examples/engine/process_curve_external_gallery.cpp`
- `examples/engine/process_curve_external_visual_export.cpp`
- `docs/examples/assets/process-curve-external/metric.visual.json`
- `visual/hero-visual-briefs.manifest.json`
- `visual/tools/check-process-curve-external-visual.mjs`
- `docs/visual/reports/process-curve-external-scale-exporter.md`

## Real Data Sources Used

The derived CSV windows are extracted from the UCR Time Series Anomaly Detection
datasets (2021), using the direct archive:

```text
https://www.cs.ucr.edu/~eamonn/time_series_data_2018/UCR_TimeSeriesAnomalyDatasets2021.zip
```

Archive hashes observed during derivation:

```text
SHA-256 ac4b991c701e620ae9cc5ebd57ae45593a36cc9c0b6ed5e3c4b7e466cf4783d4
MD5     11d539351d37c6fe3013e828ab89574a
```

The expanded checked-in CSV slices use:

- all 8 `*PowerDemand*.txt` source members
- all 28 `*InternalBleeding*.txt` source members
- 16 deterministic 36-sample windows per source file

The raw UCR archive is not checked into the repository.

## Final Counts

From `docs/examples/assets/process-curve-external/metric.visual.json`:

- total records: 737
- source records: 576
- query records: 161
- relations: 1
- source-source relation record ids: 576
- source-source relation pair values: 331,776
- coordinate states: 1
- coordinate positions: 576
- graphs: 1
- graph edges: 2,304
- properties: 11
- diagnostics: 1

Native diagnostic payload:

- `source_record_count`: 576
- `query_candidate_count`: 162
- `query_record_count`: 161
- `metric_correct`: 161
- `vector_baseline_mismatches`: 161
- `power_demand`: 128 records, 40 contrast queries
- `internal_bleeding`: 448 records, 121 contrast queries

## Native Computation Contract

The exporter keeps the following computation in C++:

- aligned curve metric distances
- padded/vector baseline distances
- nearest-neighbor winner selection
- metric margin
- correctness and mismatch flags
- dense source relation
- kNN graph
- landmark coordinates
- summary diagnostics

The JavaScript visual runtime only validates, indexes, renders and previews the
exported evidence.

## Encoding Change

The source-source relation remains `storage: "dense_matrix"` but now exports
`values` as a flat row-major numeric array instead of object-form pair entries.
This is already valid under `metric.visual.v1` and avoids repeating row/column
record ids 331,776 times.

The visual runtime now has an explicit dense-matrix reader for flat row-major
and row-array dense relation values. Matrix rendering, pair picking and
symmetry diagnostics therefore read the exported dense relation directly
instead of trying to interpret numeric cells as legacy pair objects.

## Commands Run

```bash
cmake --build build/core --target engine_process_curve_external_visual_export -- -j4
```

Result: passed.

```bash
build/core/examples/engine/engine_process_curve_external_visual_export \
  --export-json docs/examples/assets/process-curve-external/metric.visual.json
```

Result: passed. Wrote 576 source records, 161 query records and 331,776 source
relation pairs.

```bash
cmake --build build/core --target engine_process_curve_external_gallery engine_process_curve_external_visual_export -- -j4
```

Result: passed.

```bash
node visual/tools/check-visual-document.mjs docs/examples/assets/process-curve-external/metric.visual.json
```

Result: passed.

```bash
node visual/tools/check-process-curve-external-visual.mjs
```

Result: passed with 576 source records and 161 query records.

```bash
METRIC_VISUAL_SCREENSHOT_TIMEOUT_MS=6000 \
  node visual/tools/check-visual-regression-public-examples.mjs
```

Result: passed for all 8 public examples. Render proof uses nonblank canvas PNG
pixels; full-page CDP screenshots may time out in headless WebGL and are not
used as the render proof when canvas capture succeeds.

```bash
node visual/tools/check-relation-matrix-picker.mjs
```

Result: passed, including the flat row-major dense-matrix contract.

```bash
node visual/tools/check-native-hero-evidence-scale.mjs
```

Result: passed globally with `nativeScaleReadyCount: 7`; no preview remains
blocked by record count.

```bash
ctest --test-dir build/core -R 'process_curve_external|visual_(export|validate)' --output-on-failure
```

Result: passed, 19/19 tests. An earlier run failed while the volume was full;
after removing generated `/tmp` and build/export artifacts, the same CTest
selection passed cleanly.

## Acceptance State

`process-curve-external-hero` is no longer record-count-blocked. It is still
not an accepted hero because `visual-composition-not-human-accepted` remains in
the public brief manifest. The next required work is screenshot review and, if
needed, reusable composition hardening in the process-curve view/engine layer.
