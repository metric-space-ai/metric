import { flattenVectors } from "./view-utils.js";

/**
 * Compute a uniform center+scale transform that fits a set of vec3 positions
 * into a target radius. The transform is uniform (single scale on all axes plus
 * a translation), so it preserves relative metric structure — it never distorts
 * exported coordinates, it only frames them for a composed miniature stage.
 *
 * @param {Map|Array|object} source id->vec3 source consumable by flattenVectors
 * @param {string[]} ids
 * @param {object} [options]
 * @param {number} [options.targetRadius=1.6]
 * @param {boolean} [options.groundAlign=true] drop the lowest point onto groundY
 * @param {number} [options.groundY=0]
 * @returns {{center:number[], scale:number, transform:(vec3:number[])=>number[]}}
 */
export function computePositionFit(source, ids, options = {}) {
  const targetRadius = Number.isFinite(Number(options.targetRadius)) ? Number(options.targetRadius) : 1.6;
  const flat = flattenVectors(source, ids, 3);
  const min = [Infinity, Infinity, Infinity];
  const max = [-Infinity, -Infinity, -Infinity];
  for (let index = 0; index < flat.length; index += 3) {
    for (let axis = 0; axis < 3; axis += 1) {
      const value = flat[index + axis];
      if (!Number.isFinite(value)) continue;
      min[axis] = Math.min(min[axis], value);
      max[axis] = Math.max(max[axis], value);
    }
  }
  for (let axis = 0; axis < 3; axis += 1) {
    if (!Number.isFinite(min[axis])) min[axis] = 0;
    if (!Number.isFinite(max[axis])) max[axis] = 0;
  }
  const center = [(min[0] + max[0]) / 2, (min[1] + max[1]) / 2, (min[2] + max[2]) / 2];
  const span = Math.max(max[0] - min[0], max[1] - min[1], max[2] - min[2], 1e-6);
  const scale = targetRadius / (span / 2);
  const groundAlign = options.groundAlign !== false;
  const groundY = Number.isFinite(Number(options.groundY)) ? Number(options.groundY) : 0;
  const liftedMinY = (min[1] - center[1]) * scale;
  const groundShift = groundAlign ? groundY - liftedMinY : 0;

  const transform = (vec3) => [
    ((Number(vec3?.[0]) || 0) - center[0]) * scale,
    ((Number(vec3?.[1]) || 0) - center[1]) * scale + groundShift,
    ((Number(vec3?.[2]) || 0) - center[2]) * scale,
  ];

  return { center, scale, span, bounds: { min, max }, targetRadius, groundShift, transform };
}

/**
 * Apply a transform to every entry of an id->vec3 Map (or array/object source),
 * returning a new Map of fitted positions.
 *
 * @param {Map|Array|object} source
 * @param {string[]} ids
 * @param {(vec3:number[])=>number[]} transform
 * @returns {Map<string, number[]>}
 */
export function applyPositionFit(source, ids, transform) {
  const flat = flattenVectors(source, ids, 3);
  const out = new Map();
  for (let index = 0; index < ids.length; index += 1) {
    const offset = index * 3;
    out.set(String(ids[index]), transform([flat[offset], flat[offset + 1], flat[offset + 2]]));
  }
  return out;
}
