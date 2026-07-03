import { BaseLayer } from "./BaseLayer.js";
import {
  colorChannel,
  getChannel,
  getChannelArray,
  inferInstanceCount,
  positionChannel,
  scalarChannel,
} from "./channels.js";
import { buildFootprintTriangles, projectToGroundPlane } from "./geometry.js";
import {
  bindAttribute,
  configureDrawState,
  createBuffer,
  createProgram,
  pointSizeRange,
  restoreDepthWrite,
  setCameraUniforms,
  trackBuffer,
} from "./gl-utils.js";

export class GroundProjectionLayer extends BaseLayer {
  constructor(descriptor = {}, rendererOrGl = null, options = {}) {
    super(descriptor, rendererOrGl, options);
    this.count = 0;
    this.footprintCount = 0;
    this.buffers = {};
    this.spriteProgram = null;
    this.footprintProgram = null;
    this.pickProgram = null;
    this.pointSizeLimits = [1, 64];
    this.renderMode = "sprites";
    this.recordIds = [];
    this.pickColorRegistry = null;
    this.pickColorRegistrySize = -1;
  }

  ensureResources() {
    if (!this.gl) return false;
    if (!this.spriteProgram) {
      this.spriteProgram = this.track(createProgram(this.gl, "GroundProjectionLayer sprites", PROJECTION_VERTEX_SHADER, PROJECTION_FRAGMENT_SHADER));
      this.footprintProgram = this.track(createProgram(this.gl, "GroundProjectionLayer footprints", FOOTPRINT_VERTEX_SHADER, FOOTPRINT_FRAGMENT_SHADER));
      this.pointSizeLimits = pointSizeRange(this.gl);
    }
    if (this.needsUpload) this.upload();
    return true;
  }

  upload() {
    if (Array.isArray(this.geometry.footprints) && !this.channels.position) {
      this.uploadFootprints();
    } else {
      this.uploadSprites();
    }
    this.needsUpload = false;
  }

  uploadSprites() {
    const gl = this.gl;
    this.renderMode = "sprites";
    this.count = inferInstanceCount(this.channels, ["position", "targetPosition", "color", "size", "alpha"]);
    const rawPositions = positionChannel(this.channels, "position", this.count);
    const rawTargets = getChannel(this.channels, "targetPosition")
      ? positionChannel(this.channels, "targetPosition", this.count)
      : rawPositions;
    const positions = projectToGroundPlane(rawPositions, this.geometry);
    const targetPositions = projectToGroundPlane(rawTargets, this.geometry);
    const sourceHeight = heightChannel(rawPositions, this.count, this.geometry);
    const targetHeight = heightChannel(rawTargets, this.count, this.geometry);
    const colors = colorChannel(this.channels, this.count, { defaultColor: [0.12, 0.20, 0.24, 0.36] });
    const size = scalarChannel(this.channels, ["size", "radius"], this.count, defaultSize(this.geometry));
    this.recordIds = recordIdsForChannels(this.channels, this.count);
    this.pickColorRegistry = null;
    this.pickColorRegistrySize = -1;

    this.replaceBuffer("position", positions);
    this.replaceBuffer("targetPosition", targetPositions);
    this.replaceBuffer("sourceHeight", sourceHeight);
    this.replaceBuffer("targetHeight", targetHeight);
    this.replaceBuffer("color", colors);
    this.replaceBuffer("size", size);
  }

  uploadFootprints() {
    this.renderMode = "footprints";
    const triangles = buildFootprintTriangles(this.geometry.footprints, this.geometry);
    this.footprintCount = triangles.count;
    this.replaceBuffer("footprintPosition", triangles.positions);
    this.replaceBuffer("footprintColor", triangles.colors);
  }

  render(context = {}) {
    if (this.disposed || this.visible === false || !this.ensureResources()) return this;
    return this.renderMode === "footprints" ? this.renderFootprints(context) : this.renderSprites(context);
  }

