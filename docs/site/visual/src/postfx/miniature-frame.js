import { FullscreenQuadPass } from "../native-postprocess/index.js";

export const PHOTOGRAPHIC_MINIATURE_FRAME = Object.freeze({
  enabled: true,
  amount: 0.42,
  focusClarity: 0.18,
  focusLift: 0.035,
  stageGlow: 0.12,
  groundShade: 0.08,
  edgeFade: 0.08,
  warmth: 0.05,
  floorRatio: 0.52,
  floorColor: [0.86, 0.88, 0.84],
  skyColor: [0.98, 0.97, 0.92],
  stageCenter: [0.5, 0.52],
  stageScale: [0.9, 0.74],
  stageTilt: -0.1,
  floorSheen: 0.12,
  floorFalloff: 0.78,
  backlight: 0.16,
  stageSpotlight: 0.18,
  stageShadow: 0.16,
  subjectIsolation: 0.24,
  microContrast: 0.18,
  lensAberration: 0.006,
  apertureVignette: 0.08,
  focusDesaturation: 0.04,
  focusBand: 0.045,
  focusBlurCurve: 1.35,
  shadowColor: [0.67, 0.73, 0.77],
  highlightColor: [1.0, 0.965, 0.88],
});

export const MINIATURE_FRAME_FRAGMENT_SHADER = `
  precision mediump float;

  uniform sampler2D uTexture;
  uniform vec2 uTextureSize;
  uniform vec2 uTexelSize;
  uniform vec2 uFocusStart;
  uniform vec2 uFocusEnd;
  uniform float uFocusRadius;
  uniform float uFocusBand;
  uniform float uFocusBlurCurve;
  uniform float uAmount;
  uniform float uFocusClarity;
  uniform float uFocusLift;
  uniform float uStageGlow;
  uniform float uGroundShade;
  uniform float uEdgeFade;
  uniform float uWarmth;
  uniform float uFloorRatio;
  uniform vec3 uFloorColor;
  uniform vec3 uSkyColor;
  uniform vec2 uStageCenter;
  uniform vec2 uStageScale;
  uniform float uStageTilt;
  uniform float uFloorSheen;
  uniform float uFloorFalloff;
  uniform float uBacklight;
  uniform float uStageSpotlight;
  uniform float uStageShadow;
  uniform float uSubjectIsolation;
  uniform float uMicroContrast;
  uniform float uLensAberration;
  uniform float uApertureVignette;
  uniform float uFocusDesaturation;
  uniform vec3 uShadowColor;
  uniform vec3 uHighlightColor;
  varying vec2 vUv;

  float luma(vec3 color) {
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
  }

  float saturationAmount(vec3 color) {
    float high = max(color.r, max(color.g, color.b));
    float low = min(color.r, min(color.g, color.b));
    return high - low;
  }

  float distanceToFocusLine(vec2 pixel) {
    vec2 segment = uFocusEnd - uFocusStart;
    float segmentLength = max(length(segment), 0.0001);
    vec2 normal = vec2(-segment.y, segment.x) / segmentLength;
    return abs(dot(pixel - uFocusStart, normal));
  }

  void main() {
    vec4 source = texture2D(uTexture, vUv);
    vec3 color = source.rgb;
    float amount = clamp(uAmount, 0.0, 1.0);
    float sourceLuma = luma(color);
    float sourceSaturation = saturationAmount(color);

    vec2 pixel = vUv * uTextureSize;
    float focusDistance = distanceToFocusLine(pixel);
    float focusRadius = max(uFocusRadius, 1.0);
    float focusBand = clamp(uFocusBand, 0.0, focusRadius * 0.98);
    float focusCurve = max(uFocusBlurCurve, 0.05);
    float focusRamp = smoothstep(focusBand, max(focusRadius, focusBand + 0.0001), focusDistance);
    focusRamp = pow(focusRamp, focusCurve);
    float focusMask = 1.0 - focusRamp;
    float focusFalloff = 1.0 - pow(
      smoothstep(focusBand, max(focusRadius * 2.25, focusBand + 0.0001), focusDistance),
      max(focusCurve * 0.72, 0.05)
    );

    vec2 lensVector = vUv - vec2(0.5);
    float lensDistance = length(lensVector);
    vec2 lensDirection = normalize(lensVector + vec2(0.00001));
    float lensAberration = clamp(uLensAberration, 0.0, 1.0) * amount * focusRamp;
    vec2 chromaOffset = lensDirection * uTexelSize * lensAberration * mix(18.0, 86.0, smoothstep(0.08, 0.82, lensDistance));
    vec3 chromaColor = vec3(
      texture2D(uTexture, clamp(vUv + chromaOffset, vec2(0.0), vec2(1.0))).r,
      color.g,
      texture2D(uTexture, clamp(vUv - chromaOffset, vec2(0.0), vec2(1.0))).b
    );
    color = mix(color, chromaColor, clamp(lensAberration * 14.0, 0.0, 0.72));
    float chromaLuma = luma(color);
    color = mix(color, vec3(chromaLuma), clamp(uFocusDesaturation, 0.0, 1.0) * amount * focusRamp);
    sourceLuma = luma(color);
    sourceSaturation = saturationAmount(color);

    vec3 nearAverage = (
      texture2D(uTexture, clamp(vUv + vec2(uTexelSize.x, 0.0), vec2(0.0), vec2(1.0))).rgb +
      texture2D(uTexture, clamp(vUv - vec2(uTexelSize.x, 0.0), vec2(0.0), vec2(1.0))).rgb +
      texture2D(uTexture, clamp(vUv + vec2(0.0, uTexelSize.y), vec2(0.0), vec2(1.0))).rgb +
      texture2D(uTexture, clamp(vUv - vec2(0.0, uTexelSize.y), vec2(0.0), vec2(1.0))).rgb
    ) * 0.25;
    vec3 detail = color - nearAverage;
    float subjectMask = clamp(
      smoothstep(0.04, 0.22, sourceSaturation) +
      smoothstep(0.18, 0.86, sourceLuma) * (1.0 - smoothstep(0.55, 0.92, sourceLuma)) * 0.36 +
      focusFalloff * 0.32,
      0.0,
      1.0
    );
    color += detail * clamp(uFocusClarity + uMicroContrast * subjectMask, 0.0, 1.6) * focusMask * amount;
    color = mix(color, color + vec3(uFocusLift), focusMask * amount);

    float floorRatio = clamp(uFloorRatio, 0.05, 0.95);
    float lowerStage = 1.0 - smoothstep(0.0, floorRatio, vUv.y);
    float upperStage = smoothstep(floorRatio, 1.0, vUv.y);

    vec2 stageUv = vUv - uStageCenter;
    stageUv.x += stageUv.y * uStageTilt;
    stageUv /= max(uStageScale, vec2(0.001));
    float stageDistance = length(stageUv);
    float stageMask = 1.0 - smoothstep(0.42, 0.78, stageDistance);
    float stageEdge = smoothstep(0.5, 0.84, stageDistance);
    float stageCore = 1.0 - smoothstep(0.12, 0.56, stageDistance);
    float stageOuter = smoothstep(0.38, 0.9, stageDistance);
    float floorMask = lowerStage * (0.42 + 0.58 * stageMask);
    float floorDepth = pow(clamp(1.0 - vUv.y / floorRatio, 0.0, 1.0), max(uFloorFalloff, 0.05));
    vec2 lightDir = normalize(vec2(-0.55, 0.82));
    float studioSweep = dot(vUv - vec2(0.5), lightDir) * 0.5 + 0.5;
    float floorSheen = smoothstep(0.24, 0.92, studioSweep) * floorMask * (1.0 - subjectMask * 0.45);
    float stageSpotlight = stageCore * (0.62 + 0.38 * smoothstep(0.16, 0.9, studioSweep));
    float stageShadow = stageOuter * (0.45 + 0.55 * floorMask) * (1.0 - subjectMask * 0.42);
    float contactDarken = floorMask * floorDepth * (1.0 - subjectMask * 0.58);
    vec3 floorTone = mix(uFloorColor, uShadowColor, contactDarken * 0.38);
    color = mix(color, color * floorTone, contactDarken * clamp(uGroundShade, 0.0, 1.0) * amount);
    color = mix(color, uHighlightColor, floorSheen * clamp(uFloorSheen, 0.0, 1.0) * amount * 0.18);
    color = mix(color, uHighlightColor, stageSpotlight * clamp(uStageSpotlight, 0.0, 1.0) * amount * 0.16);
    color = mix(color, color * uShadowColor, stageShadow * clamp(uStageShadow, 0.0, 1.0) * amount * 0.34);

    float backlight = smoothstep(0.44, 1.0, studioSweep) * smoothstep(0.28, 1.0, vUv.y);
    color = mix(color, uHighlightColor, backlight * clamp(uBacklight, 0.0, 1.0) * amount * 0.08);
    color = mix(color, uSkyColor, upperStage * clamp(uStageGlow, 0.0, 1.0) * amount * 0.14);
    color = mix(color, color * uShadowColor, stageEdge * clamp(uEdgeFade, 0.0, 1.0) * amount * (1.0 - subjectMask * 0.34));

    vec2 centered = vUv - vec2(0.5);
    centered.x *= 0.76;
    float edge = smoothstep(0.34, 0.82, length(centered));
    color = mix(color, color * uFloorColor, edge * clamp(uEdgeFade, 0.0, 1.0) * amount);
    float apertureEdge = smoothstep(0.28, 0.86, length(centered));
    color = mix(color, color * uShadowColor, apertureEdge * clamp(uApertureVignette, 0.0, 1.0) * amount * (0.34 + 0.66 * focusRamp));
    color = mix(color, color + detail * 1.4, subjectMask * clamp(uSubjectIsolation, 0.0, 1.0) * focusFalloff * amount);

    vec3 warm = vec3(1.0 + uWarmth * 0.08, 1.0 + uWarmth * 0.015, 1.0 - uWarmth * 0.055);
    color *= warm;

    gl_FragColor = vec4(clamp(color, 0.0, 1.0), source.a);
  }
`;

