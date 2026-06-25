# Agent Task: Relation Matrix Hero Readability

## Owner Scope

You own only:

- `visual/src/views/RelationMatrixView.js`
- `visual/src/layers/RelationMatrixLayer.js`
- `visual/src/interaction/` files only if required for matrix cell preview
- `visual/tools/check-relation-matrix-readability.mjs`
- `visual/tools/check-relation-matrix-picker.mjs`
- optional report: `docs/visual/reports/relation-matrix-hero-readability.md`

Do not edit native exporters, project pages, GRAE10 or unrelated layers. You
are not alone in the worktree: do not revert changes made by other agents, and
adapt to current files.

## Objective

Make the existing scale-ready `relation-matrix-neighborhood` preview closer to
hero acceptance without changing evidence counts. The blocker is readability,
not record count. Improve the reusable relation-matrix grammar so dense pair
relations are legible as native pair evidence with row/column/cell focus and
pair preview, not as texture noise.

## Required Inputs

Read:

- `docs/visual/visual-engine-masterplan.md`
- `docs/visual/metric-visual-engine-implementation-plan.md`
- `docs/visual/reports/native-hero-scale-evidence-upgrade-plan.md`
- `docs/visual/reports/relation-matrix-readability-workstream.md`
- `visual/examples/relation-matrix-neighborhood/index.html`
- `docs/examples/assets/relation-matrix/metric.visual.json`
- `visual/hero-visual-briefs.manifest.json`

## Required Behavior

- Keep relation-matrix rendering inside engine modules.
- Preserve native pair identity; JavaScript may build GPU textures/LOD from
  exported values but must not compute metric values or neighborhoods.
- Improve readability using engine-level techniques only:
  - block bands or visible block boundaries
  - tile/LOD smoothing that avoids moire
  - row/column/cell focus
  - clear selected pair preview metadata
  - linked graph emphasis if the selected pair is present in the graph
- Do not add DOM/SVG fallbacks or page-local canvas renderers.
- Keep `relation-matrix-neighborhood` review-pending; do not mark it accepted.

## Acceptance Commands

```bash
node visual/tools/check-relation-matrix-readability.mjs
node visual/tools/check-relation-matrix-picker.mjs
node visual/tools/check-native-hero-evidence-scale.mjs
node visual/tools/check-hero-screenshot-review.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-visual-performance-large-scenes.mjs
```

## Stop Rules

Stop and write the report if the current exported evidence lacks pair metadata
needed for preview or if a visual change would require page-local rendering.

## Report

Write `docs/visual/reports/relation-matrix-hero-readability.md` with:

- changed files
- readability improvements made at engine level
- commands run and results
- screenshot paths from `output/visual/` if generated
- remaining human screenshot blocker
