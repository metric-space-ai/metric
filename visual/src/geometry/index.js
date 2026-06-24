export const GEOMETRY_SCHEMA = "metric.visual.geometry.v1";

export const DRAW_MODES = Object.freeze({
  TRIANGLES: "triangles",
  LINES: "lines",
  LINE_STRIP: "line-strip",
  TRIANGLE_STRIP: "triangle-strip",
});

export const DEFAULT_INSTANCE_ATTRIBUTES = Object.freeze({
  instancePosition: Object.freeze({ semantic: "instance-position", itemSize: 3, divisor: 1 }),
  instanceScale: Object.freeze({ semantic: "instance-scale", itemSize: 3, divisor: 1, defaultValue: [1, 1, 1] }),
  instanceColor: Object.freeze({ semantic: "instance-rgba", itemSize: 4, divisor: 1, defaultValue: [1, 1, 1, 1] }),
  instanceRotation: Object.freeze({ semantic: "instance-quaternion", itemSize: 4, divisor: 1, defaultValue: [0, 0, 0, 1] }),
  instanceOpacity: Object.freeze({ semantic: "instance-opacity", itemSize: 1, divisor: 1, defaultValue: [1] }),
});

const EPSILON = 1e-12;

export function computeBounds(vertices, itemSize = 3) {
  const data = toFloat32Array(vertices);
  const stride = Math.max(1, Math.floor(itemSize));
  const min = new Float32Array(stride);
  const max = new Float32Array(stride);
  const center = new Float32Array(stride);
  const size = new Float32Array(stride);

  if (!data.length) {
    return { min, max, center, size, radius: 0 };
  }

  min.fill(Infinity);
  max.fill(-Infinity);

  for (let offset = 0; offset <= data.length - stride; offset += stride) {
    for (let axis = 0; axis < stride; axis += 1) {
      const value = finiteNumber(data[offset + axis], 0);
      if (value < min[axis]) min[axis] = value;
      if (value > max[axis]) max[axis] = value;
    }
  }

  let radius = 0;
  for (let axis = 0; axis < stride; axis += 1) {
    center[axis] = (min[axis] + max[axis]) * 0.5;
    size[axis] = max[axis] - min[axis];
  }

  for (let offset = 0; offset <= data.length - stride; offset += stride) {
    let distanceSquared = 0;
    for (let axis = 0; axis < stride; axis += 1) {
      const delta = data[offset + axis] - center[axis];
      distanceSquared += delta * delta;
    }
    radius = Math.max(radius, Math.sqrt(distanceSquared));
  }

  return { min, max, center, size, radius };
}

export const bounds = computeBounds;

export function computeVertexNormals(vertices, indices) {
  const positions = toFloat32Array(vertices);
  const vertexCount = Math.floor(positions.length / 3);
  const normals = new Float32Array(vertexCount * 3);
  const indexArray = indices == null ? null : toIndexArray(indices, vertexCount);
  const triangleCount = indexArray ? Math.floor(indexArray.length / 3) : Math.floor(vertexCount / 3);

  for (let triangle = 0; triangle < triangleCount; triangle += 1) {
    const i0 = indexArray ? indexArray[triangle * 3] : triangle * 3;
    const i1 = indexArray ? indexArray[triangle * 3 + 1] : triangle * 3 + 1;
    const i2 = indexArray ? indexArray[triangle * 3 + 2] : triangle * 3 + 2;
    if (!isValidVertexIndex(i0, vertexCount) || !isValidVertexIndex(i1, vertexCount) || !isValidVertexIndex(i2, vertexCount)) {
      continue;
    }

    const normal = triangleNormal(positions, i0, i1, i2, [0, 0, 0], false);
    accumulateNormal(normals, i0, normal);
    accumulateNormal(normals, i1, normal);
    accumulateNormal(normals, i2, normal);
  }

  normalizeNormals(normals);
  return normals;
}

export function computeFlatNormals(vertices, indices) {
  const positions = toFloat32Array(vertices);
  const vertexCount = Math.floor(positions.length / 3);
  const indexArray = indices == null ? null : toIndexArray(indices, vertexCount);
  const triangleCount = indexArray ? Math.floor(indexArray.length / 3) : Math.floor(vertexCount / 3);
  const flatVertices = new Float32Array(triangleCount * 9);
  const normals = new Float32Array(triangleCount * 9);

  for (let triangle = 0; triangle < triangleCount; triangle += 1) {
    const sourceIndices = indexArray
      ? [indexArray[triangle * 3], indexArray[triangle * 3 + 1], indexArray[triangle * 3 + 2]]
      : [triangle * 3, triangle * 3 + 1, triangle * 3 + 2];

    if (!sourceIndices.every((index) => isValidVertexIndex(index, vertexCount))) {
      continue;
    }

    const normal = triangleNormal(positions, sourceIndices[0], sourceIndices[1], sourceIndices[2], [0, 0, 1]);
    for (let corner = 0; corner < 3; corner += 1) {
      const sourceOffset = sourceIndices[corner] * 3;
      const targetOffset = triangle * 9 + corner * 3;
      flatVertices[targetOffset] = positions[sourceOffset];
      flatVertices[targetOffset + 1] = positions[sourceOffset + 1];
      flatVertices[targetOffset + 2] = positions[sourceOffset + 2];
      normals[targetOffset] = normal[0];
      normals[targetOffset + 1] = normal[1];
      normals[targetOffset + 2] = normal[2];
    }
  }

  return {
    vertices: flatVertices,
    normals,
    indices: null,
    vertexCount: Math.floor(flatVertices.length / 3),
    triangleCount,
  };
}

