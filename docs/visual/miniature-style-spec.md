# Miniature Photographic Style Spec

This spec defines the native METRIC miniature style subsystem in
`visual/src/style/miniature/`. It ports the reusable visual grammar seen in
source visual references demos into dependency-free vanilla METRIC descriptor presets. It does not
port source visual references, Vue, external renderer, demo subjects, runtime adapters, or data algorithms.

## Source Audit

Local sources inspected:

- `/tmp/metric-trois-site/assets/Demo1.ea0cb3fb.js`
- `/tmp/metric-trois-site/assets/Demo2.836d891c.js`
- `/tmp/metric-trois-site/assets/Demo3.e1be2da1.js`
- `/tmp/metric-trois-site/assets/Demo4.ef5a6b30.js`
- `/tmp/metric-trois-site/assets/Demo5.ee55b5a1.js`
- `/tmp/metric-trois-site/assets/Demo6.7620ed17.js`
- `/tmp/metric-trois-site/assets/Demo7.e98226ba.js`
- `/tmp/metric-trois-site/assets/ShadowsExample.b12b538f.js`
- `/tmp/metric-trois-site/assets/LightExample.80edbe9a.js`
- `/tmp/metric-trois-site/assets/LoopExample.de5a1f3d.js`
- `/tmp/metric-trois-site/assets/MeshesExample.dae2c1e3.js`
- `/tmp/metric-trois-site/assets/SubSurface.1f1d6931.js`
- `/tmp/metric-trois-site/assets/Matcap.6ac6e810.js`
- `/tmp/metric-trois-site/assets/Textures.53c6bcc6.js`
- `/tmp/metric-trois-site/assets/Slider1.c172becc.js`
- `/tmp/metric-trois-site/assets/Slider2.b00bdd98.js`
- `/tmp/metric-trois-site/assets/screenshots/troisjs_5.jpg`
- `/tmp/metric-trois-site/assets/screenshots/troisjs_6.jpg`
- `/tmp/metric-trois-site/assets/screenshots/troisjs_7.jpg`
- `/tmp/metric-trois-site/assets/screenshots/troisjs_8.jpg`
- `/tmp/metric-trois-site/assets/screenshots/troisjs_13.jpg`
- `/tmp/metric-trois-site/assets/screenshots/troisjs_14.jpg`
- `/tmp/metric-trois-site/assets/screenshots/troisjs_15.jpg`
- `output/playwright/trois-demo1.png`
- `output/playwright/trois-demo3.png`
- `output/playwright/trois-demo5.png`
- `output/playwright/trois-demo6.png`
- `output/playwright/trois-demo7.png`
- `output/playwright/trois-lights.png`
- `output/playwright/trois-shadows.png`
- `output/playwright/trois-materials-subsurface.png`
- `output/playwright/trois-materials-matcap.png`
- `/tmp/metric-ref-trois-site/src/components/demos/Demo1.vue`
- `/tmp/metric-ref-trois-site/src/components/demos/Demo2.vue`
- `/tmp/metric-ref-trois-site/src/components/demos/Demo3.vue`
- `/tmp/metric-ref-trois-site/src/components/demos/Demo4.vue`
- `/tmp/metric-ref-trois-site/src/components/demos/Demo5.vue`
- `/tmp/metric-ref-trois-site/src/components/demos/Demo6.vue`
- `/tmp/metric-ref-trois-site/src/components/demos/Demo7.vue`
- `/tmp/metric-ref-trois-site/src/components/misc/LightExample.vue`
- `/tmp/metric-ref-trois-site/src/components/misc/ShadowsExample.vue`
- `/tmp/metric-ref-trois-site/src/components/materials/SubSurface.vue`
- `/tmp/metric-ref-trois/src/effects/TiltShiftPass.ts`
- `/tmp/metric-ref-trois/src/shaders/TiltShift.ts`
- `/tmp/metric-ref-trois/src/materials/SubsurfaceScatteringShader.ts`
- `/tmp/metric-ref-trois/src/components/liquid/LiquidPlane.js`
- `output/playwright/trois-reference/demo1.png`
- `output/playwright/trois-reference/demo3.png`
- `output/playwright/trois-reference/demo4.png`
- `output/playwright/trois-reference/demo5.png`
- `output/playwright/trois-reference/demo6.png`
- `output/playwright/trois-reference/demo7.png`
- `output/playwright/trois-reference/lights.png`
- `output/playwright/trois-reference/shadows.png`
- `output/playwright/trois-reference/subsurface.png`
- `/tmp/metric-trois/src/effects/BokehPass.ts`
- `/tmp/metric-trois/src/effects/FXAAPass.ts`
- `/tmp/metric-trois/src/effects/FilmPass.ts`
- `/tmp/metric-trois/src/effects/HalftonePass.ts`
- `/tmp/metric-trois/src/effects/SMAAPass.ts`
- `/tmp/metric-trois/src/effects/SSAOPass.ts`
- `/tmp/metric-trois/src/effects/TiltShiftPass.ts`
- `/tmp/metric-trois/src/effects/UnrealBloomPass.ts`
- `/tmp/metric-trois/src/effects/ZoomBlurPass.ts`
- `/tmp/metric-trois/src/shaders/TiltShift.ts`
- `/tmp/metric-trois/src/materials/Material.ts`
- `/tmp/metric-trois/src/materials/MatcapMaterial.ts`
- `/tmp/metric-trois/src/materials/ShaderMaterial.ts`
- `/tmp/metric-trois/src/materials/SubSurfaceMaterial.ts`
- `/tmp/metric-trois/src/materials/types.ts`
- `/tmp/metric-trois/src/components/sliders/AnimatedPlane.js`
- `/tmp/metric-trois/src/components/sliders/ZoomBlurImage.js`
- `/tmp/metric-trois/src/components/noisy/NoisyPlane.js`
- `/tmp/metric-trois/src/components/noisy/NoisyImage.js`
- `/tmp/metric-trois/src/components/liquid/LiquidPlane.js`
- `visual/examples/process-curve-condition-monitoring/index.html`
- `visual/src/metric-visual.js`
- `/Users/michaelwelsch/Developer/iCloud-restore-Documents-20260511-065154/files/alt/Dokumente - MacBook Air von Michael/MetricSpaceAI/Youtube/Verfahren zum Embedden eines Manifolds/python/plots/GRAE10.html`

