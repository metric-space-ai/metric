# Native METRIC Miniature Style

`visual/src/style/miniature/` contains dependency-free vanilla JavaScript
presets for applying a photographic miniature miniature and photographic visual language to
arbitrary METRIC layer descriptors.

It is not a CSS theme and not a wrapper around source visual references, Vue, external renderer, or demo
subjects. It emits plain METRIC visualization options:

- camera composition presets
- scene and palette presets
- generic material family presets for samples, meshes, surfaces, fields, lines,
  and ground planes
- light-rig descriptors
- packed multi-light material uniforms for native mesh, surface and curve
  layers
- photographic material-response controls for native point/glyph, mesh and
  surface layers: sphere shading, gloss, edge shade, saturation, roughness,
  metalness, specular power, soft shadows, rim light, base lift and
  highlight/shadow tint
- serializable viewport-ratio focus-plane descriptors
- grounding descriptors for stage floor, grid, semantic colored shadows and
  projection footprints
- automatic contact-shadow descriptors for grounded solid, glyph, and curve-path
  data layers
- shadow-catcher floor controls for palette-aware matte color, subdued grid,
  horizon falloff, tabletop sheen, contact shade, directed shadow tails and
  semantic tint separation
- camera-depth depth-of-field, compatibility tilt-shift, Bokeh, bloom, film,
  halftone, SSAO, zoom-blur, FXAA, and SMAA postprocess options
- a runtime-ready `postFx` profile containing photographic color grade,
  miniature frame shaping, stage spotlight/shadow, vignette, restrained
  highlight bloom and optional film grain
- native reference-look presets extracted from inspected source visual references examples:
  `white-tabletop`, `chromatic-surface`, `dark-tube-field`,
  `dark-gloss-studio`, `metric-city`, `colored-shadow-stage`,
  `studio-board`, and `surface-ripple`
- descriptor transforms that preserve metric channel data

## Main API

```js
import {
  MiniaturePhotographicStyle,
  applyMiniatureSceneBundle,
  createMiniaturePhotographicStyle,
  createMiniatureAnimationPreset,
  createMiniatureContactShadowDescriptors,
  createMiniatureFocusPlane,
  createMiniatureGroundingPreset,
  createMiniatureHeroProfile,
  createMiniatureHeroRuntimeOptions,
  createMiniatureHeroSceneBundle,
  createMiniatureHeroStage,
  createMiniatureHeroStyle,
  createMiniatureLookAtlas,
  createMiniatureLookAtlasEntry,
  createMiniatureLookSceneAtlas,
  createMiniatureLookSceneAtlasEntry,
  createMiniatureReferenceLookOptions,
  createMiniatureRigReport,
  createMiniatureRuntimeOptions,
  createMiniatureStyleMotionAtlas,
  createMiniatureStyleMotionAtlasEntry,
  createMiniatureStyleProfile,
  createMiniatureStyleMotionPreset,
  createMiniatureStandaloneSurface,
  inspectMiniatureStyleContract,
  inspectMiniatureStyleProfile,
  inspectMiniatureStandaloneSurface,
  publishMiniatureRigDataset,
  applyMiniatureStyleToDescriptors,
  attachMiniatureRigReporter,
  createMiniatureSceneBundle,
  createMiniatureStagePreset,
  createMetricMiniaturePalette,
  listMiniatureReferenceLooks,
  listMiniatureStyleMotionPresets,
  sampleMiniatureStyleMotion,
  selectMiniatureStyleMotionForLook,
  summarizeMiniatureStyleMotion,
} from "./index.js";
```

`createMiniaturePhotographicStyle(options)` is the preferred owner for a
reusable miniature surface. It keeps one stage contract and exposes methods for
creating runtime options, styling descriptors, creating scene bundles, applying
the stage, updating focus/postprocess, and applying bundles to a runtime. Use
it when a page or hero application should keep camera, light, focus,
postprocess, ground and motion in one place.