export function createBoxGeometry(options = {}) {
  const size = vectorOption(options.size, 3, 1);
  const width = positiveNumber(options.width, size[0]);
  const depth = positiveNumber(options.depth, size[1]);
  const height = positiveNumber(options.height, size[2]);
  const anchor = options.anchor || "center";
  const x = width * 0.5;
  const y = depth * 0.5;
  const bottom = anchor === "bottom" ? 0 : -height * 0.5;
  const top = anchor === "bottom" ? height : height * 0.5;

  const faces = [
    { normal: [0, 0, 1], vertices: [[-x, -y, top], [x, -y, top], [x, y, top], [-x, y, top]] },
    { normal: [0, 0, -1], vertices: [[-x, y, bottom], [x, y, bottom], [x, -y, bottom], [-x, -y, bottom]] },
    { normal: [0, 1, 0], vertices: [[-x, y, bottom], [-x, y, top], [x, y, top], [x, y, bottom]] },
    { normal: [0, -1, 0], vertices: [[-x, -y, top], [-x, -y, bottom], [x, -y, bottom], [x, -y, top]] },
    { normal: [1, 0, 0], vertices: [[x, -y, bottom], [x, y, bottom], [x, y, top], [x, -y, top]] },
    { normal: [-1, 0, 0], vertices: [[-x, y, bottom], [-x, -y, bottom], [-x, -y, top], [-x, y, top]] },
  ];

  const positions = new Float32Array(24 * 3);
  const normals = new Float32Array(24 * 3);
  const uvs = new Float32Array(24 * 2);
  const indices = new Uint16Array(36);
  const faceUvs = [0, 0, 1, 0, 1, 1, 0, 1];

  for (let faceIndex = 0; faceIndex < faces.length; faceIndex += 1) {
    const face = faces[faceIndex];
    const vertexBase = faceIndex * 4;
    const indexBase = faceIndex * 6;
    for (let corner = 0; corner < 4; corner += 1) {
      const vertexOffset = (vertexBase + corner) * 3;
      positions.set(face.vertices[corner], vertexOffset);
      normals.set(face.normal, vertexOffset);
      uvs[vertexBase * 2 + corner * 2] = faceUvs[corner * 2];
      uvs[vertexBase * 2 + corner * 2 + 1] = faceUvs[corner * 2 + 1];
    }
    indices.set([vertexBase, vertexBase + 1, vertexBase + 2, vertexBase, vertexBase + 2, vertexBase + 3], indexBase);
  }

  return createGeometryDescriptor({
    kind: options.kind || "box",
    topology: "triangle-mesh",
    mode: DRAW_MODES.TRIANGLES,
    attributes: {
      position: attribute(positions, 3, "position"),
      normal: attribute(normals, 3, "normal"),
      uv: attribute(uvs, 2, "uv"),
    },
    indices,
    metadata: { width, depth, height, anchor },
  });
}

export function createPlaneGeometry(options = {}) {
  const width = positiveNumber(options.width, vectorOption(options.size, 2, 1)[0]);
  const depth = positiveNumber(options.depth ?? options.height, vectorOption(options.size, 2, 1)[1]);
  const subdivisionsX = integerAtLeast(options.subdivisionsX ?? options.columns, 1);
  const subdivisionsY = integerAtLeast(options.subdivisionsY ?? options.rows, 1);
  const z = finiteNumber(options.z, 0);
  const vertexColumns = subdivisionsX + 1;
  const vertexRows = subdivisionsY + 1;
  const vertexCount = vertexColumns * vertexRows;
  const positions = new Float32Array(vertexCount * 3);
  const normals = new Float32Array(vertexCount * 3);
  const uvs = new Float32Array(vertexCount * 2);
  const triangleCount = subdivisionsX * subdivisionsY * 2;
  const indices = makeIndexArray(vertexCount, triangleCount * 3);

  let vertex = 0;
  for (let row = 0; row < vertexRows; row += 1) {
    const v = subdivisionsY === 0 ? 0 : row / subdivisionsY;
    const y = (v - 0.5) * depth;
    for (let column = 0; column < vertexColumns; column += 1) {
      const u = subdivisionsX === 0 ? 0 : column / subdivisionsX;
      const x = (u - 0.5) * width;
      positions.set([x, y, z], vertex * 3);
      normals.set([0, 0, 1], vertex * 3);
      uvs.set([u, v], vertex * 2);
      vertex += 1;
    }
  }

  let offset = 0;
  for (let row = 0; row < subdivisionsY; row += 1) {
    for (let column = 0; column < subdivisionsX; column += 1) {
      const a = row * vertexColumns + column;
      const b = (row + 1) * vertexColumns + column;
      const c = row * vertexColumns + column + 1;
      const d = (row + 1) * vertexColumns + column + 1;
      indices.set([a, b, c, c, b, d], offset);
      offset += 6;
    }
  }

  return createGeometryDescriptor({
    kind: options.kind || "plane",
    topology: "triangle-grid",
    mode: DRAW_MODES.TRIANGLES,
    attributes: {
      position: attribute(positions, 3, "position"),
      normal: attribute(normals, 3, "normal"),
      uv: attribute(uvs, 2, "uv"),
    },
    indices,
    metadata: { width, depth, z, subdivisionsX, subdivisionsY },
  });
}

