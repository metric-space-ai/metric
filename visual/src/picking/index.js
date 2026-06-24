export const NO_PICK_ID = 0;
export const MAX_RGB_PICK_ID = 0xffffff;
export const MAX_RGBA_PICK_ID = 0xffffffff;
export const DEFAULT_PICK_RADIUS = 4;
export const PICKING_PASS_CONTRACT_VERSION = 1;

export const PICK_KINDS = Object.freeze({
  RECORD: "record",
  LAYER: "layer",
  EDGE: "edge",
});

const PICK_KIND_ALIASES = Object.freeze({
  record: PICK_KINDS.RECORD,
  records: PICK_KINDS.RECORD,
  point: PICK_KINDS.RECORD,
  feature: PICK_KINDS.RECORD,
  layer: PICK_KINDS.LAYER,
  layers: PICK_KINDS.LAYER,
  edge: PICK_KINDS.EDGE,
  edges: PICK_KINDS.EDGE,
  pair: PICK_KINDS.EDGE,
  link: PICK_KINDS.EDGE,
});

export function encodePickIdRGB(id, out = [0, 0, 0], offset = 0) {
  const numericId = assertPickId(id, MAX_RGB_PICK_ID, "RGB pick id");
  out[offset] = numericId % 256;
  out[offset + 1] = Math.floor(numericId / 256) % 256;
  out[offset + 2] = Math.floor(numericId / 65536) % 256;
  return out;
}

export function encodePickIdRGBA(id, out = [0, 0, 0, 0], offset = 0) {
  const numericId = assertPickId(id, MAX_RGBA_PICK_ID, "RGBA pick id");
  out[offset] = numericId % 256;
  out[offset + 1] = Math.floor(numericId / 256) % 256;
  out[offset + 2] = Math.floor(numericId / 65536) % 256;
  out[offset + 3] = Math.floor(numericId / 16777216) % 256;
  return out;
}

export function encodePickIdRGBFloat(id, out = [0, 0, 0], offset = 0) {
  const numericId = assertPickId(id, MAX_RGB_PICK_ID, "RGB pick id");
  out[offset] = (numericId % 256) / 255;
  out[offset + 1] = (Math.floor(numericId / 256) % 256) / 255;
  out[offset + 2] = (Math.floor(numericId / 65536) % 256) / 255;
  return out;
}

export function encodePickIdRGBAFloat(id, out = [0, 0, 0, 0], offset = 0) {
  const numericId = assertPickId(id, MAX_RGBA_PICK_ID, "RGBA pick id");
  out[offset] = (numericId % 256) / 255;
  out[offset + 1] = (Math.floor(numericId / 256) % 256) / 255;
  out[offset + 2] = (Math.floor(numericId / 65536) % 256) / 255;
  out[offset + 3] = (Math.floor(numericId / 16777216) % 256) / 255;
  return out;
}

export function decodePickIdRGB(buffer, offset = 0) {
  if (!buffer) return NO_PICK_ID;
  return byteAt(buffer, offset) + byteAt(buffer, offset + 1) * 256 + byteAt(buffer, offset + 2) * 65536;
}

export function decodePickIdRGBA(buffer, offset = 0) {
  if (!buffer) return NO_PICK_ID;
  return (
    byteAt(buffer, offset)
    + byteAt(buffer, offset + 1) * 256
    + byteAt(buffer, offset + 2) * 65536
    + byteAt(buffer, offset + 3) * 16777216
  );
}

export function decodePickId(buffer, options = {}) {
  const channels = options.channels === 3 || options.format === "rgb" ? 3 : 4;
  const offset = Math.max(0, Math.floor(finiteNumber(options.offset, 0)));
  return channels === 3 ? decodePickIdRGB(buffer, offset) : decodePickIdRGBA(buffer, offset);
}

