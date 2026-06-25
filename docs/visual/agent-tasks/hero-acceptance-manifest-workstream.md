# Agent Task: Hero Acceptance Manifest Workstream

## Purpose

Make hero acceptance explicit and reproducible. A page that loads, renders,
uses native evidence, and passes grammar gates is still only a preview until a
review manifest accepts its screenshot and evidence.

This task strengthens the gate around public hero promotion. It does not polish
a visual and it does not promote any preview by default.

## Owner Scope

One worker owns only:

```text
visual/tools/check-hero-screenshot-review.mjs
visual/hero-acceptance.manifest.json
docs/visual/hero-acceptance-manifest.md
docs/visual/reports/hero-acceptance-manifest-workstream.md
```

Do not edit public examples, project pages, GRAE10 data, renderer layers,
native exporters, generated screenshots, or `output/`.

## Required Behavior

- Add a small checked-in manifest format for accepted visual heroes.
- GRAE10 must remain accepted through the existing protected hash.
- All other public previews must remain `review-pending` unless explicitly
  listed in the manifest with:
  - example id
  - native evidence requirement
  - expected primary visual grammar
  - screenshot file or generated report reference
  - human acceptance status and date
  - short visual acceptance reason
- `check-hero-screenshot-review.mjs` must read the manifest when present and
  report:
  - accepted heroes
  - review-pending previews
  - manifest entries whose evidence, grammar or screenshot contract no longer
    matches the browser regression report
- The tool must not treat successful smoke/render/performance checks as hero
  acceptance.
- Do not commit generated screenshots as baselines in this task.

## Stop Rules

- Stop if acceptance requires visual taste judgment by the script alone.
- Stop if the implementation would auto-promote all native previews.
- Stop if the manifest weakens GRAE10 protection or bypasses
  `check-public-gallery-evidence.mjs`.

## Acceptance

Run:

```bash
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-hero-screenshot-review.mjs
```

Expected current result:

- accepted hero count remains `1`
- accepted hero remains `grae10-metric-engine`
- the six other public examples remain review-pending

## Report

Write `docs/visual/reports/hero-acceptance-manifest-workstream.md` with:

- manifest schema
- checks run
- accepted and pending counts
- explicit statement that no preview was promoted
- remaining work required before a second hero can be accepted
