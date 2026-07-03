import { resolveMiniatureFocusLine } from "./focus.js";

const TAU = Math.PI * 2;

export const MINIATURE_STYLE_MOTION_PRESETS = Object.freeze({
  still: Object.freeze({ mode: "none" }),
  "focus-breath": Object.freeze({
    mode: "focus-breath",
    loop: true,
    durationMs: 7200,
    focus: Object.freeze({
      enabled: true,
      yAmplitude: 0.018,
      radiusAmplitude: 0.035,
    }),
    camera: Object.freeze({ enabled: false }),
    postprocess: Object.freeze({
      enabled: true,
      frameAmountAmplitude: 0.018,
      focusLiftAmplitude: 0.006,
    }),
  }),
  "studio-drift": Object.freeze({
    mode: "studio-drift",
    loop: true,
    durationMs: 18000,
    focus: Object.freeze({
      enabled: true,
      yAmplitude: 0.014,
      radiusAmplitude: 0.028,
    }),
    camera: Object.freeze({
      enabled: true,
      yawAmplitude: 0.026,
      pitchAmplitude: 0.008,
      respectInteraction: true,
    }),
    postprocess: Object.freeze({
      enabled: true,
      frameAmountAmplitude: 0.014,
      focusLiftAmplitude: 0.004,
    }),
  }),
  turntable: Object.freeze({
    mode: "turntable",
    loop: true,
    durationMs: 22000,
    focus: Object.freeze({ enabled: false }),
    camera: Object.freeze({
      enabled: true,
      yawSpeed: 0.00028,
      pitchAmplitude: 0,
      respectInteraction: true,
    }),
    postprocess: Object.freeze({ enabled: false }),
  }),
  "isometric-orbit": Object.freeze({
    mode: "isometric-orbit",
    loop: true,
    durationMs: 26000,
    focus: Object.freeze({
      enabled: true,
      yAmplitude: 0.01,
      radiusAmplitude: 0.018,
    }),
    camera: Object.freeze({
      enabled: true,
      yawSpeed: 0.00018,
      yawAmplitude: 0.018,
      pitchAmplitude: 0.014,
      radiusAmplitude: 0.024,
      respectInteraction: true,
    }),
    postprocess: Object.freeze({
      enabled: true,
      frameAmountAmplitude: 0.012,
      focusLiftAmplitude: 0.003,
      subjectIsolationAmplitude: 0.018,
    }),
  }),
  "ground-scan": Object.freeze({
    mode: "ground-scan",
    loop: true,
    durationMs: 10500,
    focus: Object.freeze({
      enabled: true,
      xAmplitude: 0.014,
      yAmplitude: 0.055,
      radiusAmplitude: 0.02,
    }),
    camera: Object.freeze({
      enabled: true,
      yawAmplitude: 0.012,
      pitchAmplitude: 0.004,
      respectInteraction: true,
    }),
    postprocess: Object.freeze({
      enabled: true,
      frameAmountAmplitude: 0.014,
      focusLiftAmplitude: 0.007,
      floorSheenAmplitude: 0.04,
      stageGlowAmplitude: 0.025,
      stageSpotlightAmplitude: 0.035,
      stageShadowAmplitude: 0.025,
    }),
  }),
  "studio-pulse": Object.freeze({
    mode: "studio-pulse",
    loop: true,
    durationMs: 12800,
    focus: Object.freeze({
      enabled: true,
      yAmplitude: 0.012,
      radiusAmplitude: 0.026,
    }),
    camera: Object.freeze({
      enabled: true,
      yawAmplitude: 0.016,
      pitchAmplitude: 0.006,
      radiusAmplitude: 0.018,
      respectInteraction: true,
    }),
    postprocess: Object.freeze({
      enabled: true,
      frameAmountAmplitude: 0.018,
      focusLiftAmplitude: 0.005,
      stageGlowAmplitude: 0.035,
      backlightAmplitude: 0.035,
      stageSpotlightAmplitude: 0.026,
      stageShadowAmplitude: 0.018,
      microContrastAmplitude: 0.025,
    }),
  }),
});

