import { BaseLayer } from "./BaseLayer.js";
import {
  colorChannel,
  getChannel,
  inferInstanceCount,
  positionChannel,
  scalarChannel,
} from "./channels.js";
import {
  bindAttribute,
  configureDrawState,
  createBuffer,
  createProgram,
  frameTimeSeconds,
  pointSizeRange,
  restoreDepthWrite,
  setCameraUniforms,
  trackBuffer,
} from "./gl-utils.js";

export class HeatFieldLayer extends BaseLayer {
  constructor(descriptor = {}, rendererOrGl = null, options = {}) {
    super(descriptor, rendererOrGl, options);
    this.count = 0;
    this.buffers = {};
    this.program = null;
    this.pointSizeLimits = [1, 64];
  }

  ensureResources() {
    if (!this.gl) return false;
    if (!this.program) {
      this.program = this.track(createProgram(this.gl, "HeatFieldLayer", HEAT_VERTEX_SHADER, HEAT_FRAGMENT_SHADER));
      this.pointSizeLimits = pointSizeRange(this.gl);
    }
    if (this.needsUpload) this.upload();
    return true;
  }

  upload() {
    const gl = this.gl;
    this.count = inferInstanceCount(this.channels, ["position", "scalar", "color", "radius", "size", "alpha"]);
    const positions = positionChannel(this.channels, "position", this.count);
    const colors = colorChannel(this.channels, this.count, { defaultColor: [0.20, 0.50, 0.58, 0.72] });
    const scalar = scalarChannel(this.channels, ["scalar", "value"], this.count, 0);
    const radiusChannel = getChannel(this.channels, "radius") || getChannel(this.channels, "size");
    const radius = radiusChannel
      ? scalarChannel(this.channels, radiusChannel === getChannel(this.channels, "radius") ? "radius" : "size", this.count, defaultRadius(this.geometry))
      : new Float32Array(this.count).fill(defaultRadius(this.geometry));

    this.replaceBuffer("position", positions);
    this.replaceBuffer("color", colors);
    this.replaceBuffer("scalar", scalar);
    this.replaceBuffer("radius", radius);
    this.needsUpload = false;
  }

  render(context = {}) {
    if (this.disposed || this.visible === false || !this.ensureResources() || this.count <= 0) return this;
    const gl = this.gl;
    const material = this.material || {};
    const geometry = this.geometry || {};
    const radial = geometry.mode === "grid" || material.shape === "cell" ? 0 : 1;

    configureDrawState(gl, material, {
      blend: true,
      cullFace: false,
      depthWrite: material.depthWrite === true,
    });

    this.program.use();
    setCameraUniforms(this.program, context, this.renderer);
    this.program.setUniform("uTime", frameTimeSeconds(context));
    this.program.setUniform("uPointPixelScale", numberOption(material.pointPixelScale, geometry.pointPixelScale, this.options.pointPixelScale, 10));
    this.program.setUniform("uMinPointSize", numberOption(material.minPointSize, geometry.minPointSize, 1));
    this.program.setUniform("uMaxPointSize", numberOption(material.maxPointSize, geometry.maxPointSize, this.pointSizeLimits[1] || 128));
    this.program.setUniform("uGlobalAlpha", numberOption(material.alpha, geometry.alpha, 1));
    this.program.setUniform("uRadial", radial);
    this.program.setUniform("uContour", numberOption(material.contour, geometry.contour, 0.18));
    this.program.setUniform("uGlow", numberOption(material.glow, geometry.glow, 0.14));

    bindAttribute(gl, this.program, this.capabilities, "aPosition", this.buffers.position, 3);
    bindAttribute(gl, this.program, this.capabilities, "aColor", this.buffers.color, 4);
    bindAttribute(gl, this.program, this.capabilities, "aScalar", this.buffers.scalar, 1);
    bindAttribute(gl, this.program, this.capabilities, "aRadius", this.buffers.radius, 1);
    gl.drawArrays(gl.POINTS, 0, this.count);
    restoreDepthWrite(gl);
    return this;
  }

  replaceBuffer(name, data) {
    const gl = this.gl;
    if (this.buffers[name]) {
      gl.deleteBuffer(this.buffers[name]);
      this.resources = this.resources.filter((resource) => resource.handle !== this.buffers[name]);
    }
    this.buffers[name] = createBuffer(gl, data, gl.STATIC_DRAW);
    trackBuffer(this, this.buffers[name]);
  }

  dispose() {
    this.buffers = {};
    this.program = null;
    super.dispose();
  }
}

function defaultRadius(geometry = {}) {
  if (Array.isArray(geometry.cellSize)) {
    return Math.max(0.001, (Number(geometry.cellSize[0]) || 1) + (Number(geometry.cellSize[1]) || 1)) * 0.5;
  }
  return Number.isFinite(Number(geometry.radius)) ? Number(geometry.radius) : 1;
}

function numberOption(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number)) return number;
  }
  return 0;
}

const HEAT_VERTEX_SHADER = `
attribute vec3 aPosition;
attribute vec4 aColor;
attribute float aScalar;
attribute float aRadius;

uniform mat4 uViewProjectionMatrix;
uniform float uPointPixelScale;
uniform float uMinPointSize;
uniform float uMaxPointSize;

varying vec4 vColor;
varying float vScalar;

void main() {
  gl_Position = uViewProjectionMatrix * vec4(aPosition, 1.0);
  gl_PointSize = clamp(max(aRadius, 0.001) * uPointPixelScale, uMinPointSize, uMaxPointSize);
  vColor = aColor;
  vScalar = aScalar;
}
`;

const HEAT_FRAGMENT_SHADER = `
precision mediump float;

uniform float uGlobalAlpha;
uniform float uRadial;
uniform float uTime;
uniform float uContour;
uniform float uGlow;

varying vec4 vColor;
varying float vScalar;

void main() {
  vec2 centered = gl_PointCoord * 2.0 - 1.0;
  float radius = length(centered);
  float alpha = 1.0;
  if (uRadial > 0.5) {
    if (radius > 1.0) discard;
    alpha = 1.0 - smoothstep(0.25, 1.0, radius);
  }
  float scalar = clamp(vScalar, 0.0, 1.0);
  float contour = 0.5 + 0.5 * sin((radius * 8.0 - scalar * 5.0 + uTime * 0.65) * 3.14159265359);
  float scalarLift = scalar * 0.08 + contour * uContour * (1.0 - radius) * 0.12;
  vec3 color = min(vColor.rgb + scalarLift + vec3(uGlow * scalar * (1.0 - radius)), vec3(1.0));
  float fieldAlpha = alpha * (0.82 + scalar * 0.18 + contour * uContour * 0.08);
  gl_FragColor = vec4(color, vColor.a * fieldAlpha * uGlobalAlpha);
}
`;
