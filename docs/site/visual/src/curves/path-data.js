const DEFAULT_COLOR = [0.16, 0.52, 0.64, 0.82];
const DEFAULT_WIDTH = 3;

/**
 * Normalize exported METRIC path evidence into renderable path records.
 *
 * This function does not infer, simplify, solve, smooth, cluster, or otherwise
 * change METRIC evidence. It only accepts already-exported coordinates and
 * optional visual channels so downstream builders can create GPU buffers.
 *
 * @param {object|Array|ArrayBufferView} evidence Exported path evidence.
 * @param {object} [options]
 * @param {boolean} [options.resample=false] Enable visual-only tessellation resampling.
 * @param {number} [options.maxSegmentLength] Maximum visual segment length after resampling.
 * @param {number} [options.samples] Number of visual samples per path after resampling.
 * @param {Array<number>} [options.defaultColor]
 * @param {number} [options.defaultWidth]
 * @returns {{kind:string, paths:Array, bounds:object|null, diagnostics:object}}
 */
export function normalizePathEvidence(evidence, options = {}) {
  const records = extractPathRecords(evidence);
  const paths = [];
  const diagnostics = {
    inputPathCount: records.length,
    outputPathCount: 0,
    skippedPathCount: 0,
    resampled: Boolean(options.resample || options.maxSegmentLength || options.samples),
    resamplingIsVisualTessellation: Boolean(options.resample || options.maxSegmentLength || options.samples),
  };

  for (let index = 0; index < records.length; index += 1) {
    const normalized = normalizeSinglePath(records[index], index, options);
    if (normalized.count < 2) {
      diagnostics.skippedPathCount += 1;
      continue;
    }
    paths.push(applyVisualResampling(normalized, options));
  }

  diagnostics.outputPathCount = paths.length;
  return {
    kind: "metric.visual.curve_path_evidence.v1",
    paths,
    bounds: computePathBounds(paths),
    diagnostics,
  };
}

/**
 * Project path coordinates to a ground plane for visual context.
 *
 * The returned path records preserve the original point count, times, widths,
 * colors, ids, and metadata. Only the render coordinate component is replaced.
 *
 * @param {object|Array|ArrayBufferView} evidence
 * @param {object} [options]
 * @param {"xy"|"xz"|"yz"} [options.plane="xz"]
 * @param {number} [options.groundY=0]
 * @param {number} [options.groundZ=0]
 * @param {number} [options.groundX=0]
 * @returns {{kind:string, paths:Array, bounds:object|null, diagnostics:object}}
 */
export function projectPathEvidenceToGround(evidence, options = {}) {
  const normalized = evidence?.kind === "metric.visual.curve_path_evidence.v1"
    ? evidence
    : normalizePathEvidence(evidence, options);
  const plane = options.plane || (Number.isFinite(Number(options.groundZ)) ? "xy" : "xz");
  const paths = normalized.paths.map((path) => {
    const points = new Float32Array(path.points);
    for (let offset = 0; offset < points.length; offset += 3) {
      if (plane === "xy") points[offset + 2] = finiteNumber(options.groundZ, 0);
      else if (plane === "yz") points[offset] = finiteNumber(options.groundX, 0);
      else points[offset + 1] = finiteNumber(options.groundY, 0);
    }
    return { ...path, points, metadata: { ...path.metadata, projectedPlane: plane } };
  });
  return {
    ...normalized,
    kind: "metric.visual.ground_projected_curve_path_evidence.v1",
    paths,
    bounds: computePathBounds(paths),
    diagnostics: { ...normalized.diagnostics, projectedPlane: plane },
  };
}

/**
 * Compute bounds for normalized path evidence.
 *
 * @param {Array<{points:Float32Array,count:number}>} paths
 * @returns {{min:number[], max:number[]}|null}
 */
export function computePathBounds(paths = []) {
  const min = [Infinity, Infinity, Infinity];
  const max = [-Infinity, -Infinity, -Infinity];
  for (const path of paths) {
    for (let offset = 0; offset < path.points.length; offset += 3) {
      for (let axis = 0; axis < 3; axis += 1) {
        const value = path.points[offset + axis];
        if (!Number.isFinite(value)) continue;
        min[axis] = Math.min(min[axis], value);
        max[axis] = Math.max(max[axis], value);
      }
    }
  }
  return Number.isFinite(min[0]) ? { min, max } : null;
}

