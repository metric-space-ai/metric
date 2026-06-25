# Agent Task: Project Gallery With Native Evidence

## Owner

One worker owns only:

- `docs/site/index.html`
- `docs/site/*` assets required by the gallery shell
- gallery-specific checks under `visual/tools/check-project-gallery-*.mjs`
- documentation links to already accepted visual examples

Do not edit GRAE10, native exporters, visual runtime, semantic views, example
rendering code or algorithm code. Other agents may be working in parallel; do
not revert their changes.

## Goal

Turn the project page into a simple visual portfolio for METRIC evidence:
visual first, code path second, theory only where needed.

The gallery must not claim fake heroes. It may show accepted heroes and clearly
labeled review-pending previews.

## Product Direction

Use the D3-style pattern:

- first viewport shows the accepted 60k MNIST/GRAE10 visual reference
- gallery cards show live visual examples or accepted captures
- each item links to the native C++ evidence/code path when real
- no fake Python snippets unless real bindings exist
- no wall of text before the visual proof

## Allowed Public Status

Accepted hero:

- MNIST dimension reduction, protected GRAE10 reference

Review-pending native previews:

- condition monitoring
- mixed records
- cross-space dependency
- mapping/dimensionality
- finite dynamics/noise
- relation matrix/neighborhood
- process-curve external, with its source-record-count blocker visible in
  maintainer docs, not as marketing copy

## Hard Stop Rules

- Do not publish synthetic fixture data as a hero.
- Do not add invented code snippets.
- Do not duplicate the GRAE10 hero twice.
- Do not replace the protected GRAE10 reference.
- Do not use prompt-history wording, agent status language or debug labels as
  user-facing copy.

## Acceptance

- The first screen is visual, not a text wall.
- GRAE10 is visible as the accepted visual reference.
- Gallery entries link to real native evidence examples or are plainly marked
  review-pending.
- Each code panel is backed by an existing C++ source/example path or omitted.
- No Python tab is shown unless a real binding/example exists.
- `check-public-gallery-evidence.mjs` passes.

## Validation

Run:

```bash
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-hero-screenshot-review.mjs
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-visual-regression-public-examples.mjs
```

Browser-open `docs/site/index.html` and capture/report the first viewport and
gallery section. The report must list every gallery item and its evidence
status.
