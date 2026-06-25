# Structured Data

Structured records often have a more meaningful native distance than a vector embedding. METRIC makes that distance explicit and computes directly on the resulting metric space.

Promoted examples that currently run in the core CI path:

- [string_edit_space.py](../../python/examples/metric_space/string_edit_space.py): Python strings compared with edit distance
- [structured_record_space.py](../../python/examples/metric_space/structured_record_space.py): Python structured records compared with a domain metric callable
- [time_series_alignment_space.py](../../python/examples/metric_space/time_series_alignment_space.py): Python process curves compared with an alignment-aware callable
- [histogram_transport_space.py](../../python/examples/metric_space/histogram_transport_space.py): Python histograms compared with a transport callable
- [customer_focus_groups_metric_space.py](../../python/examples/engine/customer_focus_groups_metric_space.py): Python customer focus groups over a composed metric, with real medoid personas and a vector-first centroid contrast
- [metric_space_strings.cpp](../../examples/core/metric_space_strings.cpp): strings compared with edit distance
- [custom_metric_space.cpp](../../examples/core/custom_metric_space.cpp): strings compared with a custom padded Hamming metric
- [time_series_twed_space.cpp](../../examples/core/time_series_twed_space.cpp): process curves compared with TWED
- [histogram_emd_space.cpp](../../examples/core/histogram_emd_space.cpp): histograms compared with Earth mover distance
- [mixed_structured_records.cpp](../../examples/engine/mixed_structured_records.cpp): C++ engine demo for mixed industrial records with a composed domain metric, matrix-cache runtime diagnostics, grouping, and outlier detection
- [explicit_space_representations.cpp](../../examples/core/explicit_space_representations.cpp): one finite metric space represented as matrix, tree, and graph structures
- [metric_space_intrinsic_dimension.cpp](../../examples/core/metric_space_intrinsic_dimension.cpp): expansion-dimension diagnostic over a finite line metric
- [metric_space_correlation.cpp](../../examples/core/metric_space_correlation.cpp): MGC correlation between paired records with different metric types

Low-level estimator regression coverage, not a framework pipeline example:

- [metric_space_entropy.cpp](../../examples/core/metric_space_entropy.cpp): direct kpN estimator regression over numeric and string records. Framework-level entropy examples should start from coordinates or an explicit mapping result.

## Pattern

1. Choose the record type.
2. Choose or implement the metric.
3. Build an implicit or explicit finite metric space.
4. Run compatible operators such as nearest-neighbor search, intrinsic dimension, correlation, graph construction, or mapped-coordinate entropy.
5. Promote only examples that run in CI as stable examples.

For a concrete custom callable example, see [Custom Metric Example](custom-metric.md).
