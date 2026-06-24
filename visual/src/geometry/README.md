# METRIC Visual Geometry

`visual/src/geometry` contains dependency-free geometry builders for the native WebGL canvas renderer. The module does not touch DOM, SVG, WebGL contexts, METRIC data algorithms, or external packages. It only returns typed render geometry descriptors that downstream renderer code can upload into buffers.

## Descriptor Shape

Every builder returns a plain object with this shape:

```js
{
  schema: "metric.visual.geometry.v1",
  kind: "box",
  topology: "triangle-mesh",
  mode: "triangles",
  attributes: {
    position: { semantic: "position", itemSize: 3, count: 24, array: Float32Array },
    normal: { semantic: "normal", itemSize: 3, count: 24, array: Float32Array }
  },
  indices: Uint16Array | Uint32Array | null,
  vertexCount: 24,
  indexCount: 36,
  drawCount: 36,
  primitiveCount: 12,
  bounds: { min: Float32Array, max: Float32Array, center: Float32Array, size: Float32Array, radius: Number },
  instanced: false,
  instancing: null,
  metadata: {}
}
```

Draw modes are string tokens (`"triangles"`, `"lines"`, `"line-strip"`) so the module stays WebGL-context-free. A renderer can map these to `gl.TRIANGLES`, `gl.LINES`, and `gl.LINE_STRIP`.

## Builders

- `createBoxGeometry(options)` creates an indexed, z-up box. `width` maps to X, `depth` maps to Y, and `height` maps to Z. `anchor: "bottom"` places the base on z=0.
- `createPlaneGeometry(options)` creates an XY triangle grid at `z`.
- `createGridGeometry(options)` creates an XY line grid.
- `createGroundPlaneGeometry(options)` and `createGroundGridGeometry(options)` add `projectionUv`, `shadowPosition`, and `groundCoordinate` attributes for projection/shadow receiver passes.
- `createGroundPlaneGridGeometry(options)` returns `{ plane, grid }`.
- `createLowPolySphereGeometry(options)` / `createSphereGlyphGeometry(options)` create an icosahedron-based sphere-ish glyph.
- `createCylinderGlyphGeometry(options)` creates a segment-based cylinder-ish glyph.
- `createLineStripGeometry(points, options)` creates a line-strip descriptor with cumulative distance and point index attributes.
- `createRelationEdgeBatchGeometry(edges, options)` creates line-list batches for relation edges. Edges may use coordinate endpoints or numeric endpoints with `options.positions`.
- `createSurfaceMeshGeometry(vertices, indices, options)` creates a render surface from vertices/indices and computes normals when they are not supplied.

## Normals And Bounds

- `computeVertexNormals(vertices, indices)` returns a `Float32Array` of smooth accumulated vertex normals.
- `computeFlatNormals(vertices, indices)` returns expanded non-indexed triangle vertices and matching flat normals: `{ vertices, normals, indices: null, vertexCount, triangleCount }`.
- `computeBounds(vertices, itemSize)` and the alias `bounds` return typed min/max/center/size arrays plus a bounding radius.

These helpers are for render geometry only. They do not infer METRIC semantics or run data-space algorithms.

## Instancing Descriptors

- `createInstancedBoxGeometry(options)` returns a box descriptor with instance layout metadata.
- `createPointBillboardGeometry(options)` returns a camera-facing quad descriptor with instance layout metadata.
- `createGlyphGeometry(kind, options)` chooses one of the concrete glyph builders by kind (`box`, `sphere`, `cylinder`, `billboard`, `record`, `selectedRecord`, `clusterRepresentative`, `outlier`, `landmark`, `trajectoryPoint`).

Instance attributes are descriptors, not buffers. The default layout names are `instancePosition`, `instanceScale`, `instanceColor`, `instanceRotation`, and `instanceOpacity`.
