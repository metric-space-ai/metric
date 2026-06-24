const TAU = Math.PI * 2;
const UINT32_MAX_PLUS_ONE = 4294967296;

export function clamp01(value) {
  const number = Number(value);
  if (!Number.isFinite(number)) return 0;
  if (number <= 0) return 0;
  if (number >= 1) return 1;
  return number;
}

export function lerp(a, b, t) {
  return a + (b - a) * t;
}

export function linear(t) {
  return clamp01(t);
}

export function stepStart(t) {
  return clamp01(t) > 0 ? 1 : 0;
}

export function stepEnd(t) {
  return clamp01(t) >= 1 ? 1 : 0;
}

export function smoothstep(t) {
  const x = clamp01(t);
  return x * x * (3 - 2 * x);
}

export function smootherstep(t) {
  const x = clamp01(t);
  return x * x * x * (x * (x * 6 - 15) + 10);
}

export function easeInQuad(t) {
  const x = clamp01(t);
  return x * x;
}

export function easeOutQuad(t) {
  const x = clamp01(t);
  return 1 - (1 - x) * (1 - x);
}

export function easeInOutQuad(t) {
  const x = clamp01(t);
  return x < 0.5 ? 2 * x * x : 1 - Math.pow(-2 * x + 2, 2) / 2;
}

export function easeInCubic(t) {
  const x = clamp01(t);
  return x * x * x;
}

export function easeOutCubic(t) {
  const x = clamp01(t);
  return 1 - Math.pow(1 - x, 3);
}

export function easeInOutCubic(t) {
  const x = clamp01(t);
  return x < 0.5 ? 4 * x * x * x : 1 - Math.pow(-2 * x + 2, 3) / 2;
}

export const Easing = Object.freeze({
  linear,
  stepStart,
  stepEnd,
  smoothstep,
  smoothStep: smoothstep,
  smootherstep,
  smootherStep: smootherstep,
  easeInQuad,
  easeOutQuad,
  easeInOutQuad,
  easeInCubic,
  easeOutCubic,
  easeInOutCubic,
});

export const EASING = Easing;

export function resolveEasing(easing) {
  if (typeof easing === "function") return easing;
  if (typeof easing === "string" && Easing[easing]) return Easing[easing];
  return linear;
}

export function applyEasing(easing, t) {
  return resolveEasing(easing)(t);
}

export class KeyframeTrack {
  constructor(timesOrOptions = [], values, options = {}) {
    const config = isArrayLike(timesOrOptions)
      ? { ...options, times: timesOrOptions, values }
      : (timesOrOptions || {});
    this.times = toFloat64Array(config.times || []);
    this.values = toFloat32Array(config.values || []);
    this.itemSize = positiveInteger(config.itemSize, 1);
    this.easing = resolveEasing(config.easing);
    this.defaultValue = Number.isFinite(Number(config.defaultValue)) ? Number(config.defaultValue) : 0;
    this.count = Math.min(this.times.length, Math.floor(this.values.length / this.itemSize));
    this._cursor = 0;
    this._scratch = new Float32Array(this.itemSize);
  }

  get duration() {
    return this.count > 0 ? this.times[this.count - 1] - this.times[0] : 0;
  }

  get startTime() {
    return this.count > 0 ? this.times[0] : 0;
  }

  get endTime() {
    return this.count > 0 ? this.times[this.count - 1] : 0;
  }

  sample(time, out) {
    const target = out || this._scratch;
    this.sampleInto(time, target);
    return this.itemSize === 1 && out == null ? target[0] : target;
  }

  sampleInto(time, out) {
    if (!out) {
      throw new Error("KeyframeTrack.sampleInto requires an output array");
    }
    if (this.count === 0) {
      fillArray(out, this.defaultValue);
      return out;
    }
    if (this.count === 1 || time <= this.times[0]) {
      copyFrame(this.values, out, 0, this.itemSize);
      return out;
    }
    const lastIndex = this.count - 1;
    if (time >= this.times[lastIndex]) {
      copyFrame(this.values, out, lastIndex, this.itemSize);
      return out;
    }

    const index = this.findSegment(time);
    const nextIndex = index + 1;
    const start = this.times[index];
    const end = this.times[nextIndex];
    const span = end - start;
    const t = span > 0 ? this.easing((time - start) / span) : 0;
    const base = index * this.itemSize;
    const nextBase = nextIndex * this.itemSize;
    for (let axis = 0; axis < this.itemSize; axis += 1) {
      out[axis] = lerp(this.values[base + axis], this.values[nextBase + axis], t);
    }
    return out;
  }

