/*
 * Color scale utilities for METRIC Visual.
 *
 * Dependency-free replacement for the small color-scale subset needed by the
 * native visual engine. Colors are returned as linear data arrays in RGBA
 * channel order with values in [0, 1].
 */

export const CATEGORICAL_PALETTE = [
  "#2563eb",
  "#16a34a",
  "#dc2626",
  "#7c3aed",
  "#ea580c",
  "#0891b2",
  "#be123c",
  "#4f46e5",
  "#65a30d",
  "#0f766e",
  "#9333ea",
  "#475569",
];

export const COLOR_RAMPS = {
  metric: [
    [0, "#102a43"],
    [0.2, "#155e75"],
    [0.45, "#16a34a"],
    [0.72, "#f59e0b"],
    [1, "#dc2626"],
  ],
  density: [
    [0, "#f8fafc"],
    [0.28, "#bae6fd"],
    [0.54, "#38bdf8"],
    [0.78, "#2563eb"],
    [1, "#1e1b4b"],
  ],
  heat: [
    [0, "#fff7ed"],
    [0.25, "#fed7aa"],
    [0.5, "#fb923c"],
    [0.75, "#dc2626"],
    [1, "#450a0a"],
  ],
  coolWarm: [
    [0, "#1d4ed8"],
    [0.45, "#dbeafe"],
    [0.5, "#f8fafc"],
    [0.55, "#fee2e2"],
    [1, "#b91c1c"],
  ],
  neutral: [
    [0, "#f8fafc"],
    [1, "#0f172a"],
  ],
};

export function createColorScale(options = {}) {
  if (typeof options === "function") return options;
  if (options.type === "ordinal" || options.type === "categorical" || options.scale === "categorical") {
    return createOrdinalColorScale(options);
  }
  if (options.constant !== undefined || options.color !== undefined) {
    const color = parseColor(options.constant ?? options.color);
    return () => color.slice();
  }
  return createScalarColorScale(options);
}

export function createScalarColorScale(options = {}) {
  const domain = numericDomain(options.domain);
  const ramp = normalizeRamp(options.colors || options.ramp || options.name || "metric");
  const unknown = parseColor(options.unknownColor || "#94a3b8");
  const clamp = options.clamp !== false;

  const scale = (value) => {
    const number = Number(value);
    if (!Number.isFinite(number)) return unknown.slice();
    let t = (number - domain[0]) / (domain[1] - domain[0]);
    if (clamp) t = clamp01(t);
    return sampleRamp(ramp, t);
  };
  scale.domain = domain;
  scale.ramp = ramp;
  return scale;
}

export function createOrdinalColorScale(options = {}) {
  const categories = Array.isArray(options.categories) ? options.categories.slice() : [];
  const colors = (options.colors || CATEGORICAL_PALETTE).map(parseColor);
  const unknown = parseColor(options.unknownColor || "#64748b");
  const categoryIndex = new Map(categories.map((category, index) => [String(category), index]));

  const scale = (value) => {
    if (value === undefined || value === null || Number.isNaN(value)) return unknown.slice();
    const key = String(value);
    if (!categoryIndex.has(key)) categoryIndex.set(key, categoryIndex.size);
    return colors[categoryIndex.get(key) % colors.length].slice();
  };
  scale.categories = categoryIndex;
  scale.colors = colors;
  return scale;
}

export function sampleColorRamp(ramp, count) {
  const normalized = normalizeRamp(ramp);
  const total = Math.max(1, Math.floor(count));
  if (total === 1) return [sampleRamp(normalized, 0.5)];
  const colors = [];
  for (let index = 0; index < total; index += 1) {
    colors.push(sampleRamp(normalized, index / (total - 1)));
  }
  return colors;
}

export function normalizeRamp(ramp) {
  if (typeof ramp === "string") {
    return normalizeRamp(COLOR_RAMPS[ramp] || COLOR_RAMPS.metric);
  }
  if (!Array.isArray(ramp) || ramp.length === 0) {
    return normalizeRamp(COLOR_RAMPS.metric);
  }
  if (typeof ramp[0] === "string" || Array.isArray(ramp[0]) && typeof ramp[0][0] !== "number") {
    const colors = ramp.map(parseColor);
    if (colors.length === 1) return [[0, colors[0]], [1, colors[0]]];
    return colors.map((color, index) => [index / (colors.length - 1), color]);
  }
  return ramp
    .map(([stop, color]) => [clamp01(Number(stop)), parseColor(color)])
    .sort((a, b) => a[0] - b[0]);
}

