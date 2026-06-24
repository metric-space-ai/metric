import { createMiniatureSceneBundle } from "./composition.js";
import { createMetricMiniaturePalette } from "./palette.js";
import { createMiniatureStagePreset } from "./presets.js";
import { createMiniatureStyleProfile } from "./profile.js";
import { createMiniatureRuntimeOptions } from "./runtime-options.js";
import { createMiniaturePhotographicStyle } from "./style.js";

const HERO_CAMERA = Object.freeze({
  fov: 38,
  position: Object.freeze([0, -4.55, 1.82]),
  target: Object.freeze([0, 0.08, 0.05]),
  near: 0.02,
  far: 180,
  orbit: Object.freeze({
    enableDamping: true,
    dampingFactor: 0.055,
    autoRotate: false,
    minPitch: -1.18,
    maxPitch: 1.06,
  }),
});

const HERO_FOCUS = Object.freeze({
  axis: "horizontal",
  yRatio: 0.44,
  radiusRatio: 0.27,
  focusBandRatio: 0.058,
  blurRadius: 6.15,
  blurCurve: 1.62,
  gradientRadius: 210,
});

const HERO_GROUNDING = Object.freeze({
  groundY: -0.58,
  ground: Object.freeze({
    alpha: 0.54,
    gridScale: 6,
    gridAlpha: 0.3,
    axisAlpha: 0.22,
    fade: 0.92,
    stageTilt: -0.16,
    stageScale: Object.freeze([0.96, 0.72]),
    sheen: 0.1,
    matte: 0.86,
    contactShade: 0.24,
    horizonFade: 0.22,
  }),
  projection: Object.freeze({
    alpha: 0.7,
    softness: 0.78,
    footprintStretch: 2.36,
    footprintSkew: -0.42,
    footprintCore: 0.18,
    footprintFalloff: 1.12,
    colorMix: 0.78,
    shadowDensity: 1.66,
    coreDensity: 0.62,
    edgeTint: 0.56,
    shadowDirection: Object.freeze([-0.48, 0.88]),
    shadowTail: 0.78,
    shadowTailStrength: 0.52,
    contactHardness: 0.62,
    surfaceLift: 0.018,
    pointPixelScale: 20,
  }),
  contact: Object.freeze({
    alpha: 0.64,
    pointPixelScale: 28,
    radiusScale: 1.42,
    heightScale: 0.32,
    footprintStretch: 2.38,
    footprintSkew: -0.42,
    colorMix: 0.56,
    shadowDensity: 1.84,
    coreDensity: 0.72,
    edgeTint: 0.48,
    shadowDirection: Object.freeze([-0.48, 0.88]),
    shadowTail: 0.78,
    shadowTailStrength: 0.5,
    contactHardness: 0.68,
    surfaceLift: 0.016,
  }),
});

const HERO_POSTPROCESS = Object.freeze({
  bloom: Object.freeze({
    enabled: true,
    intensity: 0.12,
    strength: 0.12,
    radius: 1.08,
    threshold: 0.94,
    knee: 0.16,
    iterations: 3,
    downsample: 2,
    saturation: 0.74,
    warmth: 0.08,
    clamp: 0.78,
  }),
  postFx: Object.freeze({
    cameraDof: Object.freeze({
      enabled: true,
      amount: 0.76,
      focusDistance: 8.4,
      aperture: 0.72,
      maxBlur: 5.6,
      focalRange: 0.18,
      quietDesaturation: 0.1,
      quietExposure: 0.05,
      focusClarity: 0.04,
      vignette: 0.08,
    }),
    grade: Object.freeze({
      enabled: true,
      contrast: 1.1,
      saturation: 1.04,
      vibrance: 0.14,
      temperature: 0.05,
      tint: -0.012,
      highlightCompression: 0.16,
    }),
    vignette: Object.freeze({
      enabled: true,
      amount: 0.12,
      radius: 0.82,
      softness: 0.76,
      color: Object.freeze([0.84, 0.87, 0.82]),
      exposurePreserve: 0.22,
    }),
    miniatureFrame: Object.freeze({
      enabled: true,
      amount: 0.46,
      focusClarity: 0.36,
      focusLift: 0.022,
      stageGlow: 0.1,
      groundShade: 0.16,
      edgeFade: 0.1,
      warmth: 0.055,
      floorRatio: 0.53,
      stageCenter: Object.freeze([0.5, 0.51]),
      stageScale: Object.freeze([0.96, 0.68]),
      stageTilt: -0.18,
      floorSheen: 0.1,
      floorFalloff: 0.72,
      backlight: 0.16,
      stageSpotlight: 0.3,
      stageShadow: 0.32,
      subjectIsolation: 0.4,
      microContrast: 0.3,
      lensAberration: 0.008,
      apertureVignette: 0.11,
      focusDesaturation: 0.05,
      shadowColor: Object.freeze([0.5, 0.58, 0.62]),
      highlightColor: Object.freeze([1, 0.97, 0.88]),
    }),
    filmGrain: Object.freeze({
      enabled: false,
      intensity: 0.012,
      size: 1.4,
      lumaResponse: 0.65,
    }),
  }),
});

