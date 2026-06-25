# External Process-Curve Scale Exporter Report

Date: 2026-06-25
Branch: `codex/visual-engine-evidence-exporters`

Status: blocked on real source evidence. No exporter, runtime, project-page,
GRAE10 or generated asset files were changed.

## Task

Task contract:

```text
docs/visual/agent-tasks/process-curve-external-scale-exporter.md
```

The target was to scale `process-curve-external-hero` to at least 500 real
source windows while keeping the aligned metric, vector baseline, winner,
margin and mismatch computations in native C++.

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
`metric.visual.v1` asset reports 64 records because query records are included
in addition to the source windows. The metric relation and coordinate state are
over the source windows, not over 500 real source records.

The license sidecar files explicitly state that these are small derived gallery
slices and that the full 94 MB UCR archive is intentionally not checked into
the repository:

```text
examples/engine/assets/process_curve_power_demand_gallery_license.md
examples/engine/assets/process_curve_internal_bleeding_gallery_license.md
```

## Decision

Do not scale this exporter by duplicating windows, synthesizing extra curves or
counting query records as source evidence. That would violate the Visual Engine
plan and would create a fake public hero.

The correct blocker is:

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

## Commands Run

```bash
rg --files examples/engine/assets docs/examples/assets | rg 'process|curve|ucr|power|bleeding|csv$'
find examples/engine/assets -maxdepth 2 -type f -print | sort
wc -l examples/engine/assets/*.csv
sed -n '1,120p' examples/engine/assets/process_curve_power_demand_gallery_license.md
sed -n '1,120p' examples/engine/assets/process_curve_internal_bleeding_gallery_license.md
```

## Acceptance State

`process-curve-external-hero` must remain preview-only and blocked by record
count. The existing native evidence is real and useful as a preview, but it is
not sufficient for a 500-record accepted hero.
