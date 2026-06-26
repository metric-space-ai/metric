# Project Site Copy And Portfolio Contract

## Purpose

Make the public project page read like a visual product portfolio instead of a
test report, prompt trace, or internal implementation log.

The project page may expose source paths and native evidence links, but the
primary on-page language must be user-facing. Internal review state can stay in
`data-*` attributes for gates and tooling.

## Owner Scope

- `docs/site/index.html`
- optional checker under `visual/tools/check-project-site-copy-contract.mjs`
- optional report under `docs/visual/reports/project-site-copy-contract.md`

Do not edit visual runtime, native exporters, GRAE10 reference files, or native
evidence assets in this task.

## Required Outcome

The page must:

- lead with the protected 60k MNIST/GRAE10 live visual;
- move directly into the portfolio/gallery;
- use short user-facing labels, not debugging or plan labels;
- avoid visible wording such as `review-pending`, `Visual:`, `Evidence:`,
  `Notes:`, `prompt`, `TODO`, `placeholder`, `fake`, `intent`, or
  `should/will` future-tense claims;
- keep links to live visuals, native C++ sources and evidence JSON;
- keep GRAE10 as the only accepted reference;
- keep all non-GRAE examples visibly not accepted as final heroes without using
  internal review jargon as user-facing copy;
- not add Python code snippets unless a real binding path is present and
  verified.

## Acceptance

Run:

```bash
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-visual-command-api.mjs
node visual/tools/check-project-site-copy-contract.mjs
```

If adding the checker is out of scope, document exactly what manual checks were
performed in the report.
