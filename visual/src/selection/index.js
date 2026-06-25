import {
  getChannel,
  getChannelArray,
  getChannelCount,
  getChannelItemSize,
} from "../layers/channels.js";

export const LINKED_SELECTION_PRESENTATION_SCHEMA = "metric.visual.linked_selection_presentation.v1";

export const SELECTION_TARGETS = Object.freeze({
  RECORDS: "records",
  PAIRS: "pairs",
  CLUSTERS: "clusters",
  LAYERS: "layers",
});

export const SELECTION_MODES = Object.freeze({
  REPLACE: "replace",
  ADD: "add",
  REMOVE: "remove",
  TOGGLE: "toggle",
});

export const BRUSH_TYPES = Object.freeze({
  RECTANGLE: "rectangle",
  LASSO: "lasso",
});

const TARGET_ALIASES = Object.freeze({
  record: SELECTION_TARGETS.RECORDS,
  records: SELECTION_TARGETS.RECORDS,
  point: SELECTION_TARGETS.RECORDS,
  points: SELECTION_TARGETS.RECORDS,
  pair: SELECTION_TARGETS.PAIRS,
  pairs: SELECTION_TARGETS.PAIRS,
  edge: SELECTION_TARGETS.PAIRS,
  edges: SELECTION_TARGETS.PAIRS,
  cluster: SELECTION_TARGETS.CLUSTERS,
  clusters: SELECTION_TARGETS.CLUSTERS,
  layer: SELECTION_TARGETS.LAYERS,
  layers: SELECTION_TARGETS.LAYERS,
});

const RECORD_CHANNEL_NAMES = Object.freeze(["recordId", "record_id", "id", "cellId"]);
const POSITION_CHANNEL_NAMES = Object.freeze(["position", "targetPosition", "sourcePosition"]);
const EDGE_VALUE_CHANNEL_NAMES = Object.freeze(["relationValue", "value", "weight"]);

export class SelectionState {
  constructor(options = {}) {
    this.records = new Set();
    this.pairs = new Set();
    this.clusters = new Set();
    this.layers = new Set();
    this.hover = options.hover || null;
    this.focus = options.focus || null;
    this.presentation = options.presentation || null;
    this.listeners = new Map();
    this.version = 0;

    if (options.records) this.replace(SELECTION_TARGETS.RECORDS, options.records, { silent: true });
    if (options.pairs) this.replace(SELECTION_TARGETS.PAIRS, options.pairs, { silent: true });
    if (options.clusters) this.replace(SELECTION_TARGETS.CLUSTERS, options.clusters, { silent: true });
    if (options.layers) this.replace(SELECTION_TARGETS.LAYERS, options.layers, { silent: true });
  }

  on(type, listener) {
    const eventType = type || "change";
    if (typeof listener !== "function") {
      throw new Error("SelectionState.on() requires a listener function.");
    }
    if (!this.listeners.has(eventType)) this.listeners.set(eventType, new Set());
    this.listeners.get(eventType).add(listener);
    return () => this.off(eventType, listener);
  }

  off(type, listener) {
    const listeners = this.listeners.get(type || "change");
    if (listeners) listeners.delete(listener);
    return this;
  }

  emit(type, payload = {}) {
    const event = {
      type,
      version: this.version,
      state: this,
      snapshot: this.snapshot(),
      ...payload,
    };
    const listeners = this.listeners.get(type);
    const allListeners = this.listeners.get("*");
    if (listeners) {
      for (const listener of listeners) listener(event);
    }
    if (allListeners) {
      for (const listener of allListeners) listener(event);
    }
    return this;
  }

  get(target) {
    return this[getSelectionTarget(target)];
  }

  has(target, value) {
    const normalizedTarget = getSelectionTarget(target);
    return this[normalizedTarget].has(normalizeSelectionValue(normalizedTarget, value));
  }

  add(target, value, options = {}) {
    return this.addMany(target, [value], options);
  }

  addMany(target, values, options = {}) {
    return this.apply(target, values, { ...options, mode: SELECTION_MODES.ADD });
  }

  remove(target, value, options = {}) {
    return this.apply(target, [value], { ...options, mode: SELECTION_MODES.REMOVE });
  }

  toggle(target, value, options = {}) {
    return this.apply(target, [value], { ...options, mode: SELECTION_MODES.TOGGLE });
  }

  replace(target, values = [], options = {}) {
    return this.apply(target, values, { ...options, mode: SELECTION_MODES.REPLACE });
  }

  clear(target = null, options = {}) {
    if (target) {
      const normalizedTarget = getSelectionTarget(target);
      const changed = this[normalizedTarget].size > 0;
      this[normalizedTarget].clear();
      if (changed) this.changed("selection", { target: normalizedTarget, mode: "clear" }, options);
      return this;
    }

    const changed = this.records.size > 0
      || this.pairs.size > 0
      || this.clusters.size > 0
      || this.layers.size > 0
      || this.presentation != null;
    this.records.clear();
    this.pairs.clear();
    this.clusters.clear();
    this.layers.clear();
    this.presentation = null;
    if (changed) this.changed("selection", { target: null, mode: "clear" }, options);
    return this;
  }

