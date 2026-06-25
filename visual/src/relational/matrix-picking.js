/**
 * Reusable hit-testing for relation-matrix layers.
 *
 * Coordinates are expressed in CSS pixels or normalized viewport space. The
 * helper only reads exported matrix evidence that already exists on a
 * RelationMatrixLayer descriptor; it never computes relation values.
 */
export function createRelationMatrixPicker(input, options = {}) {
  const matrix = resolveMatrix(input);
  const context = resolvePairContext(input, options);
  let rect = normalizeRect(options.rect || input?.geometry?.rect || [0, 0, 1, 1]);
  let canvas = options.canvas || null;

  return {
    get matrix() {
      return matrix;
    },
    get rect() {
      return rect.slice();
    },
    setRect(nextRect) {
      rect = normalizeRect(nextRect);
      return this;
    },
    setCanvas(nextCanvas) {
      canvas = nextCanvas || null;
      return this;
    },
    cellAtNormalizedPoint(x, y) {
      return cellAtNormalizedPoint(matrix, rect, x, y, context);
    },
    cellAtCanvasPoint(x, y, width, height) {
      const bounds = resolveCanvasBounds(canvas, width, height);
      if (!bounds) return null;
      return cellAtNormalizedPoint(matrix, rect, Number(x) / bounds.width, Number(y) / bounds.height, context);
    },
    cellAtClientPoint(clientX, clientY, targetCanvas = canvas) {
      const bounds = resolveClientBounds(targetCanvas);
      if (!bounds) return null;
      return cellAtNormalizedPoint(
        matrix,
        rect,
        (Number(clientX) - bounds.left) / bounds.width,
        (Number(clientY) - bounds.top) / bounds.height,
        context,
      );
    },
    describe() {
      return {
        size: matrix.size,
        recordCount: matrix.recordIds.length,
        relationId: context.relationId,
        relationName: context.relationName,
        rect: rect.slice(),
        blockCount: Array.isArray(matrix.blockRanges) ? matrix.blockRanges.length : 0,
        missingValueCount: Number.isFinite(Number(matrix.missingValueCount)) ? Number(matrix.missingValueCount) : null,
        ordering: matrix.ordering || null,
      };
    },
  };
}

export function pickRelationMatrixCell(input, point, options = {}) {
  const picker = createRelationMatrixPicker(input, options);
  if (point?.normalized) return picker.cellAtNormalizedPoint(point.x, point.y);
  if (point?.client) return picker.cellAtClientPoint(point.x, point.y, options.canvas);
  return picker.cellAtCanvasPoint(point?.x, point?.y, point?.width, point?.height);
}

function cellAtNormalizedPoint(matrix, rect, x, y, context = {}) {
  const nx = Number(x);
  const ny = Number(y);
  if (!Number.isFinite(nx) || !Number.isFinite(ny)) return null;

  const fx = (nx - rect[0]) / rect[2];
  const fy = (ny - rect[1]) / rect[3];
  if (fx < 0 || fx > 1 || fy < 0 || fy > 1) return null;

  const size = matrix.size;
  const column = Math.min(size - 1, Math.max(0, Math.floor(fx * size)));
  const row = Math.min(size - 1, Math.max(0, Math.floor(fy * size)));
  const offset = row * size + column;
  const present = matrix.present ? matrix.present[offset] === 1 : Number.isFinite(matrix.values?.[offset]);
  const evidence = matrix.pairEvidence?.get?.(offset) || null;
  const relationId = evidence?.relationId || context.relationId || null;
  const relationName = evidence?.relationName || context.relationName || null;
  const rowId = matrix.recordIds[row];
  const columnId = matrix.recordIds[column];
  const pairKey = evidence?.pairKey || `${relationId || "relation"}\u0000${rowId}\u0000${columnId}`;
  const pairId = evidence?.pairId || evidence?.id || evidence?.pair?.id || evidence?.pair?.pair_id || evidence?.pair?.pairId || null;
  const publicId = pairId || `${relationId || "relation"}:${rowId}:${columnId}`;

  return {
    id: publicId,
    pairId,
    pairKey,
    relationId,
    relationName,
    row,
    column,
    rowId,
    columnId,
    sourceId: rowId,
    targetId: columnId,
    value: matrix.values?.[offset],
    present,
    offset,
    size,
    pairIdentity: {
      relationId,
      relationName,
      pairId,
      pairKey,
      rowId,
      columnId,
      present,
    },
    properties: evidence?.properties || null,
    pairProperties: evidence?.properties || null,
    nativePair: evidence?.pair || null,
    mirrored: evidence?.mirrored === true,
  };
}

