# Implementation Choices

METRIC does not treat strategy names as the framework language. Strategy or
algorithm names are Level-3 implementation choices inside a Level-2 component.

Examples:

- `mtrc::stats::search::brute_force`
- `mtrc::stats::search::distance_table`
- `mtrc::stats::search::cover_tree`
- `mtrc::stats::search::knn_graph`
- `mtrc::stats::structural_analysis::k_medoids_options`
- `mtrc::stats::structural_analysis::dbscan_options`
- `mtrc::stats::structural_analysis::affinity_propagation_options`
- `mtrc::stats::correlate::mgc_options`
- `mtrc::space::select::farthest_first`

Use an implementation choice only after the metric-space question is already
clear. For example, MGC is a concrete dependence test between paired finite
metric spaces. It is not a metric.
