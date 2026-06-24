import { MINIATURE_STYLE_ID, MINIATURE_STYLE_SCHEMA } from "./constants.js";

const DEFAULT_ACCENTS = Object.freeze([
  "#567f9f",
  "#d25b63",
  "#2b9a8f",
  "#e39b45",
  "#76638d",
  "#49785f",
  "#b96b3c",
  "#425c73",
]);

const DEFAULT_PALETTE = Object.freeze({
  id: "metric-miniature-default",
  schema: MINIATURE_STYLE_SCHEMA,
  style: MINIATURE_STYLE_ID,
  background: "#ffffff",
  foreground: "#17242d",
  neutral: Object.freeze({
    0: "#ffffff",
    50: "#f6f8f7",
    100: "#e8eeeb",
    200: "#d3ded9",
    300: "#b7c8c1",
    500: "#71857f",
    700: "#344941",
    900: "#15221f",
  }),
  accents: DEFAULT_ACCENTS,
  sample: "#547b9c",
  sampleFocus: "#ffffff",
  mesh: "#8aa6b7",
  meshHighlight: "#bfd0da",
  surface: "#f7f8f4",
  surfaceDisplacement: "#b6c6cf",
  ground: "#f3f5f0",
  groundGrid: "#b8c5bf",
  axisX: "#77a7bf",
  axisZ: "#d8a06a",
  projection: "rgba(69, 104, 128, 0.34)",
  edge: "rgba(32, 48, 58, 0.62)",
  glow: "#f7efac",
  heatLow: "rgba(232, 244, 247, 0.28)",
  heatHigh: "rgba(218, 92, 76, 0.92)",
});

/**
 * Creates the native METRIC miniature palette.
 *
 * The palette is a renderer-facing scene vocabulary, not a CSS theme. Values
 * are intentionally plain color tokens so WebGL layer descriptors can consume
 * them without DOM, CSS, external renderer, or source visual references artifacts.
 *
 * @param {object} [options]
 * @param {string} [options.id]
 * @param {string} [options.background]
 * @param {string[]} [options.accents]
 * @param {object} [options.colors] color token overrides.
 * @returns {object}
 */
export function createMetricMiniaturePalette(options = {}) {
  const colors = options.colors || {};
  return deepFreeze({
    ...DEFAULT_PALETTE,
    id: options.id || DEFAULT_PALETTE.id,
    background: options.background || colors.background || DEFAULT_PALETTE.background,
    foreground: colors.foreground || DEFAULT_PALETTE.foreground,
    accents: Array.isArray(options.accents) ? options.accents.slice() : DEFAULT_PALETTE.accents.slice(),
    sample: colors.sample || colors.point || DEFAULT_PALETTE.sample,
    sampleFocus: colors.sampleFocus || colors.pointFocus || DEFAULT_PALETTE.sampleFocus,
    mesh: colors.mesh || DEFAULT_PALETTE.mesh,
    meshHighlight: colors.meshHighlight || DEFAULT_PALETTE.meshHighlight,
    surface: colors.surface || DEFAULT_PALETTE.surface,
    surfaceDisplacement: colors.surfaceDisplacement || DEFAULT_PALETTE.surfaceDisplacement,
    ground: colors.ground || DEFAULT_PALETTE.ground,
    groundGrid: colors.groundGrid || DEFAULT_PALETTE.groundGrid,
    axisX: colors.axisX || DEFAULT_PALETTE.axisX,
    axisZ: colors.axisZ || DEFAULT_PALETTE.axisZ,
    projection: colors.projection || DEFAULT_PALETTE.projection,
    edge: colors.edge || DEFAULT_PALETTE.edge,
    glow: colors.glow || DEFAULT_PALETTE.glow,
    heatLow: colors.heatLow || DEFAULT_PALETTE.heatLow,
    heatHigh: colors.heatHigh || DEFAULT_PALETTE.heatHigh,
    metadata: {
      ...(options.metadata || {}),
      nativeMetricStyle: true,
      sourceGrammar: "source visual references photographic miniature visual grammar",
    },
  });
}

/**
 * Parses a palette color into normalized RGBA floats.
 *
 * This helper is only for style descriptors and shader uniforms. It does not
 * map metric values to colors.
 *
 * @param {string|number[]|Float32Array} value
 * @param {number[]} [fallback]
 * @returns {number[]}
 */
export function miniatureColor(value, fallback = [1, 1, 1, 1]) {
  if (Array.isArray(value) || ArrayBuffer.isView(value)) {
    const source = Array.from(value);
    const divisor = Math.max(source[0] || 0, source[1] || 0, source[2] || 0) > 1 ? 255 : 1;
    return [
      clamp01((Number(source[0]) || 0) / divisor),
      clamp01((Number(source[1]) || 0) / divisor),
      clamp01((Number(source[2]) || 0) / divisor),
      clamp01(source.length > 3 ? Number(source[3]) : 1),
    ];
  }

  if (typeof value === "string") {
    const hex = value.trim();
    if (/^#([0-9a-f]{3}|[0-9a-f]{6})$/i.test(hex)) {
      const full = hex.length === 4
        ? `#${hex[1]}${hex[1]}${hex[2]}${hex[2]}${hex[3]}${hex[3]}`
        : hex;
      return [
        parseInt(full.slice(1, 3), 16) / 255,
        parseInt(full.slice(3, 5), 16) / 255,
        parseInt(full.slice(5, 7), 16) / 255,
        1,
      ];
    }
    const rgba = /^rgba?\(([^)]+)\)$/i.exec(hex);
    if (rgba) {
      const parts = rgba[1].split(",").map((part) => Number(part.trim()));
      return [
        clamp01((parts[0] || 0) / 255),
        clamp01((parts[1] || 0) / 255),
        clamp01((parts[2] || 0) / 255),
        clamp01(Number.isFinite(parts[3]) ? parts[3] : 1),
      ];
    }
  }

  return fallback.slice();
}

function clamp01(value) {
  return Math.max(0, Math.min(1, Number.isFinite(value) ? value : 0));
}

function deepFreeze(value) {
  if (!value || typeof value !== "object" || Object.isFrozen(value)) return value;
  Object.freeze(value);
  for (const entry of Object.values(value)) deepFreeze(entry);
  return value;
}