  apply(target, values = [], options = {}) {
    const normalizedTarget = getSelectionTarget(target);
    const mode = normalizeSelectionMode(options.mode);
    const set = this[normalizedTarget];
    const normalizedValues = Array.from(values || [], (value) => normalizeSelectionValue(normalizedTarget, value, options));
    let changed = false;

    if (mode === SELECTION_MODES.REPLACE) {
      const next = new Set(normalizedValues);
      changed = !sameSet(set, next);
      if (changed) {
        set.clear();
        for (const value of next) set.add(value);
      }
    } else if (mode === SELECTION_MODES.ADD) {
      for (const value of normalizedValues) {
        if (!set.has(value)) {
          set.add(value);
          changed = true;
        }
      }
    } else if (mode === SELECTION_MODES.REMOVE) {
      for (const value of normalizedValues) {
        if (set.delete(value)) changed = true;
      }
    } else if (mode === SELECTION_MODES.TOGGLE) {
      for (const value of normalizedValues) {
        if (set.has(value)) {
          set.delete(value);
        } else {
          set.add(value);
        }
        changed = true;
      }
    }

    if (changed) {
      this.changed("selection", {
        target: normalizedTarget,
        mode,
        values: normalizedValues,
      }, options);
    }
    return this;
  }

  setHover(value, options = {}) {
    if (sameReferenceOrJSON(this.hover, value)) return this;
    this.hover = value || null;
    this.changed("hover", { hover: this.hover }, options);
    return this;
  }

  clearHover(options = {}) {
    return this.setHover(null, options);
  }

  setFocus(value, options = {}) {
    if (sameReferenceOrJSON(this.focus, value)) return this;
    this.focus = value || null;
    this.changed("focus", { focus: this.focus }, options);
    return this;
  }

  clearFocus(options = {}) {
    return this.setFocus(null, options);
  }

  setPresentation(value, options = {}) {
    if (sameReferenceOrJSON(this.presentation, value)) return this;
    this.presentation = value || null;
    this.changed("presentation", { presentation: this.presentation }, options);
    return this;
  }

  clearPresentation(options = {}) {
    return this.setPresentation(null, options);
  }

  snapshot() {
    return {
      records: Array.from(this.records),
      pairs: Array.from(this.pairs),
      clusters: Array.from(this.clusters),
      layers: Array.from(this.layers),
      hover: this.hover,
      focus: this.focus,
      presentation: this.presentation,
      version: this.version,
    };
  }

  restore(snapshot = {}, options = {}) {
    this.records = new Set(snapshot.records || []);
    this.pairs = new Set(snapshot.pairs || []);
    this.clusters = new Set(snapshot.clusters || []);
    this.layers = new Set(snapshot.layers || []);
    this.hover = snapshot.hover || null;
    this.focus = snapshot.focus || null;
    this.presentation = snapshot.presentation || null;
    this.changed("restore", {}, options);
    return this;
  }

  changed(type, payload = {}, options = {}) {
    this.version += 1;
    if (options.silent) return this;
    this.emit(type, payload);
    if (type !== "change") this.emit("change", { cause: type, ...payload });
    return this;
  }
}

export function createSelectionState(options = {}) {
  return new SelectionState(options);
}

