# Native picking and selection contracts

This folder contains the isolated picking primitives for the native METRIC
Visual Canvas/WebGL engine. It does not wire into the current layer or runtime
implementations yet.

## Picking id format

`index.js` exports byte utilities for RGB and RGBA picking buffers:

- `encodePickIdRGB(id)` / `decodePickIdRGB(bytes)`
- `encodePickIdRGBA(id)` / `decodePickIdRGBA(bytes)`
- `encodePickIdRGBFloat(id)` / `encodePickIdRGBAFloat(id)` for shader uniforms
- `decodePickBufferPixel(buffer, x, y, width, options)`
- `readPickIdFromFramebuffer(gl, x, y, options)`

The format stores the least significant byte in red, then green, blue, and
alpha. `0` is reserved as `NO_PICK_ID`, meaning no hit.

## Registry

`PickingRegistry` maps engine ids to numeric picking ids and back:

```js
const registry = new PickingRegistry();
const pickId = registry.registerRecord("record-42", { layerId: "points" });
const color = encodePickIdRGBA(pickId);
const entry = registry.resolve(decodePickIdRGBA(color));
```

Supported registry kinds are `record`, `layer`, and `edge`. Entries retain the
original id plus optional `recordId`, `layerId`, `edgeId`, `scope`, and
`payload`.

## CPU fallback

`pickNearestProjectedPoint(request, points, options)` is a hit-mapping fallback
for renderers or layers that can supply point data before a GPU id pass exists.
It uses `camera.projectToPixel(position)` when the point has a world-space
`position`, and it also accepts supplied screen-space `screen`, `pixel`, or
`{ x, y }` points. It returns a `PickResult`.

This fallback only maps interaction hits. It does not perform data analysis or
derive clusters.

## Pass contract

`PickingPass` is a descriptor and light execution contract. Layers can opt in by
implementing either:

```js
layer.renderPicking(context)
layer.renderPickIds(context)
```

The context includes:

- `target`: the render target or framebuffer descriptor supplied to the pass
- `registry`: the `PickingRegistry`
- `picking`: the pass descriptor
- `encodePickIdRGB`, `encodePickIdRGBA`, and normalized float variants

The pass clears the target to id `0` and calls those layer hooks when present.
Current layer implementations are not required to implement the hooks yet.