export function decodePickBufferPixel(buffer, x, y, width, options = {}) {
  const channels = options.channels === 3 || options.format === "rgb" ? 3 : 4;
  const pixelX = Math.floor(finiteNumber(x, -1));
  const pixelY = Math.floor(finiteNumber(y, -1));
  const bufferWidth = Math.floor(finiteNumber(width, 0));
  const bufferHeight = Math.floor(finiteNumber(options.height, 0));

  if (!buffer || pixelX < 0 || pixelY < 0 || bufferWidth <= 0) return NO_PICK_ID;
  if (pixelX >= bufferWidth || (bufferHeight > 0 && pixelY >= bufferHeight)) return NO_PICK_ID;

  const row = options.yOrigin === "bottom" && bufferHeight > 0 ? bufferHeight - 1 - pixelY : pixelY;
  const offset = (row * bufferWidth + pixelX) * channels;
  if (offset < 0 || offset + channels > buffer.length) return NO_PICK_ID;
  return channels === 3 ? decodePickIdRGB(buffer, offset) : decodePickIdRGBA(buffer, offset);
}

export function readPickIdFromFramebuffer(gl, x, y, options = {}) {
  if (!gl || typeof gl.readPixels !== "function") {
    throw new Error("readPickIdFromFramebuffer() requires a WebGL context.");
  }

  const width = Math.max(1, Math.floor(finiteNumber(options.width, gl.drawingBufferWidth || 1)));
  const height = Math.max(1, Math.floor(finiteNumber(options.height, gl.drawingBufferHeight || 1)));
  const pixelX = Math.max(0, Math.min(width - 1, Math.floor(finiteNumber(x, 0))));
  const inputY = Math.max(0, Math.min(height - 1, Math.floor(finiteNumber(y, 0))));
  const pixelY = options.yOrigin === "bottom" ? inputY : height - 1 - inputY;
  const bytes = options.buffer || new Uint8Array(4);

  gl.readPixels(pixelX, pixelY, 1, 1, gl.RGBA, gl.UNSIGNED_BYTE, bytes);
  return options.channels === 3 || options.format === "rgb" ? decodePickIdRGB(bytes, 0) : decodePickIdRGBA(bytes, 0);
}

export class PickingRegistry {
  constructor(options = {}) {
    this.start = Math.max(1, Math.floor(finiteNumber(options.start, 1)));
    this.maxId = Math.min(
      MAX_RGBA_PICK_ID,
      Math.max(this.start, Math.floor(finiteNumber(options.maxId, MAX_RGB_PICK_ID))),
    );
    this.nextId = this.start;
    this.entriesByNumericId = new Map();
    this.numericIdByKey = new Map();
  }

  get size() {
    return this.entriesByNumericId.size;
  }

  register(kind, id, options = {}) {
    const normalizedKind = normalizePickKind(kind);
    const key = makeRegistryKey(normalizedKind, id, options.scope);
    const existingId = this.numericIdByKey.get(key);

    if (existingId !== undefined) {
      const entry = createPickEntry(existingId, normalizedKind, id, options);
      this.entriesByNumericId.set(existingId, entry);
      return existingId;
    }

    const numericId = options.numericId == null
      ? this.allocateId()
      : assertPickId(options.numericId, this.maxId, "registry numeric id");
    const previousKey = this.keyForNumericId(numericId);

    if (previousKey && previousKey !== key) {
      throw new Error(`Picking id ${numericId} is already registered.`);
    }

    const entry = createPickEntry(numericId, normalizedKind, id, options);
    this.entriesByNumericId.set(numericId, entry);
    this.numericIdByKey.set(key, numericId);
    if (numericId >= this.nextId) this.nextId = numericId + 1;
    return numericId;
  }

  registerRecord(recordId, options = {}) {
    return this.register(PICK_KINDS.RECORD, recordId, options);
  }

  registerLayer(layerId, options = {}) {
    return this.register(PICK_KINDS.LAYER, layerId, options);
  }

  registerEdge(edgeId, options = {}) {
    return this.register(PICK_KINDS.EDGE, edgeId, options);
  }

  getNumericId(kind, id, options = {}) {
    return this.numericIdByKey.get(makeRegistryKey(normalizePickKind(kind), id, options.scope)) || NO_PICK_ID;
  }