`MiniaturePhotographicStyle` is stateful only at the style/runtime boundary. It
does not compute metric values and does not own algorithm evidence.

`createMiniatureStandaloneSurface(options)` is the shared contract for generated
or hand-authored standalone WebGL documents such as exported algorithm figures.
It keeps the same camera, focus, floor, postprocess and quiet-inspection
vocabulary without requiring `MetricVisualRuntime`. Public hero surfaces should
render without visible debug bars by default. Controls, labels and record
probes are inspection affordances, and labels should be hidden unless the
inspection mode is explicitly enabled.

`createMiniatureStyleProfile(options)` creates the serializable profile for a
miniature surface. It is the transportable style contract for arbitrary METRIC
visuals: stage, camera, light rig, focus, ground relation, post-FX, fitting
defaults and style motion. It carries no runtime, canvas, WebGL handle,
function, metric records or algorithm output.

`createMiniatureStagePreset({ look })` can start from a named native reference
look. These looks are reusable style recipes, not demo subjects:

- `white-tabletop`: bright miniature stage, focus plane, floor contact and
  restrained highlight bloom.
- `chromatic-surface`: high-oblique surface/material look with moving
  chromatic studio light roles.
- `dark-tube-field`: dark glossy tube/path look for luminous trajectories and
  dense path fields.
- `dark-gloss-studio`: dark material-response look for glossy objects and
  surface-heavy metric visuals.
- `metric-city`: low-angle dense field look for many small grounded records,
  stronger contact shadows and shallow focus.
- `colored-shadow-stage`: colored-light/shadow look for categorical state,
  anomaly and class separation on a shared floor.
- `studio-board`: dark area-light board look for relation panels, density
  fields and projection-heavy scenes.
- `surface-ripple`: glossy surface-field look for entropy, density,
  transformation and path evidence over a semantic floor.

`listMiniatureReferenceLooks()` lists those plain option bundles.
`createMiniatureReferenceLookOptions(name, overrides)` returns one bundle with
overrides deeply merged. The returned data can be passed into stage/profile
construction without importing source visual references, external renderer, Vue, npm packages or demo code.

`createMiniatureLookAtlas()` turns the named looks into serializable profile
entries with a contract summary and miniature-style diagnostics. Use it for
gallery generation, documentation capture, regression checks, or renderer
tooling that needs to know which camera, light, material, ground, focus,
post-FX and style-motion contract a look provides. It carries no records,
metric values, runtime handles, canvas, WebGL resources, DOM nodes or
algorithm output.

`createMiniatureLookSceneAtlas(descriptors, options)` applies every named look
to the same descriptor set and returns ready scene bundles plus diagnostics.
This is the native regression surface for the photographic layer: a caller can
verify that arbitrary METRIC visual descriptors survive different camera,
light, focus, ground, material, post-FX and style-motion contracts without
copying example code.

`style.toProfile({ includeDiagnostics: true })` exports the current style state
and embeds the contract diagnostic. Use this in examples and generated visual
documents when a hero application needs to prove that it uses the shared
miniature look instead of hand-tuned scene code.

`createMiniatureStyleMotionPreset(kind, options)` creates style-level motion
for the photographic layer itself. It can animate focus breathing, restrained
camera drift/turntable movement and miniature-frame response over the
renderer's time state. This is distinct from descriptor animation: it does not
move records or interpolate metric evidence.

`createMiniatureStyleMotionAtlas()` creates a serializable catalog of reusable
motion contracts. Each entry records the active domains, duration, amplitudes
and sampled focus/camera/postprocess offsets. Use it when examples,
documentation captures or future visual documents need to prove that motion is
part of the shared miniature style layer rather than hand-coded page behavior.

`selectMiniatureStyleMotionForLook(look)` returns the default style motion for
a named photographic look. `createMiniatureLookAtlas()` and
`createMiniatureLookSceneAtlas()` use the same selector, so look profiles,
runtime examples and visual regression pages share one motion vocabulary.