  findSegment(time) {
    const lastSegment = this.count - 2;
    let cursor = this._cursor;
    if (cursor < 0 || cursor > lastSegment || time < this.times[cursor] || time >= this.times[cursor + 1]) {
      cursor = findKeyframeSegment(this.times, this.count, time);
    }
    this._cursor = cursor;
    return cursor;
  }
}

export function findKeyframeSegment(times, count, time) {
  let low = 0;
  let high = count - 2;
  while (low <= high) {
    const mid = (low + high) >> 1;
    if (time < times[mid]) {
      high = mid - 1;
    } else if (time >= times[mid + 1]) {
      low = mid + 1;
    } else {
      return mid;
    }
  }
  return Math.max(0, Math.min(count - 2, low));
}

export function interpolateTypedArrays(from, to, out, t, options = {}) {
  if (!out || !isArrayLike(out)) {
    throw new Error("interpolateTypedArrays requires an output typed array or array");
  }
  const fromArray = channelArray(from);
  const toArray = channelArray(to);
  const easing = resolveEasing(options.easing);
  const amount = easing(clamp01(t));

  if (!fromArray && !toArray) {
    fillArray(out, options.fallback ?? 0);
    return out;
  }
  if (!fromArray) {
    copyArray(toArray, out, options.fallback ?? 0);
    return out;
  }
  if (!toArray) {
    copyArray(fromArray, out, options.fallback ?? 0);
    return out;
  }

  const limit = Math.min(fromArray.length, toArray.length, out.length);
  for (let index = 0; index < limit; index += 1) {
    out[index] = lerp(fromArray[index], toArray[index], amount);
  }

  const fallback = options.fallback ?? 0;
  for (let index = limit; index < out.length; index += 1) {
    if (index < fromArray.length) {
      out[index] = fromArray[index];
    } else if (index < toArray.length) {
      out[index] = toArray[index];
    } else {
      out[index] = fallback;
    }
  }
  return out;
}

export function interpolateChannels(fromChannel, toChannel, outChannelOrArray, t, options = {}) {
  const out = channelArray(outChannelOrArray) || outChannelOrArray;
  return interpolateTypedArrays(fromChannel, toChannel, out, t, options);
}

export class ChannelInterpolator {
  constructor(options = {}) {
    this.from = options.from || null;
    this.to = options.to || null;
    this.output = options.output || createOutputArray(this.from, this.to);
    this.easing = options.easing || "linear";
    this.progress = 0;
  }

  setSources(from, to) {
    this.from = from || null;
    this.to = to || null;
    return this;
  }

  update(progress, out = this.output) {
    this.progress = clamp01(progress);
    interpolateTypedArrays(this.from, this.to, out, this.progress, { easing: this.easing });
    return out;
  }
}

export class CoordinateMorphState {
  constructor(options = {}) {
    this.itemSize = positiveInteger(options.itemSize || options.dimension, 3);
    this.easing = options.easing || "linear";
    this.progress = clamp01(options.progress);
    this.from = resolveCoordinateSource(options.from || options.source || options.fromCoordinate, options);
    this.to = resolveCoordinateSource(options.to || options.target || options.toCoordinate, options);
    this.output = options.output || createOutputArray(this.from, this.to);
    this.count = this.itemSize > 0 ? Math.floor(this.output.length / this.itemSize) : 0;
    this.mode = "empty";
    this.update(this.progress);
  }

  setSources(from, to, options = {}) {
    this.from = resolveCoordinateSource(from, { ...options, itemSize: this.itemSize });
    this.to = resolveCoordinateSource(to, { ...options, itemSize: this.itemSize });
    this.count = this.itemSize > 0 ? Math.floor(this.output.length / this.itemSize) : 0;
    return this.update(this.progress);
  }

  setProgress(progress) {
    return this.update(progress);
  }

