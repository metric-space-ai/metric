import { BaseLayer } from "./BaseLayer.js";
import {
  colorChannel,
  combineScalarChannels,
  getChannel,
  getChannelArray,
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

export class InstancedPointLayer extends BaseLayer {
  constructor(descriptor = {}, rendererOrGl = null, options = {}) {
    super(descriptor, rendererOrGl, options);
    this.count = 0;
    this.buffers = {};
    this.program = null;
    this.pickProgram = null;
    this.pointSizeLimits = [1, 64];
    this.recordIds = [];
    this.pickColorRegistry = null;
    this.pickColorRegistrySize = -1;
  }

  setDescriptor(descriptor = {}) {
    super.setDescriptor(descriptor);
    if (this.program) this.needsUpload = true;
    return this;
  }

  ensureResources() {
    if (!this.gl) return false;
    if (!this.program) {
      this.program = this.track(createProgram(this.gl, "InstancedPointLayer", POINT_VERTEX_SHADER, POINT_FRAGMENT_SHADER));
      this.pointSizeLimits = pointSizeRange(this.gl);
      this.initialized = true;
    }
    if (this.needsUpload) this.upload();
    return true;
  }

  upload() {
    const gl = this.gl;
    this.count = inferInstanceCount(this.channels, ["position", "targetPosition", "color", "size", "alpha"]);
    const positions = positionChannel(this.channels, "position", this.count);
    const target = getChannel(this.channels, "targetPosition")
      ? positionChannel(this.channels, "targetPosition", this.count)
      : positions;
    const colors = colorChannel(this.channels, this.count, { defaultColor: this.options.defaultColor || [0.26, 0.45, 0.68, 1] });
    const sizes = scalarChannel(this.channels, ["size", "radius"], this.count, 1);
    const focus = combineScalarChannels(this.channels, ["focusWeight", "focus", "selection"], this.count, 0);
    const phase = scalarChannel(this.channels, ["animationPhase", "phase"], this.count, 0);
    this.recordIds = recordIdsForChannels(this.channels, this.count);
    this.pickColorRegistry = null;
    this.pickColorRegistrySize = -1;

    this.replaceBuffer("position", positions);
    this.replaceBuffer("targetPosition", target);
    this.replaceBuffer("color", colors);
    this.replaceBuffer("size", sizes);
    this.replaceBuffer("focus", focus);
    this.replaceBuffer("phase", phase);
    this.needsUpload = false;
  }

  render(context = {}) {
    if (this.disposed || this.visible === false || !this.ensureResources() || this.count <= 0) return this;
    const gl = this.gl;
    const material = this.material || {};
    const geometry = this.geometry || {};
    const animation = this.animation || {};
    const pointScale = numberOption(material.pointPixelScale, geometry.pointPixelScale, this.options.pointPixelScale, 8);
    const minSize = numberOption(material.minPointSize, geometry.minPointSize, this.options.minPointSize, this.pointSizeLimits[0] || 1);
    const maxSize = numberOption(material.maxPointSize, geometry.maxPointSize, this.options.maxPointSize, this.pointSizeLimits[1] || 64);
    const morph = animationProgress(animation, context);
    const shape = shapeCode(geometry.shape || material.shape || "disc");
    const blend = material.alphaMode === "blend" || material.transparent === true;
    const depthWrite = resolvePointDepthWrite(material);
    const drawMaterial = material.depthWrite === false && depthWrite
      ? { ...material, depthWrite: true }
      : material;

    configureDrawState(gl, drawMaterial, {
      blend,
      cullFace: false,
      depthWrite,
    });

    this.program.use();
    setCameraUniforms(this.program, context, this.renderer);
    this.program.setUniform("uTime", frameTimeSeconds(context));
    this.program.setUniform("uMorph", morph);
    this.program.setUniform("uPointPixelScale", pointScale);
    this.program.setUniform("uMinPointSize", minSize);
    this.program.setUniform("uMaxPointSize", maxSize);
    this.program.setUniform("uGlobalAlpha", numberOption(material.alpha, geometry.alpha, 1));
    this.program.setUniform("uShape", shape);
    this.program.setUniform("uFocusBoost", numberOption(material.focusBoost, 0.45));
    this.program.setUniform("uPhasePulse", numberOption(material.phasePulse, animation.mode === "attention" ? 0.18 : 0.08));
    this.program.setUniform("uAmbient", numberOption(material.ambient, 0.54));
    this.program.setUniform("uPointLight", numberOption(material.keyLight, material.pointLight, material.lightIntensity, 0.34));
    this.program.setUniform("uSpecular", numberOption(material.specular, 0.08));
    this.program.setUniform("uRimLight", numberOption(material.rimLight, material.rim, 0.16));
    this.program.setUniform("uHighlightLift", numberOption(material.highlightLift, 0.22));
    this.program.setUniform("uSphereShade", numberOption(material.sphereShade, 0.62));
    this.program.setUniform("uGloss", numberOption(material.gloss, 0.22));
    this.program.setUniform("uEdgeShade", numberOption(material.edgeShade, 0.38));
    this.program.setUniform("uMaterialSaturation", numberOption(material.saturation, 0.98));
    this.program.setUniform("uShadowDensity", numberOption(material.shadowDensity, 0.44));
    this.program.setUniform("uHighlightSharpness", numberOption(material.highlightSharpness, 0.36));
    this.program.setUniform("uAmbientColor", color3(material.ambientColor, [1, 1, 1]));
    this.program.setUniform("uLightColor", color3(material.lightColor, [1, 1, 1]));
    this.program.setUniform("uFocusColor", color3(material.focusColor, [1, 1, 1]));
    this.program.setUniform("uShadowTint", color3(material.shadowTint, [0.84, 0.88, 0.86]));

    bindAttribute(gl, this.program, this.capabilities, "aPosition", this.buffers.position, 3);
    bindAttribute(gl, this.program, this.capabilities, "aTargetPosition", this.buffers.targetPosition, 3);
    bindAttribute(gl, this.program, this.capabilities, "aColor", this.buffers.color, 4);
    bindAttribute(gl, this.program, this.capabilities, "aSize", this.buffers.size, 1);
    bindAttribute(gl, this.program, this.capabilities, "aFocus", this.buffers.focus, 1);
    bindAttribute(gl, this.program, this.capabilities, "aPhase", this.buffers.phase, 1);

    gl.drawArrays(gl.POINTS, 0, this.count);
    restoreDepthWrite(gl);
    return this;
  }

  renderPicking(context = {}) {
    if (this.disposed || this.visible === false || !this.ensureResources() || this.count <= 0) return this;
    if (!context.registry || !this.recordIds.length) return this;
    const gl = this.gl;
    const material = this.material || {};
    const geometry = this.geometry || {};
    const animation = this.animation || {};
    const pointScale = numberOption(material.pointPixelScale, geometry.pointPixelScale, this.options.pointPixelScale, 8);
    const minSize = numberOption(material.minPointSize, geometry.minPointSize, this.options.minPointSize, this.pointSizeLimits[0] || 1);
    const maxSize = numberOption(material.maxPointSize, geometry.maxPointSize, this.options.maxPointSize, this.pointSizeLimits[1] || 64);

    if (!this.pickProgram) {
      this.pickProgram = this.track(createProgram(gl, "InstancedPointLayerPicking", POINT_PICK_VERTEX_SHADER, POINT_PICK_FRAGMENT_SHADER));
    }
    this.updatePickColorBuffer(context);

    configureDrawState(gl, { depthWrite: true, alphaMode: "opaque" }, {
      blend: false,
      cullFace: false,
      depthWrite: true,
    });

    this.pickProgram.use();
    setCameraUniforms(this.pickProgram, context, this.renderer);
    this.pickProgram.setUniform("uMorph", animationProgress(animation, context));
    this.pickProgram.setUniform("uPointPixelScale", pointScale);
    this.pickProgram.setUniform("uMinPointSize", minSize);
    this.pickProgram.setUniform("uMaxPointSize", maxSize);

    bindAttribute(gl, this.pickProgram, this.capabilities, "aPosition", this.buffers.position, 3);
    bindAttribute(gl, this.pickProgram, this.capabilities, "aTargetPosition", this.buffers.targetPosition, 3);
    bindAttribute(gl, this.pickProgram, this.capabilities, "aSize", this.buffers.size, 1);
    bindAttribute(gl, this.pickProgram, this.capabilities, "aPickColor", this.buffers.pickColor, 4);

    gl.drawArrays(gl.POINTS, 0, this.count);
    restoreDepthWrite(gl);
    return this;
  }

  updatePickColorBuffer(context = {}) {
    const registry = context.registry;
    const registrySize = registry?.size ?? -1;
    if (this.buffers.pickColor && this.pickColorRegistry === registry && this.pickColorRegistrySize === registrySize) {
      return this;
    }
    const encode = context.encodePickIdRGBAFloat || encodePickIdRGBAFloatLocal;
    const colors = new Float32Array(this.count * 4);
    const scope = this.descriptor?.id || this.id || this.descriptor?.primitive || null;
    for (let index = 0; index < this.count; index += 1) {
      const recordId = this.recordIds[index] ?? String(index);
      const numericId = registry.registerRecord(String(recordId), {
        scope,
        layerId: this.id,
        index,
        descriptor: this.descriptor,
      });
      encode(numericId, colors, index * 4);
    }
    this.replaceBuffer("pickColor", colors);
    this.pickColorRegistry = registry;
    this.pickColorRegistrySize = registry?.size ?? registrySize;
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
    this.pickProgram = null;
    super.dispose();
  }
}

function recordIdsForChannels(channels, count) {
  const recordChannel = getChannel(channels, ["recordId", "record_id", "id", "cellId"]);
  const ids = getChannelArray(recordChannel);
  if (!ids) return Array.from({ length: count }, (_, index) => String(index));
  return Array.from({ length: count }, (_, index) => String(ids[index] ?? index));
}

function animationProgress(animation, context) {
  if (Number.isFinite(Number(animation.progress))) return Math.max(0, Math.min(1, Number(animation.progress)));
  if (!animation.loop || !Number.isFinite(Number(animation.durationMs))) return 0;
  const duration = Math.max(1, Number(animation.durationMs));
  const timeMs = frameTimeSeconds(context) * 1000;
  const t = (timeMs % duration) / duration;
  if (animation.direction === "alternate") return t < 0.5 ? t * 2 : 2 - t * 2;
  return t;
}

function resolvePointDepthWrite(material = {}) {
  if (material.depthWrite === true || material.alphaMode === "opaque") return true;
  if (material.alphaMode === "blend" || material.transparent === true) return false;
  return material.depthWrite !== false;
}

function numberOption(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number)) return number;
  }
  return 0;
}

