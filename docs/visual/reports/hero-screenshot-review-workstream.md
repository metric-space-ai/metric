# Hero Screenshot Review Workstream

Status date: 2026-06-26

## Scope

Added a gate that separates browser screenshot generation from hero acceptance.
The gate does not judge visual taste and does not promote public previews. It
checks that screenshots exist, that the browser regression report is green, and
that only explicitly reviewed examples are marked as hero-accepted.

## Implemented

- Added `visual/tools/check-hero-screenshot-review.mjs`.
- The gate consumes
  `output/visual/check-visual-regression-public-examples/results.json`.
- It verifies that each public example has a usable canvas screenshot. Page
  screenshots are optional because Chrome DevTools full-page capture can time
  out for headless WebGL scenes while the canvas capture still proves the
  rendered visual artifact.
- It accepts the protected GRAE10 reference by default because the native 60k
  evidence and golden visual hash are already enforced.
- Optional future manual acceptances belong in
  `visual/hero-acceptance.manifest.json`.
- The gate also writes a static browser-review artifact:
  `output/visual/check-hero-screenshot-review/index.html`.
  It lays out page screenshots when available, canvas screenshots, visual
  claims, primary grammar, native-evidence state and open blockers. This is
  review evidence only; it does not accept a hero automatically.

## Verification

- `node --check visual/tools/check-hero-screenshot-review.mjs`
- `node visual/tools/check-hero-screenshot-review.mjs`
- generated gallery image-reference check:
  `output/visual/check-hero-screenshot-review/index.html` references the
  available visual review artifacts; current headless WebGL runs require the
  canvas images and treat page images as optional.

## Current Status

GRAE10 is the only accepted hero. Condition monitoring, mixed records,
cross-space dependency, relation matrix, dynamics, mapping and external
process curves stay review-pending until visual review accepts each grammar.
