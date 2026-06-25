# Visual Regression And Performance Gate Report

Date: 2026-06-25

## Scope

This milestone extends the browser-backed visual performance gate so the
objective evidence is no longer limited to synthetic point-cloud scaling. The
gate still records the original large-scene point rows, and now also loads the
public preview pages for field, glyph, matrix, graph, dynamics, and mapping
grammars.

Updated gate:

- `visual/tools/check-visual-performance-large-scenes.mjs`

No runtime, layer, example, native exporter, project page, GRAE10 reference, or
baseline file was changed. Generated screenshots and JSON reports remain
transient under `output/visual/`.

## Acceptance Commands

Passed:

```bash
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-single-render-pipeline.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-visual-performance-large-scenes.mjs
node visual/tools/check-hero-screenshot-review.mjs
```

Additional local syntax check:

```bash
node --check visual/tools/check-visual-performance-large-scenes.mjs
```

## Hero Acceptance Semantics

The public browser gates keep load, render, native evidence, intended grammar,
interaction, and performance evidence separate from hero acceptance.

`grae10-metric-engine` remains the only accepted hero because the protected 60k
GRAE10 hash and dataset shape are intact. Every other public visual page is
reported as `public-preview-only`, even when it loads, renders nonblank pixels,
uses native `metric.visual.v1` evidence, satisfies the grammar contract, passes
interaction probes, and meets performance diagnostics.

## Public Preview Classification

Regression report:

- Path: `output/visual/check-visual-regression-public-examples/results.json`
- Generated: `2026-06-25T07:43:53.505Z`
- Result: 7 checked, 0 failed

| Preview | Classification | Native evidence | Runtime layers | Draw calls |
| --- | --- | --- | ---: | ---: |
| `grae10-metric-engine` | hero-accepted | protected GRAE10 dataset | n/a | 156 |
| `condition-monitoring-hero` | public-preview-only | native `metric.visual.v1` | 6 | 390 |
| `mixed-record-hero` | public-preview-only | native `metric.visual.v1` | 5 | 328 |
| `cross-space-dependency-hero` | public-preview-only | native `metric.visual.v1` | 4 | 355 |
| `relation-matrix-neighborhood` | public-preview-only | native `metric.visual.v1` | 3 | 309 |
| `dynamics-noise-hero` | public-preview-only | native `metric.visual.v1` | 4 | 383 |
| `mapping-dimensionality-hero` | public-preview-only | native `metric.visual.v1` | 5 | 356 |

Screenshot review report:

- Path: `output/visual/check-hero-screenshot-review/results.json`
- Generated: `2026-06-25T07:44:12.871Z`
- Result: pass
- Accepted heroes: 1
- Review-pending previews: 6

## Performance Evidence

Performance report:

- Path: `output/visual/check-visual-performance-large-scenes/results.json`
- Generated: `2026-06-25T07:44:12.747Z`
- Result: pass
- Browser: headless Chrome using the available WebGL backend
- Budget: median frame <= 150 ms, at least 10 sampled frames, at least one
  runtime layer, descriptor, GPU upload, and draw call

### Point-Cloud Scaling

| Records | Result | Median frame | Frames sampled | Runtime layers | Buffer bytes | Draw calls |
| ---: | --- | ---: | ---: | ---: | ---: | ---: |
| 1,000 | pass | 8.3 ms | 421 | 1 | 52,096 | 519 |
| 10,000 | pass | 8.3 ms | 421 | 1 | 520,096 | 518 |
| 60,000 | pass | 8.3 ms | 420 | 1 | 3,120,096 | 517 |

### Grammar Diagnostics

| Preview | Grammar | Required primitives | Records | Descriptors | Runtime layers | Median frame | GPU buffer bytes | Draw calls |
| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `condition-monitoring-hero` | field | `HeatFieldLayer`, `CurveRibbonLayer` | 15 | 6 | 6 | 8.3 ms | 5,544 | 1,128 |
| `mixed-record-hero` | glyph | `InstancedGlyphLayer`, `RelationEdgeLayer` | 20 | 5 | 5 | 8.3 ms | 4,808 | 1,015 |
| `relation-matrix-neighborhood` | matrix+graph | `RelationMatrixLayer`, `RelationEdgeLayer` | 130 | 3 | 3 | 8.3 ms | 48,584 | 1,257 |
| `dynamics-noise-hero` | dynamics | `CurveRibbonLayer`, `HeatFieldLayer`, `InstancedPointLayer` | 28 | 4 | 4 | 8.3 ms | 380,208 | 1,193 |
| `mapping-dimensionality-hero` | mapping | `InstancedPointLayer`, `GroundProjectionLayer` | 15 | 5 | 5 | 8.3 ms | 4,032 | 1,075 |
| `cross-space-dependency-hero` | graph | `RelationEdgeLayer`, `InstancedGlyphLayer` | 48 | 4 | 4 | 8.3 ms | 14,136 | 1,003 |

The grammar rows explicitly set `classification: "public-preview-only"` and
`heroAccepted: false`; render/load/performance evidence does not promote them.

## Synthetic Fixtures

Local `visual/examples/*/evidence.json` files remain synthetic development
fixtures. The public pages resolve native assets under `docs/examples/assets/`
instead, and `check-public-gallery-evidence.mjs` reports no public synthetic
examples and no synthetic examples marked done.

## Generated Artifacts

Transient local artifacts were written under:

- `output/visual/check-visual-regression-public-examples/`
- `output/visual/check-visual-performance-large-scenes/results.json`
- `output/visual/check-hero-screenshot-review/results.json`

These files are not baselines and should not be committed unless a future task
explicitly promotes documented baseline artifacts.

## Remaining Gaps

- These gates prove objective mechanics, not visual taste or final hero
  acceptance.
- Public previews other than GRAE10 remain public-preview-only until stronger
  screenshot acceptance criteria exist.
- Grammar performance rows use current public native evidence sizes, which are
  intentionally smaller than large-scene point-cloud stress counts.
- Absolute timing is local-browser evidence; the JSON report records renderer
  details so future comparisons can account for the WebGL backend.
