# METRIC Docs

METRIC is a numerical computing engine for finite metric spaces.

This docs tree is the canonical home for concepts, API references, engine architecture, examples, and release rules. The root `README.md` stays short and product-facing. The public GitHub Pages site lives in `docs/site/`.

## Start Here

- [Engine Overview](engine/index.md)
- [Engine Metric Space](engine/metric-space.md)
- [Engine Intent API](engine/intent-api.md)
- [Engine Runtime Policies](engine/runtime.md)
- [Metric-Space Numerics Manifesto](manifesto.md)
- [API Surface](stability.md)
- [Testing and CI Scope](testing-and-ci.md)
- [Research Roadmap](research-roadmap.md)
- [Revival Status](revival-status.md)
- [C++ API](api/cpp.md)
- [Python API](api/python.md)
- [Python Space](python/space.md)
- [Python Intent Methods](python/intents.md)
- [Python Result Objects](python/results.md)
- [Finite Metric Spaces](concepts/finite-metric-space.md)
- [Vector Space as a Special Case](concepts/vector-space-as-special-case.md)

## Reading Order

1. Read the root [README.md](../README.md) for the product framing.
2. Read [Metric-Space Numerics Manifesto](manifesto.md) for the technical distinction from vector-search and embedding-only workflows.
3. Read [Engine Overview](engine/index.md) to understand the engine model.
4. Read [Engine Metric Space](engine/metric-space.md), [Representations](engine/representations.md), and [Intent API](engine/intent-api.md) for the engine-specific model.
5. Read [Metric Spaces](concepts/metric-space.md) and [Finite Metric Spaces](concepts/finite-metric-space.md).
6. Read [Explicit Representations](concepts/explicit-representations.md) to understand matrix, graph, and tree execution forms.
7. Read [Graph Representation Terminology](concepts/graph-representations.md) before promoting sparse graph construction.
8. Use the [Python API](api/python.md) or [C++ API](api/cpp.md) for implementation.
9. For Python, read [Python Space](python/space.md), [Python Intent Methods](python/intents.md), and [Python Result Objects](python/results.md) before strategy overrides.
10. Use [API Surface](stability.md) to understand core, expert, compatibility, extension APIs, and the module status matrix.
11. Use [Testing and CI Scope](testing-and-ci.md) to understand release gates versus historical coverage.
12. Use [Research Roadmap](research-roadmap.md) to understand which research directions can be promoted after deterministic fixtures and release gates exist.
13. Use [Revival Status](revival-status.md) to distinguish local revival completion from external release actions.

## Concepts

- [Metric-Space Numerics Manifesto](manifesto.md)
- [Engine Metric Space](engine/metric-space.md)
- [Engine Representations](engine/representations.md)
- [Engine Intent API](engine/intent-api.md)
- [Engine Strategies](engine/strategies.md)
- [Engine Operators](engine/operators.md)
- [Engine Mappings](engine/mappings.md)
- [Engine Runtime Policies](engine/runtime.md)
- [Engine Migration](engine/migration.md)
- [Metric Spaces](concepts/metric-space.md)
- [Finite Metric Spaces](concepts/finite-metric-space.md)
- [Metrics as Recoding Costs](concepts/metrics-as-recoding-costs.md)
- [Vector Space as a Special Case](concepts/vector-space-as-special-case.md)
- [Explicit Representations](concepts/explicit-representations.md)
- [Graph Representation Terminology](concepts/graph-representations.md)

## Python User Path

- [Space Constructor And Records](python/space.md)
- [Intent Methods](python/intents.md)
- [Result Objects](python/results.md)
- [Expert Strategy Overrides](python/strategies.md)
- [Errors And Troubleshooting](python/errors.md)
- [Python API Reference](api/python.md)

## Examples

- [Custom Metric Example](examples/custom-metric.md)
- [Python Custom Metric](examples/python-custom-metric.md)
- [Python Real-Data Records](examples/python-real-data.md)
- [Python Compare Spaces](examples/python-compare-spaces.md)
- [Structured Data](examples/structured-data.md)
- [Engine Demo Expected Outputs](examples/engine-demo-outputs.md)
- [Time-Series Space With TWED](examples/time-series-twed.md)
- [Histogram Space With EMD](examples/histogram-emd.md)
- [Exact Graph Edge Fixtures](examples/graph-construction.md)
- [Representative Selection](examples/representative-selection.md)
- [Entropy Diagnostics](examples/entropy-diagnostics.md)
- [Intrinsic Dimension Diagnostic](examples/intrinsic-dimension.md)
- [Correlation Between Metric Spaces](examples/correlation-between-spaces.md)
- [Industrial Anomaly Workflow](examples/industrial-anomaly-workflow.md)

## Architecture Records

The project keeps the design records that shaped the engine in the repository root. They are useful when extending the engine or reviewing why the API is organized around intent names, strategies, representations, and runtime policies.

- [Research Roadmap](research-roadmap.md)
- [Engine Implementation Plan](../ENGINE_IMPLEMENTATION_PLAN.md)
- [Capability Roadmap](../CAPABILITY_ROADMAP.md)
- [Algorithmic Gap Analysis](../ALGORITHMIC_GAP_ANALYSIS.md)
- [Representation Layer Plan](../REPRESENTATION_LAYER_PLAN.md)
- [Embedding And Mapping Roadmap](../EMBEDDING_AND_MAPPING_ROADMAP.md)
- [Native DNN Engine Plan](../NATIVE_DNN_ENGINE_PLAN.md)
- [Python User Experience Plan](../PYTHON_USER_EXPERIENCE_PLAN.md)
- [Flagship Demos Plan](../FLAGSHIP_DEMOS_PLAN.md)
- [Test And Validation Plan](../TEST_AND_VALIDATION_PLAN.md)
- [Changelog](../CHANGELOG.md)
- [Examples Tree Status](../examples/README.md)
- [Tests Tree Status](../tests/README.md)

## GitHub Pages

The public project page and technical documentation page are checked in under `docs/site/`.

If Pages is enabled for the repository and pointed at the static site artifact, the public URLs are:

- Project page: <https://metric-space-ai.github.io/metric/>
- Technical documentation: <https://metric-space-ai.github.io/metric/docs.html>
