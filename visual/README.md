# METRIC Visual

Native browser visualization for finite metric spaces, records, relations,
graphs, coordinate states, properties, timelines, mappings, and solver traces.

This library has no build step and no JavaScript package dependencies. It uses
plain ES modules, Canvas 2D, raw WebGL, METRIC-owned GLSL shaders, and the
local `metric-webgl` runtime for photographic scene/postprocess rendering. That
runtime is METRIC source code under direct edit. Native C++ examples export
`metric.evidence.v1` today and should converge on `metric.visual.v1` as the
stable import/export format.

Historical browser plots are treated only as rendered evidence for scale,
morph timing, camera motion, and interaction quality. They are not
implementation dependencies and do not define public APIs, namespaces, package
structure, or data contracts.

Open the current native engine probe through any static file server rooted at
the repository:

```text
visual/examples/native-engine-probe/index.html
```

The process-curve condition-monitoring example uses the same native runtime and
miniature scene bundle for its main spatial view:

```text
visual/examples/process-curve-condition-monitoring/index.html
```

The GRAE10 metric-engine viewer is the first `metric-webgl` path. It loads
60,000 records, morphs the same records between a 2D target projection and 3D
metric-space coordinates, keeps labels inside the 3D scene and uses the
camera-depth postprocess for miniature depth of field:

```text
visual/examples/grae10-metric-engine/index.html
```

The miniature look gallery renders one descriptor set through every reusable
photographic look contract:

```text
visual/examples/miniature-look-gallery/index.html
```

The demo expects:

```text
docs/examples/assets/process-curve-external/evidence.json
```

## Native Engine Modules

The new native engine is split by responsibility:

- `visual/src/engine/`: canvas, WebGL context, buffers, render targets, render loop.
- `visual/src/runtime/metric-webgl/`: editable METRIC WebGL scene/runtime code;
  renderer, material, postprocess, camera and picking changes belong here.
- `visual/src/animation/` and `visual/src/timeline/`: typed-array morphing, exported timeline state and render-time interpolation.
- `visual/src/geometry/` and `visual/src/glyphs/`: native geometry builders and semantic glyph registry.
- `visual/src/camera/`: perspective camera and target/orbit camera state.
- `visual/src/interaction/`: pointer normalization, focus line, camera controls.
- `visual/src/data/`: `metric.visual.v1` validation, indexing, `metric.evidence.v1` adapter.
- `visual/src/encoders/`: typed-array channel encoders for renderer layers.
- `visual/src/views/`: view descriptors that map exported evidence to visual layers.
- `visual/src/views/ProcessCurveSceneView.js`: reusable process-curve scene
  builder for morph points, semantic ground projection, state density, record
  tracks and grounded record-volume skyline.
- `visual/src/layers/`: descriptor-to-GPU WebGL layers for points, boxes, heat fields, surfaces, edges and ground projections.
- `visual/src/relational/`: dense relation matrix, sparse graph and relation diagnostics visual builders.
- `visual/src/curves/`: path, ribbon and trajectory visual builders for exported paths and record evolution.
- `visual/src/materials/` and `visual/src/shaders/`: native material descriptors and reusable GLSL chunks/factories.
- `visual/src/style/miniature/`: photographic miniature scene bundles, stage,
  scene fit, camera, material, descriptor presets, reference looks and look
  atlas diagnostics.
- `visual/src/picking/` and `visual/src/selection/`: record/edge hit mapping, hover state, selection state and brushing helpers.
- `visual/src/native-postprocess/`: native fullscreen passes, FXAA-like pass,
  scene color targets and raw camera-depth textures.
- `visual/src/postfx/`: camera-depth depth of field, final miniature frame
  shaping, color grade, vignette, restrained bloom and optional grain passes.
- `visual/src/miniature-field/`: transplanted instanced-field reference material, kept as renderer study material rather than product API.

The public ES-module entrypoint is:

```js
import * as mtrcVisual from "./visual/src/index.js";
```

The current engine path is: C++ evidence export -> `metric.visual.v1` adapter
-> `VisualSpace` -> view descriptors -> typed channels -> WebGL layers ->
native renderer/postprocess.

The native probe currently exercises:

- `metric.evidence.v1` -> `metric.visual.v1` adaptation
- `MetricVisualRuntime`
- reusable miniature scene bundles and runtime stage application
- reusable `MiniaturePhotographicStyle` owner for one consistent camera, focus,
  postprocess, ground, light and motion contract
- serializable `MiniatureStyleProfile` objects for carrying the miniature
  stage/camera/light/focus/ground/post-FX contract between examples, docs and
  future `metric.visual.v1` documents
- style-level focus breathing, camera drift and turntable motion that updates
  runtime camera/focus/post-FX state without changing metric evidence
- serializable `createMiniatureStyleMotionAtlas()` contracts for reusable
  focus, camera and postprocess motion, including sampled offsets for
  documentation and regression tooling
- reusable reference looks and `createMiniatureLookAtlas()` diagnostics for
  checking camera, light, focus, ground, material, post-FX and motion coverage
  without a demo page
- `createMiniatureLookSceneAtlas()` for applying every reusable look to the
  same descriptor bundle as a visual regression surface
- eight native reference looks: bright tabletop, chromatic surface, dark tube
  field, dark gloss studio, low-angle metric city, colored shadow stage, studio
  board and surface ripple
- style-contract diagnostics that verify camera, lighting, focus,
  depth-of-field, material, ground, post-FX, isometric staging, animation and
  runtime coverage
- reusable `createMiniatureRuntimeOptions()` setup for camera, controls,
  camera-depth/postprocess and layer factory wiring
- `ProcessCurveSceneView` shared between the native probe and the
  condition-monitoring example
- runtime stage focus consumed by pointer controls, camera DoF and compatibility
  native tilt-shift together
- stage grounding consumed by GroundPlane and GroundProjection layers for floor,
  grid and semantic colored shadow footprints
- automatic contact shadows for grounded solid, glyph and curve-path layers
  through the reusable miniature descriptor bundle
- stage light rigs lowered into native shader material uniforms
- semantic ground field and colored projection shadows
- ground projection
- coordinate morphs
- curve ribbon and tube paths
- surface/field shader motion for miniature density and entropy-style views
- raw camera-depth texture and camera DoF
- machine-readable miniature rig reports exposing
  `metricDofModel=camera-depth-circle-of-confusion`,
  `metricDofDepthMode=raw-camera-depth`, focus distance, aperture, focal range
  and active post-FX passes without visible debug UI
- machine-readable photographic rig fields for camera, light, ground,
  material family coverage, motion hooks and same-room morph state
- final post-FX miniature frame and grade