Extracted reusable aesthetic primitives:

- White or black photographic stages.
- Long-lens straight-on, high oblique, and miniature oblique cameras aimed at a
  centered subject.
- Orbit damping and optional auto-rotation as scene composition.
- Ambient fill plus colored point or spot light rigs.
- Multi-light roles as material input: ambient, key, fill and colored point
  lights must survive the descriptor transform instead of being collapsed to a
  single light. Native mesh, surface and curve layers consume the same packed
  light arrays.
- Phong-like vertex/data color lighting.
- Standard/Physical glossy material traits: metalness, roughness, specular,
  shininess, transparency, flat shading, vertex colors, size attenuation.
- Native material-response controls for data marks: point/glyph sphere shading,
  gloss, edge shading, saturation and shadow density; mesh roughness,
  metalness, specular power, soft shadows, base lift and highlight/shadow tint;
  surface roughness, saturation, shadow tint, highlight color, rim light and
  base lift.
- Texture-like material channels: color, displacement, normal, roughness and
  ambient-occlusion concepts. Native METRIC does not need image texture assets
  for every layer, but the material contract must leave room for equivalent
  data-derived maps.
- Matcap, shader material, and subsurface-style extension points as native
  descriptor metadata.
- FXAA/SMAA antialiasing.
- Tilt-shift two-pass blur with `blurRadius`, `gradientRadius`, `focusBand`,
  `blurCurve`, `start`, `end`, and pointer-driven focus line.
- Viewport-ratio focus-plane contract so resize preserves the photographic
  miniature focus zone.
- Post-FX size contract: all active frame, grade, vignette, bloom and grain
  passes must use the same drawing-buffer size as the host renderer from the
  first rendered frame.
- Miniature-frame lens cues tied to the same focus state as camera DoF or the
  compatibility tilt-shift path:
  `lensAberration`, `apertureVignette` and `focusDesaturation` are subtle
  camera effects for the out-of-focus part of the frame, not decorative UI
  filters.
- Bokeh controls: `focus`, `aperture`, `maxblur`.
- Bloom controls: `intensity`, `threshold`, `knee`, `radius`, `iterations`,
  `downsample`, `saturation`, `warmth`, `clamp`, and `sourceIntensity`.
  `strength` is accepted only as an import alias for source visual references-style references;
  the native METRIC post-FX stack stores and reports `intensity`.
