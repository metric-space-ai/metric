# Curve Visuals Integration

This package is intentionally not exported from `visual/src/index.js` and not
registered in the layer factory by this task. Those files are outside the write
scope while other agents are active.

## Required Root Export Hook

Add this line to `visual/src/index.js` when the integration owner is ready:

```js
export * from "./curves/index.js";
```

## Required Layer Registry Hook

Register the native ribbon layer wherever visual layer constructors are mapped:

```js
import { CurveRibbonLayer } from "../curves/index.js";

registry.CurveRibbonLayer = CurveRibbonLayer;
```

If the registry uses a factory switch instead of a map, add:

```js
case "CurveRibbonLayer":
  return new CurveRibbonLayer(descriptor, rendererOrGl, options);
```

## Optional Future Hooks

`createPolylinePathLayerDescriptor` emits `CurvePolylineLayer` descriptors with
line-segment buffers. It can be mapped to an existing line layer or a future
dedicated curve polyline layer.

`createTubeRibbonPathLayerDescriptor(evidence, { mode: "tube" })` emits
`CurveTubeMeshLayer` descriptors with native indexed tube geometry. The
primitive is registered in the default layer factory, so world-space tubes can
be used directly when the runtime imports `visual/src/index.js`.

## Direct Imports Without Root Export

Callers can use the package directly:

```js
import {
  CurveTubeMeshLayer,
  CurveRibbonLayer,
  createTubeRibbonPathLayerDescriptor,
} from "./curves/index.js";
```

No external renderer API, npm package, TypeScript compiler, adapter wrapper, or
build step is required.