function resolvePairContext(input, options) {
  return {
    relationId: options.relationId
      ?? input?.source?.relationId
      ?? input?.source?.texture?.relationId
      ?? input?.source?.textureData?.relationId
      ?? input?.metadata?.relationId
      ?? input?.metadata?.matrix?.relationId
      ?? input?.texture?.relationId
      ?? input?.relationId
      ?? null,
    relationName: options.relationName
      ?? input?.source?.relationName
      ?? input?.source?.texture?.relationName
      ?? input?.source?.textureData?.relationName
      ?? input?.metadata?.relationName
      ?? input?.metadata?.matrix?.relationName
      ?? input?.texture?.relationName
      ?? input?.relationName
      ?? null,
  };
}

function resolveMatrix(input) {
  const matrix =
    input?.kind === "dense-relation-matrix" ? input :
    input?.metadata?.matrix ||
    input?.source?.texture?.matrix ||
    input?.source?.textureData?.matrix ||
    input?.texture?.matrix ||
    input?.matrix ||
    null;

  if (!matrix || matrix.kind !== "dense-relation-matrix") {
    throw new Error("Relation matrix picker requires dense relation matrix evidence.");
  }
  const size = Number(matrix.size);
  if (!Number.isInteger(size) || size <= 0) {
    throw new Error("Relation matrix picker requires a positive integer matrix size.");
  }
  if (!Array.isArray(matrix.recordIds) || matrix.recordIds.length !== size) {
    throw new Error("Relation matrix picker requires recordIds matching matrix size.");
  }
  if (!matrix.values || matrix.values.length < size * size) {
    throw new Error("Relation matrix picker requires matrix values for every cell.");
  }
  if (matrix.present && matrix.present.length < size * size) {
    throw new Error("Relation matrix picker present mask is smaller than the matrix.");
  }
  return matrix;
}

function normalizeRect(rect) {
  if (!Array.isArray(rect) || rect.length < 4) {
    throw new Error("Relation matrix picker rect must be [x, y, width, height].");
  }
  const normalized = rect.slice(0, 4).map(Number);
  if (!normalized.every(Number.isFinite) || normalized[2] <= 0 || normalized[3] <= 0) {
    throw new Error("Relation matrix picker rect must contain finite positive dimensions.");
  }
  return normalized;
}

function resolveCanvasBounds(canvas, width, height) {
  const explicitWidth = Number(width);
  const explicitHeight = Number(height);
  if (Number.isFinite(explicitWidth) && explicitWidth > 0 && Number.isFinite(explicitHeight) && explicitHeight > 0) {
    return { left: 0, top: 0, width: explicitWidth, height: explicitHeight };
  }
  return resolveClientBounds(canvas);
}

function resolveClientBounds(canvas) {
  const bounds = canvas?.getBoundingClientRect?.();
  if (!bounds) return null;
  const width = Number(bounds.width);
  const height = Number(bounds.height);
  if (!Number.isFinite(width) || width <= 0 || !Number.isFinite(height) || height <= 0) return null;
  return {
    left: Number(bounds.left) || 0,
    top: Number(bounds.top) || 0,
    width,
    height,
  };
}
