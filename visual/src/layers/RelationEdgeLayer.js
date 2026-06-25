import { BaseLayer } from "./BaseLayer.js";
import {
  colorChannel,
  getChannel,
  getChannelArray,
  getChannelCount,
  getChannelItemSize,
  positionChannel,
} from "./channels.js";
import {
  bindAttribute,
  configureDrawState,
  createBuffer,
  createProgram,
  restoreDepthWrite,
  safeLineWidth,
  setCameraUniforms,
  trackBuffer,
} from "./gl-utils.js";

export class RelationEdgeLayer extends BaseLayer {
  constructor(descriptor = {}, rendererOrGl = null, options = {}) {
    super(descriptor, rendererOrGl, options);
    this.edgeCount = 0;
    this.vertexCount = 0;
    this.buffers = {};
    this.program = null;
    this.pickProgram = null;
    this.edgeEntries = [];
    this.pickColorRegistry = null;
    this.pickColorRegistrySize = -1;
  }

  ensureResources() {
    if (!this.gl) return false;
    if (!this.program) {
      this.program = this.track(createProgram(this.gl, "RelationEdgeLayer", EDGE_VERTEX_SHADER, EDGE_FRAGMENT_SHADER));
    }
    if (this.needsUpload) this.upload();
    return true;
  }

  upload() {
    const edgeCount = inferEdgeCount(this.channels);
    const positions = buildEdgePositions(this.channels, edgeCount);
    const colors = buildEdgeColors(this.channels, edgeCount);
    this.edgeCount = edgeCount;
    this.vertexCount = edgeCount * 2;
    this.edgeEntries = buildEdgeEntries(this.channels, edgeCount, this.descriptor);
    this.pickColorRegistry = null;
    this.pickColorRegistrySize = -1;
    this.replaceBuffer("position", positions);
    this.replaceBuffer("color", colors);
    this.needsUpload = false;
  }

  render(context = {}) {
    if (this.disposed || this.visible === false || !this.ensureResources() || this.vertexCount <= 0) return this;
    const gl = this.gl;
    const material = this.material || {};

    configureDrawState(gl, material, {
      blend: true,
      cullFace: false,
      depthWrite: material.depthWrite === true,
    });
    safeLineWidth(gl, this.geometry.width || material.width || 1);

    this.program.use();
    setCameraUniforms(this.program, context, this.renderer);
    this.program.setUniform("uGlobalAlpha", numberOption(material.alpha, 1));
    bindAttribute(gl, this.program, this.capabilities, "aPosition", this.buffers.position, 3);
    bindAttribute(gl, this.program, this.capabilities, "aColor", this.buffers.color, 4);
    gl.drawArrays(gl.LINES, 0, this.vertexCount);
    restoreDepthWrite(gl);
    return this;
  }

  renderPicking(context = {}) {
    if (this.disposed || this.visible === false || !this.ensureResources() || this.vertexCount <= 0) return this;
    if (!context.registry || !this.edgeEntries.length) return this;
    const gl = this.gl;
    const material = this.material || {};
    const pickWidth = numberOption(material.pickWidth, this.geometry.pickWidth, this.geometry.width, material.width, 1);

    if (!this.pickProgram) {
      this.pickProgram = this.track(createProgram(gl, "RelationEdgeLayerPicking", EDGE_PICK_VERTEX_SHADER, EDGE_PICK_FRAGMENT_SHADER));
    }
    this.updatePickColorBuffer(context);

    configureDrawState(gl, { depthWrite: true, alphaMode: "opaque" }, {
      blend: false,
      cullFace: false,
      depthWrite: true,
    });
    safeLineWidth(gl, pickWidth);

    this.pickProgram.use();
    setCameraUniforms(this.pickProgram, context, this.renderer);
    bindAttribute(gl, this.pickProgram, this.capabilities, "aPosition", this.buffers.position, 3);
    bindAttribute(gl, this.pickProgram, this.capabilities, "aPickColor", this.buffers.pickColor, 4);
    gl.drawArrays(gl.LINES, 0, this.vertexCount);
    restoreDepthWrite(gl);
    return this;
  }