The currently exported motion presets are `focus-breath`, `studio-drift`,
`turntable`, `isometric-orbit`, `ground-scan` and `studio-pulse`.

`inspectMiniatureStyleContract({ style, stage, bundle, descriptors, runtime })`
returns a diagnostic object for the reusable photographic contract. It checks
camera, lighting, focus, depth-of-field, material families, ground relation,
post-FX, tabletop/isometric staging, animation, runtime hooks and the combined
`photographicStyle` signal set. Use it in examples and gallery pages to prove
that arbitrary METRIC visuals are using the same style layer instead of
hand-tuned presentation code.

`checks.photographicStyle` is not a subjective score. It reports whether the
runtime has the concrete miniature-camera signals active together: perspective
tabletop camera, staged light rig, focus optics, ground/projection, miniature
materials, frame/grade/vignette post-FX, correct pass sizing and style or
descriptor animation.

`createMiniatureSceneBundle(descriptors, options)` is the preferred public
entrypoint for arbitrary METRIC visuals. It stages exported coordinates for the
miniature camera, applies the stage/material/light/postprocess style, creates
an optional ground plane, and returns runtime-ready `{ stage, layers, fit }`.
For coordinate morphs, the bundle keeps 2D and 3D states inside the same
miniature room. Flat `position` or `targetPosition` endpoints are lifted to
`fit.sameRoomMorph.flatY`, derived from the stage `groundY`, while true 3D
endpoints keep their exported y-variation. If a 2D endpoint arrives as
`[x, y, 0]`, the miniaturizer reprojects it to `[x, flatY, y]` so the morph
moves records within the tabletop scene instead of switching to a separate
screen-space plot.

`createMiniaturePhotographicRigContract(input)` is the machine-readable quality
contract for that same style layer. It checks the concrete rig used by a visual:
perspective miniature camera, scene light rig, focus target/depth-of-field,
miniature frame and grade, ground relation, material families and style motion.
Use it in examples and tests to prove that a visual is using the reusable
photographic rig instead of a local effect chain.

`createMiniatureRigReport(input)` wraps that contract with current
`MetricVisualRuntime` state: active camera DoF, raw scene-depth texture, applied
focus distance, focus target, hover-focus target and layer state. The report
also publishes the optical model fields that distinguish the real miniature
path from fake blur: `metricDofModel`, `metricDofFocusModel`,
`metricDofCocModel`, `metricDofDepthMode`, aperture, max blur, focal range and
the depth-texture/near-far reconstruction flags.
It also publishes the rest of the photographic rig as compact browser-test
fields: camera projection/coordinate system/FOV/obliqueness, light counts,
ground and projection layer presence, material family coverage, styled/lit
descriptor counts, same-room morph flat-Y state, and style/descriptor motion
runtime hooks. Hero pages should expose these fields through
`document.documentElement.dataset`, not through visible debug bars.
`publishMiniatureRigDataset(target, input)` writes the compact report into
`target.dataset` using stable `metric*` keys for browser tests and generated
captures. `attachMiniatureRigReporter({ runtime, target, resolve })` subscribes
to runtime focus, hover, postprocess, layer and resize events and republishes
the report without adding visible debug controls.

Run `node visual/tools/verify-miniature-rig.mjs` from the repository root to
verify the reusable Process-Curve hero contract without a browser. The check
builds the real process-curve miniature scene, validates
`createMiniaturePhotographicRigContract()`, checks
`inspectMiniatureStyleContract().checks.morphRoom`, and proves that the 2D
morph endpoint is staged on `fit.sameRoomMorph.flatY` while the 3D endpoint
keeps its y-structure. It also checks that the exported camera DoF shader
contains raw-depth camera-distance reconstruction, focus distance, aperture and
circle-of-confusion logic, and that the reusable rig report exposes camera,
light, ground, material, motion and morph-room evidence.