- Film, halftone, SSAO, and zoom-blur controls.
- Simplex-like shader displacement/noise parameters for photographic texture,
  not metric recomputation.
- Pointer-coupled focus or style motion.
- Grounding contract for the floor, grid, semantic colored shadows and
  projection footprints.
- Shadow-catcher floor contract: palette-aware matte surface, subdued grid,
  horizon/floor falloff, table sheen, contact shade, directed shadow tails and
  separate semantic tint.
- Progress-driven shader transitions for morphs and view changes. Slider
  examples show that transitions should be shader attributes/uniforms, not DOM
  page effects.

Important example findings:

- Demo 5 is not useful because it contains boxes. It is useful because it
  demonstrates the photographic contract: many instanced solid objects,
  pointer-controlled focus, a low camera, a clean white stage and no explanatory
  UI in the shot.
- The Shadows and Physics examples show that contact with a surface matters as
  much as the objects themselves. METRIC visuals therefore need a ground
  relation for every spatial scene, including semantic projections and contact
  shadows.
- The Light and Shadows examples also show that a floor is not a background.
  It is the visual object that establishes scale and contact. METRIC therefore
  treats the floor, contact cores, shadow tails and semantic projections as one
  reusable stage relation.
- The Light, SubSurface, Matcap and flower/water examples show that material
  response is part of the brand. Data marks must not remain flat debug glyphs
  when they are meant to be the main subject.
- Demo 6 and the Physics examples show why one global light is insufficient:
  colored corner lights and moving key lights create the photographic object
  response. METRIC mesh-family layers therefore accept packed multi-light
  uniforms with up to four native lights.
- Demo 6 also shows that path evidence reads as a physical miniature object
  only when it has world-space thickness and lighting response. METRIC
  therefore has `CurveTubeMeshLayer` for process curves, diffusion
  trajectories, transformation paths and solver histories that should not be
  flattened into screen-space ribbons.
- The Texture and Meshes examples show that visual quality depends on a material
  grammar, not one shader. Mesh-family layers need clear room for roughness,
  specular response, normal-like detail and data-derived surface variation.
- The Material, Light, Shadows and Demo 5 examples also show that the same
  records must remain readable under different lighting. METRIC therefore keeps
  data color, semantic shadow/projection color and photographic shading as
  separate controls instead of baking one presentation into channel data.
- The Slider examples show a different reusable idea: progress is a scene
  uniform that can drive geometry offsets, rotations, opacity and blur. METRIC
  morphs and dimensional transitions should use the same shader-level progress
  idea over exported evidence.
- The process-curve example now uses `MetricVisualRuntime` for its main
  miniature scene through `visual/src/views/ProcessCurveSceneView.js`. The
  remaining 2D Canvas panels are inspection panels over the same evidence, not
  the presentation architecture for the hero frame.
- `visual/examples/miniature-hero-frame/index.html` is the current
  full-frame-capture reference. It renders the process-curve evidence through
  the reusable Hero/Capture helpers rather than copying stage, runtime and
  postprocess wiring in page code. The current Hero preset uses a lower
  oblique camera, a tighter `targetSpan`, stronger focus falloff, stronger
  contact shadows, and subtle lens/aperture cues to stay closer to the Demo
  5-style miniature-camera reference without copying its box subject.

METRIC translation:

- Records may be points, volumes, fields, curves, relation matrices or surfaces.
  The chosen visual primitive must express the metric question, not copy a source visual references
  subject.
- A record-volume skyline is valid when the exported metric coordinates provide
  placement and the exported record payload provides a scalar visual channel
  such as curve energy, entropy contribution, density, uncertainty or anomaly
  strength.
- Thin lines and points are secondary annotation layers unless the algorithmic
  result is itself a graph or path.
- Text HUDs, status boxes and dashboard panels are not part of the photographic
  miniature frame. They can exist as inspection UI, but must not dominate hero
  captures.

## Coordinate Convention

The METRIC renderer does not inherit source visual references scene axes. The style subsystem
adapts the photographic grammar to METRIC's visual-space convention:

- `x/z` form the grounded miniature surface.
- `y` is the vertical lift/projection axis for shadows, density fields,
  morphs, and staged records.
- the default camera is a low oblique perspective looking across the `x/z`
  ground plane.