export class MiniatureStyleMotionController {
  constructor(motion = "none") {
    this.motion = createMiniatureStyleMotionPreset(motion);
    this.runtime = null;
    this.stage = null;
    this.unsubscribe = null;
    this.base = null;
    this.lastUpdate = null;
  }

  attach(runtime, stage = {}) {
    this.detach();
    this.runtime = runtime || null;
    this.stage = stage || {};
    if (!this.runtime || !this.enabled) return this;
    this.base = captureStyleMotionBase(this.runtime, this.stage);
    this.lastUpdate = null;
    if (this.runtime.renderer && typeof this.runtime.renderer.on === "function") {
      this.unsubscribe = this.runtime.renderer.on("beforeRender", (context) => {
        this.update(context?.time || {});
      });
    }
    return this;
  }

  detach() {
    if (this.unsubscribe) {
      this.unsubscribe();
      this.unsubscribe = null;
    }
    this.runtime = null;
    this.stage = null;
    this.base = null;
    this.lastUpdate = null;
    return this;
  }

  setMotion(motion) {
    this.motion = createMiniatureStyleMotionPreset(motion);
    if (this.runtime) {
      this.attach(this.runtime, this.stage);
    }
    return this;
  }

  resetBase() {
    if (this.runtime) {
      this.base = captureStyleMotionBase(this.runtime, this.stage || {});
    }
    return this;
  }

  get enabled() {
    return this.motion?.mode && this.motion.mode !== "none" && this.motion.enabled !== false;
  }

  update(time = {}) {
    if (!this.enabled || !this.runtime || !this.base) return this;
    const seconds = finiteNumber(time.seconds, time.elapsed * 0.001, time.elapsedMs * 0.001, 0);
    const phase = styleMotionPhase(this.motion, seconds);
    const applied = {
      focus: applyFocusMotion(this.runtime, this.base.focus, this.motion.focus, phase),
      camera: applyCameraMotion(this.runtime, this.base.camera, this.motion.camera, phase, seconds),
      postprocess: applyFrameMotion(this.runtime, this.base.frame, this.motion.postprocess, phase),
    };
    this.lastUpdate = {
      seconds,
      phase,
      applied,
      activeDomains: activeMotionDomains(this.motion),
    };
    return this;
  }

  toJSON() {
    return {
      schema: "metric.visual.miniature_style_motion.v1",
      enabled: this.enabled,
      motion: clonePlainObject(this.motion),
      hasRuntime: Boolean(this.runtime),
      hasBase: Boolean(this.base),
      hasBeforeRenderHook: Boolean(this.unsubscribe),
      activeDomains: activeMotionDomains(this.motion),
      lastUpdate: clonePlainObject(this.lastUpdate),
    };
  }
}

export function createMiniatureStyleMotionController(motion = "none") {
  return new MiniatureStyleMotionController(motion);
}

export function createMiniatureStyleMotionPreset(kind = "none", options = {}) {
  if (kind === false || kind == null) return { mode: "none" };
  if (typeof kind === "object") {
    const mode = kind.mode || kind.kind || "focus-breath";
    return createMiniatureStyleMotionPreset(mode, { ...kind, ...options });
  }
  const mode = String(kind || "none");
  if (mode === "none" || mode === "still") return { mode: "none", ...(options || {}) };
  const preset = MINIATURE_STYLE_MOTION_PRESETS[mode] || { mode };
  return deepMerge(preset, options || {});
}

export function listMiniatureStyleMotionPresets() {
  return Object.keys(MINIATURE_STYLE_MOTION_PRESETS)
    .filter((name) => name !== "still")
    .map((name) => createMiniatureStyleMotionPreset(name));
}

export function selectMiniatureStyleMotionForLook(look) {
  const name = typeof look === "string" ? look : look?.look || look?.name || "";
  if (name === "dark-tube-field") return "isometric-orbit";
  if (name === "chromatic-surface" || name === "dark-gloss-studio") return "studio-drift";
  if (name === "metric-city" || name === "colored-shadow-stage" || name === "surface-ripple") return "ground-scan";
  if (name === "studio-board") return "studio-pulse";
  return "focus-breath";
}