`createMiniatureRuntimeOptions(options)` is the preferred bridge into
`MetricVisualRuntime`. It derives runtime options from one miniature stage:
canvas, camera, clear color, focus/postprocess, controls and layer-factory
wiring. Use it so arbitrary METRIC visuals share the same photographic camera,
depth-of-field and interaction defaults instead of duplicating setup code. It
passes the stage `focus` contract as the Runtime `focusLine`, so the
compatibility tilt-shift, pointer focus and resize behavior begin from the same
viewport-ratio focus plane. Photographic hero surfaces should additionally feed
the post-FX stack with a camera-depth texture and update `postFx.cameraDof`
through `focusDistance`, `depthNear`, `depthFar` and `aperture`.
For record-level inspection, pass `hoverFocus` through
`createMiniatureRuntimeOptions()`. Hover focus is still part of the same
photographic system: it resolves the hovered record to a world-space focus
target and lets the Runtime update the camera depth-of-field pass after render.

`createMiniatureHeroStage(options)`,
`createMiniatureHeroProfile(options)`, `createMiniatureHeroStyle(options)`,
`createMiniatureHeroSceneBundle(descriptors, options)`, and
`createMiniatureHeroRuntimeOptions(options)` are the full-frame capture
helpers. They use the same style/profile/runtime contracts as the generic API,
but start from a stricter photographic preset: lower oblique camera, stronger
tabletop grounding, semantic colored contact shadows, restrained studio drift
and a focus/post-FX stack tuned for large screenshots and project-page visuals.
They remain descriptor-based and do not compute metric data.

`applyMiniatureSceneBundle(runtime, bundle)` applies `stage` through runtime
hooks and replaces layer descriptors with `bundle.layers`.

Typical runtime use:

```js
const style = createMiniaturePhotographicStyle({
  profile: createMiniatureStyleProfile({
    stage: createMiniatureStagePreset({ variant: "white" }),
    styleMotion: "focus-breath",
  }),
  layerFactory: createLayerFromDescriptor,
});
const { runtimeOptions } = style.createRuntimeOptions({ canvas });
const runtime = new MetricVisualRuntime(runtimeOptions);
style.attachRuntime(runtime);
style.applyToRuntime(runtime, { descriptors });
const profile = style.toProfile({ includeDiagnostics: true });
```

Typical full-frame capture use:

```js
const setup = createMiniatureHeroRuntimeOptions({
  canvas,
  layerFactory: createLayerFromDescriptor,
});
const runtime = new MetricVisualRuntime(setup.runtimeOptions);
setup.style.attachRuntime(runtime);
const bundle = createMiniatureHeroSceneBundle(descriptors, {
  stage: setup.stage,
});
setup.style.applyBundle(runtime, bundle);
```

`applyMiniatureStyleToDescriptors(descriptors, options)` returns layer
descriptors with miniature material, geometry, animation, and metadata hints.
It keeps descriptor `channels` unchanged.

`createMiniatureDescriptorBundle(descriptors, options)` is the lower-level
descriptor-only API. It returns:

```js
{
  schema: "metric.visual.miniature_descriptor_bundle.v1",
  style: "metric.visual.style.miniature.photographic",
  stage,
  layers
}
```

`fitMiniatureSceneDescriptors(descriptors, options)` is available when callers
need only camera-stage fitting without applying materials or ground.

`createMiniatureAnimationPreset(kind, options)` creates renderer-facing
animation descriptors such as `still`, `focus`, `coordinate-morph`,
`attention`, `uncertainty`, `surface`, `timeline`, and `camera-orbit`.

Style-level motion is separate and lives in `MiniaturePhotographicStyle`:

- `focus-breath`: small focus-plane and miniature-frame breathing.
- `studio-drift`: subtle focus, frame and camera drift for hero captures.
- `turntable`: camera orbit over a fixed stage.

Style motion is applied from the runtime `beforeRender` hook and respects
camera interaction by default. It should remain slow and photographic; fast
motion belongs to timeline evidence or explicit user interaction.