/**
 * Resample a path with linear interpolation for visual tessellation only.
 *
 * This is appropriate for long exported segments that need enough vertices for
 * thick ribbons or tubes. The returned points remain on the original polyline;
 * no algorithmic curve fitting or evidence mutation is performed.
 *
 * @param {object} path Normalized path.
 * @param {object} [options]
 * @param {number} [options.maxSegmentLength]
 * @param {number} [options.samples]
 * @returns {object}
 */
export function resamplePathForTessellation(path, options = {}) {
  const totalLength = path.totalLength || computeCumulativeLengths(path.points, path.count).totalLength;
  const targetCount = resolveResampleCount(path.count, totalLength, options);
  if (targetCount <= path.count) return path;

  const lengths = computeCumulativeLengths(path.points, path.count).lengths;
  const points = new Float32Array(targetCount * 3);
  const colors = new Float32Array(targetCount * 4);
  const widths = new Float32Array(targetCount);
  const times = path.times ? new Float32Array(targetCount) : null;

  for (let index = 0; index < targetCount; index += 1) {
    const distance = targetCount === 1 ? 0 : (index / (targetCount - 1)) * totalLength;
    const sample = samplePathAtDistance(path, lengths, distance);
    points.set(sample.position, index * 3);
    colors.set(sample.color, index * 4);
    widths[index] = sample.width;
    if (times) times[index] = sample.time;
  }

  return {
    ...path,
    points,
    colors,
    widths,
    times,
    count: targetCount,
    totalLength,
    metadata: {
      ...path.metadata,
      visualResampling: {
        originalCount: path.count,
        outputCount: targetCount,
        maxSegmentLength: options.maxSegmentLength ?? null,
        samples: options.samples ?? null,
        evidenceMutation: false,
      },
    },
  };
}

/**
 * Build cumulative path distances.
 *
 * @param {Float32Array} points
 * @param {number} count
 * @returns {{lengths:Float32Array,totalLength:number}}
 */
export function computeCumulativeLengths(points, count) {
  const lengths = new Float32Array(count);
  let totalLength = 0;
  for (let index = 1; index < count; index += 1) {
    const a = (index - 1) * 3;
    const b = index * 3;
    totalLength += Math.hypot(
      points[b] - points[a],
      points[b + 1] - points[a + 1],
      points[b + 2] - points[a + 2],
    );
    lengths[index] = totalLength;
  }
  return { lengths, totalLength };
}

function extractPathRecords(evidence) {
  if (!evidence) return [];
  if (evidence.kind === "metric.visual.curve_path_evidence.v1" && Array.isArray(evidence.paths)) return evidence.paths;
  if (ArrayBuffer.isView(evidence)) return [{ positions: evidence }];
  if (Array.isArray(evidence)) {
    if (!evidence.length) return [];
    if (isPointLike(evidence[0])) return [{ points: evidence }];
    return evidence;
  }
  return evidence.paths
    || evidence.trajectories
    || evidence.routes
    || evidence.curves
    || evidence.traces
    || evidence.histories
    || evidence.pathBundle
    || [evidence];
}

function normalizeSinglePath(record, index, options) {
  if (record?.kind === "metric.visual.curve_path") return record;
  const source = record || {};
  const points = extractPoints(source);
  const count = Math.floor(points.length / 3);
  const defaultColor = normalizeColor(options.defaultColor || source.color || source.material?.color || DEFAULT_COLOR, DEFAULT_COLOR);
  const colors = extractColors(source, count, defaultColor);
  const widths = extractWidths(source, count, finiteNumber(options.defaultWidth ?? source.width, DEFAULT_WIDTH));
  const times = extractScalars(source, ["times", "time", "t"], count);
  const cumulative = computeCumulativeLengths(points, count);
  return {
    kind: "metric.visual.curve_path",
    id: String(source.id ?? source.pathId ?? source.trajectoryId ?? `path-${index}`),
    points,
    count,
    colors,
    widths,
    times,
    totalLength: cumulative.totalLength,
    metadata: {
      ...source.metadata,
      label: source.label,
      evidenceType: source.evidenceType || source.type || options.evidenceType || "path",
      sourceIndex: index,
    },
  };
}