export function buildLinkedSelectionPresentation(selection = {}, context = {}) {
  const runtime = context.runtime || null;
  const visualSpace = context.visualSpace || runtime?.visualSpace || null;
  const document = context.document || visualSpace?.document || runtime?.document || null;
  const descriptors = toSelectionArray(context.layerDescriptors ?? context.descriptors ?? runtime?.layerDescriptors);
  const pair = normalizePresentationPair(selection?.pair);
  const selectedRecordId = selection?.recordId == null ? null : String(selection.recordId);
  const recordTargets = selectedRecordTargets(selectedRecordId, pair, visualSpace);
  const presentation = {
    schema: LINKED_SELECTION_PRESENTATION_SCHEMA,
    kind: pair ? "pair" : selectedRecordId != null ? "record" : "none",
    source: selection?.source || null,
    pickSource: selection?.pickSource || selection?.pickingSource || pair?.pickSource || null,
    selectedRecordId,
    selectedPair: pair ? clonePresentationValue(pair) : null,
    records: recordTargets.map((target) => ({
      recordId: target.recordId,
      roles: target.roles.slice(),
      record: target.record ? recordSummary(target.record) : null,
    })),
    relationMatrixCells: [],
    graphEdges: [],
    pairedSpaceBridges: [],
    recordFeatures: [],
    features: [],
    highlight: {
      recordIds: [],
      relationMatrixCells: [],
      graphEdgeIds: [],
      pairedSpaceBridgeIds: [],
    },
    counts: {
      records: 0,
      relationMatrixCells: 0,
      graphEdges: 0,
      pairedSpaceBridges: 0,
      features: 0,
    },
  };

  if (!pair && selectedRecordId == null) return presentation;

  const seen = {
    records: new Set(),
    matrices: new Set(),
    graphEdges: new Set(),
    bridges: new Set(),
  };

  for (const descriptor of descriptors) {
    if (!descriptor || descriptor.visible === false) continue;
    collectRecordPresentationFeatures(presentation, descriptor, recordTargets, seen.records);
    collectRelationMatrixPresentationFeatures(presentation, descriptor, { pair, selectedRecordId }, seen.matrices);
    collectRelationEdgePresentationFeatures(presentation, descriptor, { pair, selectedRecordId }, seen);
  }

  presentation.features = [
    ...presentation.recordFeatures,
    ...presentation.relationMatrixCells,
    ...presentation.graphEdges,
    ...presentation.pairedSpaceBridges,
  ];
  presentation.highlight.recordIds = uniqueStrings(presentation.records.map((record) => record.recordId));
  presentation.highlight.relationMatrixCells = presentation.relationMatrixCells.map((feature) => ({
    layerId: feature.layerId,
    relationId: feature.relationId,
    rowId: feature.rowId,
    columnId: feature.columnId,
    row: feature.row,
    column: feature.column,
  }));
  presentation.highlight.graphEdgeIds = uniqueStrings(presentation.graphEdges.map((feature) => feature.edgeId));
  presentation.highlight.pairedSpaceBridgeIds = uniqueStrings(presentation.pairedSpaceBridges.map((feature) => feature.edgeId));
  presentation.counts = {
    records: presentation.recordFeatures.length,
    relationMatrixCells: presentation.relationMatrixCells.length,
    graphEdges: presentation.graphEdges.length,
    pairedSpaceBridges: presentation.pairedSpaceBridges.length,
    features: presentation.features.length,
  };
  presentation.document = document?.schema ? { schema: document.schema } : null;
  return presentation;
}

export function getSelectionTarget(target) {
  const normalized = TARGET_ALIASES[String(target || SELECTION_TARGETS.RECORDS).toLowerCase()];
  if (!normalized) {
    throw new Error(`Unsupported selection target "${target}".`);
  }
  return normalized;
}

export function normalizeSelectionMode(mode = SELECTION_MODES.REPLACE) {
  const value = String(mode || SELECTION_MODES.REPLACE).toLowerCase();
  if (
    value === SELECTION_MODES.REPLACE
    || value === SELECTION_MODES.ADD
    || value === SELECTION_MODES.REMOVE
    || value === SELECTION_MODES.TOGGLE
  ) {
    return value;
  }
  throw new Error(`Unsupported selection mode "${mode}".`);
}

export function makePairKey(a, b, options = {}) {
  const pair = Array.isArray(a) && b === undefined ? a : [a, b];
  if (pair.length < 2) throw new Error("makePairKey() requires two ids.");
  const left = stableSelectionId(pair[0]);
  const right = stableSelectionId(pair[1]);
  if (options.ordered === false && right < left) {
    return JSON.stringify([right, left]);
  }
  return JSON.stringify([left, right]);
}

export function normalizeScreenPoint(point) {
  if (!point) return null;
  if (Array.isArray(point) || ArrayBuffer.isView(point)) {
    if (point.length < 2) return null;
    return finitePoint(point[0], point[1]);
  }
  if (point.screen) return normalizeScreenPoint(point.screen);
  if (point.pixel) return normalizeScreenPoint(point.pixel);
  if (point.css) return normalizeScreenPoint(point.css);
  if (Number.isFinite(Number(point.x)) && Number.isFinite(Number(point.y))) {
    return finitePoint(point.x, point.y);
  }
  return null;
}

export function createRectangleBrush(start, end) {
  const a = normalizeScreenPoint(start);
  const b = normalizeScreenPoint(end);
  if (!a || !b) throw new Error("createRectangleBrush() requires two screen-space points.");
  const x0 = Math.min(a.x, b.x);
  const y0 = Math.min(a.y, b.y);
  const x1 = Math.max(a.x, b.x);
  const y1 = Math.max(a.y, b.y);
  return {
    type: BRUSH_TYPES.RECTANGLE,
    x0,
    y0,
    x1,
    y1,
    width: x1 - x0,
    height: y1 - y0,
    start: a,
    end: b,
  };
}

export function createLassoBrush(points = []) {
  const normalizedPoints = Array.from(points, normalizeScreenPoint).filter(Boolean);
  if (normalizedPoints.length < 3) {
    throw new Error("createLassoBrush() requires at least three screen-space points.");
  }
  return {
    type: BRUSH_TYPES.LASSO,
    points: normalizedPoints,
    bounds: boundsForPoints(normalizedPoints),
  };
}

export function pointInRectangleBrush(point, brush) {
  const p = normalizeScreenPoint(point);
  if (!p || !brush) return false;
  return p.x >= brush.x0 && p.x <= brush.x1 && p.y >= brush.y0 && p.y <= brush.y1;
}

