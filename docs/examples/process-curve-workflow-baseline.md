# Process Curve Workflow Baseline

This report records the first CI-safe non-vector workflow fixture for METRIC
production readiness.

Command:

```bash
build/core/examples/engine/engine_process_curves_space
```

Fixture:

- source records are short process curves with shifted, flat, spike, and early
  ramp regimes
- query is a delayed ramp process curve
- METRIC path uses an alignment-cost metric over sequences
- baseline path treats the same records as padded point vectors and uses
  pointwise Euclidean distance

Expected output:

```text
process workflow metric winner = late_ramp at 2
process workflow vector comparison winner = flat_hold at 3
process workflow baseline mismatch = yes
process workflow metric margin = 3
process workflow groups = 2
process workflow outliers = 2
process workflow dense evaluations = 25
process curve graph edges = 4
process benchmark records = 10
process benchmark queries = 4
process benchmark metric correct = 4/4
process benchmark vector mismatches = 4/4
process benchmark average metric margin = 3
process benchmark dense evaluations = 100
process benchmark graph edges = 24
process gallery records = 15
process gallery queries = 6
process gallery metric correct = 6/6
process gallery vector mismatches = 6/6
process gallery average metric margin = 2.33333
process gallery dense evaluations = 225
process gallery graph edges = 60
```

Interpretation:

- the alignment metric identifies `late_ramp` as the nearest process state
- the point-vector comparison incorrectly selects `flat_hold`
- the fixture records dense distance materialization cost for the current
  five-record smoke case
- the richer benchmark fixture repeats the comparison over four delayed-ramp
  queries and ten records, where the alignment metric selects the expected
  process family in all cases and the padded vector comparison misses it in all
  cases
- the gallery fixture extends the same story to 15 records and six delayed-ramp
  queries; it remains deterministic and records materialization and graph
  diagnostics for report reuse
- the licensed external gallery slices and executable benchmark are tracked in
  [Process Curve External Gallery](process-curve-external-gallery.md), covering
  PowerDemand and InternalBleeding with the same metric-vs-vector comparison
  comparison
- the fitted mapping variant is tracked in
  [Process Curve PHATE Map](process-curve-phate-map.md), where the same
  non-vector source shape feeds a native PHATE-AE pipeline through an explicit
  `feature_record_codec`
