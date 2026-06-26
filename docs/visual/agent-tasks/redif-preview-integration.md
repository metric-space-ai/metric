# Redif Native Preview Integration

## Purpose

Use the existing native Redif metric-dynamics evidence as an engine preview
without promoting it to a public hero.

This is not a new algorithm implementation. C++ already exports the evidence.
The browser side must only load, validate, render, inspect and animate the
exported `metric.visual.v1` document.

## Owner Scope

- `visual/examples/redif-metric-dynamics-preview/index.html`
- optional checker under `visual/tools/check-redif-preview.mjs`
- optional report under `docs/visual/reports/redif-preview-integration.md`

Do not edit:

- `visual/examples/grae10-metric-engine/**`
- `visual/regression-baselines/grae10-metric-engine.sha256`
- native C++ Redif algorithms
- `docs/site/index.html` unless explicitly coordinating with the parent agent

## Evidence Source

Use only:

```text
docs/examples/assets/redif-metric-dynamics/metric.visual.json
```

The page must fail if the document is synthetic, missing explicit native
provenance, or does not validate with:

```bash
node visual/tools/check-redif-visual-export.mjs docs/examples/assets/redif-metric-dynamics/metric.visual.json
node visual/tools/check-visual-document.mjs docs/examples/assets/redif-metric-dynamics/metric.visual.json
```

## Visual Contract

Use the public command surface:

```js
const visual = await createMetricVisual({ canvas, evidence });
visual.showDynamics(...);
```

Required visual grammar:

- exported Redif timeline states rendered as metric-space state changes;
- exported path/entropy property rendered as field or color evidence;
- relation matrix available as supporting pair evidence if useful;
- no page-local metric, entropy, Redif, embedding or coordinate computation;
- no synthetic point padding to make the scene look bigger.

## Acceptance

Run:

```bash
node visual/tools/check-redif-visual-export.mjs docs/examples/assets/redif-metric-dynamics/metric.visual.json
node visual/tools/check-visual-document.mjs docs/examples/assets/redif-metric-dynamics/metric.visual.json
node visual/tools/check-redif-preview.mjs
node visual/tools/check-grae10-golden.mjs
```

The result can be a review-pending algorithm preview. It must not be added to
accepted heroes.
