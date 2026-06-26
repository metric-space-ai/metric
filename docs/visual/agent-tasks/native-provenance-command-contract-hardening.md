# Agent Task: Native Provenance And Command Contract Hardening

## Owner

One worker owns only:

```text
visual/src/metric-visual.js
visual/src/data/*
visual/tools/check-public-gallery-evidence.mjs
visual/tools/check-visual-command-api.mjs
visual/tools/check-visual-regression-public-examples.mjs
visual/tools/check-native-hero-evidence-scale.mjs
visual/tools/check-hero-visual-briefs.mjs
docs/visual/reports/native-provenance-command-contract-hardening.md
```

Do not edit examples, project page, native exporters, GRAE10, runtime, layers or
semantic views unless the parent explicitly assigns that follow-up. Other
agents may work in parallel; do not revert their changes.

## Goal

Make public native-evidence and command use mechanically enforceable.

The current plan requires explicit native provenance:

```text
provenance.native_export === true
```

Equivalent camelCase output may be accepted only through one centralized helper
that documents the accepted spellings. Runtime strings, writer path names or
mentions of C++ must not be enough to classify evidence as native.

The public gallery must also be fully covered by the command contract. A public
example that is not GRAE10-protected must have an expected semantic command in
the command gate. If a site-linked example is missing from the expected-command
map, the gate must fail instead of accepting any command.

## Required Fixes

1. Add or consolidate one helper for explicit native-export detection.
   - It must accept the actual writer spelling used by current native assets.
   - It must reject text-only hints such as writer paths or runtime labels.
   - It must be reused by public-gallery, native-scale and regression checks
     where practical.

2. Harden public gallery evidence checks.
   - Public native assets must prove explicit native export.
   - Public synthetic fixture references must remain rejected.
   - GRAE10 remains the protected accepted reference and must not be modified.

3. Harden command API coverage.
   - Every site-linked public example except explicitly exempt GRAE10/internal
     references must appear in the expected command map.
   - A page-local script may not rewrite native evidence into a materially
     different public result before calling the command.
   - If page-local mutation is found, either fail the gate or document a precise
     allowed exception with a reason.

4. Write a concise report in:

```text
docs/visual/reports/native-provenance-command-contract-hardening.md
```

## Stop Rules

- Do not change accepted GRAE10 output or its hash.
- Do not loosen gates to make current pages pass.
- Do not mark review-pending previews as accepted heroes.
- Do not add JavaScript algorithm computation.

## Validation

Run:

```bash
node --check visual/src/metric-visual.js
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-native-hero-evidence-scale.mjs
node visual/tools/check-visual-command-api.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-grae10-golden.mjs
```

Report exact commands and results.
