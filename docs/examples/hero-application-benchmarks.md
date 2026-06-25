<!--
  Generated, byte-stable artifact. Regenerate with:
    cmake --build build/core --target engine_hero_application_benchmarks
    ./build/core/examples/engine/engine_hero_application_benchmarks > docs/examples/hero-application-benchmarks.md
  Wall-clock timing is emitted on stderr only (machine-dependent; not part of this artifact). -->

# METRIC Hero Application Benchmarks

## Run Metadata

| Field | Value |
|---|---|
| Suite | hero application evidence suite |
| Source | examples/engine/hero_application_benchmarks.cpp |
| Build profile | core preset |
| Platform | portable deterministic fixture |
| Artifact | docs/examples/hero-application-benchmarks.md |
| Notes | deterministic correctness + work-reduction evidence; wall-clock reported on stderr only |

## Representation Cost

| Benchmark | Representation | Records | Distance evals | Cached | Dense slots | Memory bytes | Exactness | Materialization | Notes |
|---|---|---:|---:|---:|---:|---:|---|---|---|
| mixed records | distance_table | 8 | 64 | 64 | 64 | 1088 | exact | materialized | composed-record all-pairs cache |
| condition monitoring | distance_table | 14 | 196 | 196 | 196 | 3248 | exact | materialized | TWED cycle-curve all-pairs cache |
| cross-space dependency | distance_table | 12 | 144 | 144 | 144 | 2400 | exact | materialized | process-space distance matrix |
| metric mapping | distance_table | 16 | 256 | 256 | 256 | 4224 | exact | materialized | arc source-space all-pairs cache |

## Application Workflow Evidence

| Benchmark | Queries | Metric-space matches | Vector-comparison mismatches | Latent matches | Avg metric margin | Diagnostics |
|---|---:|---:|---:|---:|---:|---|
| mixed records | 4 | 4/4 | 4/4 | - | 0.358734 | composed-domain-metric-vs-numeric-vector |
| condition monitoring | 9 | 9/9 | 1/9 | - | 0.451708 | TWED-1NN-accuracy-vs-euclidean-window |
| cross-space dependency | 12 | 12/12 | 9/12 | - | - | MGC dependence score=1; raw nearest-value pairing correct=3/12 |
| metric mapping | 16 | 16/16 | 0/16 | 16/16 | - | diffusion-potential anchors: dense_evals=n^2; reproducible (identical coordinates on rerun) |

> Note on the metric-mapping row: `Vector-comparison mismatches 0/16` means the naive
> vector 1-NN baseline matched the metric-space neighbour on all 16 queries, i.e. on
> this fixture the headline 1-NN column shows **no** metric advantage — and the hero
> deliberately does **not** assert that parametric diffusion coordinates beats classical MDS on neighbour recall.
> The asserted advantage is structural: the derived mapping artifact is **invertible** and supports
> an **out-of-sample transform** (a parametric finite-metric-space map), which the
> non-parametric MDS baseline does not provide. Read this row as evidence of the
> parametric/invertible capability, not of a nearest-neighbour win.

## Performance Evidence

| Benchmark | Workload | Records | Shared ops | Naive evals | Materialized evals | Cache-miss evals | Reduction | Exact | Notes |
|---|---|---:|---:|---:|---:|---:|---:|---|---|
| mixed records | neighbour/relation/summary over composed gallery | 8 | 3 | 192 | 64 | 0 | 3 | yes | composed domain metric vs flat Euclidean projection |
| condition monitoring | neighbour/outlier/structure over cycle gallery | 14 | 3 | 588 | 196 | 0 | 3 | yes | TWED elastic metric vs Euclidean window vector |
| cross-space dependency | process geometry shared by MGC + readouts | 12 | 3 | 432 | 144 | 0 | 3 | yes | MGC dependence vs raw nearest-value pairing |
| metric mapping | source geometry shared by diffusion target + readouts | 16 | 3 | 768 | 256 | 0 | 3 | yes | diffusion-coordinate-geometry targets driven by the source metric |