  update(progress = this.progress) {
    this.progress = clamp01(progress);
    const hasFrom = Boolean(this.from && this.from.length);
    const hasTo = Boolean(this.to && this.to.length);
    if (!hasFrom && !hasTo) {
      this.mode = "empty";
      fillArray(this.output, 0);
      return this.output;
    }
    if (!hasFrom || !hasTo) {
      this.mode = "constant";
      copyArray(hasFrom ? this.from : this.to, this.output, 0);
      return this.output;
    }
    this.mode = "morph";
    interpolateTypedArrays(this.from, this.to, this.output, this.progress, { easing: this.easing });
    return this.output;
  }

  descriptor(out = {}) {
    out.mode = this.mode;
    out.progress = this.progress;
    out.itemSize = this.itemSize;
    out.count = this.count;
    out.length = this.output.length;
    return out;
  }
}

export function flattenCoordinateState(coordinate, recordIds, output, options = {}) {
  const itemSize = positiveInteger(options.itemSize || coordinate?.dimension, 3);
  const positions = coordinate?.record_positions || coordinate?.recordPositions || coordinate?.positions || coordinate;
  if (!positions) {
    return output || new Float32Array(0);
  }
  if (isArrayLike(positions) && typeof positions[0] === "number") {
    const out = output || new Float32Array(positions.length);
    copyArray(positions, out, 0);
    return out;
  }

  const ids = recordIds || recordIdsFromCoordinate(coordinate);
  const out = output || new Float32Array(ids.length * itemSize);
  const byId = Array.isArray(positions) ? mapPositionsById(positions) : null;
  for (let index = 0; index < ids.length; index += 1) {
    const entry = byId ? byId.get(String(ids[index])) : valueForKey(positions, ids[index]);
    writeVector(out, index * itemSize, vectorFromPosition(entry), itemSize, options.fallback ?? 0);
  }
  return out;
}

export function recordIdsFromCoordinate(coordinate) {
  const positions = coordinate?.record_positions || coordinate?.recordPositions || [];
  const ids = [];
  if (!Array.isArray(positions)) return ids;
  for (let index = 0; index < positions.length; index += 1) {
    const entry = positions[index];
    const id = entry && (entry.record_id ?? entry.recordId ?? entry.id);
    ids.push(id == null ? String(index) : String(id));
  }
  return ids;
}

export class AnimatedWeightChannel {
  constructor(options = {}) {
    const count = positiveInteger(options.count, inferArrayLength(options.current || options.target || options.output));
    this.current = options.current || new Float32Array(count);
    this.start = options.start || new Float32Array(count);
    this.target = options.target || new Float32Array(count);
    this.output = options.output || this.current;
    this.durationMs = Math.max(0, Number(options.durationMs ?? 160));
    this.easing = options.easing || "smoothstep";
    this.elapsedMs = 0;
    this.active = false;
    this.length = Math.min(this.current.length, this.start.length, this.target.length, this.output.length);
    if (options.initial != null) {
      fillArray(this.current, Number(options.initial) || 0);
      fillArray(this.target, Number(options.initial) || 0);
      copyArray(this.current, this.output, 0);
    }
  }

  setTarget(values, options = {}) {
    copyArray(this.current, this.start, 0);
    if (values == null) {
      fillArray(this.target, Number(options.fallback ?? 0));
    } else {
      copyArray(values, this.target, Number(options.fallback ?? 0));
    }
    this.durationMs = Math.max(0, Number(options.durationMs ?? this.durationMs));
    this.easing = options.easing || this.easing;
    this.elapsedMs = 0;
    this.active = true;
    if (this.durationMs === 0) {
      this.snap(this.target);
    }
    return this;
  }

  setTargetAt(index, value) {
    if (index < 0 || index >= this.length) return this;
    this.target[index] = Number.isFinite(Number(value)) ? Number(value) : 0;
    return this;
  }

  snap(values = this.target) {
    copyArray(values, this.current, 0);
    copyArray(this.current, this.output, 0);
    this.elapsedMs = this.durationMs;
    this.active = false;
    return this.output;
  }

  update(deltaMs, out = this.output) {
    if (!this.active) {
      if (out !== this.current) copyArray(this.current, out, 0);
      return out;
    }
    this.elapsedMs += Math.max(0, Number(deltaMs) || 0);
    const progress = this.durationMs > 0 ? clamp01(this.elapsedMs / this.durationMs) : 1;
    const amount = resolveEasing(this.easing)(progress);
    for (let index = 0; index < this.length; index += 1) {
      this.current[index] = lerp(this.start[index], this.target[index], amount);
      out[index] = this.current[index];
    }
    if (progress >= 1) {
      this.active = false;
    }
    return out;
  }
}

