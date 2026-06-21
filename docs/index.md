# METRIC Docs

METRIC is a native C++ engine for finite metric spaces.

This documentation covers the product concepts, C++ API, bindings, examples, stability labels, and release-facing build rules. The public GitHub Pages site is checked in under `docs/site/`.

## Start Here

1. [Finite Metric Spaces](concepts/finite-metric-space.md)
2. [Metric Spaces](concepts/metric-space.md)
3. [Vector Space as a Special Case](concepts/vector-space-as-special-case.md)
4. [Engine Overview](engine/index.md)
5. [C++ API](api/cpp.md)
6. [Stability Labels](stability.md)

## Engine

- [Engine Overview](engine/index.md)
- [Metric Space](engine/metric-space.md)
- [Representations](engine/representations.md)
- [Intent API](engine/intent-api.md)
- [Strategies](engine/strategies.md)
- [Operators](engine/operators.md)
- [Mappings](engine/mappings.md)
- [Runtime Policies](engine/runtime.md)
- [Migration](engine/migration.md)

## Concepts

- [Finite Metric Spaces](concepts/finite-metric-space.md)
- [Metric Spaces](concepts/metric-space.md)
- [Metrics as Recoding Costs](concepts/metrics-as-recoding-costs.md)
- [Vector Space as a Special Case](concepts/vector-space-as-special-case.md)
- [Explicit Representations](concepts/explicit-representations.md)
- [Graph Representation Terminology](concepts/graph-representations.md)
- [Metric-Space Numerics Manifesto](manifesto.md)

## APIs

- [C++ API](api/cpp.md)
- [Python Binding API](api/python.md)
- [Stability Labels](stability.md)
- [Testing and CI Scope](testing-and-ci.md)
- [Release Checklist](release-checklist.md)

## Python Binding

Python is a binding and adapter layer over the native engine surface.

- [Space Constructor And Records](python/space.md)
- [Intent Methods](python/intents.md)
- [Result Objects](python/results.md)
- [Expert Strategy Overrides](python/strategies.md)
- [Errors And Troubleshooting](python/errors.md)
- [Python API Reference](api/python.md)

## Hero Examples

- [Engine Demo Expected Outputs](examples/engine-demo-outputs.md)
- [String Edit Baseline](examples/string-edit-baseline.md)
- [Histogram Transport Baseline](examples/histogram-transport-baseline.md)
- [Process Curve External Gallery](examples/process-curve-external-gallery.md)
- [Process Curve Gallery Benchmark](examples/process-curve-gallery-benchmark.md)
- [Process Curve PHATE Map](examples/process-curve-phate-map.md)
- [Process Curve PHATE Gallery](examples/process-curve-phate-gallery.md)
- [Distribution Image Recoding Baseline](examples/distribution-image-recoding-baseline.md)
- [Mixed Structured Record Baseline](examples/mixed-structured-record-baseline.md)
- [Cross-Space Dependency Baseline](examples/cross-space-dependency-baseline.md)
- [PHATE-AE Pipeline Hero](examples/phate-ae-pipeline-hero.md)
- [Engine Benchmark Report](examples/engine-benchmark-report.md)

## Additional Examples

- [Custom Metric Example](examples/custom-metric.md)
- [Python Custom Metric](examples/python-custom-metric.md)
- [Python Real-Data Records](examples/python-real-data.md)
- [Python Compare Spaces](examples/python-compare-spaces.md)
- [Structured Data](examples/structured-data.md)
- [Time-Series Space With TWED](examples/time-series-twed.md)
- [Histogram Space With EMD](examples/histogram-emd.md)
- [Exact Graph Edge Fixtures](examples/graph-construction.md)
- [Representative Selection](examples/representative-selection.md)
- [Entropy Diagnostics](examples/entropy-diagnostics.md)
- [Intrinsic Dimension Diagnostic](examples/intrinsic-dimension.md)
- [Correlation Between Metric Spaces](examples/correlation-between-spaces.md)
- [Industrial Anomaly Workflow](examples/industrial-anomaly-workflow.md)

## GitHub Pages

- Project page: <https://metric-space-ai.github.io/metric/>
- Technical documentation: <https://metric-space-ai.github.io/metric/docs.html>