  getRecordNumericId(recordId, options = {}) {
    return this.getNumericId(PICK_KINDS.RECORD, recordId, options);
  }

  getLayerNumericId(layerId, options = {}) {
    return this.getNumericId(PICK_KINDS.LAYER, layerId, options);
  }

  getEdgeNumericId(edgeId, options = {}) {
    return this.getNumericId(PICK_KINDS.EDGE, edgeId, options);
  }

  lookup(numericId) {
    return this.entriesByNumericId.get(toPickId(numericId)) || null;
  }

  resolve(numericId, options = {}) {
    const pickId = toPickId(numericId);
    if (pickId === NO_PICK_ID) return null;
    const entry = this.lookup(pickId);
    if (entry || options.allowUnknown === false) return entry;
    return createUnknownPickEntry(pickId);
  }

  resolveRGB(buffer, offset = 0, options = {}) {
    return this.resolve(decodePickIdRGB(buffer, offset), options);
  }

  resolveRGBA(buffer, offset = 0, options = {}) {
    return this.resolve(decodePickIdRGBA(buffer, offset), options);
  }

  delete(kind, id, options = {}) {
    const key = makeRegistryKey(normalizePickKind(kind), id, options.scope);
    const numericId = this.numericIdByKey.get(key);
    if (numericId === undefined) return false;
    this.numericIdByKey.delete(key);
    this.entriesByNumericId.delete(numericId);
    return true;
  }

  deleteNumericId(numericId) {
    const pickId = toPickId(numericId);
    const key = this.keyForNumericId(pickId);
    if (!key) return false;
    this.numericIdByKey.delete(key);
    this.entriesByNumericId.delete(pickId);
    return true;
  }

  clear() {
    this.entriesByNumericId.clear();
    this.numericIdByKey.clear();
    this.nextId = this.start;
    return this;
  }

  encodeRGB(kind, id, out = [0, 0, 0], offset = 0, options = {}) {
    return encodePickIdRGB(this.getNumericId(kind, id, options), out, offset);
  }

  encodeRGBA(kind, id, out = [0, 0, 0, 0], offset = 0, options = {}) {
    return encodePickIdRGBA(this.getNumericId(kind, id, options), out, offset);
  }

  entries() {
    return this.entriesByNumericId.values();
  }

  toJSON() {
    return Array.from(this.entriesByNumericId.values()).map((entry) => ({ ...entry }));
  }

  allocateId() {
    while (this.entriesByNumericId.has(this.nextId)) {
      this.nextId += 1;
    }
    if (this.nextId > this.maxId) {
      throw new Error(`PickingRegistry exhausted ids up to ${this.maxId}.`);
    }
    const numericId = this.nextId;
    this.nextId += 1;
    return numericId;
  }

  keyForNumericId(numericId) {
    const entry = this.entriesByNumericId.get(toPickId(numericId));
    return entry ? makeRegistryKey(entry.kind, entry.id, entry.scope) : null;
  }
}

export class PickRequest {
  constructor(options = {}) {
    const pointerPixel = readPointerPixel(options.pointer);
    const pixel = options.pixel || options.screen || pointerPixel;

    this.x = finiteNumber(options.x ?? pixel?.x, 0);
    this.y = finiteNumber(options.y ?? pixel?.y, 0);
    this.radius = Math.max(0, finiteNumber(options.radius, DEFAULT_PICK_RADIUS));
    this.pointer = options.pointer || null;
    this.camera = options.camera || null;
    this.viewport = options.viewport || null;
    this.registry = options.registry || null;
    this.layers = options.layers || null;
    this.target = options.target || null;
    this.mode = options.mode || "auto";
    this.source = options.source || (this.pointer ? "pointer" : "programmatic");
    this.includeInvisible = Boolean(options.includeInvisible);
    this.timestamp = options.timestamp ?? options.time ?? null;
  }

  static fromPointer(pointer, options = {}) {
    return new PickRequest({ ...options, pointer });
  }

