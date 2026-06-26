# External Process-Curve Scale Exporter Report

Date: 2026-06-26
Branch: `codex/visual-engine-evidence-exporters`

Status: blocked on real source evidence. No exporter, CSV asset, generated
visual asset, Visual Runtime, project page or GRAE10 file was changed.

## Task

Task contract:

```text
docs/visual/agent-tasks/process-curve-external-scale-exporter.md
```

The target was to scale `process-curve-external-hero` to at least 500 real
source windows while keeping aligned metric, vector baseline, nearest-neighbor
winner, margin and mismatch computations in native C++.

## Changed Files

- `docs/visual/reports/process-curve-external-scale-exporter.md`

## Current Evidence In Repo

The exporter reads only these checked-in UCR-derived CSV slices:

```text
examples/engine/assets/process_curve_power_demand_gallery.csv
examples/engine/assets/process_curve_internal_bleeding_gallery.csv
```

Line counts:

```text
25 examples/engine/assets/process_curve_power_demand_gallery.csv
25 examples/engine/assets/process_curve_internal_bleeding_gallery.csv
```

Each file contains one header row and 24 source windows. The checked-in source
evidence therefore contains 48 real source windows total. The current
`metric.visual.v1` asset contains 64 records total because it includes 16 query
records in addition to the 48 source windows. The source-source aligned metric
relation and landmark coordinate state cover 48 source records, not 500.

The license sidecar files explicitly state that these are small derived gallery
slices and that the full 94 MB UCR archive is intentionally not checked into
the repository:

```text
examples/engine/assets/process_curve_power_demand_gallery_license.md
examples/engine/assets/process_curve_internal_bleeding_gallery_license.md
```

## Final Counts

From `docs/examples/assets/process-curve-external/metric.visual.json`:

- total records: 64
- source records: 48
- query records: 16
- relations: 1
- source-source relation record ids: 48
- source-source relation pair values: 2304
- coordinate states: 1
- coordinate positions: 48
- graphs: 1
- graph edges: 192
- properties: 11
- diagnostics: 1

Native diagnostic payload:

- `source_record_count`: 48
- `query_record_count`: 16
- `metric_correct`: 16
- `vector_baseline_mismatches`: 16
- `power_demand`: 24 records, 8 queries
- `internal_bleeding`: 24 records, 8 queries

## Decision

Do not scale this exporter by duplicating windows, synthesizing extra curves,
padding queries into the source relation, or adding JavaScript/Python
algorithmic computation. That would violate the task contract and create fake
public evidence.

Exact blocker:

```text
missing-real-source-windows-for-500-record-hero
```

## Minimum Evidence Needed

To unblock this hero, add at least 500 licensed real source windows derived
from the cited UCR Time Series Anomaly Detection datasets, or another properly
licensed real process-curve corpus.

The expanded source must include:

- source file identifier
- source start/end indices
- source role label
- original time-series values
- enough held-out or downsampled query windows to prove metric winner vs vector
  baseline mismatch

After those files exist, update:

```text
examples/engine/process_curve_external_visual_export.cpp
```

The exporter must continue computing in native C++:

- aligned curve metric distances
- padded/vector baseline distances
- nearest-neighbor winner selection
- metric margin
- correctness and mismatch flags
- dense source relation
- kNN graph
- landmark coordinates
- summary diagnostics

If query records remain outside the source-source relation, add explicit native
query-to-source assignment relation or graph evidence.

## Commands Run

```bash
cmake --build build/core --target engine_process_curve_external_visual_export -- -j4
```

Result: passed. Built `engine_process_curve_external_visual_export`; linker
reported only the existing duplicate-library warning for `-ldl` and `-lm`.

```bash
ctest --test-dir build/core -R 'visual_(export|validate)' --output-on-failure
```

Result: passed, 18/18 tests.

```bash
node visual/tools/check-visual-document.mjs docs/examples/assets/process-curve-external/metric.visual.json
```

Result: passed. Reported native `metric.visual.v1`, `synthetic: false`, 64
records, 1 relation, 1 coordinate, 11 properties and 1 diagnostic.

```bash
node visual/tools/check-native-hero-evidence-scale.mjs
```

Result: passed globally with `ok: true`; `nativeScaleReadyCount: 6`.
`process-curve-external-hero` remains `nativeScaleReady: false` with
`recordCount: 48`, `totalRecordCount: 64`, `scaleRecordCountSource:
metric-relation-record-ids`, `minRecordCountForHero: 500`, and blockers
`record-count-below-hero-minimum` plus `visual-composition-not-human-accepted`.

```bash
node visual/tools/check-public-gallery-evidence.mjs
```

Result: passed with `ok: true`. GRAE10 hash remained
`464f6a90c36c1e9c6b4ec90068500dc226740d65b251918aca567f99d64d3d5e`.

```bash
node visual/tools/check-visual-regression-public-examples.mjs
```

Result: passed with `ok: true`, 8 total pages and 0 failures.
`process-curve-external-hero` loaded native evidence, rendered nonblank, and
remained categorized as `public-preview-only`.

## Acceptance State

`process-curve-external-hero` remains record-count-blocked. The existing native
evidence is real and useful as a preview, but it is not sufficient for a
500-source-record accepted hero. Visual composition also still needs human
screenshot review before any hero acceptance.