export function createGridGeometry(options = {}) {
  const width = positiveNumber(options.width, vectorOption(options.size, 2, 10)[0]);
  const depth = positiveNumber(options.depth ?? options.height, vectorOption(options.size, 2, 10)[1]);
  const divisionsX = integerAtLeast(options.divisionsX ?? options.columns, 1);
  const divisionsY = integerAtLeast(options.divisionsY ?? options.rows, 1);
  const z = finiteNumber(options.z, 0);
  const lineCount = divisionsX + 1 + divisionsY + 1;
  const positions = new Float32Array(lineCount * 2 * 3);
  const gridUv = new Float32Array(lineCount * 2 * 2);
  const xMin = -width * 0.5;
  const xMax = width * 0.5;
  const yMin = -depth * 0.5;
  const yMax = depth * 0.5;
  let vertex = 0;

  for (let column = 0; column <= divisionsX; column += 1) {
    const u = column / divisionsX;
    const x = xMin + width * u;
    vertex = writeLineVertex(positions, gridUv, vertex, [x, yMin, z], [u, 0]);
    vertex = writeLineVertex(positions, gridUv, vertex, [x, yMax, z], [u, 1]);
  }

  for (let row = 0; row <= divisionsY; row += 1) {
    const v = row / divisionsY;
    const y = yMin + depth * v;
    vertex = writeLineVertex(positions, gridUv, vertex, [xMin, y, z], [0, v]);
    vertex = writeLineVertex(positions, gridUv, vertex, [xMax, y, z], [1, v]);
  }

  return createGeometryDescriptor({
    kind: options.kind || "grid",
    topology: "line-grid",
    mode: DRAW_MODES.LINES,
    attributes: {
      position: attribute(positions, 3, "position"),
      gridUv: attribute(gridUv, 2, "grid-uv"),
    },
    indices: null,
    metadata: { width, depth, z, divisionsX, divisionsY, lineCount },
  });
}

export function createGroundPlaneGeometry(options = {}) {
  const plane = createPlaneGeometry({
    ...options,
    kind: options.kind || "ground-plane",
  });
  const positions = plane.attributes.position.array;
  const vertexCount = plane.vertexCount;
  const projectionUv = new Float32Array(plane.attributes.uv.array);
  const shadowPosition = new Float32Array(vertexCount * 3);
  const groundCoordinate = new Float32Array(vertexCount * 2);

  for (let vertex = 0; vertex < vertexCount; vertex += 1) {
    const positionOffset = vertex * 3;
    const groundOffset = vertex * 2;
    shadowPosition[positionOffset] = positions[positionOffset];
    shadowPosition[positionOffset + 1] = positions[positionOffset + 1];
    shadowPosition[positionOffset + 2] = positions[positionOffset + 2];
    groundCoordinate[groundOffset] = positions[positionOffset];
    groundCoordinate[groundOffset + 1] = positions[positionOffset + 1];
  }

  return withAttributes(plane, {
    projectionUv: attribute(projectionUv, 2, "projection-uv"),
    shadowPosition: attribute(shadowPosition, 3, "shadow-receiver-position"),
    groundCoordinate: attribute(groundCoordinate, 2, "ground-coordinate"),
  }, {
    metadata: {
      ...plane.metadata,
      receivesProjection: true,
      receivesShadow: true,
    },
  });
}

export function createGroundGridGeometry(options = {}) {
  const grid = createGridGeometry({
    ...options,
    kind: options.kind || "ground-grid",
  });
  const positions = grid.attributes.position.array;
  const vertexCount = grid.vertexCount;
  const shadowPosition = new Float32Array(positions);
  const groundCoordinate = new Float32Array(vertexCount * 2);

  for (let vertex = 0; vertex < vertexCount; vertex += 1) {
    groundCoordinate[vertex * 2] = positions[vertex * 3];
    groundCoordinate[vertex * 2 + 1] = positions[vertex * 3 + 1];
  }

  return withAttributes(grid, {
    projectionUv: attribute(new Float32Array(grid.attributes.gridUv.array), 2, "projection-uv"),
    shadowPosition: attribute(shadowPosition, 3, "shadow-receiver-position"),
    groundCoordinate: attribute(groundCoordinate, 2, "ground-coordinate"),
  }, {
    metadata: {
      ...grid.metadata,
      receivesProjection: true,
      receivesShadow: false,
    },
  });
}

