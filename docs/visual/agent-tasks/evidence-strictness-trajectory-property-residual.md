# Agent Task: Evidence Strictness For Trajectory, Property And Residual Views

## Owner

One worker owns only:

```text
visual/src/views/TrajectoryPathView.js
visual/src/views/ProcessCurveSceneView.js
visual/src/views/PropertyFieldView.js
visual/src/views/MappingView.js
visual/src/metric-visual.js
visual/tools/check-*.mjs
docs/visual/reports/evidence-strictness-trajectory-property-residual.md
```

Coordinate with the native-provenance worker if both need
`visual/src/metric-visual.js` or the same check file. Do not revert other
workers. Do not edit examples, native exporters, project page, GRAE10 or
runtime/layer internals.

## Goal

Prevent renderer convenience fallbacks from being mistaken for METRIC evidence.

JavaScript may index, render, pick, interpolate and animate exported evidence.
It must not silently invent trajectory evidence, property roles or residual
claims.

## Required Fixes

1. Trajectory evidence
   - Public/hero grammars must not treat raw record order as trajectory
     evidence.
   - Record order may remain an internal layout fallback only if diagnostics
     clearly mark it as non-evidence.
   - `TrajectoryPathView` and `ProcessCurveSceneView` should prefer exported
     paths, graphs, transitions or timeline coordinate states.
   - If required trajectory evidence is absent for a public command, expose a
     hard diagnostic or fail the matching gate.

2. Property-role evidence
   - Hero-specific commands must not silently choose the first scalar property
     when a named role is required.
   - Density, entropy, anomaly, uncertainty or residual fields must identify
     the exported property ID used.
   - Diagnostics must state whether a field used an explicit property or a
     fallback.

3. Mapping residual evidence
   - `MappingView` must not render or classify residual/error vectors as
     preservation evidence unless an explicit residual/preservation-error
     property exists and finite values are present.
   - If source/target coordinates exist but residual evidence is missing, a
     coordinate-delta visual may remain only if it is not labelled residual or
     preservation error.

4. Checks
   - Add or harden checks so public examples cannot pass with record-order
     trajectory evidence, scalar-property defaulting, or residual layers with
     `residualPropertyId: null`.

5. Report

Write:

```text
docs/visual/reports/evidence-strictness-trajectory-property-residual.md
```

## Stop Rules

- Do not change accepted GRAE10 output or baseline.
- Do not add JavaScript algorithm computation.
- Do not delete a visual grammar to make checks pass; preserve the grammar and
  expose missing native evidence clearly.
- Do not mark review-pending previews accepted.

## Validation

Run:

```bash
node --check visual/src/views/TrajectoryPathView.js
node --check visual/src/views/ProcessCurveSceneView.js
node --check visual/src/views/PropertyFieldView.js
node --check visual/src/views/MappingView.js
node visual/tools/check-views.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-visual-command-api.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
```

Report exact commands and results.