export function pointInLassoBrush(point, brush) {
  const p = normalizeScreenPoint(point);
  if (!p || !brush || !Array.isArray(brush.points) || brush.points.length < 3) return false;
  if (brush.bounds && !pointInRectangleBrush(p, brush.bounds)) return false;

  let inside = false;
  const polygon = brush.points;

  for (let i = 0, j = polygon.length - 1; i < polygon.length; j = i, i += 1) {
    const a = polygon[i];
    const b = polygon[j];
    if (pointOnSegment(p, a, b)) return true;
    const intersects = (a.y > p.y) !== (b.y > p.y)
      && p.x < ((b.x - a.x) * (p.y - a.y)) / (b.y - a.y) + a.x;
    if (intersects) inside = !inside;
  }

  return inside;
}

export function brushContainsPoint(brush, point) {
  if (!brush) return false;
  if (brush.type === BRUSH_TYPES.RECTANGLE || brush.x0 != null) {
    return pointInRectangleBrush(point, brush);
  }
  if (brush.type === BRUSH_TYPES.LASSO || Array.isArray(brush.points)) {
    return pointInLassoBrush(point, brush);
  }
  return false;
}

export function filterBrushPoints(points, brush, options = {}) {
  if (!points || typeof points[Symbol.iterator] !== "function") return [];
  const results = [];
  const accessor = options.pointAccessor || options.screenAccessor;

  for (const item of points) {
    const point = typeof accessor === "function" ? normalizeScreenPoint(accessor(item)) : normalizeScreenPoint(item);
    if (!point || !brushContainsPoint(brush, point)) continue;
    results.push(options.includePoint ? { item, point } : item);
  }

  return results;
}

export function collectBrushIds(points, brush, options = {}) {
  const target = getSelectionTarget(options.target || SELECTION_TARGETS.RECORDS);
  const hits = options.preFiltered ? Array.from(points || []) : filterBrushPoints(points, brush, options);
  const ids = [];

  for (const item of hits) {
    const source = item?.item ?? item;
    const id = readBrushSelectionId(source, target, options);
    if (id != null) ids.push(id);
  }

  return ids;
}

export function applyBrushSelection(selection, points, brush, options = {}) {
  if (!selection || typeof selection.apply !== "function") {
    throw new Error("applyBrushSelection() requires a SelectionState-like object.");
  }
  const target = getSelectionTarget(options.target || SELECTION_TARGETS.RECORDS);
  const hits = filterBrushPoints(points, brush, { ...options, includePoint: Boolean(options.includePoint) });
  const ids = collectBrushIds(hits, brush, { ...options, target, preFiltered: true });
  selection.apply(target, ids, { mode: options.mode || SELECTION_MODES.REPLACE });
  return { hits, ids, target };
}

export function boundsForPoints(points = []) {
  let x0 = Infinity;
  let y0 = Infinity;
  let x1 = -Infinity;
  let y1 = -Infinity;

  for (const point of points) {
    const p = normalizeScreenPoint(point);
    if (!p) continue;
    x0 = Math.min(x0, p.x);
    y0 = Math.min(y0, p.y);
    x1 = Math.max(x1, p.x);
    y1 = Math.max(y1, p.y);
  }

  if (!Number.isFinite(x0)) {
    return createRectangleBrush([0, 0], [0, 0]);
  }

  return {
    type: BRUSH_TYPES.RECTANGLE,
    x0,
    y0,
    x1,
    y1,
    width: x1 - x0,
    height: y1 - y0,
  };
}

function collectRecordPresentationFeatures(presentation, descriptor, recordTargets, seen) {
  if (!recordTargets.length || descriptor.primitive === "RelationMatrixLayer" || descriptor.primitive === "RelationEdgeLayer") {
    return presentation;
  }
  const channels = descriptor.channels || {};
  const recordChannel = getChannel(channels, RECORD_CHANNEL_NAMES);
  const positionChannel = getChannel(channels, POSITION_CHANNEL_NAMES);
  const recordIds = getChannelArray(recordChannel);
  const positions = getChannelArray(positionChannel);
  if (!recordIds?.length || !positions?.length) return presentation;

  const itemSize = getChannelItemSize(positionChannel, 3);
  const count = Math.min(getChannelCount(recordChannel, 1), getChannelCount(positionChannel, itemSize));
  const targetsById = new Map(recordTargets.map((target) => [target.recordId, target]));
  for (let index = 0; index < count; index += 1) {
    const recordId = recordIds[index] == null ? null : String(recordIds[index]);
    const target = recordId == null ? null : targetsById.get(recordId);
    if (!target) continue;
    const source = descriptor.source || {};
    const metadata = descriptor.metadata || {};
    const layerId = descriptor.id || null;
    const key = `${layerId || descriptor.primitive || "record"}:${recordId}:${index}`;
    if (seen.has(key)) continue;
    seen.add(key);
    presentation.recordFeatures.push({
      kind: "record",
      feature: "metric-space-record",
      recordId,
      roles: target.roles.slice(),
      layerId,
      descriptorId: descriptor.id || null,
      descriptorKind: descriptor.kind || descriptor.primitive || null,
      viewId: source.viewId || metadata.viewId || null,
      viewKind: source.viewKind || metadata.viewKind || null,
      spaceId: source.spaceId || metadata.spaceId || metadata.pairedSpace?.spaceId || null,
      coordinateId: source.coordinateId || metadata.coordinateId || metadata.pairedSpace?.coordinateId || null,
      pairedSpaceRole: source.pairedSpaceRole || metadata.pairedSpace?.role || metadata.selectionModel?.pairedSpaceRole || null,
      pairSetId: source.pairSetId || metadata.pairedSpace?.pairSetId || metadata.selectionModel?.pairSetId || null,
      index,
      position: vectorAt(positions, itemSize, index),
    });
  }
  return presentation;
}