export function createGroundPlaneGridGeometry(options = {}) {
  return {
    plane: createGroundPlaneGeometry(options.plane || options),
    grid: createGroundGridGeometry(options.grid || options),
  };
}

export function createLowPolySphereGeometry(options = {}) {
  const radius = positiveNumber(options.radius ?? options.size, 0.5);
  const flat = options.flat !== false;
  const t = (1 + Math.sqrt(5)) * 0.5;
  const rawVertices = [
    [-1, t, 0], [1, t, 0], [-1, -t, 0], [1, -t, 0],
    [0, -1, t], [0, 1, t], [0, -1, -t], [0, 1, -t],
    [t, 0, -1], [t, 0, 1], [-t, 0, -1], [-t, 0, 1],
  ];
  const basePositions = new Float32Array(rawVertices.length * 3);
  for (let index = 0; index < rawVertices.length; index += 1) {
    const normalized = normalize3(rawVertices[index], radius);
    basePositions.set(normalized, index * 3);
  }
  const baseIndices = new Uint16Array([
    0, 11, 5, 0, 5, 1, 0, 1, 7, 0, 7, 10, 0, 10, 11,
    1, 5, 9, 5, 11, 4, 11, 10, 2, 10, 7, 6, 7, 1, 8,
    3, 9, 4, 3, 4, 2, 3, 2, 6, 3, 6, 8, 3, 8, 9,
    4, 9, 5, 2, 4, 11, 6, 2, 10, 8, 6, 7, 9, 8, 1,
  ]);

  if (flat) {
    const flatData = computeFlatNormals(basePositions, baseIndices);
    return createGeometryDescriptor({
      kind: options.kind || "low-poly-sphere",
      topology: "triangle-mesh",
      mode: DRAW_MODES.TRIANGLES,
      attributes: {
        position: attribute(flatData.vertices, 3, "position"),
        normal: attribute(flatData.normals, 3, "normal"),
      },
      indices: null,
      metadata: { radius, flat: true, source: "icosahedron" },
    });
  }

  return createGeometryDescriptor({
    kind: options.kind || "low-poly-sphere",
    topology: "triangle-mesh",
    mode: DRAW_MODES.TRIANGLES,
    attributes: {
      position: attribute(basePositions, 3, "position"),
      normal: attribute(computeVertexNormals(basePositions, baseIndices), 3, "normal"),
    },
    indices: baseIndices,
    metadata: { radius, flat: false, source: "icosahedron" },
  });
}

export const createSphereGlyphGeometry = createLowPolySphereGeometry;

export function createCylinderGlyphGeometry(options = {}) {
  const segments = integerAtLeast(options.segments, 8);
  const radius = positiveNumber(options.radius, 0.5);
  const height = positiveNumber(options.height, 1);
  const anchor = options.anchor || "center";
  const bottom = anchor === "bottom" ? 0 : -height * 0.5;
  const top = anchor === "bottom" ? height : height * 0.5;
  const sideVertexCount = segments * 2;
  const capVertexCount = (segments + 1) * 2;
  const vertexCount = sideVertexCount + capVertexCount;
  const positions = new Float32Array(vertexCount * 3);
  const normals = new Float32Array(vertexCount * 3);
  const uvs = new Float32Array(vertexCount * 2);
  const indices = makeIndexArray(vertexCount, segments * 12);

  for (let segment = 0; segment < segments; segment += 1) {
    const angle = (segment / segments) * Math.PI * 2;
    const x = Math.cos(angle);
    const y = Math.sin(angle);
    const bottomIndex = segment * 2;
    const topIndex = bottomIndex + 1;
    positions.set([x * radius, y * radius, bottom], bottomIndex * 3);
    positions.set([x * radius, y * radius, top], topIndex * 3);
    normals.set([x, y, 0], bottomIndex * 3);
    normals.set([x, y, 0], topIndex * 3);
    uvs.set([segment / segments, 0], bottomIndex * 2);
    uvs.set([segment / segments, 1], topIndex * 2);
  }

  const topCenter = sideVertexCount;
  const bottomCenter = topCenter + segments + 1;
  positions.set([0, 0, top], topCenter * 3);
  normals.set([0, 0, 1], topCenter * 3);
  uvs.set([0.5, 0.5], topCenter * 2);
  positions.set([0, 0, bottom], bottomCenter * 3);
  normals.set([0, 0, -1], bottomCenter * 3);
  uvs.set([0.5, 0.5], bottomCenter * 2);

  for (let segment = 0; segment < segments; segment += 1) {
    const angle = (segment / segments) * Math.PI * 2;
    const x = Math.cos(angle);
    const y = Math.sin(angle);
    const topRing = topCenter + 1 + segment;
    const bottomRing = bottomCenter + 1 + segment;
    positions.set([x * radius, y * radius, top], topRing * 3);
    normals.set([0, 0, 1], topRing * 3);
    uvs.set([x * 0.5 + 0.5, y * 0.5 + 0.5], topRing * 2);
    positions.set([x * radius, y * radius, bottom], bottomRing * 3);
    normals.set([0, 0, -1], bottomRing * 3);
    uvs.set([x * 0.5 + 0.5, y * 0.5 + 0.5], bottomRing * 2);
  }

  let offset = 0;
  for (let segment = 0; segment < segments; segment += 1) {
    const next = (segment + 1) % segments;
    const bottom0 = segment * 2;
    const top0 = bottom0 + 1;
    const bottom1 = next * 2;
    const top1 = bottom1 + 1;
    indices.set([bottom0, bottom1, top0, bottom1, top1, top0], offset);
    offset += 6;

    const top0Cap = topCenter + 1 + segment;
    const top1Cap = topCenter + 1 + next;
    indices.set([topCenter, top0Cap, top1Cap], offset);
    offset += 3;

    const bottom0Cap = bottomCenter + 1 + segment;
    const bottom1Cap = bottomCenter + 1 + next;
    indices.set([bottomCenter, bottom1Cap, bottom0Cap], offset);
    offset += 3;
  }

  return createGeometryDescriptor({
    kind: options.kind || "cylinder-glyph",
    topology: "triangle-mesh",
    mode: DRAW_MODES.TRIANGLES,
    attributes: {
      position: attribute(positions, 3, "position"),
      normal: attribute(normals, 3, "normal"),
      uv: attribute(uvs, 2, "uv"),
    },
    indices,
    metadata: { radius, height, segments, anchor },
  });
}

