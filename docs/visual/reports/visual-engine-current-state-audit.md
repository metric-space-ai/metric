# Visual Engine Current-State Gate Audit

Date: 2026-06-26

Scope: factual current-state audit for the METRIC Visual Engine. This report
does not rewrite the Visual Engine plan, does not promote heroes and does not
define new acceptance criteria. It maps the current objective to checked proof
and remaining gaps.

## Current Objective

The current Visual Engine objective is engine-first:

```text
metric.visual.v1 evidence
  -> data/indexing
  -> semantic view object
  -> layer descriptors
  -> MetricVisualRuntime
  -> WebGL layers/postprocess
  -> interaction/preview/selection
```

Hero pages are acceptance fixtures for that engine. A public hero is not
accepted if it requires a custom page render helper, a page-local descriptor
pipeline or synthetic public hero evidence.

## Accepted Public Reference

Accepted public hero/reference: `grae10-metric-engine` only.

Evidence:

- `visual/hero-acceptance.manifest.json` contains one accepted hero:
  `grae10-metric-engine`, accepted on 2026-06-25.
- `node visual/tools/check-grae10-golden.mjs` passed and reported golden hash
  `464f6a90c36c1e9c6b4ec90068500dc226740d65b251918aca567f99d64d3d5e`.
- The GRAE10 page is a protected 60k MNIST/GRAE10 visual reference. The hash
  gate proves the checked reference page did not change. It does not prove a
  from-scratch full-scale retraining/export pipeline.

The current non-GRAE public pages are native public previews, not accepted
heroes.

## Native Public Preview Set

The current public native preview set has seven entries. All seven passed the
native evidence scale gate. There are no current record-count blockers reported
by the gate.

| Preview | Native evidence asset | Scale status | Current blockers |
| --- | --- | --- | --- |
| `condition-monitoring-hero` | `docs/examples/assets/condition-monitoring/metric.visual.json` | Native scale ready: 528 records, 2 relations, 1 record type. Minimum: 500 records, 1 relation. | `visual-composition-not-human-accepted` |
| `mixed-record-hero` | `docs/examples/assets/mixed-records/metric.visual.json` | Native scale ready: 2000 records, 1 relation, 4 record types. Minimum: 2000 records, 1 relation, 4 record types. | `visual-composition-not-human-accepted` |
| `cross-space-dependency-hero` | `docs/examples/assets/cross-space-dependency/metric.visual.json` | Native scale ready: 512 records, 3 relations, 1 record type. Minimum: 500 records, 2 relations. | `visual-composition-not-human-accepted` |
| `relation-matrix-neighborhood` | `docs/examples/assets/relation-matrix/metric.visual.json` | Native scale ready: 130 records, 1 relation, 1 record type. Minimum: 128 records, 1 relation. | `visual-composition-not-human-accepted`; `matrix-readability-not-human-accepted` |
| `dynamics-noise-hero` | `docs/examples/assets/dynamics-noise/metric.visual.json` | Native scale ready: 512 records, 2 relations, 1 record type. Minimum: 500 records, 1 relation. | `visual-composition-not-human-accepted` |
| `mapping-dimensionality-hero` | `docs/examples/assets/mapping-dimensionality/metric.visual.json` | Native scale ready: 1000 records, 2 relations, 1 record type. Minimum: 1000 records, 1 relation. | `visual-composition-not-human-accepted` |
| `process-curve-external-hero` | `docs/examples/assets/process-curve-external/metric.visual.json` | Native scale ready: 576 scale records from metric-relation record ids, 737 total records, 1 relation, 2 record types. Minimum: 500 records, 1 relation. | `visual-composition-not-human-accepted` |

The public gallery evidence gate also reports:

- Public examples: the seven previews above.
- Public native assets: one native `metric.visual.v1` asset for each preview.
- Public synthetic evidence: none.
- Synthetic development fixtures still exist for six example directories, but
  the gate reports none are linked as public evidence and none are marked done
  in progress docs.

## Gate Map

| Gate | Result | What it proves now | What it does not prove |
| --- | --- | --- | --- |
| `node visual/tools/check-grae10-golden.mjs` | Pass | The protected GRAE10 reference page matches the checked SHA-256 baseline. | It does not promote any other page and does not prove a full native retraining/export pipeline for GRAE10. |
| `node visual/tools/check-native-hero-evidence-scale.mjs` | Pass: `ok: true`, 7 rows, 7 native-scale-ready rows, `blockedByRecordCount: []`. | Each public preview brief maps to a readable native `metric.visual.v1` asset with explicit native export provenance, non-synthetic status, and enough records/relations/types for its current brief. | It does not perform browser rendering and does not grant screenshot or human composition acceptance. |
| `node visual/tools/check-public-gallery-evidence.mjs` | Pass: `ok: true`; GRAE10 hash matches; `publicSynthetic: []`; seven public native assets. | The public project page has not replaced the GRAE10 reference and does not publish synthetic fixtures as public hero evidence. | It does not judge visual quality, grammar legibility or hero acceptance. |
| `node visual/tools/check-single-render-pipeline.mjs` | Pass. | Static guard found no forbidden parallel metric-space hero helper, no forbidden direct view imports in `*-hero` pages, and no forbidden trajectory/property/process-curve descriptor ownership in `visual/src/metric-visual.js`. | It is a static pattern gate, not a complete proof that every future path is architecturally clean. |
| `node visual/tools/check-hero-grammar-contract.mjs` | Pass. | The checked hero pages contain required grammar evidence for their preview class and do not use the forbidden generic metric-space hero helper. | It is a static page-content guard; it does not inspect the rendered image or grant acceptance. |
| `node visual/tools/check-hero-visual-briefs.mjs` | Pass: `ok: true`, 7 public previews, 7 briefs, no issues. | Each public preview has a review-pending visual brief, required native evidence, required WebGL primitives and required semantic descriptor roles in the browser regression report. | It does not promote heroes. This audit did not rerun the browser regression command; the brief gate consumed existing `output/visual/check-visual-regression-public-examples/results.json` generated at `2026-06-26T19:35:02.797Z`. |

