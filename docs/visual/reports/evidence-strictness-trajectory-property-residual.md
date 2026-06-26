# Evidence Strictness For Trajectory, Property And Residual Views

Date: 2026-06-26
Branch: `codex/visual-engine-evidence-exporters`

Status: partial engine hardening integrated in the main thread. This report
covers the first strictness fix: trajectory/path descriptors must not present
record-order layout as metric evidence.

## Changed Files

- `visual/src/views/TrajectoryPathView.js`
- `visual/src/views/ProcessCurveSceneView.js`
- `docs/visual/reports/evidence-strictness-trajectory-property-residual.md`

## Change

`TrajectoryPathView` now marks record-order paths as a layout fallback:

```text
isEvidence: false
evidenceClaim: false
fallback: true
fallbackReason: record-order-layout-is-not-trajectory-evidence
```

The same flags are copied into descriptor metadata and native-evidence
references. This keeps record-order visuals available for internal layout
contexts without letting them masquerade as exported trajectory evidence.

`ProcessCurveSceneView` now prefers exported graph/relation trajectory evidence
whenever available. Record-order tracks are used only when no graph/relation
track can be resolved, or when graph trajectory use is explicitly disabled.

## Remaining Strictness Work

The follow-up items remain open:

- harden public browser gates so public examples fail if their trajectory layer
  uses `evidenceClaim: false`
- enforce explicit property roles for hero-specific fields instead of allowing
  scalar-property defaulting in public hero commands
- keep mapping residual/error layers disabled unless an explicit residual or
  preservation-error property exists with finite values

These are assigned in:

```text
docs/visual/agent-tasks/evidence-strictness-trajectory-property-residual.md
```

## Validation

```bash
node --check visual/src/views/TrajectoryPathView.js
node --check visual/src/views/ProcessCurveSceneView.js
```

Result: passed.

```bash
node visual/tools/check-views.mjs
```

Result: passed, 80 total checks and 0 failures.

```bash
node visual/tools/check-hero-grammar-contract.mjs
```

Result: passed.

```bash
METRIC_VISUAL_EXAMPLES=condition-monitoring-hero,process-curve-external-hero node visual/tools/check-visual-regression-public-examples.mjs
```

Result: passed with `ok: true`, 2 total pages and 0 failures.
The condition-monitoring curve-picking probe used
`track:condition-monitoring:graph-trajectory`, confirming that the public
condition preview resolves exported graph trajectory evidence instead of the
record-order layout fallback.
