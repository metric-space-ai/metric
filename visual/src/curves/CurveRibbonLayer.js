import { BaseLayer } from "../layers/BaseLayer.js";
import {
  bindAttribute,
  configureDrawState,
  createBuffer,
  createProgram,
  frameTimeSeconds,
  restoreDepthWrite,
  setCameraUniforms,
  trackBuffer,
} from "../layers/gl-utils.js";
import { buildRibbonPathBuffers } from "./tessellation.js";

/**
 * Native WebGL layer for thick camera-facing path ribbons.
 *
 * The layer consumes `start`, `end`, `side`, `along`, `color`, and `width`
 * channels, typically produced by `createTubeRibbonPathLayerDescriptor`.
 * Widths are screen pixels. Coordinates are already-exported METRIC evidence;
 * this class only uploads and draws GPU buffers.
 */
export class CurveRibbonLayer extends BaseLayer {
  constructor(descriptor = {}, rendererOrGl = null, options = {}) {
    super(descriptor, rendererOrGl, options);
    this.vertexCount = 0;
    this.buffers = {};
    this.program = null;
  }

  ensureResources() {
    if (!this.gl) return false;
    if (!this.program) {
      this.program = this.track(createProgram(this.gl, "CurveRibbonLayer", RIBBON_VERTEX_SHADER, RIBBON_FRAGMENT_SHADER));
    }
    if (this.needsUpload) this.upload();
    return true;
  }

  upload() {
    const source = hasRibbonChannels(this.channels)
      ? { channels: this.channels, vertexCount: inferChannelCount(this.channels.start, 3) }
      : buildRibbonPathBuffers(this.source?.evidence || this.source || this.descriptor.source, {
        ...this.geometry,
        ...this.material,
      });
    const channels = source.channels || {};
    this.vertexCount = source.vertexCount || inferChannelCount(channels.start, 3);
    this.replaceBuffer("start", channelArray(channels.start, 3, this.vertexCount));
    this.replaceBuffer("end", channelArray(channels.end, 3, this.vertexCount));
    this.replaceBuffer("side", channelArray(channels.side, 1, this.vertexCount));
    this.replaceBuffer("along", channelArray(channels.along, 1, this.vertexCount));
    this.replaceBuffer("color", channelArray(channels.color, 4, this.vertexCount, [0.16, 0.52, 0.64, 0.82]));
    this.replaceBuffer("width", channelArray(channels.width, 1, this.vertexCount, [3]));
    this.replaceBuffer("distance", channelArray(channels.distance, 1, this.vertexCount));
    this.needsUpload = false;
  }

