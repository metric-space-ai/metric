const DEFAULT_TILE_SUMMARY_LIMIT = 512;

/**
 * Build engine-level readability metadata for a dense relation matrix.
 *
 * The profile is derived only from exported matrix values and masks. It does
 * not infer new relation values, reorder records, or replace the semantic
 * matrix picker.
 *
 * @param {object} matrix Dense relation matrix evidence.
 * @param {object} [options]
 * @returns {object}
 */
export function createRelationMatrixReadabilityProfile(matrix, options = {}) {
  const size = positiveInteger(matrix?.size, 0);
  const width = positiveInteger(matrix?.width, size);
  const height = positiveInteger(matrix?.height, size);
  const cellCount = width * height;
  const presentValueCount = countPresentValues(matrix, cellCount);
  const density = cellCount > 0 ? presentValueCount / cellCount : 0;
  const blockRanges = normalizeBlockRanges(matrix?.blockRanges, size);
  const blockCoverage = describeBlockCoverage(blockRanges, size);
  const tileSize = resolveTileSize(options, size);
  const tiles = summarizeMatrixTiles(matrix, tileSize, options);
  const smoothingCellPixels = finiteOption(options.smoothingCellPixels ?? options.readabilityCellPixels, 4.25);
  const smoothingStrength = finiteOption(options.smoothingStrength ?? options.valueSmoothing, 0.72);
  const lodSmoothingStrength = finiteOption(options.lodSmoothingStrength ?? options.denseLodSmoothingStrength, density >= 0.6 ? 0.46 : 0.28);
  const lodStartCellPixels = finiteOption(options.lodSmoothingStartCellPixels ?? options.lodStartCellPixels, 2.45);
  const lodFullCellPixels = finiteOption(options.lodSmoothingFullCellPixels ?? options.lodFullCellPixels, 0.85);
  const tileSummaryStrength = finiteOption(options.tileSummaryStrength ?? options.tileSummaryLodStrength, density >= 0.6 ? 0.68 : 0.34);

  return {
    kind: "relation-matrix-readability-profile",
    version: 1,
    matrixSize: size,
    width,
    height,
    cellCount,
    presentValueCount,
    density,
    densityClass: density >= 0.85 ? "dense" : density >= 0.35 ? "mixed" : "sparse",
    renderer: {
      primitive: "RelationMatrixLayer",
      path: "webgl-texture-layer",
      webglOnly: true,
      domFallback: false,
      svgFallback: false,
      semanticPicker: "relation-matrix-picking",
      preservesNativePairIdentity: true,
    },
    blocks: {
      kind: "relation-matrix-block-ranges",
      ranges: blockRanges,
      boundaries: blockBoundaries(blockRanges, size),
      labels: blockRanges.map((range) => range.label).filter(Boolean),
      labeledCount: blockRanges.filter((range) => range.label).length,
      coverage: blockCoverage,
      shaderRangeLimit: 16,
      shaderBoundaryLimit: 16,
    },
    tiles,
    lod: {
      kind: "shader-footprint-lod",
      strategy: "cell-footprint-neighborhood-smoothing",
      denseCellSmoothing: {
        enabled: true,
        smoothingCellPixels,
        smoothingStrength,
        lodSmoothingStrength,
        startCellPixels: lodStartCellPixels,
        fullCellPixels: lodFullCellPixels,
        kernel: "weighted-3x3",
      },
      tileSummaryLod: {
        enabled: true,
        strategy: "gpu-tile-summary-texture",
        source: "exported-relation-texture-downsample",
        tileSize,
        tileCount: tiles.count,
        strength: tileSummaryStrength,
      },
      levels: [
        {
          id: "cell-exact",
          role: "cell",
          maxSmoothing: 0,
          minCellPixels: smoothingCellPixels,
        },
        {
          id: "cell-footprint",
          role: "dense-cell-smoothing",
          maxSmoothing: smoothingStrength,
          minCellPixels: lodStartCellPixels,
        },
        {
          id: "neighborhood-lod",
          role: "dense-neighborhood-smoothing",
          maxSmoothing: lodSmoothingStrength,
          fullAtCellPixels: lodFullCellPixels,
          kernel: "weighted-3x3",
        },
        {
          id: "logical-tiles",
          role: "tile-summary-texture",
          tileSize,
          tileCount: tiles.count,
          maxSmoothing: tileSummaryStrength,
        },
      ],
    },
    selection: {
      selectedFeatures: ["row", "column", "cell"],
      row: { source: "pair.rowId|recordId", overlay: "horizontal-band" },
      column: { source: "pair.columnId|recordId", overlay: "vertical-band" },
      cell: { source: "pair.rowId+pair.columnId", overlay: "cell-fill-and-outline" },
    },
  };
}

