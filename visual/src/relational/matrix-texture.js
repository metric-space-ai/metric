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
  const denseValues = resolveDenseMatrixValues(source);
  const sourceRecordIds = denseValues
    ? collectDenseMatrixRecordIds(source, options)
    : collectRecordIds(source, options);
  const recordIds = resolveMatrixRecordIds(sourceRecordIds, source, options);
  const size = recordIds.length;
  const indexById = indexRecordIds(recordIds);
  const missingValue = options.missingValue ?? DEFAULT_MISSING_VALUE;
  const diagonalValue = options.diagonalValue ?? 0;
  const values = new Float32Array(size * size);
  const present = new Uint8Array(size * size);
  const pairEvidence = new Map();
  const symmetric = options.symmetric === true ? "mirror" : options.symmetric || "directed";
  const relationId = options.relationId
    ?? source?.id
    ?? source?.relation_id
    ?? source?.relationId
    ?? null;
  const relationName = options.relationName
    ?? source?.name
    ?? source?.label
    ?? null;

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

  if (denseValues) {
    const counters = fillFromDenseMatrixValues({
      denseValues,
      sourceRecordIds,
      recordIds,
      values,
      present,
      missingValue,
    });
    pairCount = counters.pairCount;
    acceptedPairCount = counters.acceptedPairCount;
    skippedPairCount = counters.skippedPairCount;
  } else {
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
        relationId,
        relationName,
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
          relationId,
          relationName,
        }));
      }
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
  const symmetryDiagnostics = denseValues
    ? diagnoseDenseMatrixSymmetry(recordIds, values, present, options)
    : diagnoseRelationSymmetry(source, { ...options, recordIds: sourceRecordIds });
  const resolvedPairEvidence = denseValues
    ? createDenseMatrixPairEvidenceLookup({
      recordIds,
      values,
      present,
      relationId,
      relationName,
    })
    : pairEvidence;

  return {
    kind: "dense-relation-matrix",
    size,
    width: size,
    height: size,
    recordIds,
    sourceRecordIds,
    relationId,
    relationName,
    values,
    present,
    pairEvidence: resolvedPairEvidence,
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
      relationId,
      relationName,
      denseEncoding: denseValues?.kind || null,
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
    relationId: matrix.relationId,
    relationName: matrix.relationName,
    scaler,
    palette,
    diagnostics: matrix.diagnostics,
  };
}

function resolveDenseMatrixValues(source) {
  const values = source?.values;
  if (!Array.isArray(values) || !values.length) return null;
  if (Array.isArray(values[0])) {
    return { kind: "row-array dense matrix", values, size: values.length };
  }
  if (values.every((entry) => entry == null || typeof entry !== "object")) {
    const recordIds = Array.isArray(source?.record_ids)
      ? source.record_ids
      : Array.isArray(source?.recordIds)
        ? source.recordIds
        : [];
    const size = recordIds.length || Math.sqrt(values.length);
    if (Number.isInteger(size) && size > 0 && values.length === size * size) {
      return { kind: "row-major flat dense matrix", values, size };
    }
  }
  return null;
}

function collectDenseMatrixRecordIds(source, options) {
  const explicit = Array.isArray(source?.record_ids)
    ? source.record_ids
    : Array.isArray(source?.recordIds)
      ? source.recordIds
      : null;
  if (explicit?.length) return explicit.map((id) => String(id));
  return collectRecordIds(source, options);
}

function fillFromDenseMatrixValues(input) {
  const {
    denseValues,
    sourceRecordIds,
    recordIds,
    values,
    present,
    missingValue,
  } = input;
  const sourceIndexById = indexRecordIds(sourceRecordIds);
  let pairCount = 0;
  let acceptedPairCount = 0;
  let skippedPairCount = 0;

  for (let row = 0; row < recordIds.length; row += 1) {
    const sourceRow = sourceIndexById.get(recordIds[row]);
    for (let column = 0; column < recordIds.length; column += 1) {
      const sourceColumn = sourceIndexById.get(recordIds[column]);
      const offset = row * recordIds.length + column;
      pairCount += 1;
      if (sourceRow == null || sourceColumn == null) {
        values[offset] = missingValue;
        present[offset] = 0;
        skippedPairCount += 1;
        continue;
      }
      const value = Number(readDenseMatrixValue(denseValues, sourceRow, sourceColumn));
      if (!Number.isFinite(value)) {
        values[offset] = missingValue;
        present[offset] = 0;
        skippedPairCount += 1;
        continue;
      }
      values[offset] = value;
      present[offset] = 1;
      acceptedPairCount += 1;
    }
  }

  return { pairCount, acceptedPairCount, skippedPairCount };
}

