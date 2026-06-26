import { BaseLayer } from "../layers/BaseLayer.js";
import {
  bindAttribute,
  configureDrawState,
  createBuffer,
  createProgram,
  restoreDepthWrite,
  trackBuffer,
} from "../layers/gl-utils.js";
import { buildRelationMatrixTextureData } from "./matrix-texture.js";
import { createRelationMatrixReadabilityProfile } from "./matrix-readability.js";
import { createRelationMatrixDiagnostics } from "./diagnostics.js";

const MAX_BLOCK_BOUNDARIES = 32;
const MAX_BLOCK_RANGES = 32;

/**
 * WebGL layer for rendering prebuilt relation matrix texture data.
 *
 * The layer accepts `descriptor.source.texture` produced by
 * buildRelationMatrixTextureData(). If passed raw relation evidence instead,
 * it uses that builder to encode the supplied exported pair values; it does not
 * compute metric relations itself.
 */
export class RelationMatrixLayer extends BaseLayer {
  constructor(descriptor = {}, rendererOrGl = null, options = {}) {
    super(descriptor, rendererOrGl, options);
    this.program = null;
    this.texture = null;
    this.tileSummaryTexture = null;
    this.buffers = {};
    this.texturePayload = null;
    this.tileSummaryPayload = null;
    this.readabilityProfile = this.descriptor?.metadata?.readability || null;
    this.vertexCount = 4;
    this.blockBoundaries = new Float32Array(MAX_BLOCK_BOUNDARIES);
    this.blockBoundaryCount = 0;
    this.blockRanges = new Float32Array(MAX_BLOCK_RANGES * 2);
    this.blockRangeCount = 0;
    this.blockShaderCapacity = {
      rangeLimit: MAX_BLOCK_RANGES,
      boundaryLimit: MAX_BLOCK_BOUNDARIES,
      rangeCount: 0,
      boundaryCount: 0,
      truncatedRangeCount: 0,
      truncatedBoundaryCount: 0,
      truncated: false,
    };
    this.selection = {
      row: -1,
      column: -1,
      rowActive: false,
      columnActive: false,
      active: false,
    };
    this.selectionDetails = null;
  }

  ensureResources() {
    if (!this.gl) return false;
    if (!this.program) {
      this.program = this.track(createProgram(this.gl, "RelationMatrixLayer", MATRIX_VERTEX_SHADER, MATRIX_FRAGMENT_SHADER));
    }
    if (!this.buffers.quad) {
      this.buffers.quad = createBuffer(this.gl, QUAD_VERTICES, this.gl.STATIC_DRAW);
      trackBuffer(this, this.buffers.quad);
    }
    if (this.needsUpload) this.upload();
    return true;
  }

  upload() {
    const payload = resolveTexturePayload(this.source, this.descriptor);
    this.texturePayload = payload;
    this.readabilityProfile = resolveReadabilityProfile(payload, this.descriptor);
    this.replaceTexture(payload);
    this.needsUpload = false;
  }

