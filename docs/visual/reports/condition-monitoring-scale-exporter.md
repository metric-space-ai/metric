# Condition Monitoring Scale Exporter Report

Date: 2026-06-25
Branch: `codex/visual-engine-evidence-exporters`

Status note, 2026-06-26: this is a historical exporter execution report. The
current authoritative scale/gate state is
`docs/visual/reports/native-hero-scale-evidence-upgrade-plan.md` and
`docs/visual/reports/visual-regression-performance-workstream.md`. The stale
record-count blocker described below has since been removed from the active
Visual Brief state; the remaining blocker is screenshot/composition acceptance.

## Changed Files

- `examples/engine/condition_monitoring_visual_export.cpp`
- `docs/examples/assets/condition-monitoring/metric.visual.json`
- `docs/visual/reports/condition-monitoring-scale-exporter.md`

## Final Evidence Counts

- Records: 528 native process-window records.
- Truth/run phases: 128 healthy baseline, 180 slow drift, 4 abrupt fault, 112 recovery, 104 recurring signature.
- Relations: 2.
  - `condition-monitoring-twed`: 278,784 native TWED values over the 528-record finite metric space.
  - `condition-monitoring-transition`: 527 ordered process-window transition edges.
- Graphs: 1 `process-window-trajectory` graph with 527 edges.
- Coordinates: 2 coordinate states, each with 528 positions.
  - `landmark-3d`
  - `process-state-trajectory-3d`
- Properties: 10 record properties, each with 528 values.
- Timelines: 1 `condition-evolution` timeline with 528 native state steps.
- Diagnostics: 2.
  - `native-reference-checks`
  - `metric-law-check`

## Native Computation Sources

- TWED process-window relation: C++ `process_metric()` and the exporter distance matrix loop.
- Transition/path relation: C++ ordered process-window sequence in `build_condition_monitoring_document()`.
- Nearest healthy score: C++ `distance_to_healthy()`.
- Nearest catalog score/regime: C++ `classify()` using `mtrc::find_neighbors()`.
- Density property: C++ `local_density()` over native TWED distances.
- Anomaly/severity score: C++ thresholding of nearest-healthy TWED distance against native healthy radius.
- Outlier flags: C++ `mtrc::find_outliers()` on the monitored TWED space.
- Landmark coordinates: C++ `landmark_coordinates()` from native TWED distances.
- Process trajectory coordinates/timeline states: C++ exporter state derived from record order, native severity, truth regime and phase.
- Metric-law diagnostics: C++ `metric_law_diagnostics()` over the native TWED matrix.
- Record previews: every record keeps a native `time_series` payload with `sample_rate_hz`, `truth` and `run_phase`.

No JavaScript, Python or page code computes METRIC values in this change.

## Commands Run

```bash
cmake --build build/core --target engine_condition_monitoring_visual_export -- -j4
build/core/examples/engine/engine_condition_monitoring_visual_export --export-dir docs/examples/assets/condition-monitoring
ctest --test-dir build/core -R 'visual_(export|validate)' --output-on-failure
node visual/tools/check-visual-document.mjs docs/examples/assets/condition-monitoring/metric.visual.json
node visual/tools/check-native-hero-evidence-scale.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-visual-regression-public-examples.mjs
```

Results:

- Build: passed.
- Public asset regeneration: passed.
- CTest visual export/validate selection: passed, 14/14 tests.
- `check-visual-document`: passed for `docs/examples/assets/condition-monitoring/metric.visual.json`.
- `check-public-gallery-evidence`: passed.
- `check-visual-regression-public-examples`: passed, 8/8 examples, condition-monitoring render nonblank with native evidence.
- `check-native-hero-evidence-scale`: command exited 1. The condition-monitoring row is `nativeScaleReady: true` with 528 records and 2 relations, but the out-of-scope `visual/hero-visual-briefs.manifest.json` still lists `record-count-below-hero-minimum`, so the gate reports `stale-record-count-blocker`.

## Remaining Blocker

The native condition-monitoring evidence now meets the scale threshold. The remaining blocker is manifest/review state, not C++ evidence generation:

- `condition-monitoring-hero` still carries the stale `record-count-below-hero-minimum` acceptance blocker in `visual/hero-visual-briefs.manifest.json`.
- `visual-composition-not-human-accepted` remains until human screenshot review accepts the preview composition.

The manifest is outside this task's owner scope, so it was not edited.
