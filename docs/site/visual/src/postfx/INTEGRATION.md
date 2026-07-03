# Post-FX Integration Hooks

This package integrates with the native postprocess pipeline and can consume
the scene target's raw camera-depth texture for photographic miniature depth of
field.

## Hook 1: Import

In the runtime or visual entry module that owns final rendering, import:

```js
import { MetricPostFxStack } from "./postfx/index.js";
```

If importing from another folder, adjust the relative path to `visual/src/postfx/index.js`.

## Hook 2: Construct After WebGL Context Creation

Create the stack after the WebGL context and native postprocess pipeline exist:

```js
const postFx = new MetricPostFxStack(gl, {
  cameraDof: {
    enabled: true,
    focusDistance: 8.5,
    aperture: 0.8,
    maxBlur: 6,
  },
  bloom: {
    enabled: true,
    threshold: 0.78,
    intensity: 0.18,
    radius: 1.15,
  },
  miniatureFrame: {
    enabled: true,
    amount: 0.42,
    focusClarity: 0.18,
    stageGlow: 0.12,
    groundShade: 0.08,
  },
  grade: {
    contrast: 1.08,
    saturation: 0.94,
    highlightCompression: 0.16,
  },
  vignette: {
    amount: 0.18,
    radius: 0.56,
  },
  filmGrain: {
    enabled: false,
  },
});
```

## Hook 3: Add Last

Add the stack after antialiasing or compatibility native passes:

```js
nativePostprocessPipeline.addPass(existingFxaaPass);
nativePostprocessPipeline.addPass(postFx);
```

The post-FX stack should be last because it performs optical focus, frame
shaping and final photographic grade. If `cameraDof.enabled` is true, construct
the native pipeline with `sceneDepthTexture: true` so the scene color target is
paired with a raw depth texture:

```js
const nativePostprocessPipeline = new NativePostProcessPipeline(gl, {
  sceneDepthTexture: true,
});
```

Within the stack, `MiniatureCameraDofPass` runs before bloom/frame/grade. It
reconstructs camera distance from the raw depth texture and derives blur from
focus distance, aperture and circle of confusion. `MiniatureFramePass` then
applies the reusable miniature stage treatment to arbitrary METRIC visuals.

## Hook 4: Resize

No separate resize hook is required when the stack is registered as a native pipeline pass. The existing pipeline calls `setSize(width, height)` on registered passes.

If used manually, call:

```js
postFx.setSize(width, height);
```

## Hook 5: Per-Frame Time

Only film grain needs time, and grain is disabled by default. If grain is enabled, update it once per frame before rendering:

```js
postFx.setTime(performance.now() * 0.001);
```

## Hook 6: Focus Input

For photographic focus through `MetricVisualRuntime`, set a scene-bound focus
target. The runtime resolves that target through the active camera matrix every
frame:

```js
runtime.setCameraFocusPoint(focusedRecordPosition);
runtime.selectRecord(recordId);
runtime.setCameraFocusDistance(8.5);
```

When a visual needs pointer-driven focus, enable `hoverFocus` on the runtime
instead of adding a screen-space overlay. The runtime projects candidate record
positions, resolves the hovered record to a world-space focus target, and emits
`camerafocuschange` after the next frame has applied the camera distance to
`MiniatureCameraDofPass`:

```js
runtime.setHoverFocusOptions({ enabled: true, thresholdPx: 54 });
runtime.on("camerafocuschange", ({ focusDistance }) => {
  // Applied photographic focus distance for the current camera.
});
```

For lower-level post-FX use, update the camera DoF pass from the focused scene
point after the camera matrix is current:

```js
postFx.cameraDof.setFocusDistance(cameraDistanceToFocusedRecord);
```

For compatibility focus-line effects, create a focus line and pass its uniforms
into the focus consumer:

```js
import { FocusLine } from "./postfx/index.js";

const focusLine = new FocusLine({ width, height, radius: 96 });
focusLine.setFromPointer(pointerEvent, canvas);
const uniforms = focusLine.toUniforms();
```

The immediate post-FX stack remains safe without a depth texture:
`MiniatureCameraDofPass` becomes neutral when no camera-depth texture is
available. Do not replace it with a fake screen-space overlay.

```js
nativePipeline.renderScene(drawScene, {
  camera,
  cameraDepthTexture: nativePipeline.sceneTarget.depthTexture,
  cameraDepthEncoding: "raw-depth",
});
```

## Hook 7: Disposal

Dispose the stack when the renderer is torn down:

```js
postFx.dispose();
```

## Expected Placement

The final render path should be:

```text
scene render target
  -> raw camera-depth texture from the same camera
  -> compatibility native postprocess passes
  -> MetricPostFxStack
  -> screen or caller-provided output target
```

Do not place halftone, scanline, glitch, or decorative effects in the default chain. They conflict with the METRIC miniature photographic style.