  renderSprites(context) {
    if (this.count <= 0) return this;
    const gl = this.gl;
    const material = this.material || {};
    const geometry = this.geometry || {};
    const morph = animationProgress(this.animation || {}, context);

    configureDrawState(gl, material, {
      blend: true,
      cullFace: false,
      depthWrite: false,
    });

    this.spriteProgram.use();
    setCameraUniforms(this.spriteProgram, context, this.renderer);
    this.spriteProgram.setUniform("uMorph", morph);
    this.spriteProgram.setUniform("uPointPixelScale", numberOption(material.pointPixelScale, geometry.pointPixelScale, this.options.pointPixelScale, 12));
    this.spriteProgram.setUniform("uMinPointSize", numberOption(material.minPointSize, geometry.minPointSize, 1));
    this.spriteProgram.setUniform("uMaxPointSize", numberOption(material.maxPointSize, geometry.maxPointSize, this.pointSizeLimits[1] || 128));
    this.spriteProgram.setUniform("uGlobalAlpha", numberOption(material.alpha, geometry.alpha, 1));
    this.spriteProgram.setUniform("uSoftness", numberOption(material.softness, geometry.softness, 0.58));
    this.spriteProgram.setUniform("uFootprintStretch", numberOption(material.footprintStretch, geometry.footprintStretch, 1));
    this.spriteProgram.setUniform("uFootprintSkew", numberOption(material.footprintSkew, geometry.footprintSkew, 0));
    this.spriteProgram.setUniform("uFootprintCore", numberOption(material.footprintCore, geometry.footprintCore, 0.16));
    this.spriteProgram.setUniform("uFootprintFalloff", numberOption(material.footprintFalloff, geometry.footprintFalloff, 1));
    this.spriteProgram.setUniform("uColorMix", numberOption(material.colorMix, geometry.colorMix, 1));
    this.spriteProgram.setUniform("uShadowDensity", numberOption(material.shadowDensity, geometry.shadowDensity, 1));
    this.spriteProgram.setUniform("uCoreDensity", numberOption(material.coreDensity, geometry.coreDensity, 0.28));
    this.spriteProgram.setUniform("uEdgeTint", numberOption(material.edgeTint, geometry.edgeTint, 0.55));
    this.spriteProgram.setUniform("uNeutralShadow", color3Option(material.neutralShadow, material.shadowColor, geometry.neutralShadow, [0.46, 0.5, 0.52]));
    this.spriteProgram.setUniform("uShadowDirection", vec2Option(material.shadowDirection, geometry.shadowDirection, [-0.44, 0.9]));
    this.spriteProgram.setUniform("uShadowTail", numberOption(material.shadowTail, geometry.shadowTail, 0.62));
    this.spriteProgram.setUniform("uShadowTailStrength", numberOption(material.shadowTailStrength, geometry.shadowTailStrength, 0.34));
    this.spriteProgram.setUniform("uContactHardness", numberOption(material.contactHardness, geometry.contactHardness, 0.36));
    this.spriteProgram.setUniform("uSurfaceLift", numberOption(material.surfaceLift, geometry.surfaceLift, 0.04));
    this.spriteProgram.setUniform("uHeightShadowScale", numberOption(material.heightShadowScale, geometry.heightShadowScale, 0.22));
    this.spriteProgram.setUniform("uHeightAlphaFade", numberOption(material.heightAlphaFade, geometry.heightAlphaFade, 0.28));

    bindAttribute(gl, this.spriteProgram, this.capabilities, "aPosition", this.buffers.position, 3);
    bindAttribute(gl, this.spriteProgram, this.capabilities, "aTargetPosition", this.buffers.targetPosition, 3);
    bindAttribute(gl, this.spriteProgram, this.capabilities, "aSourceHeight", this.buffers.sourceHeight, 1);
    bindAttribute(gl, this.spriteProgram, this.capabilities, "aTargetHeight", this.buffers.targetHeight, 1);
    bindAttribute(gl, this.spriteProgram, this.capabilities, "aColor", this.buffers.color, 4);
    bindAttribute(gl, this.spriteProgram, this.capabilities, "aSize", this.buffers.size, 1);
    gl.drawArrays(gl.POINTS, 0, this.count);
    restoreDepthWrite(gl);
    return this;
  }