  render(context = {}) {
    if (this.disposed || this.visible === false || !this.ensureResources() || this.vertexCount <= 0) return this;
    const gl = this.gl;
    const material = this.material || {};
    configureDrawState(gl, material, {
      blend: true,
      cullFace: false,
      depthWrite: material.depthWrite === true,
    });

    this.program.use();
    setCameraUniforms(this.program, context, this.renderer);
    this.program.setUniform("uViewport", resolveViewport(gl, context, this.renderer));
    this.program.setUniform("uGlobalAlpha", finiteNumber(material.alpha, 1));
    this.program.setUniform("uWidthScale", finiteNumber(material.widthScale, 1));
    this.program.setUniform("uDepthBias", finiteNumber(material.depthBias, 0));
    this.program.setUniform("uAmbient", finiteNumber(material.ambient, 0.42));
    this.program.setUniform("uPointLight", finiteNumber(material.pointLight, material.lightIntensity, 0.5));
    this.program.setUniform("uEmission", finiteNumber(material.emission, 0.18));
    this.program.setUniform("uRimLight", finiteNumber(material.rimLight, 0.26));
    this.program.setUniform("uCoreGlow", finiteNumber(material.coreGlow, 0.14));
    this.program.setUniform("uEdgeFeather", finiteNumber(material.edgeFeather, material.feather, 0.32));
    this.program.setUniform("uTubeShade", finiteNumber(material.tubeShade, 0.62));
    this.program.setUniform("uFlowStrength", finiteNumber(material.flowStrength, 0.08));
    this.program.setUniform("uFlowScale", finiteNumber(material.flowScale, 2.4));
    this.program.setUniform("uFlowSpeed", finiteNumber(material.flowSpeed, 0.18));
    this.program.setUniform("uSaturation", finiteNumber(material.saturation, 1));
    this.program.setUniform("uDepthShade", finiteNumber(material.depthShade, 0.2));
    this.program.setUniform("uTime", frameTimeSeconds(context));
    this.program.setUniform("uAmbientColor", color3(material.ambientColor, [1, 1, 1]));
    this.program.setUniform("uLightColor", color3(material.lightColor, [1, 1, 1]));
    this.program.setUniform("uShadowTint", color3(material.shadowTint, [0.58, 0.66, 0.7]));
    this.program.setUniform("uHighlightColor", color3(material.highlightColor, [1, 0.96, 0.86]));
    bindAttribute(gl, this.program, this.capabilities, "aStart", this.buffers.start, 3);
    bindAttribute(gl, this.program, this.capabilities, "aEnd", this.buffers.end, 3);
    bindAttribute(gl, this.program, this.capabilities, "aSide", this.buffers.side, 1);
    bindAttribute(gl, this.program, this.capabilities, "aAlong", this.buffers.along, 1);
    bindAttribute(gl, this.program, this.capabilities, "aColor", this.buffers.color, 4);
    bindAttribute(gl, this.program, this.capabilities, "aWidth", this.buffers.width, 1);
    bindAttribute(gl, this.program, this.capabilities, "aDistance", this.buffers.distance, 1);
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

function hasRibbonChannels(channels = {}) {
  return Boolean(channels.start && channels.end && channels.side && channels.along);
}

function inferChannelCount(channel, itemSize) {
  const count = Number(channel?.count);
  if (Number.isFinite(count) && count >= 0) return Math.floor(count);
  const array = rawArray(channel);
  return array ? Math.floor(array.length / itemSize) : 0;
}

function channelArray(channel, itemSize, count, fallback = [0]) {
  const out = new Float32Array(count * itemSize);
  const source = rawArray(channel);
  const sourceItemSize = Math.max(1, Math.floor(Number(channel?.itemSize || channel?.size || itemSize)));
  for (let index = 0; index < count; index += 1) {
    for (let component = 0; component < itemSize; component += 1) {
      const fallbackValue = fallback[Math.min(component, fallback.length - 1)] ?? 0;
      const value = source?.[index * sourceItemSize + Math.min(component, sourceItemSize - 1)];
      out[index * itemSize + component] = finiteNumber(value, fallbackValue);
    }
  }
  return out;
}

function rawArray(channel) {
  if (ArrayBuffer.isView(channel) || Array.isArray(channel)) return channel;
  if (ArrayBuffer.isView(channel?.array) || Array.isArray(channel?.array)) return channel.array;
  if (ArrayBuffer.isView(channel?.data) || Array.isArray(channel?.data)) return channel.data;
  return null;
}

function resolveViewport(gl, context, renderer) {
  const viewport = context?.viewport || renderer?.viewport;
  const width = viewport?.width || viewport?.[2] || gl.canvas?.width || 1;
  const height = viewport?.height || viewport?.[3] || gl.canvas?.height || 1;
  return [Math.max(1, width), Math.max(1, height)];
}

function finiteNumber(value, fallback) {
  const number = Number(value);
  return Number.isFinite(number) ? number : fallback;
}

function color3(value, fallback) {
  if (Array.isArray(value) || ArrayBuffer.isView(value)) {
    const source = Array.from(value);
    const divisor = Math.max(source[0] || 0, source[1] || 0, source[2] || 0) > 1 ? 255 : 1;
    return [
      clamp01((Number(source[0]) || 0) / divisor),
      clamp01((Number(source[1]) || 0) / divisor),
      clamp01((Number(source[2]) || 0) / divisor),
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

function clamp01(value) {
  return Math.max(0, Math.min(1, Number.isFinite(value) ? value : 0));
}

const RIBBON_VERTEX_SHADER = `
attribute vec3 aStart;
attribute vec3 aEnd;
attribute float aSide;
attribute float aAlong;
attribute vec4 aColor;
attribute float aWidth;
attribute float aDistance;

uniform mat4 uViewProjectionMatrix;
uniform vec2 uViewport;
uniform float uWidthScale;
uniform float uDepthBias;

varying vec4 vColor;
varying float vDistance;
varying float vSide;
varying float vAlong;

void main() {
  vec4 clipStart = uViewProjectionMatrix * vec4(aStart, 1.0);
  vec4 clipEnd = uViewProjectionMatrix * vec4(aEnd, 1.0);
  vec4 center = mix(clipStart, clipEnd, aAlong);
  vec2 startNdc = clipStart.xy / max(abs(clipStart.w), 0.000001);
  vec2 endNdc = clipEnd.xy / max(abs(clipEnd.w), 0.000001);
  vec2 direction = endNdc - startNdc;
  float directionLength = length(direction);
  direction = directionLength > 0.000001 ? direction / directionLength : vec2(1.0, 0.0);
  vec2 normal = vec2(-direction.y, direction.x);
  vec2 pixelOffset = normal * aSide * aWidth * uWidthScale;
  vec2 clipOffset = (pixelOffset / max(uViewport, vec2(1.0))) * 2.0 * center.w;
  gl_Position = center + vec4(clipOffset, uDepthBias * center.w, 0.0);
  vColor = aColor;
  vDistance = aDistance;
  vSide = abs(aSide);
  vAlong = aAlong;
}
`;

const RIBBON_FRAGMENT_SHADER = `
precision mediump float;

uniform float uGlobalAlpha;
uniform float uAmbient;
uniform float uPointLight;
uniform float uEmission;
uniform float uRimLight;
uniform float uCoreGlow;
uniform float uEdgeFeather;
uniform float uTubeShade;
uniform float uFlowStrength;
uniform float uFlowScale;
uniform float uFlowSpeed;
uniform float uSaturation;
uniform float uDepthShade;
uniform float uTime;
uniform vec3 uAmbientColor;
uniform vec3 uLightColor;
uniform vec3 uShadowTint;
uniform vec3 uHighlightColor;

varying vec4 vColor;
varying float vDistance;
varying float vSide;
varying float vAlong;

void main() {
  float side = clamp(vSide, 0.0, 1.0);
  float feather = clamp(uEdgeFeather, 0.04, 0.96);
  float edge = smoothstep(1.0 - feather, 1.0, side);
  float core = 1.0 - edge;
  float tube = pow(max(0.0, 1.0 - side * side), max(0.08, uTubeShade));
  float longitudinal = 0.72 + 0.28 * sin(vAlong * 3.14159265);
  float flow = 0.5 + 0.5 * sin(vDistance * uFlowScale - uTime * uFlowSpeed);
  float flowHighlight = smoothstep(0.72, 1.0, flow) * uFlowStrength * core;
  vec3 gray = vec3(dot(vColor.rgb, vec3(0.299, 0.587, 0.114)));
  vec3 base = mix(gray, vColor.rgb, clamp(uSaturation, 0.0, 1.8));
  vec3 shadedBase = mix(base * uShadowTint, base, 1.0 - uDepthShade * (1.0 - tube));
  vec3 lit = shadedBase * (uAmbientColor * uAmbient + uLightColor * uPointLight * (0.28 + tube * 0.72));
  lit += uHighlightColor * (pow(max(tube, 0.0), 2.0) * uCoreGlow + flowHighlight);
  lit += uLightColor * (core * uEmission + pow(core, 3.0) * uRimLight) * longitudinal;
  vec3 color = pow(max(lit, vec3(0.0)), vec3(1.0 / 2.2));
  float alpha = vColor.a * uGlobalAlpha * (1.0 - edge * 0.46);
  gl_FragColor = vec4(color, alpha);
}
`;