  toJSON() {
    return {
      x: this.x,
      y: this.y,
      radius: this.radius,
      mode: this.mode,
      source: this.source,
      includeInvisible: this.includeInvisible,
      timestamp: this.timestamp,
    };
  }
}

export class PickResult {
  constructor(options = {}) {
    const entry = options.entry || null;
    const numericId = toPickId(options.numericId ?? options.pickId ?? entry?.numericId ?? NO_PICK_ID);

    this.hit = options.hit ?? (numericId !== NO_PICK_ID || Boolean(entry));
    this.numericId = numericId;
    this.kind = options.kind ?? entry?.kind ?? null;
    this.id = options.id ?? entry?.id ?? null;
    this.recordId = options.recordId ?? entry?.recordId ?? null;
    this.layerId = options.layerId ?? entry?.layerId ?? null;
    this.edgeId = options.edgeId ?? entry?.edgeId ?? null;
    this.entry = entry;
    this.point = options.point || null;
    this.pixel = options.pixel || options.screen || null;
    this.distance = finiteNumber(options.distance, this.hit ? 0 : Infinity);
    this.distanceSquared = finiteNumber(options.distanceSquared, this.distance * this.distance);
    this.depth = options.depth ?? null;
    this.source = options.source || "unknown";
    this.request = options.request || null;
    this.raw = options.raw || null;
  }

  static none(request = null, options = {}) {
    return new PickResult({
      ...options,
      hit: false,
      numericId: NO_PICK_ID,
      request,
      source: options.source || "none",
      distance: Infinity,
      distanceSquared: Infinity,
    });
  }

  static fromEntry(entry, options = {}) {
    if (!entry) return PickResult.none(options.request || null, options);
    return new PickResult({
      ...options,
      hit: true,
      numericId: entry.numericId,
      entry,
      kind: entry.kind,
      id: entry.id,
      recordId: entry.recordId,
      layerId: entry.layerId,
      edgeId: entry.edgeId,
    });
  }

  toJSON() {
    return {
      hit: this.hit,
      numericId: this.numericId,
      kind: this.kind,
      id: this.id,
      recordId: this.recordId,
      layerId: this.layerId,
      edgeId: this.edgeId,
      pixel: this.pixel,
      distance: this.distance,
      depth: this.depth,
      source: this.source,
    };
  }
}

export function createPickRequest(options = {}) {
  return new PickRequest(options);
}

export function createPickResult(options = {}) {
  return new PickResult(options);
}

export function pickResultFromNumericId(numericId, registry, options = {}) {
  const pickId = toPickId(numericId);
  if (pickId === NO_PICK_ID) return PickResult.none(options.request || null, options);
  const entry = registry?.resolve ? registry.resolve(pickId) : null;
  if (entry) return PickResult.fromEntry(entry, { ...options, numericId: pickId });
  return new PickResult({ ...options, hit: true, numericId: pickId, source: options.source || "buffer" });
}

export function pickResultFromBuffer(buffer, registry, options = {}) {
  const numericId = decodePickId(buffer, options);
  return pickResultFromNumericId(numericId, registry, { ...options, source: options.source || "buffer" });
}

