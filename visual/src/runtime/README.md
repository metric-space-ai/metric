# METRIC Visual Runtime

`visual/src/runtime` is the high-level native coordination layer for the METRIC visual stack. It owns a canvas-backed runtime and wires together the existing renderer, scene, miniature camera preset, focus line, camera controls, visual document/session state, native postprocess pipeline, and optional future layer factory.

## Responsibilities

- Create and expose a `VisualRenderer`, `VisualScene`, `miniaturePerspective` camera, `FocusLineState`, controls, and postprocess pipeline.
- Accept `metric.visual.v1` documents through `setDocument()` and normalize them into a `VisualSpace`.
- Accept an existing `VisualSpace` through `setVisualSpace()`.
- Store view descriptors through `setViewDescriptors()` and collect layer descriptors from views that already know how to describe their layers.
- Materialize render layers only through an injected `layerFactory` or a future `createLayerFromDescriptor` export from `visual/src/layers/index.js`.
- Degrade to a no-layer state when no layer factory exists, while exposing warnings and `layerState`.
- Coordinate selection state through `selectRecord()` and `clearSelection()`.
- Coordinate photographic focus through `setFocusTarget()`,
  `setCameraFocusPoint()` and `setCameraFocusDistance()`. Record selection can
  promote the selected record coordinate to a camera-space focus distance for
  `MiniatureCameraDofPass`.
- Optionally resolve pointer hover into a scene-bound focus target with
  `hoverFocus`. This keeps interaction photographic: the hovered record updates
  the camera depth-of-field plane instead of drawing a screen-space blur mask.
- Emit `camerafocuschange` after render when the focus target has resolved to a
  new camera distance. Examples and tests should use this event when they need
  the actually applied postprocess focus distance.
- Provide the lifecycle methods `start()`, `stop()`, `renderOnce()`, `resize()`, and `dispose()`.
- Keep the miniature style contract: the default camera is
  `miniaturePerspective`, compatibility focus-line state is forwarded to
  `TiltShiftPass.setFocusState()` when available, and the post-FX stack can
  request a raw camera-depth texture for `MiniatureCameraDofPass`.
- Expose `getState().postprocess` so tests and generated examples can verify
  active pass names, focus target, camera DoF settings, scene depth texture
  availability and depth encoding without visible debug UI.
- Report the optical DoF contract explicitly. A photographic miniature runtime
  exposes `camera-depth-circle-of-confusion`, `camera-space-distance`,
  `aperture-relative-defocus`, `raw-camera-depth`, aperture, max blur, focal
  range and the camera near/far reconstruction flags through
  `getState().postprocess.postFx.cameraDof`.
- Feed the reusable miniature rig reporter through `getState()`, so examples
  can publish a machine-readable contract for camera DoF, raw depth,
  focus/hover focus and layer state without adding visible debug bars.

## Photographic Focus Contract

`focusLine` is the compatibility path for older screen-space tilt-shift passes.
The photographic miniature path uses a scene-bound focus target:

```js
runtime.setCameraFocusPoint([x, y, z]);
runtime.selectRecord(recordId);
runtime.setCameraFocusDistance(8.4);
runtime.applyStagePreset({ focusTarget: { type: "point", position: [x, y, z] } });
```

When a frame renders, the runtime converts a world-space focus point through
the active camera `viewMatrix` and updates `MiniatureCameraDofPass` with the
resulting camera distance. A 2D-to-3D morph therefore stays in one camera room:
only layer positions move, while depth-of-field, ground relation and camera
composition remain shared.

The DoF pass is not a screen-space blur overlay. It requests the scene raw-depth
texture, reconstructs camera distance from the active camera near/far planes,
then derives blur from focus distance, aperture, focal range and
circle-of-confusion. Browser examples publish those fields through
`attachMiniatureRigReporter()` so screenshots can be checked without showing
developer controls.

The miniature scene-bundle fit enforces the same rule before descriptors reach
the runtime. Flat morph endpoints are staged as `[x, flatY, z]` above the shared
ground plane, and accidental `[x, y, 0]` 2D endpoints are reprojected to
`[x, flatY, y]`. The runtime then sees ordinary 3D channels and can apply the
same camera, focus distance and postprocess stack for both morph endpoints.

Hover focus uses the same mechanism. Enable it when layer descriptors expose a
record channel and world-space `position` or `targetPosition` channels:

```js
const runtime = new MetricVisualRuntime({
  hoverFocus: {
    enabled: true,
    thresholdPx: 54,
    positionChannels: ["position", "targetPosition"],
  },
});

runtime.on("camerafocuschange", ({ focusDistance, focusTarget }) => {
  // focusDistance is the value currently applied to MiniatureCameraDofPass.
});
```

## Non-Responsibilities

- Do not compute layouts, projections, embeddings, clustering, graph metrics, or other visual algorithms.
- Do not implement concrete layer classes in this package.
- Do not reinterpret arbitrary view descriptors into view classes. Views must already provide layer descriptors or expose `toLayerDescriptors()`.
- Do not own data validation rules beyond delegating to `VisualSpace` and the `metric.visual.v1` schema helpers.
- Do not import external packages or renderer dependencies.
- Do not mutate modules outside `visual/src/runtime`.

## Layer Factory Contract

The runtime accepts either a function:

```js
const runtime = new MetricVisualRuntime({
  layers: {
    factory: (descriptor, context) => createLayer(descriptor, context),
  },
});
```

or an object with `createLayerFromDescriptor(descriptor, context)`.

If no factory is injected, the runtime attempts an optional dynamic import of `../layers/index.js`. Missing modules are treated as an expected early-stage state: descriptors are kept, no scene children are created, and `runtime.getState().layerState` reports `missing-factory`.
