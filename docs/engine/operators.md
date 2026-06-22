# Stats And Modify Components

The old word "operator" is too generic for METRIC's public language. New code
is organized by what happens to the finite metric space.

## Stats

`mtrc::stats` investigates an existing space without changing its source
records or metric. Search, sample, properties, correlate, and structural
analysis are Level-2 user workflows over one (or two paired) finite metric
spaces. Vector records are one special case; every workflow is computed from
metric values, not coordinates.

- `mtrc::stats::search`: neighbor, range, and batch queries
- `mtrc::stats::sample`: deterministic sampling and metric-space walks
- `mtrc::stats::properties`: `profile`, distance distribution, entropy, density,
  intrinsic dimension, local volume, summaries
- `mtrc::stats::correlate`: dependence/correlation between paired spaces and its
  permutation significance
- `mtrc::stats::structural_analysis`: groups, outliers, representatives, and
  cluster validity diagnostics

`stats::properties::profile` is a cohesive diagnostic summary of one space, with
optional distance-distribution and local-volume sections computed only on
request. Entropy is a `stats::properties` computation over one finite metric
space; its result reports an explicit success/failure status and a valid
negative differential entropy is not a failure. Intrinsic dimension is a
finite-space growth diagnostic, not a manifold-dimension guarantee. MGC is a
`stats::correlate` implementation for dependence between two aligned finite
metric spaces -- a statistic in `[-1, 1]`, never a record metric -- and
`mgc_significance` is its seeded permutation test.

## Modify

`mtrc::modify` constructs a changed or derived finite metric space.

- `mtrc::modify::represent`: representative spaces
- `mtrc::modify::reduce`: thinning or coarsening finite spaces
- `mtrc::modify::expand`: domain-supported interpolation or expansion
- `mtrc::modify::resample`: sampling correction and denoise-style workflows
- `mtrc::modify::map`: maps into derived spaces, including coordinate spaces
- `mtrc::modify::dynamics`: diffusion and evolution over a finite space
- `mtrc::modify::compose`: inspectable composed workflows

Concrete implementations such as MGC, DBSCAN, PCFA, or PHATE-AE are named after
the owning component is clear.
