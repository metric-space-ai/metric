# Stability Labels

This page defines the support meaning of the current revival surface.

## Stable Revival Surface

- C++ namespace: `mtrc`
- core object: `mtrc::MetricSpace<Record, Metric>`
- record identity: `mtrc::RecordId`
- metric catalog aggregate: `<metric/metric/catalog.hpp>`
- engine aggregate: `<metric/engine.hpp>`
- record aggregate: `<metric/record.hpp>`
- search entry point: `mtrc::stats::search::find_neighbors`
- storage/index components under `mtrc::space::storage`
- current Level-1 source homes under `metric/record`, `metric/space`,
  `metric/metric`, `metric/stats`, `metric/modify`, `metric/solve`, and
  `metric/numeric`

Stable means the component is expected to compile in the core C++ preset and is
safe for current examples, subject to ordinary API evolution before a 1.0
release.

## Admission-Gated Surface

Metrics are stable only when their domain, parameters, and metric-law status
are documented. Pairwise functions without an admitted true-metric contract
belong in quarantine and must not be advertised as normal metrics.

## Binding Surface

Python is an adapter layer. It may expose native C++ capabilities and convert
user data, but it must not implement metric math or solvers independently.

## Experimental Surface

Research workflows, mappings, native mapping artifacts, and hero applications can
exist in-tree before they are production-grade. Promotion requires:

- documented finite metric-space interpretation
- native C++ implementation
- deterministic tests or examples
- result metadata and diagnostics
- no non-metric function advertised as a metric