export function writeIndexSetWeights(out, activeSet, options = {}) {
  const activeWeight = Number.isFinite(Number(options.activeWeight)) ? Number(options.activeWeight) : 1;
  const inactiveWeight = Number.isFinite(Number(options.inactiveWeight)) ? Number(options.inactiveWeight) : 0;
  const ids = options.ids || null;
  for (let index = 0; index < out.length; index += 1) {
    const key = ids ? ids[index] : index;
    out[index] = activeSet && activeSet.has(key) ? activeWeight : inactiveWeight;
  }
  return out;
}

export function combineFocusSelectionWeights(focus, selection, out, options = {}) {
  if (!out || !isArrayLike(out)) {
    throw new Error("combineFocusSelectionWeights requires an output typed array or array");
  }
  const focusArray = channelArray(focus);
  const selectionArray = channelArray(selection);
  const focusScale = Number.isFinite(Number(options.focusScale)) ? Number(options.focusScale) : 1;
  const selectionScale = Number.isFinite(Number(options.selectionScale)) ? Number(options.selectionScale) : 1;
  const base = Number.isFinite(Number(options.base)) ? Number(options.base) : 0;
  for (let index = 0; index < out.length; index += 1) {
    const focusValue = focusArray && index < focusArray.length ? focusArray[index] : 0;
    const selectionValue = selectionArray && index < selectionArray.length ? selectionArray[index] : 0;
    out[index] = base + focusValue * focusScale + selectionValue * selectionScale;
  }
  return out;
}

export function hashUint32(value, seed = 0) {
  let h = (Number(value) >>> 0) ^ (Number(seed) >>> 0);
  h = Math.imul(h ^ (h >>> 16), 0x7feb352d);
  h = Math.imul(h ^ (h >>> 15), 0x846ca68b);
  return (h ^ (h >>> 16)) >>> 0;
}

export function hashString(value, seed = 0) {
  const text = String(value);
  let h = (2166136261 ^ (Number(seed) >>> 0)) >>> 0;
  for (let index = 0; index < text.length; index += 1) {
    h ^= text.charCodeAt(index);
    h = Math.imul(h, 16777619);
  }
  return hashUint32(h, seed);
}

export function hashValue(value, seed = 0) {
  return typeof value === "number" && Number.isFinite(value)
    ? hashUint32(value, seed)
    : hashString(value, seed);
}

export function random01FromHash(value, seed = 0) {
  return hashValue(value, seed) / UINT32_MAX_PLUS_ONE;
}

export function deterministicPhase(value, seed = 0) {
  return random01FromHash(value, seed);
}

export function fillDeterministicPhases(out, seed = 0, offset = 0) {
  for (let index = 0; index < out.length; index += 1) {
    out[index] = deterministicPhase(index + offset, seed);
  }
  return out;
}

export function valueNoise1D(x, seed = 0) {
  const floor = Math.floor(Number(x) || 0);
  const fraction = (Number(x) || 0) - floor;
  const a = random01FromHash(floor, seed);
  const b = random01FromHash(floor + 1, seed);
  return lerp(a, b, smootherstep(fraction));
}

export function signedValueNoise1D(x, seed = 0) {
  return valueNoise1D(x, seed) * 2 - 1;
}

export function phaseOscillation(phase, timeSeconds, frequency = 1) {
  return Math.sin((Number(phase) + Number(timeSeconds) * Number(frequency)) * TAU);
}

export class AnimationScheduler {
  constructor(options = {}) {
    this.timeScale = Number.isFinite(Number(options.timeScale)) ? Number(options.timeScale) : 1;
    this.running = options.running !== false;
    this.elapsedMs = Number.isFinite(Number(options.elapsedMs)) ? Number(options.elapsedMs) : 0;
    this.entries = [];
    this._lastNow = undefined;
    this._frame = {
      scheduler: this,
      sourceTime: null,
      deltaMs: 0,
      elapsedMs: this.elapsedMs,
      localTimeMs: 0,
      progress: 0,
      iteration: 0,
      done: false,
    };
  }

