import { miniatureColor } from "./palette.js";

const DEFAULT_LIGHTING = Object.freeze({
  ambientColor: Object.freeze([1, 1, 1]),
  ambientIntensity: 0.52,
  lightColor: Object.freeze([1, 1, 1]),
  lightIntensity: 0.48,
  lightDirection: Object.freeze([0.35, 0.85, 0.45]),
});

const MAX_NATIVE_LIGHTS = 4;

/**
 * Converts a miniature light rig into shader-ready plain uniforms.
 *
 * The public style vocabulary describes scene lights. Native layers do not
 * instantiate a retained scene graph, so they consume the same rig as compact
 * material uniforms.
 *
 * @param {object} [rig]
 * @returns {object}
 */
export function flattenMiniatureLightRig(rig = {}) {
  const ambient = rig.ambient || {};
  const lights = normalizeLightList(rig);
  const key = lights[0] || {};
  const direction = normalizeDirection(key.direction || key.position || DEFAULT_LIGHTING.lightDirection);
  const ambientColor = color3(ambient.color, DEFAULT_LIGHTING.ambientColor);
  const lightColor = color3(key.color, DEFAULT_LIGHTING.lightColor);
  const packed = packLights(lights);

  return {
    lightingRig: rig.mode || "ambient-plus-key",
    ambientColor,
    ambientIntensity: finiteNumber(ambient.intensity, DEFAULT_LIGHTING.ambientIntensity),
    lightColor,
    lightIntensity: finiteNumber(key.intensity, DEFAULT_LIGHTING.lightIntensity),
    lightDirection: direction,
    pointLightDirections: packed.directions,
    pointLightColors: packed.colors,
    pointLightIntensities: packed.intensities,
    pointLightCount: packed.count,
  };
}

/**
 * Adds shader-ready light uniforms to a miniature material.
 *
 * @param {object} material
 * @param {object} [rig]
 * @returns {object}
 */
export function applyMiniatureLightingToMaterial(material = {}, rig = {}) {
  if (material.lighting === "unlit") return material;
  const lighting = flattenMiniatureLightRig(rig);
  return {
    ...material,
    ...lighting,
    ambient: finiteNumber(material.ambient, lighting.ambientIntensity),
    pointLight: finiteNumber(material.pointLight, lighting.lightIntensity),
  };
}

function firstPointLight(rig) {
  return Array.isArray(rig.points) && rig.points.length ? rig.points[0] : null;
}

function normalizeLightList(rig = {}) {
  const lights = [];
  if (rig.key) lights.push(rig.key);
  if (Array.isArray(rig.points)) lights.push(...rig.points);
  if (rig.fill) lights.push(rig.fill);
  if (!lights.length && firstPointLight(rig)) lights.push(firstPointLight(rig));
  if (!lights.length) {
    lights.push({
      color: DEFAULT_LIGHTING.lightColor,
      intensity: DEFAULT_LIGHTING.lightIntensity,
      direction: DEFAULT_LIGHTING.lightDirection,
    });
  }
  return lights.slice(0, MAX_NATIVE_LIGHTS);
}

function packLights(lights) {
  const directions = new Float32Array(MAX_NATIVE_LIGHTS * 3);
  const colors = new Float32Array(MAX_NATIVE_LIGHTS * 3);
  const intensities = new Float32Array(MAX_NATIVE_LIGHTS);
  let count = 0;

  for (let index = 0; index < MAX_NATIVE_LIGHTS; index += 1) {
    const light = lights[index] || null;
    const direction = normalizeDirection(light?.direction || light?.position || DEFAULT_LIGHTING.lightDirection);
    const color = color3(light?.color, DEFAULT_LIGHTING.lightColor);
    const intensity = light ? finiteNumber(light.intensity, DEFAULT_LIGHTING.lightIntensity) : 0;
    directions.set(direction, index * 3);
    colors.set(color, index * 3);
    intensities[index] = intensity;
    if (light && intensity > 0) count += 1;
  }

  return { directions, colors, intensities, count };
}

function color3(value, fallback) {
  const rgba = miniatureColor(value, [...fallback, 1]);
  return [rgba[0], rgba[1], rgba[2]];
}

function normalizeDirection(value) {
  const x = finiteNumber(value?.[0], value?.x, DEFAULT_LIGHTING.lightDirection[0]);
  const y = finiteNumber(value?.[1], value?.y, DEFAULT_LIGHTING.lightDirection[1]);
  const z = finiteNumber(value?.[2], value?.z, DEFAULT_LIGHTING.lightDirection[2]);
  const length = Math.hypot(x, y, z) || 1;
  return [x / length, y / length, z / length];
}

function finiteNumber(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number)) return number;
  }
  return 0;
}