function shapeCode(shape) {
  if (shape === "square" || shape === "rect") return 1;
  if (shape === "diamond") return 2;
  if (shape === "ring") return 3;
  return 0;
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

function encodePickIdRGBAFloatLocal(id, out = [0, 0, 0, 0], offset = 0) {
  const numericId = Math.max(0, Math.floor(Number(id) || 0));
  out[offset] = (numericId % 256) / 255;
  out[offset + 1] = (Math.floor(numericId / 256) % 256) / 255;
  out[offset + 2] = (Math.floor(numericId / 65536) % 256) / 255;
  out[offset + 3] = (Math.floor(numericId / 16777216) % 256) / 255;
  return out;
}

const POINT_VERTEX_SHADER = `
attribute vec3 aPosition;
attribute vec3 aTargetPosition;
attribute vec4 aColor;
attribute float aSize;
attribute float aFocus;
attribute float aPhase;

uniform mat4 uViewProjectionMatrix;
uniform float uTime;
uniform float uMorph;
uniform float uPointPixelScale;
uniform float uMinPointSize;
uniform float uMaxPointSize;
uniform float uFocusBoost;
uniform float uPhasePulse;

varying vec4 vColor;
varying float vFocus;
varying float vPhase;

void main() {
  vec3 position = mix(aPosition, aTargetPosition, clamp(uMorph, 0.0, 1.0));
  float phase = sin(uTime * 4.0 + aPhase * 6.28318530718) * 0.5 + 0.5;
  float focus = clamp(aFocus, 0.0, 1.0);
  float pulse = 1.0 + phase * uPhasePulse * max(focus, 0.25);
  gl_Position = uViewProjectionMatrix * vec4(position, 1.0);
  gl_PointSize = clamp(max(aSize, 0.001) * uPointPixelScale * (1.0 + focus * uFocusBoost) * pulse, uMinPointSize, uMaxPointSize);
  vColor = aColor;
  vFocus = focus;
  vPhase = phase;
}
`;

const POINT_FRAGMENT_SHADER = `
precision mediump float;

uniform float uGlobalAlpha;
uniform float uShape;
uniform float uAmbient;
uniform float uPointLight;
uniform float uSpecular;
uniform float uRimLight;
uniform float uHighlightLift;
uniform float uSphereShade;
uniform float uGloss;
uniform float uEdgeShade;
uniform float uMaterialSaturation;
uniform float uShadowDensity;
uniform float uHighlightSharpness;
uniform vec3 uAmbientColor;
uniform vec3 uLightColor;
uniform vec3 uFocusColor;
uniform vec3 uShadowTint;

varying vec4 vColor;
varying float vFocus;
varying float vPhase;

float luma(vec3 color) {
  return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

vec3 adjustSaturation(vec3 color, float amount) {
  return mix(vec3(luma(color)), color, clamp(amount, 0.0, 2.0));
}

void main() {
  vec2 p = gl_PointCoord * 2.0 - 1.0;
  float radius = length(p);
  float alpha = 1.0;
  if (uShape < 0.5) {
    alpha = 1.0 - smoothstep(0.78, 1.0, radius);
    if (radius > 1.0) discard;
  } else if (uShape < 1.5) {
    alpha = 1.0;
  } else if (uShape < 2.5) {
    float diamond = abs(p.x) + abs(p.y);
    alpha = 1.0 - smoothstep(0.86, 1.0, diamond);
    if (diamond > 1.0) discard;
  } else {
    alpha = smoothstep(0.42, 0.56, radius) * (1.0 - smoothstep(0.86, 1.0, radius));
    if (radius > 1.0 || radius < 0.38) discard;
  }
  float center = 1.0 - smoothstep(0.0, 0.95, radius);
  float edge = smoothstep(0.42, 1.0, radius);
  float normalZ = sqrt(max(0.001, 1.0 - dot(p, p) * 0.82));
  vec3 spriteNormal = normalize(vec3(p.x * 0.72, p.y * 0.72, normalZ));
  vec3 lightDir = normalize(vec3(-0.46, -0.58, 1.0));
  vec3 viewDir = vec3(0.0, 0.0, 1.0);
  float diffuse = max(dot(spriteNormal, lightDir), 0.0);
  float sphereShade = mix(1.0, 0.38 + diffuse * 0.62, clamp(uSphereShade, 0.0, 1.0));
  float sharpness = mix(8.0, 72.0, clamp(uHighlightSharpness, 0.0, 1.0));
  float gloss = pow(max(dot(normalize(lightDir + viewDir), spriteNormal), 0.0), sharpness) * clamp(uGloss, 0.0, 1.0);
  vec3 baseColor = adjustSaturation(vColor.rgb, uMaterialSaturation);
  vec3 lit = baseColor * uAmbientColor * uAmbient;
  lit += baseColor * uLightColor * uPointLight * sphereShade;
  lit += uLightColor * (pow(center, 3.0) * uSpecular + gloss * uSpecular);
  float edgeShade = edge * clamp(uEdgeShade, 0.0, 1.0) + (1.0 - diffuse) * clamp(uShadowDensity, 0.0, 1.0) * 0.42;
  lit = mix(lit, lit * uShadowTint, clamp(edgeShade * clamp(uRimLight + 0.32, 0.0, 1.0), 0.0, 1.0));
  lit = mix(lit, uFocusColor, vFocus * clamp(uHighlightLift, 0.0, 1.0));
  lit += uLightColor * vPhase * vFocus * 0.04;
  vec3 color = pow(max(lit, vec3(0.0)), vec3(1.0 / 2.2));
  gl_FragColor = vec4(color, vColor.a * alpha * uGlobalAlpha);
}
`;

const POINT_PICK_VERTEX_SHADER = `
attribute vec3 aPosition;
attribute vec3 aTargetPosition;
attribute float aSize;
attribute vec4 aPickColor;

uniform mat4 uViewProjectionMatrix;
uniform float uMorph;
uniform float uPointPixelScale;
uniform float uMinPointSize;
uniform float uMaxPointSize;

varying vec4 vPickColor;

void main() {
  vec3 position = mix(aPosition, aTargetPosition, clamp(uMorph, 0.0, 1.0));
  gl_Position = uViewProjectionMatrix * vec4(position, 1.0);
  gl_PointSize = clamp(max(aSize, 0.001) * uPointPixelScale, uMinPointSize, uMaxPointSize);
  vPickColor = aPickColor;
}
`;

const POINT_PICK_FRAGMENT_SHADER = `
precision mediump float;

varying vec4 vPickColor;

void main() {
  vec2 centered = gl_PointCoord * 2.0 - 1.0;
  if (dot(centered, centered) > 1.0) discard;
  gl_FragColor = vPickColor;
}
`;