  add(animation, options = {}) {
    const entry = createSchedulerEntry(animation, options);
    this.entries.push(entry);
    return entry;
  }

  remove(entry) {
    const index = this.entries.indexOf(entry);
    if (index < 0) return false;
    this.entries[index] = this.entries[this.entries.length - 1];
    this.entries.pop();
    entry.active = false;
    return true;
  }

  clear() {
    for (let index = 0; index < this.entries.length; index += 1) {
      this.entries[index].active = false;
    }
    this.entries.length = 0;
    return this;
  }

  play() {
    this.running = true;
    return this;
  }

  pause() {
    this.running = false;
    return this;
  }

  stop() {
    this.running = false;
    this.elapsedMs = 0;
    this._lastNow = undefined;
    for (let index = 0; index < this.entries.length; index += 1) {
      this.entries[index].elapsedMs = 0;
    }
    return this;
  }

  seek(elapsedMs) {
    this.elapsedMs = Math.max(0, Number(elapsedMs) || 0);
    this._lastNow = undefined;
    return this;
  }

  tick(renderLoopTime) {
    const deltaMs = deltaFromRenderTime(renderLoopTime, this._lastNow);
    if (renderLoopTime && Number.isFinite(Number(renderLoopTime.now))) {
      this._lastNow = Number(renderLoopTime.now);
    }
    return this.update(deltaMs, renderLoopTime || null);
  }

  update(deltaMs, sourceTime = null) {
    if (!this.running) return this;
    const scaledDelta = Math.max(0, Number(deltaMs) || 0) * this.timeScale;
    this.elapsedMs += scaledDelta;
    const entries = this.entries;
    for (let index = 0; index < entries.length;) {
      const entry = entries[index];
      if (!entry.active) {
        entries[index] = entries[entries.length - 1];
        entries.pop();
        continue;
      }
      if (entry.paused) {
        index += 1;
        continue;
      }
      entry.elapsedMs += scaledDelta;
      if (entry.elapsedMs < entry.delayMs) {
        index += 1;
        continue;
      }
      updateSchedulerEntry(entry, this._frame, sourceTime, scaledDelta, this.elapsedMs);
      entry.update.call(entry.target, this._frame, entry);
      if (this._frame.done && entry.removeOnComplete) {
        if (typeof entry.onComplete === "function") entry.onComplete(entry);
        entries[index] = entries[entries.length - 1];
        entries.pop();
      } else {
        index += 1;
      }
    }
    return this;
  }
}

function createSchedulerEntry(animation, options) {
  const isFunction = typeof animation === "function";
  const source = isFunction ? null : (animation || {});
  const update = isFunction ? animation : source.update;
  if (typeof update !== "function") {
    throw new Error("AnimationScheduler.add requires a function or an object with update(frame)");
  }
  const durationMs = Number(options.durationMs ?? source.durationMs ?? 0);
  const loop = options.loop ?? source.loop ?? false;
  const iterations = loop === true
    ? Infinity
    : Number.isFinite(Number(loop)) && Number(loop) > 0
      ? Math.max(1, Math.floor(Number(loop)))
      : 1;
  return {
    target: source,
    update,
    active: true,
    paused: false,
    elapsedMs: Number.isFinite(Number(options.elapsedMs)) ? Number(options.elapsedMs) : 0,
    delayMs: Math.max(0, Number(options.delayMs ?? source.delayMs ?? 0) || 0),
    durationMs: Math.max(0, Number.isFinite(durationMs) ? durationMs : 0),
    iterations,
    removeOnComplete: options.removeOnComplete ?? source.removeOnComplete ?? true,
    onComplete: options.onComplete || source.onComplete || null,
  };
}

function updateSchedulerEntry(entry, frame, sourceTime, deltaMs, schedulerElapsedMs) {
  const activeElapsed = Math.max(0, entry.elapsedMs - entry.delayMs);
  const duration = entry.durationMs;
  let iteration = 0;
  let localTimeMs = activeElapsed;
  let progress = duration > 0 ? 1 : 0;
  let done = false;
  if (duration > 0) {
    iteration = Math.floor(activeElapsed / duration);
    if (iteration >= entry.iterations) {
      iteration = entry.iterations - 1;
      localTimeMs = duration;
      progress = 1;
      done = true;
    } else {
      localTimeMs = activeElapsed - iteration * duration;
      progress = clamp01(localTimeMs / duration);
    }
  } else if (entry.iterations !== Infinity) {
    done = true;
  }
  frame.sourceTime = sourceTime;
  frame.deltaMs = deltaMs;
  frame.elapsedMs = schedulerElapsedMs;
  frame.localTimeMs = localTimeMs;
  frame.progress = progress;
  frame.iteration = iteration;
  frame.done = done;
}