const HERO_MATERIALS = Object.freeze({
  sample: Object.freeze({
    pointPixelScale: 12,
    minPointSize: 2,
    maxPointSize: 48,
    alpha: 0.9,
    ambient: 0.54,
    pointLight: 0.44,
    sphereShade: 0.68,
    gloss: 0.26,
    edgeShade: 0.42,
    focusBoost: 0.38,
    phasePulse: 0.04,
  }),
  mesh: Object.freeze({
    ambient: 0.62,
    pointLight: 0.88,
    specular: 0.22,
    specularPower: 48,
    roughness: 0.48,
    metalness: 0.16,
    shadowSoftness: 0.28,
    baseLift: 0.025,
    faceContrast: 0.34,
    verticalGradient: 0.22,
  }),
  line: Object.freeze({
    alpha: 0.74,
    thickness: 1.55,
    feather: 0.34,
    edgeFeather: 0.38,
    emission: 0.18,
    rimLight: 0.34,
    coreGlow: 0.22,
    tubeShade: 0.52,
    flowStrength: 0.12,
    flowScale: 3.1,
    flowSpeed: 0.22,
    saturation: 1.08,
    depthShade: 0.26,
    shadowTint: "#7f979f",
    highlightColor: "#fff0d0",
  }),
  field: Object.freeze({
    alpha: 0.78,
    feather: 0.34,
    contour: 0.2,
    glow: 0.14,
  }),
});

const HERO_STYLE_MOTION = Object.freeze({
  mode: "studio-drift",
  durationMs: 20000,
  focus: Object.freeze({
    enabled: true,
    yAmplitude: 0.012,
    radiusAmplitude: 0.02,
  }),
  camera: Object.freeze({
    enabled: true,
    yawAmplitude: 0.018,
    pitchAmplitude: 0.006,
    respectInteraction: true,
  }),
  postprocess: Object.freeze({
    enabled: true,
    frameAmountAmplitude: 0.012,
    focusLiftAmplitude: 0.004,
    stageSpotlightAmplitude: 0.018,
    stageShadowAmplitude: 0.018,
    lensAberrationAmplitude: 0.0015,
    apertureVignetteAmplitude: 0.012,
  }),
});

/**
 * Creates a full-frame photographic miniature stage for METRIC captures.
 *
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniatureHeroStage(options = {}) {
  const palette = options.palette || createMetricMiniaturePalette(options.paletteOptions || {});
  return createMiniatureStagePreset({
    id: options.id || "metric-miniature-hero-stage",
    variant: options.variant || "white",
    palette,
    background: options.background || "#eef1e9",
    camera: deepMerge(HERO_CAMERA, options.camera || {}),
    focus: deepMerge(HERO_FOCUS, options.focus || options.tiltShift || {}),
    materials: deepMerge(HERO_MATERIALS, options.materials || {}),
    grounding: deepMerge(HERO_GROUNDING, options.grounding || {}),
    lightRig: options.lightRig || "whiteStage",
    postprocess: deepMerge(HERO_POSTPROCESS, options.postprocess || {}),
    interaction: {
      pointerFocus: true,
      orbitDamping: true,
      ...(options.interaction || {}),
    },
    metadata: {
      purpose: "full-frame metric-space capture",
      referenceExamples: [
        "metric focus-depth study tilt-shift focus line",
        "metric ground-light study ground relation",
        "metric instanced-field study lit instanced fields",
        "metric motion study motion grammar",
      ],
      ...(options.metadata || {}),
    },
    ...(options.stage || {}),
  });
}

/**
 * Creates a serializable full-frame miniature style profile.
 *
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniatureHeroProfile(options = {}) {
  const stage = options.stage || createMiniatureHeroStage(options.stageOptions || options);
  return createMiniatureStyleProfile({
    id: options.id || "metric-miniature-hero-profile",
    variant: options.variant || stage.variant || "white",
    stage,
    fit: deepMerge({ targetSpan: 2.74 }, options.fit || {}),
    ground: options.ground === false ? false : deepMerge({ padding: 0.64 }, options.ground || {}),
    contacts: options.contacts ?? true,
    motion: options.motion ?? "miniature",
    styleMotion: options.styleMotion === false
      ? "none"
      : deepMerge(HERO_STYLE_MOTION, options.styleMotion || {}),
    metadata: {
      purpose: "full-frame metric-space capture profile",
      ...(options.metadata || {}),
    },
  });
}

/**
 * Creates a style coordinator for full-frame miniature rendering.
 *
 * @param {object} [options]
 * @returns {import("./style.js").MiniaturePhotographicStyle}
 */
