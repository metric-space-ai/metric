# Agent Task: Project Site Visual Portfolio Redesign

## Owner

One worker owns only:

```text
docs/site/index.html
docs/site/*
visual/tools/check-project-gallery-*.mjs
docs/visual/reports/project-site-visual-portfolio-redesign.md
```

Do not edit GRAE10, visual examples, runtime, views, native exporters, schema,
hero evidence assets or public-gate tools outside the optional
`check-project-gallery-*` scope. Other agents may work in parallel; do not
revert their changes.

## Goal

Make the project page a visual portfolio for METRIC, not a wall of text.

The first viewport must lead with the accepted 60k MNIST/GRAE10 visual reference
and a concise explanation. A gallery must follow quickly, with each item showing
its current evidence status and linking to the real visual example and native
C++ source path when available.

## Content Rules

- No prompt-history wording, agent-status language, debug labels or internal
  implementation chatter in user-facing copy.
- No fake code snippets.
- No Python tab unless a real binding/example exists for that item.
- Do not duplicate the GRAE10 hero twice.
- Review-pending previews may be shown only as native previews, not accepted
  heroes.
- Keep theory concise and below the visual proof.

## Visual Direction

- Use the current METRIC miniature visual identity from the accepted visual
  reference as the page's visual tone.
- First screen: brand, one short statement, accepted GRAE10 visual reference,
  direct links to gallery and code.
- Gallery: compact D3-style portfolio grid/list with live/example links and
  C++ evidence paths.
- Code panel: show only real existing C++ paths or omit the panel.

## Required Evidence Status

Accepted:

- `grae10-metric-engine`

Review-pending native previews:

- `condition-monitoring-hero`
- `mixed-record-hero`
- `cross-space-dependency-hero`
- `mapping-dimensionality-hero`
- `dynamics-noise-hero`
- `relation-matrix-neighborhood`
- `process-curve-external-hero` is native-scale-ready but still review-pending;
  do not market it as an accepted hero.

## Validation

Run:

```bash
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-hero-screenshot-review.mjs
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-visual-regression-public-examples.mjs
```

Open `docs/site/index.html` in a browser and capture or describe:

- first viewport composition
- gallery section
- exact gallery items and evidence status

Write:

```text
docs/visual/reports/project-site-visual-portfolio-redesign.md
```
