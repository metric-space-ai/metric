# METRIC Visual Progress

This file tracks what is actually proven in the tree. It deliberately separates
engine capability, native evidence, synthetic development fixtures, and public
gallery readiness.

## Current Rule

A public hero is ready only when all of these are true:

- the dataset or reference asset is documented
- the result is checked by native C++ code, not by JavaScript-only fixture logic
- the visualization reads that checked evidence or a documented reference asset
- the public page exposes no synthetic fixture as a live result

Synthetic visual fixtures may exist under `visual/examples/*/evidence.json`.
They are engine development fixtures only. They are not public heroes.

## Checks

```bash
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-views.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-single-render-pipeline.mjs
node visual/tools/check-public-gallery-evidence.mjs
ctest --test-dir build/core -L 'metric_application_evidence|metric_phate_pipeline|metric_mnist|metric_visual_integrity|metric_benchmark_report' --output-on-failure
```

Browser screenshot checks currently need a Playwright package on `NODE_PATH` or
the bundled Playwright CLI wrapper. The Node scripts must not be reported as
passing when the package is unavailable.

## Proven Native Evidence

| Evidence | Status | Proof |
| --- | --- | --- |
| MNIST 60k dimension-reduction reference | checked | `examples/engine/mnist_grae10_integrity.cpp` verifies `60,000` records, labels against the MNIST IDX training labels, finite 2D coordinates, finite 3D coordinates, and the protected GRAE10 visual hash. |
| MNIST PHATE-AE native smoke/gallery path | checked, not 60k gallery | `examples/engine/mnist_phate_autoencoder_smoke.cpp` and `examples/engine/mnist_phate_autoencoder_gallery.cpp` run native PHATE-AE on a balanced MNIST subset. This does not replace the 60k GRAE10 reference. |
| UCR process curves | checked | `examples/engine/process_curve_external_gallery.cpp` exports query winners, distance tables, summaries, and SVG assets from the UCR Time Series Anomaly Detection datasets. |
| Engine application examples | checked | CTest label `metric_application_evidence` covers the current native C++ application examples and assertions. |
| Condition-monitoring visual evidence | checked native export, not public hero | `examples/engine/condition_monitoring_visual_export.cpp` writes `docs/examples/assets/condition-monitoring/metric.visual.json`; `visual/tools/check-visual-document.mjs` validates the exported `metric.visual.v1` document with `provenance.synthetic: false`. |
| Mixed-record visual evidence | checked native export, not public hero | `examples/engine/mixed_finite_records_visual_export.cpp` writes `docs/examples/assets/mixed-records/metric.visual.json`; `visual/tools/check-visual-document.mjs` validates records, composed payloads, metric relation, coordinates, properties and diagnostics. |
| Cross-space dependence visual evidence | checked native export, not public hero | `examples/engine/cross_space_dependency_visual_export.cpp` writes `docs/examples/assets/cross-space-dependency/metric.visual.json`; the document contains two spaces, paired relations and dependence diagnostics computed in C++. |
| Dynamics/noise visual evidence | checked native export, not public hero | `examples/engine/finite_metric_dynamics_visual_export.cpp` writes `docs/examples/assets/dynamics-noise/metric.visual.json`; the document contains native trajectory, timeline, relation and diagnostic evidence. |
| Relation-matrix visual evidence | checked native export, not public hero | `examples/engine/relation_matrix_visual_export.cpp` writes `docs/examples/assets/relation-matrix/metric.visual.json`; the document contains a native block-ordered relation table and supporting properties. |
| Mapping/dimensionality visual evidence | checked native export, not public hero | `examples/engine/mapping_dimensionality_visual_export.cpp` writes `docs/examples/assets/mapping-dimensionality/metric.visual.json`; the document contains source/target spaces, coordinate states and mapping diagnostics. |

## Proven Visual Engine Capability

| Capability | Status | Proof |
| --- | --- | --- |
| GRAE10 protected visual | checked | `visual/examples/grae10-metric-engine/index.html` must match `visual/regression-baselines/grae10-metric-engine.sha256`; `visual/tools/check-grae10-golden.mjs` enforces this. |
| Shared semantic views | checked headlessly | `visual/tools/check-views.mjs` verifies the current semantic views produce renderable descriptors. |
| Grammar contract | checked headlessly | `visual/tools/check-hero-grammar-contract.mjs` rejects collapsing unrelated hero concepts into one point-cloud-only grammar. |
| Single runtime path | checked headlessly | `visual/tools/check-single-render-pipeline.mjs` protects the one-runtime pipeline rule. |
| Public gallery evidence gate | checked headlessly | `visual/tools/check-public-gallery-evidence.mjs` blocks synthetic hero fixtures from the public site and protects the GRAE10 reference hash. |

## Public Gallery Status

| Hero | Public status | Reason |
| --- | --- | --- |
| MNIST dimension reduction | live | Full 60k visual reference exists and is protected by C++ integrity plus hash checks. |
| UCR process curves | live | Real dataset, native C++ export, and checked public assets exist. |
| Mixed records | not live | Native `metric.visual.v1` evidence exists. Public status still needs a distinct mixed-record visual grammar, screenshot review, and gallery wiring away from the synthetic fixture. |
| Cross-space dependence | not live | Native `metric.visual.v1` evidence exists. Public status still needs a two-space dependence grammar, screenshot review, and gallery wiring away from the synthetic fixture. |
| Condition monitoring | not live | Native `metric.visual.v1` evidence exists. Public status still needs a curated condition-monitoring visual grammar that does not regress the existing UCR proof. |
| Relation matrix/neighborhood | not live | Native `metric.visual.v1` evidence exists. Public status still needs a legible matrix/neighborhood visual grammar, screenshot review, and gallery wiring away from the synthetic fixture. |
| Dynamics/noise | not live | Native `metric.visual.v1` evidence exists. Public status still needs a trajectory/dynamics grammar, screenshot review, and gallery wiring away from the synthetic fixture. |
| Mapping/dimensionality | not live | Native `metric.visual.v1` evidence exists. Public status still needs a mapping/morph visual grammar, screenshot review, and gallery wiring away from the synthetic fixture. |
| Metric discovery | not live | Needs native metric-admission evidence and rejected-candidate documentation. |

## Synthetic Development Fixtures

These examples may be useful to develop the renderer, but they must not be
published as real hero applications until native evidence replaces their
`provenance.synthetic: true` documents:

- `visual/examples/condition-monitoring-hero/evidence.json`
- `visual/examples/cross-space-dependency-hero/evidence.json`
- `visual/examples/dynamics-noise-hero/evidence.json`
- `visual/examples/mapping-dimensionality-hero/evidence.json`
- `visual/examples/mixed-record-hero/evidence.json`
- `visual/examples/relation-matrix-neighborhood/evidence.json`

## Next Implementation Step

The native exporter foundation now exists for the planned visual families. The
next production step is not more exporter scaffolding and not custom hero HTML;
it is to consume the checked `docs/examples/assets/*/metric.visual.json`
documents through reusable visual commands and distinct semantic grammars:

1. wire each exported document into `createMetricVisual` or the corresponding
   high-level visual command
2. replace the public-facing synthetic fixture path only after the native
   evidence is loaded by that reusable command
3. give each hero a distinct grammar: metric-space sculpture, mixed-record
   glyphs, paired-space dependence, relation matrix/neighborhood, dynamics
   trajectory, or mapping/morph
4. run browser screenshot review against the visual brief for that grammar
5. add each hero to the project gallery only after the evidence gate,
   grammar check, visual document check and screenshot review pass
