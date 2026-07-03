import {
  computeCumulativeLengths,
  normalizePathEvidence,
  projectPathEvidenceToGround,
} from "./path-data.js";

/**
 * Build GL line-segment buffers from normalized path evidence.
 *
 * @param {object|Array|ArrayBufferView} evidence
 * @param {object} [options]
 * @returns {object}
 */
export function buildPolylinePathBuffers(evidence, options = {}) {
  const normalized = evidence?.kind?.includes("curve_path_evidence")
    ? evidence
    : normalizePathEvidence(evidence, options);
  const segmentCount = countSegments(normalized.paths);
  const positions = new Float32Array(segmentCount * 2 * 3);
  const colors = new Float32Array(segmentCount * 2 * 4);
  const widths = new Float32Array(segmentCount * 2);
  const distances = new Float32Array(segmentCount * 2);
  const pathIndices = new Float32Array(segmentCount * 2);
  let vertex = 0;

  normalized.paths.forEach((path, pathIndex) => {
    const lengths = computeCumulativeLengths(path.points, path.count).lengths;
    for (let point = 0; point < path.count - 1; point += 1) {
      copyPathVertex(path, point, positions, colors, widths, vertex);
      distances[vertex] = lengths[point];
      pathIndices[vertex] = pathIndex;
      vertex += 1;
      copyPathVertex(path, point + 1, positions, colors, widths, vertex);
      distances[vertex] = lengths[point + 1];
      pathIndices[vertex] = pathIndex;
      vertex += 1;
    }
  });

  return {
    kind: "metric.visual.polyline_path_buffers.v1",
    evidence: normalized,
    segmentCount,
    vertexCount: vertex,
    channels: {
      position: { array: positions, itemSize: 3, count: vertex },
      color: { array: colors, itemSize: 4, count: vertex },
      width: { array: widths, itemSize: 1, count: vertex },
      distance: { array: distances, itemSize: 1, count: vertex },
      pathIndex: { array: pathIndices, itemSize: 1, count: vertex },
    },
    bounds: normalized.bounds,
  };
}

/**
 * Build camera-facing triangle buffers for thick path ribbons.
 *
 * The shader expands each segment in screen space from `start` to `end`, so
 * width is interpreted as CSS/WebGL pixels instead of world units.
 *
 * @param {object|Array|ArrayBufferView} evidence
 * @param {object} [options]
 * @returns {object}
 */
export function buildRibbonPathBuffers(evidence, options = {}) {
  const normalized = evidence?.kind?.includes("curve_path_evidence")
    ? evidence
    : normalizePathEvidence(evidence, options);
  const segmentCount = countSegments(normalized.paths);
  const vertexCount = segmentCount * 6;
  const starts = new Float32Array(vertexCount * 3);
  const ends = new Float32Array(vertexCount * 3);
  const sides = new Float32Array(vertexCount);
  const along = new Float32Array(vertexCount);
  const colors = new Float32Array(vertexCount * 4);
  const widths = new Float32Array(vertexCount);
  const distances = new Float32Array(vertexCount);
  const pathIndices = new Float32Array(vertexCount);
  let vertex = 0;

  normalized.paths.forEach((path, pathIndex) => {
    const lengths = computeCumulativeLengths(path.points, path.count).lengths;
    for (let point = 0; point < path.count - 1; point += 1) {
      const pattern = [
        [0, -1], [0, 1], [1, -1],
        [1, -1], [0, 1], [1, 1],
      ];
      for (let corner = 0; corner < pattern.length; corner += 1) {
        const endpoint = pattern[corner][0] === 0 ? point : point + 1;
        starts.set(path.points.subarray(point * 3, point * 3 + 3), vertex * 3);
        ends.set(path.points.subarray((point + 1) * 3, (point + 1) * 3 + 3), vertex * 3);
        sides[vertex] = pattern[corner][1];
        along[vertex] = pattern[corner][0];
        colors.set(path.colors.subarray(endpoint * 4, endpoint * 4 + 4), vertex * 4);
        widths[vertex] = path.widths[endpoint];
        distances[vertex] = lengths[endpoint];
        pathIndices[vertex] = pathIndex;
        vertex += 1;
      }
    }
  });

  return {
    kind: "metric.visual.ribbon_path_buffers.v1",
    evidence: normalized,
    segmentCount,
    vertexCount,
    channels: {
      start: { array: starts, itemSize: 3, count: vertex },
      end: { array: ends, itemSize: 3, count: vertex },
      side: { array: sides, itemSize: 1, count: vertex },
      along: { array: along, itemSize: 1, count: vertex },
      color: { array: colors, itemSize: 4, count: vertex },
      width: { array: widths, itemSize: 1, count: vertex },
      distance: { array: distances, itemSize: 1, count: vertex },
      pathIndex: { array: pathIndices, itemSize: 1, count: vertex },
    },
    bounds: normalized.bounds,
  };
}