function collectRelationMatrixPresentationFeatures(presentation, descriptor, selection, seen) {
  if (descriptor.primitive !== "RelationMatrixLayer") return presentation;
  const matrix = descriptor.metadata?.matrix
    || descriptor.source?.texture?.matrix
    || descriptor.source?.textureData?.matrix
    || descriptor.source?.matrix
    || null;
  const recordIds = matrix?.recordIds || descriptor.source?.recordIds || descriptor.metadata?.recordIds || [];
  if (!recordIds.length) return presentation;
  const relationId = descriptor.source?.relationId || descriptor.metadata?.relationId || matrix?.relationId || null;
  const relationName = descriptor.source?.relationName || descriptor.metadata?.relationName || matrix?.relationName || null;
  const pair = selection.pair;
  const selectedRecordId = selection.selectedRecordId;

  if (pair) {
    if (!relationCompatible(pair, relationId, { allowMissingPairRelation: false })) return presentation;
    const row = findRecordIndex(recordIds, pair.rowId, pair.row);
    const column = findRecordIndex(recordIds, pair.columnId, pair.column);
    if (row < 0 && column < 0) return presentation;
    pushMatrixFeature(presentation, descriptor, matrix, {
      relationId,
      relationName,
      rowId: pair.rowId,
      columnId: pair.columnId,
      row,
      column,
      roles: ["row", "column", "cell"],
      source: "selected-pair",
    }, seen);
    return presentation;
  }

  if (selectedRecordId != null) {
    const index = findRecordIndex(recordIds, selectedRecordId);
    if (index < 0) return presentation;
    pushMatrixFeature(presentation, descriptor, matrix, {
      relationId,
      relationName,
      rowId: selectedRecordId,
      columnId: selectedRecordId,
      row: index,
      column: index,
      roles: ["row", "column", "record-diagonal"],
      source: "selected-record",
    }, seen);
  }
  return presentation;
}

function pushMatrixFeature(presentation, descriptor, matrix, feature, seen) {
  const layerId = descriptor.id || null;
  const key = `${layerId || "matrix"}:${feature.relationId || ""}:${feature.rowId}:${feature.columnId}`;
  if (seen.has(key)) return;
  seen.add(key);
  const value = matrixCellValue(matrix, feature.row, feature.column);
  presentation.relationMatrixCells.push({
    kind: "relation-matrix-cell",
    feature: "relation-matrix-cell",
    layerId,
    descriptorId: descriptor.id || null,
    descriptorKind: descriptor.kind || descriptor.primitive || null,
    viewId: descriptor.source?.viewId || descriptor.metadata?.viewId || null,
    viewKind: descriptor.source?.viewKind || descriptor.metadata?.viewKind || null,
    relationId: feature.relationId,
    relationName: feature.relationName,
    rowId: feature.rowId,
    columnId: feature.columnId,
    row: feature.row,
    column: feature.column,
    rowActive: feature.row >= 0,
    columnActive: feature.column >= 0,
    cellActive: feature.row >= 0 && feature.column >= 0,
    roles: feature.roles.slice(),
    value: value.value,
    present: value.present,
    selectionSource: feature.source,
  });
}

