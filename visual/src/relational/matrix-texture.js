import {
  collectRecordIds,
  indexRecordIds,
  pairSourceId,
  pairTargetId,
  pairValue,
  relationPairs,
} from "./relation-source.js";
import { diagnoseRelationSymmetry } from "./diagnostics.js";
import { resolveRelationScaler, summarizeFiniteValues } from "./scaling.js";

const DEFAULT_MISSING_VALUE = Number.NaN;

/**
 * Build a dense NxN relation matrix from exported pair values.
 *
 * Missing cells remain as `missingValue`. Set `symmetric: "mirror"` to copy
 * supplied values into the reciprocal cell for known symmetric exports; the
 * function never computes new metric values.
 *
 * @param {object|Array} source Relation source or raw pair list.
 * @param {object} [options]
 * @param {Array<string|number|object>} [options.recordIds]
 * @param {string|string[]} [options.valueKey="value"]
 * @param {"directed"|"mirror"|boolean} [options.symmetric="directed"]
 * @param {number} [options.diagonalValue=0]
 * @param {number} [options.missingValue=NaN]
 * @returns {object}
 */
export function buildDenseRelationMatrix(source, options = {}) {
  const sourceRecordIds = collectRecordIds(source, options);
  const recordIds = resolveMatrixRecordIds(sourceRecordIds, source, options);
  const size = recordIds.length;
  const indexById = indexRecordIds(recordIds);
  const missingValue = options.missingValue ?? DEFAULT_MISSING_VALUE;
  const diagonalValue = options.diagonalValue ?? 0;
  const values = new Float32Array(size * size);
  const present = new Uint8Array(size * size);
  const pairEvidence = new Map();
  const symmetric = options.symmetric === true ? "mirror" : options.symmetric || "directed";

  for (let index = 0; index < values.length; index += 1) values[index] = missingValue;
  for (let index = 0; index < size; index += 1) {
    const offset = index * size + index;
    values[offset] = diagonalValue;
    present[offset] = Number.isFinite(diagonalValue) ? 1 : 0;
  }

  let pairCount = 0;
  let acceptedPairCount = 0;
  let duplicatePairCount = 0;
  let skippedPairCount = 0;

  for (const pair of relationPairs(source)) {
    pairCount += 1;
    const sourceId = pairSourceId(pair, sourceRecordIds);
    const targetId = pairTargetId(pair, sourceRecordIds);
    const sourceIndex = indexById.get(sourceId);
    const targetIndex = indexById.get(targetId);
    const value = pairValue(pair, options.valueKey);

    if (sourceIndex == null || targetIndex == null || !Number.isFinite(value)) {
      skippedPairCount += 1;
      continue;
    }

    const offset = sourceIndex * size + targetIndex;
    if (present[offset]) duplicatePairCount += 1;
    values[offset] = value;
    present[offset] = 1;
    pairEvidence.set(offset, describePairEvidence(pair, {
      rowId: sourceId,
      columnId: targetId,
      row: sourceIndex,
      column: targetIndex,
      offset,
      value,
      mirrored: false,
    }));
    acceptedPairCount += 1;

    if (symmetric === "mirror" && sourceIndex !== targetIndex) {
      const reverseOffset = targetIndex * size + sourceIndex;
      if (present[reverseOffset]) duplicatePairCount += 1;
      values[reverseOffset] = value;
      present[reverseOffset] = 1;
      pairEvidence.set(reverseOffset, describePairEvidence(pair, {
        rowId: targetId,
        columnId: sourceId,
        row: targetIndex,
        column: sourceIndex,
        offset: reverseOffset,
        value,
        mirrored: true,
      }));
    }
  }

  let presentValueCount = 0;
  for (let index = 0; index < present.length; index += 1) {
    if (present[index]) presentValueCount += 1;
  }
  const missingValueCount = present.length - presentValueCount;
  const blockRanges = normalizeBlockRanges(
    options.blockRanges
      || options.block_ranges
      || source?.metadata?.block_ranges
      || source?.metadata?.blockRanges,
    size,
  );
  const metricLawDiagnostic = source?.metadata?.law_check
    || source?.metadata?.lawCheck
    || source?.diagnostics?.law_check
    || source?.diagnostics?.lawCheck
    || null;
  const ordering = describeMatrixOrdering(sourceRecordIds, recordIds, source, options, blockRanges);
  const symmetryDiagnostics = diagnoseRelationSymmetry(source, { ...options, recordIds: sourceRecordIds });

  return {
    kind: "dense-relation-matrix",
    size,
    width: size,
    height: size,
    recordIds,
    sourceRecordIds,
    values,
    present,
    pairEvidence,
    missingValue,
    diagonalValue,
    symmetric,
    pairCount,
    acceptedPairCount,
    duplicatePairCount,
    skippedPairCount,
    presentValueCount,
    missingValueCount,
    ordering,
    blockRanges,
    metricLawDiagnostic,
    finiteSummary: summarizeFiniteMatrix(values, present),
    diagnostics: {
      ...symmetryDiagnostics,
      matrixDimensions: { width: size, height: size, size },
      presentValueCount,
      missingValueCount,
      acceptedPairCount,
      duplicatePairCount,
      skippedPairCount,
      blockRanges,
      ordering,
      metricLawDiagnostic,
    },
  };
}