- the default camera up vector is `[0, 0, 1]`, so the ground reads like a
  photographed tabletop rather than a generic plotting plane.

This axis convention is a METRIC renderer decision. It is not copied from source visual references.

## Non-Goals

- No box-specific API or Demo5 subject clone.
- No METRIC math, layout, dimensionality reduction, distance computation, or
  channel derivation in this package.
- No CSS theme.
- No runtime dependencies, npm packages, build tools, TypeScript, source visual references, Vue,
  external renderer, or adapter wrappers.
- No algorithmic data derivation in JavaScript. Renderer-side calibrating,
  projection, material motion and focus animation are allowed only as visual
  transforms over exported evidence.

## Public API

Direct import:

```js
import {
  MiniaturePhotographicStyle,
  applyMiniatureSceneBundle,
  applyMiniatureLightingToMaterial,
  applyMiniatureStyleToDescriptors,
  computeMiniatureSceneBounds,
  createMiniatureAnimationPreset,
  createMetricMiniaturePalette,
  createMiniatureCameraOptions,
  createMiniatureContactShadowDescriptor,
  createMiniatureContactShadowDescriptors,
  createMiniatureDescriptorBundle,
  createMiniatureFieldMaterial,
  createMiniatureFocusPlane,
  createMiniaturePhotographicStyle,
  createMiniaturePhotographicStyleFromProfile,
  createMiniatureGroundDescriptorOptions,
  createMiniatureGroundingPreset,
  createMiniatureGroundMaterial,
  createMiniatureHeroProfile,
  createMiniatureHeroRuntimeOptions,
  createMiniatureHeroSceneBundle,
  createMiniatureHeroStage,
  createMiniatureHeroStyle,
  createMiniatureLightRig,
  createMiniatureLookAtlas,
  createMiniatureLookAtlasEntry,
  createMiniatureLookSceneAtlas,
  createMiniatureLookSceneAtlasEntry,
  createMiniatureLineMaterial,
  createMiniatureMaterial,
  createMiniatureMeshMaterial,
  createMiniatureNoiseOptions,
  createMiniaturePointMaterial,
  createMiniaturePostprocessOptions,
  createMiniatureProjectionStyle,
  createMiniatureReferenceLookOptions,
  createMiniatureRuntimeOptions,
  createMiniatureSceneBundle,
  createMiniatureStyleMotionAtlas,
  createMiniatureStyleMotionAtlasEntry,
  createMiniatureStyleProfile,
  createMiniatureStyleMotionPreset,
  createMiniatureStagePreset,
  createMiniatureSurfaceMaterial,
  createMiniatureTiltShiftOptions,
  createTiltShiftOptions,
  fitMiniatureSceneDescriptors,
  flattenMiniatureLightRig,
  inspectMiniatureStyleContract,
  inspectMiniatureStyleProfile,
  isMiniatureStyleProfile,
  listMiniatureReferenceLooks,
  listMiniatureStyleMotionPresets,
  normalizeMiniatureStyleProfile,
  resolveMiniatureFocusLine,
  resolveMiniatureAnimationForFamily,
  sampleMiniatureStyleMotion,
  selectMiniatureStyleMotionForLook,
  summarizeMiniatureStyleMotion,
  miniatureGroundFromFit,
} from "../../visual/src/style/miniature/index.js";
```

The same API is also available through `visual/src/index.js`.

## Descriptor Contract

`createMiniaturePhotographicStyle()` is the stateful style/postprocess owner
for a visual surface. It keeps one stage contract and applies it to arbitrary
METRIC descriptors or prebuilt scene bundles. It is the preferred API when a
page, hero application or visual gallery needs one consistent miniature look
instead of hand-wiring camera, focus, postprocess, ground, light and motion in
the example.

`MiniaturePhotographicStyle` exposes:

- `createRuntimeOptions()` for `MetricVisualRuntime` setup.
- `styleDescriptors()` for material/light/motion descriptor styling.
- `createDescriptorBundle()` for styled layers without coordinate calibrating.
- `createSceneBundle()` for staged, calibrated miniature scenes.
- `attachRuntime()`, `applyStage()`, `applyBundle()` and `applyToRuntime()` for
  runtime application.
- `applyFocus()` and `applyPostprocess()` for live updates without rebuilding
  layers.