export function createLineStripGeometry(points = [], options = {}) {
  const positions = toPointArray(points);
  const closed = Boolean(options.closed);
  const sourceCount = Math.floor(positions.length / 3);
  const vertexCount = closed && sourceCount > 1 ? sourceCount + 1 : sourceCount;
  const output = new Float32Array(vertexCount * 3);
  const distances = new Float32Array(vertexCount);
  const pointIndex = new Float32Array(vertexCount);

  output.set(positions);
  if (closed && sourceCount > 1) {
    output.set(positions.slice(0, 3), sourceCount * 3);
  }

  let cumulative = 0;
  for (let vertex = 0; vertex < vertexCount; vertex += 1) {
    if (vertex > 0) {
      cumulative += distance3(output, vertex - 1, output, vertex);
    }
    distances[vertex] = cumulative;
    pointIndex[vertex] = vertex < sourceCount ? vertex : 0;
  }

  return createGeometryDescriptor({
    kind: options.kind || "line-strip",
    topology: closed ? "closed-line-strip" : "line-strip",
    mode: DRAW_MODES.LINE_STRIP,
    attributes: {
      position: attribute(output, 3, "position"),
      cumulativeDistance: attribute(distances, 1, "cumulative-distance"),
      pointIndex: attribute(pointIndex, 1, "point-index"),
    },
    indices: null,
    metadata: { closed, pointCount: sourceCount, length: cumulative },
  });
}

export function createRelationEdgeBatchGeometry(edges = [], options = {}) {
  const sourceEdges = Array.isArray(edges) ? edges : [];
  const positions = new Float32Array(sourceEdges.length * 2 * 3);
  const relationIndex = new Float32Array(sourceEdges.length * 2);
  const endpoint = new Float32Array(sourceEdges.length * 2);
  const weight = new Float32Array(sourceEdges.length * 2);
  const edgeIds = new Array(sourceEdges.length);

  for (let edgeIndex = 0; edgeIndex < sourceEdges.length; edgeIndex += 1) {
    const edge = sourceEdges[edgeIndex];
    const parsed = parseEdge(edge, options.positions);
    positions.set(parsed.source, edgeIndex * 6);
    positions.set(parsed.target, edgeIndex * 6 + 3);
    relationIndex[edgeIndex * 2] = edgeIndex;
    relationIndex[edgeIndex * 2 + 1] = edgeIndex;
    endpoint[edgeIndex * 2] = 0;
    endpoint[edgeIndex * 2 + 1] = 1;
    weight[edgeIndex * 2] = parsed.weight;
    weight[edgeIndex * 2 + 1] = parsed.weight;
    edgeIds[edgeIndex] = parsed.id;
  }

  return createGeometryDescriptor({
    kind: options.kind || "relation-edge-batch",
    topology: "line-list",
    mode: DRAW_MODES.LINES,
    attributes: {
      position: attribute(positions, 3, "position"),
      relationIndex: attribute(relationIndex, 1, "relation-index"),
      endpoint: attribute(endpoint, 1, "edge-endpoint"),
      weight: attribute(weight, 1, "edge-weight"),
    },
    indices: null,
    metadata: { edgeCount: sourceEdges.length, edgeIds },
  });
}

