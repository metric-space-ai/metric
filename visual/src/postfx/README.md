# METRIC Visual Post-FX

Dependency-free native WebGL postprocessing for the final photographic treatment of METRIC Visual scenes.

The subsystem is intentionally scoped to finishing effects that reinforce miniature data-world rendering:

- restrained highlight bloom, not neon glow
- miniature frame shaping that couples focus, floor, sky and edge falloff
- photographic color grade
- edge-aware vignette
- optional fine film grain
- camera-depth depth of field driven by focus distance, aperture and circle of
  confusion
- focus-line and optional depth helpers for compatibility paths

The modules are plain JavaScript ES modules and embed their GLSL strings. They do not require runtime packages, build tools, adapters, or transpilation.

## Public Surface

```js
import {
  MetricPostFxStack,
  HighlightBloomPass,
  MiniatureFramePass,
  ColorGradePass,
  VignettePass,
  FilmGrainPass,
  MiniatureCameraDofPass,
  FocusLine,
  OptionalDepthFocus,
} from "./postfx/index.js";
```

`MetricPostFxStack` is the recommended integration point. It behaves like a native postprocess pass:

```js
const postFx = new MetricPostFxStack(gl, {
  cameraDof: {
    enabled: true,
    focusDistance: 8.5,
    depthNear: 1.2,
    depthFar: 18,
    aperture: 0.9,
  },
  bloom: { intensity: 0.18, threshold: 0.78 },
  miniatureFrame: { amount: 0.42, focusClarity: 0.18 },
  grade: { contrast: 1.08, saturation: 0.94 },
  vignette: { amount: 0.18 },
  filmGrain: { enabled: false },
});

nativePipeline.addPass(postFx);
```

The stack is also responsible for size propagation. Its internal passes and
temporary targets are initialized and resized together, so every active pass
sees the same drawing-buffer size as the host renderer. Miniature style
diagnostics verify this contract through `postFx.passSize`.

Individual passes can also be added directly when the host wants a custom order:

```js
nativePipeline.addPass(new HighlightBloomPass(gl));
nativePipeline.addPass(new MiniatureFramePass(gl));
nativePipeline.addPass(new ColorGradePass(gl));
nativePipeline.addPass(new VignettePass(gl));
```

## Default Order

`MetricPostFxStack` renders:

1. camera-depth depth of field, enabled only when configured and supplied with a camera-depth texture
2. highlight bloom
3. miniature frame shaping
4. color grade
5. vignette
6. film grain, disabled by default

Camera-depth depth of field happens before the frame/grade stack so the later
photographic treatment works on the same optical image a camera would see.
Bloom happens before frame shaping and grading so highlights are softened
before the final photographic contrast and color balance. The miniature frame
pass sits before grading and vignette because it describes the stage relation:
focus clarity, floor/sky lift, ground shade and edge falloff. Grain is last so
it is not blurred or graded into a visible overlay.

## Effect Notes

`HighlightBloomPass` extracts only bright regions, downsamples to an internal target, performs repeated separable blur, then composites back into the source color texture. Defaults are intentionally low-intensity.

`MiniatureCameraDofPass` is the physical miniature-optics pass. It expects a
camera-depth texture in `context.cameraDepthTexture`, `context.depthTexture`,
`context.cameraDepthTarget.texture`, or `context.depthTarget.texture`. The red
channel must encode normalized linear camera distance from `depthNear` to
`depthFar`; alpha marks covered pixels. The pass reconstructs camera distance,
compares it with `focusDistance`, applies `aperture`, and derives the blur
radius from a circle-of-confusion calculation. Without a depth texture the pass
is neutral; it must not fall back to a screen-space blur band.

`MiniatureFramePass` is the reusable METRIC miniature finish. It is not a demo
effect. It receives the same focus state as the camera DoF pass or the
compatibility tilt-shift pass and adds
restrained focus clarity, subject isolation, stage floor/sky relation, floor
sheen, stage spotlight, stage shadow, neutral shadow tint, backlight, edge fade and warm/cool frame balance
over arbitrary METRIC scenes.

Important frame options:

- `stageCenter`, `stageScale`, `stageTilt`: screen-space tabletop matte.
- `floorRatio`, `floorColor`, `skyColor`: photographed floor/sky split.
- `floorSheen`, `floorFalloff`, `groundShade`: lower-stage floor response.
- `stageSpotlight`, `stageShadow`: tabletop light pool and edge falloff.
- `subjectIsolation`, `microContrast`: color/luma-aware focus separation.
- `shadowColor`, `highlightColor`, `backlight`: studio light direction.

`ColorGradePass` supports exposure, contrast, saturation, vibrance, temperature, tint, gamma, black and white points, lift, gain, shadow tint, highlight tint, and highlight compression.

`VignettePass` uses a soft elliptical falloff with colorized edge shading. It is designed to guide the eye toward the miniature field without making the frame look synthetic.

`FilmGrainPass` is opt-in. It adds fine luminance-aware noise only; scanlines, halftone, glitch, and ornamental treatments are intentionally absent from the default stack.

`FocusLine` converts pointer positions and focus-line geometry into stable
uniforms for compatibility tilt/focus passes. For the photographic miniature
path, runtime code should instead update `MiniatureCameraDofPass.focusDistance`
from the focused record or scene point after the camera matrix is known.
`OptionalDepthFocus` feature-detects depth texture support but does not require
depth textures for the immediate color-texture path.

`FocusLine` also carries the photographic focus-shaping fields shared with the
native tilt-shift pass:

- `focusBand`: the fully sharp band around the focus line.
- `blurCurve`: the curve exponent for the ramp from the sharp band into the
  blurred region.

Runtime style code should forward these values together with `start`, `end`
and `radius`; otherwise the postprocess stack can no longer prove that all
focus-aware passes are using the same optical contract.

## Runtime Gate

For production use, a miniature visual should satisfy these post-FX checks:

- `MiniatureCameraDofPass`, `MiniatureFramePass`, `ColorGradePass`, and
  `VignettePass` are present when the visual promises the photographic
  miniature look.
- `MiniatureCameraDofPass` receives `focusDistance`, `depthNear`, `depthFar`,
  `aperture`, and a camera-depth texture from the same camera used to render
  the scene.
- compatibility `TiltShiftPass` and `MiniatureFramePass` both receive the
  expected `focusBand` and `blurCurve` from the same focus state when the
  legacy screen-space path is used.
- active stack passes have `width` and `height` equal to the renderer drawing
  buffer size.
- disabled optional passes, such as bloom or grain, still resize correctly so
  enabling them later cannot create a first-frame mismatch.

## Source Notes

Reference implementations were reviewed for effect shape only. The code here is native WebGL, dependency-free, and does not expose external pass names in the public API.