- `setStyleMotion()`, `attachStyleMotion()` and `detachStyleMotion()` for
  focus/camera/postprocess motion over the existing runtime clock.
- `inspectContract()` for camera, lighting, focus, depth-of-field, material,
  ground, post-FX, isometric staging, animation, runtime coverage and the
  combined photographic-style signal set.
- `toProfile()` for exporting the serializable miniature style contract.

`createMiniatureStyleProfile()` is the renderer-facing style profile. It stores
stage, camera, light rig, focus, ground relation, post-FX, calibrating defaults and
style motion as plain data. It is the correct way to move a METRIC miniature
look between examples, documentation captures and future visual documents. It
does not store records, relation values, algorithm evidence, runtimes, DOM
nodes, WebGL objects or functions.

`createMiniatureStagePreset({ look })` accepts native reference-look presets
derived from the inspected source visual references examples. These are style recipes, not demo
subjects and not runtime adapters:

- `white-tabletop`: Demo5/Shadows-style bright tabletop, focus plane, floor
  contact, semantic projection and restrained highlight bloom.
- `chromatic-surface`: Demo1-style high-oblique surface/material response with
  chromatic moving studio light roles.
- `dark-tube-field`: Demo6-style dark glossy tube/path field with luminous
  line material and stronger selective bloom.
- `dark-gloss-studio`: Demo7/materials-style dark glossy object/surface
  response with colored corner lights.
- `metric-city`: DOF-test/Demo5-style low-angle dense field for many grounded
  records, stronger contact shadows and shallow focus.
- `colored-shadow-stage`: ShadowsExample/Demo4-style colored light and
  shadow field for categorical state, class and anomaly separation.
- `studio-board`: LightExample-style dark area-light board for relation
  panels, density fields and floor-heavy projections.
- `surface-ripple`: Demo7/Demo1-style glossy surface response for entropy,
  density, transformation and path evidence over a semantic floor.

`listMiniatureReferenceLooks()` lists those plain option bundles.
`createMiniatureReferenceLookOptions(name, overrides)` returns one bundle with
overrides deeply merged so examples can switch look families without copying
camera, light, material and postprocess wiring.

`createMiniatureLookAtlas()` creates a serializable atlas of those looks. Each
entry includes a miniature style profile, a compact contract summary, and the
same diagnostic gate used by runtime examples. The atlas is the preferred
inspection surface for documentation, galleries and regression checks because
it proves camera, light, focus, ground, material, post-FX and style-motion
coverage without needing metric records, WebGL handles or a demo page.

`createMiniatureLookSceneAtlas(descriptors, options)` goes one step further:
it applies every look to one descriptor set and returns ready scene bundles.
Use this to prove that the style/postprocess layer is reusable across arbitrary
METRIC visuals, not tied to one page or one renderer setup.

`createMiniaturePhotographicStyleFromProfile(profile)` rebuilds the style owner
from that plain profile. `inspectMiniatureStyleProfile()` runs the same
contract diagnostic without needing a live runtime.

`inspectMiniatureStyleContract()` is also available as a stateless function for
stage/bundle/runtime diagnostics. It returns `status: "ready"` only when the
photographic miniature contract is covered by current evidence. It reports
missing pieces as structured warnings; it does not inspect records or compute
metric values.

The `photographicStyle` diagnostic is a combined gate over the renderer-facing
style signals. It requires perspective tabletop camera cues, at least ambient
plus a directed light role, active focus optics (`focusBand`, `blurCurve`,
`blurRadius`, `gradientRadius`), ground/projection presence, miniature material
coverage, frame/grade/vignette post-FX, pass sizing and either style or
descriptor animation when descriptors are present.

`createMiniatureSceneBundle()` is the preferred high-level API for arbitrary
METRIC visuals. It combines scene calibrating, stage creation, ground-plane sizing,
descriptor styling, contact-shadow generation and runtime-ready layer
generation. It returns `{ stage, layers, fit }`.

`createMiniatureRuntimeOptions()` is the high-level API for arbitrary runtime
surfaces. It maps one stage into `MetricVisualRuntime` options: camera, scene
background, focus/postprocess, controls, renderer flags and layer-factory
wiring. It must pass the stage `focus` contract into the Runtime `focusLine`
so initial render, resize handling, pointer focus and the tilt-shift pass all
share the same viewport-ratio Schärfeebene. Examples should use this bridge
instead of copying Runtime setup.

