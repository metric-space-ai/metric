# Mapping Residual Composition Acceptance

Date: 2026-06-26

## Summary

`MappingView` now supports an engine-level residual composition pass for
representative residual vectors. The mapping hero still loads only native
evidence from:

```text
docs/examples/assets/mapping-dimensionality/metric.visual.json
```

No page-local mapping algorithm was added, and the page does not rewrite or
clear native evidence. GRAE10 files and the hero acceptance manifest were not
edited.

## Implementation

- Added residual representative selection in `MappingView` using native residual
  values, residual buckets, angular sectors and radial buckets.
- Added reusable residual vector controls for sample limit, layer order, alpha
  scaling, residual magnitude scaling and length clamping.
- Kept the default `MappingView` residual behavior compatible: consumers get
  the complete residual vector set unless they explicitly request sampling.
- Updated `mapping-dimensionality-hero` to call `showMapping()` with engine
  options only:
  - `residualVectorLimit: 120`
  - native residual candidates: `1000`
  - residual property: `local-mapping-distortion`
  - residual strategy: `representative-residual-buckets`
  - residual vector max length: `0.62`
  - residual layer order: `-1`, behind the point structure
- Added `visual/tools/check-mapping-composition.mjs` to prove the mapping
  composition contract.

## Descriptor Evidence

The final mapping composition emits:

- `GroundPlaneLayer`
- `GroundProjectionLayer`
- `BillboardLabelLayer`
- `InstancedPointLayer`
- `RelationEdgeLayer`

The residual layer reports:

```json
{
  "recordCount": 120,
  "nativeResidualRecordCount": 1000,
  "residualPropertyId": "local-mapping-distortion",
  "residualSelection": {
    "strategy": "representative-residual-buckets",
    "candidateCount": 1000,
    "selectedCount": 120,
    "limit": 120,
    "residualBucketCount": 5,
    "angularSectorCount": 18,
    "radialBucketCount": 3
  }
}
```

The mapping motion timing keeps the required hold/transition/hold profile:

```json
{
  "profile": "source-hold-quick-transition-target-hold",
  "sourceHoldMs": 1600,
  "transitionMs": 720,
  "targetHoldMs": 1700,
  "resetHoldMs": 360,
  "totalMs": 5200
}
```

## Acceptance Checks

Passed:

- `node --check visual/src/views/MappingView.js`
- `node --check visual/tools/check-mapping-composition.mjs`
- `node visual/tools/check-mapping-composition.mjs`
  - `ok: true`
  - `18` checks, `0` failures
- `node visual/tools/check-mapping-motion-grammar.mjs`
  - `ok: true`
  - `29` checks, `0` failures
- `node visual/tools/check-grae10-golden.mjs`
  - GRAE10 hash: `464f6a90c36c1e9c6b4ec90068500dc226740d65b251918aca567f99d64d3d5e`
- `node visual/tools/check-visual-command-api.mjs`
  - `ok: true`
- `node visual/tools/check-views.mjs`
  - `ok: true`
  - `80` checks, `0` failures
- `node visual/tools/check-hero-visual-briefs.mjs`
  - `ok: true`
  - `mapping-dimensionality-hero` remains `review-pending`
- `METRIC_VISUAL_EXAMPLES=mapping-dimensionality-hero node visual/tools/check-visual-regression-public-examples.mjs`
  - `ok: true`
  - `1` example, `0` failures
  - mapping residual layer: `120` residual representatives from native evidence
  - `heroAccepted: false`
- `node visual/tools/check-visual-regression-public-examples.mjs`
  - `ok: true`
  - `8` examples, `0` failures
  - run after the mapping-only check so screenshot review had full manifest context
- `node visual/tools/check-hero-screenshot-review.mjs`
  - `ok: true`
  - accepted hero: `grae10-metric-engine`
  - `mapping-dimensionality-hero` remains `review-pending`
  - blocker remains `visual-composition-not-human-accepted`
- `git diff --check -- visual/src/views/MappingView.js visual/examples/mapping-dimensionality-hero/index.html visual/tools/check-mapping-composition.mjs`

## Acceptance Status

The mapping preview is not marked as hero-accepted. The acceptance manifest was
not edited. The remaining state is intentionally:

```text
visual-composition-not-human-accepted
```

## Relation-Edge Legibility Update

Date: 2026-06-26

Updated the reusable mapping residual vector descriptor path without changing
native residual evidence or adding a residual fallback.

Additional descriptor evidence now exposed by `MappingView`:

- residual vectors include `endpointEmphasis` derived from exported residual
  magnitude;
- `edgeLegibility.rank` declares deterministic native residual-magnitude
  representative ranking;
- `edgeLegibility.laneBundle` keeps residual lanes disabled while naming the
  representative residual bucket grouping;
- `edgeLegibility.sampling` records 120 rendered representatives from 1,000
  native residual candidates for the composition preview;
- residual vectors still require explicit `local-mapping-distortion` evidence,
  and the no-residual case still emits no residual/error layer;
- `RelationEdgeLayer.getDiagnostics()` can report applied alpha and emphasis
  summaries for runtime checkers.

Verification run for this update:

```bash
node visual/tools/check-linked-edge-emphasis.mjs
node visual/tools/check-mapping-motion-grammar.mjs
node visual/tools/check-mapping-composition.mjs
```

All passed. The acceptance blocker remains
`visual-composition-not-human-accepted`.
