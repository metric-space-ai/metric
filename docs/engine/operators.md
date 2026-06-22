# Stats And Modify Components

The old word "operator" is too generic for METRIC's public language. New code
is organized by what happens to the finite metric space.

## Stats

`mtrc::stats` investigates an existing space without changing its source
records or metric.

- `mtrc::stats::search`: neighbor and range queries
- `mtrc::stats::sample`: sampling and metric-space walks
- `mtrc::stats::properties`: entropy, density, intrinsic dimension, summaries
- `mtrc::stats::correlate`: dependence or correlation between paired spaces
- `mtrc::stats::structural_analysis`: groups, outliers, representatives

Entropy is a `stats::properties` computation over one finite metric space. MGC
is a `stats::correlate` implementation for dependence between two aligned
finite metric spaces. Neither is a record metric.

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
