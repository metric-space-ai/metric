import { FullscreenQuadPass } from "../native-postprocess/index.js";
import { FILM_GRAIN_FRAGMENT_SHADER } from "./shaders.js";

export const SUBTLE_FILM_GRAIN = Object.freeze({
  intensity: 0.018,
  size: 1.4,
  lumaResponse: 0.65,
  time: 0,
});

export class FilmGrainPass extends FullscreenQuadPass {
  constructor(gl, options = {}) {
    const grain = normalizeGrainOptions({ ...SUBTLE_FILM_GRAIN, ...options });
    super(gl, {
      ...options,
      enabled: options.enabled === true,
      fragmentShader: FILM_GRAIN_FRAGMENT_SHADER,
      uniforms: {
        uIntensity: grain.intensity,
        uSize: grain.size,
        uLumaResponse: grain.lumaResponse,
        uTime: grain.time,
        ...(options.uniforms || {}),
      },
    });
    this.grain = grain;
  }

  setGrain(options = {}) {
    if ("enabled" in options) this.enabled = options.enabled === true;
    this.grain = normalizeGrainOptions({ ...this.grain, ...options });
    return this
      .setUniform("uIntensity", this.grain.intensity)
      .setUniform("uSize", this.grain.size)
      .setUniform("uLumaResponse", this.grain.lumaResponse)
      .setUniform("uTime", this.grain.time);
  }

  setTime(value) {
    this.grain.time = Number.isFinite(Number(value)) ? Number(value) : this.grain.time;
    return this.setUniform("uTime", this.grain.time);
  }
}

export function createSubtleFilmGrain(overrides = {}) {
  return normalizeGrainOptions({ ...SUBTLE_FILM_GRAIN, ...overrides });
}

function normalizeGrainOptions(options) {
  return {
    intensity: clampNumber(options.intensity, 0, 0.12, 0.018),
    size: clampNumber(options.size, 1, 6, 1.4),
    lumaResponse: clampNumber(options.lumaResponse, 0, 1, 0.65),
    time: Number.isFinite(Number(options.time)) ? Number(options.time) : 0,
  };
}

function clampNumber(value, min, max, fallback) {
  const number = Number(value);
  if (!Number.isFinite(number)) return fallback;
  return Math.min(max, Math.max(min, number));
}