export function pickNearestProjectedPoint(requestInput, points, options = {}) {
  const request = requestInput instanceof PickRequest ? requestInput : new PickRequest(requestInput);
  const camera = options.camera || request.camera;
  const registry = options.registry || request.registry;
  const radius = Math.max(0, finiteNumber(options.radius, request.radius));
  const maxDistanceSquared = Number.isFinite(radius) ? radius * radius : Infinity;
  const includeInvisible = Boolean(options.includeInvisible ?? request.includeInvisible);
  const project = camera && typeof camera.projectToPixel === "function"
    ? (position) => camera.projectToPixel(position, {})
    : null;

  let bestItem = null;
  let bestPixel = null;
  let bestDistanceSquared = maxDistanceSquared;

  if (!points || typeof points[Symbol.iterator] !== "function") {
    return PickResult.none(request, { source: "cpu-nearest" });
  }

  for (const item of points) {
    const pixel = resolveProjectedPixel(item, project, options);
    if (!pixel) continue;
    if (!includeInvisible && pixel.visible === false) continue;

    const dx = pixel.x - request.x;
    const dy = pixel.y - request.y;
    const distanceSquared = dx * dx + dy * dy;

    if (distanceSquared <= bestDistanceSquared) {
      bestDistanceSquared = distanceSquared;
      bestItem = item;
      bestPixel = {
        x: pixel.x,
        y: pixel.y,
        depth: pixel.depth ?? null,
        visible: pixel.visible !== false,
      };
    }
  }

  if (!bestItem) {
    return PickResult.none(request, { source: "cpu-nearest" });
  }

  const entry = resolvePointEntry(bestItem, registry);
  const numericId = entry?.numericId ?? readPointNumericId(bestItem);
  return new PickResult({
    hit: true,
    numericId,
    entry,
    kind: entry?.kind ?? inferPointKind(bestItem),
    id: entry?.id ?? bestItem.id ?? null,
    recordId: entry?.recordId ?? bestItem.recordId ?? null,
    layerId: entry?.layerId ?? bestItem.layerId ?? null,
    edgeId: entry?.edgeId ?? bestItem.edgeId ?? null,
    point: bestItem,
    pixel: bestPixel,
    depth: bestPixel.depth,
    distanceSquared: bestDistanceSquared,
    distance: Math.sqrt(bestDistanceSquared),
    source: "cpu-nearest",
    request,
    raw: bestItem,
  });
}

export class PickingPass {
  constructor(options = {}) {
    this.name = options.name || "picking-pass";
    this.enabled = options.enabled !== false;
    this.clear = options.clear !== false;
    this.clearDepth = options.clearDepth !== false;
    this.clearColor = options.clearColor || [0, 0, 0, 0];
    this.target = options.target || null;
    this.registry = options.registry || new PickingRegistry(options.registryOptions);
    this.layers = options.layers || null;
    this.width = Math.max(1, Math.floor(finiteNumber(options.width, 1)));
    this.height = Math.max(1, Math.floor(finiteNumber(options.height, 1)));
    this.contractVersion = PICKING_PASS_CONTRACT_VERSION;
  }

  setSize(width, height) {
    this.width = Math.max(1, Math.floor(finiteNumber(width, 1)));
    this.height = Math.max(1, Math.floor(finiteNumber(height, 1)));
    if (this.target && typeof this.target.setSize === "function") {
      this.target.setSize(this.width, this.height);
    }
    return this;
  }

  setTarget(target) {
    this.target = target || null;
    return this;
  }

  setLayers(layers) {
    this.layers = layers || null;
    return this;
  }

  createLayerContext(context = {}) {
    return {
      ...context,
      pass: this,
      target: this.target,
      registry: this.registry,
      picking: this.toDescriptor(),
      encodePickIdRGB,
      encodePickIdRGBA,
      encodePickIdRGBFloat,
      encodePickIdRGBAFloat,
    };
  }

  bindTarget(context = {}) {
    const gl = context.gl;
    if (this.target && typeof this.target.bind === "function") {
      this.target.bind();
    } else if (gl && this.target?.framebuffer) {
      gl.bindFramebuffer(gl.FRAMEBUFFER, this.target.framebuffer);
      gl.viewport(0, 0, this.target.width || this.width, this.target.height || this.height);
    }

    if (gl && this.clear) {
      gl.clearColor(this.clearColor[0], this.clearColor[1], this.clearColor[2], this.clearColor[3]);
      let mask = gl.COLOR_BUFFER_BIT;
      if (this.clearDepth) {
        gl.clearDepth(1);
        mask |= gl.DEPTH_BUFFER_BIT;
      }
      gl.clear(mask);
    }
    return this.target;
  }