export function createSurfaceMeshGeometry(verticesOrOptions = [], indicesOrOptions = null, maybeOptions = {}) {
  const source = isPlainObject(verticesOrOptions)
    ? verticesOrOptions
    : { vertices: verticesOrOptions, indices: indicesOrOptions, ...maybeOptions };
  const flatNormals = Boolean(source.flatNormals);
  const sourceVertices = source.vertices || source.positions || [];
  const positions = toPointArray(sourceVertices);
  const vertexCount = Math.floor(positions.length / 3);
  const indices = source.indices == null ? null : toIndexArray(source.indices, vertexCount);

  let renderPositions = positions;
  let renderNormals = source.normals ? toPointArray(source.normals) : null;
  let renderIndices = indices;

  if (!renderNormals) {
    if (flatNormals) {
      const flat = computeFlatNormals(renderPositions, renderIndices);
      renderPositions = flat.vertices;
      renderNormals = flat.normals;
      renderIndices = null;
    } else {
      renderNormals = computeVertexNormals(renderPositions, renderIndices);
    }
  }

  const attributes = {
    position: attribute(renderPositions, 3, "position"),
    normal: attribute(renderNormals, 3, "normal"),
  };

  if (source.uvs || source.uv) {
    attributes.uv = attribute(toFloat32Array(source.uvs || source.uv), 2, "uv");
  }
  if (source.colors || source.color) {
    attributes.color = attribute(toFloat32Array(source.colors || source.color), 4, "rgba");
  }
  if (source.scalar || source.scalars) {
    attributes.scalar = attribute(toFloat32Array(source.scalar || source.scalars), 1, "scalar");
  }

  return createGeometryDescriptor({
    kind: source.kind || "surface-mesh",
    topology: source.topology || "triangle-mesh",
    mode: source.mode || DRAW_MODES.TRIANGLES,
    attributes,
    indices: renderIndices,
    metadata: {
      normalPolicy: source.normals ? "provided" : flatNormals ? "computed-flat" : "computed-vertex",
      sourceVertexCount: vertexCount,
      sourceIndexCount: indices ? indices.length : 0,
      ...(source.metadata || {}),
    },
  });
}

export function createInstancedBoxGeometry(options = {}) {
  return withInstancing(createBoxGeometry({ ...options, kind: options.kind || "instanced-box" }), {
    strategy: "per-instance-transform",
    attributes: mergeInstanceAttributes(DEFAULT_INSTANCE_ATTRIBUTES, options.instanceAttributes),
    maxInstances: nonNegativeInteger(options.maxInstances, 0),
  });
}

export function createPointBillboardGeometry(options = {}) {
  const size = positiveNumber(options.size, 1);
  const half = size * 0.5;
  const positions = new Float32Array([
    -half, -half, 0,
    half, -half, 0,
    half, half, 0,
    -half, half, 0,
  ]);
  const corners = new Float32Array([
    -1, -1,
    1, -1,
    1, 1,
    -1, 1,
  ]);
  const uvs = new Float32Array([
    0, 0,
    1, 0,
    1, 1,
    0, 1,
  ]);
  const normals = new Float32Array([
    0, 0, 1,
    0, 0, 1,
    0, 0, 1,
    0, 0, 1,
  ]);
  const indices = new Uint16Array([0, 1, 2, 0, 2, 3]);
  const geometry = createGeometryDescriptor({
    kind: options.kind || "point-billboard",
    topology: "billboard-quad",
    mode: DRAW_MODES.TRIANGLES,
    attributes: {
      position: attribute(positions, 3, "billboard-local-position"),
      normal: attribute(normals, 3, "normal"),
      corner: attribute(corners, 2, "billboard-corner"),
      uv: attribute(uvs, 2, "uv"),
    },
    indices,
    metadata: {
      size,
      facing: options.facing || "camera",
      sizeSpace: options.sizeSpace || "world",
    },
  });

  return withInstancing(geometry, {
    strategy: "camera-facing-billboard",
    attributes: mergeInstanceAttributes({
      instancePosition: DEFAULT_INSTANCE_ATTRIBUTES.instancePosition,
      instanceSize: { semantic: "instance-billboard-size", itemSize: 2, divisor: 1, defaultValue: [size, size] },
      instanceColor: DEFAULT_INSTANCE_ATTRIBUTES.instanceColor,
      instanceRotation: { semantic: "instance-screen-rotation", itemSize: 1, divisor: 1, defaultValue: [0] },
      instanceOpacity: DEFAULT_INSTANCE_ATTRIBUTES.instanceOpacity,
    }, options.instanceAttributes),
    maxInstances: nonNegativeInteger(options.maxInstances, 0),
  });
}