function collectRelationEdgePresentationFeatures(presentation, descriptor, selection, seen) {
  if (descriptor.primitive !== "RelationEdgeLayer") return presentation;
  const graph = descriptor.metadata?.graph || descriptor.source?.graph || null;
  const edges = Array.isArray(graph?.edges) ? graph.edges : [];
  if (!edges.length) return presentation;
  const relationId = descriptor.source?.relationId || graph?.relationId || graph?.edge_relation_id || descriptor.metadata?.relationId || null;
  const graphId = descriptor.source?.graphId || graph?.id || descriptor.metadata?.graphId || null;
  const pairSetId = descriptor.source?.pairSetId || descriptor.metadata?.selectionModel?.pairSetId || graph?.id || null;
  const sourcePositions = vec3ChannelReader(descriptor.channels, ["sourcePosition", "source"]);
  const targetPositions = vec3ChannelReader(descriptor.channels, ["targetPosition", "target"]);
  const values = getChannelArray(getChannel(descriptor.channels || {}, EDGE_VALUE_CHANNEL_NAMES));
  const isBridge = isPairedSpaceBridgeDescriptor(descriptor, graph);
  const count = Math.min(edges.length, Math.max(sourcePositions.count, targetPositions.count, edges.length));

  for (let index = 0; index < count; index += 1) {
    const edge = normalizePresentationEdge(edges[index] || {}, {
      descriptor,
      relationId,
      graphId,
      pairSetId,
      value: values?.[index],
      index,
      sourcePosition: sourcePositions.at(index),
      targetPosition: targetPositions.at(index),
      isBridge,
    });
    const match = edgeSelectionMatch(edge, selection, { isBridge, relationId });
    if (!match) continue;
    if (isBridge) {
      pushBridgeFeature(presentation, edge, descriptor, match, seen.bridges);
    } else {
      pushGraphEdgeFeature(presentation, edge, descriptor, match, seen.graphEdges);
    }
  }
  return presentation;
}

function pushGraphEdgeFeature(presentation, edge, descriptor, match, seen) {
  const key = `${descriptor.id || "edge"}:${edge.edgeId}:${match.kind}`;
  if (seen.has(key)) return;
  seen.add(key);
  presentation.graphEdges.push({
    ...edgeFeatureBase(edge, descriptor, match),
    kind: "graph-edge",
    feature: "graph-edge",
  });
}

function pushBridgeFeature(presentation, edge, descriptor, match, seen) {
  const key = `${descriptor.id || "bridge"}:${edge.edgeId}:${match.kind}`;
  if (seen.has(key)) return;
  seen.add(key);
  presentation.pairedSpaceBridges.push({
    ...edgeFeatureBase(edge, descriptor, match),
    kind: "paired-space-bridge",
    feature: "paired-space-bridge",
    pairId: edge.pairId,
    pairSetId: edge.pairSetId,
    sourceSpaceId: edge.sourceSpaceId,
    targetSpaceId: edge.targetSpaceId,
    sourceCoordinateId: edge.sourceCoordinateId,
    targetCoordinateId: edge.targetCoordinateId,
  });
}

function edgeFeatureBase(edge, descriptor, match) {
  return {
    layerId: descriptor.id || edge.layerId || null,
    descriptorId: descriptor.id || null,
    descriptorKind: descriptor.kind || descriptor.primitive || null,
    viewId: descriptor.source?.viewId || descriptor.metadata?.viewId || null,
    viewKind: descriptor.source?.viewKind || descriptor.metadata?.viewKind || null,
    relationId: edge.relationId,
    graphId: edge.graphId,
    edgeId: edge.edgeId,
    rowId: edge.rowId,
    columnId: edge.columnId,
    sourceId: edge.sourceId,
    targetId: edge.targetId,
    row: edge.row,
    column: edge.column,
    value: edge.value,
    present: edge.present,
    index: edge.index,
    sourcePosition: edge.sourcePosition,
    targetPosition: edge.targetPosition,
    selectionMatch: match,
  };
}

function selectedRecordTargets(selectedRecordId, pair, visualSpace = null) {
  const targets = new Map();
  if (selectedRecordId != null) addRecordTarget(targets, selectedRecordId, ["selected"], visualSpace);
  if (pair) {
    addRecordTarget(targets, pair.rowId, ["row", "source"], visualSpace);
    addRecordTarget(targets, pair.columnId, ["column", "target"], visualSpace);
  }
  return Array.from(targets.values());
}

function addRecordTarget(targets, recordId, roles, visualSpace = null) {
  if (recordId == null) return;
  const key = String(recordId);
  if (!targets.has(key)) {
    targets.set(key, {
      recordId: key,
      roles: [],
      record: visualSpace?.getRecord ? visualSpace.getRecord(key) : null,
    });
  }
  const target = targets.get(key);
  for (const role of roles) {
    if (!target.roles.includes(role)) target.roles.push(role);
  }
}

function recordSummary(record) {
  if (!record) return null;
  return {
    id: record.id ?? record.record_id ?? null,
    label: record.label ?? record.name ?? record.id ?? null,
    datasetId: record.dataset_id ?? record.datasetId ?? null,
    type: record.record_type ?? record.recordType ?? record.type ?? null,
  };
}