  render(context = {}, layers = this.layers) {
    if (!this.enabled) return this.target;
    this.bindTarget(context);
    const layerContext = this.createLayerContext(context);
    const renderLayers = resolveLayerList(layers, context);

    for (const layer of renderLayers) {
      if (!layer || layer.visible === false || layer.enabled === false) continue;
      if (typeof layer.renderPicking === "function") {
        layer.renderPicking(layerContext);
      } else if (typeof layer.renderPickIds === "function") {
        layer.renderPickIds(layerContext);
      }
    }

    return this.target;
  }

  toDescriptor() {
    return {
      name: this.name,
      version: this.contractVersion,
      target: this.target,
      clearColor: this.clearColor.slice ? this.clearColor.slice() : Array.from(this.clearColor),
      clearDepth: this.clearDepth,
      methods: ["renderPicking(context)", "renderPickIds(context)"],
      idFormat: "uint-rgba-little-endian",
      noHitId: NO_PICK_ID,
      maxRgbId: MAX_RGB_PICK_ID,
      maxRgbaId: MAX_RGBA_PICK_ID,
    };
  }
}

export function createPickingPass(options = {}) {
  return new PickingPass(options);
}

function assertPickId(value, maxId, label) {
  const numericId = Number(value);
  if (!Number.isInteger(numericId) || numericId < 0 || numericId > maxId) {
    throw new RangeError(`${label} must be an integer between 0 and ${maxId}.`);
  }
  return numericId;
}

function toPickId(value) {
  const numericId = Number(value);
  return Number.isInteger(numericId) && numericId > 0 ? numericId : NO_PICK_ID;
}

function byteAt(buffer, offset) {
  const value = Number(buffer[offset]);
  if (!Number.isFinite(value)) return 0;
  return Math.max(0, Math.min(255, Math.floor(value)));
}

function finiteNumber(value, fallback = 0) {
  const number = Number(value);
  return Number.isFinite(number) ? number : fallback;
}

function normalizePickKind(kind) {
  const normalized = PICK_KIND_ALIASES[String(kind || PICK_KINDS.RECORD).toLowerCase()];
  if (!normalized) {
    throw new Error(`Unsupported picking kind "${kind}". Expected record, layer, or edge.`);
  }
  return normalized;
}

function makeRegistryKey(kind, id, scope = null) {
  return JSON.stringify([kind, scope ?? null, stableId(id)]);
}

function stableId(id) {
  if (id == null) throw new Error("Picking id cannot be null or undefined.");
  if (typeof id === "string" || typeof id === "number" || typeof id === "boolean") return String(id);
  try {
    return JSON.stringify(id);
  } catch {
    return String(id);
  }
}

function createPickEntry(numericId, kind, id, options = {}) {
  return {
    numericId,
    kind,
    id,
    recordId: options.recordId ?? (kind === PICK_KINDS.RECORD ? id : null),
    layerId: options.layerId ?? (kind === PICK_KINDS.LAYER ? id : null),
    edgeId: options.edgeId ?? (kind === PICK_KINDS.EDGE ? id : null),
    scope: options.scope ?? null,
    payload: options.payload ?? options.data ?? null,
  };
}

function createUnknownPickEntry(numericId) {
  return {
    numericId,
    kind: null,
    id: null,
    recordId: null,
    layerId: null,
    edgeId: null,
    scope: null,
    payload: null,
  };
}

function readPointerPixel(pointer) {
  if (!pointer) return null;
  if (pointer.pixel && isFinitePoint(pointer.pixel)) return { x: pointer.pixel.x, y: pointer.pixel.y };
  if (isFinitePoint(pointer)) return { x: pointer.x, y: pointer.y };
  if (pointer.css && isFinitePoint(pointer.css)) return { x: pointer.css.x, y: pointer.css.y };
  return null;
}

function resolveProjectedPixel(item, project, options) {
  const screenPoint = resolveScreenPoint(item, options.pointAccessor || options.screenAccessor);
  if (screenPoint) return screenPoint;

  const position = resolveWorldPosition(item, options.positionAccessor);
  if (!position || !project) return null;
  return project(position);
}