  renderFootprints(context) {
    if (this.footprintCount <= 0) return this;
    const gl = this.gl;
    const material = this.material || {};
    configureDrawState(gl, material, {
      blend: true,
      cullFace: false,
      depthWrite: false,
    });

    this.footprintProgram.use();
    setCameraUniforms(this.footprintProgram, context, this.renderer);
    this.footprintProgram.setUniform("uGlobalAlpha", numberOption(material.alpha, this.geometry.alpha, 1));
    bindAttribute(gl, this.footprintProgram, this.capabilities, "aPosition", this.buffers.footprintPosition, 3);
    bindAttribute(gl, this.footprintProgram, this.capabilities, "aColor", this.buffers.footprintColor, 4);
    gl.drawArrays(gl.TRIANGLES, 0, this.footprintCount);
    restoreDepthWrite(gl);
    return this;
  }

  renderPicking(context = {}) {
    if (this.disposed || this.visible === false || !this.ensureResources()) return this;
    if (this.renderMode !== "sprites" || this.count <= 0 || !context.registry || !this.recordIds.length) return this;
    const gl = this.gl;
    const material = this.material || {};
    const geometry = this.geometry || {};
    const morph = animationProgress(this.animation || {}, context);

    if (!this.pickProgram) {
      this.pickProgram = this.track(createProgram(gl, "GroundProjectionLayerPicking", PROJECTION_PICK_VERTEX_SHADER, PROJECTION_PICK_FRAGMENT_SHADER));
    }
    this.updatePickColorBuffer(context);

    configureDrawState(gl, { depthWrite: true, alphaMode: "opaque" }, {
      blend: false,
      cullFace: false,
      depthWrite: true,
    });

    this.pickProgram.use();
    setCameraUniforms(this.pickProgram, context, this.renderer);
    this.pickProgram.setUniform("uMorph", morph);
    this.pickProgram.setUniform("uPointPixelScale", numberOption(material.pointPixelScale, geometry.pointPixelScale, this.options.pointPixelScale, 12));
    this.pickProgram.setUniform("uMinPointSize", numberOption(material.minPointSize, geometry.minPointSize, 1));
    this.pickProgram.setUniform("uMaxPointSize", numberOption(material.maxPointSize, geometry.maxPointSize, this.pointSizeLimits[1] || 128));
    this.pickProgram.setUniform("uHeightShadowScale", numberOption(material.heightShadowScale, geometry.heightShadowScale, 0.22));

    bindAttribute(gl, this.pickProgram, this.capabilities, "aPosition", this.buffers.position, 3);
    bindAttribute(gl, this.pickProgram, this.capabilities, "aTargetPosition", this.buffers.targetPosition, 3);
    bindAttribute(gl, this.pickProgram, this.capabilities, "aSourceHeight", this.buffers.sourceHeight, 1);
    bindAttribute(gl, this.pickProgram, this.capabilities, "aTargetHeight", this.buffers.targetHeight, 1);
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
    this.spriteProgram = null;
    this.footprintProgram = null;
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

function defaultSize(geometry = {}) {
  return Number.isFinite(Number(geometry.size)) ? Number(geometry.size) : 1;
}

function heightChannel(positions, count, geometry = {}) {
  const groundY = Number.isFinite(Number(geometry.groundY)) ? Number(geometry.groundY) : 0;
  const out = new Float32Array(Math.max(0, count));
  for (let index = 0; index < count; index += 1) {
    const y = Number(positions[index * 3 + 1]);
    out[index] = Math.max(0, Number.isFinite(y) ? y - groundY : 0);
  }
  return out;
}

function animationProgress(animation, context) {
  if (Number.isFinite(Number(animation.progress))) return Math.max(0, Math.min(1, Number(animation.progress)));
  if (!animation.loop || !Number.isFinite(Number(animation.durationMs))) return 0;
  const duration = Math.max(1, Number(animation.durationMs));
  const elapsed = typeof context?.time?.elapsed === "number"
    ? context.time.elapsed
    : (typeof performance !== "undefined" ? performance.now() : 0);
  const phase = ((elapsed % duration) + duration) % duration;
  const t = phase / duration;
  if (animation.direction === "alternate") {
    return t < 0.5 ? t * 2 : 2 - t * 2;
  }
  return t;
}

function numberOption(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number)) return number;
  }
  return 0;
}

