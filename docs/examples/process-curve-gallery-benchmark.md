# Process Curve Gallery Benchmark

This report records the larger process-curve hero fixture used for gallery and
benchmark evidence outside the minimal smoke story.

Command:

```bash
build/core/examples/engine/engine_process_curves_space
```

Gallery fixture:

- 15 short process-curve records
- six delayed-ramp query curves
- source families: `normal_reference`, `flat_hold`, `late_ramp`, `spike`, and
  `early_ramp`
- METRIC path: alignment-cost metric over process sequences
- baseline path: padded point-vector Euclidean nearest-neighbor search

Expected gallery output:

```text
process gallery records = 15
process gallery queries = 6
process gallery metric correct = 6/6
process gallery vector mismatches = 6/6
process gallery average metric margin = 2.33333
process gallery dense evaluations = 225
process gallery graph edges = 60
```

Interpretation:

- the alignment metric selects the expected `late_ramp` family for every query
- the padded vector baseline misses the expected family for every query
- dense materialization cost is explicit: 15 x 15 distance evaluations
- the graph diagnostic records 60 threshold edges at distance `<= 3.0`
- this is still a deterministic repository fixture, not an external
  publication dataset
- the licensed external gallery slices are tracked separately in
  [Process Curve External Gallery](process-curve-external-gallery.md)

Next evidence needed:

- preserve record IDs from raw trace to query result
- add a third licensed external domain only if it adds a materially different
  metric story
- keep publication screenshots showing metric winners and vector-baseline misses
  synchronized with executable output