export function summarizeMatrixTiles(matrix, tileSize, options = {}) {
  const size = positiveInteger(matrix?.size, 0);
  const width = positiveInteger(matrix?.width, size);
  const height = positiveInteger(matrix?.height, size);
  const safeTileSize = Math.max(1, positiveInteger(tileSize, resolveTileSize(options, size)));
  const rows = height > 0 ? Math.ceil(height / safeTileSize) : 0;
  const columns = width > 0 ? Math.ceil(width / safeTileSize) : 0;
  const count = rows * columns;
  const maxSummaries = Math.max(0, positiveInteger(options.maxTileSummaries ?? DEFAULT_TILE_SUMMARY_LIMIT, DEFAULT_TILE_SUMMARY_LIMIT));
  const summaries = [];

  for (let rowTile = 0; rowTile < rows; rowTile += 1) {
    for (let columnTile = 0; columnTile < columns; columnTile += 1) {
      if (summaries.length >= maxSummaries) continue;
      summaries.push(summarizeTile(matrix, {
        rowTile,
        columnTile,
        rowStart: rowTile * safeTileSize,
        rowEnd: Math.min(height, (rowTile + 1) * safeTileSize),
        columnStart: columnTile * safeTileSize,
        columnEnd: Math.min(width, (columnTile + 1) * safeTileSize),
        width,
      }));
    }
  }

  return {
    kind: "relation-matrix-logical-tile-grid",
    tileSize: safeTileSize,
    rows,
    columns,
    count,
    summarizedCount: summaries.length,
    coverage: summaries.length === count ? "full" : "capped",
    rowBoundaries: tileBoundaries(height, safeTileSize),
    columnBoundaries: tileBoundaries(width, safeTileSize),
    summaries,
  };
}

function summarizeTile(matrix, bounds) {
  let presentCount = 0;
  let finiteCount = 0;
  let min = Infinity;
  let max = -Infinity;
  let sum = 0;

  for (let row = bounds.rowStart; row < bounds.rowEnd; row += 1) {
    for (let column = bounds.columnStart; column < bounds.columnEnd; column += 1) {
      const offset = row * bounds.width + column;
      const present = isPresent(matrix, offset);
      if (present) presentCount += 1;
      const value = Number(matrix?.values?.[offset]);
      if (!present || !Number.isFinite(value)) continue;
      finiteCount += 1;
      min = Math.min(min, value);
      max = Math.max(max, value);
      sum += value;
    }
  }

  const cellCount = Math.max(0, bounds.rowEnd - bounds.rowStart) * Math.max(0, bounds.columnEnd - bounds.columnStart);
  return {
    rowTile: bounds.rowTile,
    columnTile: bounds.columnTile,
    rowStart: bounds.rowStart,
    rowEnd: bounds.rowEnd,
    columnStart: bounds.columnStart,
    columnEnd: bounds.columnEnd,
    cellCount,
    presentCount,
    presentRatio: cellCount > 0 ? presentCount / cellCount : 0,
    finiteCount,
    min: finiteCount > 0 ? min : null,
    max: finiteCount > 0 ? max : null,
    mean: finiteCount > 0 ? sum / finiteCount : null,
  };
}

function resolveTileSize(options, size) {
  const explicit = positiveInteger(options.tileSize ?? options.readabilityTileSize ?? options.tile_size, 0);
  if (explicit > 0) return explicit;
  if (size <= 64) return 16;
  if (size <= 256) return 32;
  if (size <= 512) return 64;
  return 128;
}

function countPresentValues(matrix, cellCount) {
  const explicit = positiveInteger(matrix?.presentValueCount, -1);
  if (explicit >= 0) return Math.min(cellCount, explicit);
  let count = 0;
  for (let index = 0; index < cellCount; index += 1) {
    if (isPresent(matrix, index)) count += 1;
  }
  return count;
}

function isPresent(matrix, offset) {
  if (matrix?.present) return matrix.present[offset] === 1;
  return Number.isFinite(Number(matrix?.values?.[offset]));
}

function normalizeBlockRanges(ranges, size) {
  if (!Array.isArray(ranges) || size <= 0) return [];
  const normalized = [];
  for (const range of ranges) {
    const start = positiveInteger(range?.start ?? range?.start_index ?? range?.startIndex, -1);
    const end = positiveInteger(range?.end_exclusive ?? range?.endExclusive ?? range?.end ?? range?.stop, -1);
    if (start < 0 || end <= start || end > size) continue;
    normalized.push({
      start,
      endExclusive: end,
      unitStart: start / size,
      unitEnd: end / size,
      label: range?.block_label ?? range?.blockLabel ?? range?.label ?? range?.family ?? "",
    });
  }
  return normalized.sort((a, b) => a.start - b.start || a.endExclusive - b.endExclusive);
}

function blockBoundaries(ranges, size) {
  if (!size) return [];
  const seen = new Set();
  const boundaries = [];
  for (const range of ranges) {
    addBoundary(boundaries, seen, range.start, size);
    addBoundary(boundaries, seen, range.endExclusive, size);
  }
  return boundaries;
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
    const width = Math.max(0, Math.min(size, range.endExclusive) - Math.max(0, range.start));
    coveredRecords += width;
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

function tileBoundaries(size, tileSize) {
  const boundaries = [];
  for (let index = tileSize; index < size; index += tileSize) {
    boundaries.push({ index, unit: index / size });
  }
  return boundaries;
}

function addBoundary(boundaries, seen, index, size) {
  if (!Number.isInteger(index) || index <= 0 || index >= size || seen.has(index)) return;
  seen.add(index);
  boundaries.push({ index, unit: index / size });
}

function finiteOption(value, fallback) {
  const number = Number(value);
  return Number.isFinite(number) ? number : fallback;
}

function positiveInteger(value, fallback) {
  const number = Number(value);
  return Number.isInteger(number) && number >= 0 ? number : fallback;
}