export function createMiniatureStyleMotionAtlas(options = {}) {
  const motions = Array.isArray(options.motions) && options.motions.length
    ? options.motions
    : listMiniatureStyleMotionPresets();
  const entries = motions.map((motion) => createMiniatureStyleMotionAtlasEntry(motion, options));
  return deepFreeze({
    schema: "metric.visual.miniature_style_motion_atlas.v1",
    id: options.id || "metric-miniature-style-motion-atlas",
    entries,
    summary: {
      entryCount: entries.length,
      modes: entries.map((entry) => entry.mode),
      animatedCount: entries.filter((entry) => entry.contract.animated).length,
      focusCount: entries.filter((entry) => entry.contract.activeDomains.focus).length,
      cameraCount: entries.filter((entry) => entry.contract.activeDomains.camera).length,
      postprocessCount: entries.filter((entry) => entry.contract.activeDomains.postprocess).length,
    },
    metadata: {
      reusableMiniatureStyle: true,
      motionIsStyleContract: true,
      ...(options.metadata || {}),
    },
  });
}

export function createMiniatureStyleMotionAtlasEntry(motion, options = {}) {
  const preset = createMiniatureStyleMotionPreset(motion, options.overrides || {});
  return deepFreeze({
    schema: "metric.visual.miniature_style_motion_atlas_entry.v1",
    mode: preset.mode || "none",
    contract: summarizeMiniatureStyleMotion(preset),
    samples: sampleMiniatureStyleMotion(preset, options.sampleSeconds),
  });
}

export function summarizeMiniatureStyleMotion(motion = {}) {
  const preset = createMiniatureStyleMotionPreset(motion);
  const activeDomains = activeMotionDomains(preset);
  return {
    mode: preset.mode || "none",
    animated: Boolean(preset.mode && preset.mode !== "none"),
    loop: preset.loop !== false,
    durationMs: positiveNumber(preset.durationMs, 1),
    activeDomains,
    focus: activeDomains.focus ? summarizeFocusMotion(preset.focus) : null,
    camera: activeDomains.camera ? summarizeCameraMotion(preset.camera) : null,
    postprocess: activeDomains.postprocess ? summarizePostprocessMotion(preset.postprocess) : null,
  };
}

export function sampleMiniatureStyleMotion(motion = {}, sampleSeconds) {
  const preset = createMiniatureStyleMotionPreset(motion);
  const samples = Array.isArray(sampleSeconds) && sampleSeconds.length
    ? sampleSeconds
    : defaultSampleSeconds(preset);
  return samples.map((seconds) => {
    const phase = styleMotionPhase(preset, seconds);
    return {
      seconds,
      phase,
      focus: sampleFocusMotion(preset.focus, phase),
      camera: sampleCameraMotion(preset.camera, phase, seconds),
      postprocess: samplePostprocessMotion(preset.postprocess, phase),
    };
  });
}

function captureStyleMotionBase(runtime, stage = {}) {
  const camera = runtime.cameraState?.toJSON ? runtime.cameraState.toJSON() : {};
  const focus = clonePlainObject(stage.focus || runtime.options?.focusLine || {});
  const frame = clonePlainObject(
    runtime.postprocess?.postFxStack?.miniatureFrame?.frame
      || stage.postprocess?.postFx?.miniatureFrame
      || stage.postprocess?.miniatureFrame
      || {},
  );
  return { camera, focus, frame };
}

