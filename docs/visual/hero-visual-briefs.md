# Hero Visual Briefs

Public previews are not accepted heroes. Each public preview must still have a
visual brief so that future work starts from a visual claim and evidence
contract instead of from a generic rendered page.

The checked-in brief manifest is:

```text
visual/hero-visual-briefs.manifest.json
```

The validation gate is:

```bash
node visual/tools/check-hero-visual-briefs.mjs
```

## Purpose

The brief manifest records, per public preview:

- the one-sentence visual claim
- the expected primary visual grammar from the browser regression report
- the primary visual grammar in human terms
- native evidence requirements
- minimum evidence size for hero acceptance
- required WebGL layer primitives
- explicit blockers that keep the preview from being accepted as a hero

This gate does not promote a preview. It prevents a preview from becoming an
untracked rendered artifact with no claim, no acceptance blocker and no
evidence-size target.

## Gate Behavior

`check-hero-visual-briefs.mjs` reads:

```text
output/visual/check-visual-regression-public-examples/results.json
visual/hero-visual-briefs.manifest.json
```

It verifies that every `public-preview-only` page in the browser regression
report has a matching brief. For each brief it checks:

- `reviewStatus` is `review-pending`
- native evidence is required
- the expected grammar matches the browser regression grammar
- all required layer primitives are present in the runtime descriptor list
- minimum relation count is satisfied
- if record count is below the hero target, the blocker
  `record-count-below-hero-minimum` is explicitly present
- `visual-composition-not-human-accepted` remains present until screenshot
  acceptance exists

## Current State

Current checked previews: `7`.

Native scale status: all seven current public previews meet their manifest
evidence targets. `check-native-hero-evidence-scale.mjs` reports
`nativeScaleReadyCount: 7` and `blockedByRecordCount: []`.

Remaining acceptance blockers are visual review blockers, not scale blockers.
All seven previews remain blocked by `visual-composition-not-human-accepted`.
`relation-matrix-neighborhood` also remains blocked by
`matrix-readability-not-human-accepted`.
