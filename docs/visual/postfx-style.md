# METRIC Visual Post-FX Style

The post-FX layer should make METRIC Visual feel like a photographed miniature data-world: tangible, spatial, and optically coherent. It should not read as a neon dashboard, glitch graphic, or decorative shader showcase.

## Defaults

- Bloom is a highlight recovery pass, not a glow effect. Use high thresholds and low intensity.
- Color grade should preserve data hue legibility while adding slight photographic warmth and compressed highlights.
- Vignette should guide attention toward the field center without visibly darkening the interface.
- Film grain is optional and disabled by default. When enabled, keep it fine and luminance-aware.
- Halftone, scanlines, chromatic split, glitch, posterization, and ornamental overlays are not part of the default style.

## Recommended Baseline

```js
new MetricPostFxStack(gl, {
  cameraDof: {
    enabled: true,
    focusDistance: 8.5,
    depthNear: 1.2,
    depthFar: 18,
    aperture: 0.9,
    maxBlur: 6.2,
  },
  bloom: {
    threshold: 0.78,
    knee: 0.16,
    intensity: 0.18,
    radius: 1.15,
    iterations: 3,
    downsample: 2,
    saturation: 0.72,
    warmth: 0.1,
  },
  grade: {
    contrast: 1.08,
    saturation: 0.94,
    vibrance: 0.08,
    temperature: 0.08,
    highlightCompression: 0.16,
  },
  vignette: {
    amount: 0.18,
    radius: 0.56,
    softness: 0.58,
  },
  filmGrain: {
    enabled: false,
  },
});
```

## Tuning Rules

Raise bloom threshold before raising bloom intensity. If bloom is visible on midtones, it is too low.

Use saturation below `1.0` for dense categorical data. Use small positive vibrance to keep muted structures alive without pushing already saturated channels.

Prefer highlight compression over exposure reduction when bright structures clip. Exposure changes should remain scene-level decisions.

Keep vignette color slightly cool-neutral. Pure black edges make the scene feel composited rather than photographed.

Use camera-depth depth of field as the photographic miniature path. The pass
needs a color target and a camera-depth texture rendered from the same camera.
The depth texture stores normalized linear camera distance in the red channel
and coverage in alpha; the pass reconstructs camera distance and derives blur
from `focusDistance`, `aperture`, `depthNear`, `depthFar`, and the resulting
circle of confusion. If no depth texture is supplied, the pass must be neutral,
not a fake screen overlay.

Use focus-line effects only as a compatibility path for renderers that cannot
provide camera depth. Keep the immediate post-FX stack usable from color
texture alone, but do not call that path the physical miniature effect.

Use a real in-focus band before the blur ramp. Miniature visuals should pass
`focusBand`/`focusBandRatio` and `blurCurve` through the same focus state as
`start`, `end`, and `radius`; otherwise the tilt-shift pass becomes a hard
screen blur instead of a photographic focus plane.

## QA Checklist

- White and pale labels remain readable.
- Categorical colors are still distinguishable after grading.
- Bloom appears only on genuinely bright highlights.
- Bloom diagnostics expose `intensity`; `strength` is treated only as a
  reference/import alias.
- Grain is invisible in still screenshots unless intentionally enabled.
- The final frame looks photographic, not synthetic or ornamental.
- Active post-FX passes are sized to the renderer drawing buffer on the first
  frame and after every resize.
- The camera DoF pass consumes a camera-depth texture from the same camera that
  rendered the scene.
- The camera DoF diagnostics report non-null `focusDistance`, `depthNear`,
  `depthFar`, and `aperture` for examples that claim the physical miniature
  look.
- Compatibility tilt-shift and miniature-frame diagnostics report non-null
  `focusBand` and the expected `blurCurve` only for renderers that use the
  legacy screen-space path.
- `inspectMiniatureStyleContract().checks.photographicStyle` is green for
  example pages that claim to use the miniature look.