  render(context = {}) {
    if (this.disposed || this.visible === false || !this.ensureResources() || !this.texture) return this;
    const gl = this.gl;
    const material = this.material || {};
    const geometry = this.geometry || {};
    const rect = geometry.rect || [0, 0, 1, 1];
    const viewportSize = resolveViewportSize(gl, context, this.renderer);
    const matrixPixelSize = [
      Math.max(1, Math.abs(Number(rect[2]) || 0) * viewportSize[0]),
      Math.max(1, Math.abs(Number(rect[3]) || 0) * viewportSize[1]),
    ];
    const cellPixelSize = [
      matrixPixelSize[0] / Math.max(1, this.texturePayload.width),
      matrixPixelSize[1] / Math.max(1, this.texturePayload.height),
    ];

    configureDrawState(gl, material, {
      blend: true,
      cullFace: false,
      depthTest: false,
      depthWrite: false,
    });

    this.program.use();
    this.program.bindTexture("uMatrixTexture", this.texture, 0);
    this.program.bindTexture("uTileSummaryTexture", this.tileSummaryTexture || this.texture, 1);
    this.program.setUniform("uRect", rect);
    this.program.setUniform("uAlpha", numberOption(material.alpha, 1));
    this.program.setUniform("uBackground", material.background || [0.02, 0.025, 0.03, 1]);
    this.program.setUniform("uTextureSize", [this.texturePayload.width, this.texturePayload.height]);
    this.program.setUniform("uMatrixPixelSize", matrixPixelSize);
    this.program.setUniform("uCellPixelSize", cellPixelSize);
    this.program.setUniform("uSmoothingCellPixels", numberOption(material.smoothingCellPixels, material.readabilityCellPixels, 4.25));
    this.program.setUniform("uSmoothingStrength", numberOption(material.smoothingStrength, material.valueSmoothing, 0.72));
    this.program.setUniform("uLodSmoothingStrength", numberOption(material.lodSmoothingStrength, this.readabilityProfile?.lod?.denseCellSmoothing?.lodSmoothingStrength, 0.46));
    this.program.setUniform("uLodSmoothingStartCellPixels", numberOption(material.lodSmoothingStartCellPixels, this.readabilityProfile?.lod?.denseCellSmoothing?.startCellPixels, 2.45));
    this.program.setUniform("uLodSmoothingFullCellPixels", numberOption(material.lodSmoothingFullCellPixels, this.readabilityProfile?.lod?.denseCellSmoothing?.fullCellPixels, 0.85));
    this.program.setUniform("uSelectedCell", [
      this.selection.columnActive ? this.selection.column : -1,
      this.selection.rowActive ? this.selection.row : -1,
    ]);
    this.program.setUniform("uSelectionAlpha", numberOption(material.selectionAlpha, 0.54));
    this.program.setUniform("uSelectionRowAlpha", numberOption(material.selectionRowAlpha, material.selectionAlpha, 0.54));
    this.program.setUniform("uSelectionColumnAlpha", numberOption(material.selectionColumnAlpha, material.selectionAlpha, 0.54));
    this.program.setUniform("uSelectionCellAlpha", numberOption(material.selectionCellAlpha, 0.9));
    this.program.setUniform("uSelectionOutlineAlpha", numberOption(material.selectionOutlineAlpha, 1));
    this.program.setUniform("uSelectionOutlinePixels", numberOption(material.selectionOutlinePixels, 2.2));
    this.program.setUniform("uSelectionColor", material.selectionColor || [1.0, 0.86, 0.42, 1]);
    this.program.setUniform("uSelectionRowColor", material.selectionRowColor || material.selectionColor || [1.0, 0.86, 0.42, 1]);
    this.program.setUniform("uSelectionColumnColor", material.selectionColumnColor || [0.42, 0.66, 1.0, 1]);
    this.program.setUniform("uSelectionCellColor", material.selectionCellColor || material.selectionColor || [1.0, 0.92, 0.56, 1]);
    this.program.setUniform("uFocusBackdropColor", material.focusBackdropColor || material.background || [0.02, 0.025, 0.03, 1]);
    this.program.setUniform("uFocusBackdropAlpha", numberOption(material.focusBackdropAlpha, 0.38));
    this.program.setUniform("uFocusBlockColor", material.focusBlockColor || material.blockBandColor || [1.0, 0.95, 0.72, 1]);
    this.program.setUniform("uFocusBlockAlpha", numberOption(material.focusBlockAlpha, 0.14));
    this.program.setUniform("uBlockRangeCount", this.blockRangeCount);
    this.program.setUniform("uBlockRanges", this.blockRanges);
    this.program.setUniform("uBlockBandColor", material.blockBandColor || [1.0, 0.95, 0.72, 1]);
    this.program.setUniform("uBlockBandAlpha", numberOption(material.blockBandAlpha, 0.11));
    this.program.setUniform("uBlockBoundaryCount", this.blockBoundaryCount);
    this.program.setUniform("uBlockBoundaries", this.blockBoundaries);
    this.program.setUniform("uBlockLineColor", material.blockLineColor || [1.0, 1.0, 1.0, 1]);
    this.program.setUniform("uBlockLineAlpha", numberOption(material.blockLineAlpha, 0.64));
    this.program.setUniform("uBlockLineWidthCells", numberOption(material.blockLineWidthCells, 1.15));
    this.program.setUniform("uOuterBorderAlpha", numberOption(material.outerBorderAlpha, 0.7));
    this.program.setUniform("uTileSize", numberOption(material.tileSize, this.readabilityProfile?.tiles?.tileSize, 0));
    this.program.setUniform("uTileSummaryGridSize", [
      this.tileSummaryPayload?.width || 0,
      this.tileSummaryPayload?.height || 0,
    ]);
    this.program.setUniform(
      "uTileSummaryStrength",
      numberOption(material.tileSummaryStrength, this.readabilityProfile?.lod?.tileSummaryLod?.strength, 0.68),
    );
    this.program.setUniform("uTileBoundaryAlpha", numberOption(material.tileBoundaryAlpha, 0.2));
    this.program.setUniform("uTileBoundaryWidthCells", numberOption(material.tileBoundaryWidthCells, 0.5));
    this.program.setUniform("uTileBoundaryColor", material.tileBoundaryColor || [0.72, 0.84, 1.0, 1]);
    bindAttribute(gl, this.program, this.capabilities, "aUnitPosition", this.buffers.quad, 2);
    gl.drawArrays(gl.TRIANGLE_STRIP, 0, this.vertexCount);
    restoreDepthWrite(gl);
    return this;
  }

