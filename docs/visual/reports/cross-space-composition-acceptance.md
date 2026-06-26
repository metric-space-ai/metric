# Cross-Space Composition Acceptance

Generated: 2026-06-26

## Scope

Updated the reusable paired-space visual path and the cross-space dependency
preview only. The preview still uses:

```text
docs/examples/assets/cross-space-dependency/metric.visual.json
```

No native exporter, GRAE10, hero acceptance manifest, or other hero page files
were edited.

## Changes

- `CrossSpaceView` now reads the exported
  `cross-space-dependence-bridge-relation` values as visible bridge evidence.
- The view preserves the full 512 exported paired records as the linked
  selection graph while rendering the 96 exported high-contribution bridge
  records as uncluttered visible relation geometry.
- Bridge descriptors now expose explicit sampling/count metadata under
  `metric.visual.cross_space_bridge_sampling.v1`, including candidate pairs,
  exported bridge count, rendered bridge count, graph edge count, relation id,
  graph id, and native-evidence flags.
- The bridge material uses exported relation values for alpha/color/emphasis
  scaling without computing dependence or correlations in JavaScript.
- `CrossSpaceView` emits reusable paired-space label anchors and wider shared
  staging so the two finite metric spaces read as separate domains.
- The hero page passes composition options into `showCrossSpace` and reads pair
  preview/bridge counts from runtime descriptors and shared selection
  presentation instead of drawing page-local bridge helpers.
- Added `visual/tools/check-cross-space-composition.mjs` to assert the
  cross-space composition contract.

## Evidence Summary

- Record pairs: 512
- Exported visible bridge relation values: 96
- Runtime graph-pickable bridge edges in browser regression: 96
- Selected public view kind: `cross-space`
- Semantic view kind: `paired-space`
- Runtime layers after update: 5
- Review status remains `review-pending`
- Acceptance blocker remains `visual-composition-not-human-accepted`

## Checks

Passed:

```bash
node visual/tools/check-cross-space-composition.mjs
node visual/tools/check-cross-space-linked-selection.mjs
node visual/tools/check-linked-selection-presentation.mjs
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-hero-visual-briefs.mjs
METRIC_VISUAL_EXAMPLES=cross-space-dependency-hero node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-hero-screenshot-review.mjs
```

The full public regression was run after the scoped cross-space regression
because the scoped command overwrites the shared regression report that the
aggregate review tools consume.

Blocked by unrelated out-of-scope `DynamicsView` errors:

```bash
node visual/tools/check-visual-command-api.mjs
node visual/tools/check-views.mjs
```

Observed failures:

```text
ReferenceError: scalarDomainForValues is not defined
  at new DynamicsView (visual/src/views/DynamicsView.js:84:63)

ReferenceError: firstPropertyRef is not defined
  at DynamicsView.fromVisualSpace (visual/src/views/DynamicsView.js:229:38)
```

`visual/src/views/DynamicsView.js` is outside this task's assigned write scope.

## Acceptance Status

The cross-space composition-specific checks and browser regression pass. The
preview remains review-pending and was not marked accepted.

## Relation-Edge Legibility Update

Date: 2026-06-26

Updated the reusable paired-space bridge descriptor path without changing the
native bridge relation, the full 512-record linked selection graph, or the
public page.

Additional descriptor evidence now exposed by `CrossSpaceView`:

- visible bridge geometry remains the 96 exported high-contribution bridge
  relation values;
- the linked selection graph remains all 512 paired records;
- bridge channels include `endpointEmphasis` from exported dependence values;
- `edgeLegibility.rank` declares deterministic native dependence-value ranking;
- `edgeLegibility.laneBundle` declares paired-observation lanes using
  `sourceId:targetId:pairSetId`;
- `edgeLegibility.sampling` explicitly records 96 rendered bridges from 512
  source graph edges, with `preservesSelectionGraph: true`;
- `RelationEdgeLayer.getDiagnostics()` can report applied alpha, lane and
  emphasis summaries for runtime checkers.

Verification run for this update:

```bash
node visual/tools/check-linked-edge-emphasis.mjs
node visual/tools/check-cross-space-linked-selection.mjs
node visual/tools/check-cross-space-composition.mjs
```

All passed. The acceptance blocker remains
`visual-composition-not-human-accepted`.