export function createMiniatureHeroStyle(options = {}) {
  const profile = options.profile || createMiniatureHeroProfile(options.profileOptions || options);
  return createMiniaturePhotographicStyle({
    ...options,
    profile,
    fit: options.fit ?? profile.fit,
    ground: options.ground ?? profile.ground,
    contacts: options.contacts ?? profile.contacts,
    motion: options.motion ?? profile.motion,
    styleMotion: options.styleMotion ?? profile.styleMotion,
  });
}

/**
 * Stages arbitrary layer descriptors for a full-frame miniature capture.
 *
 * @param {object[]|object} descriptors
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniatureHeroSceneBundle(descriptors, options = {}) {
  const stage = options.stage || createMiniatureHeroStage(options.stageOptions || options);
  return createMiniatureSceneBundle(descriptors, {
    ...options,
    stage,
    fit: deepMerge({ targetSpan: 2.74 }, options.fit || {}),
    ground: options.ground === false
      ? false
      : deepMerge({ groundY: stage.grounding?.groundY, padding: 0.64 }, options.ground || {}),
    contacts: options.contacts ?? true,
    motion: options.motion ?? "miniature",
    metadata: {
      fullFrameMiniatureCapture: true,
      ...(options.metadata || {}),
    },
  });
}

/**
 * Creates runtime options and the style objects needed for a full-frame canvas.
 *
 * @param {object} [options]
 * @returns {{profile: object, style: object, stage: object, runtimeOptions: object}}
 */
export function createMiniatureHeroRuntimeOptions(options = {}) {
  const profile = options.profile || createMiniatureHeroProfile(options.profileOptions || options);
  const style = options.style || createMiniatureHeroStyle({
    ...options,
    profile,
  });
  const setup = createMiniatureRuntimeOptions({
    ...options,
    stage: options.stage || style.stage,
    layerFactory: options.layerFactory || style.layerFactory,
    layers: {
      ...(options.layers || {}),
      factory: options.layerFactory || options.layers?.factory || style.layerFactory,
    },
  });
  return {
    profile,
    style,
    stage: setup.stage,
    runtimeOptions: setup.runtimeOptions,
  };
}

function deepMerge(base, override) {
  if (override === false) return false;
  if (!isPlainObject(base) || !isPlainObject(override)) {
    return clonePlainObject(override ?? base);
  }
  const next = clonePlainObject(base);
  for (const [key, value] of Object.entries(override || {})) {
    next[key] = isPlainObject(value) && isPlainObject(next[key])
      ? deepMerge(next[key], value)
      : clonePlainObject(value);
  }
  return next;
}

function clonePlainObject(value) {
  if (value == null || typeof value !== "object") return value;
  if (Array.isArray(value)) return value.map(clonePlainObject);
  if (ArrayBuffer.isView(value)) return Array.from(value);
  const next = {};
  for (const [key, entry] of Object.entries(value)) {
    next[key] = clonePlainObject(entry);
  }
  return next;
}

function isPlainObject(value) {
  return Boolean(value && typeof value === "object" && !Array.isArray(value) && !ArrayBuffer.isView(value));
}