  setSelection(selection = {}) {
    const input = selection || {};
    const pair = input.pair || null;
    const rowId = pair?.rowId ?? pair?.row_id ?? pair?.sourceId ?? pair?.source_id
      ?? input.rowId ?? input.row_id ?? input.sourceId ?? input.source_id ?? input.recordId;
    const columnId = pair?.columnId ?? pair?.column_id ?? pair?.targetId ?? pair?.target_id
      ?? input.columnId ?? input.column_id ?? input.targetId ?? input.target_id ?? input.recordId;
    const matrix = this.texturePayload?.matrix || this.descriptor?.metadata?.matrix;
    const row = findMatrixIndex(matrix, rowId, pair?.row ?? input.row);
    const column = findMatrixIndex(matrix, columnId, pair?.column ?? input.column);
    const rowActive = row >= 0;
    const columnActive = column >= 0;
    const active = rowActive && columnActive;
    this.selectionDetails = createSelectionDetails(input, {
      matrix,
      row,
      column,
      rowActive,
      columnActive,
      active,
      relationId: this.source?.relationId || this.metadata?.relationId || matrix?.relationId || null,
      relationName: this.source?.relationName || this.metadata?.relationName || matrix?.relationName || null,
    });
    if (
      this.selection.row === row
      && this.selection.column === column
      && this.selection.rowActive === rowActive
      && this.selection.columnActive === columnActive
      && this.selection.active === active
    ) {
      return this;
    }
    this.selection = { row, column, rowActive, columnActive, active };
    return this;
  }

  replaceTexture(payload) {
    const gl = this.gl;
    this.updateBlockBoundaries(payload);
    if (this.texture) {
      gl.deleteTexture(this.texture);
      this.resources = this.resources.filter((resource) => resource.handle !== this.texture);
    }
    if (this.tileSummaryTexture) {
      gl.deleteTexture(this.tileSummaryTexture);
      this.resources = this.resources.filter((resource) => resource.handle !== this.tileSummaryTexture);
    }

    const tileSummaryPayload = buildTileSummaryTexturePayload(payload, this.readabilityProfile);
    const texture = createRgbaTexture(gl, payload, "Unable to create relation matrix texture.");
    const tileSummaryTexture = createRgbaTexture(gl, tileSummaryPayload, "Unable to create relation matrix tile summary texture.");
    this.texture = texture;
    this.tileSummaryTexture = tileSummaryTexture;
    this.tileSummaryPayload = tileSummaryPayload;
    this.track({ kind: "texture", handle: texture });
    this.track({ kind: "texture", handle: tileSummaryTexture });
  }

  updateBlockBoundaries(payload) {
    this.blockBoundaries.fill(0);
    this.blockBoundaryCount = 0;
    this.blockRanges.fill(0);
    this.blockRangeCount = 0;
    this.readabilityProfile = this.readabilityProfile || resolveReadabilityProfile(payload, this.descriptor);
    const matrix = payload?.matrix || null;
    const size = Number(matrix?.size);
    if (!Number.isFinite(size) || size <= 0) return this;
    const blockShaderInput = collectBlockShaderInputStats(matrix.blockRanges || [], size);
    this.blockShaderCapacity = {
      rangeLimit: MAX_BLOCK_RANGES,
      boundaryLimit: MAX_BLOCK_BOUNDARIES,
      rangeCount: blockShaderInput.rangeCount,
      boundaryCount: blockShaderInput.boundaryCount,
      truncatedRangeCount: Math.max(0, blockShaderInput.rangeCount - MAX_BLOCK_RANGES),
      truncatedBoundaryCount: Math.max(0, blockShaderInput.boundaryCount - MAX_BLOCK_BOUNDARIES),
      truncated: blockShaderInput.rangeCount > MAX_BLOCK_RANGES || blockShaderInput.boundaryCount > MAX_BLOCK_BOUNDARIES,
    };

    const seen = new Set();
    for (const range of matrix.blockRanges || []) {
      const start = Number(range.start ?? range.start_index ?? range.startIndex);
      const end = Number(range.end_exclusive ?? range.endExclusive ?? range.end);
      if (!Number.isFinite(start) || !Number.isFinite(end) || end <= start || start < 0 || end > size) continue;
      if (this.blockRangeCount < MAX_BLOCK_RANGES) {
        const offset = this.blockRangeCount * 2;
        this.blockRanges[offset] = start / size;
        this.blockRanges[offset + 1] = end / size;
        this.blockRangeCount += 1;
      }
      addBlockBoundary(this.blockBoundaries, seen, start, size, this);
      addBlockBoundary(this.blockBoundaries, seen, end, size, this);
    }
    return this;
  }

  dispose() {
    this.program = null;
    this.texture = null;
    this.tileSummaryTexture = null;
    this.tileSummaryPayload = null;
    this.buffers = {};
    super.dispose();
  }