function applyFocusMotion(runtime, baseFocus = {}, focusMotion = {}, phase = 0) {
  if (!focusMotion || focusMotion.enabled === false || !runtime.focusLine) return false;
  const wave = Math.sin(phase);
  const yRatio = clamp01(finiteNumber(baseFocus.yRatio, 0.5) + wave * finiteNumber(focusMotion.yAmplitude, 0));
  const xRatio = clamp01(finiteNumber(baseFocus.xRatio, 0.5) + wave * finiteNumber(focusMotion.xAmplitude, 0));
  const radiusRatio = positiveNumber(
    finiteNumber(baseFocus.radiusRatio, 1 / 3) * (1 + Math.sin(phase + Math.PI * 0.5) * finiteNumber(focusMotion.radiusAmplitude, 0)),
    baseFocus.radiusRatio,
    1 / 3,
  );
  const focus = {
    ...baseFocus,
    xRatio,
    yRatio,
    radiusRatio,
  };
  const size = runtime.renderer?.size || {};
  const width = size.drawingBufferWidth || size.width || runtime.focusLine.width || 1;
  const height = size.drawingBufferHeight || size.height || runtime.focusLine.height || 1;
  const resolved = resolveMiniatureFocusLine(focus, width, height);
  runtime.focusLine.setLine(resolved.start, resolved.end, resolved.radius, {
    axis: resolved.axis,
    focusBand: resolved.focusBand,
    blurCurve: resolved.blurCurve,
  });
  if (typeof runtime.syncFocusLine === "function") runtime.syncFocusLine();
  return true;
}

function applyCameraMotion(runtime, baseCamera = {}, cameraMotion = {}, phase = 0, seconds = 0) {
  if (!cameraMotion || cameraMotion.enabled === false || !runtime.cameraState) return false;
  if (cameraMotion.respectInteraction !== false && runtime.controls?.dragging) return false;
  const yawSpeed = finiteNumber(cameraMotion.yawSpeed, 0);
  const yaw = finiteNumber(baseCamera.yaw, runtime.cameraState.yaw, 0)
    + Math.sin(phase) * finiteNumber(cameraMotion.yawAmplitude, 0)
    + seconds * yawSpeed;
  const pitch = finiteNumber(baseCamera.pitch, runtime.cameraState.pitch, 0)
    + Math.sin(phase + Math.PI * 0.5) * finiteNumber(cameraMotion.pitchAmplitude, 0);
  const radius = positiveNumber(baseCamera.radius, runtime.cameraState.radius, 1)
    * (1 + Math.sin(phase + Math.PI) * finiteNumber(cameraMotion.radiusAmplitude, 0));
  runtime.cameraState.setOrbit({ yaw, pitch, radius });
  runtime.cameraState.applyTo(runtime.camera);
  runtime.renderer?.setCamera?.(runtime.camera);
  return true;
}

function applyFrameMotion(runtime, baseFrame = {}, frameMotion = {}, phase = 0) {
  if (!frameMotion || frameMotion.enabled === false) return false;
  const framePass = runtime.postprocess?.postFxStack?.miniatureFrame;
  if (!framePass || typeof framePass.setFrame !== "function") return false;
  const wave = Math.sin(phase + Math.PI * 0.35);
  framePass.setFrame(composeFrameMotion(baseFrame, frameMotion, wave));
  return true;
}

function activeMotionDomains(motion = {}) {
  return {
    focus: Boolean(motion.focus && motion.focus.enabled !== false),
    camera: Boolean(motion.camera && motion.camera.enabled !== false),
    postprocess: Boolean(motion.postprocess && motion.postprocess.enabled !== false),
  };
}

function styleMotionPhase(motion, seconds) {
  const duration = Math.max(1, finiteNumber(motion.durationMs, 7200)) * 0.001;
  if (motion.loop === false) return clamp01(seconds / duration) * TAU;
  return ((seconds % duration) / duration) * TAU;
}

