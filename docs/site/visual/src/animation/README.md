# METRIC Visual Animation

Plain ES-module animation helpers for the native METRIC Visual canvas/WebGL
engine. This module has no dependencies and does not own
`requestAnimationFrame`; callers drive it from `RenderLoop`, tests, or another
clock.

The module only animates already-exported visual state. It does not run METRIC
algorithms, infer missing coordinates, or synthesize metric values. Missing
sources produce empty output or constant motion from the available source.

## Exports

- `Easing` / `EASING`, `resolveEasing`, `applyEasing`, `clamp01`, `lerp`
- `KeyframeTrack` for scalar/vector keyframes sampled into caller-owned output
- `interpolateTypedArrays`, `interpolateChannels`, `ChannelInterpolator`
- `CoordinateMorphState`, `flattenCoordinateState`, `recordIdsFromCoordinate`
- `AnimatedWeightChannel`, `writeIndexSetWeights`,
  `combineFocusSelectionWeights`
- `hashUint32`, `hashString`, `hashValue`, `deterministicPhase`,
  `fillDeterministicPhases`, `valueNoise1D`, `signedValueNoise1D`,
  `phaseOscillation`
- `AnimationScheduler`

## Hot Path Contract

Per-frame methods write into existing typed arrays or stable objects:

```js
import { CoordinateMorphState } from "./index.js";

const morph = new CoordinateMorphState({
  from: positionChannel,
  to: targetPositionChannel,
  output: renderPositions,
  easing: "smoothstep",
});

function render(time) {
  morph.update(time.seconds % 1);
  // upload renderPositions or mark the owning GPU buffer dirty
}
```

`KeyframeTrack.sampleInto`, `interpolateTypedArrays`,
`CoordinateMorphState.update`, `AnimatedWeightChannel.update`, and
`AnimationScheduler.update` avoid new arrays during normal frame updates.
Constructors and source-normalization helpers may allocate once.

## Scheduler

`AnimationScheduler` is a small state machine for animations that are advanced
by an external clock:

```js
import { AnimationScheduler } from "./index.js";

const scheduler = new AnimationScheduler();
scheduler.add((frame) => {
  morph.update(frame.progress);
}, { durationMs: 1200, loop: true });

renderLoop = new RenderLoop((time) => {
  scheduler.tick(time);
  renderer.render();
});
```

The scheduler never calls `requestAnimationFrame`. It accepts the same time
object shape produced by `visual/src/engine/render-loop.js`.

## Deterministic Visual Motion

Phase and noise helpers are deterministic and intended only for visual motion
such as shimmer, breathing focus halos, or staggered selection fades. They are
not random sources for METRIC algorithms or data generation.