  describeReadability() {
    const profile = this.readabilityProfile || this.descriptor?.metadata?.readability || null;
    const diagnostics = this.getDiagnostics();
    return {
      primitive: "RelationMatrixLayer",
      path: "webgl-texture-layer",
      semanticPicker: "relation-matrix-picking",
      domFallback: false,
      svgFallback: false,
      selection: { ...this.selection },
      selected: diagnostics.selected,
      selectedPair: diagnostics.selectedPair,
      linkedGraph: diagnostics.linkedGraph,
      readability: profile,
      diagnostics,
      tileSummary: this.tileSummaryPayload ? {
        width: this.tileSummaryPayload.width,
        height: this.tileSummaryPayload.height,
        tileSize: this.tileSummaryPayload.tileSize,
        source: this.tileSummaryPayload.source,
      } : null,
      shaderCapacity: { ...this.blockShaderCapacity },
    };
  }

  getDiagnostics() {
    const profile = this.readabilityProfile || this.descriptor?.metadata?.readability || null;
    const matrix = this.texturePayload?.matrix || this.descriptor?.metadata?.matrix || null;
    const diagnostics = createRelationMatrixDiagnostics({
      matrix,
      readability: profile,
      selection: this.selection,
      tileSummary: this.tileSummaryPayload,
      relationId: this.source?.relationId || this.metadata?.relationId || matrix?.relationId || null,
      relationName: this.source?.relationName || this.metadata?.relationName || matrix?.relationName || null,
    });
    return {
      ...diagnostics,
      selectedFocus: this.selectionDetails,
      selectedPair: this.selectionDetails?.kind === "pair" ? this.selectionDetails : null,
      linkedGraph: this.selectionDetails?.linkedGraph || null,
      shaderCapacity: { ...this.blockShaderCapacity },
    };
  }
}

function resolveTexturePayload(source = {}, descriptor = {}) {
  if (source.texture?.kind === "relation-matrix-texture-data") return source.texture;
  if (source.textureData?.kind === "relation-matrix-texture-data") return source.textureData;
  if (source.kind === "relation-matrix-texture-data") return source;
  if (source.relations || source.pairs || Array.isArray(source)) {
    return buildRelationMatrixTextureData(source, descriptor.metadata?.matrixOptions || {});
  }
  throw new Error("RelationMatrixLayer requires relation matrix texture data or exported relation pairs.");
}

function createRgbaTexture(gl, payload, errorMessage) {
  const texture = gl.createTexture();
  if (!texture) throw new Error(errorMessage);
  gl.bindTexture(gl.TEXTURE_2D, texture);
  gl.pixelStorei(gl.UNPACK_ALIGNMENT, 1);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
  gl.texImage2D(
    gl.TEXTURE_2D,
    0,
    gl.RGBA,
    payload.width,
    payload.height,
    0,
    gl.RGBA,
    gl.UNSIGNED_BYTE,
    payload.data,
  );
  return texture;
}

function buildTileSummaryTexturePayload(payload, readabilityProfile) {
  const tileSize = Math.max(1, Math.floor(Number(readabilityProfile?.lod?.tileSummaryLod?.tileSize
    ?? readabilityProfile?.tiles?.tileSize
    ?? 0)));
  const sourceWidth = Math.max(1, Math.floor(Number(payload?.width) || 1));
  const sourceHeight = Math.max(1, Math.floor(Number(payload?.height) || 1));
  const width = tileSize > 0 ? Math.max(1, Math.ceil(sourceWidth / tileSize)) : 1;
  const height = tileSize > 0 ? Math.max(1, Math.ceil(sourceHeight / tileSize)) : 1;
  const data = new Uint8Array(width * height * 4);
  const sourceData = payload?.data || new Uint8Array(sourceWidth * sourceHeight * 4);

  for (let tileY = 0; tileY < height; tileY += 1) {
    for (let tileX = 0; tileX < width; tileX += 1) {
      let red = 0;
      let green = 0;
      let blue = 0;
      let alpha = 0;
      let samples = 0;
      const startX = tileX * tileSize;
      const startY = tileY * tileSize;
      const endX = Math.min(sourceWidth, startX + tileSize);
      const endY = Math.min(sourceHeight, startY + tileSize);
      for (let y = startY; y < endY; y += 1) {
        for (let x = startX; x < endX; x += 1) {
          const sourceOffset = (y * sourceWidth + x) * 4;
          const sourceAlpha = sourceData[sourceOffset + 3];
          red += sourceData[sourceOffset] * sourceAlpha;
          green += sourceData[sourceOffset + 1] * sourceAlpha;
          blue += sourceData[sourceOffset + 2] * sourceAlpha;
          alpha += sourceAlpha;
          samples += 1;
        }
      }
      const targetOffset = (tileY * width + tileX) * 4;
      const colorDenominator = Math.max(1, alpha);
      const alphaDenominator = Math.max(1, samples);
      data[targetOffset] = Math.round(red / colorDenominator);
      data[targetOffset + 1] = Math.round(green / colorDenominator);
      data[targetOffset + 2] = Math.round(blue / colorDenominator);
      data[targetOffset + 3] = Math.round(alpha / alphaDenominator);
    }
  }

  return {
    kind: "relation-matrix-tile-summary-texture-data",
    source: "exported-relation-texture-downsample",
    width,
    height,
    tileSize,
    sourceWidth,
    sourceHeight,
    data,
  };
}

