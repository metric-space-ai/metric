# Hero Screenshot Review Workstream

Status date: 2026-06-25

## Scope

Added a gate that separates browser screenshot generation from hero acceptance.
The gate does not judge visual taste and does not promote public previews. It
checks that screenshots exist, that the browser regression report is green, and
that only explicitly reviewed examples are marked as hero-accepted.

## Implemented

- Added `visual/tools/check-hero-screenshot-review.mjs`.
- The gate consumes
  `output/visual/check-visual-regression-public-examples/results.json`.
- It verifies page and canvas screenshots for each public example.
- It accepts the protected GRAE10 reference by default because the native 60k
  evidence and golden visual hash are already enforced.
- Optional future manual acceptances belong in
  `docs/visual/hero-screenshot-review.json`.

## Verification

- `node --check visual/tools/check-hero-screenshot-review.mjs`
- `node visual/tools/check-hero-screenshot-review.mjs`

## Current Status

GRAE10 is the only accepted hero. Condition monitoring, mixed records,
cross-space dependency, relation matrix, dynamics and mapping stay
review-pending until screenshot review accepts each grammar.
