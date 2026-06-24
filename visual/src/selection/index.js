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

export class SelectionState {
  constructor(options = {}) {
    this.records = new Set();
    this.pairs = new Set();
    this.clusters = new Set();
    this.layers = new Set();
    this.hover = options.hover || null;
    this.focus = options.focus || null;
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
      || this.layers.size > 0;
    this.records.clear();
    this.pairs.clear();
    this.clusters.clear();
    this.layers.clear();
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

  snapshot() {
    return {
      records: Array.from(this.records),
      pairs: Array.from(this.pairs),
      clusters: Array.from(this.clusters),
      layers: Array.from(this.layers),
      hover: this.hover,
      focus: this.focus,
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
