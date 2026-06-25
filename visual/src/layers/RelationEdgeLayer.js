import { BaseLayer } from "./BaseLayer.js";
import {
  colorChannel,
  combineScalarChannels,
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
    this.baseEmphasis = new Float32Array();
    this.edgeEmphasis = new Float32Array();
    this.selection = null;
    this.selectionActive = false;
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
    this.baseEmphasis = buildBaseEdgeEmphasis(this.channels, edgeCount);
    this.edgeEmphasis = this.selectionEmphasisData();
    const emphasis = buildVertexEdgeEmphasis(this.edgeEmphasis);
    this.pickColorRegistry = null;
    this.pickColorRegistrySize = -1;
    this.replaceBuffer("position", positions);
    this.replaceBuffer("color", colors);
    this.replaceBuffer("emphasis", emphasis);
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
    this.program.setUniform("uSelectionActive", this.selectionActive ? 1 : 0);
    this.program.setUniform("uEmphasisStrength", numberOption(material.emphasisStrength, 0.42));
    bindAttribute(gl, this.program, this.capabilities, "aPosition", this.buffers.position, 3);
    bindAttribute(gl, this.program, this.capabilities, "aColor", this.buffers.color, 4);
    bindAttribute(gl, this.program, this.capabilities, "aEdgeEmphasis", this.buffers.emphasis, 1);
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

  setSelection(selection = {}) {
    this.selection = selection || null;
    this.updateEmphasisBuffer();
    return this;
  }

  clearSelection() {
    return this.setSelection(null);
  }

  selectionEmphasisData() {
    const emphasis = new Float32Array(this.edgeCount);
    if (this.baseEmphasis.length) emphasis.set(this.baseEmphasis.subarray(0, this.edgeCount));
    const matcher = createSelectionMatcher(this.selection, this.descriptor);
    this.selectionActive = matcher.active;
    if (!matcher.active || !this.edgeEntries.length) return emphasis;

    for (let index = 0; index < Math.min(this.edgeEntries.length, emphasis.length); index += 1) {
      const score = edgeSelectionEmphasis(this.edgeEntries[index], matcher, this.descriptor);
      if (score > emphasis[index]) emphasis[index] = score;
    }
    return emphasis;
  }

  updateEmphasisBuffer() {
    this.edgeEmphasis = this.selectionEmphasisData();
    if (!this.gl || !this.buffers.emphasis || this.needsUpload) {
      this.needsUpload = true;
      return this;
    }
    this.replaceBuffer("emphasis", buildVertexEdgeEmphasis(this.edgeEmphasis));
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

function buildBaseEdgeEmphasis(channels, edgeCount) {
  return combineScalarChannels(channels, ["edgeEmphasis", "emphasis", "focusWeight", "focus", "selection"], edgeCount, 0);
}

function buildVertexEdgeEmphasis(perEdge) {
  const out = new Float32Array(perEdge.length * 2);
  for (let index = 0; index < perEdge.length; index += 1) {
    const value = clamp01(perEdge[index]);
    out[index * 2] = value;
    out[index * 2 + 1] = value;
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
  const pairSetId = descriptor.source?.pairSetId || descriptor.metadata?.selectionModel?.pairSetId || graph?.id || null;
  const isBridge = isPairedSpaceBridgeDescriptor(descriptor, graph);
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
      row: integerOrNull(edge.sourceIndex ?? edge.row),
      column: integerOrNull(edge.targetIndex ?? edge.column),
      pairId: edge.pairId ?? edge.pair_id ?? null,
      pairSetId,
      sourceSpaceId: edge.sourceSpaceId ?? edge.source_space_id ?? null,
      targetSpaceId: edge.targetSpaceId ?? edge.target_space_id ?? null,
      sourceCoordinateId: edge.sourceCoordinateId ?? edge.source_coordinate_id ?? null,
      targetCoordinateId: edge.targetCoordinateId ?? edge.target_coordinate_id ?? null,
      isBridge,
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

function createSelectionMatcher(selection = null, descriptor = {}) {
  const pair = normalizeSelectionPair(selection?.pair || selection);
  const selectedRecordId = stringOrNull(selection?.recordId ?? selection?.record_id ?? selection?.selectedRecordId ?? selection?.selected_record_id);
  const edgeId = stringOrNull(selection?.edgeId ?? selection?.edge_id);
  const rowId = stringOrNull(selection?.rowId ?? selection?.row_id ?? selection?.sourceId ?? selection?.source_id);
  const columnId = stringOrNull(selection?.columnId ?? selection?.column_id ?? selection?.targetId ?? selection?.target_id);
  const presentation = selection?.presentation || null;
  const features = collectPresentationEdgeFeatures(presentation, descriptor);
  const highlightedEdgeIds = new Set([
    ...toStringArray(presentation?.highlight?.graphEdgeIds),
    ...toStringArray(presentation?.highlight?.pairedSpaceBridgeIds),
  ]);
  const active = Boolean(
    selectedRecordId
    || edgeId
    || rowId
    || columnId
    || pair
    || features.length
    || highlightedEdgeIds.size,
  );
  return { active, pair, selectedRecordId, edgeId, rowId, columnId, features, highlightedEdgeIds };
}

function collectPresentationEdgeFeatures(presentation, descriptor = {}) {
  const out = [];
  for (const feature of [
    ...toArray(presentation?.graphEdges),
    ...toArray(presentation?.pairedSpaceBridges),
    ...toArray(presentation?.features).filter((entry) => entry?.feature === "graph-edge" || entry?.feature === "paired-space-bridge"),
  ]) {
    if (!feature || !featureMatchesLayer(feature, descriptor)) continue;
    out.push(feature);
  }
  return out;
}

function edgeSelectionEmphasis(edge, matcher, descriptor = {}) {
  if (!edge) return 0;
  let score = 0;
  if (matcher.edgeId && matcher.edgeId === stringOrNull(edge.edgeId)) score = Math.max(score, 1);
  if (matcher.pair && pairMatchesEdge(matcher.pair, edge)) score = Math.max(score, 1);
  if (matcher.rowId && matcher.columnId && idsMatchPair(matcher.rowId, matcher.columnId, edge)) score = Math.max(score, 1);
  if (matcher.selectedRecordId && idMatchesAny(matcher.selectedRecordId, edge.rowId, edge.columnId, edge.sourceId, edge.targetId)) {
    score = Math.max(score, 0.68);
  }

  for (const feature of matcher.features) {
    if (!featureMatchesEdge(feature, edge, descriptor)) continue;
    const kind = feature.selectionMatch?.kind || feature.selection_match?.kind;
    score = Math.max(score, kind === "pair" ? 1 : kind === "record-endpoint" ? 0.68 : 0.86);
  }

  if (score === 0 && matcher.highlightedEdgeIds.has(stringOrNull(edge.edgeId))) score = 0.86;
  return score;
}

function featureMatchesLayer(feature, descriptor = {}) {
  const layerId = stringOrNull(feature.layerId ?? feature.layer_id ?? feature.descriptorId ?? feature.descriptor_id);
  if (!layerId) return true;
  return layerId === stringOrNull(descriptor.id);
}

function featureMatchesEdge(feature, edge, descriptor = {}) {
  if (!featureMatchesLayer(feature, descriptor)) return false;
  if (feature.index != null && integerOrNull(feature.index) === edge.index) return true;
  const featureEdgeId = stringOrNull(feature.edgeId ?? feature.edge_id);
  if (featureEdgeId && featureEdgeId === stringOrNull(edge.edgeId)) return true;
  const featurePairId = stringOrNull(feature.pairId ?? feature.pair_id);
  if (featurePairId && featurePairId === stringOrNull(edge.pairId)) return true;
  const rowId = stringOrNull(feature.rowId ?? feature.row_id ?? feature.sourceId ?? feature.source_id);
  const columnId = stringOrNull(feature.columnId ?? feature.column_id ?? feature.targetId ?? feature.target_id);
  if (rowId && columnId && idsMatchPair(rowId, columnId, edge)) return true;
  return false;
}

function normalizeSelectionPair(pair) {
  if (!pair || typeof pair !== "object") return null;
  const rowId = stringOrNull(pair.rowId ?? pair.row_id ?? pair.sourceId ?? pair.source_id);
  const columnId = stringOrNull(pair.columnId ?? pair.column_id ?? pair.targetId ?? pair.target_id);
  const edgeId = stringOrNull(pair.edgeId ?? pair.edge_id);
  const pairId = stringOrNull(pair.pairId ?? pair.pair_id);
  if (!rowId && !columnId && !edgeId && !pairId) return null;
  return {
    relationId: stringOrNull(pair.relationId ?? pair.relation_id),
    graphId: stringOrNull(pair.graphId ?? pair.graph_id),
    pairSetId: stringOrNull(pair.pairSetId ?? pair.pair_set_id),
    edgeId,
    pairId,
    rowId,
    columnId,
  };
}

function pairMatchesEdge(pair, edge) {
  if (pair.edgeId && pair.edgeId === stringOrNull(edge.edgeId)) return true;
  if (pair.pairId && pair.pairId === stringOrNull(edge.pairId)) return true;
  if (pair.graphId && edge.graphId != null && pair.graphId !== stringOrNull(edge.graphId)) return false;
  if (pair.pairSetId && edge.pairSetId != null && pair.pairSetId !== stringOrNull(edge.pairSetId)) return false;
  if (pair.relationId && edge.relationId != null && pair.relationId !== stringOrNull(edge.relationId)) return false;
  return Boolean(pair.rowId && pair.columnId && idsMatchPair(pair.rowId, pair.columnId, edge));
}

function idsMatchPair(rowId, columnId, edge) {
  const row = stringOrNull(edge.rowId ?? edge.sourceId);
  const column = stringOrNull(edge.columnId ?? edge.targetId);
  return (rowId === row && columnId === column) || (rowId === column && columnId === row);
}

function idMatchesAny(id, ...values) {
  const key = stringOrNull(id);
  return Boolean(key && values.some((value) => stringOrNull(value) === key));
}

function isPairedSpaceBridgeDescriptor(descriptor, graph) {
  const model = descriptor.metadata?.selectionModel || {};
  return graph?.kind === "paired-space-linked-pairs"
    || descriptor.metadata?.primaryGrammar === "paired-space"
    || model.kind === "paired-space-linked-selection"
    || descriptor.source?.role === "dependence bridge"
    || descriptor.metadata?.role === "dependence bridge";
}

function integerOrNull(value) {
  const number = Number(value);
  return Number.isInteger(number) ? number : null;
}

function stringOrNull(value) {
  return value == null ? null : String(value);
}

function toArray(value) {
  return Array.isArray(value) ? value : [];
}

function toStringArray(value) {
  return toArray(value).map((entry) => stringOrNull(entry)).filter((entry) => entry != null);
}

function clamp01(value) {
  return Math.max(0, Math.min(1, Number(value) || 0));
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
attribute float aEdgeEmphasis;

uniform mat4 uViewProjectionMatrix;

varying vec4 vColor;
varying float vEdgeEmphasis;

void main() {
  gl_Position = uViewProjectionMatrix * vec4(aPosition, 1.0);
  vColor = aColor;
  vEdgeEmphasis = clamp(aEdgeEmphasis, 0.0, 1.0);
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
uniform float uSelectionActive;
uniform float uEmphasisStrength;
varying vec4 vColor;
varying float vEdgeEmphasis;

void main() {
  float emphasis = clamp(vEdgeEmphasis, 0.0, 1.0);
  float dim = mix(1.0, 0.38, uSelectionActive * (1.0 - emphasis));
  vec3 color = min(vec3(1.0), vColor.rgb * (1.0 + emphasis * uEmphasisStrength) + vec3(0.10 * emphasis));
  float alpha = clamp(vColor.a * uGlobalAlpha * mix(dim, 1.18, emphasis), 0.0, 1.0);
  gl_FragColor = vec4(color, alpha);
}
`;
