/*
 * Tilt-shift postprocess pass.
 *
 * The blur model is adapted from the local tilt-shift filter Tilt Shift filter and the source visual references
 * wrapper for it, but implemented here as dependency-free native WebGL.
 */

import { FullscreenQuadPass, RenderTarget } from "./postprocess.js";

export const TILT_SHIFT_FRAGMENT_SHADER = `
  precision mediump float;

  uniform sampler2D tDiffuse;
  uniform float blurRadius;
  uniform float gradientRadius;
  uniform vec2 start;
  uniform vec2 end;
  uniform vec2 delta;
  uniform vec2 texSize;
  varying vec2 vUv;

  float random(vec3 scale, float seed) {
    /* use the fragment position for a different seed per-pixel */
    return fract(sin(dot(gl_FragCoord.xyz + seed, scale)) * 43758.5453 + seed);
  }

  void main() {
    vec4 color = vec4(0.0);
    float total = 0.0;

    /* randomize the lookup values to hide the fixed number of samples */
    float offset = random(vec3(12.9898, 78.233, 151.7182), 0.0);

    vec2 normal = normalize(vec2(start.y - end.y, end.x - start.x));
    float radius = smoothstep(0.0, 1.0, abs(dot(vUv * texSize - start, normal)) / gradientRadius) * blurRadius;
    for (float t = -30.0; t <= 30.0; t++) {
      float percent = (t + offset - 0.5) / 30.0;
      float weight = 1.0 - abs(percent);
      vec4 texel = texture2D(tDiffuse, vUv + delta / texSize * percent * radius);

      /* switch to pre-multiplied alpha to correctly blur transparent images */
      texel.rgb *= texel.a;

      color += texel * weight;
      total += 2.0 * weight;
    }

    gl_FragColor = color / total;

    /* switch back from pre-multiplied alpha */
    gl_FragColor.rgb /= gl_FragColor.a + 0.00001;
  }
`;

export class TiltShiftPass {
  constructor(gl, options = {}) {
    this.gl = gl;
    this.enabled = options.enabled !== false;
    this.width = Math.max(1, Math.floor(options.width || gl.canvas?.width || 1));
    this.height = Math.max(1, Math.floor(options.height || gl.canvas?.height || 1));
    this.blurRadius = options.blurRadius ?? 10;
    this.gradientRadius = options.gradientRadius ?? 100;
    this.focusBand = options.focusBand ?? 0;
    this.blurCurve = options.blurCurve ?? 1;
    this.start = toPoint(options.start, [0, this.height * 0.5]);
    this.end = toPoint(options.end, [this.width, this.height * 0.5]);
    this.tempTarget = new RenderTarget(gl, this.width, this.height);
    this.pass1 = new FullscreenQuadPass(gl, {
      fragmentShader: TILT_SHIFT_FRAGMENT_SHADER,
      textureUniform: "tDiffuse",
    });
    this.pass2 = new FullscreenQuadPass(gl, {
      fragmentShader: TILT_SHIFT_FRAGMENT_SHADER,
      textureUniform: "tDiffuse",
    });
    this.setSize(this.width, this.height);
    this.updateUniforms();
  }

  setSize(width, height) {
    this.width = Math.max(1, Math.floor(width));
    this.height = Math.max(1, Math.floor(height));
    this.tempTarget.setSize(this.width, this.height);
    this.pass1.setSize(this.width, this.height);
    this.pass2.setSize(this.width, this.height);
    this.updateUniforms();
  }

  setBlurRadius(value) {
    this.blurRadius = Math.max(0, Number(value) || 0);
    this.updateUniforms();
    return this;
  }

  setGradientRadius(value) {
    this.gradientRadius = Math.max(0.0001, Number(value) || 0.0001);
    this.updateUniforms();
    return this;
  }

  setFocusBand(value) {
    this.focusBand = Math.max(0, Number(value) || 0);
    this.updateUniforms();
    return this;
  }

  setBlurCurve(value) {
    this.blurCurve = Math.max(0.05, Number(value) || 1);
    this.updateUniforms();
    return this;
  }

  setFocusLine(start, end) {
    this.start = toPoint(start, this.start);
    this.end = toPoint(end, this.end);
    this.updateUniforms();
    return this;
  }