function readDenseMatrixValue(denseValues, row, column) {
  if (denseValues.kind === "row-array dense matrix") return denseValues.values[row]?.[column];
  return denseValues.values[row * denseValues.size + column];
}

function createDenseMatrixPairEvidenceLookup(input) {
  const { recordIds, values, present, relationId, relationName } = input;
  return {
    get(offset) {
      const index = Number(offset);
      const size = recordIds.length;
      if (!Number.isInteger(index) || index < 0 || index >= size * size) return null;
      if (present && present[index] !== 1) return null;
      const row = Math.floor(index / size);
      const column = index % size;
      const rowId = recordIds[row];
      const columnId = recordIds[column];
      const value = values[index];
      return describePairEvidence({
        row_id: rowId,
        column_id: columnId,
        value,
      }, {
        rowId,
        columnId,
        row,
        column,
        offset: index,
        value,
        mirrored: false,
        relationId,
        relationName,
      });
    },
  };
}

function diagnoseDenseMatrixSymmetry(recordIds, values, present, options = {}) {
  const tolerance = Number.isFinite(options.tolerance) ? Math.max(0, options.tolerance) : 1e-9;
  const missingReciprocals = [];
  const mismatches = [];
  const asymmetricPairs = [];
  let presentPairCount = 0;
  let reciprocalCount = 0;
  let comparedCount = 0;
  let directedOnlyCount = 0;
  let identicalReciprocalCount = 0;
  let maxDelta = 0;
  const size = recordIds.length;

  for (let row = 0; row < size; row += 1) {
    for (let column = 0; column < size; column += 1) {
      const offset = row * size + column;
      if (present && present[offset] !== 1) continue;
      presentPairCount += 1;
      if (row === column) continue;
      const reverseOffset = column * size + row;
      const hasReverse = !present || present[reverseOffset] === 1;
      const value = values[offset];
      if (!hasReverse) {
        directedOnlyCount += 1;
        missingReciprocals.push({ source: recordIds[row], target: recordIds[column], value });
        continue;
      }
      reciprocalCount += 1;
      const delta = Math.abs(value - values[reverseOffset]);
      if (delta <= tolerance) identicalReciprocalCount += 1;
      if (delta > maxDelta) maxDelta = delta;
      if (row < column) {
        comparedCount += 1;
        if (delta > tolerance) {
          const mismatch = {
            source: recordIds[row],
            target: recordIds[column],
            value,
            reverseValue: values[reverseOffset],
            delta,
          };
          mismatches.push(mismatch);
          asymmetricPairs.push(mismatch);
        }
      } else if (delta > tolerance) {
        asymmetricPairs.push({
          source: recordIds[row],
          target: recordIds[column],
          value,
          reverseValue: values[reverseOffset],
          delta,
        });
      }
    }
  }

  const symmetric = {
    kind: "symmetric-relation-check",
    recordCount: size,
    pairCount: presentPairCount,
    reciprocalCount,
    comparedCount,
    tolerance,
    maxDelta,
    missingReciprocalCount: missingReciprocals.length,
    mismatchCount: mismatches.length,
    isSymmetric: missingReciprocals.length === 0 && mismatches.length === 0,
    missingReciprocals,
    mismatches,
  };
  const asymmetric = {
    kind: "asymmetric-relation-check",
    pairCount: presentPairCount,
    reciprocalCount,
    directedOnlyCount,
    identicalReciprocalCount,
    asymmetricPairCount: asymmetricPairs.length,
    tolerance,
    maxDelta,
    hasDirectedEvidence: directedOnlyCount > 0 || asymmetricPairs.length > 0,
    asymmetricPairs,
  };
  return {
    kind: "relation-symmetry-diagnostics",
    symmetric,
    asymmetric,
    recommendedMatrixMode: symmetric.isSymmetric ? "symmetric" : "directed",
  };
}

function describePairEvidence(pair, cell) {
  const properties = pairProperties(pair);
  const relationId = pair?.relation_id ?? pair?.relationId ?? cell.relationId ?? null;
  const pairId = pair?.id ?? pair?.pair_id ?? pair?.pairId ?? null;
  const pairKey = `${relationId || "relation"}\u0000${cell.rowId}\u0000${cell.columnId}`;
  return {
    id: pairId,
    pairId,
    pairKey,
    relationId,
    relationName: cell.relationName ?? null,
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
