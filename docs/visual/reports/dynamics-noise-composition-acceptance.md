# Dynamics Noise Composition Acceptance

Date: 2026-06-26

## Changed Files

- `visual/src/views/TrajectoryPathView.js`
- `visual/src/views/PropertyFieldView.js`
- `visual/src/views/DynamicsView.js`
- `visual/tools/check-trajectory-path-view.mjs`
- `visual/tools/check-dynamics-motion-grammar.mjs`
- `visual/tools/check-dynamics-timeline-control.mjs`
- `docs/visual/reports/dynamics-noise-composition-acceptance.md`

No dynamics page HTML, synthetic evidence, native exporter, GRAE10 file,
public hero manifest, or project-site file was changed.

## Scope Result

The dynamics/noise preview still uses only:

```text
docs/examples/assets/dynamics-noise/metric.visual.json
```

The reusable dynamics descriptors now expose
`metric.visual.layer_hierarchy.v1` metadata for:

- propagation/uncertainty field: `support-field`, below history, path, current
  state and labels
- state-history context: `state-history-context`, above fields and below the
  primary trajectory, current state and labels
- trajectory path: `trajectory-path`, above fields/projections and below
  current state/labels
- current timeline state: `current-state`, above field, history context and
  trajectory

`TrajectoryPathView` owns the shared hierarchy schema and trajectory band.
`PropertyFieldView` owns the reusable scalar-field band. `DynamicsView` applies
the same contract to its timeline field, state-history context and current
state descriptors. All hierarchy descriptors report
`algorithmicComputation: false`.

## Descriptor Evidence

The finite-dynamics grammar still renders:

- `CurveRibbonLayer` for exported timeline-state paths
- `HeatFieldLayer` for exported record/timeline scalar field evidence
- `InstancedPointLayer` for current state and history context

The trajectory uses the exported reverse-reconstruction timeline with 512
records and 41 coordinate states. Timeline controls remain metadata-driven via
`metric.visual.timeline_control.v1`.

## Validation

- `node --check visual/src/views/TrajectoryPathView.js`: passed.
- `node --check visual/src/views/PropertyFieldView.js`: passed.
- `node --check visual/src/views/DynamicsView.js`: passed.
- `node --check visual/tools/check-dynamics-motion-grammar.mjs`: passed.
- `node --check visual/tools/check-dynamics-timeline-control.mjs`: passed.
- `node visual/tools/check-trajectory-path-view.mjs`: passed, `total: 19`,
  `failed: 0`.
- `node visual/tools/check-dynamics-motion-grammar.mjs`: passed, `total: 26`,
  `failed: 0`.
- `node visual/tools/check-dynamics-timeline-control.mjs`: passed,
  `total: 16`, `failed: 0`.
- `node visual/tools/check-hero-visual-briefs.mjs`: passed,
  `dynamics-noise-hero` roles include `trajectory/path`, `property-field`,
  `timeline-state-history-context` and `current-timeline-state`.
- `node visual/tools/check-public-gallery-evidence.mjs`: passed,
  `ok: true`, public dynamics evidence points to
  `../../../docs/examples/assets/dynamics-noise/metric.visual.json`.

## Remaining Status

`dynamics-noise-hero` remains review-pending. The blocker is still:

```text
visual-composition-not-human-accepted
```
