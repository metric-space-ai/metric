import {
  collectRecordIds,
  indexRecordIds,
  pairSourceId,
  pairTargetId,
  pairValue,
  relationPairs,
} from "./relation-source.js";

export const RELATION_MATRIX_READABILITY_DIAGNOSTICS_SCHEMA = "metric.visual.relation_matrix_readability_diagnostics.v1";

/**
 * Check reciprocal relation values for symmetric visualization diagnostics.
 *
 * The check reports missing reciprocals and mismatched reciprocal values. It
 * does not repair or infer missing values because the renderer must preserve
 * exported relation evidence.
 *
 * @param {object|Array} source
 * @param {object} [options]
 * @param {number} [options.tolerance=1e-9]
 * @returns {object}
 */
export function checkSymmetricRelations(source, options = {}) {
  const tolerance = Number.isFinite(options.tolerance) ? Math.max(0, options.tolerance) : 1e-9;
  const recordIds = collectRecordIds(source, options);
  const indexById = indexRecordIds(recordIds);
  const values = relationMap(source, { ...options, recordIds });
  const missingReciprocals = [];
  const mismatches = [];
  let reciprocalCount = 0;
  let comparedCount = 0;
  let maxDelta = 0;

  for (const [key, entry] of values.entries()) {
    if (entry.source === entry.target) continue;
    const reverse = values.get(pairKey(entry.target, entry.source));
    if (!reverse) {
      missingReciprocals.push({ source: entry.source, target: entry.target, value: entry.value });
      continue;
    }

    reciprocalCount += 1;
    if (indexById.get(entry.source) > indexById.get(entry.target)) continue;
    comparedCount += 1;
    const delta = Math.abs(entry.value - reverse.value);
    if (delta > maxDelta) maxDelta = delta;
    if (delta > tolerance) {
      mismatches.push({
        source: entry.source,
        target: entry.target,
        value: entry.value,
        reverseValue: reverse.value,
        delta,
      });
    }
  }

  return {
    kind: "symmetric-relation-check",
    recordCount: recordIds.length,
    pairCount: values.size,
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
}

/**
 * Diagnose directed/asymmetric relation evidence for visualization.
 *
 * Useful for deciding whether matrix rendering should show the full directed
 * matrix, whether mirroring would hide meaningful evidence, and how many
 * reciprocal pairs are numerically identical.
 *
 * @param {object|Array} source
 * @param {object} [options]
 * @returns {object}
 */
export function checkAsymmetricRelations(source, options = {}) {
  const tolerance = Number.isFinite(options.tolerance) ? Math.max(0, options.tolerance) : 1e-9;
  const values = relationMap(source, options);
  const asymmetricPairs = [];
  let reciprocalCount = 0;
  let identicalReciprocalCount = 0;
  let directedOnlyCount = 0;
  let maxDelta = 0;

  for (const entry of values.values()) {
    if (entry.source === entry.target) continue;
    const reverse = values.get(pairKey(entry.target, entry.source));
    if (!reverse) {
      directedOnlyCount += 1;
      continue;
    }
    reciprocalCount += 1;
    const delta = Math.abs(entry.value - reverse.value);
    if (delta <= tolerance) identicalReciprocalCount += 1;
    if (delta > maxDelta) maxDelta = delta;
    if (delta > tolerance) {
      asymmetricPairs.push({
        source: entry.source,
        target: entry.target,
        value: entry.value,
        reverseValue: reverse.value,
        delta,
      });
    }
  }

  return {
    kind: "asymmetric-relation-check",
    pairCount: values.size,
    reciprocalCount,
    directedOnlyCount,
    identicalReciprocalCount,
    asymmetricPairCount: asymmetricPairs.length,
    tolerance,
    maxDelta,
    hasDirectedEvidence: directedOnlyCount > 0 || asymmetricPairs.length > 0,
    asymmetricPairs,
  };
}

/**
 * Run both symmetry diagnostics and return a compact summary.
 *
 * @param {object|Array} source
 * @param {object} [options]
 * @returns {object}
 */
export function diagnoseRelationSymmetry(source, options = {}) {
  const symmetric = checkSymmetricRelations(source, options);
  const asymmetric = checkAsymmetricRelations(source, options);
  return {
    kind: "relation-symmetry-diagnostics",
    symmetric,
    asymmetric,
    recommendedMatrixMode: symmetric.isSymmetric ? "symmetric" : "directed",
  };
}

/**
 * Build a compact diagnostic payload for matrix readability contracts.
 *
 * The payload is intentionally descriptive: it reports exported matrix and
 * layer state without computing missing relation values or graph topology.
 *
 * @param {object} input
 * @param {object} [input.matrix]
 * @param {object} [input.readability]
 * @param {object} [input.selection]
 * @param {object} [input.tileSummary]
 * @param {object} [input.graph]
 * @param {object} [input.renderHierarchy]
 * @param {string|null} [input.relationId]
 * @param {string|null} [input.relationName]
 * @returns {object}
 */
export function createRelationMatrixDiagnostics(input = {}) {
  const matrix = input.matrix || {};
  const readability = input.readability || {};
  const graph = input.graph || null;
  const dimensions = matrixDimensions(matrix);
  const relationId = input.relationId
    ?? matrix.relationId
    ?? graph?.relationId
    ?? graph?.edge_relation_id
    ?? null;
  const relationName = input.relationName ?? matrix.relationName ?? null;
  const tileSummarySource = input.tileSummary?.source
    ?? readability?.lod?.tileSummaryLod?.source
    ?? null;
  const blocks = blockDiagnostics(matrix, readability, dimensions);
  const metricLawDiagnostic = metricLawDiagnosticFor(matrix);
  const selected = normalizeSelectionState(input.selection, dimensions);

  return {
    schema: RELATION_MATRIX_READABILITY_DIAGNOSTICS_SCHEMA,
    kind: "relation-matrix-readability-diagnostics",
    relationId,
    relationName,
    matrixDimensions: dimensions,
    blockCount: blocks.count,
    blockLabelCount: blocks.labeledCount,
    blockBoundaryCount: Array.isArray(readability?.blocks?.boundaries) ? readability.blocks.boundaries.length : null,
    blocks,
    blockBoundaryDiagnostics: blockBoundaryDiagnostics(readability, blocks, dimensions),
    tileCount: integerOrNull(readability?.tiles?.count),
    tileSize: integerOrNull(readability?.tiles?.tileSize ?? input.tileSummary?.tileSize),
    tileSummarySource,
    tileSummaryDimensions: input.tileSummary ? {
      width: integerOrNull(input.tileSummary.width),
      height: integerOrNull(input.tileSummary.height),
    } : null,
    tileDiagnostics: tileDiagnostics(readability, input.tileSummary),
    metricLawDiagnostic,
    metricLawDiagnosticReferences: metricLawReferenceSummary(metricLawDiagnostic),
    missingValueCount: integerOrNull(matrix.missingValueCount),
    presentValueCount: integerOrNull(matrix.presentValueCount),
    selected,
    focusDiagnostics: focusDiagnostics(selected),
    renderHierarchy: normalizeRenderHierarchy(input.renderHierarchy),
    graph: graph ? {
      graphId: graph.id ?? null,
      relationId: graph.relationId ?? graph.edge_relation_id ?? relationId,
      edgeCount: integerOrNull(graph.edgeCount ?? graph.graphEdgeCount ?? graph.edges?.length),
      graphEdgeCount: integerOrNull(graph.edgeCount ?? graph.graphEdgeCount ?? graph.edges?.length),
      native: graph.native === true,
    } : null,
  };
}

function blockBoundaryDiagnostics(readability = {}, blocks = {}, dimensions = {}) {
  const boundaries = Array.isArray(readability?.blocks?.boundaries) ? readability.blocks.boundaries : [];
  return {
    kind: "relation-matrix-block-boundary-diagnostics",
    source: boundaries.length ? "exported-block-ranges" : "none",
    matrixSize: integerOrNull(dimensions.size),
    boundaryCount: boundaries.length,
    boundaries: boundaries.map((boundary) => ({
      index: integerOrNull(boundary.index),
      unit: Number.isFinite(Number(boundary.unit)) ? Number(boundary.unit) : null,
    })),
    blockCount: integerOrNull(blocks.count),
    labeledBlockCount: integerOrNull(blocks.labeledCount),
    coverage: blocks.coverage || null,
    shaderRangeLimit: integerOrNull(readability?.blocks?.shaderRangeLimit),
    shaderBoundaryLimit: integerOrNull(readability?.blocks?.shaderBoundaryLimit),
  };
}

function tileDiagnostics(readability = {}, tileSummary = null) {
  const tiles = readability?.tiles || {};
  return {
    kind: "relation-matrix-tile-diagnostics",
    source: tileSummary?.source ?? readability?.lod?.tileSummaryLod?.source ?? null,
    strategy: readability?.lod?.tileSummaryLod?.strategy ?? null,
    tileSize: integerOrNull(tiles.tileSize ?? tileSummary?.tileSize),
    rows: integerOrNull(tiles.rows ?? tileSummary?.height),
    columns: integerOrNull(tiles.columns ?? tileSummary?.width),
    count: integerOrNull(tiles.count),
    summarizedCount: integerOrNull(tiles.summarizedCount),
    coverage: tiles.coverage ?? null,
    rowBoundaryCount: Array.isArray(tiles.rowBoundaries) ? tiles.rowBoundaries.length : null,
    columnBoundaryCount: Array.isArray(tiles.columnBoundaries) ? tiles.columnBoundaries.length : null,
    summaryTexture: tileSummary ? {
      width: integerOrNull(tileSummary.width),
      height: integerOrNull(tileSummary.height),
      sourceWidth: integerOrNull(tileSummary.sourceWidth),
      sourceHeight: integerOrNull(tileSummary.sourceHeight),
    } : null,
  };
}

function focusDiagnostics(selected) {
  return {
    kind: "relation-matrix-focus-diagnostics",
    supportedFeatures: ["row", "column", "cell"],
    selectedState: selected?.state || "none",
    rowActive: selected?.row?.active === true,
    columnActive: selected?.column?.active === true,
    cellActive: selected?.cell?.active === true,
    presentation: {
      row: "horizontal-band-with-edge-guides",
      column: "vertical-band-with-edge-guides",
      cell: "cell-fill-and-outline",
      backdrop: "dim-unfocused-cells",
      blockContext: "selected-row-column-block-bands",
    },
  };
}

function normalizeRenderHierarchy(renderHierarchy = null) {
  return {
    kind: "relation-matrix-render-hierarchy",
    matrixRole: renderHierarchy?.matrixRole ?? "primary-relation-matrix",
    matrixRenderPhase: renderHierarchy?.matrixRenderPhase ?? "screen-readable-overlay",
    matrixPostprocessGroup: renderHierarchy?.matrixPostprocessGroup ?? "screen-readable-overlay",
    matrixCameraDof: renderHierarchy?.matrixCameraDof ?? "bypass",
    matrixDepthTest: renderHierarchy?.matrixDepthTest === true ? true : false,
    graphRole: renderHierarchy?.graphRole ?? "supporting-neighborhood-graph",
    graphExpectedRenderPhase: renderHierarchy?.graphExpectedRenderPhase ?? "scene",
    rule: renderHierarchy?.rule ?? "graph-first-matrix-overlay",
  };
}

function metricLawDiagnosticFor(matrix = {}) {
  return matrix?.metricLawDiagnostic
    || matrix?.diagnostics?.metricLawDiagnostic
    || matrix?.metadata?.law_check
    || matrix?.metadata?.lawCheck
    || null;
}

function metricLawReferenceSummary(diagnostic) {
  if (!diagnostic || typeof diagnostic !== "object" || Array.isArray(diagnostic)) return null;
  return {
    checked: diagnostic.checked ?? null,
    operator: diagnostic.operator ?? null,
    finite: diagnostic.finite === true,
    diagonalZero: diagnostic.diagonal_zero ?? diagnostic.diagonalZero ?? null,
    symmetric: diagnostic.symmetric ?? null,
    triangle: diagnostic.triangle ?? null,
    pairCount: integerOrNull(diagnostic.pair_count ?? diagnostic.pairCount),
    triangleTriplets: integerOrNull(diagnostic.triangle_triplets ?? diagnostic.triangleTriplets),
  };
}

function relationMap(source, options = {}) {
  const recordIds = options.recordIds || collectRecordIds(source, options);
  const out = new Map();
  for (const pair of relationPairs(source)) {
    const sourceId = pairSourceId(pair, recordIds);
    const targetId = pairTargetId(pair, recordIds);
    const value = pairValue(pair, options.valueKey);
    if (!sourceId || !targetId || !Number.isFinite(value)) continue;
    out.set(pairKey(sourceId, targetId), { source: sourceId, target: targetId, value, pair });
  }
  return out;
}

function pairKey(source, target) {
  return `${source}\u0000${target}`;
}

function matrixDimensions(matrix) {
  const size = integerOrNull(matrix?.size ?? matrix?.recordIds?.length);
  const width = integerOrNull(matrix?.width ?? size);
  const height = integerOrNull(matrix?.height ?? size);
  return { width, height, size };
}

function blockDiagnostics(matrix, readability, dimensions) {
  const rawRanges = Array.isArray(readability?.blocks?.ranges)
    ? readability.blocks.ranges
    : Array.isArray(matrix?.blockRanges)
      ? matrix.blockRanges
      : [];
  const size = integerOrNull(dimensions?.size) ?? integerOrNull(matrix?.size) ?? 0;
  const ranges = rawRanges.map((range) => normalizeBlockRange(range, size)).filter(Boolean);
  const labels = ranges.map((range) => range.label).filter(Boolean);
  const coverage = readability?.blocks?.coverage ?? describeBlockCoverage(ranges, size);

  return {
    kind: "relation-matrix-block-diagnostics",
    count: ranges.length,
    labeledCount: labels.length,
    labels,
    coverage,
    ranges,
  };
}

function normalizeBlockRange(range, size) {
  if (!range || typeof range !== "object" || Array.isArray(range)) return null;
  const start = integerOrNull(range.start);
  const end = integerOrNull(range.endExclusive ?? range.end_exclusive ?? range.end ?? range.stop);
  if (start == null || end == null || start < 0 || end <= start || (size > 0 && end > size)) return null;
  const label = String(range.label ?? range.block_label ?? range.blockLabel ?? range.family ?? "").trim();
  return {
    start,
    endExclusive: end,
    size: end - start,
    unitStart: size > 0 ? start / size : null,
    unitEnd: size > 0 ? end / size : null,
    label,
  };
}

function describeBlockCoverage(ranges, size) {
  if (!ranges.length || size <= 0) {
    return {
      state: "empty",
      coveredRecords: 0,
      uncoveredRecords: Math.max(0, size),
      contiguous: false,
      startsAtZero: false,
      endsAtSize: false,
    };
  }

  let coveredRecords = 0;
  let cursor = 0;
  let contiguous = ranges[0].start === 0;
  for (const range of ranges) {
    if (range.start !== cursor) contiguous = false;
    coveredRecords += range.size;
    cursor = Math.max(cursor, range.endExclusive);
  }

  const startsAtZero = ranges[0].start === 0;
  const endsAtSize = ranges[ranges.length - 1].endExclusive === size;
  const full = contiguous && startsAtZero && endsAtSize && coveredRecords === size;
  return {
    state: full ? "full" : "partial",
    coveredRecords,
    uncoveredRecords: Math.max(0, size - coveredRecords),
    contiguous,
    startsAtZero,
    endsAtSize,
  };
}

function normalizeSelectionState(selection = {}, dimensions = {}) {
  const row = integerOrNull(selection.row);
  const column = integerOrNull(selection.column);
  const rowActive = selection.rowActive === true || (row != null && row >= 0);
  const columnActive = selection.columnActive === true || (column != null && column >= 0);
  const cellActive = selection.active === true || (rowActive && columnActive);
  return {
    row: {
      index: rowActive ? row : null,
      active: rowActive,
    },
    column: {
      index: columnActive ? column : null,
      active: columnActive,
    },
    cell: {
      row: cellActive ? row : null,
      column: cellActive ? column : null,
      active: cellActive,
    },
    state: cellActive ? "cell" : rowActive && !columnActive ? "row" : columnActive && !rowActive ? "column" : "none",
    matrixDimensions: dimensions,
  };
}

function integerOrNull(value) {
  const number = Number(value);
  return Number.isInteger(number) ? number : null;
}