function resolveReadabilityProfile(payload, descriptor = {}) {
  return descriptor?.metadata?.readability
    || payload?.readability
    || createRelationMatrixReadabilityProfile(payload?.matrix, descriptor.metadata?.matrixOptions || {});
}

function createSelectionDetails(selection = {}, context = {}) {
  const matrix = context.matrix || {};
  const pair = selection.pair || null;
  const rowId = stringOrNull(
    pair?.rowId ?? pair?.row_id ?? pair?.sourceId ?? pair?.source_id
    ?? selection.rowId ?? selection.row_id ?? selection.sourceId ?? selection.source_id
    ?? matrix.recordIds?.[context.row],
  );
  const columnId = stringOrNull(
    pair?.columnId ?? pair?.column_id ?? pair?.targetId ?? pair?.target_id
    ?? selection.columnId ?? selection.column_id ?? selection.targetId ?? selection.target_id
    ?? matrix.recordIds?.[context.column],
  );
  const relationId = stringOrNull(
    pair?.relationId ?? pair?.relation_id ?? selection.relationId ?? selection.relation_id
    ?? context.relationId ?? matrix.relationId,
  );
  const relationName = stringOrNull(
    pair?.relationName ?? pair?.relation_name ?? selection.relationName ?? selection.relation_name
    ?? context.relationName ?? matrix.relationName,
  );
  const dimensions = {
    width: integerOrNull(matrix.width ?? matrix.size ?? matrix.recordIds?.length),
    height: integerOrNull(matrix.height ?? matrix.size ?? matrix.recordIds?.length),
    size: integerOrNull(matrix.size ?? matrix.recordIds?.length),
  };
  const offset = context.row >= 0 && context.column >= 0 && dimensions.width > 0
    ? context.row * dimensions.width + context.column
    : -1;
  const value = offset >= 0 ? Number(matrix.values?.[offset]) : Number(pair?.value);
  const present = offset >= 0
    ? matrix.present
      ? matrix.present[offset] === 1
      : Number.isFinite(value)
    : Boolean(pair?.present);
  const linkedGraph = linkedGraphDetails(selection.presentation, { rowId, columnId, relationId });
  const kind = pair ? "pair" : context.rowActive && context.columnActive ? "record-diagonal" : context.rowActive ? "row" : context.columnActive ? "column" : "none";

  if (kind === "none") return null;
  return {
    kind,
    relationId,
    relationName,
    rowId,
    columnId,
    row: context.rowActive ? context.row : null,
    column: context.columnActive ? context.column : null,
    rowActive: context.rowActive === true,
    columnActive: context.columnActive === true,
    cellActive: context.active === true,
    value: Number.isFinite(value) ? value : null,
    present,
    pairKey: stringOrNull(pair?.pairKey ?? pair?.pair_key) || (relationId && rowId && columnId ? `${relationId}\u0000${rowId}\u0000${columnId}` : null),
    nativePairPresent: Boolean(pair?.nativePair || pair?.native_pair || pair?.pair),
    linkedGraph,
  };
}

function linkedGraphDetails(presentation, selected = {}) {
  const graphEdges = Array.isArray(presentation?.graphEdges) ? presentation.graphEdges : [];
  const matching = graphEdges.filter((edge) => graphEdgeMatches(edge, selected));
  return {
    present: matching.length > 0,
    graphEdgeCount: matching.length,
    edgeIds: matching.map((edge) => stringOrNull(edge.edgeId ?? edge.edge_id)).filter(Boolean),
    graphIds: uniqueStrings(matching.map((edge) => stringOrNull(edge.graphId ?? edge.graph_id)).filter(Boolean)),
  };
}

function graphEdgeMatches(edge, selected = {}) {
  if (!edge) return false;
  const relationId = stringOrNull(edge.relationId ?? edge.relation_id);
  if (selected.relationId && relationId && selected.relationId !== relationId) return false;
  const row = stringOrNull(edge.rowId ?? edge.row_id ?? edge.sourceId ?? edge.source_id);
  const column = stringOrNull(edge.columnId ?? edge.column_id ?? edge.targetId ?? edge.target_id);
  if (!selected.rowId || !selected.columnId || !row || !column) return false;
  return (selected.rowId === row && selected.columnId === column)
    || (selected.rowId === column && selected.columnId === row);
}

function findMatrixIndex(matrix, id, fallbackIndex) {
  const index = matrix?.recordIds?.findIndex?.((recordId) => String(recordId) === String(id)) ?? -1;
  if (index >= 0) return index;
  const numeric = Number(fallbackIndex);
  const size = Number(matrix?.size ?? matrix?.recordIds?.length ?? 0);
  return Number.isInteger(numeric) && numeric >= 0 && numeric < size ? numeric : -1;
}

