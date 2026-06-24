# Visual Regression And Performance Gate Report

Date: 2026-06-24

## Scope

This workstream added dependency-free browser gates under `visual/tools/`:

- `visual/tools/check-visual-regression-public-examples.mjs`
- `visual/tools/check-visual-performance-large-scenes.mjs`

The browser runner uses an installed Chrome/Chromium through the Chrome DevTools
Protocol and writes transient artifacts under `output/visual/`. No baselines are
generated or updated.

## Acceptance Commands

Passed:

```bash
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-single-render-pipeline.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-visual-performance-large-scenes.mjs
```

## Public Preview Classification

| Preview | Classification | Browser gate | Evidence | Runtime layers | Draw calls |
| --- | --- | ---: | --- | ---: | ---: |
| `grae10-metric-engine` | hero-accepted | pass | protected 60k GRAE10 reference hash + dataset shape | n/a | 156 |
| `condition-monitoring-hero` | public preview only | pass | native `metric.visual.v1`, 15 records | 6 | 246 |
| `mixed-record-hero` | public preview only | pass | native `metric.visual.v1`, 20 records | 5 | 235 |
| `cross-space-dependency-hero` | public preview only | pass | native `metric.visual.v1`, 48 records | 4 | 224 |
| `relation-matrix-neighborhood` | public preview only | pass | native `metric.visual.v1`, 130 records | 3 | 177 |
| `dynamics-noise-hero` | public preview only | pass | native `metric.visual.v1`, 28 records | 3 | 177 |
| `mapping-dimensionality-hero` | public preview only | pass | native `metric.visual.v1`, 15 records | 5 | 235 |

The regression gate explicitly keeps hero acceptance separate from load/render.
Only the protected GRAE10 reference is marked hero-accepted. The other public
views are classified as public previews even when browser load, nonblank render,
native evidence, intended grammar, and pointer interaction all pass.

## Synthetic Fixtures

The following local fixtures remain synthetic development inputs and are not
treated as public hero evidence:

- `visual/examples/condition-monitoring-hero/evidence.json` (1536 records)
- `visual/examples/cross-space-dependency-hero/evidence.json` (1100 records)
- `visual/examples/dynamics-noise-hero/evidence.json` (640 records)
- `visual/examples/mapping-dimensionality-hero/evidence.json` (2304 records)
- `visual/examples/mixed-record-hero/evidence.json` (2100 records)
- `visual/examples/relation-matrix-neighborhood/evidence.json` (144 records)

## Performance Probe

`check-visual-performance-large-scenes.mjs` records record count, descriptor
count, runtime layer count, frame timing, GPU buffer uploads, and draw-count
diagnostics.

| Records | Result | Median frame | Frames sampled | Runtime layers | Buffer bytes | Draw calls | Renderer | Notes |
| ---: | --- | ---: | ---: | ---: | ---: | ---: | --- | --- |
| 1,000 | pass | 8.3 ms | 421 | 1 | 52,096 | 527 | ANGLE Metal | within budget |
| 10,000 | pass | 8.3 ms | 421 | 1 | 520,096 | 516 | ANGLE Metal | within budget |
| 60,000 | pass | 8.3 ms | 421 | 1 | 3,120,096 | 524 | ANGLE Metal | within budget |

The performance tool no longer forces SwiftShader by default. It records the
actual WebGL backend in the result and still supports
`METRIC_VISUAL_FORCE_SWIFTSHADER=1` when a software-renderer comparison is
needed.

## Generated Artifacts

Transient artifacts were written under:

- `output/visual/check-visual-regression-public-examples/`
- `output/visual/check-visual-performance-large-scenes/results.json`

These files are for local review and should not be committed as baselines.

## Open

The browser gates prove load, render, native-evidence use, intended grammar,
interaction, GRAE10 protection and large-scene frame budget. They do not prove
visual taste or hero acceptance. Only GRAE10 remains hero-accepted; the other
public examples remain public previews until they receive stronger datasets and
manual screenshot review against their visual briefs.
