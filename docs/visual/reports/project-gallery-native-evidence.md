# Project gallery native evidence report

Generated: 2026-06-26

## Public page state

- Page: `docs/site/index.html`
- Browser URL used for capture: `http://127.0.0.1:9876/docs/site/index.html`
- First viewport capture: `output/visual/project-gallery-native-evidence/first-viewport.png`
- Gallery viewport capture: `output/visual/project-gallery-native-evidence/gallery-section.png`
- Browser-observed GRAE10 iframe count: 1
- Browser-observed gallery iframe count: 7
- Browser-observed gallery statuses: `accepted reference`, then seven `review pending` previews

The public page keeps the protected 60k MNIST/GRAE10 visual in the first viewport
and links it from the gallery evidence row instead of embedding a duplicate
GRAE10 iframe. The page does not publish Python snippets or Python tabs.

## Validation

All required checks passed:

```bash
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-hero-screenshot-review.mjs
```

Validation artifacts:

- `output/visual/check-visual-regression-public-examples/results.json`
- `output/visual/check-hero-screenshot-review/results.json`
- `output/visual/check-hero-screenshot-review/index.html`

`check-public-gallery-evidence.mjs` reported `ok: true`, no public synthetic
fixtures, and native `metric.visual.v1` assets for each review-pending gallery
preview. `check-visual-regression-public-examples.mjs` reported `ok: true` with
8 total examples and 0 failures. `check-hero-screenshot-review.mjs` reported
`ok: true`, with `grae10-metric-engine` accepted and the other public examples
review-pending.

## Gallery evidence status

| Gallery item | Public status | Evidence status | Visual path | C++ source | Evidence path |
| --- | --- | --- | --- | --- | --- |
| MNIST dimension reduction | Accepted reference | Protected 60k GRAE10 dataset, render OK, grammar OK, hero accepted | `visual/examples/grae10-metric-engine/index.html` | `examples/engine/mnist_grae10_integrity.cpp` | `visual/examples/grae10-metric-engine/grae10-data.json`; `docs/examples/mnist-grae10-gallery.md` |
| Condition monitoring | Review pending | Native `metric.visual.v1`, 528 records, 2 relations, render OK, grammar OK | `visual/examples/condition-monitoring-hero/index.html` | `examples/engine/condition_monitoring_visual_export.cpp` | `docs/examples/assets/condition-monitoring/metric.visual.json` |
| Mixed records | Review pending | Native `metric.visual.v1`, 2000 records, 1 relation, render OK, grammar OK | `visual/examples/mixed-record-hero/index.html` | `examples/engine/mixed_finite_records_visual_export.cpp` | `docs/examples/assets/mixed-records/metric.visual.json` |
| Cross-space dependency | Review pending | Native `metric.visual.v1`, 512 records, 3 relations, render OK, grammar OK | `visual/examples/cross-space-dependency-hero/index.html` | `examples/engine/cross_space_dependency_visual_export.cpp` | `docs/examples/assets/cross-space-dependency/metric.visual.json` |
| Mapping and dimensionality | Review pending | Native `metric.visual.v1`, 1000 records, 2 relations, render OK, grammar OK | `visual/examples/mapping-dimensionality-hero/index.html` | `examples/engine/mapping_dimensionality_visual_export.cpp` | `docs/examples/assets/mapping-dimensionality/metric.visual.json` |
| Finite dynamics | Review pending | Native `metric.visual.v1`, 512 records, 2 relations, render OK, grammar OK | `visual/examples/dynamics-noise-hero/index.html` | `examples/engine/finite_metric_dynamics_visual_export.cpp` | `docs/examples/assets/dynamics-noise/metric.visual.json` |
| Relation matrix | Review pending | Native `metric.visual.v1`, 130 records, 1 relation, render OK, grammar OK | `visual/examples/relation-matrix-neighborhood/index.html` | `examples/engine/relation_matrix_visual_export.cpp` | `docs/examples/assets/relation-matrix/metric.visual.json` |
| UCR process curves | Review pending | Native `metric.visual.v1`, 64 records, 1 relation, render OK, grammar OK; maintainer blocker: `record-count-below-hero-minimum` | `visual/examples/process-curve-external-hero/index.html` | `examples/engine/process_curve_external_visual_export.cpp` | `docs/examples/assets/process-curve-external/metric.visual.json` |
