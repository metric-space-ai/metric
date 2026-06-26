# Project Site Visual Portfolio Redesign

Generated: 2026-06-26

## Scope

- Updated `docs/site/index.html`.
- Added this report at `docs/visual/reports/project-site-visual-portfolio-redesign.md`.
- No Engine, Exporter, GRAE10 implementation, runtime, visual example, schema or public-gate source files were edited.

## Page Result

The project page now opens as a visual portfolio. The first viewport leads with
the accepted 60k MNIST/GRAE10 reference in a single embedded visual. The copy is
short, and the hero links directly to the gallery plus the real C++ evidence
path `examples/engine/mnist_grae10_integrity.cpp`.

The gallery follows as a compact portfolio list. It shows one accepted reference
row without duplicating the GRAE10 iframe, then seven native preview rows with
live visual links, native evidence JSON links and real C++ source paths.

No Python tabs, invented snippets or UI command snippets were added. The public
page shows only real existing source and evidence paths.

## Browser Check

Browser URL:

```text
http://127.0.0.1:9877/docs/site/index.html
```

Screenshots:

- First viewport: `output/visual/project-site-visual-portfolio-redesign/first-viewport.png`
- Gallery section: `output/visual/project-site-visual-portfolio-redesign/gallery-section.png`

Observed in browser at 1280x820:

- Page title: `METRIC | Visual Evidence Portfolio`
- GRAE10 iframe count: 1
- Hero GRAE10 iframe count: 1
- Gallery native preview iframe count: 7
- Console errors: 0
- Console warnings: 1 existing WebGL warning from `visual/src/layers/GroundPlaneLayer.js:92`

First viewport composition:

- Fixed top navigation with `METRIC`, Gallery, Concept, Docs and GitHub.
- Left side contains the accepted 60k MNIST reference label, brand, concise
  explanation and direct links.
- Right side is dominated by the accepted
  `visual/examples/grae10-metric-engine/index.html` visual reference.

Gallery composition:

- Compact accepted-reference row for `grae10-metric-engine`, with no duplicate
  GRAE10 iframe.
- Native preview rows use compact live previews and real evidence/source paths.
- `process-curve-external-hero` is shown as a native preview, not an accepted
  hero; the source-record-count blocker remains in maintainer documentation.

## Gallery Items

| Item | Browser status | Label | Visual path | C++ path | Evidence path |
| --- | --- | --- | --- | --- | --- |
| `grae10-metric-engine` | `accepted` | Accepted reference | `visual/examples/grae10-metric-engine/index.html` | `examples/engine/mnist_grae10_integrity.cpp` | `visual/examples/grae10-metric-engine/grae10-data.json` |
| `condition-monitoring-hero` | `review-pending` | Native preview | `visual/examples/condition-monitoring-hero/index.html` | `examples/engine/condition_monitoring_visual_export.cpp` | `docs/examples/assets/condition-monitoring/metric.visual.json` |
| `mixed-record-hero` | `review-pending` | Native preview | `visual/examples/mixed-record-hero/index.html` | `examples/engine/mixed_finite_records_visual_export.cpp` | `docs/examples/assets/mixed-records/metric.visual.json` |
| `cross-space-dependency-hero` | `review-pending` | Native preview | `visual/examples/cross-space-dependency-hero/index.html` | `examples/engine/cross_space_dependency_visual_export.cpp` | `docs/examples/assets/cross-space-dependency/metric.visual.json` |
| `mapping-dimensionality-hero` | `review-pending` | Native preview | `visual/examples/mapping-dimensionality-hero/index.html` | `examples/engine/mapping_dimensionality_visual_export.cpp` | `docs/examples/assets/mapping-dimensionality/metric.visual.json` |
| `dynamics-noise-hero` | `review-pending` | Native preview | `visual/examples/dynamics-noise-hero/index.html` | `examples/engine/finite_metric_dynamics_visual_export.cpp` | `docs/examples/assets/dynamics-noise/metric.visual.json` |
| `relation-matrix-neighborhood` | `review-pending` | Native preview | `visual/examples/relation-matrix-neighborhood/index.html` | `examples/engine/relation_matrix_visual_export.cpp` | `docs/examples/assets/relation-matrix/metric.visual.json` |
| `process-curve-external-hero` | `review-pending scale-blocked` | Native preview | `visual/examples/process-curve-external-hero/index.html` | `examples/engine/process_curve_external_visual_export.cpp` | `docs/examples/assets/process-curve-external/metric.visual.json` |

