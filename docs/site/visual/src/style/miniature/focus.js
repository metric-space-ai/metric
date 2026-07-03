const DEFAULT_FOCUS_WIDTH = 100;
const DEFAULT_FOCUS_HEIGHT = 100;
const DEFAULT_FOCUS_BAND_RATIO = 0.045;
const DEFAULT_BLUR_CURVE = 1.35;

/**
 * Creates the serializable focus-plane contract used by miniature stages.
 *
 * The contract is viewport-ratio based so the same stage can be resized without
 * baking a one-off pixel focus line into an example.
 *
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniatureFocusPlane(options = {}) {
  const width = positiveNumber(options.width, options.viewportWidth, DEFAULT_FOCUS_WIDTH);
  const height = positiveNumber(options.height, options.viewportHeight, DEFAULT_FOCUS_HEIGHT);
  const axis = options.axis === "vertical" ? "vertical" : "horizontal";
  const explicitStart = normalizePoint2(options.start, null);
  const explicitEnd = normalizePoint2(options.end, null);
  const midpoint = midpointFromLine(explicitStart, explicitEnd);
  const xRatio = clamp01(finiteNumber(
    options.xRatio,
    ratioFromCoordinate(options.x, width),
    ratioFromCoordinate(options.focusX, width),
    midpoint ? midpoint[0] / width : undefined,
    0.5,
  ));
  const yRatio = clamp01(finiteNumber(
    options.yRatio,
    ratioFromCoordinate(options.y, height),
    ratioFromCoordinate(options.focusY, height),
    midpoint ? midpoint[1] / height : undefined,
    0.5,
  ));
  const radiusRatio = positiveNumber(
    options.radiusRatio,
    ratioFromCoordinate(options.gradientRadius, height),
    ratioFromCoordinate(options.radius, height),
    1 / 3,
  );
  const focusBandRatio = Math.max(0, finiteNumber(
    options.focusBandRatio,
    ratioFromCoordinate(options.focusBand, height),
    DEFAULT_FOCUS_BAND_RATIO,
  ));
  const anchor = options.anchor || (explicitStart && explicitEnd ? "viewport-pixels" : "viewport-ratio");
  const focus = {
    enabled: options.enabled !== false,
    kind: "miniature-focus-plane",
    mode: "tilt-shift-line",
    anchor,
    axis,
    xRatio,
    yRatio,
    radiusRatio,
    focusBandRatio,
    blurRadius: Math.max(0, finiteNumber(options.blurRadius, 10)),
    blurCurve: Math.max(0.05, finiteNumber(options.blurCurve, DEFAULT_BLUR_CURVE)),
    width,
    height,
    pointer: {
      enabled: options.pointer?.enabled ?? options.pointer !== false,
      axis,
      mode: axis === "vertical" ? "viewport-x-focus-line" : "viewport-y-focus-line",
      normalizedInput: true,
      preserveRadius: true,
      ...(typeof options.pointer === "object" ? options.pointer : {}),
    },
  };
  const line = resolveMiniatureFocusLine(focus, width, height);
  focus.start = line.start;
  focus.end = line.end;
  focus.radius = line.radius;
  focus.focusBand = line.focusBand;
  focus.blurCurve = line.blurCurve;
  focus.gradientRadius = line.radius;
  return deepFreeze(focus);
}

/**
 * Resolves a miniature focus contract to pixel-space uniforms.
 *
 * @param {object} focus
 * @param {number} width
 * @param {number} height
 * @returns {{start:number[], end:number[], radius:number, axis:string}}
 */
