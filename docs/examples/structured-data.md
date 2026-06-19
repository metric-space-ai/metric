# Structured Data

Structured records often have a more meaningful native distance than a vector embedding. METRIC makes that distance explicit and computes directly on the resulting metric space.

Promoted examples that currently run in the core CI path:

- [metric_space_strings.cpp](../../examples/core/metric_space_strings.cpp): strings compared with edit distance
- [custom_metric_space.cpp](../../examples/core/custom_metric_space.cpp): strings compared with a custom padded Hamming metric
- [time_series_twed_space.cpp](../../examples/core/time_series_twed_space.cpp): process curves compared with TWED
- [histogram_emd_space.cpp](../../examples/core/histogram_emd_space.cpp): histograms compared with Earth mover distance
- [explicit_space_representations.cpp](../../examples/core/explicit_space_representations.cpp): one finite metric space represented as matrix, tree, and graph structures
- [metric_space_entropy.cpp](../../examples/core/metric_space_entropy.cpp): entropy diagnostics over numeric and string records
- [metric_space_correlation.cpp](../../examples/core/metric_space_correlation.cpp): MGC correlation between paired records with different metric types

## Pattern

1. Choose the record type.
2. Choose or implement the metric.
3. Build an implicit or explicit finite metric space.
4. Run operators such as nearest-neighbor search, entropy, correlation, or graph construction.
5. Promote only examples that run in CI as stable examples.

For a concrete custom callable example, see [Custom Metric Example](custom-metric.md).