function color3Option(...values) {
  const fallback = values.pop() || [0, 0, 0];
  for (const value of values) {
    if (!value) continue;
    if (Array.isArray(value) || ArrayBuffer.isView(value)) {
      const divisor = Math.max(value[0] || 0, value[1] || 0, value[2] || 0) > 1 ? 255 : 1;
      return [
        clamp01(numberOption(value[0], fallback[0]) / divisor),
        clamp01(numberOption(value[1], fallback[1]) / divisor),
        clamp01(numberOption(value[2], fallback[2]) / divisor),
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
    if (typeof value === "object") {
      return [
        numberOption(value.r, value.x, fallback[0]),
        numberOption(value.g, value.y, fallback[1]),
        numberOption(value.b, value.z, fallback[2]),
      ];
    }
  }
  return fallback.slice();
}

function vec2Option(...values) {
  const fallback = values.pop() || [1, 0];
  for (const value of values) {
    if (!value) continue;
    let x;
    let y;
    if (Array.isArray(value) || ArrayBuffer.isView(value)) {
      x = numberOption(value[0], fallback[0]);
      y = numberOption(value[1], fallback[1]);
    } else if (typeof value === "object") {
      x = numberOption(value.x, value[0], fallback[0]);
      y = numberOption(value.y, value[1], fallback[1]);
    } else {
      continue;
    }
    const length = Math.hypot(x, y) || 1;
    return [x / length, y / length];
  }
  const length = Math.hypot(fallback[0], fallback[1]) || 1;
  return [fallback[0] / length, fallback[1] / length];
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

const PROJECTION_VERTEX_SHADER = `
attribute vec3 aPosition;
attribute vec3 aTargetPosition;
attribute vec4 aColor;
attribute float aSize;
attribute float aSourceHeight;
attribute float aTargetHeight;

uniform mat4 uViewProjectionMatrix;
uniform float uMorph;
uniform float uPointPixelScale;
uniform float uMinPointSize;
uniform float uMaxPointSize;
uniform float uHeightShadowScale;

varying vec4 vColor;
varying float vHeight;

void main() {
  vec3 position = mix(aPosition, aTargetPosition, clamp(uMorph, 0.0, 1.0));
  float height = mix(aSourceHeight, aTargetHeight, clamp(uMorph, 0.0, 1.0));
  float heightScale = 1.0 + clamp(height * uHeightShadowScale, 0.0, 1.8);
  gl_Position = uViewProjectionMatrix * vec4(position, 1.0);
  gl_PointSize = clamp(max(aSize, 0.001) * uPointPixelScale * heightScale, uMinPointSize, uMaxPointSize);
  vColor = aColor;
  vHeight = height;
}
`;

const PROJECTION_FRAGMENT_SHADER = `
precision mediump float;

uniform float uGlobalAlpha;
uniform float uSoftness;
uniform float uFootprintStretch;
uniform float uFootprintSkew;
uniform float uFootprintCore;
uniform float uFootprintFalloff;
uniform float uColorMix;
uniform float uShadowDensity;
uniform float uCoreDensity;
uniform float uEdgeTint;
uniform vec3 uNeutralShadow;
uniform vec2 uShadowDirection;
uniform float uShadowTail;
uniform float uShadowTailStrength;
uniform float uContactHardness;
uniform float uSurfaceLift;
uniform float uHeightAlphaFade;

varying vec4 vColor;
varying float vHeight;

void main() {
  vec2 p = gl_PointCoord * 2.0 - 1.0;
  vec2 q = vec2(p.x + p.y * uFootprintSkew, p.y / max(uFootprintStretch, 0.001));
  float radius = length(q);
  float falloff = max(uFootprintFalloff, 0.001);
  vec2 shadowDir = normalize(uShadowDirection);
  float along = dot(q, shadowDir);
  float side = length(q - shadowDir * along);
  float tailReach = max(uShadowTail, 0.001) * falloff;
  float tailT = clamp(along / tailReach, 0.0, 1.0);
  float tailWidth = mix(falloff * 0.52, falloff * 0.16, tailT);
  float tailAlpha = (1.0 - smoothstep(tailWidth * 0.56, tailWidth, side))
    * smoothstep(-0.08, 0.08, along)
    * (1.0 - smoothstep(tailReach * 0.34, tailReach, along))
    * clamp(uShadowTailStrength, 0.0, 1.0);
  if (radius > falloff && tailAlpha <= 0.001) discard;
  float inner = clamp(uFootprintCore, 0.0, 0.98);
  float featherStart = clamp(1.0 - uSoftness, inner, falloff);
  float alpha = 1.0 - smoothstep(featherStart, falloff, radius);
  float core = 1.0 - smoothstep(0.0, max(inner, 0.001), radius);
  float contact = pow(core, mix(1.8, 0.72, clamp(uContactHardness, 0.0, 1.0)));
  alpha = max(alpha * (0.64 + contact * (0.32 + clamp(uCoreDensity, 0.0, 1.0) * 0.34)), tailAlpha);
  float heightFade = clamp(vHeight * uHeightAlphaFade, 0.0, 0.72);
  alpha *= mix(1.0, 0.46, heightFade);
  float luma = dot(vColor.rgb, vec3(0.299, 0.587, 0.114));
  float colorMix = clamp(uColorMix, 0.0, 1.0);
  float semanticMix = colorMix * mix(clamp(uEdgeTint, 0.0, 1.0), 1.0, contact);
  vec3 semanticColor = mix(vec3(luma), vColor.rgb, colorMix);
  vec3 neutralShadow = clamp(uNeutralShadow * (0.95 - contact * clamp(uCoreDensity, 0.0, 1.0) * 0.28), 0.0, 1.0);
  vec3 shadowColor = mix(neutralShadow, semanticColor, semanticMix);
  shadowColor = mix(shadowColor, vec3(1.0), clamp(uSurfaceLift, 0.0, 1.0) * (1.0 - contact) * (1.0 - tailAlpha));
  float density = clamp(uShadowDensity, 0.0, 4.0);
  gl_FragColor = vec4(shadowColor, vColor.a * alpha * uGlobalAlpha * density);
}
`;

const PROJECTION_PICK_VERTEX_SHADER = `
attribute vec3 aPosition;
attribute vec3 aTargetPosition;
attribute float aSize;
attribute float aSourceHeight;
attribute float aTargetHeight;
attribute vec4 aPickColor;

uniform mat4 uViewProjectionMatrix;
uniform float uMorph;
uniform float uPointPixelScale;
uniform float uMinPointSize;
uniform float uMaxPointSize;
uniform float uHeightShadowScale;

varying vec4 vPickColor;

void main() {
  vec3 position = mix(aPosition, aTargetPosition, clamp(uMorph, 0.0, 1.0));
  float height = mix(aSourceHeight, aTargetHeight, clamp(uMorph, 0.0, 1.0));
  float heightScale = 1.0 + clamp(height * uHeightShadowScale, 0.0, 1.8);
  gl_Position = uViewProjectionMatrix * vec4(position, 1.0);
  gl_PointSize = clamp(max(aSize, 0.001) * uPointPixelScale * heightScale, uMinPointSize, uMaxPointSize);
  vPickColor = aPickColor;
}
`;

const PROJECTION_PICK_FRAGMENT_SHADER = `
precision mediump float;

varying vec4 vPickColor;

void main() {
  vec2 p = gl_PointCoord * 2.0 - 1.0;
  if (dot(p, p) > 1.0) discard;
  gl_FragColor = vPickColor;
}
`;

const FOOTPRINT_VERTEX_SHADER = `
attribute vec3 aPosition;
attribute vec4 aColor;

uniform mat4 uViewProjectionMatrix;

varying vec4 vColor;

void main() {
  gl_Position = uViewProjectionMatrix * vec4(aPosition, 1.0);
  vColor = aColor;
}
`;

const FOOTPRINT_FRAGMENT_SHADER = `
precision mediump float;

uniform float uGlobalAlpha;
varying vec4 vColor;

void main() {
  gl_FragColor = vec4(vColor.rgb, vColor.a * uGlobalAlpha);
}
`;