## Engine, Runtime And Evidence Status

Evidence status:

- GRAE10 accepted reference is hash-stable.
- Seven public non-GRAE previews load native `metric.visual.v1` assets from
  `docs/examples/assets/*/metric.visual.json`.
- All seven preview assets pass native provenance, schema, non-synthetic and
  scale checks.
- No public preview is currently blocked by record count or relation count.

Runtime status:

- The public previews are represented in the browser regression artifact used
  by `check-hero-visual-briefs.mjs`.
- That gate reports the expected primitives and semantic descriptor roles for
  every preview brief.
- Browser load/render proof is not the same as hero readiness.

Engine architecture status:

- The single-pipeline guard currently passes for the scanned source and example
  paths.
- The current accepted exception is the protected GRAE10 reference. Direct
  runtime/probe patterns must remain protected or internal diagnostics, not
  general public hero implementation patterns.
- Public preview work should continue through `createMetricVisual()`, semantic
  `show*()` commands or semantic view objects, not custom page-local render
  helpers.

## Remaining Acceptance Blockers

The blockers before any non-GRAE preview can become an accepted hero are not
scale blockers in the current gate output. They are acceptance blockers:

- All seven non-GRAE previews remain `review-pending`.
- All seven carry `visual-composition-not-human-accepted`.
- `relation-matrix-neighborhood` also carries
  `matrix-readability-not-human-accepted`.
- No non-GRAE preview appears in `visual/hero-acceptance.manifest.json`.
- Synthetic development fixtures must stay out of public hero evidence.

The current state is therefore:

```text
native evidence present + scale gate pass + grammar/brief gates pass
!= accepted public hero
```

Promotion requires a separate acceptance decision backed by screenshot/review
evidence and a manifest update. The gates run here intentionally do not perform
that promotion.

## Concrete Next Work Packages

1. Refresh browser and screenshot review proof for the seven previews.
   Run the browser-heavy public regression and screenshot-review gates, inspect
   the generated review gallery, and record preview-specific accept/reject
   outcomes. Do not mark a preview accepted because it loads or because its
   native evidence passes scale.

2. Fix preview composition through reusable engine owners.
   If a preview fails visual composition, make the change in semantic views,
   layers, runtime, style or interaction modules. The public HTML page should
   stay a thin `createMetricVisual()` plus semantic command/view fixture.

3. Clear relation-matrix readability as a separate engine package.
   `relation-matrix-neighborhood` has a distinct readability blocker. Work
   should stay in relation-matrix semantic view/layer/readability metadata,
   picking and pair-preview support, not DOM/SVG/page-local matrix rendering.

4. Preserve native evidence provenance on any evidence refresh.
   If a preview asset changes, regenerate it from the native C++ exporter,
   keep explicit `provenance.native_export === true`, validate the visual
   document, then rerun native scale and public gallery evidence gates. Do not
   fill gaps with JavaScript-computed or synthetic public hero data.

5. Promote only through the acceptance manifest after review.
   A non-GRAE preview becomes accepted only when its visual composition blocker
   is removed by review, its evidence gates still pass, its screenshot/review
   artifact is current, and `visual/hero-acceptance.manifest.json` is updated
   deliberately. Until then, GRAE10 remains the only accepted public reference.

## Command Results

Commands run from `/Users/michaelwelsch/Documents/metric`:

| Command | Result summary |
| --- | --- |
| `node visual/tools/check-grae10-golden.mjs` | Exit 0. `GRAE10 golden reference OK: 464f6a90c36c1e9c6b4ec90068500dc226740d65b251918aca567f99d64d3d5e`. |
| `node visual/tools/check-native-hero-evidence-scale.mjs` | Exit 0. `ok: true`; generated at `2026-06-26T19:43:45.480Z`; `rowCount: 7`; `nativeScaleReadyCount: 7`; `blockedByRecordCount: []`; no issues. |
| `node visual/tools/check-public-gallery-evidence.mjs` | Exit 0. `ok: true`; GRAE10 hash matched; seven public preview examples; seven public native assets; `publicSynthetic: []`; `syntheticDone: []`; no issues. |
| `node visual/tools/check-single-render-pipeline.mjs` | Exit 0. `Single render pipeline check passed.` |
| `node visual/tools/check-hero-grammar-contract.mjs` | Exit 0. `Hero visual grammar check passed.` |
| `node visual/tools/check-hero-visual-briefs.mjs` | Exit 0. `ok: true`; generated at `2026-06-26T19:43:45.232Z`; `publicPreviewCount: 7`; `briefCount: 7`; `blockedByRecordCount: []`; no issues. |

No requested command was skipped as too slow.