export function sampleRamp(ramp, value) {
  const t = clamp01(value);
  if (t <= ramp[0][0]) return ramp[0][1].slice();
  for (let index = 1; index < ramp.length; index += 1) {
    const previous = ramp[index - 1];
    const next = ramp[index];
    if (t <= next[0]) {
      const local = (t - previous[0]) / Math.max(next[0] - previous[0], 0.000001);
      return interpolateColor(previous[1], next[1], local);
    }
  }
  return ramp[ramp.length - 1][1].slice();
}

export function interpolateColor(a, b, t) {
  const f = clamp01(t);
  return [
    lerp(a[0], b[0], f),
    lerp(a[1], b[1], f),
    lerp(a[2], b[2], f),
    lerp(a[3] ?? 1, b[3] ?? 1, f),
  ];
}

export function parseColor(input) {
  if (Array.isArray(input)) {
    return [
      normalizeChannel(input[0]),
      normalizeChannel(input[1]),
      normalizeChannel(input[2]),
      input[3] === undefined ? 1 : normalizeChannel(input[3], true),
    ];
  }
  if (input && typeof input === "object") {
    return [
      normalizeChannel(input.r),
      normalizeChannel(input.g),
      normalizeChannel(input.b),
      input.a === undefined ? 1 : normalizeChannel(input.a, true),
    ];
  }
  if (typeof input !== "string") return [0, 0, 0, 1];
  const value = input.trim();
  if (value.startsWith("#")) return parseHexColor(value);
  if (value.startsWith("rgb")) return parseRgbFunction(value);
  return parseHexColor(`#${value}`);
}

export function rgbaToHex(color, includeAlpha = false) {
  const rgba = parseColor(color);
  const parts = rgba.map((channel) => Math.round(clamp01(channel) * 255).toString(16).padStart(2, "0"));
  return `#${parts.slice(0, includeAlpha ? 4 : 3).join("")}`;
}

export function rgbaToCss(color) {
  const rgba = parseColor(color);
  return `rgba(${Math.round(rgba[0] * 255)}, ${Math.round(rgba[1] * 255)}, ${Math.round(rgba[2] * 255)}, ${rgba[3].toFixed(3)})`;
}

export function extent(values) {
  let min = Infinity;
  let max = -Infinity;
  for (const value of values) {
    const number = Number(value);
    if (!Number.isFinite(number)) continue;
    min = Math.min(min, number);
    max = Math.max(max, number);
  }
  if (!Number.isFinite(min) || !Number.isFinite(max)) return [0, 1];
  if (min === max) return [min - 1, max + 1];
  return [min, max];
}

export function numericDomain(domain) {
  if (Array.isArray(domain) && domain.length >= 2) {
    const min = Number(domain[0]);
    const max = Number(domain[1]);
    if (Number.isFinite(min) && Number.isFinite(max) && min !== max) return [min, max];
    if (Number.isFinite(min)) return [min - 1, min + 1];
  }
  return [0, 1];
}

export function clamp01(value) {
  return Math.max(0, Math.min(1, Number(value)));
}

export function lerp(a, b, t) {
  return a + (b - a) * t;
}

function parseHexColor(value) {
  const hex = value.slice(1);
  if (hex.length === 3 || hex.length === 4) {
    const r = parseInt(hex[0] + hex[0], 16);
    const g = parseInt(hex[1] + hex[1], 16);
    const b = parseInt(hex[2] + hex[2], 16);
    const a = hex.length === 4 ? parseInt(hex[3] + hex[3], 16) : 255;
    return [r / 255, g / 255, b / 255, a / 255];
  }
  if (hex.length === 6 || hex.length === 8) {
    const r = parseInt(hex.slice(0, 2), 16);
    const g = parseInt(hex.slice(2, 4), 16);
    const b = parseInt(hex.slice(4, 6), 16);
    const a = hex.length === 8 ? parseInt(hex.slice(6, 8), 16) : 255;
    return [r / 255, g / 255, b / 255, a / 255];
  }
  return [0, 0, 0, 1];
}

function parseRgbFunction(value) {
  const numbers = value.match(/[\d.]+/g) || [];
  return [
    normalizeChannel(numbers[0]),
    normalizeChannel(numbers[1]),
    normalizeChannel(numbers[2]),
    numbers[3] === undefined ? 1 : normalizeChannel(numbers[3], true),
  ];
}

function normalizeChannel(value, alpha = false) {
  const number = Number(value);
  if (!Number.isFinite(number)) return alpha ? 1 : 0;
  if (number > 1) return clamp01(number / 255);
  return clamp01(number);
}
