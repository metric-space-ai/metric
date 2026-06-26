# Redif Preview Integration

Date: 2026-06-26

## Result

Redif metric-dynamics native evidence now has a review-pending engine preview:

```text
visual/examples/redif-metric-dynamics-preview/index.html
```

The page loads only:

```text
docs/examples/assets/redif-metric-dynamics/metric.visual.json
```

It renders through the public command surface:

```js
const visual = await createMetricVisual({ canvas, evidence });
visual.showDynamics(...);
```

No JavaScript Redif, distance, entropy, coordinate or metric computation is
performed by the page. The browser uses exported timeline states, the exported
transport-path property, the exported entropy property and the exported source
metric relation.

This is an engine preview. It is not added to accepted heroes and is not part
of the public project-page gallery.

## Verification

```bash
node visual/tools/check-redif-visual-export.mjs docs/examples/assets/redif-metric-dynamics/metric.visual.json
node visual/tools/check-visual-document.mjs docs/examples/assets/redif-metric-dynamics/metric.visual.json
node visual/tools/check-redif-preview.mjs
node visual/tools/check-grae10-golden.mjs
METRIC_VISUAL_EXAMPLES=redif-metric-dynamics-preview METRIC_VISUAL_OUT=output/visual/redif-preview-browser node visual/tools/check-visual-examples.mjs
```

The browser smoke rendered one canvas with no console or page errors and wrote:

```text
output/visual/redif-preview-browser/redif-metric-dynamics-preview.png
```

Generated screenshots are review artifacts and are not committed.
