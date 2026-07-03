export class FocusLine {
  constructor(options = {}) {
    this.width = Math.max(1, Math.floor(options.width || 1));
    this.height = Math.max(1, Math.floor(options.height || 1));
    this.radius = clampNumber(options.radius, 0.0001, 100000, Math.max(this.height, this.width) * 0.12);
    this.falloff = clampNumber(options.falloff, 0.0001, 100000, this.radius * 1.6);
    this.focusBand = clampNumber(options.focusBand, 0, 100000, 0);
    this.blurCurve = clampNumber(options.blurCurve, 0.05, 100, 1);
    this.start = point2(options.start, [0, this.height * 0.5]);
    this.end = point2(options.end, [this.width, this.height * 0.5]);
    this.clampToSegment = options.clampToSegment === true;
  }

  setSize(width, height) {
    this.width = Math.max(1, Math.floor(width));
    this.height = Math.max(1, Math.floor(height));
    return this;
  }

  setRadius(radius, falloff = this.falloff) {
    this.radius = clampNumber(radius, 0.0001, 100000, this.radius);
    this.falloff = clampNumber(falloff, 0.0001, 100000, this.falloff);
    return this;
  }

  setOptics(options = {}) {
    if (options.focusBand !== undefined) {
      this.focusBand = clampNumber(options.focusBand, 0, 100000, this.focusBand);
    }
    if (options.blurCurve !== undefined) {
      this.blurCurve = clampNumber(options.blurCurve, 0.05, 100, this.blurCurve);
    }
    return this;
  }

  setLine(start, end) {
    this.start = point2(start, this.start);
    this.end = point2(end, this.end);
    return this;
  }

  setPoint(x, y, options = {}) {
    const angle = options.angle ?? this.angle();
    const length = options.length ?? Math.max(this.width, this.height) * 2;
    const half = length * 0.5;
    const dx = Math.cos(angle) * half;
    const dy = Math.sin(angle) * half;
    return this.setLine([x - dx, y - dy], [x + dx, y + dy]);
  }

  setFromPointer(event, element, options = {}) {
    const point = texturePointFromPointer(event, element, this.width, this.height, options);
    return this.setPoint(point.x, point.y, options);
  }

  angle() {
    const dx = this.end[0] - this.start[0];
    const dy = this.end[1] - this.start[1];
    if (Math.hypot(dx, dy) < 0.0001) return 0;
    return Math.atan2(dy, dx);
  }

  distanceToPoint(point) {
    return distanceToFocusLine(point2(point, [0, 0]), this.start, this.end, {
      clampToSegment: this.clampToSegment,
    });
  }

  focusAmountAt(point) {
    const distance = this.distanceToPoint(point);
    return 1 - smoothstep(this.radius, this.radius + this.falloff, distance);
  }

  toUniforms(prefix = "uFocus") {
    return {
      [`${prefix}Start`]: this.start.slice(),
      [`${prefix}End`]: this.end.slice(),
      [`${prefix}Radius`]: this.radius,
      [`${prefix}Band`]: this.focusBand,
      [`${prefix}BlurCurve`]: this.blurCurve,
      [`${prefix}Falloff`]: this.falloff,
      [`${prefix}TextureSize`]: [this.width, this.height],
    };
  }

  toJSON() {
    return {
      start: this.start.slice(),
      end: this.end.slice(),
      radius: this.radius,
      falloff: this.falloff,
      focusBand: this.focusBand,
      blurCurve: this.blurCurve,
      width: this.width,
      height: this.height,
      clampToSegment: this.clampToSegment,
    };
  }
}

export class OptionalDepthFocus {
  constructor(gl, options = {}) {
    this.gl = gl || null;
    this.depthTexture = options.depthTexture || null;
    this.near = clampNumber(options.near, 0.0001, 1000000, 0.1);
    this.far = clampNumber(options.far, this.near + 0.0001, 1000000, 1000);
    this.focusDepth = clampNumber(options.focusDepth, 0, 1, 0.5);
    this.depthRange = clampNumber(options.depthRange, 0.0001, 1, 0.08);
    this.capabilities = gl ? detectDepthTextureSupport(gl) : { supported: false, reason: "No WebGL context provided" };
    this.enabled = Boolean(options.enabled && this.depthTexture && this.capabilities.supported);
  }

  setDepthTexture(depthTexture) {
    this.depthTexture = depthTexture || null;
    this.enabled = Boolean(this.depthTexture && this.capabilities.supported);
    return this;
  }

  setCameraRange(near, far) {
    this.near = clampNumber(near, 0.0001, 1000000, this.near);
    this.far = clampNumber(far, this.near + 0.0001, 1000000, this.far);
    return this;
  }

  setFocusDepth(focusDepth, depthRange = this.depthRange) {
    this.focusDepth = clampNumber(focusDepth, 0, 1, this.focusDepth);
    this.depthRange = clampNumber(depthRange, 0.0001, 1, this.depthRange);
    return this;
  }

  toUniforms(prefix = "uDepthFocus") {
    return {
      [`${prefix}Enabled`]: this.enabled,
      [`${prefix}NearFar`]: [this.near, this.far],
      [`${prefix}FocusDepth`]: this.focusDepth,
      [`${prefix}Range`]: this.depthRange,
    };
  }
}

export function createFocusLine(options = {}) {
  return new FocusLine(options);
}

export function texturePointFromPointer(event, element, textureWidth, textureHeight, options = {}) {
  const rect = element.getBoundingClientRect();
  const cssX = event.clientX - rect.left;
  const cssY = event.clientY - rect.top;
  const x = cssX * (textureWidth / Math.max(1, rect.width));
  const yFromTop = cssY * (textureHeight / Math.max(1, rect.height));
  const flipY = options.flipY !== false;
  return {
    x,
    y: flipY ? textureHeight - yFromTop : yFromTop,
  };
}

export function detectDepthTextureSupport(gl) {
  if (!gl) return { supported: false, reason: "No WebGL context provided" };
  const isWebGL2 = typeof WebGL2RenderingContext !== "undefined" && gl instanceof WebGL2RenderingContext;
  if (isWebGL2) return { supported: true, mode: "webgl2" };
  const extension = gl.getExtension("WEBGL_depth_texture");
  if (extension) return { supported: true, mode: "extension", extension };
  return { supported: false, reason: "Depth textures are not available on this context" };
}

export function distanceToFocusLine(point, start, end, options = {}) {
  const px = point[0];
  const py = point[1];
  const ax = start[0];
  const ay = start[1];
  const bx = end[0];
  const by = end[1];
  const dx = bx - ax;
  const dy = by - ay;
  const lengthSq = dx * dx + dy * dy;
  if (lengthSq < 0.0001) return Math.hypot(px - ax, py - ay);
  const projection = ((px - ax) * dx + (py - ay) * dy) / lengthSq;
  const t = options.clampToSegment ? Math.max(0, Math.min(1, projection)) : projection;
  return Math.hypot(px - (ax + t * dx), py - (ay + t * dy));
}

function point2(value, fallback) {
  if (!value) return fallback.slice();
  if (Array.isArray(value)) return [Number(value[0]) || 0, Number(value[1]) || 0];
  return [Number(value.x) || 0, Number(value.y) || 0];
}

function smoothstep(edge0, edge1, x) {
  const t = Math.max(0, Math.min(1, (x - edge0) / Math.max(edge1 - edge0, 0.0001)));
  return t * t * (3 - 2 * t);
}

function clampNumber(value, min, max, fallback) {
  const number = Number(value);
  if (!Number.isFinite(number)) return fallback;
  return Math.min(max, Math.max(min, number));
}