## Validation Commands

```bash
node visual/tools/check-public-gallery-evidence.mjs
```

Result: passed, `ok: true`, no issues. The page links seven public native
preview examples and no public synthetic fixtures. GRAE10 hash:
`464f6a90c36c1e9c6b4ec90068500dc226740d65b251918aca567f99d64d3d5e`.

```bash
node visual/tools/check-hero-screenshot-review.mjs
```

Result: passed, `ok: true`. Accepted heroes: `grae10-metric-engine`. Review
pending previews: `condition-monitoring-hero`, `mixed-record-hero`,
`cross-space-dependency-hero`, `mapping-dimensionality-hero`,
`dynamics-noise-hero`, `relation-matrix-neighborhood`,
`process-curve-external-hero`.

```bash
node visual/tools/check-grae10-golden.mjs
```

Result: passed. GRAE10 golden reference OK:
`464f6a90c36c1e9c6b4ec90068500dc226740d65b251918aca567f99d64d3d5e`.

```bash
node visual/tools/check-visual-regression-public-examples.mjs
```

Result: passed, `ok: true`, 8 total public examples, 0 failures. Artifacts were
written under `output/visual/check-visual-regression-public-examples/`.

## Supporting Commands

```bash
node - <<'NODE'
const fs = require('fs');
const path = require('path');
const root = process.cwd();
const sitePath = path.join(root, 'docs/site/index.html');
const html = fs.readFileSync(sitePath, 'utf8');
const attrs = [...html.matchAll(/(?:href|src)="([^"]+)"/g)].map(m => m[1]);
const local = attrs.filter(v => !v.startsWith('http') && !v.startsWith('#') && !v.startsWith('data:'));
const missing = [];
for (const value of local) {
  const resolved = path.resolve(path.dirname(sitePath), value);
  if (!fs.existsSync(resolved)) missing.push({ value, resolved });
}
console.log(JSON.stringify({ localLinks: local.length, missing }, null, 2));
NODE
```

Result: 44 local links checked, 0 missing.

```bash
python3 -m http.server 9877 --bind 127.0.0.1
```

Result: local static server served the page for browser capture.

```bash
/Users/michaelwelsch/.codex/skills/playwright/scripts/playwright_cli.sh open http://127.0.0.1:9877/docs/site/index.html
/Users/michaelwelsch/.codex/skills/playwright/scripts/playwright_cli.sh resize 1280 820
/Users/michaelwelsch/.codex/skills/playwright/scripts/playwright_cli.sh screenshot --filename output/visual/project-site-visual-portfolio-redesign/first-viewport.png
/Users/michaelwelsch/.codex/skills/playwright/scripts/playwright_cli.sh goto http://127.0.0.1:9877/docs/site/index.html#gallery
/Users/michaelwelsch/.codex/skills/playwright/scripts/playwright_cli.sh screenshot --filename output/visual/project-site-visual-portfolio-redesign/gallery-section.png
```

Result: browser opened the final page, captured both requested views and
reported 0 console errors.

```bash
/Users/michaelwelsch/.codex/skills/playwright/scripts/playwright_cli.sh eval '() => ({ title: document.title, heroIframeCount: document.querySelectorAll(".hero iframe[src*=\"grae10-metric-engine\"]").length, graeIframeCount: document.querySelectorAll("iframe[src*=\"grae10-metric-engine\"]").length, galleryItems: Array.from(document.querySelectorAll("[data-gallery-item]")).map((node) => ({ item: node.dataset.galleryItem, status: node.dataset.evidenceStatus, label: node.querySelector(".status")?.textContent.trim() || null })), previewIframeCount: document.querySelectorAll(".portfolio iframe").length })'
```

Result: DOM summary confirmed one GRAE10 iframe, seven native preview iframes
and the exact gallery statuses listed above.