  updatePickColorBuffer(context = {}) {
    const registry = context.registry;
    const registrySize = registry?.size ?? -1;
    if (this.buffers.pickColor && this.pickColorRegistry === registry && this.pickColorRegistrySize === registrySize) {
      return this;
    }
    const encode = context.encodePickIdRGBAFloat || encodePickIdRGBAFloatLocal;
    const colors = new Float32Array(this.vertexCount * 4);
    const scope = this.descriptor?.id || this.id || this.descriptor?.primitive || null;
    for (let index = 0; index < this.edgeCount; index += 1) {
      const entry = this.edgeEntries[index] || fallbackEdgeEntry(index, this.descriptor);
      const numericId = registry.registerEdge(String(entry.edgeId), {
        scope,
        layerId: this.id,
        index,
        descriptor: this.descriptor,
        payload: entry,
      });
      encode(numericId, colors, index * 8);
      encode(numericId, colors, index * 8 + 4);
    }
    this.replaceBuffer("pickColor", colors);
    this.pickColorRegistry = registry;
    this.pickColorRegistrySize = registry?.size ?? registrySize;
    return this;
  }

  replaceBuffer(name, data) {
    const gl = this.gl;
    if (this.buffers[name]) {
      gl.deleteBuffer(this.buffers[name]);
      this.resources = this.resources.filter((resource) => resource.handle !== this.buffers[name]);
    }
    this.buffers[name] = createBuffer(gl, data, gl.STATIC_DRAW);
    trackBuffer(this, this.buffers[name]);
  }

  dispose() {
    this.buffers = {};
    this.program = null;
    this.pickProgram = null;
    super.dispose();
  }
}

function inferEdgeCount(channels) {
  const source = getChannel(channels, ["sourcePosition", "source"]);
  const target = getChannel(channels, ["targetPosition", "target"]);
  if (source && target) return Math.min(getChannelCount(source, 3), getChannelCount(target, 3));
  const position = getChannel(channels, "position");
  const itemSize = getChannelItemSize(position, 3);
  if (itemSize >= 6) return getChannelCount(position, itemSize);
  return Math.floor(getChannelCount(position, 3) / 2);
}

function buildEdgePositions(channels, edgeCount) {
  const source = getChannel(channels, ["sourcePosition", "source"]);
  const target = getChannel(channels, ["targetPosition", "target"]);
  const out = new Float32Array(edgeCount * 2 * 3);
  if (source && target) {
    const sources = positionChannel(channels, ["sourcePosition", "source"], edgeCount);
    const targets = positionChannel(channels, ["targetPosition", "target"], edgeCount);
    for (let index = 0; index < edgeCount; index += 1) {
      out.set(sources.subarray(index * 3, index * 3 + 3), index * 6);
      out.set(targets.subarray(index * 3, index * 3 + 3), index * 6 + 3);
    }
    return out;
  }

  const position = getChannel(channels, "position");
  const array = getChannelArray(position);
  const itemSize = getChannelItemSize(position, 3);
  if (!array) return out;
  for (let index = 0; index < edgeCount; index += 1) {
    const targetOffset = index * 6;
    if (itemSize >= 6) {
      const sourceOffset = index * itemSize;
      for (let component = 0; component < 6; component += 1) {
        out[targetOffset + component] = Number(array[sourceOffset + component]) || 0;
      }
    } else {
      const sourceOffset = index * 6;
      for (let component = 0; component < 6; component += 1) {
        out[targetOffset + component] = Number(array[sourceOffset + component]) || 0;
      }
    }
  }
  return out;
}

function buildEdgeColors(channels, edgeCount) {
  const perEdge = colorChannel(channels, edgeCount, { defaultColor: [0.18, 0.30, 0.38, 0.42] });
  const out = new Float32Array(edgeCount * 2 * 4);
  for (let index = 0; index < edgeCount; index += 1) {
    const color = perEdge.subarray(index * 4, index * 4 + 4);
    out.set(color, index * 8);
    out.set(color, index * 8 + 4);
  }
  return out;
}