`createMiniatureHeroStage()`, `createMiniatureHeroProfile()`,
`createMiniatureHeroStyle()`, `createMiniatureHeroSceneBundle()` and
`createMiniatureHeroRuntimeOptions()` are the stricter full-frame capture
helpers. They are not a separate renderer and not process-curve-specific. They
apply the same profile/runtime/bundle model with a stronger capture preset:
low oblique camera, larger staged footprint, brighter but less washed-out
tabletop, stronger contact/projection shadows, semantic color retention,
restrained studio drift and a tuned miniature frame pass.

`applyMiniatureSceneBundle(runtime, bundle)` applies the stage and layer
descriptors through `MetricVisualRuntime` hooks.

`applyMiniatureStyleToDescriptors()` accepts ordinary METRIC visual layer
descriptors and returns ordinary layer descriptors. It preserves `channels`
without cloning typed arrays. The transform only merges:

- `material`
- `geometry` style hints
- `animation` style hints
- `metadata`
- `order` when an offset is requested

Primitive-to-family mapping is generic:

- `InstancedPointLayer` and `InstancedGlyphLayer` -> `sample`
- `InstancedBoxLayer` -> `mesh`
- `SurfaceLayer` -> `surface`
- `HeatFieldLayer` and `GroundProjectionLayer` -> `field`
- `RelationEdgeLayer` and `CurveRibbonLayer` -> `line`
- `RelationMatrixLayer` -> `field`
- `GroundPlaneLayer` -> `ground`
- `BillboardLabelLayer` -> `sample` with the dedicated `miniature-label`
  role

The presence of `InstancedBoxLayer` in the mapping is renderer compatibility,
not a public box subject API.

Labels are scene descriptors, not page overlays. A label-bearing grammar must
emit `BillboardLabelLayer` descriptors so labels share the same camera,
selection and miniature-style contract as the surrounding evidence. The public
miniature-scene gate rejects label roles that do not resolve to a
`BillboardLabelLayer`.

When `stage.grounding.contact.enabled` is true, grounded solid, glyph, and
curve-path source layers receive an additional `GroundProjectionLayer` contact
descriptor. Solid and glyph contacts reuse exported position, size, height, and
color channels. Curve contacts reuse normalized path evidence and project the
path samples to `groundY`. In both cases the contact layer is a visual shadow
companion: it projects existing evidence onto the stage floor and never
computes metric values. Disable this with `contacts: false` for non-grounded
scenes.

## Stage Contract

`createMiniatureStagePreset()` returns:

- `scene`: background and clear color.
- `camera`: perspective composition options.
- `focus`: serializable viewport-ratio focus-plane options.
- `grounding`: serializable floor/projection/contact-shadow options.
- `lightRig`: ambient/key/fill or colored point rig descriptors.
- `materials`: sample, mesh, surface, field, line, and ground families.
- `postprocess`: camera-depth depth of field, FXAA/SMAA, compatibility
  tilt-shift, Bokeh, bloom, grade, vignette, film grain, halftone, SSAO,
  zoom-blur, and a runtime-ready `postFx` object.
- `interaction`: pointer focus and orbit damping hints.

Renderer integration is expected to consume these plain objects through native
METRIC renderer hooks. The stage contract now has three paths:

- runtime hooks consume `scene`, `camera`, `focus` and `postprocess`.
- descriptor style transforms consume `lightRig` and `materials`, then write
  shader-ready material uniforms into native layer descriptors.
- ground/projection descriptor transforms consume `grounding`, then write
  GroundPlane and GroundProjection material options.
- `postprocess.tiltShift` is derived from `focus`; it is a compatibility path,
  not the source of truth for stage composition.
- `postprocess.postFx` is the canonical runtime container for restrained
  camera-depth depth of field, highlight bloom, miniature frame shaping,
  photographic color grade, vignette, and film grain. Bloom is enabled by
  default in the miniature stage as a low-intensity highlight recovery pass,
  not as a decorative glow.
  Runtime callers may also pass top-level `miniatureFrame`, `grade`,
  `vignette`, `bloom`, `film`, or `filmGrain`; those are normalized into
  `postFx` unless `postFx: false` is explicitly set.

The style subsystem does not instantiate render passes. `MetricVisualRuntime`
instantiates the native passes from these plain objects.