function extractPoints(source) {
  if (source?.points instanceof Float32Array && source.points.length % 3 === 0) return source.points;
  const flat = source?.positions || source?.position || source?.coordinates || source?.coords || source?.points || source?.path;
  if (ArrayBuffer.isView(flat)) return typedPositionsToFloat32(flat, source.itemSize || source.size || 3);
  if (Array.isArray(flat)) {
    if (!flat.length) return new Float32Array(0);
    if (typeof flat[0] === "number") return typedPositionsToFloat32(flat, source.itemSize || source.size || 3);
    return pointListToFloat32(flat);
  }
  return new Float32Array(0);
}

function typedPositionsToFloat32(source, itemSize = 3) {
  const size = Math.max(2, Math.floor(Number(itemSize) || 3));
  const count = Math.floor(source.length / size);
  const out = new Float32Array(count * 3);
  for (let index = 0; index < count; index += 1) {
    const sourceOffset = index * size;
    const targetOffset = index * 3;
    out[targetOffset] = finiteNumber(source[sourceOffset], 0);
    out[targetOffset + 1] = finiteNumber(source[sourceOffset + 1], 0);
    out[targetOffset + 2] = finiteNumber(source[sourceOffset + 2], 0);
  }
  return out;
}

function pointListToFloat32(points) {
  const out = new Float32Array(points.length * 3);
  for (let index = 0; index < points.length; index += 1) {
    const point = points[index];
    const offset = index * 3;
    out[offset] = finiteNumber(Array.isArray(point) || ArrayBuffer.isView(point) ? point[0] : point?.x, 0);
    out[offset + 1] = finiteNumber(Array.isArray(point) || ArrayBuffer.isView(point) ? point[1] : point?.y, 0);
    out[offset + 2] = finiteNumber(Array.isArray(point) || ArrayBuffer.isView(point) ? point[2] : point?.z, 0);
  }
  return out;
}

function extractColors(source, count, fallback) {
  if (source?.colors instanceof Float32Array && source.colors.length >= count * 4) return source.colors;
  const colorSource = source?.colors || source?.color || source?.rgba;
  const out = new Float32Array(count * 4);
  if (ArrayBuffer.isView(colorSource) || (Array.isArray(colorSource) && typeof colorSource[0] === "number")) {
    const itemSize = colorSource.length >= count * 4 ? 4 : colorSource.length >= count * 3 ? 3 : colorSource.length;
    for (let index = 0; index < count; index += 1) {
      const color = normalizeColor([
        colorSource[index * itemSize],
        colorSource[index * itemSize + 1],
        colorSource[index * itemSize + 2],
        itemSize >= 4 ? colorSource[index * itemSize + 3] : fallback[3],
      ], fallback);
      out.set(color, index * 4);
    }
    return out;
  }
  const pointColors = source?.points || source?.path;
  if (Array.isArray(pointColors) && pointColors.length === count && pointColors.some((point) => point?.color || point?.rgba)) {
    for (let index = 0; index < count; index += 1) {
      out.set(normalizeColor(pointColors[index]?.color || pointColors[index]?.rgba, fallback), index * 4);
    }
    return out;
  }
  for (let index = 0; index < count; index += 1) out.set(fallback, index * 4);
  return out;
}

function extractWidths(source, count, fallback) {
  const direct = source?.widths || source?.width;
  const out = new Float32Array(count);
  if (ArrayBuffer.isView(direct) || Array.isArray(direct)) {
    for (let index = 0; index < count; index += 1) out[index] = finiteNumber(direct[index], fallback);
    return out;
  }
  const pointWidths = source?.points || source?.path;
  if (Array.isArray(pointWidths) && pointWidths.length === count && pointWidths.some((point) => point?.width != null)) {
    for (let index = 0; index < count; index += 1) out[index] = finiteNumber(pointWidths[index]?.width, fallback);
    return out;
  }
  out.fill(fallback);
  return out;
}