export function createGlyphGeometry(kind = "record", options = {}) {
  const normalized = normalizeKind(kind);
  if (normalized === "box" || normalized === "cube" || normalized === "selected-record") {
    return createBoxGeometry({ ...options, kind: options.kind || normalized });
  }
  if (normalized === "sphere" || normalized === "low-poly-sphere" || normalized === "record" || normalized === "outlier") {
    return createLowPolySphereGeometry({ ...options, kind: options.kind || normalized });
  }
  if (normalized === "cylinder" || normalized === "cylinder-glyph" || normalized === "cluster-representative" || normalized === "landmark") {
    return createCylinderGlyphGeometry({ ...options, kind: options.kind || normalized });
  }
  if (normalized === "billboard" || normalized === "point-billboard" || normalized === "trajectory-point") {
    return createPointBillboardGeometry({ ...options, kind: options.kind || normalized });
  }
  if (normalized === "plane") {
    return createPlaneGeometry({ ...options, kind: options.kind || normalized });
  }
  if (normalized === "instanced-box") {
    return createInstancedBoxGeometry({ ...options, kind: options.kind || normalized });
  }
  throw new Error(`Unknown glyph geometry kind: ${kind}`);
}

export function createGeometryDescriptor({
  kind,
  topology,
  mode,
  attributes,
  indices = null,
  metadata = {},
  instancing = null,
}) {
  const position = attributes?.position;
  if (!position || !(position.array instanceof Float32Array) || position.itemSize !== 3) {
    throw new Error("Geometry descriptors require a Float32Array position attribute with itemSize 3.");
  }

  const vertexCount = position.count;
  const indexArray = indices == null ? null : toIndexArray(indices, vertexCount);
  const drawCount = indexArray ? indexArray.length : vertexCount;

  return {
    schema: GEOMETRY_SCHEMA,
    kind,
    topology,
    mode,
    attributes,
    indices: indexArray,
    vertexCount,
    indexCount: indexArray ? indexArray.length : 0,
    drawCount,
    primitiveCount: primitiveCount(mode, drawCount),
    bounds: computeBounds(position.array, 3),
    instanced: Boolean(instancing),
    instancing,
    metadata,
  };
}

function attribute(array, itemSize, semantic, options = {}) {
  return {
    semantic,
    itemSize,
    count: Math.floor(array.length / itemSize),
    array,
    ...options,
  };
}

function withAttributes(geometry, attributes, options = {}) {
  const merged = {
    ...geometry,
    attributes: {
      ...geometry.attributes,
      ...attributes,
    },
    metadata: options.metadata || geometry.metadata,
  };
  return merged;
}

function withInstancing(geometry, instancing) {
  return {
    ...geometry,
    instanced: true,
    instancing,
  };
}

function mergeInstanceAttributes(base, overrides) {
  return {
    ...base,
    ...(overrides || {}),
  };
}

function primitiveCount(mode, drawCount) {
  if (mode === DRAW_MODES.TRIANGLES) return Math.floor(drawCount / 3);
  if (mode === DRAW_MODES.LINES) return Math.floor(drawCount / 2);
  if (mode === DRAW_MODES.LINE_STRIP) return Math.max(0, drawCount - 1);
  if (mode === DRAW_MODES.TRIANGLE_STRIP) return Math.max(0, drawCount - 2);
  return drawCount;
}

function writeLineVertex(positions, gridUv, vertex, position, uv) {
  positions.set(position, vertex * 3);
  gridUv.set(uv, vertex * 2);
  return vertex + 1;
}

function parseEdge(edge, positionSource) {
  const positions = positionSource == null ? null : toPointArray(positionSource);
  let source = null;
  let target = null;
  let weight = 1;
  let id = undefined;

  if (Array.isArray(edge)) {
    source = edge[0];
    target = edge[1];
    weight = finiteNumber(edge[2], 1);
    id = edge[3];
  } else if (isPlainObject(edge)) {
    source = edge.source ?? edge.from ?? edge.a ?? edge.start ?? edge.sourceIndex ?? edge.fromIndex;
    target = edge.target ?? edge.to ?? edge.b ?? edge.end ?? edge.targetIndex ?? edge.toIndex;
    weight = finiteNumber(edge.weight ?? edge.value ?? edge.strength, 1);
    id = edge.id ?? edge.edgeId ?? edge.relationId;
  }

  return {
    source: readPosition(source, positions),
    target: readPosition(target, positions),
    weight,
    id,
  };
}

function readPosition(value, positions) {
  if (typeof value === "number" && positions) {
    const index = Math.floor(value);
    if (index >= 0 && index * 3 + 2 < positions.length) {
      return [
        positions[index * 3],
        positions[index * 3 + 1],
        positions[index * 3 + 2],
      ];
    }
  }
  if (ArrayBuffer.isView(value) || Array.isArray(value)) {
    return [
      finiteNumber(value[0], 0),
      finiteNumber(value[1], 0),
      finiteNumber(value[2], 0),
    ];
  }
  return [0, 0, 0];
}

