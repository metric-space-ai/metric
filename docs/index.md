# METRIC Docs

METRIC is a native C++ framework for finite metric-space computing.

The documentation starts with the mathematical object, then the C++ component
layout, then concrete metrics, examples, bindings, and release rules.

## Start Here

1. [Finite Metric Spaces](concepts/finite-metric-space.md)
2. [METRIC Vocabulary](concepts/metric-vocabulary.md)
3. [Metric Discovery](metrics/index.md)
4. [C++ API](api/cpp.md)
5. [Python Binding API](api/python.md)
6. [Stability Labels](stability.md)

## Engine

- [Engine Overview](engine/index.md)
- [Mental Model](engine/mental-model.md)
- [Metric Space](engine/metric-space.md)
- [Storage And Indexes](engine/representations.md)
- [Stats And Modify Components](engine/operators.md)
- [Implementation Choices](engine/strategies.md)
- [Mappings](engine/mappings.md)
- [Execution Policies](engine/runtime.md)
- [Migration](engine/migration.md)

## Concepts

- [Finite Metric Spaces](concepts/finite-metric-space.md)
- [METRIC Vocabulary](concepts/metric-vocabulary.md)
- [Metric Spaces](concepts/metric-space.md)
- [Metrics as Recoding Costs](concepts/metrics-as-recoding-costs.md)
- [Vector Space as a Special Case](concepts/vector-space-as-special-case.md)
- [Explicit Representations](concepts/explicit-representations.md)
- [Graph Representation Terminology](concepts/graph-representations.md)
- [Metric-Space Numerics Manifesto](manifesto.md)

## Metric Discovery

- [Metric Discovery](metrics/index.md)
- [True Metric Catalog](metrics/true-metric-catalog.md)
- [Metric Admission Rules](metrics/metric-admission.md)
- [Rejected Non-Metrics And Metric Variants](metrics/rejected-non-metrics.md)
- [Metric Quarantine Inventory](metrics/quarantine-inventory.md)

## APIs

- [C++ API](api/cpp.md)
- [Python Binding API](api/python.md)
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

## Application Workflows

- [String Edit Baseline](examples/string-edit-baseline.md)
- [Histogram Transport Baseline](examples/histogram-transport-baseline.md)
- [Process Curve External Gallery](examples/process-curve-external-gallery.md)
- [Distribution Image Recoding Baseline](examples/distribution-image-recoding-baseline.md)
- [Mixed Structured Record Baseline](examples/mixed-structured-record-baseline.md)
- [Mixed Finite Metric Records (Hero)](examples/mixed-finite-records-hero.md)
- [Cross-Space Dependency (Hero)](examples/cross-space-dependency.md)
- [Cross-Space Dependency Baseline](examples/cross-space-dependency-baseline.md)
- [PHATE-AE Pipeline Workflow](examples/phate-ae-pipeline-workflow.md)
- [Entropy Diagnostics](examples/entropy-diagnostics.md)
- [Industrial Anomaly Workflow](examples/industrial-anomaly-workflow.md)
- [Engine Benchmark Report](examples/engine-benchmark-report.md)
- [Hero Application Benchmarks](examples/hero-application-benchmarks.md)
- [Benchmark Report Scaffold](examples/benchmark-report-scaffold.md)

## GitHub Pages

- Project page: <https://metric-space-ai.github.io/metric/>
- Technical documentation: <https://metric-space-ai.github.io/metric/docs.html>
