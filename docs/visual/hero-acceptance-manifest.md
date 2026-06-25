# Hero Acceptance Manifest

Hero acceptance is explicit review state, not a side effect of smoke tests,
render checks, screenshot generation, native evidence, or performance gates.
`visual/hero-acceptance.manifest.json` is the checked-in source of truth for
public visual heroes that have been accepted after review.

GRAE10 remains the only accepted hero until a later manifest change explicitly
accepts another example. The GRAE10 entry still depends on the protected golden
hash and native 60k dataset contract enforced by the existing GRAE10 gates.

## Schema

The manifest is a JSON object with:

- `schemaVersion`: currently `1`.
- `acceptedHeroes`: array of manually accepted hero entries.

Each `acceptedHeroes` entry must include:

- `exampleId`: public visual example id from the browser regression report.
- `requiresNativeEvidence`: must be `true`; synthetic evidence cannot satisfy
  public hero acceptance.
- `expectedPrimaryVisualGrammar`: exact grammar label expected in
  `output/visual/check-visual-regression-public-examples/results.json`.
- `screenshot`: either a generated report reference or an exact screenshot file
  contract. Current entries use:
  `generatedReport`,
  `requiresPageScreenshot`,
  `requiresCanvasScreenshot`.
- `humanAcceptance`: object with `status: "accepted"` and an ISO date
  (`YYYY-MM-DD`).
- `acceptanceReason`: short human-readable reason for accepting the visual.

## Gate Behavior

`node visual/tools/check-hero-screenshot-review.mjs` reads the manifest and the
browser regression report. It reports:

- accepted heroes from the manifest plus the protected GRAE10 fallback;
- public previews that are still `review-pending`;
- manifest entries missing from the regression report;
- accepted entries whose native evidence, grammar label, screenshot contract,
  render state, or GRAE10 protection no longer matches the regression report;
- any regression report that marks a hero accepted without review.

Passing smoke, render, screenshot, native evidence, or performance checks does
not promote a preview. A second hero requires a new manifest entry with human
acceptance status, date, reason, native evidence requirement, expected grammar,
and screenshot/report contract.

If a public preview still exists in
`visual/hero-visual-briefs.manifest.json`, all `acceptanceBlockers` must be
removed before it can be accepted. The screenshot-review gate rejects an
acceptance manifest entry that still has open visual-brief blockers or
`reviewStatus: "review-pending"`.
