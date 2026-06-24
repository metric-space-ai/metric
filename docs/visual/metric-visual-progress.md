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
| Mixed records | not live | Current visual example is synthetic fixture evidence. Needs native exported evidence from a real heterogeneous source or an explicitly documented checked fixture. |
| Cross-space dependence | not live | Current visual example is synthetic fixture evidence. Needs native paired-space export from a real paired dataset. |
| Condition monitoring | not live | Current visual example is synthetic fixture evidence. UCR process curves cover the current real condition-monitoring proof. A separate condition-monitoring hero needs real process-run evidence. |
| Relation matrix/neighborhood | not live | Current visual example is synthetic fixture evidence. Needs a meaningful native relation table with block order, neighborhoods and pair previews. |
| Dynamics/noise | not live | Current visual example is synthetic fixture evidence. Needs native trajectory or perturbation evidence. |
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

Build native `metric.visual.v1` exporters for the planned heroes, one by one,
before changing the project page:

1. export a real or documented native mixed-record evidence document
2. export a real paired-space dependence document
3. export a real relation-matrix/neighborhood document
4. export a real dynamics or perturbation document
5. connect each exported document to a distinct visual grammar
6. add each hero to the public page only after the evidence gate passes