function extractScalars(source, names, count) {
  for (const name of names) {
    const direct = source?.[name];
    if (ArrayBuffer.isView(direct) || Array.isArray(direct)) {
      const out = new Float32Array(count);
      for (let index = 0; index < count; index += 1) out[index] = finiteNumber(direct[index], index);
      return out;
    }
  }
  const points = source?.points || source?.path;
  if (Array.isArray(points) && points.length === count && points.some((point) => point?.time != null || point?.t != null)) {
    const out = new Float32Array(count);
    for (let index = 0; index < count; index += 1) out[index] = finiteNumber(points[index]?.time ?? points[index]?.t, index);
    return out;
  }
  return null;
}

function applyVisualResampling(path, options) {
  if (!(options.resample || options.maxSegmentLength || options.samples)) return path;
  return resamplePathForTessellation(path, options);
}

function resolveResampleCount(count, totalLength, options) {
  const samples = Math.floor(Number(options.samples) || 0);
  if (samples > count) return samples;
  const maxSegmentLength = Number(options.maxSegmentLength);
  if (Number.isFinite(maxSegmentLength) && maxSegmentLength > 0 && totalLength > 0) {
    return Math.max(count, Math.ceil(totalLength / maxSegmentLength) + 1);
  }
  return count;
}

function samplePathAtDistance(path, lengths, distance) {
  const lastIndex = path.count - 1;
  if (distance <= 0) return samplePathPoint(path, 0, 0, 0);
  if (distance >= lengths[lastIndex]) return samplePathPoint(path, lastIndex - 1, lastIndex, 1);
  let upper = 1;
  while (upper < lengths.length && lengths[upper] < distance) upper += 1;
  const lower = Math.max(0, upper - 1);
  const span = Math.max(0.000001, lengths[upper] - lengths[lower]);
  return samplePathPoint(path, lower, upper, (distance - lengths[lower]) / span);
}

function samplePathPoint(path, lower, upper, t) {
  const ao = lower * 3;
  const bo = upper * 3;
  const ac = lower * 4;
  const bc = upper * 4;
  return {
    position: [
      lerp(path.points[ao], path.points[bo], t),
      lerp(path.points[ao + 1], path.points[bo + 1], t),
      lerp(path.points[ao + 2], path.points[bo + 2], t),
    ],
    color: [
      lerp(path.colors[ac], path.colors[bc], t),
      lerp(path.colors[ac + 1], path.colors[bc + 1], t),
      lerp(path.colors[ac + 2], path.colors[bc + 2], t),
      lerp(path.colors[ac + 3], path.colors[bc + 3], t),
    ],
    width: lerp(path.widths[lower], path.widths[upper], t),
    time: path.times ? lerp(path.times[lower], path.times[upper], t) : 0,
  };
}

function normalizeColor(color, fallback) {
  if (!(Array.isArray(color) || ArrayBuffer.isView(color))) return fallback.slice();
  const divisor = Math.max(Number(color[0]) || 0, Number(color[1]) || 0, Number(color[2]) || 0) > 1 ? 255 : 1;
  return [
    clamp01(finiteNumber(color[0], fallback[0]) / divisor),
    clamp01(finiteNumber(color[1], fallback[1]) / divisor),
    clamp01(finiteNumber(color[2], fallback[2]) / divisor),
    clamp01(color[3] == null ? fallback[3] : finiteNumber(color[3], fallback[3])),
  ];
}

function isPointLike(value) {
  return ArrayBuffer.isView(value)
    || Array.isArray(value)
    || (value && typeof value === "object" && ("x" in value || "y" in value || "z" in value));
}

function finiteNumber(value, fallback) {
  const number = Number(value);
  return Number.isFinite(number) ? number : fallback;
}

function clamp01(value) {
  return Math.max(0, Math.min(1, finiteNumber(value, 0)));
}

function lerp(a, b, t) {
  return a + (b - a) * t;
}

