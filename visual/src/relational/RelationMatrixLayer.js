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
    this.selection = {
      row: -1,
      column: -1,
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

  render() {
    if (this.disposed || this.visible === false || !this.ensureResources() || !this.texture) return this;
    const gl = this.gl;
    const material = this.material || {};
    const geometry = this.geometry || {};
    const rect = geometry.rect || [0, 0, 1, 1];

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
    this.program.setUniform("uSelectedCell", [
      this.selection.active ? this.selection.column : -1,
      this.selection.active ? this.selection.row : -1,
    ]);
    this.program.setUniform("uSelectionAlpha", numberOption(material.selectionAlpha, 0.32));
    this.program.setUniform("uSelectionCellAlpha", numberOption(material.selectionCellAlpha, 0.64));
    this.program.setUniform("uSelectionColor", material.selectionColor || [1.0, 0.86, 0.42, 1]);
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
    const active = row >= 0 && column >= 0;
    if (this.selection.row === row && this.selection.column === column && this.selection.active === active) return this;
    this.selection = { row, column, active };
    return this;
  }

  replaceTexture(payload) {
    const gl = this.gl;
    if (this.texture) {
      gl.deleteTexture(this.texture);
      this.resources = this.resources.filter((resource) => resource.handle !== this.texture);
    }

    const texture = gl.createTexture();
    if (!texture) throw new Error("Unable to create relation matrix texture.");
    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.pixelStorei(gl.UNPACK_ALIGNMENT, 1);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
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
  vUv = vec2(aUnitPosition.x, 1.0 - aUnitPosition.y);
  vUnit = aUnitPosition;
}
`;

export const MATRIX_FRAGMENT_SHADER = `
precision mediump float;

uniform sampler2D uMatrixTexture;
uniform float uAlpha;
uniform vec4 uBackground;
uniform vec2 uTextureSize;
uniform vec2 uSelectedCell;
uniform vec4 uSelectionColor;
uniform float uSelectionAlpha;
uniform float uSelectionCellAlpha;

varying vec2 vUv;
varying vec2 vUnit;

void main() {
  vec2 cellUv = (floor(vUv * uTextureSize) + vec2(0.5)) / uTextureSize;
  vec4 texel = texture2D(uMatrixTexture, cellUv);
  float alpha = texel.a * uAlpha;
  vec3 color = mix(uBackground.rgb, texel.rgb, texel.a);
  vec2 visibleCell = floor(vUnit * uTextureSize);
  float rowHit = step(0.5, 1.0 - abs(visibleCell.y - uSelectedCell.y));
  float columnHit = step(0.5, 1.0 - abs(visibleCell.x - uSelectedCell.x));
  float hasSelection = step(0.0, uSelectedCell.x) * step(0.0, uSelectedCell.y);
  float lineHit = clamp(max(rowHit, columnHit) * hasSelection, 0.0, 1.0);
  float cellHit = clamp(rowHit * columnHit * hasSelection, 0.0, 1.0);
  color = mix(color, uSelectionColor.rgb, lineHit * uSelectionAlpha);
  color = mix(color, uSelectionColor.rgb, cellHit * uSelectionCellAlpha);
  alpha = max(alpha, (lineHit * 0.42 + cellHit * 0.58) * uAlpha);
  gl_FragColor = vec4(color, alpha);
}
`;
