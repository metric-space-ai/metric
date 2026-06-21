# Engine Benchmark Report

This is the first native C++ benchmark report artifact for the production-ready
track.

Command:

```bash
build/core/examples/engine/engine_benchmark_report
```

Generated report:

```markdown
# METRIC Engine Benchmark Report

## Run Metadata

| Field | Value |
|---|---|
| Suite | engine hero baseline suite |
| Source | examples/engine/benchmark_report.cpp |
| Build profile | core preset |
| Platform | portable deterministic fixture |
| Artifact | docs/examples/engine-benchmark-report.md |
| Notes | not a timing benchmark |

## Representation Cost

| Benchmark | Representation | Records | Distance evals | Cached | Dense slots | Memory bytes | Exactness | Materialization | Notes |
|---|---|---:|---:|---:|---:|---:|---|---|---|
| string edit baseline | matrix_cache | 12 | 144 | 144 | 144 | 1248 | exact | materialized | edit-distance all-pairs cache |
| histogram transport baseline | matrix_cache | 8 | 64 | 64 | 64 | 1088 | exact | materialized | cumulative-transport all-pairs cache |
| process curve PHATE gallery | matrix_cache | 15 | 225 | 225 | 225 | 3720 | exact | materialized | aligned-curve PHATE target distance cache |
| process curve PHATE gallery | knn_graph_index | 15 | 210 | 60 | 0 | 2160 | approximate | materialized | k=4 sparse neighbor index for gallery inspection |
| mixed structured record baseline | matrix_cache | 8 | 64 | 64 | 64 | 1088 | exact | materialized | composed mixed-record all-pairs cache |
| distribution image recoding baseline | matrix_cache | 8 | 64 | 64 | 64 | 1088 | exact | materialized | grid-transport all-pairs cache |
| distribution image recoding baseline | graph_topology | 8 | 0 | 2 | 0 | 112 | approximate | topology | threshold graph over transport geometry |
| cross-space MGC dependency baseline | matrix_cache | 12 | 144 | 144 | 144 | 2400 | exact | materialized | process-space distance matrix |
| cross-space MGC dependency baseline | matrix_cache | 12 | 144 | 144 | 144 | 1248 | exact | materialized | quality-space distance matrix |

## Hero Quality

| Benchmark | Queries | Metric hits | Vector misses | Latent hits | Avg metric margin | Diagnostics |
|---|---:|---:|---:|---:|---:|---|
| string edit baseline | 4 | 4/4 | 4/4 | - | 2 | edit-distance-vs-anagram-vector |
| histogram transport baseline | 4 | 4/4 | 4/4 | - | 3.25 | transport-vs-coordinate-vector |
| process curve PHATE gallery | 6 | 6/6 | 6/6 | 6/6 | 2.16667 | native PHATE-AE OOS: avg rank 1.33333; max rank 3; avg penalty 0.166667; max penalty 1 |
| mixed structured record baseline | 4 | 4/4 | 4/4 | - | 0.358734 | composed-domain-metric-vs-numeric-vector |
| distribution image recoding baseline | 4 | 4/4 | 4/4 | - | 0.416667 | transport-recoding-vs-flat-pixel-vector |
| cross-space MGC dependency baseline | 12 | 12/12 | 9/12 | - | - | MGC dependency score=1; raw-vector pairing correct=3/12 |
```

Validation scope:

- the generator recomputes the string edit-distance, histogram transport,
  mixed-record, distribution recoding, and cross-space dependency comparisons
  natively in C++
- the process-curve gallery representation rows are computed from the native
  aligned-curve source metric space
- the mixed-record and distribution/image rows assert that domain metrics beat
  flat vector projections on all promoted queries
- the cross-space MGC row asserts native metric-space dependency detection while
  raw value pairing misses 9 of 12 records
- the PHATE-AE latent and OOS diagnostic values are the report snapshot for the
  separately asserted `engine_process_curve_phate_gallery` C++ smoke fixture
- `example_engine_benchmark_report` runs the generator under CTest
- the manual `benchmark.yml` workflow renders the same report with
  runner/build/commit metadata and uploads it as
  `metric-engine-benchmark-report.md`

This report is not a timing benchmark. It is a deterministic quality and
representation-cost artifact for the promoted hero fixtures.