function resolveScreenPoint(item, accessor) {
  const point = typeof accessor === "function" ? accessor(item) : null;
  if (isFinitePoint(point)) return point;
  if (item?.screen && isFinitePoint(item.screen)) return item.screen;
  if (item?.pixel && isFinitePoint(item.pixel)) return item.pixel;
  if (isFinitePoint(item) && item.z == null) return item;
  if ((Array.isArray(item) || ArrayBuffer.isView(item)) && item.length === 2) {
    return { x: finiteNumber(item[0]), y: finiteNumber(item[1]) };
  }
  return null;
}

function resolveWorldPosition(item, accessor) {
  const position = typeof accessor === "function" ? accessor(item) : null;
  if (isPosition(position)) return normalizeWorldPosition(position);
  if (isPosition(item?.position)) return normalizeWorldPosition(item.position);
  if (isPosition(item?.world)) return normalizeWorldPosition(item.world);
  if (isPosition(item?.xyz)) return normalizeWorldPosition(item.xyz);
  if (isPosition(item)) return normalizeWorldPosition(item);
  return null;
}

function isFinitePoint(point) {
  return point && Number.isFinite(Number(point.x)) && Number.isFinite(Number(point.y));
}

function isPosition(position) {
  if (!position) return false;
  if (Array.isArray(position) || ArrayBuffer.isView(position)) {
    return position.length >= 3
      && Number.isFinite(Number(position[0]))
      && Number.isFinite(Number(position[1]))
      && Number.isFinite(Number(position[2]));
  }
  return Number.isFinite(Number(position.x))
    && Number.isFinite(Number(position.y))
    && Number.isFinite(Number(position.z));
}

function normalizeWorldPosition(position) {
  if (Array.isArray(position) || ArrayBuffer.isView(position)) return position;
  return [Number(position.x), Number(position.y), Number(position.z)];
}

function readPointNumericId(item) {
  const value = item?.numericId ?? item?.pickId ?? item?.pickingId;
  return toPickId(value);
}

function resolvePointEntry(item, registry) {
  const numericId = readPointNumericId(item);
  if (numericId && registry?.resolve) return registry.resolve(numericId);
  if (item?.recordId != null && registry?.getRecordNumericId) {
    const recordPickId = registry.getRecordNumericId(item.recordId, { scope: item.scope });
    return recordPickId ? registry.resolve(recordPickId) : pointEntryFromItem(item);
  }
  if (item?.edgeId != null && registry?.getEdgeNumericId) {
    const edgePickId = registry.getEdgeNumericId(item.edgeId, { scope: item.scope });
    return edgePickId ? registry.resolve(edgePickId) : pointEntryFromItem(item);
  }
  if (item?.layerId != null && registry?.getLayerNumericId) {
    const layerPickId = registry.getLayerNumericId(item.layerId, { scope: item.scope });
    return layerPickId ? registry.resolve(layerPickId) : pointEntryFromItem(item);
  }
  return pointEntryFromItem(item);
}

function pointEntryFromItem(item) {
  if (!item || typeof item !== "object") return null;
  const kind = inferPointKind(item);
  const id = kind === PICK_KINDS.EDGE
    ? item.edgeId
    : kind === PICK_KINDS.LAYER
      ? item.layerId
      : item.recordId ?? item.id;
  if (id == null && !readPointNumericId(item)) return null;
  return createPickEntry(readPointNumericId(item), kind || PICK_KINDS.RECORD, id ?? readPointNumericId(item), item);
}

function inferPointKind(item) {
  if (!item || typeof item !== "object") return null;
  if (item.edgeId != null) return PICK_KINDS.EDGE;
  if (item.recordId != null) return PICK_KINDS.RECORD;
  if (item.layerId != null) return PICK_KINDS.LAYER;
  return null;
}

function resolveLayerList(layers, context) {
  if (layers && typeof layers[Symbol.iterator] === "function") return layers;
  const sceneLayers = context?.scene?.layers;
  if (sceneLayers && typeof sceneLayers[Symbol.iterator] === "function") return sceneLayers;
  return [];
}
