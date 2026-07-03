const DEFAULT_STANDALONE_THEME = Object.freeze({
  variant: "dark",
  accent: "oklch(72% 0.14 215)",
  panel: "oklch(12% 0.014 235 / 0.58)",
  panelLine: "oklch(64% 0.04 230 / 0.16)",
  text: "oklch(93% 0.016 230)",
  muted: "oklch(74% 0.025 230)",
  background: "oklch(15% 0.014 235)",
});

export const MINIATURE_STANDALONE_CLASSNAMES = Object.freeze({
  root: "metric-miniature-standalone",
  stage: "metric-miniature-stage",
  canvas: "metric-miniature-canvas",
  topbar: "metric-miniature-topbar",
  title: "metric-miniature-title",
  controls: "metric-miniature-controls",
  control: "metric-miniature-control",
  legend: "metric-miniature-legend",
  inspector: "metric-miniature-inspector",
  focusMask: "metric-miniature-focus-mask",
  frame: "metric-miniature-frame",
  grain: "metric-miniature-grain",
});

/**
 * Creates the shared miniature shell contract for standalone visual documents.
 *
 * This is for generated or hand-authored WebGL pages that do not instantiate
 * MetricVisualRuntime but still need the same photographic miniature vocabulary:
 * low oblique camera, turntable motion, focus mask, subdued inspection UI,
 * floor relation and final frame treatment.
 *
 * It does not compute records, distances, embeddings or metric evidence.
 *
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniatureStandaloneSurface(options = {}) {
  const theme = normalizeTheme(options.theme);
  const focus = createMiniatureStandaloneFocus(options.focus);
  const camera = createMiniatureStandaloneCamera(options.camera);
  const motion = createMiniatureStandaloneMotion(options.motion);
  const projection = createMiniatureStandaloneProjection(options.projection);
  const ui = createMiniatureStandaloneUi(options.ui);
  const postprocess = createMiniatureStandalonePostprocess(options.postprocess, focus);
  return deepFreeze({
    schema: "metric.visual.miniature_standalone_surface.v1",
    id: options.id || "metric-miniature-standalone-surface",
    classNames: MINIATURE_STANDALONE_CLASSNAMES,
    theme,
    camera,
    focus,
    motion,
    projection,
    ui,
    postprocess,
    cssText: createMiniatureStandaloneCss({ theme, focus, ui, postprocess }),
    diagnostics: inspectMiniatureStandaloneSurface({
      theme,
      camera,
      focus,
      motion,
      projection,
      ui,
      postprocess,
    }),
  });
}

export function createMiniatureStandaloneCamera(options = {}) {
  return {
    mode: "miniature-standalone-camera",
    projection: "perspective",
    fov: finiteNumber(options.fov, 37),
    yaw: finiteNumber(options.yaw, -0.72),
    pitch: clamp(finiteNumber(options.pitch, 0.72), 0.16, 1.38),
    radius: positiveNumber(options.radius, 9.4),
    target: point3(options.target, [0, 0.1, 0]),
    floorAxis: options.floorAxis || "xz",
    up: point3(options.up, [0, 1, 0]),
  };
}

export function createMiniatureStandaloneFocus(options = {}) {
  return {
    mode: "miniature-standalone-focus",
    yRatio: clamp(finiteNumber(options.yRatio, 0.46), 0, 1),
    focusBandRatio: clamp(finiteNumber(options.focusBandRatio, 0.26), 0.02, 0.9),
    blurPx: positiveNumber(options.blurPx, 5.5),
    maskStartRatio: clamp(finiteNumber(options.maskStartRatio, 0.17), 0, 1),
    maskNearRatio: clamp(finiteNumber(options.maskNearRatio, 0.35), 0, 1),
    maskFarRatio: clamp(finiteNumber(options.maskFarRatio, 0.62), 0, 1),
    maskEndRatio: clamp(finiteNumber(options.maskEndRatio, 0.82), 0, 1),
  };
}

export function createMiniatureStandaloneMotion(options = {}) {
  const enabled = options.enabled !== false;
  return {
    mode: options.mode || "turntable",
    enabled,
    yawSpeed: enabled ? finiteNumber(options.yawSpeed, 0.00022) : 0,
    pauseOnDrag: options.pauseOnDrag !== false,
    morphAutoPlay: options.morphAutoPlay !== false,
    morphDurationMs: positiveNumber(options.morphDurationMs, 27500),
  };
}

export function createMiniatureStandaloneProjection(options = {}) {
  return {
    mode: "semantic-floor-projection",
    enabled: options.enabled !== false,
    opacity: clamp(finiteNumber(options.opacity, 0.34), 0, 1),
    pointScale: positiveNumber(options.pointScale, 1.18),
    densityAlpha: clamp(finiteNumber(options.densityAlpha, 0.052), 0, 1),
    baseAlpha: clamp(finiteNumber(options.baseAlpha, 0.026), 0, 1),
    floorLift: finiteNumber(options.floorLift, -0.04),
  };
}

export function createMiniatureStandaloneUi(options = {}) {
  return {
    mode: "quiet-inspection-overlay",
    visible: options.visible !== false,
    titleVisible: options.titleVisible !== false,
    controlsVisible: options.controlsVisible !== false,
    legendVisible: options.legendVisible !== false,
    inspectorVisible: options.inspectorVisible !== false,
    compact: options.compact !== false,
    panelBlurPx: positiveNumber(options.panelBlurPx, 16),
    panelRadiusPx: positiveNumber(options.panelRadiusPx, 9),
  };
}

export function createMiniatureStandalonePostprocess(options = {}, focus = createMiniatureStandaloneFocus()) {
  return {
    mode: "standalone-miniature-postprocess",
    enabled: options.enabled !== false,
    focusMask: options.focusMask !== false,
    frame: options.frame !== false,
    grain: options.grain !== false,
    backdropBlurPx: positiveNumber(options.backdropBlurPx, focus.blurPx),
    saturation: finiteNumber(options.saturation, 1.04),
    vignetteOpacity: clamp(finiteNumber(options.vignetteOpacity, 0.18), 0, 1),
    grainOpacity: clamp(finiteNumber(options.grainOpacity, 0.045), 0, 1),
  };
}

export function createMiniatureTurntableState(options = {}) {
  const camera = createMiniatureStandaloneCamera(options.camera || options);
  const motion = createMiniatureStandaloneMotion(options.motion || options);
  return {
    camera,
    motion,
    dragging: false,
    lastTime: 0,
  };
}

export function stepMiniatureTurntable(state, timeMs, options = {}) {
  if (!state || typeof state !== "object") {
    throw new Error("stepMiniatureTurntable() requires a state object.");
  }
  const motion = state.motion || createMiniatureStandaloneMotion();
  const camera = state.camera || createMiniatureStandaloneCamera();
  const lastTime = finiteNumber(state.lastTime, timeMs);
  const dt = Math.min(48, Math.max(0, finiteNumber(timeMs, lastTime) - lastTime || 16));
  const shouldPause = motion.pauseOnDrag !== false && (state.dragging || options.dragging);
  if (motion.enabled !== false && !shouldPause) {
    camera.yaw = finiteNumber(camera.yaw, 0) + dt * finiteNumber(motion.yawSpeed, 0.00022);
  }
  state.lastTime = finiteNumber(timeMs, lastTime + dt);
  state.camera = camera;
  state.motion = motion;
  return state;
}

export function inspectMiniatureStandaloneSurface(surface = {}) {
  const camera = surface.camera || {};
  const focus = surface.focus || {};
  const motion = surface.motion || {};
  const projection = surface.projection || {};
  const ui = surface.ui || {};
  const postprocess = surface.postprocess || {};
  const checks = {
    camera: Boolean(camera.projection === "perspective" && Number.isFinite(camera.fov) && camera.floorAxis),
    turntable: Boolean(motion.enabled !== false && motion.mode === "turntable" && motion.yawSpeed > 0),
    focusMask: Boolean(postprocess.focusMask !== false && focus.blurPx > 0),
    floorProjection: Boolean(projection.enabled !== false && projection.mode === "semantic-floor-projection"),
    quietUi: Boolean(ui.mode === "quiet-inspection-overlay" && ui.panelBlurPx > 0),
    frame: Boolean(postprocess.enabled !== false && postprocess.frame !== false),
  };
  const missing = Object.entries(checks)
    .filter(([, ok]) => !ok)
    .map(([key]) => key);
  return {
    schema: "metric.visual.miniature_standalone_surface_diagnostics.v1",
    ok: missing.length === 0,
    status: missing.length ? "partial" : "ready",
    checks,
    missing,
  };
}

export function createMiniatureStandaloneCss(options = {}) {
  const theme = normalizeTheme(options.theme);
  const focus = createMiniatureStandaloneFocus(options.focus || {});
  const ui = createMiniatureStandaloneUi(options.ui || {});
  const postprocess = createMiniatureStandalonePostprocess(options.postprocess || {}, focus);
  const c = MINIATURE_STANDALONE_CLASSNAMES;
  return `
.${c.root} {
  --metric-miniature-bg: ${theme.background};
  --metric-miniature-panel: ${theme.panel};
  --metric-miniature-line: ${theme.panelLine};
  --metric-miniature-text: ${theme.text};
  --metric-miniature-muted: ${theme.muted};
  --metric-miniature-accent: ${theme.accent};
  color: var(--metric-miniature-text);
  background: radial-gradient(circle at 50% 34%, oklch(24% 0.02 230), var(--metric-miniature-bg) 63%);
  font: 13px/1.45 -apple-system, BlinkMacSystemFont, "Segoe UI", system-ui, sans-serif;
}
.${c.stage} {
  position: absolute;
  inset: 0;
  overflow: hidden;
}
.${c.canvas} {
  position: absolute;
  inset: 0;
  width: 100%;
  height: 100%;
  display: block;
  cursor: grab;
}
.${c.canvas}:active { cursor: grabbing; }
.${c.topbar} {
  position: absolute;
  left: 16px;
  top: 16px;
  right: 16px;
  z-index: 8;
  display: grid;
  grid-template-columns: minmax(220px, 320px) 1fr auto;
  gap: 14px;
  align-items: start;
  pointer-events: none;
}
.${c.title},
.${c.controls},
.${c.legend},
.${c.inspector} {
  pointer-events: auto;
  border: 1px solid var(--metric-miniature-line);
  border-radius: ${ui.panelRadiusPx}px;
  background: var(--metric-miniature-panel);
  backdrop-filter: blur(${ui.panelBlurPx}px);
  -webkit-backdrop-filter: blur(${ui.panelBlurPx}px);
  box-shadow: 0 14px 40px oklch(0% 0 0 / 0.24);
}
.${c.title} {
  display: flex;
  flex-direction: column;
  min-width: 224px;
  padding: 10px 12px;
}
.${c.title} b { font-size: 16px; font-weight: 650; }
.${c.title} span { color: var(--metric-miniature-muted); font-size: 12px; }
.${c.controls} {
  justify-self: end;
  display: flex;
  gap: 8px;
  align-items: center;
  flex-wrap: wrap;
  max-width: 690px;
  justify-content: flex-end;
  padding: 8px;
}
.${c.control} {
  display: flex;
  gap: 6px;
  align-items: center;
  color: var(--metric-miniature-muted);
  white-space: nowrap;
  font-size: 12px;
}
.${c.control} input[type="range"] {
  width: 96px;
  accent-color: var(--metric-miniature-accent);
}
.${c.controls} button {
  font: inherit;
  color: var(--metric-miniature-text);
  background: oklch(18% 0.018 235 / 0.72);
  border: 1px solid var(--metric-miniature-line);
  border-radius: 7px;
  padding: 5px 8px;
  cursor: pointer;
}
.${c.focusMask} {
  pointer-events: none;
  position: absolute;
  inset: 0;
  z-index: 1;
  backdrop-filter: blur(${postprocess.backdropBlurPx}px) saturate(${postprocess.saturation});
  -webkit-backdrop-filter: blur(${postprocess.backdropBlurPx}px) saturate(${postprocess.saturation});
  mask-image: linear-gradient(to bottom,
    black 0%,
    black ${Math.round(focus.maskStartRatio * 100)}%,
    transparent ${Math.round(focus.maskNearRatio * 100)}%,
    transparent ${Math.round(focus.maskFarRatio * 100)}%,
    black ${Math.round(focus.maskEndRatio * 100)}%,
    black 100%);
  -webkit-mask-image: linear-gradient(to bottom,
    black 0%,
    black ${Math.round(focus.maskStartRatio * 100)}%,
    transparent ${Math.round(focus.maskNearRatio * 100)}%,
    transparent ${Math.round(focus.maskFarRatio * 100)}%,
    black ${Math.round(focus.maskEndRatio * 100)}%,
    black 100%);
}
.${c.frame} {
  pointer-events: none;
  position: absolute;
  inset: 0;
  z-index: 2;
  background:
    radial-gradient(ellipse at 50% 46%, transparent 0 48%, oklch(7% 0.012 235 / ${postprocess.vignetteOpacity}) 90%),
    linear-gradient(to bottom, oklch(94% 0.035 220 / 0.07), transparent 24%, transparent 70%, oklch(4% 0.01 235 / 0.18));
  mix-blend-mode: soft-light;
}
.${c.grain} {
  pointer-events: none;
  position: absolute;
  inset: 0;
  z-index: 3;
  opacity: ${postprocess.grainOpacity};
  background-image:
    linear-gradient(115deg, transparent 0 48%, oklch(95% 0.01 220 / 0.08) 49%, transparent 51%),
    radial-gradient(circle at 24% 17%, oklch(100% 0 0 / 0.05) 0 1px, transparent 1.4px);
  background-size: 9px 9px, 7px 7px;
}
.${c.legend} {
  position: absolute;
  left: 16px;
  bottom: 16px;
  z-index: 6;
  display: flex;
  gap: 6px;
  flex-wrap: wrap;
  width: min(430px, calc(100% - 32px));
  padding: 8px;
}
.${c.inspector} {
  position: absolute;
  right: 16px;
  bottom: 16px;
  z-index: 6;
  width: 238px;
  min-height: 70px;
  padding: 10px 11px;
}
@media (max-width: 900px) {
  .${c.topbar} {
    grid-template-columns: 1fr;
    left: 12px;
    right: 12px;
    top: 12px;
  }
  .${c.controls} {
    justify-self: start;
    max-width: 100%;
  }
  .${c.legend} {
    left: 12px;
    bottom: 10px;
    width: calc(100% - 24px);
  }
  .${c.inspector} {
    left: 12px;
    right: auto;
    bottom: 72px;
    width: min(300px, calc(100% - 24px));
  }
}`.trim();
}

function normalizeTheme(theme = {}) {
  return {
    ...DEFAULT_STANDALONE_THEME,
    ...(theme || {}),
  };
}

function point3(value, fallback) {
  if (!Array.isArray(value)) return fallback.slice();
  return [
    finiteNumber(value[0], fallback[0]),
    finiteNumber(value[1], fallback[1]),
    finiteNumber(value[2], fallback[2]),
  ];
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
  const number = finiteNumber(value, min);
  return Math.max(min, Math.min(max, number));
}

function deepFreeze(value) {
  if (!value || typeof value !== "object") return value;
  Object.freeze(value);
  for (const entry of Object.values(value)) {
    if (entry && typeof entry === "object" && !Object.isFrozen(entry)) deepFreeze(entry);
  }
  return value;
}