export function resolveMiniatureFocusLine(focus = {}, width = DEFAULT_FOCUS_WIDTH, height = DEFAULT_FOCUS_HEIGHT) {
  const resolvedWidth = positiveNumber(width, focus.width, DEFAULT_FOCUS_WIDTH);
  const resolvedHeight = positiveNumber(height, focus.height, DEFAULT_FOCUS_HEIGHT);
  const axis = focus.axis === "vertical" ? "vertical" : "horizontal";
  const focusBandRatio = finiteNumber(focus.focusBandRatio, DEFAULT_FOCUS_BAND_RATIO);
  const focusBand = focus.anchor === "viewport-ratio"
    ? Math.max(0, focusBandRatio * resolvedHeight)
    : Math.max(0, finiteNumber(focus.focusBand, focusBandRatio * resolvedHeight, 0));
  const blurCurve = Math.max(0.05, finiteNumber(focus.blurCurve, DEFAULT_BLUR_CURVE));
  const radius = focus.anchor === "viewport-ratio"
    ? positiveNumber(
      finiteNumber(focus.radiusRatio, 1 / 3) * resolvedHeight,
      resolvedHeight / 3,
    )
    : positiveNumber(
      focus.radius,
      focus.gradientRadius,
      finiteNumber(focus.radiusRatio, 1 / 3) * resolvedHeight,
      resolvedHeight / 3,
    );
  if (focus.anchor !== "viewport-ratio" && (focus.start || focus.end)) {
    return {
      axis,
      radius,
      focusBand,
      blurCurve,
      start: normalizePoint2(focus.start, axis === "vertical" ? [resolvedWidth * 0.5, 0] : [0, resolvedHeight * 0.5]),
      end: normalizePoint2(focus.end, axis === "vertical" ? [resolvedWidth * 0.5, resolvedHeight] : [resolvedWidth, resolvedHeight * 0.5]),
    };
  }
  if (axis === "vertical") {
    const x = clamp(finiteNumber(focus.x, finiteNumber(focus.xRatio, 0.5) * resolvedWidth), 0, resolvedWidth);
    return { axis, radius, focusBand, blurCurve, start: [x, 0], end: [x, resolvedHeight] };
  }
  const y = clamp(finiteNumber(focus.y, focus.focusY, finiteNumber(focus.yRatio, 0.5) * resolvedHeight), 0, resolvedHeight);
  return { axis, radius, focusBand, blurCurve, start: [0, y], end: [resolvedWidth, y] };
}

/**
 * Creates native tilt-shift pass options from the focus-plane contract.
 *
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniatureTiltShiftOptions(options = {}) {
  const focus = options.kind === "miniature-focus-plane" ? options : createMiniatureFocusPlane(options);
  const line = resolveMiniatureFocusLine(focus, focus.width, focus.height);
  return deepFreeze({
    enabled: focus.enabled !== false,
    kind: "tilt-shift",
    focusKind: focus.kind,
    passes: 2,
    blurRadius: Math.max(0, finiteNumber(options.blurRadius, focus.blurRadius, 10)),
    gradientRadius: line.radius,
    focusBand: line.focusBand,
    focusBandRatio: focus.focusBandRatio,
    blurCurve: line.blurCurve,
    start: line.start,
    end: line.end,
    axis: line.axis,
    pointer: focus.pointer || {
      enabled: true,
      axis: line.axis,
      mode: line.axis === "vertical" ? "viewport-x-focus-line" : "viewport-y-focus-line",
      normalizedInput: true,
      preserveRadius: true,
    },
  });
}

function midpointFromLine(start, end) {
  if (!start || !end) return null;
  return [(start[0] + end[0]) * 0.5, (start[1] + end[1]) * 0.5];
}

function ratioFromCoordinate(value, extent) {
  const number = Number(value);
  if (!Number.isFinite(number)) return undefined;
  const size = positiveNumber(extent, 1);
  return number / size;
}

function normalizePoint2(value, fallback) {
  if (Array.isArray(value) || ArrayBuffer.isView(value)) {
    return [finiteNumber(value[0], fallback?.[0] ?? 0), finiteNumber(value[1], fallback?.[1] ?? 0)];
  }
  if (value && typeof value === "object") {
    return [finiteNumber(value.x, fallback?.[0] ?? 0), finiteNumber(value.y, fallback?.[1] ?? 0)];
  }
  return fallback ? fallback.slice() : null;
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
  if (!value || typeof value !== "object" || Object.isFrozen(value)) return value;
  Object.freeze(value);
  for (const entry of Object.values(value)) deepFreeze(entry);
  return value;
}