function stringOrNull(value) {
  if (value === undefined || value === null) return null;
  const text = String(value);
  return text.length ? text : null;
}

function integerOrNull(value) {
  const number = Number(value);
  return Number.isInteger(number) ? number : null;
}

function uniqueStrings(values) {
  return Array.from(new Set(values.filter((value) => typeof value === "string" && value.length > 0)));
}

function numberOption(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number)) return number;
  }
  return 0;
}

function resolveViewportSize(gl, context = {}, renderer = null) {
  const size = context.size || renderer?.size || {};
  const width = Number(size.drawingBufferWidth ?? size.width ?? gl?.drawingBufferWidth ?? gl?.canvas?.width ?? 1);
  const height = Number(size.drawingBufferHeight ?? size.height ?? gl?.drawingBufferHeight ?? gl?.canvas?.height ?? 1);
  return [
    Number.isFinite(width) && width > 0 ? width : 1,
    Number.isFinite(height) && height > 0 ? height : 1,
  ];
}

function addBlockBoundary(out, seen, value, size, layer) {
  if (!Number.isFinite(value) || value <= 0 || value >= size || seen.has(value)) return;
  if (layer.blockBoundaryCount >= MAX_BLOCK_BOUNDARIES) return;
  seen.add(value);
  out[layer.blockBoundaryCount] = value / size;
  layer.blockBoundaryCount += 1;
}

function collectBlockShaderInputStats(ranges, size) {
  if (!Array.isArray(ranges) || !Number.isFinite(size) || size <= 0) {
    return { rangeCount: 0, boundaryCount: 0 };
  }
  let rangeCount = 0;
  const boundaries = new Set();
  for (const range of ranges) {
    const start = Number(range.start ?? range.start_index ?? range.startIndex);
    const end = Number(range.end_exclusive ?? range.endExclusive ?? range.end);
    if (!Number.isFinite(start) || !Number.isFinite(end) || end <= start || start < 0 || end > size) continue;
    rangeCount += 1;
    if (start > 0 && start < size) boundaries.add(start);
    if (end > 0 && end < size) boundaries.add(end);
  }
  return { rangeCount, boundaryCount: boundaries.size };
}

const QUAD_VERTICES = new Float32Array([
  0, 0,
  1, 0,
  0, 1,
  1, 1,
]);

export const MATRIX_VERTEX_SHADER = `
attribute vec2 aUnitPosition;

uniform vec4 uRect;

varying vec2 vUv;
varying vec2 vUnit;

void main() {
  vec2 ndc = vec2(
    -1.0 + (uRect.x + aUnitPosition.x * uRect.z) * 2.0,
     1.0 - (uRect.y + aUnitPosition.y * uRect.w) * 2.0
  );
  gl_Position = vec4(ndc, 0.0, 1.0);
  vUv = aUnitPosition;
  vUnit = aUnitPosition;
}
`;

