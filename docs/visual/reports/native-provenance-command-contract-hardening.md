# Native Provenance And Command Contract Hardening

## Summary

- Added `visual/src/data/provenance.js` as the single implementation for explicit native-export detection.
- Accepted native-export spellings are limited to boolean `provenance.native_export === true` and `provenance.nativeExport === true`.
- Writer paths, runtime labels, computation strings, and C++ text hints are ignored by the helper and covered by the command gate self-check.
- Public gallery evidence now checks every site-linked public example except protected GRAE10, including `process-curve-external-hero`, for native `docs/examples/assets/*/metric.visual.json` evidence with explicit native export.
- Public synthetic JSON references remain rejected by resolving fetched JSON targets and checking synthetic provenance.
- Command API coverage now fails if a non-exempt site-linked public example is missing from the expected-command map.
- Command API checks now reject page-local material rewrites of fetched evidence before `createMetricVisual`.

## Scope Notes

- No examples, project page, native exporters, GRAE10 output, runtime, layers, or views were changed.
- No out-of-owner follow-up was required.
- GRAE10 remains protected by the existing golden hash.

## Changed Files

- `visual/src/data/provenance.js`
- `visual/src/metric-visual.js`
- `visual/tools/check-public-gallery-evidence.mjs`
- `visual/tools/check-native-hero-evidence-scale.mjs`
- `visual/tools/check-visual-command-api.mjs`
- `visual/tools/check-visual-regression-public-examples.mjs`
- `docs/visual/reports/native-provenance-command-contract-hardening.md`

## Validation

| Command | Result |
| --- | --- |
| `node --check visual/src/metric-visual.js` | PASS |
| `node visual/tools/check-public-gallery-evidence.mjs` | PASS, `ok: true`, 7 non-GRAE10 public examples checked, 0 issues |
| `node visual/tools/check-native-hero-evidence-scale.mjs` | PASS, `ok: true`, 7 rows, 0 issues |
| `node visual/tools/check-visual-command-api.mjs` | PASS, `ok: true`, 13 command cases, 0 failures |
| `node visual/tools/check-visual-regression-public-examples.mjs` | PASS, `ok: true`, 8 public examples, 0 failed |
| `node visual/tools/check-grae10-golden.mjs` | PASS, GRAE10 hash `464f6a90c36c1e9c6b4ec90068500dc226740d65b251918aca567f99d64d3d5e` |

Additional local checks:

| Command | Result |
| --- | --- |
| `node --check visual/src/data/provenance.js && node --check visual/tools/check-public-gallery-evidence.mjs && node --check visual/tools/check-native-hero-evidence-scale.mjs && node --check visual/tools/check-visual-command-api.mjs && node --check visual/tools/check-visual-regression-public-examples.mjs` | PASS |
| `git diff --check -- visual/src/metric-visual.js visual/src/data/provenance.js visual/tools/check-public-gallery-evidence.mjs visual/tools/check-native-hero-evidence-scale.mjs visual/tools/check-visual-command-api.mjs visual/tools/check-visual-regression-public-examples.mjs` | PASS |