/**
 * Build RGBA8 matrix texture data for WebGL matrix rendering.
 *
 * Channels:
 * - R/G/B: encoded color from the selected palette.
 * - A: 255 when the cell has relation evidence, otherwise missingAlpha.
 *
 * @param {object|Array} source
 * @param {object} [options]
 * @param {string|object|function} [options.scale="minMax"]
 * @param {string|Array<Array<number>>} [options.palette="magma"]
 * @param {number} [options.missingAlpha=0]
 * @returns {object}
 */
export function buildRelationMatrixTextureData(source, options = {}) {
  const matrix = source?.kind === "dense-relation-matrix" ? source : buildDenseRelationMatrix(source, options);
  const finiteValues = [];
  for (let index = 0; index < matrix.values.length; index += 1) {
    if (matrix.present[index]) finiteValues.push(matrix.values[index]);
  }
  const scaler = resolveRelationScaler(Float64Array.from(finiteValues), options.scale || "minMax");
  const palette = resolvePalette(options.palette || "magma");
  const data = new Uint8Array(matrix.width * matrix.height * 4);
  const missingAlpha = clampByte(options.missingAlpha ?? 0);
  const missingColor = options.missingColor || [0, 0, 0];

  for (let row = 0; row < matrix.height; row += 1) {
    for (let column = 0; column < matrix.width; column += 1) {
      const sourceOffset = row * matrix.width + column;
      const targetOffset = sourceOffset * 4;
      if (!matrix.present[sourceOffset]) {
        data[targetOffset] = clampByte(missingColor[0] || 0);
        data[targetOffset + 1] = clampByte(missingColor[1] || 0);
        data[targetOffset + 2] = clampByte(missingColor[2] || 0);
        data[targetOffset + 3] = missingAlpha;
        continue;
      }
      const color = samplePalette(palette, scaler.scale(matrix.values[sourceOffset]));
      data[targetOffset] = color[0];
      data[targetOffset + 1] = color[1];
      data[targetOffset + 2] = color[2];
      data[targetOffset + 3] = clampByte(options.alpha ?? 255);
    }
  }

  return {
    kind: "relation-matrix-texture-data",
    width: matrix.width,
    height: matrix.height,
    format: "rgba8",
    data,
    matrix,
    recordIds: matrix.recordIds,
    scaler,
    palette,
    diagnostics: matrix.diagnostics,
  };
}

function describePairEvidence(pair, cell) {
  const properties = pairProperties(pair);
  return {
    id: pair?.id ?? pair?.pair_id ?? pair?.pairId ?? null,
    relationId: pair?.relation_id ?? pair?.relationId ?? null,
    rowId: cell.rowId,
    columnId: cell.columnId,
    sourceId: cell.rowId,
    targetId: cell.columnId,
    row: cell.row,
    column: cell.column,
    offset: cell.offset,
    value: cell.value,
    mirrored: cell.mirrored,
    properties,
    pair,
  };
}

function pairProperties(pair) {
  if (!pair || typeof pair !== "object" || Array.isArray(pair)) return null;
  const explicit = pair.properties ?? pair.pair_properties ?? pair.pairProperties;
  if (explicit && typeof explicit === "object" && !Array.isArray(explicit)) return { ...explicit };
  return null;
}

export function summarizeFiniteMatrix(values, present) {
  const finite = [];
  for (let index = 0; index < values.length; index += 1) {
    if (present && !present[index]) continue;
    const value = Number(values[index]);
    if (Number.isFinite(value)) finite.push(value);
  }
  return summarizeFiniteValues(finite);
}

export function resolvePalette(palette) {
  if (Array.isArray(palette)) return palette.map(normalizeColorStop);
  switch (palette) {
    case "viridis":
      return [
        [68, 1, 84],
        [59, 82, 139],
        [33, 145, 140],
        [94, 201, 98],
        [253, 231, 37],
      ];
    case "metric":
      return [
        [20, 28, 38],
        [41, 92, 117],
        [82, 148, 139],
        [214, 181, 94],
        [244, 103, 72],
      ];
    case "gray":
    case "grey":
      return [
        [22, 22, 24],
        [245, 245, 245],
      ];
    case "magma":
    default:
      return [
        [0, 0, 4],
        [59, 15, 112],
        [140, 41, 129],
        [221, 73, 104],
        [252, 160, 90],
        [252, 253, 191],
      ];
  }
}