function toPointArray(values) {
  if (values instanceof Float32Array && values.length % 3 === 0) {
    return new Float32Array(values);
  }
  if (ArrayBuffer.isView(values) && values.length % 3 === 0) {
    return new Float32Array(values);
  }
  if (!Array.isArray(values)) {
    return new Float32Array(0);
  }
  if (values.length === 0) {
    return new Float32Array(0);
  }
  if (typeof values[0] === "number") {
    return new Float32Array(values);
  }
  const output = new Float32Array(values.length * 3);
  for (let index = 0; index < values.length; index += 1) {
    const value = values[index] || [];
    output[index * 3] = finiteNumber(value[0], 0);
    output[index * 3 + 1] = finiteNumber(value[1], 0);
    output[index * 3 + 2] = finiteNumber(value[2], 0);
  }
  return output;
}

function toFloat32Array(values) {
  if (values instanceof Float32Array) return new Float32Array(values);
  if (ArrayBuffer.isView(values) || Array.isArray(values)) return new Float32Array(values);
  return new Float32Array(0);
}

function toIndexArray(indices, vertexCount) {
  if (indices == null) return null;
  const source = ArrayBuffer.isView(indices) || Array.isArray(indices) ? indices : [];
  const maxIndex = maxValue(source);
  const safeVertexCount = Math.max(vertexCount, maxIndex + 1);
  const output = makeIndexArray(safeVertexCount, source.length);
  for (let index = 0; index < source.length; index += 1) {
    output[index] = Math.max(0, Math.floor(finiteNumber(source[index], 0)));
  }
  return output;
}

function makeIndexArray(vertexCount, length) {
  return vertexCount > 65535 ? new Uint32Array(length) : new Uint16Array(length);
}

function maxValue(values) {
  let max = 0;
  for (let index = 0; index < values.length; index += 1) {
    const value = Math.floor(finiteNumber(values[index], 0));
    if (value > max) max = value;
  }
  return max;
}

function triangleNormal(positions, i0, i1, i2, fallback = [0, 0, 0], normalize = true) {
  const a = i0 * 3;
  const b = i1 * 3;
  const c = i2 * 3;
  const abx = positions[b] - positions[a];
  const aby = positions[b + 1] - positions[a + 1];
  const abz = positions[b + 2] - positions[a + 2];
  const acx = positions[c] - positions[a];
  const acy = positions[c + 1] - positions[a + 1];
  const acz = positions[c + 2] - positions[a + 2];
  const x = aby * acz - abz * acy;
  const y = abz * acx - abx * acz;
  const z = abx * acy - aby * acx;
  if (!normalize) return [x, y, z];
  return normalize3([x, y, z], 1, fallback);
}

function accumulateNormal(normals, vertexIndex, normal) {
  const offset = vertexIndex * 3;
  normals[offset] += normal[0];
  normals[offset + 1] += normal[1];
  normals[offset + 2] += normal[2];
}

function normalizeNormals(normals) {
  for (let offset = 0; offset < normals.length; offset += 3) {
    const normalized = normalize3([normals[offset], normals[offset + 1], normals[offset + 2]], 1, [0, 0, 1]);
    normals.set(normalized, offset);
  }
}

function normalize3(values, scale = 1, fallback = [0, 0, 0]) {
  const x = finiteNumber(values[0], 0);
  const y = finiteNumber(values[1], 0);
  const z = finiteNumber(values[2], 0);
  const length = Math.hypot(x, y, z);
  if (length <= EPSILON) return [...fallback];
  return [x / length * scale, y / length * scale, z / length * scale];
}

function distance3(a, ai, b, bi) {
  const ao = ai * 3;
  const bo = bi * 3;
  return Math.hypot(a[ao] - b[bo], a[ao + 1] - b[bo + 1], a[ao + 2] - b[bo + 2]);
}

function vectorOption(value, itemSize, fallback) {
  const out = new Array(itemSize);
  for (let index = 0; index < itemSize; index += 1) {
    out[index] = positiveNumber(Array.isArray(value) || ArrayBuffer.isView(value) ? value[index] : value, fallback);
  }
  return out;
}

function normalizeKind(kind) {
  return String(kind || "")
    .replace(/([a-z])([A-Z])/g, "$1-$2")
    .replace(/_/g, "-")
    .toLowerCase();
}

function finiteNumber(value, fallback) {
  const next = Number(value);
  return Number.isFinite(next) ? next : fallback;
}

function positiveNumber(value, fallback) {
  const next = finiteNumber(value, fallback);
  return next > 0 ? next : fallback;
}

function integerAtLeast(value, minimum) {
  const next = Math.floor(finiteNumber(value, minimum));
  return Math.max(minimum, next);
}

function nonNegativeInteger(value, fallback) {
  return Math.max(0, Math.floor(finiteNumber(value, fallback)));
}

function isValidVertexIndex(index, vertexCount) {
  return Number.isInteger(index) && index >= 0 && index < vertexCount;
}

function isPlainObject(value) {
  return value !== null && typeof value === "object" && !Array.isArray(value) && !ArrayBuffer.isView(value);
}