export class MiniatureFramePass extends FullscreenQuadPass {
  constructor(gl, options = {}) {
    const frame = normalizeFrameOptions({ ...PHOTOGRAPHIC_MINIATURE_FRAME, ...options });
    super(gl, {
      ...options,
      fragmentShader: MINIATURE_FRAME_FRAGMENT_SHADER,
      uniforms: {
        uFocusStart: frame.focusStart,
        uFocusEnd: frame.focusEnd,
        uFocusRadius: frame.focusRadius,
        uFocusBand: frame.focusBand,
        uFocusBlurCurve: frame.focusBlurCurve,
        uAmount: frame.amount,
        uFocusClarity: frame.focusClarity,
        uFocusLift: frame.focusLift,
        uStageGlow: frame.stageGlow,
        uGroundShade: frame.groundShade,
        uEdgeFade: frame.edgeFade,
        uWarmth: frame.warmth,
        uFloorRatio: frame.floorRatio,
        uFloorColor: frame.floorColor,
        uSkyColor: frame.skyColor,
        uStageCenter: frame.stageCenter,
        uStageScale: frame.stageScale,
        uStageTilt: frame.stageTilt,
        uFloorSheen: frame.floorSheen,
        uFloorFalloff: frame.floorFalloff,
        uBacklight: frame.backlight,
        uStageSpotlight: frame.stageSpotlight,
        uStageShadow: frame.stageShadow,
        uSubjectIsolation: frame.subjectIsolation,
        uMicroContrast: frame.microContrast,
        uLensAberration: frame.lensAberration,
        uApertureVignette: frame.apertureVignette,
        uFocusDesaturation: frame.focusDesaturation,
        uShadowColor: frame.shadowColor,
        uHighlightColor: frame.highlightColor,
        ...(options.uniforms || {}),
      },
    });
    this.frame = frame;
    this.focusState = null;
    this.hasExplicitFocusLine = Boolean(options.focusStart || options.focusEnd || options.start || options.end);
    this.hasExplicitFocusRadius = Boolean(options.focusRadius || options.radius);
  }