function deltaFromRenderTime(renderLoopTime, lastNow) {
  if (renderLoopTime && Number.isFinite(Number(renderLoopTime.delta))) {
    return Math.max(0, Number(renderLoopTime.delta));
  }
  if (renderLoopTime && Number.isFinite(Number(renderLoopTime.now)) && Number.isFinite(lastNow)) {
    return Math.max(0, Number(renderLoopTime.now) - Number(lastNow));
  }
  return 0;
}

function resolveCoordinateSource(source, options) {
  if (!source) return null;
  const array = channelArray(source);
  if (array) return array;
  if (source.record_positions || source.recordPositions || source.positions) {
    return flattenCoordinateState(source, options.recordIds, options.outputSource, options);
  }
  return null;
}

function createOutputArray(from, to) {
  const fromArray = channelArray(from);
  const toArray = channelArray(to);
  const length = Math.max(fromArray ? fromArray.length : 0, toArray ? toArray.length : 0);
  return new Float32Array(length);
}

function channelArray(value) {
  if (!value) return null;
  if (isArrayLike(value)) return value;
  if (value.array && isArrayLike(value.array)) return value.array;
  if (value.data && isArrayLike(value.data)) return value.data;
  return null;
}

function isArrayLike(value) {
  return Array.isArray(value) || (ArrayBuffer.isView(value) && typeof value.length === "number");
}

function toFloat32Array(value) {
  if (value instanceof Float32Array) return value;
  if (!isArrayLike(value)) return new Float32Array(0);
  return new Float32Array(value);
}

function toFloat64Array(value) {
  if (value instanceof Float64Array) return value;
  if (!isArrayLike(value)) return new Float64Array(0);
  return new Float64Array(value);
}

function positiveInteger(value, fallback) {
  const number = Number(value);
  return Number.isFinite(number) && number > 0 ? Math.floor(number) : fallback;
}

function inferArrayLength(value) {
  const array = channelArray(value);
  return array ? array.length : 0;
}

function copyFrame(source, out, frameIndex, itemSize) {
  const offset = frameIndex * itemSize;
  for (let axis = 0; axis < itemSize; axis += 1) {
    out[axis] = source[offset + axis];
  }
}

function copyArray(source, out, fallback) {
  const limit = source ? Math.min(source.length, out.length) : 0;
  for (let index = 0; index < limit; index += 1) {
    const value = Number(source[index]);
    out[index] = Number.isFinite(value) ? value : fallback;
  }
  for (let index = limit; index < out.length; index += 1) {
    out[index] = fallback;
  }
  return out;
}

function fillArray(out, value) {
  if (typeof out.fill === "function") {
    out.fill(value);
  } else {
    for (let index = 0; index < out.length; index += 1) out[index] = value;
  }
  return out;
}

function mapPositionsById(positions) {
  const byId = new Map();
  for (let index = 0; index < positions.length; index += 1) {
    const entry = positions[index];
    const id = entry && (entry.record_id ?? entry.recordId ?? entry.id);
    if (id != null) byId.set(String(id), entry);
  }
  return byId;
}

function valueForKey(source, key) {
  if (!source) return undefined;
  if (source instanceof Map) return source.get(key) ?? source.get(String(key));
  return source[key] ?? source[String(key)];
}

function vectorFromPosition(entry) {
  if (!entry) return null;
  if (isArrayLike(entry) && typeof entry[0] === "number") return entry;
  return entry.position || entry.coordinates || entry.values || entry.value || null;
}

function writeVector(out, offset, values, itemSize, fallback) {
  for (let axis = 0; axis < itemSize; axis += 1) {
    const value = values && axis < values.length ? Number(values[axis]) : fallback;
    out[offset + axis] = Number.isFinite(value) ? value : fallback;
  }
}