/**
 * Build an indexed tube-like mesh with world-space radius.
 *
 * This is a native typed-array mesh generator inspired by conventional tube
 * geometry. It does not depend on an external renderer and does not evaluate any METRIC
 * algorithm. It only turns exported points into renderable rings.
 *
 * @param {object|Array|ArrayBufferView} evidence
 * @param {object} [options]
 * @param {number} [options.radius=0.03]
 * @param {number} [options.radialSegments=8]
 * @param {boolean} [options.useWidthsAsRadius=false]
 * @returns {object}
 */
export function buildTubePathGeometry(evidence, options = {}) {
  const normalized = evidence?.kind?.includes("curve_path_evidence")
    ? evidence
    : normalizePathEvidence(evidence, options);
  const radialSegments = Math.max(3, Math.floor(Number(options.radialSegments) || 8));
  const radiusFallback = Number.isFinite(Number(options.radius)) ? Number(options.radius) : 0.03;
  const totalRingCount = normalized.paths.reduce((sum, path) => sum + path.count, 0);
  const verticesPerRing = radialSegments + 1;
  const vertexCount = totalRingCount * verticesPerRing;
  const triangleIndexCount = normalized.paths.reduce((sum, path) => sum + Math.max(0, path.count - 1) * radialSegments * 6, 0);
  const positions = new Float32Array(vertexCount * 3);
  const normals = new Float32Array(vertexCount * 3);
  const colors = new Float32Array(vertexCount * 4);
  const distances = new Float32Array(vertexCount);
  const pathIndices = new Float32Array(vertexCount);
  const indices = vertexCount > 65535 ? new Uint32Array(triangleIndexCount) : new Uint16Array(triangleIndexCount);
  const ranges = [];
  let vertexOffset = 0;
  let indexOffset = 0;

  normalized.paths.forEach((path, pathIndex) => {
    const frames = computePathFrames(path.points, path.count);
    const lengths = computeCumulativeLengths(path.points, path.count).lengths;
    const startVertex = vertexOffset;
    for (let point = 0; point < path.count; point += 1) {
      const pointOffset = point * 3;
      const radius = options.useWidthsAsRadius === true ? finitePositive(path.widths[point], radiusFallback) : radiusFallback;
      for (let side = 0; side <= radialSegments; side += 1) {
        const angle = (side / radialSegments) * Math.PI * 2;
        const nx = Math.cos(angle) * frames.normals[point * 3] + Math.sin(angle) * frames.binormals[point * 3];
        const ny = Math.cos(angle) * frames.normals[point * 3 + 1] + Math.sin(angle) * frames.binormals[point * 3 + 1];
        const nz = Math.cos(angle) * frames.normals[point * 3 + 2] + Math.sin(angle) * frames.binormals[point * 3 + 2];
        const target = vertexOffset * 3;
        positions[target] = path.points[pointOffset] + nx * radius;
        positions[target + 1] = path.points[pointOffset + 1] + ny * radius;
        positions[target + 2] = path.points[pointOffset + 2] + nz * radius;
        normals[target] = nx;
        normals[target + 1] = ny;
        normals[target + 2] = nz;
        colors.set(path.colors.subarray(point * 4, point * 4 + 4), vertexOffset * 4);
        distances[vertexOffset] = lengths[point];
        pathIndices[vertexOffset] = pathIndex;
        vertexOffset += 1;
      }
    }
    for (let point = 0; point < path.count - 1; point += 1) {
      for (let side = 0; side < radialSegments; side += 1) {
        const a = startVertex + point * verticesPerRing + side;
        const b = startVertex + (point + 1) * verticesPerRing + side;
        const c = startVertex + (point + 1) * verticesPerRing + side + 1;
        const d = startVertex + point * verticesPerRing + side + 1;
        indices[indexOffset++] = a;
        indices[indexOffset++] = b;
        indices[indexOffset++] = d;
        indices[indexOffset++] = b;
        indices[indexOffset++] = c;
        indices[indexOffset++] = d;
      }
    }
    ranges.push({
      pathId: path.id,
      vertexOffset: startVertex,
      vertexCount: path.count * verticesPerRing,
      indexOffset,
    });
  });

  return {
    kind: "metric.visual.tube_path_geometry.v1",
    evidence: normalized,
    positions,
    normals,
    colors,
    distances,
    pathIndices,
    indices,
    vertexCount,
    indexCount: indices.length,
    radialSegments,
    ranges,
    bounds: normalized.bounds,
  };
}