## Focus Contract

`createMiniatureFocusPlane()` returns a serializable focus object:

- `kind: "miniature-focus-plane"`
- `anchor: "viewport-ratio"` by default
- `axis: "horizontal"` or `"vertical"`
- `xRatio`, `yRatio`, and `radiusRatio`
- `focusBandRatio`
- `blurRadius`
- `blurCurve`
- `pointer` focus hints

`resolveMiniatureFocusLine(focus, width, height)` converts that contract to the
pixel-space `start`, `end`, `radius`, `focusBand`, and `blurCurve` uniforms
used by the native compatibility `TiltShiftPass`. `focusBand` is the fully
sharp core around the focus line; `blurCurve` controls the ramp into the
out-of-focus region for renderers that cannot provide camera depth.

The photographic miniature path uses `MiniatureCameraDofPass` instead. It
receives a camera-depth texture from the same camera that rendered the scene,
reconstructs camera distance from `depthNear` and `depthFar`, and derives blur
from `focusDistance`, `aperture`, and circle of confusion. The same runtime
focus state is also passed to `MiniatureFramePass`, where it drives restrained
focus clarity and local stage lift after the optical pass has produced the
depth-aware focus separation.

The runtime and rig report must expose this as
`camera-depth-circle-of-confusion`, not as a generic blur. A valid hero frame
therefore reports raw camera depth, camera-space focus distance,
aperture-relative defocus, focal range, aperture, max blur and active
near/far-plane reconstruction. Visible debug bars are not part of the
photographic frame; those values live in the machine-readable `metric*`
dataset fields.

The frame pass also carries the reusable camera-stage finish:

- `stageCenter`, `stageScale` and `stageTilt` define the screen-space tabletop
  matte.
- `floorRatio`, `floorColor`, `skyColor`, `floorSheen`, `floorFalloff` and
  `groundShade` shape the floor/sky relation.
- `stageSpotlight` and `stageShadow` shape the tabletop light pool and edge
  falloff so colored looks read as lighting on a floor instead of a flat
  tinted background.
- `subjectIsolation` and `microContrast` use color/luma/focus cues to separate
  data objects from the photographic floor without needing algorithmic data.
- `shadowColor`, `highlightColor` and `backlight` define restrained studio
  light direction for arbitrary METRIC scenes.

The ratio values are authoritative when `anchor` is `viewport-ratio`; embedded
pixel values are only a snapshot for consumers that need immediate pass
options. This prevents a stage from baking the default 100px preset size into a
real canvas.

## Grounding Contract

`createMiniatureGroundingPreset()` returns the stage-level contract that keeps
floor, grid, projection shadow and contact semantics aligned:

- `kind: "miniature-grounding"`
- `plane: "xz"`
- `groundY`
- `ground`: grid scale, grid width, axis width, alpha and fade
- `projection`: semantic colored shadow material controls
- `contact`: contact-shadow defaults for grounded solid, glyph, and curve-path
  layers

`createMiniatureProjectionStyle()` creates the material options consumed by
`GroundProjectionLayer`. The important photographic controls are:

- `footprintStretch`: elongates projected shadows along one screen-space axis.
- `footprintSkew`: creates the isometric slant expected from a stage light.
- `footprintCore` and `footprintFalloff`: control the dense center and soft
  fade of the shadow.
- `colorMix`: keeps semantic color in the shadow while allowing photographic
  desaturation.
- `shadowDensity`: increases or reduces the whole projection density.
- `coreDensity`: darkens the contact core without changing exported geometry.
- `edgeTint`: controls how much semantic color remains in the feathered edge.
- `neutralShadow`: neutral photographic shadow color used before semantic color
  is mixed back in.

This is still data-preserving visualization. A projection shadow uses exported
positions and exported colors. It does not infer new metric structure.

`createMiniatureContactShadowDescriptors()` applies the same projection
language to grounded object and curve-path layers. Contact shadows are
lower-level visual companions to marks, not algorithm outputs. They exist to
make points, glyphs, volumes, and path bodies read as photographed miniature
objects on a floor instead of flat plot symbols.

## Layer Effects

Current native layer support:

- `InstancedBoxLayer` reads the miniature light rig through material uniforms.
- `SurfaceLayer` reads packed multi-light, saturation, roughness, shadow tint,
  highlight color, rim-light, base-lift and optional style-noise displacement
  uniforms.
