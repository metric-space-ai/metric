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
- the page does not present generated foundation fixtures as real-world dataset
  evidence

Synthetic visual fixtures may exist under `visual/examples/*/evidence.json`.
They are engine development fixtures only. They are not public heroes.

Native generated fixtures may be linked as public previews when their
`metric.visual.v1` export is native, schema-valid, and explicit about its
fixture/foundation status. A public preview is not a hero acceptance claim.

## Checks

```bash
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-views.mjs
node visual/tools/check-view-reference-contract.mjs
node visual/tools/check-relation-matrix-picker.mjs
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
| View reference validation | checked headlessly | `visual/tools/check-view-reference-contract.mjs` verifies explicit coordinate, relation and property IDs fail hard instead of falling back to unrelated defaults; it also verifies `MappingView` emits labels from an exported label property. |
| Relation matrix picking | checked headlessly and in browser | `visual/src/relational/matrix-picking.js` maps pointer positions to exported dense-matrix cells; `visual/tools/check-relation-matrix-picker.mjs` verifies the contract and the relation-matrix page uses that engine helper for pair preview. |
| Engine pair preview | checked in browser | `MetricVisualSurface` builds relation-matrix pair pickers from `RelationMatrixLayer` descriptors and feeds pair inputs into `RecordPreviewPanel`; the relation-matrix page no longer owns a custom pair panel. Browser smoke verified `pc-054 ↔ pc-057` with value `2.8311` from native evidence. |
| Grammar contract | checked headlessly | `visual/tools/check-hero-grammar-contract.mjs` rejects collapsing unrelated hero concepts into one point-cloud-only grammar. |
| Single runtime path | checked headlessly | `visual/tools/check-single-render-pipeline.mjs` protects the one-runtime pipeline rule. |
| Public gallery evidence gate | checked headlessly | `visual/tools/check-public-gallery-evidence.mjs` blocks synthetic hero fixtures from the public site and protects the GRAE10 reference hash. |

## Public Gallery Status

| Hero | Public status | Reason |
| --- | --- | --- |
| MNIST dimension reduction | live | Full 60k visual reference exists and is protected by C++ integrity plus hash checks. |
| UCR process curves | live | Real dataset, native C++ export, and checked public assets exist. |
| Mixed records | public preview, not hero-accepted | Native `metric.visual.v1` evidence exists, the page loads it, and the project page links it. Screenshot acceptance and larger public data are still pending. |
| Cross-space dependence | public preview, not hero-accepted | Native `metric.visual.v1` evidence exists, the page loads it, and the project page links it. Screenshot acceptance and stronger visual curation are still pending. |
| Condition monitoring | public preview, not hero-accepted | Native `metric.visual.v1` evidence exists, the page loads it, and the project page links it. Screenshot acceptance and a curated proof that does not regress the existing UCR proof are still pending. |
| Relation matrix/neighborhood | public preview, not hero-accepted | Native `metric.visual.v1` evidence exists, the page loads it, and the project page links it. Screenshot acceptance and matrix readability review are still pending. |
| Dynamics/noise | public preview, not hero-accepted | Native `metric.visual.v1` evidence exists, the page loads it, and the project page links it. Screenshot acceptance and trajectory readability review are still pending. |
| Mapping/dimensionality | public preview, not hero-accepted | Native `metric.visual.v1` evidence exists, the page loads it, and the project page links it. Screenshot acceptance and morph readability review are still pending. |
| Metric discovery | not live | Needs native metric-admission evidence and rejected-candidate documentation. |

## Synthetic Development Fixtures

These `evidence.json` files may be useful to develop the renderer, but they must
not be published as real hero applications. The matching example pages now load
native assets from `docs/examples/assets/*/metric.visual.json`; these fixture
files remain only as renderer development inputs:

- `visual/examples/condition-monitoring-hero/evidence.json`
- `visual/examples/cross-space-dependency-hero/evidence.json`
- `visual/examples/dynamics-noise-hero/evidence.json`
- `visual/examples/mapping-dimensionality-hero/evidence.json`
- `visual/examples/mixed-record-hero/evidence.json`
- `visual/examples/relation-matrix-neighborhood/evidence.json`

## Next Implementation Step

The native exporter foundation now exists for the planned visual families, and
the project page contains public preview links for those native views. The next
production step is not more exporter scaffolding and not custom hero HTML; it is
to turn each preview into an accepted hero through screenshot review, stronger
visual grammar and larger public evidence where needed:

1. keep each public page on native `docs/examples/assets/*/metric.visual.json`
   evidence through `createMetricVisual`
2. give each hero a distinct grammar: metric-space sculpture, mixed-record
   glyphs, paired-space dependence, relation matrix/neighborhood, dynamics
   trajectory, or mapping/morph
3. run browser screenshot review against the visual brief for that grammar
4. replace small native fixtures with larger public evidence where the current
   record count does not yet prove application value
5. mark a preview as hero-accepted only after evidence gate, grammar check,
   visual document check and screenshot review pass