function composeFrameMotion(baseFrame = {}, frameMotion = {}, wave = 0) {
  return {
    amount: animatedFrameValue(baseFrame, frameMotion, "amount", "frameAmountAmplitude", wave, 0.42, 0, 1),
    focusLift: animatedFrameValue(baseFrame, frameMotion, "focusLift", "focusLiftAmplitude", wave, 0.035, 0, 0.35),
    floorSheen: animatedFrameValue(baseFrame, frameMotion, "floorSheen", "floorSheenAmplitude", wave, 0.12, 0, 1),
    stageGlow: animatedFrameValue(baseFrame, frameMotion, "stageGlow", "stageGlowAmplitude", wave, 0.12, 0, 1),
    backlight: animatedFrameValue(baseFrame, frameMotion, "backlight", "backlightAmplitude", wave, 0.16, 0, 1),
    stageSpotlight: animatedFrameValue(baseFrame, frameMotion, "stageSpotlight", "stageSpotlightAmplitude", wave, 0.18, 0, 1),
    stageShadow: animatedFrameValue(baseFrame, frameMotion, "stageShadow", "stageShadowAmplitude", wave, 0.16, 0, 1),
    subjectIsolation: animatedFrameValue(baseFrame, frameMotion, "subjectIsolation", "subjectIsolationAmplitude", wave, 0.24, 0, 1),
    microContrast: animatedFrameValue(baseFrame, frameMotion, "microContrast", "microContrastAmplitude", wave, 0.18, 0, 1),
    edgeFade: animatedFrameValue(baseFrame, frameMotion, "edgeFade", "edgeFadeAmplitude", wave, 0.08, 0, 1),
    lensAberration: animatedFrameValue(baseFrame, frameMotion, "lensAberration", "lensAberrationAmplitude", wave, 0.006, 0, 1),
    apertureVignette: animatedFrameValue(baseFrame, frameMotion, "apertureVignette", "apertureVignetteAmplitude", wave, 0.08, 0, 1),
    focusDesaturation: animatedFrameValue(baseFrame, frameMotion, "focusDesaturation", "focusDesaturationAmplitude", wave, 0.04, 0, 1),
  };
}

function animatedFrameValue(baseFrame, frameMotion, key, amplitudeKey, wave, fallback, min, max) {
  return clamp(finiteNumber(baseFrame[key], fallback) + wave * finiteNumber(frameMotion[amplitudeKey], 0), min, max);
}

function defaultSampleSeconds(motion = {}) {
  const duration = Math.max(1, finiteNumber(motion.durationMs, 7200)) * 0.001;
  return [0, duration * 0.25, duration * 0.5, duration * 0.75].map((value) => Number(value.toFixed(3)));
}

function summarizeFocusMotion(focus = {}) {
  return {
    xAmplitude: finiteNumber(focus.xAmplitude, 0),
    yAmplitude: finiteNumber(focus.yAmplitude, 0),
    radiusAmplitude: finiteNumber(focus.radiusAmplitude, 0),
  };
}

function summarizeCameraMotion(camera = {}) {
  return {
    yawSpeed: finiteNumber(camera.yawSpeed, 0),
    yawAmplitude: finiteNumber(camera.yawAmplitude, 0),
    pitchAmplitude: finiteNumber(camera.pitchAmplitude, 0),
    radiusAmplitude: finiteNumber(camera.radiusAmplitude, 0),
    respectInteraction: camera.respectInteraction !== false,
  };
}

function summarizePostprocessMotion(postprocess = {}) {
  return {
    frameAmountAmplitude: finiteNumber(postprocess.frameAmountAmplitude, 0),
    focusLiftAmplitude: finiteNumber(postprocess.focusLiftAmplitude, 0),
    floorSheenAmplitude: finiteNumber(postprocess.floorSheenAmplitude, 0),
    stageGlowAmplitude: finiteNumber(postprocess.stageGlowAmplitude, 0),
    backlightAmplitude: finiteNumber(postprocess.backlightAmplitude, 0),
    stageSpotlightAmplitude: finiteNumber(postprocess.stageSpotlightAmplitude, 0),
    stageShadowAmplitude: finiteNumber(postprocess.stageShadowAmplitude, 0),
    subjectIsolationAmplitude: finiteNumber(postprocess.subjectIsolationAmplitude, 0),
    microContrastAmplitude: finiteNumber(postprocess.microContrastAmplitude, 0),
    edgeFadeAmplitude: finiteNumber(postprocess.edgeFadeAmplitude, 0),
    lensAberrationAmplitude: finiteNumber(postprocess.lensAberrationAmplitude, 0),
    apertureVignetteAmplitude: finiteNumber(postprocess.apertureVignetteAmplitude, 0),
    focusDesaturationAmplitude: finiteNumber(postprocess.focusDesaturationAmplitude, 0),
  };
}

