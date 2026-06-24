import { finiteRelationValues } from "./relation-source.js";

/**
 * Summarize finite numeric relation values.
 *
 * @param {ArrayLike<number>} values
 * @returns {{count:number, min:number, max:number, range:number, mean:number}}
 */
export function summarizeFiniteValues(values) {
  let count = 0;
  let min = Number.POSITIVE_INFINITY;
  let max = Number.NEGATIVE_INFINITY;
  let sum = 0;

  for (let index = 0; index < values.length; index += 1) {
    const value = Number(values[index]);
    if (!Number.isFinite(value)) continue;
    count += 1;
    sum += value;
    if (value < min) min = value;
    if (value > max) max = value;
  }

  if (!count) {
    return { count: 0, min: Number.NaN, max: Number.NaN, range: Number.NaN, mean: Number.NaN };
  }
  return { count, min, max, range: max - min, mean: sum / count };
}

/**
 * Build a simple min/max scaler for exported relation values.
 *
 * @param {ArrayLike<number>|object} valuesOrSource
 * @param {object} [options]
 * @param {number} [options.min]
 * @param {number} [options.max]
 * @param {boolean} [options.invert=false] Invert the 0..1 output.
 * @returns {{kind:string, min:number, max:number, scale:function(number):number, unscale:function(number):number}}
 */
export function createMinMaxScaler(valuesOrSource, options = {}) {
  const values = valuesFrom(valuesOrSource, options);
  const summary = summarizeFiniteValues(values);
  const min = Number.isFinite(options.min) ? Number(options.min) : summary.min;
  const max = Number.isFinite(options.max) ? Number(options.max) : summary.max;
  const range = max - min;
  const invert = options.invert === true;

  return {
    kind: "minMax",
    min,
    max,
    scale(value) {
      const number = Number(value);
      if (!Number.isFinite(number) || !Number.isFinite(range) || range <= 0) return 0;
      const unit = clamp01((number - min) / range);
      return invert ? 1 - unit : unit;
    },
    unscale(unit) {
      const next = invert ? 1 - Number(unit) : Number(unit);
      return min + clamp01(next) * range;
    },
  };
}

/**
 * Build a quantile scaler that maps values to their empirical rank.
 *
 * This is useful when a small number of large relation values would otherwise
 * flatten visible contrast in matrix or edge encodings.
 *
 * @param {ArrayLike<number>|object} valuesOrSource
 * @param {object} [options]
 * @param {number} [options.buckets=256]
 * @param {boolean} [options.invert=false]
 * @returns {{kind:string, breakpoints:Float64Array, scale:function(number):number}}
 */
export function createQuantileScaler(valuesOrSource, options = {}) {
  const breakpoints = computeQuantileBreakpoints(valuesFrom(valuesOrSource, options), options.buckets || 256);
  const invert = options.invert === true;

  return {
    kind: "quantile",
    breakpoints,
    scale(value) {
      const number = Number(value);
      if (!Number.isFinite(number) || breakpoints.length === 0) return 0;
      let low = 0;
      let high = breakpoints.length - 1;
      while (low < high) {
        const mid = (low + high) >> 1;
        if (number <= breakpoints[mid]) high = mid;
        else low = mid + 1;
      }
      const unit = breakpoints.length <= 1 ? 0 : low / (breakpoints.length - 1);
      return invert ? 1 - unit : unit;
    },
  };
}

/**
 * Compute quantile breakpoints from finite values.
 *
 * @param {ArrayLike<number>} values
 * @param {number} buckets
 * @returns {Float64Array}
 */
export function computeQuantileBreakpoints(values, buckets = 256) {
  const sorted = [];
  for (let index = 0; index < values.length; index += 1) {
    const value = Number(values[index]);
    if (Number.isFinite(value)) sorted.push(value);
  }
  sorted.sort((a, b) => a - b);
  if (!sorted.length) return new Float64Array();

  const count = Math.max(2, Math.floor(Number(buckets) || 2));
  const out = new Float64Array(count);
  for (let index = 0; index < count; index += 1) {
    const position = (index / (count - 1)) * (sorted.length - 1);
    const lower = Math.floor(position);
    const upper = Math.ceil(position);
    const t = position - lower;
    out[index] = sorted[lower] * (1 - t) + sorted[upper] * t;
  }
  return out;
}

/**
 * Scale values into a Float32Array in the 0..1 interval.
 *
 * @param {ArrayLike<number>} values
 * @param {{scale:function(number):number}} scaler
 * @returns {Float32Array}
 */
export function scaleValuesToUnit(values, scaler) {
  const out = new Float32Array(values.length);
  for (let index = 0; index < values.length; index += 1) {
    out[index] = clamp01(scaler.scale(values[index]));
  }
  return out;
}

/**
 * Construct a scaler from a name or existing scaler object.
 *
 * @param {ArrayLike<number>|object} valuesOrSource
 * @param {string|object|function} [scale]
 * @returns {object}
 */
export function resolveRelationScaler(valuesOrSource, scale = "minMax") {
  if (typeof scale === "function") return { kind: "custom", scale };
  if (scale && typeof scale.scale === "function") return scale;
  if (scale === "quantile" || scale?.kind === "quantile") {
    return createQuantileScaler(valuesOrSource, scale || {});
  }
  return createMinMaxScaler(valuesOrSource, scale || {});
}

export function clamp01(value) {
  const number = Number(value);
  if (!Number.isFinite(number)) return 0;
  return Math.max(0, Math.min(1, number));
}

function valuesFrom(valuesOrSource, options) {
  if (ArrayBuffer.isView(valuesOrSource) || Array.isArray(valuesOrSource)) return valuesOrSource;
  return finiteRelationValues(valuesOrSource, options);
}