function normalizePresentationPair(pair) {
  if (!pair || typeof pair !== "object") return null;
  const rowId = pair.rowId ?? pair.row_id ?? pair.sourceId ?? pair.source_id;
  const columnId = pair.columnId ?? pair.column_id ?? pair.targetId ?? pair.target_id;
  if (rowId == null || columnId == null) return null;
  return {
    relationId: pair.relationId ?? pair.relation_id ?? null,
    relationName: pair.relationName ?? pair.relation_name ?? null,
    relationType: pair.relationType ?? pair.relation_type ?? null,
    graphId: pair.graphId ?? pair.graph_id ?? null,
    edgeId: pair.edgeId ?? pair.edge_id ?? null,
    pairId: pair.pairId ?? pair.pair_id ?? null,
    pairSetId: pair.pairSetId ?? pair.pair_set_id ?? null,
    rowId: String(rowId),
    columnId: String(columnId),
    row: integerOrNull(pair.row),
    column: integerOrNull(pair.column),
    value: pair.value,
    present: pair.present !== false,
    pickSource: pair.pickSource ?? pair.pick_source ?? null,
  };
}

function normalizePresentationEdge(edge, options) {
  const rowId = edge.source ?? edge.rowId ?? edge.row_id ?? edge.sourceId ?? edge.source_id ?? edge.a;
  const columnId = edge.target ?? edge.columnId ?? edge.column_id ?? edge.targetId ?? edge.target_id ?? edge.b;
  const edgeId = edge.id || edge.edgeId || edge.edge_id
    || `${options.relationId || options.graphId || options.pairSetId || options.descriptor.id || "edge"}:${rowId}:${columnId}:${options.index}`;
  return {
    relationId: options.relationId,
    graphId: options.graphId,
    edgeId: String(edgeId),
    pairId: edge.pairId ?? edge.pair_id ?? null,
    pairSetId: options.pairSetId,
    rowId: rowId == null ? null : String(rowId),
    columnId: columnId == null ? null : String(columnId),
    sourceId: rowId == null ? null : String(rowId),
    targetId: columnId == null ? null : String(columnId),
    row: integerOrNull(edge.sourceIndex ?? edge.row),
    column: integerOrNull(edge.targetIndex ?? edge.column),
    value: edge.value ?? options.value ?? null,
    present: edge.present !== false,
    index: options.index,
    layerId: options.descriptor.id || null,
    sourcePosition: options.sourcePosition,
    targetPosition: options.targetPosition,
    sourceSpaceId: edge.sourceSpaceId ?? edge.source_space_id ?? null,
    targetSpaceId: edge.targetSpaceId ?? edge.target_space_id ?? null,
    sourceCoordinateId: edge.sourceCoordinateId ?? edge.source_coordinate_id ?? null,
    targetCoordinateId: edge.targetCoordinateId ?? edge.target_coordinate_id ?? null,
    isBridge: options.isBridge,
  };
}

function edgeSelectionMatch(edge, selection, options = {}) {
  const pair = selection.pair;
  if (pair && pairMatchesEdge(pair, edge, options)) {
    return {
      kind: "pair",
      direction: pair.rowId === edge.rowId && pair.columnId === edge.columnId ? "forward" : "reverse",
    };
  }
  if (selection.selectedRecordId != null && (selection.selectedRecordId === edge.rowId || selection.selectedRecordId === edge.columnId)) {
    return {
      kind: "record-endpoint",
      recordId: selection.selectedRecordId,
      endpoint: selection.selectedRecordId === edge.rowId ? "source" : "target",
    };
  }
  return null;
}

function pairMatchesEdge(pair, edge, options = {}) {
  if (pair.edgeId != null && edge.edgeId != null && String(pair.edgeId) === String(edge.edgeId)) return true;
  if (pair.pairId != null && edge.pairId != null && String(pair.pairId) === String(edge.pairId)) return true;
  const relationOk = options.isBridge
    ? (pair.pairSetId == null || edge.pairSetId == null || String(pair.pairSetId) === String(edge.pairSetId))
    : relationCompatible(pair, edge.relationId, { allowMissingPairRelation: false });
  if (!relationOk) return false;
  const forward = pair.rowId === edge.rowId && pair.columnId === edge.columnId;
  const reverse = pair.rowId === edge.columnId && pair.columnId === edge.rowId;
  return forward || reverse;
}

function relationCompatible(pair, relationId, options = {}) {
  const pairRelation = pair?.relationId == null ? null : String(pair.relationId);
  const descriptorRelation = relationId == null ? null : String(relationId);
  if (pairRelation && descriptorRelation) return pairRelation === descriptorRelation;
  if (!pairRelation && !descriptorRelation) return true;
  return options.allowMissingPairRelation !== false;
}

function isPairedSpaceBridgeDescriptor(descriptor, graph) {
  const model = descriptor.metadata?.selectionModel || {};
  return graph?.kind === "paired-space-linked-pairs"
    || descriptor.metadata?.primaryGrammar === "paired-space"
    || model.kind === "paired-space-linked-selection"
    || descriptor.source?.role === "dependence bridge"
    || descriptor.metadata?.role === "dependence bridge";
}

function matrixCellValue(matrix, row, column) {
  if (!matrix || row < 0 || column < 0) return { value: undefined, present: false };
  const width = Math.max(1, Number(matrix.width ?? matrix.size ?? matrix.recordIds?.length ?? 1) || 1);
  const offset = row * width + column;
  const values = matrix.values;
  const present = matrix.present;
  return {
    value: values?.[offset],
    present: present ? present[offset] !== 0 : values?.[offset] !== undefined,
  };
}