function buildEdgeEntries(channels, edgeCount, descriptor = {}) {
  const graph = descriptor.metadata?.graph || descriptor.source?.graph || null;
  const graphEdges = Array.isArray(graph?.edges) ? graph.edges : [];
  const values = getChannelArray(getChannel(channels, ["relationValue", "value", "weight"]));
  const edgeIds = getChannelArray(getChannel(channels, ["edgeId", "edge_id", "id"]));
  const sourceIds = getChannelArray(getChannel(channels, ["sourceId", "source_id", "rowId", "row_id"]));
  const targetIds = getChannelArray(getChannel(channels, ["targetId", "target_id", "columnId", "column_id"]));
  const relationId = descriptor.source?.relationId || graph?.relationId || graph?.edge_relation_id || descriptor.metadata?.relationId || null;
  const graphId = descriptor.source?.graphId || graph?.id || descriptor.metadata?.graphId || null;
  const entries = new Array(edgeCount);

  for (let index = 0; index < edgeCount; index += 1) {
    const edge = graphEdges[index] || {};
    const rowId = edge.source ?? edge.rowId ?? edge.row_id ?? edge.source_id ?? edge.a ?? sourceIds?.[index] ?? null;
    const columnId = edge.target ?? edge.columnId ?? edge.column_id ?? edge.target_id ?? edge.b ?? targetIds?.[index] ?? null;
    const edgeId = edgeIds?.[index]
      ?? edge.id
      ?? edge.edgeId
      ?? edge.edge_id
      ?? (rowId != null && columnId != null
        ? `${relationId || graphId || descriptor.id || "edge"}:${rowId}:${columnId}:${index}`
        : `${descriptor.id || descriptor.primitive || "edge"}:${index}`);
    entries[index] = {
      relationId,
      graphId,
      edgeId: String(edgeId),
      rowId: rowId == null ? null : String(rowId),
      columnId: columnId == null ? null : String(columnId),
      sourceId: rowId == null ? null : String(rowId),
      targetId: columnId == null ? null : String(columnId),
      value: edge.value ?? values?.[index] ?? null,
      present: edge.present !== false,
      layerId: descriptor.id || null,
      index,
    };
  }

  return entries;
}

function fallbackEdgeEntry(index, descriptor = {}) {
  return {
    edgeId: `${descriptor.id || descriptor.primitive || "edge"}:${index}`,
    layerId: descriptor.id || null,
    index,
    present: true,
  };
}

function encodePickIdRGBAFloatLocal(id, out = [0, 0, 0, 0], offset = 0) {
  const numericId = Math.max(0, Math.floor(Number(id) || 0));
  out[offset] = (numericId % 256) / 255;
  out[offset + 1] = (Math.floor(numericId / 256) % 256) / 255;
  out[offset + 2] = (Math.floor(numericId / 65536) % 256) / 255;
  out[offset + 3] = (Math.floor(numericId / 16777216) % 256) / 255;
  return out;
}

function numberOption(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number)) return number;
  }
  return 0;
}

const EDGE_VERTEX_SHADER = `
attribute vec3 aPosition;
attribute vec4 aColor;

uniform mat4 uViewProjectionMatrix;

varying vec4 vColor;

void main() {
  gl_Position = uViewProjectionMatrix * vec4(aPosition, 1.0);
  vColor = aColor;
}
`;

const EDGE_PICK_VERTEX_SHADER = `
attribute vec3 aPosition;
attribute vec4 aPickColor;

uniform mat4 uViewProjectionMatrix;

varying vec4 vPickColor;

void main() {
  gl_Position = uViewProjectionMatrix * vec4(aPosition, 1.0);
  vPickColor = aPickColor;
}
`;

const EDGE_PICK_FRAGMENT_SHADER = `
precision mediump float;

varying vec4 vPickColor;

void main() {
  gl_FragColor = vPickColor;
}
`;

const EDGE_FRAGMENT_SHADER = `
precision mediump float;

uniform float uGlobalAlpha;
varying vec4 vColor;

void main() {
  gl_FragColor = vec4(vColor.rgb, vColor.a * uGlobalAlpha);
}
`;