- `HeatFieldLayer` reads contour/glow controls for density, entropy and
  semantic-ground fields.
- `CurveRibbonLayer` is classified as a line-family metric path layer.
- `CurveRibbonLayer` consumes the line material family as a photographic path
  material: `coreGlow`, `edgeFeather`, `tubeShade`, `flowStrength`,
  `flowScale`, `flowSpeed`, `saturation`, `depthShade`, `shadowTint` and
  `highlightColor` are visual response controls over exported path distance.
- `CurveTubeMeshLayer` is also classified as a line-family metric path layer,
  but renders indexed world-space tube geometry with normals. It uses the same
  material vocabulary to produce lit miniature path bodies instead of flat
  annotations.
- `RelationMatrixLayer` is classified as a field-family relation layer.
- `inspectMiniatureStyleContract()` validates the runtime post-FX contract:
  required photographic passes are present, the stack has no unsized active
  passes, and the reported pass size matches the renderer drawing buffer.
- The same diagnostic exposes `checks.photographicStyle`, which groups the
  camera, lighting, focus optics, ground/projection, material, post-FX and
  animation signals that make the scene read as one photographic miniature
  visual system.

## Animation Presets

Miniature animation is declarative and renderer-facing. Presets describe how
existing visual channels may move; they do not compute or invent metric values.

Style-level motion is deliberately separate from descriptor animation:

- `focus-breath`: animates the focus plane and final miniature frame.
- `studio-drift`: animates focus, frame response and restrained camera drift.
- `turntable`: animates camera yaw over a fixed stage.
- `isometric-orbit`: keeps the miniature object staged while orbiting the
  camera and breathing the frame.
- `ground-scan`: sweeps the focus plane over semantic ground projections while
  strengthening floor sheen and stage glow.
- `studio-pulse`: keeps a dark studio scene alive through camera, focus and
  frame-response motion.

`createMiniatureStyleMotionAtlas()` exports those style-level motions as plain
data with active domains, durations, amplitudes and sampled
focus/camera/postprocess offsets. `selectMiniatureStyleMotionForLook(look)` is
the shared selector used by the look atlas and scene atlas, so a named look
carries camera, lighting, grounding, material, postprocess and motion as one
coherent profile.

These motions update runtime camera/focus/post-FX state from the renderer
clock. They do not alter descriptor channels, metric values, coordinates,
relations or record payloads.

Runtime diagnostics must distinguish declared motion from running motion. A
valid attached style-motion controller reports:

- active domains for focus, camera and postprocess motion.
- `hasRuntime`, `hasBase` and `hasBeforeRenderHook`.
- `lastUpdate` with phase, time and per-domain application flags once the
  runtime is running.

This keeps photographic motion reusable: hero pages, algorithm galleries and
inspection surfaces can share the same slow camera/focus/frame behavior without
copying demo code.

Supported presets:

- `still`: no animation.
- `focus`: subtle focus/attention pulse for samples.
- `coordinate-morph`: transition between exported coordinate channels.
- `attention`: selection or focus emphasis.
- `uncertainty`: small renderer motion weighted by exported uncertainty.
- `surface`: shader-side surface/field motion for material texture.
- `timeline`: playback over exported timeline evidence.
- `camera-orbit`: camera composition motion, not data motion.

## Example

```js
const bundle = createMiniatureSceneBundle(layerDescriptors, {
  fit: {
    targetSpan: 2,
  },
  ground: {
    padding: 0.5,
  },
  stageOptions: {
    variant: "white",
    focus: {
      yRatio: 0.5,
      radiusRatio: 0.24,
      focusBandRatio: 0.07,
      blurRadius: 5.4,
      blurCurve: 1.42,
    },
    grounding: {
      groundY: -0.56,
      projection: {
        footprintStretch: 1.85,
        footprintSkew: -0.28,
      },
    },
  },
});

applyMiniatureSceneBundle(runtime, bundle);
```

## Why This Is Native METRIC Visualization

The output is METRIC-owned descriptor vocabulary. It can be applied to point
clouds, surfaces, fields, projections, relations, glyphs, meshes, or future
native layers without carrying a source visual references object model. The subsystem describes a
photographic look: camera, light, material, focus, postprocess, texture, and
motion. It does not define what the data means or how the data is positioned.