  setFocusPoint(x, y, options = {}) {
    const angle = options.angle ?? this.focusAngle();
    const length = options.length ?? Math.max(this.width, this.height) * 2;
    const half = length * 0.5;
    const dx = Math.cos(angle) * half;
    const dy = Math.sin(angle) * half;
    return this.setFocusLine([x - dx, y - dy], [x + dx, y + dy]);
  }

  setFocusLineFromPointer(event, element = this.gl.canvas, options = {}) {
    const point = texturePointFromPointer(event, element, this.width, this.height, options);
    return this.setFocusPoint(point.x, point.y, options);
  }

  setFocusState(focusLine) {
    if (!focusLine) return this;
    if (typeof focusLine.toUniforms === "function") {
      const uniforms = focusLine.toUniforms();
      this.gradientRadius = Math.max(0.0001, finiteOr(uniforms.uFocusRadius, this.gradientRadius));
      this.focusBand = Math.max(0, finiteOr(uniforms.uFocusBand, this.focusBand));
      this.blurCurve = Math.max(0.05, finiteOr(uniforms.uFocusBlurCurve, this.blurCurve));
      this.start = toPoint(uniforms.uFocusStart, this.start);
      this.end = toPoint(uniforms.uFocusEnd, this.end);
    } else {
      this.gradientRadius = Math.max(0.0001, finiteOr(focusLine.radius, this.gradientRadius));
      this.focusBand = Math.max(0, finiteOr(focusLine.focusBand, this.focusBand));
      this.blurCurve = Math.max(0.05, finiteOr(focusLine.blurCurve, this.blurCurve));
      this.start = toPoint(focusLine.start, this.start);
      this.end = toPoint(focusLine.end, this.end);
    }
    this.updateUniforms();
    return this;
  }

  setOptions(options = {}) {
    if ("blurRadius" in options) this.blurRadius = Math.max(0, Number(options.blurRadius) || 0);
    if ("gradientRadius" in options) this.gradientRadius = Math.max(0.0001, Number(options.gradientRadius) || 0.0001);
    if ("focusBand" in options) this.focusBand = Math.max(0, Number(options.focusBand) || 0);
    if ("blurCurve" in options) this.blurCurve = Math.max(0.05, Number(options.blurCurve) || 1);
    if ("start" in options || "end" in options) {
      this.start = toPoint(options.start, this.start);
      this.end = toPoint(options.end, this.end);
    }
    this.updateUniforms();
    return this;
  }

  focusAngle() {
    const dx = this.end[0] - this.start[0];
    const dy = this.end[1] - this.start[1];
    if (Math.hypot(dx, dy) < 0.0001) return 0;
    return Math.atan2(dy, dx);
  }

  render(context, inputTexture, outputTarget = null) {
    const width = context.width || this.width;
    const height = context.height || this.height;
    if (width !== this.width || height !== this.height) {
      this.setSize(width, height);
    }
    this.updateUniforms();
    this.pass1.render(context, inputTexture, this.tempTarget);
    this.pass2.render(context, this.tempTarget.texture, outputTarget);
  }

  dispose() {
    this.pass1.dispose();
    this.pass2.dispose();
    this.tempTarget.dispose();
  }

  updateUniforms() {
    let line = [this.end[0] - this.start[0], this.end[1] - this.start[1]];
    let length = Math.hypot(line[0], line[1]);
    if (length < 0.0001) {
      line = [1, 0];
      length = 1;
    }
    const along = [line[0] / length, line[1] / length];
    const across = [-along[1], along[0]];
    const shared = {
      blurRadius: this.blurRadius,
      gradientRadius: this.gradientRadius,
      start: this.start,
      end: this.end,
      texSize: [this.width, this.height],
    };

    for (const [name, value] of Object.entries(shared)) {
      this.pass1.setUniform(name, value);
      this.pass2.setUniform(name, value);
    }
    this.pass1.setUniform("delta", along);
    this.pass2.setUniform("delta", across);
  }
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

function toPoint(value, fallback) {
  if (!value) return [fallback[0], fallback[1]];
  if (Array.isArray(value)) return [Number(value[0]) || 0, Number(value[1]) || 0];
  return [Number(value.x) || 0, Number(value.y) || 0];
}

function finiteOr(value, fallback) {
  const number = Number(value);
  return Number.isFinite(number) ? number : fallback;
}