export function samplePalette(palette, unit) {
  const stops = resolvePalette(palette);
  if (stops.length === 0) return [0, 0, 0];
  if (stops.length === 1) return stops[0].slice();
  const t = Math.max(0, Math.min(1, Number(unit) || 0));
  const scaled = t * (stops.length - 1);
  const low = Math.floor(scaled);
  const high = Math.min(stops.length - 1, low + 1);
  const local = scaled - low;
  return [
    clampByte(lerp(stops[low][0], stops[high][0], local)),
    clampByte(lerp(stops[low][1], stops[high][1], local)),
    clampByte(lerp(stops[low][2], stops[high][2], local)),
  ];
}

function normalizeColorStop(stop) {
  return [
    clampByte(stop[0] <= 1 ? stop[0] * 255 : stop[0]),
    clampByte(stop[1] <= 1 ? stop[1] * 255 : stop[1]),
    clampByte(stop[2] <= 1 ? stop[2] * 255 : stop[2]),
  ];
}

function lerp(a, b, t) {
  return a * (1 - t) + b * t;
}

function clampByte(value) {
  const number = Number(value);
  if (!Number.isFinite(number)) return 0;
  return Math.max(0, Math.min(255, Math.round(number)));
}

function resolveMatrixRecordIds(recordIds, source, options) {
  const requestedOrder = options.recordOrder
    ?? options.record_order
    ?? (Array.isArray(options.order) || isPlainObject(options.order) ? options.order : undefined)
    ?? source?.recordOrder
    ?? source?.record_order
    ?? source?.metadata?.recordOrder
    ?? source?.metadata?.record_order
    ?? source?.metadata?.tileOrder
    ?? source?.metadata?.tile_order
    ?? source?.metadata?.blockOrder
    ?? source?.metadata?.block_order;
  return applyRecordOrder(recordIds, requestedOrder);
}

function applyRecordOrder(recordIds, requestedOrder) {
  if (!requestedOrder) return recordIds;
  if (isPlainObject(requestedOrder)) {
    return applyRecordOrder(
      recordIds,
      requestedOrder.recordIds
        || requestedOrder.record_ids
        || requestedOrder.ids
        || requestedOrder.order
        || requestedOrder.records,
    );
  }
  if (!Array.isArray(requestedOrder)) return recordIds;

  const ordered = [];
  const seen = new Set();
  const sourceIds = recordIds.map((id) => String(id));
  const sourceSet = new Set(sourceIds);

  for (const entry of requestedOrder) {
    const id = orderEntryToId(entry, recordIds);
    if (id == null || !sourceSet.has(id) || seen.has(id)) continue;
    seen.add(id);
    ordered.push(id);
  }

  if (!ordered.length) return recordIds;
  for (const id of sourceIds) {
    if (!seen.has(id)) ordered.push(id);
  }
  return ordered;
}

function orderEntryToId(entry, recordIds) {
  if (Number.isInteger(entry) && entry >= 0 && entry < recordIds.length) return String(recordIds[entry]);
  if (typeof entry === "string" || typeof entry === "number" || typeof entry === "boolean") return String(entry);
  if (!isPlainObject(entry)) return null;
  const id = entry.id ?? entry.recordId ?? entry.record_id ?? entry.key;
  if (id != null) return String(id);
  if (Number.isInteger(entry.index) && entry.index >= 0 && entry.index < recordIds.length) return String(recordIds[entry.index]);
  return null;
}

function describeMatrixOrdering(sourceRecordIds, recordIds, source, options, blockRanges) {
  const reordered = sourceRecordIds.length === recordIds.length
    && sourceRecordIds.some((id, index) => String(id) !== String(recordIds[index]));
  const nativeOrder = source?.metadata?.record_order
    ?? source?.metadata?.recordOrder
    ?? source?.record_order
    ?? source?.recordOrder
    ?? null;
  const explicitOrder = options.recordOrder ?? options.record_order ?? null;
  return {
    source: explicitOrder ? "options" : nativeOrder ? "native" : "record_ids",
    nativeOrder,
    reordered,
    blockCount: blockRanges.length,
    recordCount: recordIds.length,
  };
}

function normalizeBlockRanges(ranges, size) {
  if (!Array.isArray(ranges)) return [];
  const out = [];
  for (const range of ranges) {
    if (!isPlainObject(range)) continue;
    const start = finiteInteger(range.start ?? range.start_index ?? range.startIndex);
    const end = finiteInteger(range.end_exclusive ?? range.endExclusive ?? range.end ?? range.stop);
    if (start == null || end == null || end <= start || start < 0 || end > size) continue;
    out.push({
      ...range,
      start,
      end_exclusive: end,
      label: range.block_label ?? range.blockLabel ?? range.label ?? range.family ?? "",
    });
  }
  return out.sort((a, b) => a.start - b.start || a.end_exclusive - b.end_exclusive);
}

function finiteInteger(value) {
  const number = Number(value);
  return Number.isInteger(number) ? number : null;
}

function isPlainObject(value) {
  return value !== null && typeof value === "object" && !Array.isArray(value);
}
