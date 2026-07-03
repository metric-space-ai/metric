import { createMiniatureNoiseOptions } from "./presets.js";

const FAMILY_DEFAULT_ANIMATION = Object.freeze({
  sample: "focus",
  mesh: "still",
  surface: "surface",
  field: "still",
  line: "still",
  ground: "still",
});

/**
 * Creates a reusable miniature animation descriptor.
 *
 * These presets describe how the renderer may animate already-exported visual
 * evidence. They never derive metric values or invent hidden states.
 *
 * @param {string|object|boolean} [kind]
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniatureAnimationPreset(kind = "still", options = {}) {
  if (kind === false) return { mode: "none" };
  if (kind && typeof kind === "object") {
    return createMiniatureAnimationPreset(kind.kind || kind.mode || "still", { ...kind, ...options });
  }

  const name = String(kind || "still");
  if (name === "none" || name === "still") {
    return { mode: "none", ...(options || {}) };
  }
  if (name === "focus" || name === "subtle-focus") {
    return {
      mode: "subtle-focus",
      loop: options.loop !== false,
      direction: options.direction || "alternate",
      durationMs: positiveNumber(options.durationMs, 5200),
      easing: options.easing || "smoothstep",
      ...(options || {}),
    };
  }
  if (name === "coordinate-morph" || name === "morph") {
    return {
      mode: "coordinate-morph",
      channel: options.channel || "targetPosition",
      requiresChannels: options.requiresChannels || ["position", "targetPosition"],
      progress: finiteNumber(options.progress, 0),
      loop: Boolean(options.loop),
      direction: options.direction || "alternate",
      durationMs: positiveNumber(options.durationMs, 1200),
      easing: options.easing || "smoothstep",
      ...(options || {}),
    };
  }
  if (name === "attention") {
    return {
      mode: "attention",
      loop: options.loop !== false,
      direction: options.direction || "alternate",
      durationMs: positiveNumber(options.durationMs, 1800),
      easing: options.easing || "smoothstep",
      ...(options || {}),
    };
  }
  if (name === "uncertainty") {
    return {
      mode: "uncertainty",
      loop: options.loop !== false,
      direction: options.direction || "alternate",
      durationMs: positiveNumber(options.durationMs, 3600),
      amplitude: finiteNumber(options.amplitude, 0.035),
      easing: options.easing || "smoothstep",
      ...(options || {}),
    };
  }
  if (name === "surface" || name === "style-noise") {
    return {
      mode: "style-noise",
      loop: options.loop !== false,
      durationMs: positiveNumber(options.durationMs, 8000),
      noise: options.noise || createMiniatureNoiseOptions(options.noiseOptions || {}),
      ...(options || {}),
    };
  }
  if (name === "timeline") {
    return {
      mode: "timeline",
      timelineId: options.timelineId ?? options.timeline_id ?? null,
      loop: Boolean(options.loop),
      playbackRate: finiteNumber(options.playbackRate, 1),
      easing: options.easing || "linear",
      ...(options || {}),
    };
  }
  if (name === "camera-orbit" || name === "orbit") {
    return {
      mode: "camera-orbit",
      loop: options.loop !== false,
      yawSpeed: finiteNumber(options.yawSpeed, 0.035),
      pitchAmplitude: finiteNumber(options.pitchAmplitude, 0),
      durationMs: positiveNumber(options.durationMs, 12000),
      easing: options.easing || "linear",
      ...(options || {}),
    };
  }
  return {
    mode: name,
    ...(options || {}),
  };
}

/**
 * Resolves the animation for a styled layer family.
 *
 * @param {string} family
 * @param {object} [existing]
 * @param {object} [options]
 * @returns {object}
 */
export function resolveMiniatureAnimationForFamily(family, existing = {}, options = {}) {
  const animation = existing && typeof existing === "object" ? existing : {};
  if (animation.mode && animation.mode !== "none") {
    return {
      ...createMiniatureAnimationPreset(animation.mode, options.override || {}),
      ...animation,
    };
  }
  const motion = options.motion;
  const neutralAnimation = copyWithout(animation, ["mode"]);
  if (motion === false) return createMiniatureAnimationPreset("still", neutralAnimation);
  const preset = motion || FAMILY_DEFAULT_ANIMATION[family] || "still";
  return {
    ...createMiniatureAnimationPreset(preset, options.animationOptions || {}),
    ...neutralAnimation,
  };
}

function copyWithout(input, excluded) {
  const out = {};
  for (const [key, value] of Object.entries(input || {})) {
    if (!excluded.includes(key)) out[key] = value;
  }
  return out;
}

function finiteNumber(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number)) return number;
  }
  return 0;
}

function positiveNumber(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number) && number > 0) return number;
  }
  return 1;
}
