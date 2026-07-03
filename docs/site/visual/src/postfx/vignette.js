import { FullscreenQuadPass } from "../native-postprocess/index.js";
import { VIGNETTE_FRAGMENT_SHADER } from "./shaders.js";

export const PHOTOGRAPHIC_VIGNETTE = Object.freeze({
  amount: 0.18,
  radius: 0.56,
  softness: 0.58,
  roundness: 0.42,
  center: [0.5, 0.5],
  color: [0.72, 0.76, 0.82],
  exposurePreserve: 0.24,
});

export class VignettePass extends FullscreenQuadPass {
  constructor(gl, options = {}) {
    const vignette = normalizeVignetteOptions({ ...PHOTOGRAPHIC_VIGNETTE, ...options });
    super(gl, {
      ...options,
      fragmentShader: VIGNETTE_FRAGMENT_SHADER,
      uniforms: {
        uAmount: vignette.amount,
        uRadius: vignette.radius,
        uSoftness: vignette.softness,
        uRoundness: vignette.roundness,
        uCenter: vignette.center,
        uColor: vignette.color,
        uExposurePreserve: vignette.exposurePreserve,
        ...(options.uniforms || {}),
      },
    });
    this.vignette = vignette;
  }

  setVignette(options = {}) {
    if ("enabled" in options) this.enabled = options.enabled !== false;
    this.vignette = normalizeVignetteOptions({ ...this.vignette, ...options });
    return this
      .setUniform("uAmount", this.vignette.amount)
      .setUniform("uRadius", this.vignette.radius)
      .setUniform("uSoftness", this.vignette.softness)
      .setUniform("uRoundness", this.vignette.roundness)
      .setUniform("uCenter", this.vignette.center)
      .setUniform("uColor", this.vignette.color)
      .setUniform("uExposurePreserve", this.vignette.exposurePreserve);
  }

  setAmount(value) {
    this.vignette.amount = clampNumber(value, 0, 1, this.vignette.amount);
    return this.setUniform("uAmount", this.vignette.amount);
  }
}

export function createPhotographicVignette(overrides = {}) {
  return normalizeVignetteOptions({ ...PHOTOGRAPHIC_VIGNETTE, ...overrides });
}

function normalizeVignetteOptions(options) {
  return {
    amount: clampNumber(options.amount, 0, 1, 0.18),
    radius: clampNumber(options.radius, 0, 1.5, 0.56),
    softness: clampNumber(options.softness, 0.001, 1.5, 0.58),
    roundness: clampNumber(options.roundness, 0, 1, 0.42),
    center: point2(options.center, [0.5, 0.5]),
    color: color3(options.color, [0.72, 0.76, 0.82]),
    exposurePreserve: clampNumber(options.exposurePreserve, 0, 1, 0.24),
  };
}

function point2(value, fallback) {
  if (!value) return fallback.slice();
  if (Array.isArray(value)) return [clampNumber(value[0], 0, 1, fallback[0]), clampNumber(value[1], 0, 1, fallback[1])];
  return [clampNumber(value.x, 0, 1, fallback[0]), clampNumber(value.y, 0, 1, fallback[1])];
}

function color3(value, fallback) {
  if (!value) return fallback.slice();
  if (Array.isArray(value)) {
    return [
      clampNumber(value[0], 0, 4, fallback[0]),
      clampNumber(value[1], 0, 4, fallback[1]),
      clampNumber(value[2], 0, 4, fallback[2]),
    ];
  }
  return [
    clampNumber(value.r ?? value.x, 0, 4, fallback[0]),
    clampNumber(value.g ?? value.y, 0, 4, fallback[1]),
    clampNumber(value.b ?? value.z, 0, 4, fallback[2]),
  ];
}

function clampNumber(value, min, max, fallback) {
  const number = Number(value);
  if (!Number.isFinite(number)) return fallback;
  return Math.min(max, Math.max(min, number));
}