The motion controller exposes its runtime state through `toJSON()`: active
domains (`focus`, `camera`, `postprocess`), runtime attachment, captured base
state, before-render hook presence and the last applied frame update.
`inspectMiniatureStyleContract()` uses that state as a gate, so a miniature
surface can prove that style motion is actually running instead of merely
declared in a profile.

`createMiniatureFocusPlane(options)` creates the stage-level focus contract.
It stores `xRatio`, `yRatio`, and `radiusRatio` as the source of truth, then
derives compatibility tilt-shift pixel uniforms for the current canvas. `MetricVisualRuntime`
consumes this through `applyStagePreset()`, so pointer focus, resize handling
and the native compatibility tilt-shift pass stay synchronized.

The focus contract also carries `focusBandRatio` and `blurCurve`. The ratio is
resolved to a pixel `focusBand` at runtime, giving the focus plane a real sharp
core before the blur ramp begins. `blurCurve` shapes that ramp. Both fields are
forwarded to the compatibility `TiltShiftPass` and `MiniatureFramePass`, then reported by
`inspectMiniatureStyleContract()` under `focus`, `depthOfField`,
`postprocess`, and `photographicStyle`.

`createMiniatureStagePreset(options).postprocess` is directly consumable by
`MetricVisualRuntime.setPostprocessOptions()`. It includes compatibility
tilt-shift, FXAA and `postFx` by default; `postFx` carries camera DoF, the
reusable miniature frame pass, photographic grade and vignette as named passes.
Pass `postFx: false` only when
the final photographic stack should be explicitly disabled.

`createMiniatureGroundingPreset(options)` creates the stage-level floor and
projection contract. GroundPlane and GroundProjection descriptors consume it so
the photographic floor, grid, semantic colored shadows and isometric
projection footprints stay aligned across arbitrary METRIC visuals.
The floor is a visual relation, not a decorative background: it carries contact
cores, directed shadow tails, semantic projection color and low-dimensional
ground evidence without changing exported metric data.

`flattenMiniatureLightRig(rig)` keeps the stage light roles shader-ready. It
preserves ambient light plus up to four key/fill/colored point lights as packed
arrays, while still exposing the older single-light fields for compatibility.

`createMiniatureContactShadowDescriptors(descriptors, options)` creates
GroundProjection companion descriptors for grounded solid, glyph, and
curve-path data layers. Solid and glyph contacts use existing position, size,
height and color channels. Curve-path contacts use existing normalized path
evidence and project only onto the stage ground. No contact descriptor derives
metric values. `createMiniatureSceneBundle()` uses it automatically unless
`contacts: false` is passed or `stage.grounding.contact.enabled` is false.

## Source Audit Summary

The extracted visual grammar comes from local source visual references site/runtime sources:

- `/tmp/metric-trois-site/assets/Demo1.ea0cb3fb.js`: white stage, high/top camera, physical material, colored moving point lights, noisy displacement surface, refraction/gloss object.
- `/tmp/metric-trois-site/assets/Demo2.836d891c.js`: black stage, straight-on long-lens camera, noisy image texture.
- `/tmp/metric-trois-site/assets/Demo3.e1be2da1.js`: multi-light colored studio scene, transparent metallic Standard material, Phong text, bloom and halftone postprocess, pointer-coupled motion.
- `/tmp/metric-trois-site/assets/Demo4.ef5a6b30.js`: high oblique camera, auto-rotating orbit, spot lights, shadows, Phong material, bloom.
- `/tmp/metric-trois-site/assets/Demo5.ee55b5a1.js`: white stage, oblique camera, ambient plus point light, Phong-like vertex color, FXAA, two-pass tilt-shift, pointer-driven horizontal focus line.
- `/tmp/metric-trois-site/assets/Demo6.7620ed17.js`: black stage, orbit damping, glossy Standard material with high metalness, colored corner point lights, simplex-like pointer-coupled motion.
- `/tmp/metric-trois-site/assets/Demo7.e98226ba.js`: ambient plus four colored point lights, glossy liquid surface, pointer-driven surface disturbance.
- `/tmp/metric-trois-site/assets/ShadowsExample.b12b538f.js` and `/tmp/metric-trois-site/assets/LightExample.80edbe9a.js`: ground contact, shadows and light roles as scene composition.
- `/tmp/metric-trois-site/assets/Textures.53c6bcc6.js`: base color, displacement, normal, roughness and ambient occlusion as one material response contract.
- `/tmp/metric-trois-site/assets/SubSurface.1f1d6931.js` and `/tmp/metric-trois-site/assets/Matcap.6ac6e810.js`: material response extension points.
- `/tmp/metric-ref-trois-site/src/components/demos/Demo1.vue` through
  `Demo7.vue`: source-level camera, light, material, pointer, focus and
  postprocess contracts behind the public demos.
