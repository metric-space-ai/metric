# Dynamics Timeline Controls Workstream

Status date: 2026-06-25

## Implemented

- `visual/src/timeline/index.js` now promotes timeline-step properties into
  `metric.visual.timeline_property_sample.v1` samples. The samples are matched
  by exported timeline id, step id or step index and are marked as exported
  evidence with `algorithmicComputation: false`.
- `createTimelineControlDescriptor()` now emits user-facing timeline control
  metadata on top of the existing `metric.visual.timeline_control.v1` contract:
  a scrubber presentation, range binding, playback/reset controls, readout
  text and start/middle/end marks backed by sampled exported states.
- `DynamicsView` now consumes the sampled timeline state to select exported
  coordinate states for start/middle/end positions and emits a reusable
  `HeatFieldLayer` descriptor for the active ground/field state. The field
  descriptor carries selected exported record-property values plus a
  `timelineScalar` channel from exported timeline-step properties.
- The new field and control metadata do not recompute diffusion, Redif
  inverse-dynamics or reconstruction values in JavaScript. They only select and expose
  exported coordinates/properties.

## Evidence

- The native dynamics export has 41 reverse reconstruction coordinate states.
- The start/middle/end marks resolve to:
  `coord-reverse-00`, `coord-reverse-20`, `coord-reverse-40`.
- The timeline field state can be bound to exported
  `reverse-mse-to-clean` samples. The headless gate verifies the sampled values
  `0.0175987`, `0.0190614` and `0.0375194` for start/middle/end.
- `DynamicsView` keeps the user controls in descriptors. No page-local timeline
  controls were added.

## Verification

- `node visual/tools/check-dynamics-timeline-control.mjs`
- `node visual/tools/check-timeline-motion-contract.mjs`
- `node visual/tools/check-dynamics-motion-grammar.mjs`
- `node visual/tools/check-visual-command-api.mjs`
- `node visual/tools/check-views.mjs`
- `node visual/tools/check-single-render-pipeline.mjs`

## Remaining Gaps

- Runtime UI widgets are still descriptor consumers to build later; this slice
  only defines the reusable control contract and view descriptors.
- Dynamics/noise remains public preview, not hero-accepted. Screenshot review
  and trajectory/field readability acceptance are still required.
