# Native Curve Visuals

`visual/src/curves/` contains dependency-free JavaScript primitives for drawing
exported METRIC path evidence as polylines, thick camera-facing ribbons,
tube-like meshes, trajectory bundles, and ground projections.

The package does not compute METRIC algorithms. It consumes exported
coordinates, time samples, route points, solver states, or trajectory traces and
turns them into descriptor channels and GPU-ready typed arrays.

## Supported Evidence

The normalizer accepts one path, a path array, or an object containing
`paths`, `trajectories`, `routes`, `curves`, `traces`, or `histories`.

Each path can use flat typed arrays:

```js
{
  id: "solver-history-a",
  positions: new Float32Array([0, 0, 0, 1, 0.4, 0.1, 2, 0.7, 0.2]),
  times: new Float32Array([0, 1, 2]),
  colors: new Float32Array([0.1, 0.5, 0.7, 1, 0.2, 0.6, 0.6, 1, 0.9, 0.6, 0.2, 1]),
  widths: new Float32Array([2, 4, 3])
}
```

Object or tuple points are also accepted:

```js
{
  id: "nearest-neighbor-route",
  points: [
    { x: 0, y: 0, z: 0, time: 0, width: 2, color: [0.1, 0.5, 0.7, 1] },
    { x: 1, y: 0.2, z: 0.3, time: 1, width: 3 },
    [2, 0.6, 0.5]
  ]
}
```

## Descriptor Builders

```js
import {
  createGroundProjectedPathLayerDescriptor,
  createPolylinePathLayerDescriptor,
  createTrajectoryBundleLayerDescriptor,
  createTubeRibbonPathLayerDescriptor,
} from "./curves/index.js";

const ribbon = createTubeRibbonPathLayerDescriptor(evidence, {
  id: "diffusion-ribbons",
  widthScale: 1,
  resample: true,
  maxSegmentLength: 0.25,
});

const bundle = createTrajectoryBundleLayerDescriptor({ trajectories }, {
  evidenceRole: "diffusion-trajectory",
  defaultWidth: 2.5,
});

const ground = createGroundProjectedPathLayerDescriptor(evidence, {
  plane: "xz",
  groundY: 0,
});
```

`createTubeRibbonPathLayerDescriptor` defaults to `CurveRibbonLayer`, a native
WebGL layer with embedded shaders. Pass `mode: "tube"` to generate indexed
world-space tube geometry for `CurveTubeMeshLayer` without using an external renderer runtime.
Tube mode uses `options.radius` as a world-space radius unless
`useWidthsAsRadius: true` is supplied.

## Visual Resampling

`resample`, `samples`, and `maxSegmentLength` are strictly visual tessellation
controls. Resampling linearly inserts points along exported segments so ribbons
and tubes have enough vertices. It does not fit a curve, infer an algorithmic
trajectory, alter time evidence, or replace the original METRIC output.

Descriptors include:

```js
metadata: {
  algorithmicComputation: false,
  visualTessellationOnly: true,
  resampling: { evidenceMutation: false }
}
```

## Native Layers

`CurveRibbonLayer` consumes these channels:

- `start`: segment start position, item size 3
- `end`: segment end position, item size 3
- `side`: ribbon side, `-1` or `1`
- `along`: segment endpoint, `0` or `1`
- `color`: per-vertex RGBA
- `width`: per-vertex screen-pixel width
- `distance`: cumulative path distance

It uses screen-space expansion in the vertex shader, so line width is consistent
across platforms where native `gl.lineWidth` is clamped.

The ribbon shader consumes the Miniature line material family. In addition to
`alpha`, `ambient`, `pointLight`, `emission` and `rimLight`, it supports
photographic path controls:

- `coreGlow`: highlight in the path center.
- `edgeFeather`: soft edge width across the ribbon.
- `tubeShade`: tube-like cross-section shading.
- `flowStrength`, `flowScale`, `flowSpeed`: slow light movement along exported
  path distance.
- `saturation`, `depthShade`, `shadowTint`, `highlightColor`: material color
  response that keeps semantic color separate from photographic lighting.

These controls are visual material parameters only. They do not infer or alter
path geometry, time, distance values, or algorithmic evidence.

`CurveTubeMeshLayer` consumes indexed tube channels:

- `position`: world-space tube vertex position, item size 3
- `normal`: world-space tube normal, item size 3
- `color`: per-vertex RGBA
- `distance`: cumulative distance along the exported path
- `pathIndex`: path family index for multi-path bundles
- `geometry.indices`: indexed triangle topology

It renders real world-space path bodies with the same line-family photographic
material controls. This is the preferred primitive when a process curve,
diffusion trajectory, transformation path, or solver history must read as a
miniature object rather than a flat screen-space annotation.
