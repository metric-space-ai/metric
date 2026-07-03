# METRIC Visual Layers

`visual/src/layers/` is the descriptor-to-GPU bridge for the native METRIC
Visual Engine. It consumes layer descriptors emitted by `visual/src/views/` and
uploads their typed channel arrays to raw WebGL buffers. It does not compute
METRIC data products and does not depend on npm packages, TypeScript, external renderer,
source visual references, or the earlier point-cloud renderer runtime code.

## Descriptor Mapping

- `InstancedPointLayer`: renders `position`, optional `targetPosition`,
  `color`, `size`, `alpha`, `focusWeight`/`focus`/`selection`, and
  `animationPhase`/`phase` as WebGL point sprites. Morph progress is read from
  `descriptor.animation.progress` or loop timing metadata. Its material accepts
  native photographic controls for sphere shading, gloss, edge shade,
  saturation, shadow density and highlight sharpness so dense point clouds read
  as lit miniature matter instead of flat screen markers.
- `InstancedGlyphLayer`: uses the point-sprite renderer with glyph-oriented
  defaults. It is intentionally lightweight until richer glyph meshes are
  required.
- `InstancedBoxLayer`: renders cube instances from `position`, `size`,
  `height` or `scale`, `color`, `alpha`, focus, and phase channels. It requires
  WebGL2 instancing or `ANGLE_instanced_arrays`; capabilities are taken from
  the engine when available or detected locally from the context. Its material
  accepts roughness, metalness, specular power, saturation, soft shadow,
  shadow-tint, highlight-tint and base-lift controls for a reusable
  photographed-object response.
- `HeatFieldLayer`: renders dense scalar fields as independent point
  sprites/cells from `position`, `scalar`, `color`, `radius`/`size`, and
  `alpha`. It does no interpolation between samples.
- `GroundProjectionLayer`: renders projected positions as soft radial
  footprints on `groundY` or `groundZ`. When a descriptor carries
  `geometry.footprints` and no `position` channel, simple polygon footprints
  are triangulated as flat renderer geometry. Sprite projections support
  directed shadow tails, contact hardness, semantic edge tint, neutral shadow
  color, core density and surface lift so one layer can read as both
  photographic contact and metric-space ground evidence.
- `GroundPlaneLayer`: renders the palette-aware miniature floor with a subdued
  grid, axes, horizon falloff, tabletop sheen, contact shade and stage matte
  controls.
- `SurfaceLayer`: renders vertex geometry from `position`, `color`, `alpha`,
  optional `normal`, and optional `geometry.indices`. Normals are computed only
  when `geometry.normalPolicy === "renderer-compute"`. Its material accepts the
  same photographic tone vocabulary as the other lit families: saturation,
  roughness, specular power, shadow tint, highlight color, rim light and base
  lift, plus packed multi-light uniforms and optional style-noise displacement.
- `RelationEdgeLayer`: renders sparse edges from `sourcePosition` and
  `targetPosition`, or from interleaved `position` endpoints, with per-edge
  `color` and `alpha`.

## Lifecycle

`LayerFactory.createLayerFromDescriptor(descriptor, rendererOrGl, options)`
looks up `descriptor.primitive` in the registry and returns a `BaseLayer`
subclass. Layers can be added directly to `VisualScene`; they expose
`update(context)`, `render(context)`, `setDescriptor(descriptor)`,
`setVisible(visible)`, `setOrder(order)`, `getBounds()`, and `dispose()`.

Resources are created lazily when a WebGL context is available. Calling
`setDescriptor()` marks buffers dirty; the next render uploads the new channel
data. `dispose()` deletes shader programs and buffers owned by the layer.

## Limitations

- Rendering fallbacks may create visual colors from scalar/category channels
  when a descriptor lacks an explicit `color` channel. This is a renderer
  fallback, not a METRIC algorithm.
- WebGL line width support is implementation-dependent, so
  `RelationEdgeLayer` width is clamped to the browser's supported range.
- Point-sprite sizes are pixel-scaled by material or geometry options
  (`pointPixelScale`, `minPointSize`, `maxPointSize`), because WebGL point
  sprites do not represent scene-unit quads by themselves.
- Exported footprint parsing accepts common `vertices`, `points`, `polygon`,
  or direct array shapes. Complex polygons with holes are not triangulated.
