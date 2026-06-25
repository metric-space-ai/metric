# Timeline Control Widget Workstream

Status date: 2026-06-25

## Implemented

- Added `TimelineControlWidget`, a reusable DOM interaction widget that consumes
  `metric.visual.timeline_control.v1` descriptors and discovers those controls
  from DynamicsView-emitted layer descriptor metadata.
- The widget exposes scrubber, play/pause and reset state from descriptor
  controls, marks and exported timeline samples. Scrubbing snaps to the
  descriptor range when the control declares nearest-exported-step selection.
- `MetricVisualSurface.showDynamics()` now attaches the widget when a browser
  DOM is available. Widget updates feed back through `showDynamics()` with
  normalized timeline progress, so DynamicsView resamples exported timeline
  state and property samples.
- Non-dynamics semantic views dispose the timeline widget to avoid stale
  controls. Headless checks remain guarded because attachment requires DOM.

## Evidence

- The widget reads descriptor metadata only: timeline id, range, controls, marks,
  exported samples and state bindings.
- Scrubbing to the middle mark selects `coord-reverse-20` and exported
  `reverse-mse-to-clean` sample `0.0190614`.
- The widget source does not contain diffusion, Redif or inverse-dynamics algorithm paths.

## Verification

- `node visual/tools/check-dynamics-timeline-control.mjs`
- `node visual/tools/check-timeline-control-widget.mjs`

## Remaining Gaps

- Playback currently advances the descriptor-normalized value and rebuilds
  dynamics on exported-step changes. A future runtime-level timeline mutator
  could avoid full descriptor rebuilds when the renderer gains direct timeline
  state mutation.
- The widget uses compact built-in DOM controls. Screenshot acceptance is still
  needed for public gallery placement and mobile visual polish.
