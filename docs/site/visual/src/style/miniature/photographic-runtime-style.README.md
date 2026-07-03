# Photographic Runtime Style

`photographic-runtime-style.js` is a direct-import helper layer. It does not
modify the miniature barrel export and does not import source visual references or external renderer.

## Runtime Setup

```js
import {
  createPhotographicMetricRuntimeSetup,
  createPhotographicMetricSceneBundle,
} from "./style/miniature/photographic-runtime-style.js";

const setup = createPhotographicMetricRuntimeSetup({
  canvas,
  layerFactory,
  stageOptions: {
    camera: { position: [0, -20, 10], target: [0, 0, 0], up: [0, 1, 0] },
  },
});

const bundle = createPhotographicMetricSceneBundle(descriptors, {
  stage: setup.stage,
  contacts: { includeSamples: false },
});
```

`setup.runtimeOptions` is ready for `MetricVisualRuntime`. It keeps renderer
resize and pointer input enabled, uses a y-up camera over an x/z ground plane,
enables FXAA and tilt-shift, and enables `postFx.cameraDof`, which makes the
runtime request a camera depth texture.

## Demo5 Source Contract

`DEMO5_TILT_SHIFT_SOURCE_CONTRACT` records the local Demo5 parameters as a
renderer contract:

- white scene background
- camera at `[0, -20, 10]`, looking at the origin
- ambient light plus point light at `[0, 0, 20]`
- Phong-like vertex-color material defaults
- RenderPass, FXAAPass, TiltShiftPass
- focus radius `height / 3`, focus line at `height / 2`
- pointer-y focus-line updates

## Hold-Morph-Hold

```js
import {
  applyHoldMorphHoldProgressToDescriptors,
  createHoldMorphHoldTimeline,
  sampleHoldMorphHoldTimeline,
} from "./style/miniature/photographic-runtime-style.js";

const timeline = createHoldMorphHoldTimeline();
const sample = sampleHoldMorphHoldTimeline(timeline, performance.now());
const nextDescriptors = applyHoldMorphHoldProgressToDescriptors(bundle.layers, sample);
```

For live GRAE10 playback, prefer updating the existing morph layers'
`animation.progress` from the sampled value and requesting a render. That avoids
rebuilding the 60k point-cloud buffers per frame.