/**
 * Project evidence to a ground plane and build buffers from the projected paths.
 *
 * @param {object|Array|ArrayBufferView} evidence
 * @param {object} [options]
 * @returns {object}
 */
export function buildGroundProjectedPathBuffers(evidence, options = {}) {
  const projected = projectPathEvidenceToGround(evidence, options);
  return options.mode === "ribbon"
    ? buildRibbonPathBuffers(projected, options)
    : buildPolylinePathBuffers(projected, options);
}

function countSegments(paths) {
  return paths.reduce((sum, path) => sum + Math.max(0, path.count - 1), 0);
}

function copyPathVertex(path, point, positions, colors, widths, vertex) {
  positions.set(path.points.subarray(point * 3, point * 3 + 3), vertex * 3);
  colors.set(path.colors.subarray(point * 4, point * 4 + 4), vertex * 4);
  widths[vertex] = path.widths[point];
}

function computePathFrames(points, count) {
  const tangents = new Float32Array(count * 3);
  const normals = new Float32Array(count * 3);
  const binormals = new Float32Array(count * 3);
  for (let index = 0; index < count; index += 1) {
    const prev = Math.max(0, index - 1);
    const next = Math.min(count - 1, index + 1);
    writeNormalizedDifference(tangents, index, points, prev, next);
  }
  let reference = Math.abs(tangents[1]) < 0.92 ? [0, 1, 0] : [1, 0, 0];
  writeCross(normals, 0, reference, 0, tangents, 0);
  normalizeVector(normals, 0);
  writeCross(binormals, 0, tangents, 0, normals, 0);
  normalizeVector(binormals, 0);

  for (let index = 1; index < count; index += 1) {
    const previousNormal = [normals[(index - 1) * 3], normals[(index - 1) * 3 + 1], normals[(index - 1) * 3 + 2]];
    const tangentOffset = index * 3;
    const dot = previousNormal[0] * tangents[tangentOffset]
      + previousNormal[1] * tangents[tangentOffset + 1]
      + previousNormal[2] * tangents[tangentOffset + 2];
    normals[tangentOffset] = previousNormal[0] - tangents[tangentOffset] * dot;
    normals[tangentOffset + 1] = previousNormal[1] - tangents[tangentOffset + 1] * dot;
    normals[tangentOffset + 2] = previousNormal[2] - tangents[tangentOffset + 2] * dot;
    if (!normalizeVector(normals, index)) {
      reference = Math.abs(tangents[tangentOffset + 1]) < 0.92 ? [0, 1, 0] : [1, 0, 0];
      writeCross(normals, index, reference, 0, tangents, index);
      normalizeVector(normals, index);
    }
    writeCross(binormals, index, tangents, index, normals, index);
    normalizeVector(binormals, index);
  }
  return { tangents, normals, binormals };
}

function writeNormalizedDifference(out, targetIndex, points, aIndex, bIndex) {
  const target = targetIndex * 3;
  const a = aIndex * 3;
  const b = bIndex * 3;
  out[target] = points[b] - points[a];
  out[target + 1] = points[b + 1] - points[a + 1];
  out[target + 2] = points[b + 2] - points[a + 2];
  normalizeVector(out, targetIndex);
}

function writeCross(out, outIndex, a, aIndex, b, bIndex) {
  const target = outIndex * 3;
  const bx = Array.isArray(b) ? b[0] : b[bIndex * 3];
  const by = Array.isArray(b) ? b[1] : b[bIndex * 3 + 1];
  const bz = Array.isArray(b) ? b[2] : b[bIndex * 3 + 2];
  const ax = Array.isArray(a) ? a[0] : a[aIndex * 3];
  const ay = Array.isArray(a) ? a[1] : a[aIndex * 3 + 1];
  const az = Array.isArray(a) ? a[2] : a[aIndex * 3 + 2];
  out[target] = ay * bz - az * by;
  out[target + 1] = az * bx - ax * bz;
  out[target + 2] = ax * by - ay * bx;
}

function normalizeVector(out, index) {
  const offset = index * 3;
  const length = Math.hypot(out[offset], out[offset + 1], out[offset + 2]);
  if (!Number.isFinite(length) || length < 0.000001) return false;
  out[offset] /= length;
  out[offset + 1] /= length;
  out[offset + 2] /= length;
  return true;
}

function finitePositive(value, fallback) {
  const number = Number(value);
  return Number.isFinite(number) && number > 0 ? number : fallback;
}