export const MATRIX_FRAGMENT_SHADER = `
precision mediump float;

uniform sampler2D uMatrixTexture;
uniform sampler2D uTileSummaryTexture;
uniform float uAlpha;
uniform vec4 uBackground;
uniform vec2 uTextureSize;
uniform vec2 uMatrixPixelSize;
uniform vec2 uCellPixelSize;
uniform float uSmoothingCellPixels;
uniform float uSmoothingStrength;
uniform float uLodSmoothingStrength;
uniform float uLodSmoothingStartCellPixels;
uniform float uLodSmoothingFullCellPixels;
uniform vec2 uSelectedCell;
uniform vec4 uSelectionColor;
uniform vec4 uSelectionRowColor;
uniform vec4 uSelectionColumnColor;
uniform vec4 uSelectionCellColor;
uniform vec4 uFocusBackdropColor;
uniform float uFocusBackdropAlpha;
uniform vec4 uFocusBlockColor;
uniform float uFocusBlockAlpha;
uniform float uSelectionAlpha;
uniform float uSelectionRowAlpha;
uniform float uSelectionColumnAlpha;
uniform float uSelectionCellAlpha;
uniform float uSelectionOutlineAlpha;
uniform float uSelectionOutlinePixels;
uniform int uBlockRangeCount;
uniform vec2 uBlockRanges[32];
uniform vec4 uBlockBandColor;
uniform float uBlockBandAlpha;
uniform int uBlockBoundaryCount;
uniform float uBlockBoundaries[32];
uniform vec4 uBlockLineColor;
uniform float uBlockLineAlpha;
uniform float uBlockLineWidthCells;
uniform float uOuterBorderAlpha;
uniform float uTileSize;
uniform vec2 uTileSummaryGridSize;
uniform float uTileSummaryStrength;
uniform vec4 uTileBoundaryColor;
uniform float uTileBoundaryAlpha;
uniform float uTileBoundaryWidthCells;

varying vec2 vUv;
varying vec2 vUnit;

void main() {
  vec2 safeUv = clamp(vUv, vec2(0.0), vec2(0.999999));
  vec2 halfTexel = vec2(0.5) / uTextureSize;
  vec2 texelStep = vec2(1.0) / uTextureSize;
  vec2 cellUv = (floor(safeUv * uTextureSize) + vec2(0.5)) / uTextureSize;
  vec4 exactTexel = texture2D(uMatrixTexture, cellUv);
  vec4 smoothTexel = texture2D(uMatrixTexture, clamp(vUv, halfTexel, vec2(1.0) - halfTexel));
  vec4 neighborhoodTexel = exactTexel * 0.24;
  neighborhoodTexel += texture2D(uMatrixTexture, clamp(cellUv + vec2(texelStep.x, 0.0), halfTexel, vec2(1.0) - halfTexel)) * 0.12;
  neighborhoodTexel += texture2D(uMatrixTexture, clamp(cellUv + vec2(-texelStep.x, 0.0), halfTexel, vec2(1.0) - halfTexel)) * 0.12;
  neighborhoodTexel += texture2D(uMatrixTexture, clamp(cellUv + vec2(0.0, texelStep.y), halfTexel, vec2(1.0) - halfTexel)) * 0.12;
  neighborhoodTexel += texture2D(uMatrixTexture, clamp(cellUv + vec2(0.0, -texelStep.y), halfTexel, vec2(1.0) - halfTexel)) * 0.12;
  neighborhoodTexel += texture2D(uMatrixTexture, clamp(cellUv + vec2(texelStep.x, texelStep.y), halfTexel, vec2(1.0) - halfTexel)) * 0.07;
  neighborhoodTexel += texture2D(uMatrixTexture, clamp(cellUv + vec2(-texelStep.x, texelStep.y), halfTexel, vec2(1.0) - halfTexel)) * 0.07;
  neighborhoodTexel += texture2D(uMatrixTexture, clamp(cellUv + vec2(texelStep.x, -texelStep.y), halfTexel, vec2(1.0) - halfTexel)) * 0.07;
  neighborhoodTexel += texture2D(uMatrixTexture, clamp(cellUv + vec2(-texelStep.x, -texelStep.y), halfTexel, vec2(1.0) - halfTexel)) * 0.07;
  float minCellPixels = max(0.001, min(uCellPixelSize.x, uCellPixelSize.y));
  float smoothing = clamp((uSmoothingCellPixels - minCellPixels) / max(uSmoothingCellPixels, 0.001), 0.0, 1.0)
    * clamp(uSmoothingStrength, 0.0, 1.0);
  float lodRange = max(0.001, uLodSmoothingStartCellPixels - uLodSmoothingFullCellPixels);
  float lodSmoothing = clamp((uLodSmoothingStartCellPixels - minCellPixels) / lodRange, 0.0, 1.0)
    * clamp(uLodSmoothingStrength, 0.0, 1.0);
  vec4 texel = mix(exactTexel, smoothTexel, smoothing);
  texel = mix(texel, neighborhoodTexel, lodSmoothing);
  vec2 matrixCoord = safeUv * uTextureSize;
  if (uTileSize > 0.0 && uTileSummaryGridSize.x > 0.0 && uTileSummaryGridSize.y > 0.0) {
    vec2 tileCoord = floor(matrixCoord / uTileSize);
    vec2 tileUv = (tileCoord + vec2(0.5)) / uTileSummaryGridSize;
    vec4 tileTexel = texture2D(uTileSummaryTexture, clamp(tileUv, vec2(0.0), vec2(1.0)));
    float tileSummaryMix = lodSmoothing * clamp(uTileSummaryStrength, 0.0, 1.0);
    texel = mix(texel, tileTexel, tileSummaryMix);
  }
  float alpha = texel.a * uAlpha;
  vec3 color = mix(uBackground.rgb, texel.rgb, texel.a);
  vec2 visibleCell = floor(matrixCoord);
  vec2 cellLocal = fract(matrixCoord);
  float rowSelected = step(0.0, uSelectedCell.y);
  float columnSelected = step(0.0, uSelectedCell.x);
  float rowHit = step(0.5, 1.0 - abs(visibleCell.y - uSelectedCell.y)) * rowSelected;
  float columnHit = step(0.5, 1.0 - abs(visibleCell.x - uSelectedCell.x)) * columnSelected;
  float lineHit = clamp(max(rowHit, columnHit), 0.0, 1.0);
  float cellHit = clamp(rowHit * columnHit, 0.0, 1.0);
  float selectionActive = clamp(max(rowSelected, columnSelected), 0.0, 1.0);
  float focusBackdropHit = selectionActive * (1.0 - lineHit);
  float rowOnlyHit = rowHit * (1.0 - cellHit);
  float columnOnlyHit = columnHit * (1.0 - cellHit);
  float outlineWidthCells = clamp(uSelectionOutlinePixels / minCellPixels, 0.08, 0.48);
  float edgeDistance = min(min(cellLocal.x, 1.0 - cellLocal.x), min(cellLocal.y, 1.0 - cellLocal.y));
  float selectedCellOutline = cellHit * (1.0 - smoothstep(0.0, outlineWidthCells, edgeDistance));
  float sameBlockHit = 0.0;
  float selectedBlockHit = 0.0;
  float selectedRowUnit = (uSelectedCell.y + 0.5) / max(uTextureSize.y, 1.0);
  float selectedColumnUnit = (uSelectedCell.x + 0.5) / max(uTextureSize.x, 1.0);
  for (int index = 0; index < 32; index++) {
    if (index < uBlockRangeCount) {
      vec2 range = uBlockRanges[index];
      float insideX = step(range.x, safeUv.x) * step(safeUv.x, range.y);
      float insideY = step(range.x, safeUv.y) * step(safeUv.y, range.y);
      sameBlockHit = max(sameBlockHit, insideX * insideY);
      float selectedRowInRange = step(range.x, selectedRowUnit) * step(selectedRowUnit, range.y) * rowSelected;
      float selectedColumnInRange = step(range.x, selectedColumnUnit) * step(selectedColumnUnit, range.y) * columnSelected;
      selectedBlockHit = max(selectedBlockHit, max(insideY * selectedRowInRange, insideX * selectedColumnInRange));
    }
  }
  float boundaryHit = 0.0;
  float boundaryWidth = max(0.0006, uBlockLineWidthCells / max(uTextureSize.x, uTextureSize.y));
  for (int index = 0; index < 32; index++) {
    if (index < uBlockBoundaryCount) {
      float boundary = uBlockBoundaries[index];
      float distanceToBoundary = min(abs(vUnit.x - boundary), abs(vUnit.y - boundary));
      boundaryHit = max(boundaryHit, 1.0 - smoothstep(0.0, boundaryWidth, distanceToBoundary));
    }
  }
  float tileBoundaryHit = 0.0;
  if (uTileSize > 0.0) {
    vec2 tileLocal = mod(matrixCoord, uTileSize);
    vec2 tileDistance = min(tileLocal, uTileSize - tileLocal);
    float tileDistanceCells = min(tileDistance.x, tileDistance.y);
    float tileWidthCells = clamp(uTileBoundaryWidthCells, 0.04, 0.95);
    float interior = step(0.5, matrixCoord.x) * step(0.5, matrixCoord.y)
      * step(matrixCoord.x, uTextureSize.x - 0.5) * step(matrixCoord.y, uTextureSize.y - 0.5);
    tileBoundaryHit = (1.0 - smoothstep(0.0, tileWidthCells, tileDistanceCells)) * interior;
  }
  float borderWidth = max(0.0008, 1.25 / max(1.0, min(uMatrixPixelSize.x, uMatrixPixelSize.y)));
  float borderDistance = min(min(safeUv.x, 1.0 - safeUv.x), min(safeUv.y, 1.0 - safeUv.y));
  float borderHit = 1.0 - smoothstep(0.0, borderWidth, borderDistance);
  color = mix(color, uFocusBackdropColor.rgb, focusBackdropHit * uFocusBackdropAlpha);
  color = mix(color, uBlockBandColor.rgb, sameBlockHit * uBlockBandAlpha);
  color = mix(color, uFocusBlockColor.rgb, selectedBlockHit * uFocusBlockAlpha);
  color = mix(color, uTileBoundaryColor.rgb, tileBoundaryHit * uTileBoundaryAlpha);
  color = mix(color, uBlockLineColor.rgb, boundaryHit * uBlockLineAlpha);
  color = mix(color, uSelectionRowColor.rgb, rowOnlyHit * uSelectionRowAlpha);
  color = mix(color, uSelectionColumnColor.rgb, columnOnlyHit * uSelectionColumnAlpha);
  color = mix(color, uSelectionColor.rgb, lineHit * uSelectionAlpha * 0.2);
  color = mix(color, uSelectionCellColor.rgb, cellHit * uSelectionCellAlpha);
  color = mix(color, uSelectionCellColor.rgb, selectedCellOutline * uSelectionOutlineAlpha);
  color = mix(color, uBlockLineColor.rgb, borderHit * uOuterBorderAlpha);
  alpha = max(alpha, tileBoundaryHit * uTileBoundaryColor.a * uTileBoundaryAlpha * uAlpha);
  alpha = max(alpha, selectedBlockHit * uFocusBlockColor.a * uFocusBlockAlpha * uAlpha);
  alpha = max(alpha, boundaryHit * uBlockLineColor.a * uBlockLineAlpha * uAlpha);
  alpha = max(alpha, borderHit * uBlockLineColor.a * uOuterBorderAlpha * uAlpha);
  alpha = max(alpha, (lineHit * 0.44 + cellHit * 0.36 + selectedCellOutline * 0.72) * uAlpha);
  gl_FragColor = vec4(color, alpha);
}
`;