  setSize(width, height) {
    super.setSize(width, height);
    if (!this.focusState && !this.hasExplicitFocusLine) {
      this.setUniform("uFocusStart", [0, this.height * 0.52]);
      this.setUniform("uFocusEnd", [this.width, this.height * 0.52]);
    }
    if (!this.focusState && !this.hasExplicitFocusRadius) {
      this.setUniform("uFocusRadius", this.height * this.frame.focusRadius);
    }
    if (!this.focusState) {
      this.setUniform("uFocusBand", this.height * this.frame.focusBand);
      this.setUniform("uFocusBlurCurve", this.frame.focusBlurCurve);
    }
    return this;
  }

  setFrame(options = {}) {
    if ("enabled" in options) this.enabled = options.enabled !== false;
    if (options.focusStart || options.focusEnd || options.start || options.end) {
      this.hasExplicitFocusLine = true;
    }
    if (options.focusRadius || options.radius) {
      this.hasExplicitFocusRadius = true;
    }
    this.frame = normalizeFrameOptions({ ...this.frame, ...options });
    applyFrameUniforms(this, this.frame);
    return this.setSize(this.width, this.height);
  }

  setFocusState(focusState) {
    this.focusState = focusState || null;
    if (!focusState) return this.setSize(this.width, this.height);
    const uniforms = typeof focusState.toUniforms === "function"
      ? focusState.toUniforms()
      : focusState;
    return this
      .setUniform("uFocusStart", point2(uniforms.uFocusStart || focusState.start, this.frame.focusStart))
      .setUniform("uFocusEnd", point2(uniforms.uFocusEnd || focusState.end, this.frame.focusEnd))
      .setUniform("uFocusRadius", positiveNumber(uniforms.uFocusRadius ?? focusState.radius, this.frame.focusRadius))
      .setUniform("uFocusBand", nonNegativeNumber(uniforms.uFocusBand ?? focusState.focusBand, this.frame.focusBand))
      .setUniform("uFocusBlurCurve", positiveNumber(uniforms.uFocusBlurCurve ?? focusState.blurCurve, this.frame.focusBlurCurve));
  }