function findRecordIndex(recordIds, recordId, fallbackIndex = null) {
  if (recordId != null) {
    const match = recordIds.findIndex((id) => String(id) === String(recordId));
    if (match >= 0) return match;
  }
  const index = integerOrNull(fallbackIndex);
  return index != null && index >= 0 && index < recordIds.length ? index : -1;
}

function vec3ChannelReader(channels = {}, names) {
  const channel = getChannel(channels, names);
  const array = getChannelArray(channel);
  const itemSize = getChannelItemSize(channel, 3);
  const count = getChannelCount(channel, itemSize);
  return {
    count,
    at(index) {
      if (!array || index < 0 || index >= count) return null;
      return vectorAt(array, itemSize, index);
    },
  };
}

function vectorAt(array, itemSize, index) {
  const offset = index * itemSize;
  return [
    finiteNumberOrNull(array?.[offset]) ?? 0,
    finiteNumberOrNull(array?.[offset + 1]) ?? 0,
    finiteNumberOrNull(array?.[offset + 2]) ?? 0,
  ];
}

function integerOrNull(value) {
  const number = Number(value);
  return Number.isInteger(number) ? number : null;
}

function finiteNumberOrNull(value) {
  const number = Number(value);
  return Number.isFinite(number) ? number : null;
}

function toSelectionArray(value) {
  if (Array.isArray(value)) return value;
  if (value == null) return [];
  return [value];
}

function uniqueStrings(values) {
  return Array.from(new Set(values.filter((value) => value != null).map(String)));
}

function clonePresentationValue(value) {
  if (Array.isArray(value)) return value.map(clonePresentationValue);
  if (ArrayBuffer.isView(value)) return Array.from(value);
  if (!value || typeof value !== "object") return value;
  const out = {};
  for (const [key, entry] of Object.entries(value)) out[key] = clonePresentationValue(entry);
  return out;
}

function normalizeSelectionValue(target, value, options = {}) {
  if (target === SELECTION_TARGETS.PAIRS) {
    if (Array.isArray(value)) return makePairKey(value, undefined, options);
    if (value && typeof value === "object") {
      if (value.pairId != null) return stableSelectionId(value.pairId);
      if (value.edgeId != null) return stableSelectionId(value.edgeId);
      if (value.sourceId != null && value.targetId != null) {
        return makePairKey(value.sourceId, value.targetId, options);
      }
    }
  }
  return stableSelectionId(value);
}

function readBrushSelectionId(item, target, options = {}) {
  if (typeof options.idAccessor === "function") {
    const id = options.idAccessor(item, target);
    return id == null ? null : normalizeSelectionValue(target, id, options);
  }
  if (!item || typeof item !== "object") {
    return item == null ? null : normalizeSelectionValue(target, item, options);
  }
  if (target === SELECTION_TARGETS.RECORDS) {
    const id = item.recordId ?? item.id;
    return id == null ? null : normalizeSelectionValue(target, id, options);
  }
  if (target === SELECTION_TARGETS.PAIRS) {
    if (item.pairId != null || item.edgeId != null || (item.sourceId != null && item.targetId != null)) {
      return normalizeSelectionValue(target, item, options);
    }
    return item.id == null ? null : normalizeSelectionValue(target, item.id, options);
  }
  if (target === SELECTION_TARGETS.CLUSTERS) {
    const id = item.clusterId ?? item.id;
    return id == null ? null : normalizeSelectionValue(target, id, options);
  }
  if (target === SELECTION_TARGETS.LAYERS) {
    const id = item.layerId ?? item.id;
    return id == null ? null : normalizeSelectionValue(target, id, options);
  }
  return null;
}

function stableSelectionId(value) {
  if (value == null) throw new Error("Selection id cannot be null or undefined.");
  if (typeof value === "string" || typeof value === "number" || typeof value === "boolean") return String(value);
  try {
    return JSON.stringify(value);
  } catch {
    return String(value);
  }
}

function finitePoint(x, y) {
  return {
    x: Number(x),
    y: Number(y),
  };
}

function sameSet(a, b) {
  if (a.size !== b.size) return false;
  for (const value of a) {
    if (!b.has(value)) return false;
  }
  return true;
}

function sameReferenceOrJSON(a, b) {
  if (a === b) return true;
  if (!a || !b) return false;
  try {
    return JSON.stringify(a) === JSON.stringify(b);
  } catch {
    return false;
  }
}

function pointOnSegment(point, a, b) {
  const cross = (point.y - a.y) * (b.x - a.x) - (point.x - a.x) * (b.y - a.y);
  if (Math.abs(cross) > 1e-6) return false;
  const dot = (point.x - a.x) * (b.x - a.x) + (point.y - a.y) * (b.y - a.y);
  if (dot < 0) return false;
  const lengthSquared = (b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y);
  return dot <= lengthSquared;
}