function sampleFocusMotion(focus = {}, phase = 0) {
  if (!focus || focus.enabled === false) return null;
  const wave = Math.sin(phase);
  return {
    xRatioOffset: wave * finiteNumber(focus.xAmplitude, 0),
    yRatioOffset: wave * finiteNumber(focus.yAmplitude, 0),
    radiusScale: 1 + Math.sin(phase + Math.PI * 0.5) * finiteNumber(focus.radiusAmplitude, 0),
  };
}

function sampleCameraMotion(camera = {}, phase = 0, seconds = 0) {
  if (!camera || camera.enabled === false) return null;
  return {
    yawOffset: Math.sin(phase) * finiteNumber(camera.yawAmplitude, 0) + seconds * finiteNumber(camera.yawSpeed, 0),
    pitchOffset: Math.sin(phase + Math.PI * 0.5) * finiteNumber(camera.pitchAmplitude, 0),
    radiusScale: 1 + Math.sin(phase + Math.PI) * finiteNumber(camera.radiusAmplitude, 0),
  };
}

function samplePostprocessMotion(postprocess = {}, phase = 0) {
  if (!postprocess || postprocess.enabled === false) return null;
  const wave = Math.sin(phase + Math.PI * 0.35);
  return {
    frameAmountOffset: wave * finiteNumber(postprocess.frameAmountAmplitude, 0),
    focusLiftOffset: wave * finiteNumber(postprocess.focusLiftAmplitude, 0),
    floorSheenOffset: wave * finiteNumber(postprocess.floorSheenAmplitude, 0),
    stageGlowOffset: wave * finiteNumber(postprocess.stageGlowAmplitude, 0),
    backlightOffset: wave * finiteNumber(postprocess.backlightAmplitude, 0),
    stageSpotlightOffset: wave * finiteNumber(postprocess.stageSpotlightAmplitude, 0),
    stageShadowOffset: wave * finiteNumber(postprocess.stageShadowAmplitude, 0),
    subjectIsolationOffset: wave * finiteNumber(postprocess.subjectIsolationAmplitude, 0),
    microContrastOffset: wave * finiteNumber(postprocess.microContrastAmplitude, 0),
    edgeFadeOffset: wave * finiteNumber(postprocess.edgeFadeAmplitude, 0),
    lensAberrationOffset: wave * finiteNumber(postprocess.lensAberrationAmplitude, 0),
    apertureVignetteOffset: wave * finiteNumber(postprocess.apertureVignetteAmplitude, 0),
    focusDesaturationOffset: wave * finiteNumber(postprocess.focusDesaturationAmplitude, 0),
  };
}

function deepMerge(base, override) {
  if (!isPlainObject(base) || !isPlainObject(override)) return clonePlainObject(override ?? base);
  const next = clonePlainObject(base);
  for (const [key, value] of Object.entries(override)) {
    next[key] = isPlainObject(value) && isPlainObject(next[key])
      ? deepMerge(next[key], value)
      : clonePlainObject(value);
  }
  return next;
}

function clonePlainObject(value) {
  if (value == null || typeof value !== "object") return value;
  if (Array.isArray(value)) return value.map(clonePlainObject);
  if (ArrayBuffer.isView(value)) return value;
  const next = {};
  for (const [key, entry] of Object.entries(value)) next[key] = clonePlainObject(entry);
  return next;
}

function isPlainObject(value) {
  return Boolean(value && typeof value === "object" && !Array.isArray(value) && !ArrayBuffer.isView(value));
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

function clamp(value, min, max) {
  return Math.max(min, Math.min(max, value));
}

function clamp01(value) {
  return clamp(value, 0, 1);
}

function deepFreeze(value) {
  if (ArrayBuffer.isView(value)) return value;
  if (!value || typeof value !== "object" || Object.isFrozen(value)) return value;
  Object.freeze(value);
  for (const entry of Object.values(value)) deepFreeze(entry);
  return value;
}
