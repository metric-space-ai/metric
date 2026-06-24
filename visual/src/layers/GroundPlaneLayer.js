import { BaseLayer } from "./BaseLayer.js";
import {
  bindAttribute,
  configureDrawState,
  createBuffer,
  createProgram,
  restoreDepthWrite,
  setCameraUniforms,
  trackBuffer,
} from "./gl-utils.js";

export class GroundPlaneLayer extends BaseLayer {
  constructor(descriptor = {}, rendererOrGl = null, options = {}) {
    super(descriptor, rendererOrGl, options);
    this.program = null;
    this.buffers = {};
    this.vertexCount = 0;
  }

  ensureResources() {
    if (!this.gl) return false;
    if (!this.program) {
      this.program = this.track(createProgram(this.gl, "GroundPlaneLayer", GROUND_VERTEX_SHADER, GROUND_FRAGMENT_SHADER));
    }
    if (this.needsUpload) this.upload();
    return true;
  }

  upload() {
    const geometry = this.geometry || {};
    const width = positiveNumber(geometry.width, geometry.size, 5);
    const depth = positiveNumber(geometry.depth, geometry.size, 5);
    const y = finiteNumber(geometry.y, geometry.groundY, -0.75);
    const positions = new Float32Array([
      -width * 0.5, y, -depth * 0.5,
       width * 0.5, y, -depth * 0.5,
      -width * 0.5, y,  depth * 0.5,
       width * 0.5, y, -depth * 0.5,
       width * 0.5, y,  depth * 0.5,
      -width * 0.5, y,  depth * 0.5,
    ]);
    const uv = new Float32Array([
      0, 0,
      1, 0,
      0, 1,
      1, 0,
      1, 1,
      0, 1,
    ]);

    this.vertexCount = 6;
    this.replaceBuffer("position", positions);
    this.replaceBuffer("uv", uv);
    this.needsUpload = false;
  }