  render(context, inputTexture, outputTarget = null) {
    if (context?.focusState) this.setFocusState(context.focusState);
    return super.render(context, inputTexture, outputTarget);
  }
}

export function createPhotographicMiniatureFrame(overrides = {}) {
  return normalizeFrameOptions({ ...PHOTOGRAPHIC_MINIATURE_FRAME, ...overrides });
}

function applyFrameUniforms(pass, frame) {
  return pass
    .setUniform("uFocusStart", frame.focusStart)
    .setUniform("uFocusEnd", frame.focusEnd)
    .setUniform("uFocusRadius", frame.focusRadius)
    .setUniform("uFocusBand", frame.focusBand)
    .setUniform("uFocusBlurCurve", frame.focusBlurCurve)
    .setUniform("uAmount", frame.amount)
    .setUniform("uFocusClarity", frame.focusClarity)
    .setUniform("uFocusLift", frame.focusLift)
    .setUniform("uStageGlow", frame.stageGlow)
    .setUniform("uGroundShade", frame.groundShade)
    .setUniform("uEdgeFade", frame.edgeFade)
    .setUniform("uWarmth", frame.warmth)
    .setUniform("uFloorRatio", frame.floorRatio)
    .setUniform("uFloorColor", frame.floorColor)
    .setUniform("uSkyColor", frame.skyColor)
    .setUniform("uStageCenter", frame.stageCenter)
    .setUniform("uStageScale", frame.stageScale)
    .setUniform("uStageTilt", frame.stageTilt)
    .setUniform("uFloorSheen", frame.floorSheen)
    .setUniform("uFloorFalloff", frame.floorFalloff)
    .setUniform("uBacklight", frame.backlight)
    .setUniform("uStageSpotlight", frame.stageSpotlight)
    .setUniform("uStageShadow", frame.stageShadow)
    .setUniform("uSubjectIsolation", frame.subjectIsolation)
    .setUniform("uMicroContrast", frame.microContrast)
    .setUniform("uLensAberration", frame.lensAberration)
    .setUniform("uApertureVignette", frame.apertureVignette)
    .setUniform("uFocusDesaturation", frame.focusDesaturation)
    .setUniform("uShadowColor", frame.shadowColor)
    .setUniform("uHighlightColor", frame.highlightColor);
}

