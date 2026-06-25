# Visual Acceptance Gates Report

Date: 2026-06-25

## Scope

This run executed the visual acceptance gates after the record-preview
presentation and relation-matrix readability integration. It covers public
visual examples, large-scene performance, grammar-specific preview evidence,
and the separate hero screenshot review gate.

Generated screenshots and JSON reports remain transient under `output/visual/`.
No project page, native exporter, protected GRAE10 data, screenshot baseline, or
public hero acceptance manifest was changed by this gate run.

## Commands Run

```bash
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-single-render-pipeline.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-visual-performance-large-scenes.mjs
node visual/tools/check-hero-screenshot-review.mjs
```

## Generated Reports

| Gate | Path | Generated |
| --- | --- | --- |
| Public visual regression | `output/visual/check-visual-regression-public-examples/results.json` | `2026-06-25T10:01:04.477Z` |
| Large-scene performance | `output/visual/check-visual-performance-large-scenes/results.json` | `2026-06-25T09:57:30.611Z` |
| Hero screenshot review | `output/visual/check-hero-screenshot-review/results.json` | `2026-06-25T10:01:35.477Z` |

## Hero Acceptance

- Accepted hero count: 1
- Review-pending preview count: 6
- Accepted hero: `grae10-metric-engine`
- Public previews remain `public-preview-only`; load, render, native evidence,
  grammar, screenshots, and performance evidence do not promote them to
  hero-accepted.

## Public Regression

| Preview | Classification | Grammar | Native evidence | Runtime layers | Draw calls |
| --- | --- | --- | --- | ---: | ---: |
| `grae10-metric-engine` | `hero-accepted` | pass | protected GRAE10 dataset | n/a | 208 |
| `condition-monitoring-hero` | `public-preview-only` | pass | yes | 6 | 453 |
| `mixed-record-hero` | `public-preview-only` | pass | yes | 5 | 368 |
| `cross-space-dependency-hero` | `public-preview-only` | pass | yes | 4 | 374 |
| `relation-matrix-neighborhood` | `public-preview-only` | pass | yes | 3 | 291 |
| `dynamics-noise-hero` | `public-preview-only` | pass | yes | 4 | 364 |
| `mapping-dimensionality-hero` | `public-preview-only` | pass | yes | 5 | 356 |

## Performance Rows

| Records | Result | Descriptors | Runtime layers | Frames | Median frame | GPU buffer bytes | Draw calls |
| ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| 1,000 | pass | 1 | 1 | 421 | 8.3 ms | 52,096 | 521 |
| 10,000 | pass | 1 | 1 | 421 | 8.3 ms | 520,096 | 526 |
| 60,000 | pass | 1 | 1 | 420 | 8.3 ms | 3,120,096 | 506 |

## Grammar Preview Status

| Preview | Grammar | Status | Hero accepted | Records | Descriptors | Runtime layers | Required primitives | Median frame | Draw calls |
| --- | --- | --- | --- | ---: | ---: | ---: | --- | ---: | ---: |
| `condition-monitoring-hero` | `field` | pass, `public-preview-only` | false | 15 | 6 | 6 | `HeatFieldLayer`, `CurveRibbonLayer` | 8.3 ms | 2,472 |
| `mixed-record-hero` | `glyph` | pass, `public-preview-only` | false | 20 | 5 | 5 | `InstancedGlyphLayer`, `RelationEdgeLayer` | 8.3 ms | 2,015 |
| `relation-matrix-neighborhood` | `matrix+graph` | pass, `public-preview-only` | false | 130 | 3 | 3 | `RelationMatrixLayer`, `RelationEdgeLayer` | 8.3 ms | 1,419 |
| `dynamics-noise-hero` | `dynamics` | pass, `public-preview-only` | false | 28 | 4 | 4 | `CurveRibbonLayer`, `HeatFieldLayer`, `InstancedPointLayer` | 8.3 ms | 1,345 |
| `mapping-dimensionality-hero` | `mapping` | pass, `public-preview-only` | false | 15 | 5 | 5 | `InstancedPointLayer`, `GroundProjectionLayer` | 8.3 ms | 1,315 |
| `cross-space-dependency-hero` | `graph` | pass, `public-preview-only` | false | 48 | 4 | 4 | `RelationEdgeLayer`, `InstancedGlyphLayer` | 8.3 ms | 1,022 |

## Generated Artifacts

Generated screenshots and JSON output stayed under `output/visual/`:

- `output/visual/check-visual-regression-public-examples/`
- `output/visual/check-visual-performance-large-scenes/results.json`
- `output/visual/check-hero-screenshot-review/results.json`

These generated `output/` files were not staged or committed.

## Remaining Gaps

- The gates prove objective mechanics, not final visual taste.
- Public previews other than GRAE10 remain review-pending until explicit
  screenshot acceptance criteria or a review manifest exists.
- Performance timings are local-browser evidence; renderer details are captured
  in the generated JSON for future comparison.