- `/tmp/metric-ref-trois/src/components/sliders/AnimatedPlane.js`: progress
  as a shader uniform for geometry offsets, rotations and UV tiling.
- `/tmp/metric-ref-trois/src/components/liquid/LiquidPlane.js`: pointer-driven
  surface disturbance as a material/geometry response, not a DOM animation.
- `output/playwright/trois-reference/demo1.png`,
  `output/playwright/trois-reference/demo3.png`,
  `output/playwright/trois-reference/demo4.png`,
  `output/playwright/trois-reference/demo5.png`,
  `output/playwright/trois-reference/demo6.png`,
  `output/playwright/trois-reference/demo7.png`,
  `output/playwright/trois-reference/lights.png`,
  `output/playwright/trois-reference/shadows.png`, and
  `output/playwright/trois-reference/subsurface.png`: live browser captures
  used to compare actual composition, material response and focus staging.

`visual/src/views/ProcessCurveSceneView.js` is the current METRIC translation
of those findings for process-curve evidence. It creates reusable layer
descriptors for morph points, semantic ground projection, state density field,
record-order curve paths and grounded record-volume skyline, then passes them
through `createMiniatureSceneBundle()`. Hero captures can request
`trackMode: "tube"` so the record-order path renders through
`CurveTubeMeshLayer` as a lit world-space miniature body instead of a flat
screen-space ribbon.

`visual/examples/miniature-hero-frame/index.html` is the current full-frame
capture target. It renders the same process-curve evidence through
`createMiniatureHeroRuntimeOptions()` and `createMiniatureHeroSceneBundle()` so
the project-page style can evolve independently from dashboard panels.

Relevant source visual references source parameters are represented as native option names:
`focus`, `xRatio`, `yRatio`, `radiusRatio`, `aperture`, `maxblur`,
`blurRadius`, `gradientRadius`, `start`, `end`, `strength`, `radius`,
`threshold`, `noiseIntensity`, `scanlinesIntensity`, `scanlinesCount`, `shape`,
`scatter`, `center`, `metalness`, `roughness`, `shininess`, `specular`,
`specularPower`, `sphereShade`, `gloss`, `edgeShade`, `shadowDensity`,
`shadowSoftness`, `highlightSharpness`, `saturation`, `vertexColors`, and
`sizeAttenuation`.
Grounding-specific native options include `groundY`, `gridScale`,
`footprintStretch`, `footprintSkew`, `footprintCore`, `footprintFalloff`,
`colorMix`, `shadowDensity`, `coreDensity`, `edgeTint` and `neutralShadow`.
Contact-specific options include `contact.alpha`, `contact.pointPixelScale`,
`contact.radiusScale`, `contact.heightScale`, `contact.footprintStretch`,
`contact.footprintSkew`, `contact.colorMix`, `contact.shadowDensity`,
`contact.coreDensity`, `contact.edgeTint` and `contact.neutralShadow`.

The METRIC GRAE reference remains useful for high-volume point
clouds, 2D/3D morphs, turntable interaction and animation continuity. It is
not a runtime dependency or naming source for the METRIC visual library.
