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

const MAX_BLOCK_BOUNDARIES = 16;
const MAX_BLOCK_RANGES = 16;

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
    this.buffers = {};
    this.texturePayload = null;
    this.vertexCount = 4;
    this.blockBoundaries = new Float32Array(MAX_BLOCK_BOUNDARIES);
    this.blockBoundaryCount = 0;
    this.blockRanges = new Float32Array(MAX_BLOCK_RANGES * 2);
    this.blockRangeCount = 0;
    this.selection = {
      row: -1,
      column: -1,
      rowActive: false,
      columnActive: false,
      active: false,
    };
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
    this.program.setUniform("uRect", rect);
    this.program.setUniform("uAlpha", numberOption(material.alpha, 1));
    this.program.setUniform("uBackground", material.background || [0.02, 0.025, 0.03, 1]);
    this.program.setUniform("uTextureSize", [this.texturePayload.width, this.texturePayload.height]);
    this.program.setUniform("uMatrixPixelSize", matrixPixelSize);
    this.program.setUniform("uCellPixelSize", cellPixelSize);
    this.program.setUniform("uSmoothingCellPixels", numberOption(material.smoothingCellPixels, material.readabilityCellPixels, 4.25));
    this.program.setUniform("uSmoothingStrength", numberOption(material.smoothingStrength, material.valueSmoothing, 0.72));
    this.program.setUniform("uSelectedCell", [
      this.selection.columnActive ? this.selection.column : -1,
      this.selection.rowActive ? this.selection.row : -1,
    ]);
    this.program.setUniform("uSelectionAlpha", numberOption(material.selectionAlpha, 0.38));
    this.program.setUniform("uSelectionCellAlpha", numberOption(material.selectionCellAlpha, 0.72));
    this.program.setUniform("uSelectionOutlineAlpha", numberOption(material.selectionOutlineAlpha, 0.92));
    this.program.setUniform("uSelectionOutlinePixels", numberOption(material.selectionOutlinePixels, 1.35));
    this.program.setUniform("uSelectionColor", material.selectionColor || [1.0, 0.86, 0.42, 1]);
    this.program.setUniform("uBlockRangeCount", this.blockRangeCount);
    this.program.setUniform("uBlockRanges", this.blockRanges);
    this.program.setUniform("uBlockBandColor", material.blockBandColor || [1.0, 0.95, 0.72, 1]);
    this.program.setUniform("uBlockBandAlpha", numberOption(material.blockBandAlpha, 0.055));
    this.program.setUniform("uBlockBoundaryCount", this.blockBoundaryCount);
    this.program.setUniform("uBlockBoundaries", this.blockBoundaries);
    this.program.setUniform("uBlockLineColor", material.blockLineColor || [1.0, 1.0, 1.0, 1]);
    this.program.setUniform("uBlockLineAlpha", numberOption(material.blockLineAlpha, 0.32));
    this.program.setUniform("uBlockLineWidthCells", numberOption(material.blockLineWidthCells, 0.68));
    this.program.setUniform("uOuterBorderAlpha", numberOption(material.outerBorderAlpha, 0.42));
    bindAttribute(gl, this.program, this.capabilities, "aUnitPosition", this.buffers.quad, 2);
    gl.drawArrays(gl.TRIANGLE_STRIP, 0, this.vertexCount);
    restoreDepthWrite(gl);
    return this;
  }

  setSelection(selection = {}) {
    const pair = selection.pair || null;
    const rowId = pair?.rowId ?? pair?.row_id ?? pair?.sourceId ?? pair?.source_id ?? selection.recordId;
    const columnId = pair?.columnId ?? pair?.column_id ?? pair?.targetId ?? pair?.target_id ?? selection.recordId;
    const matrix = this.texturePayload?.matrix || this.descriptor?.metadata?.matrix;
    const row = matrix?.recordIds?.findIndex?.((id) => String(id) === String(rowId)) ?? -1;
    const column = matrix?.recordIds?.findIndex?.((id) => String(id) === String(columnId)) ?? -1;
    const rowActive = row >= 0;
    const columnActive = column >= 0;
    const active = rowActive && columnActive;
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

    const texture = gl.createTexture();
    if (!texture) throw new Error("Unable to create relation matrix texture.");
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

    this.texture = texture;
    this.track({ kind: "texture", handle: texture });
  }

  updateBlockBoundaries(payload) {
    this.blockBoundaries.fill(0);
    this.blockBoundaryCount = 0;
    this.blockRanges.fill(0);
    this.blockRangeCount = 0;
    const matrix = payload?.matrix || null;
    const size = Number(matrix?.size);
    if (!Number.isFinite(size) || size <= 0) return this;

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
    this.buffers = {};
    super.dispose();
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
uniform float uAlpha;
uniform vec4 uBackground;
uniform vec2 uTextureSize;
uniform vec2 uMatrixPixelSize;
uniform vec2 uCellPixelSize;
uniform float uSmoothingCellPixels;
uniform float uSmoothingStrength;
uniform vec2 uSelectedCell;
uniform vec4 uSelectionColor;
uniform float uSelectionAlpha;
uniform float uSelectionCellAlpha;
uniform float uSelectionOutlineAlpha;
uniform float uSelectionOutlinePixels;
uniform int uBlockRangeCount;
uniform vec2 uBlockRanges[16];
uniform vec4 uBlockBandColor;
uniform float uBlockBandAlpha;
uniform int uBlockBoundaryCount;
uniform float uBlockBoundaries[16];
uniform vec4 uBlockLineColor;
uniform float uBlockLineAlpha;
uniform float uBlockLineWidthCells;
uniform float uOuterBorderAlpha;

varying vec2 vUv;
varying vec2 vUnit;

void main() {
  vec2 safeUv = clamp(vUv, vec2(0.0), vec2(0.999999));
  vec2 halfTexel = vec2(0.5) / uTextureSize;
  vec2 cellUv = (floor(safeUv * uTextureSize) + vec2(0.5)) / uTextureSize;
  vec4 exactTexel = texture2D(uMatrixTexture, cellUv);
  vec4 smoothTexel = texture2D(uMatrixTexture, clamp(vUv, halfTexel, vec2(1.0) - halfTexel));
  float minCellPixels = max(0.001, min(uCellPixelSize.x, uCellPixelSize.y));
  float smoothing = clamp((uSmoothingCellPixels - minCellPixels) / max(uSmoothingCellPixels, 0.001), 0.0, 1.0)
    * clamp(uSmoothingStrength, 0.0, 1.0);
  vec4 texel = mix(exactTexel, smoothTexel, smoothing);
  float alpha = texel.a * uAlpha;
  vec3 color = mix(uBackground.rgb, texel.rgb, texel.a);
  vec2 matrixCoord = safeUv * uTextureSize;
  vec2 visibleCell = floor(matrixCoord);
  vec2 cellLocal = fract(matrixCoord);
  float rowSelected = step(0.0, uSelectedCell.y);
  float columnSelected = step(0.0, uSelectedCell.x);
  float rowHit = step(0.5, 1.0 - abs(visibleCell.y - uSelectedCell.y)) * rowSelected;
  float columnHit = step(0.5, 1.0 - abs(visibleCell.x - uSelectedCell.x)) * columnSelected;
  float lineHit = clamp(max(rowHit, columnHit), 0.0, 1.0);
  float cellHit = clamp(rowHit * columnHit, 0.0, 1.0);
  float outlineWidthCells = clamp(uSelectionOutlinePixels / minCellPixels, 0.08, 0.48);
  float edgeDistance = min(min(cellLocal.x, 1.0 - cellLocal.x), min(cellLocal.y, 1.0 - cellLocal.y));
  float selectedCellOutline = cellHit * (1.0 - smoothstep(0.0, outlineWidthCells, edgeDistance));
  float sameBlockHit = 0.0;
  for (int index = 0; index < 16; index++) {
    if (index < uBlockRangeCount) {
      vec2 range = uBlockRanges[index];
      float insideX = step(range.x, safeUv.x) * step(safeUv.x, range.y);
      float insideY = step(range.x, safeUv.y) * step(safeUv.y, range.y);
      sameBlockHit = max(sameBlockHit, insideX * insideY);
    }
  }
  float boundaryHit = 0.0;
  float boundaryWidth = max(0.0006, uBlockLineWidthCells / max(uTextureSize.x, uTextureSize.y));
  for (int index = 0; index < 16; index++) {
    if (index < uBlockBoundaryCount) {
      float boundary = uBlockBoundaries[index];
      float distanceToBoundary = min(abs(vUnit.x - boundary), abs(vUnit.y - boundary));
      boundaryHit = max(boundaryHit, 1.0 - smoothstep(0.0, boundaryWidth, distanceToBoundary));
    }
  }
  float borderWidth = max(0.0008, 1.25 / max(1.0, min(uMatrixPixelSize.x, uMatrixPixelSize.y)));
  float borderDistance = min(min(safeUv.x, 1.0 - safeUv.x), min(safeUv.y, 1.0 - safeUv.y));
  float borderHit = 1.0 - smoothstep(0.0, borderWidth, borderDistance);
  color = mix(color, uBlockBandColor.rgb, sameBlockHit * uBlockBandAlpha);
  color = mix(color, uBlockLineColor.rgb, boundaryHit * uBlockLineAlpha);
  color = mix(color, uSelectionColor.rgb, lineHit * uSelectionAlpha);
  color = mix(color, uSelectionColor.rgb, cellHit * uSelectionCellAlpha);
  color = mix(color, uSelectionColor.rgb, selectedCellOutline * uSelectionOutlineAlpha);
  color = mix(color, uBlockLineColor.rgb, borderHit * uOuterBorderAlpha);
  alpha = max(alpha, boundaryHit * uBlockLineColor.a * uBlockLineAlpha * uAlpha);
  alpha = max(alpha, borderHit * uBlockLineColor.a * uOuterBorderAlpha * uAlpha);
  alpha = max(alpha, (lineHit * 0.44 + cellHit * 0.36 + selectedCellOutline * 0.72) * uAlpha);
  gl_FragColor = vec4(color, alpha);
}
`;
