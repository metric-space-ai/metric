# Visual Regression And Performance Workstream Report

Date: 2026-06-26

## Scope

This run covers the regression/performance gate scope from
`docs/visual/agent-tasks/visual-regression-performance-workstream.md`.
The checked gate scripts are:

- `visual/tools/check-visual-regression-public-examples.mjs`
- `visual/tools/check-visual-performance-large-scenes.mjs`

No runtime, layer, semantic view, example, native exporter, project page, GRAE10
reference code, or GRAE10 baseline file was edited. The protected baseline
`visual/regression-baselines/grae10-metric-engine.sha256` remains
authoritative and was not regenerated.

Generated screenshots and JSON outputs stayed under `output/visual/` and are
transient local review artifacts, not baselines.

## Commands Run

Required acceptance commands:

```bash
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-single-render-pipeline.mjs
```

Regression/performance gate commands:

```bash
node --check visual/tools/check-visual-regression-public-examples.mjs
node --check visual/tools/check-visual-performance-large-scenes.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-visual-performance-large-scenes.mjs
```

## Browser Gate Status

The browser gate was available and executed. No missing Playwright/Chrome/CDP
gate was accepted as a pass.

| Gate | Result | Browser | Report |
| --- | --- | --- | --- |
| Public visual regression | pass | Chrome for Testing via Playwright cache | `output/visual/check-visual-regression-public-examples/results.json` |
| Large-scene performance | pass | Google Chrome | `output/visual/check-visual-performance-large-scenes/results.json` |

## Acceptance Results

| Command | Result |
| --- | --- |
| `node visual/tools/check-grae10-golden.mjs` | pass |
| `node visual/tools/check-public-gallery-evidence.mjs` | pass |
| `node visual/tools/check-hero-grammar-contract.mjs` | pass |
| `node visual/tools/check-single-render-pipeline.mjs` | pass |
| `node --check visual/tools/check-visual-regression-public-examples.mjs` | pass |
| `node --check visual/tools/check-visual-performance-large-scenes.mjs` | pass |
| `node visual/tools/check-visual-regression-public-examples.mjs` | pass |
| `node visual/tools/check-visual-performance-large-scenes.mjs` | pass |

## Public Preview Classification

The regression gate separates load, render, native evidence, intended grammar,
interaction, and hero status. Render success alone does not promote any public
preview.

| Preview | Classification | Hero accepted | Load | Render | Native evidence | Grammar | Interaction | Records |
| --- | --- | --- | --- | --- | --- | --- | --- | ---: |
| `grae10-metric-engine` | hero-accepted | yes | pass | pass | pass | pass | pass | 60,000 |
| `condition-monitoring-hero` | public-preview-only | no | pass | pass | pass | pass | pass | 528 |
| `mixed-record-hero` | public-preview-only | no | pass | pass | pass | pass | pass | 2,000 |
| `cross-space-dependency-hero` | public-preview-only | no | pass | pass | pass | pass | pass | 512 |
| `mapping-dimensionality-hero` | public-preview-only | no | pass | pass | pass | pass | pass | 1,000 |
| `dynamics-noise-hero` | public-preview-only | no | pass | pass | pass | pass | pass | 512 |
| `relation-matrix-neighborhood` | public-preview-only | no | pass | pass | pass | pass | pass | 130 |
| `process-curve-external-hero` | public-preview-only | no | pass | pass | pass | pass | pass | 737 total / 576 source |

Detected synthetic development fixtures remain fixtures and are not promoted:

- `condition-monitoring-hero`
- `cross-space-dependency-hero`
- `dynamics-noise-hero`
- `mapping-dimensionality-hero`
- `mixed-record-hero`
- `relation-matrix-neighborhood`

No public preview was blocked by missing native evidence or record count in
this run. The process-curve preview remains preview-only only because manual
visual composition acceptance has not promoted it to an accepted hero.

## Large-Scene Performance

Budget: median frame <= 150 ms, at least 10 frames, at least one descriptor,
runtime layer, GPU buffer upload, and draw call.

| Records | Result | Descriptors | Runtime layers | Frames | Median frame | GPU buffer bytes | Draw calls |
| ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| 1,000 | pass | 1 | 1 | 421 | 8.3 ms | 52,096 | 532 |
| 10,000 | pass | 1 | 1 | 422 | 8.3 ms | 520,096 | 529 |
| 60,000 | pass | 1 | 1 | 421 | 8.3 ms | 3,120,096 | 521 |

## Grammar Performance Rows

The performance gate read `visual/hero-visual-briefs.manifest.json` and matched
all 7 required preview rows.

| Preview | Grammar | Result | Classification | Records | Minimum | Runtime layers | Median frame | Draw calls | Acceptance blockers |
| --- | --- | --- | --- | ---: | ---: | ---: | ---: | ---: | --- |
| `condition-monitoring-hero` | field | pass | public-preview-only | 528 | 500 | 6 | 8.3 ms | 1,128 | `visual-composition-not-human-accepted` |
| `mixed-record-hero` | glyph | pass | public-preview-only | 2,000 | 2,000 | 5 | 8.3 ms | 1,055 | `visual-composition-not-human-accepted` |
| `relation-matrix-neighborhood` | matrix+graph | pass | public-preview-only | 130 | 128 | 3 | 8.3 ms | 969 | `visual-composition-not-human-accepted`, `matrix-readability-not-human-accepted` |
| `dynamics-noise-hero` | dynamics | pass | public-preview-only | 512 | 500 | 3 | 8.3 ms | 933 | `visual-composition-not-human-accepted` |
| `mapping-dimensionality-hero` | mapping | pass | public-preview-only | 1,000 | 1,000 | 5 | 8.3 ms | 1,055 | `visual-composition-not-human-accepted` |
| `cross-space-dependency-hero` | graph | pass | public-preview-only | 512 | 500 | 4 | 8.3 ms | 1,022 | `visual-composition-not-human-accepted` |
| `process-curve-external-hero` | process-curve | pass | public-preview-only | 737 total / 576 source | 500 | 9 | 8.3 ms | 1,431 | `visual-composition-not-human-accepted` |

## Generated Artifacts

Transient local artifacts produced by this run:

- `output/visual/check-visual-regression-public-examples/results.json`
- `output/visual/check-visual-regression-public-examples/*.canvas.png`
- `output/visual/check-visual-performance-large-scenes/results.json`

These files are not documented as baselines and should not be committed.

The browser regression gate proves render output from canvas PNG pixels. On
the current macOS/headless Chromium setup, CDP full-page `Page.captureScreenshot`
can time out on WebGL scenes; this is recorded as a non-blocking artifact error
when the canvas PNG is captured and nonblank.

## Remaining Gaps

- The gates prove objective mechanics and performance budgets, not final visual
  taste.
- Public previews other than protected GRAE10 remain review-pending until
  explicit screenshot acceptance.
- `process-curve-external-hero` now meets the real source-record minimum and
  remains blocked only by visual composition acceptance.
