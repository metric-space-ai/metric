# Engine Benchmark Report

This is the native C++ benchmark report artifact for the production-ready track.
It is regenerated from the promoted generator and consolidates correctness,
representation-cost, and deterministic work-reduction evidence for the hero
applications.

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
| Suite | engine application workflow suite |
| Source | examples/engine/benchmark_report.cpp |
| Build profile | core preset |
| Platform | portable deterministic fixture |
| Artifact | docs/examples/engine-benchmark-report.md |
| Notes | not a timing benchmark |

## Representation Cost

| Benchmark | Representation | Records | Distance evals | Cached | Dense slots | Memory bytes | Exactness | Materialization | Notes |
|---|---|---:|---:|---:|---:|---:|---|---|---|
| string edit baseline | distance_table | 12 | 144 | 144 | 144 | 1248 | exact | materialized | edit-distance all-pairs cache |
| histogram transport baseline | distance_table | 8 | 64 | 64 | 64 | 1088 | exact | materialized | cumulative-transport all-pairs cache |
| process curve diffusion-coordinate gallery | distance_table | 15 | 225 | 225 | 225 | 3720 | exact | materialized | aligned-curve diffusion-coordinate target distance cache |
| process curve diffusion-coordinate gallery | knn_graph_index | 15 | 210 | 60 | 0 | 2160 | approximate | materialized | k=4 sparse neighbor index for gallery inspection |
| mixed structured record baseline | distance_table | 8 | 64 | 64 | 64 | 1088 | exact | materialized | composed mixed-record all-pairs cache |
| distribution image recoding baseline | distance_table | 8 | 64 | 64 | 64 | 1088 | exact | materialized | grid-transport all-pairs cache |
| distribution image recoding baseline | graph_topology | 8 | 0 | 2 | 0 | 112 | approximate | topology | threshold graph over transport geometry |
| cross-space MGC dependency baseline | distance_table | 12 | 144 | 144 | 144 | 2400 | exact | materialized | process-space distance matrix |
| cross-space MGC dependency baseline | distance_table | 12 | 144 | 144 | 144 | 1248 | exact | materialized | quality-space distance matrix |
| condition monitoring TWED baseline | distance_table | 11 | 121 | 121 | 121 | 2024 | exact | materialized | healthy+fault gallery all-pairs cache |

## Application Workflow Evidence

| Benchmark | Queries | Metric-space matches | Vector-comparison mismatches | Latent matches | Avg metric margin | Diagnostics |
|---|---:|---:|---:|---:|---:|---|
| string edit baseline | 4 | 4/4 | 4/4 | - | 2 | edit-distance-vs-anagram-vector |
| histogram transport baseline | 4 | 4/4 | 4/4 | - | 3.25 | transport-vs-coordinate-vector |
| process curve diffusion-coordinate gallery | 6 | 6/6 | 6/6 | 6/6 | 2.16667 | native parametric diffusion coordinate OOS: avg rank 1.33333; max rank 3; avg penalty 0.166667; max penalty 1 |
| mixed structured record baseline | 4 | 4/4 | 4/4 | - | 0.358734 | composed-domain-metric-vs-numeric-vector |
| distribution image recoding baseline | 4 | 4/4 | 4/4 | - | 0.416667 | transport-recoding-vs-flat-pixel-vector |
| cross-space MGC dependency baseline | 12 | 12/12 | 9/12 | - | - | MGC dependency score=1; raw-vector pairing correct=3/12 |
| condition monitoring TWED baseline | 10 | 10/10 | 2/10 | - | 5.18 | TWED novelty: faults 5/5 detected, 0 false alarms; padded-vector baseline hides 2 faults; DBSCAN isolates 5 vs 2 |

## Performance Evidence

| Benchmark | Workload | Records | Shared ops | Naive evals | Materialized evals | Cache-miss evals | Reduction | Exact | Notes |
|---|---|---:|---:|---:|---:|---:|---:|---|---|
| mixed structured record baseline | shared gallery operators | 8 | 4 | 224 | 64 | 0 | 3.5 | yes | composite domain metric reused across operators |
| condition monitoring TWED baseline | shared gallery operators | 11 | 4 | 440 | 121 | 0 | 3.63636 | yes | true TWED metric reused across operators |
| cross-space MGC dependency baseline | shared process-space operators | 12 | 4 | 528 | 144 | 0 | 3.66667 | yes | process distance matrix reused across operators |
| process curve diffusion-coordinate gallery | shared gallery operators | 15 | 4 | 840 | 225 | 0 | 3.73333 | yes | aligned-curve distance reused across operators |
```

Validation scope:

- the generator recomputes the string edit-distance, histogram transport,
  mixed-record, distribution recoding, cross-space dependency, and
  condition-monitoring comparisons natively in C++
- the process-curve gallery representation rows are computed from the native
  aligned-curve source metric space
- the mixed-record, distribution/image, and condition-monitoring rows assert
  that domain metrics beat flat vector projections on all promoted queries
- the condition-monitoring row asserts that a TWED novelty detector flags every
  injected fault with no false alarm, while a padded point-wise Euclidean
  baseline hides two faults inside its phase-inflated normal envelope; a DBSCAN
  outlier sweep over the combined gallery isolates 5 faults under TWED versus 2
  under the baseline
- the cross-space MGC row asserts native metric-space dependency detection while
  raw value pairing misses 9 of 12 records
- the parametric diffusion coordinate latent and OOS diagnostic values are the report snapshot for the
  separately asserted `engine_process_curve_diffusion_coordinate_gallery` C++ smoke fixture
- the **Performance Evidence** rows are measured with a shared-counter metric
  wrapper: a naive pipeline re-evaluates the domain metric for each of four
  distance-consuming operator passes over a hero gallery, while a materialized
  DistanceTable evaluates the pairwise structure once and serves every later pass
  from cache with zero additional metric evaluations (`Cache-miss evals = 0`),
  returning identical distances (`Exact = yes`). The reduction factor is the
  ratio of naive to materialized metric evaluations and is fully deterministic
- `example_engine_benchmark_report` runs the generator under CTest; the
  `benchmark_report_smoke` core gate independently asserts the Markdown shape of
  all four sections
- the manual `benchmark.yml` workflow renders the same report with
  runner/build/commit metadata and uploads it as
  `metric-engine-benchmark-report.md`

This report is not a wall-clock timing benchmark. It is a deterministic quality,
representation-cost, and work-reduction artifact for the promoted hero fixtures.
Optional indicative wall-clock timing is available out-of-band by setting
`METRIC_BENCHMARK_REPORT_TIMING=1`, which appends an `## Indicative Timing`
section (platform-specific, never asserted in CI) and is therefore omitted from
this checked-in artifact.
```