function normalizeFrameOptions(options = {}) {
  return {
    enabled: options.enabled !== false,
    amount: clampNumber(options.amount, 0, 1, 0.42),
    focusClarity: clampNumber(options.focusClarity, 0, 1, 0.18),
    focusLift: clampNumber(options.focusLift, 0, 0.35, 0.035),
    stageGlow: clampNumber(options.stageGlow, 0, 1, 0.12),
    groundShade: clampNumber(options.groundShade, 0, 1, 0.08),
    edgeFade: clampNumber(options.edgeFade, 0, 1, 0.08),
    warmth: clampNumber(options.warmth, -1, 1, 0.05),
    floorRatio: clampNumber(options.floorRatio, 0.05, 0.95, 0.52),
    floorColor: color3(options.floorColor, [0.86, 0.88, 0.84]),
    skyColor: color3(options.skyColor, [0.98, 0.97, 0.92]),
    stageCenter: point2(options.stageCenter, [0.5, 0.52]),
    stageScale: point2(options.stageScale, [0.9, 0.74]),
    stageTilt: clampNumber(options.stageTilt, -1, 1, -0.1),
    floorSheen: clampNumber(options.floorSheen, 0, 1, 0.12),
    floorFalloff: clampNumber(options.floorFalloff, 0.05, 4, 0.78),
    backlight: clampNumber(options.backlight, 0, 1, 0.16),
    stageSpotlight: clampNumber(options.stageSpotlight, 0, 1, 0.18),
    stageShadow: clampNumber(options.stageShadow, 0, 1, 0.16),
    subjectIsolation: clampNumber(options.subjectIsolation, 0, 1, 0.24),
    microContrast: clampNumber(options.microContrast, 0, 1, 0.18),
    lensAberration: clampNumber(options.lensAberration, 0, 1, 0.006),
    apertureVignette: clampNumber(options.apertureVignette, 0, 1, 0.08),
    focusDesaturation: clampNumber(options.focusDesaturation, 0, 1, 0.04),
    focusBand: clampNumber(options.focusBand ?? options.focusBandRatio, 0, 1, 0.045),
    focusBlurCurve: clampNumber(options.focusBlurCurve ?? options.blurCurve, 0.05, 8, 1.35),
    shadowColor: color3(options.shadowColor, [0.67, 0.73, 0.77]),
    highlightColor: color3(options.highlightColor, [1.0, 0.965, 0.88]),
    focusStart: point2(options.focusStart || options.start, [0, 0.5]),
    focusEnd: point2(options.focusEnd || options.end, [1, 0.5]),
    focusRadius: positiveNumber(options.focusRadius ?? options.radius, 0.33),
  };
}

function point2(value, fallback) {
  if (!value) return fallback.slice();
  if (Array.isArray(value) || ArrayBuffer.isView(value)) {
    return [finiteNumber(value[0], fallback[0]), finiteNumber(value[1], fallback[1])];
  }
  return [finiteNumber(value.x, fallback[0]), finiteNumber(value.y, fallback[1])];
}

function color3(value, fallback) {
  if (!value) return fallback.slice();
  if (Array.isArray(value) || ArrayBuffer.isView(value)) {
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

function positiveNumber(value, fallback) {
  const number = Number(value);
  return Number.isFinite(number) && number > 0 ? number : fallback;
}

function nonNegativeNumber(value, fallback) {
  const number = Number(value);
  return Number.isFinite(number) && number >= 0 ? number : fallback;
}

function finiteNumber(value, fallback) {
  const number = Number(value);
  return Number.isFinite(number) ? number : fallback;
}

function clampNumber(value, min, max, fallback) {
  const number = Number(value);
  if (!Number.isFinite(number)) return fallback;
  return Math.min(max, Math.max(min, number));
}
