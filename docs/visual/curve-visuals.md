# METRIC Visual Curve And Tube Visuals

`visual/src/curves/` adds native, dependency-free primitives for rendering
arbitrary exported METRIC path evidence:

- time-series curves
- transformation paths
- diffusion trajectories
- morph traces
- nearest-neighbor routes
- solver histories

The subsystem does not compute METRIC algorithms. It treats the incoming path
coordinates, time values, route ids, colors, widths, and metadata as evidence
that has already been produced elsewhere. Its job is to normalize that evidence
for visualization and build GPU buffers or descriptors.

## Package Contents

- `path-data.js`: evidence normalization, bounds, ground projection, and
  visual-only resampling.
- `tessellation.js`: line, ribbon, ground-projected, and tube typed-array
  builders.
- `descriptors.js`: layer descriptor builders.
- `CurveRibbonLayer.js`: native WebGL thick ribbon layer with embedded shaders.
- `index.js`: direct package exports.
- `README.md`: usage examples.
- `INTEGRATION.md`: exact root export and registry hooks.
- `smoke.js`: pure JavaScript smoke snippet.

## Evidence Shape

The normalizer accepts a single path, an array of paths, or an object containing
one of these keys:

- `paths`
- `trajectories`
- `routes`
- `curves`
- `traces`
- `histories`

Path coordinates can be flat typed arrays:

```js
{
  id: "diffusion-42",
  positions: new Float32Array([0, 0, 0, 1, 0.5, 0.2, 2, 0.8, 0.4]),
  times: new Float32Array([0, 1, 2]),
  widths: new Float32Array([2, 4, 3])
}
```

They can also be object or tuple points:

```js
{
  id: "morph-trace-a",
  points: [
    { x: 0, y: 0, z: 0, time: 0, color: [0.1, 0.5, 0.7, 1] },
    { x: 1, y: 0.2, z: 0.4, time: 1 },
    [2, 0.8, 0.6]
  ]
}
```

## Descriptor Builders

`createPolylinePathLayerDescriptor(evidence, options)` builds line-segment
channels for a future `CurvePolylineLayer` or another line-capable layer.

`createTubeRibbonPathLayerDescriptor(evidence, options)` builds a thick
camera-facing ribbon descriptor for `CurveRibbonLayer`. Widths are screen-pixel
widths, which avoids platform-dependent `gl.lineWidth` limits.

`createTubeRibbonPathLayerDescriptor(evidence, { mode: "tube" })` builds a
`CurveTubeMeshLayer` descriptor with indexed native tube geometry: positions,
normals, colors, path distances, path indices, and triangle indices. Use it
when world-space path bodies are required.

`createTrajectoryBundleLayerDescriptor(evidence, options)` packages multi-path
evidence such as solver histories, diffusion trajectories, or route families as
a bundle descriptor. It defaults to the native ribbon layer and can emit
polyline channels with `mode: "polyline"`.

`createGroundProjectedPathLayerDescriptor(evidence, options)` projects paths to
`xy`, `xz`, or `yz` for contextual ground overlays and then emits polyline or
ribbon channels.

## Native Ribbon Layer

`CurveRibbonLayer` follows the local native layer conventions:

- extends `BaseLayer`
- uses `createProgram`, `createBuffer`, `bindAttribute`, and camera uniforms
- embeds its vertex and fragment shaders
- uploads typed arrays from descriptor channels
- draws `gl.TRIANGLES`

The layer expands each segment in clip space from these attributes:

- `aStart`
- `aEnd`
- `aSide`
- `aAlong`
- `aColor`
- `aWidth`
- `aDistance`

Per-vertex color, alpha, and width are preserved from evidence channels.

## Native Tube Mesh Layer

`CurveTubeMeshLayer` follows the same local layer conventions, but draws
indexed triangles instead of screen-space expanded ribbons. It consumes:

- `aPosition`
- `aNormal`
- `aColor`
- `aDistance`
- `aPathIndex`

The shader applies the miniature line-family material response over actual
tube normals: multi-light diffuse/specular response, rim light, core glow,
shadow tint, highlight tint and distance-driven flow. It does not recompute
curves or alter path evidence.

## Miniature Grounding

Curve layers participate in the shared Miniature grounding contract. When a
scene bundle enables contact shadows, `CurveTubeMeshLayer`,
`CurveRibbonLayer`, and `CurvePolylineLayer` can receive a visual
`GroundProjectionLayer` companion built from the same normalized path evidence.

The contact descriptor projects existing path samples to the stage `groundY`
and uses path-specific contact sizing. It must not use screen-space ribbon
width or tube radius as a metric statement. The contact is only the
photographic floor relation that makes a curve read as a miniature path body
inside the scene.

## Visual Resampling Boundary

Path resampling is provided only for visual tessellation. It linearly inserts
additional samples on exported segments when `resample`, `samples`, or
`maxSegmentLength` is supplied.

This does not:

- fit a spline
- infer a trajectory
- change solver states
- recompute nearest neighbors
- alter diffusion or morph evidence
- replace the original exported path

Descriptors explicitly report:

```js
metadata: {
  algorithmicComputation: false,
  visualTessellationOnly: true,
  resampling: {
    evidenceMutation: false
  }
}
```

## Integration Required Later

This task intentionally avoids editing the root exports, layer registry, runtime,
examples, or shared files. The future integration owner should:

```js
export * from "./curves/index.js";
```

in `visual/src/index.js`, and register:

```js
import { CurveRibbonLayer } from "../curves/index.js";

registry.CurveRibbonLayer = CurveRibbonLayer;
```

or the equivalent factory switch case.
