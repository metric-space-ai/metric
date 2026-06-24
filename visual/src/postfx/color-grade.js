import { FullscreenQuadPass } from "../native-postprocess/index.js";
import { COLOR_GRADE_FRAGMENT_SHADER } from "./shaders.js";

export const PHOTOGRAPHIC_MINIATURE_GRADE = Object.freeze({
  exposure: 0,
  contrast: 1.08,
  contrastPivot: 0.48,
  saturation: 0.94,
  vibrance: 0.08,
  temperature: 0.08,
  tint: -0.015,
  gamma: 1,
  blackPoint: 0.006,
  whitePoint: 1,
  highlightCompression: 0.16,
  lift: [0.002, 0.002, 0.004],
  gain: [1.02, 1.01, 0.99],
  shadowTint: [0.94, 0.98, 1.04],
  highlightTint: [1.04, 1.015, 0.965],
  shadowTintStrength: 0.08,
  highlightTintStrength: 0.1,
});

export class ColorGradePass extends FullscreenQuadPass {
  constructor(gl, options = {}) {
    const grade = normalizeGradeOptions({ ...PHOTOGRAPHIC_MINIATURE_GRADE, ...options });
    super(gl, {
      ...options,
      fragmentShader: COLOR_GRADE_FRAGMENT_SHADER,
      uniforms: {
        uExposure: grade.exposure,
        uContrast: grade.contrast,
        uContrastPivot: grade.contrastPivot,
        uSaturation: grade.saturation,
        uVibrance: grade.vibrance,
        uTemperature: grade.temperature,
        uTint: grade.tint,
        uGamma: grade.gamma,
        uBlackPoint: grade.blackPoint,
        uWhitePoint: grade.whitePoint,
        uHighlightCompression: grade.highlightCompression,
        uLift: grade.lift,
        uGain: grade.gain,
        uShadowTint: grade.shadowTint,
        uHighlightTint: grade.highlightTint,
        uShadowTintStrength: grade.shadowTintStrength,
        uHighlightTintStrength: grade.highlightTintStrength,
        ...(options.uniforms || {}),
      },
    });
    this.grade = grade;
  }

  setGrade(options = {}) {
    if ("enabled" in options) this.enabled = options.enabled !== false;
    this.grade = normalizeGradeOptions({ ...this.grade, ...options });
    applyGradeUniforms(this, this.grade);
    return this;
  }

  setExposure(value) {
    this.grade.exposure = clampNumber(value, -3, 3, this.grade.exposure);
    return this.setUniform("uExposure", this.grade.exposure);
  }

  setContrast(value) {
    this.grade.contrast = clampNumber(value, 0, 3, this.grade.contrast);
    return this.setUniform("uContrast", this.grade.contrast);
  }

  setSaturation(value) {
    this.grade.saturation = clampNumber(value, 0, 2.5, this.grade.saturation);
    return this.setUniform("uSaturation", this.grade.saturation);
  }

  setVibrance(value) {
    this.grade.vibrance = clampNumber(value, -1, 1, this.grade.vibrance);
    return this.setUniform("uVibrance", this.grade.vibrance);
  }
}

export function createPhotographicMiniatureGrade(overrides = {}) {
  return normalizeGradeOptions({ ...PHOTOGRAPHIC_MINIATURE_GRADE, ...overrides });
}

function applyGradeUniforms(pass, grade) {
  pass
    .setUniform("uExposure", grade.exposure)
    .setUniform("uContrast", grade.contrast)
    .setUniform("uContrastPivot", grade.contrastPivot)
    .setUniform("uSaturation", grade.saturation)
    .setUniform("uVibrance", grade.vibrance)
    .setUniform("uTemperature", grade.temperature)
    .setUniform("uTint", grade.tint)
    .setUniform("uGamma", grade.gamma)
    .setUniform("uBlackPoint", grade.blackPoint)
    .setUniform("uWhitePoint", grade.whitePoint)
    .setUniform("uHighlightCompression", grade.highlightCompression)
    .setUniform("uLift", grade.lift)
    .setUniform("uGain", grade.gain)
    .setUniform("uShadowTint", grade.shadowTint)
    .setUniform("uHighlightTint", grade.highlightTint)
    .setUniform("uShadowTintStrength", grade.shadowTintStrength)
    .setUniform("uHighlightTintStrength", grade.highlightTintStrength);
}

function normalizeGradeOptions(options) {
  return {
    exposure: clampNumber(options.exposure, -3, 3, 0),
    contrast: clampNumber(options.contrast, 0, 3, 1),
    contrastPivot: clampNumber(options.contrastPivot, 0, 1, 0.5),
    saturation: clampNumber(options.saturation, 0, 2.5, 1),
    vibrance: clampNumber(options.vibrance, -1, 1, 0),
    temperature: clampNumber(options.temperature, -1, 1, 0),
    tint: clampNumber(options.tint, -1, 1, 0),
    gamma: clampNumber(options.gamma, 0.1, 4, 1),
    blackPoint: clampNumber(options.blackPoint, 0, 0.95, 0),
    whitePoint: clampNumber(options.whitePoint, 0.05, 4, 1),
    highlightCompression: clampNumber(options.highlightCompression, 0, 1, 0),
    lift: color3(options.lift, [0, 0, 0], -1, 1),
    gain: color3(options.gain, [1, 1, 1], 0, 4),
    shadowTint: color3(options.shadowTint, [1, 1, 1], 0, 4),
    highlightTint: color3(options.highlightTint, [1, 1, 1], 0, 4),
    shadowTintStrength: clampNumber(options.shadowTintStrength, 0, 1, 0),
    highlightTintStrength: clampNumber(options.highlightTintStrength, 0, 1, 0),
  };
}

function color3(value, fallback, min, max) {
  if (!value) return fallback.slice();
  if (Array.isArray(value)) {
    return [
      clampNumber(value[0], min, max, fallback[0]),
      clampNumber(value[1], min, max, fallback[1]),
      clampNumber(value[2], min, max, fallback[2]),
    ];
  }
  return [
    clampNumber(value.r ?? value.x, min, max, fallback[0]),
    clampNumber(value.g ?? value.y, min, max, fallback[1]),
    clampNumber(value.b ?? value.z, min, max, fallback[2]),
  ];
}

function clampNumber(value, min, max, fallback) {
  const number = Number(value);
  if (!Number.isFinite(number)) return fallback;
  return Math.min(max, Math.max(min, number));
}