  render(context = {}) {
    if (this.disposed || this.visible === false || !this.ensureResources() || this.vertexCount <= 0) return this;
    const gl = this.gl;
    const material = this.material || {};
    const geometry = this.geometry || {};

    configureDrawState(gl, material, {
      blend: true,
      cullFace: false,
      depthWrite: false,
    });

    this.program.use();
    setCameraUniforms(this.program, context, this.renderer);
    this.program.setUniform("uBaseColor", color3(material.baseColor, [0.075, 0.095, 0.105]));
    this.program.setUniform("uGridColor", color3(material.gridColor, [0.28, 0.40, 0.42]));
    this.program.setUniform("uAxisXColor", color3(material.axisXColor, [0.44, 0.67, 0.82]));
    this.program.setUniform("uAxisZColor", color3(material.axisZColor, [0.94, 0.55, 0.20]));
    this.program.setUniform("uAlpha", finiteNumber(material.alpha, 0.72));
    this.program.setUniform("uGridScale", positiveNumber(material.gridScale, geometry.gridScale, 8));
    this.program.setUniform("uGridWidth", positiveNumber(material.gridWidth, 0.025));
    this.program.setUniform("uAxisWidth", positiveNumber(material.axisWidth, 0.016));
    this.program.setUniform("uFade", positiveNumber(material.fade, 0.78));
    this.program.setUniform("uGridAlpha", finiteNumber(material.gridAlpha, 0.34));
    this.program.setUniform("uAxisAlpha", finiteNumber(material.axisAlpha, 0.32));
    this.program.setUniform("uStageTilt", finiteNumber(material.stageTilt, -0.12));
    this.program.setUniform("uStageScale", vec2(material.stageScale, [0.92, 0.78]));
    this.program.setUniform("uHorizonColor", color3(material.horizonColor, [0.94, 0.95, 0.91]));
    this.program.setUniform("uSheenColor", color3(material.sheenColor, [1.0, 0.98, 0.9]));
    this.program.setUniform("uSheen", finiteNumber(material.sheen, 0.12));
    this.program.setUniform("uMatte", finiteNumber(material.matte, 0.72));
    this.program.setUniform("uContactShade", finiteNumber(material.contactShade, 0.08));
    this.program.setUniform("uHorizonFade", finiteNumber(material.horizonFade, 0.24));

    bindAttribute(gl, this.program, this.capabilities, "aPosition", this.buffers.position, 3);
    bindAttribute(gl, this.program, this.capabilities, "aUv", this.buffers.uv, 2);
    gl.drawArrays(gl.TRIANGLES, 0, this.vertexCount);
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

function color3(value, fallback) {
  if (Array.isArray(value) || ArrayBuffer.isView(value)) {
    const divisor = Math.max(value[0] || 0, value[1] || 0, value[2] || 0) > 1 ? 255 : 1;
    return [
      clamp01(finiteNumber(value[0], fallback[0]) / divisor),
      clamp01(finiteNumber(value[1], fallback[1]) / divisor),
      clamp01(finiteNumber(value[2], fallback[2]) / divisor),
    ];
  }
  if (typeof value === "string") {
    const hex = value.trim();
    if (/^#([0-9a-f]{3}|[0-9a-f]{6})$/i.test(hex)) {
      const full = hex.length === 4
        ? `#${hex[1]}${hex[1]}${hex[2]}${hex[2]}${hex[3]}${hex[3]}`
        : hex;
      return [
        parseInt(full.slice(1, 3), 16) / 255,
        parseInt(full.slice(3, 5), 16) / 255,
        parseInt(full.slice(5, 7), 16) / 255,
      ];
    }
  }
  return fallback;
}

function vec2(value, fallback) {
  if (Array.isArray(value) || ArrayBuffer.isView(value)) {
    return [finiteNumber(value[0], fallback[0]), finiteNumber(value[1], fallback[1])];
  }
  if (value && typeof value === "object") {
    return [finiteNumber(value.x, fallback[0]), finiteNumber(value.y, fallback[1])];
  }
  return fallback.slice();
}

function clamp01(value) {
  return Math.max(0, Math.min(1, Number.isFinite(value) ? value : 0));
}

const GROUND_VERTEX_SHADER = `
attribute vec3 aPosition;
attribute vec2 aUv;

uniform mat4 uViewProjectionMatrix;

varying vec2 vUv;
varying vec3 vWorld;

void main() {
  vUv = aUv;
  vWorld = aPosition;
  gl_Position = uViewProjectionMatrix * vec4(aPosition, 1.0);
}
`;

const GROUND_FRAGMENT_SHADER = `
precision mediump float;

uniform vec3 uBaseColor;
uniform vec3 uGridColor;
uniform vec3 uAxisXColor;
uniform vec3 uAxisZColor;
uniform float uAlpha;
uniform float uGridScale;
uniform float uGridWidth;
uniform float uAxisWidth;
uniform float uFade;
uniform float uGridAlpha;
uniform float uAxisAlpha;
uniform float uStageTilt;
uniform vec2 uStageScale;
uniform vec3 uHorizonColor;
uniform vec3 uSheenColor;
uniform float uSheen;
uniform float uMatte;
uniform float uContactShade;
uniform float uHorizonFade;

varying vec2 vUv;
varying vec3 vWorld;

float gridLine(float coordinate) {
  float cell = abs(fract(coordinate * uGridScale - 0.5) - 0.5);
  return 1.0 - smoothstep(uGridWidth, uGridWidth + 0.012, cell);
}

void main() {
  vec2 centered = vUv * 2.0 - 1.0;
  vec2 stageUv = centered;
  stageUv.x += stageUv.y * uStageTilt;
  stageUv /= max(uStageScale, vec2(0.001));
  float radial = length(stageUv);
  float fade = 1.0 - smoothstep(uFade, 1.18, radial);
  float frontDepth = pow(clamp(1.0 - vUv.y, 0.0, 1.0), 1.15);
  float backDepth = smoothstep(0.44, 1.0, vUv.y);
  float grid = max(gridLine(vUv.x), gridLine(vUv.y));
  float axisX = 1.0 - smoothstep(uAxisWidth, uAxisWidth + 0.008, abs(vWorld.z));
  float axisZ = 1.0 - smoothstep(uAxisWidth, uAxisWidth + 0.008, abs(vWorld.x));
  vec2 lightDir = normalize(vec2(-0.54, 0.84));
  float sweep = dot(centered * vec2(0.78, 1.0), lightDir) * 0.5 + 0.5;
  float sheen = smoothstep(0.48, 0.96, sweep) * smoothstep(0.06, 0.62, vUv.y) * (1.0 - radial * 0.52);
  float contactShade = frontDepth * (1.0 - smoothstep(0.12, 0.9, radial));
  vec3 color = mix(uBaseColor, uHorizonColor, backDepth * clamp(uHorizonFade, 0.0, 1.0));
  color = mix(color, uSheenColor, sheen * clamp(uSheen, 0.0, 1.0));
  color = mix(color, color * vec3(0.82, 0.86, 0.84), contactShade * clamp(uContactShade, 0.0, 1.0));
  color = mix(color, uGridColor, grid * clamp(uGridAlpha, 0.0, 1.0));
  color = mix(color, uAxisXColor, axisX * clamp(uAxisAlpha, 0.0, 1.0));
  color = mix(color, uAxisZColor, axisZ * clamp(uAxisAlpha, 0.0, 1.0) * 0.86);
  color = mix(vec3(dot(color, vec3(0.2126, 0.7152, 0.0722))), color, clamp(uMatte, 0.0, 1.4));
  float alpha = uAlpha * fade * (0.5 + grid * 0.24 + max(axisX, axisZ) * 0.18 + sheen * 0.12);
  gl_FragColor = vec4(color, alpha);
}
`;
