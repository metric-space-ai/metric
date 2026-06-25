import { BaseLayer } from "./BaseLayer.js";
import {
  channelToFloat32,
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

export class InstancedGlyphLayer extends BaseLayer {
  constructor(descriptor = {}, rendererOrGl = null, options = {}) {
    super(descriptor, rendererOrGl, options);
    this.count = 0;
    this.buffers = {};
    this.program = null;
    this.pickProgram = null;
    this.pointSizeLimits = [1, 64];
    this.recordIds = [];
    this.baseFocus = new Float32Array();
    this.selectedRecordId = null;
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
      this.program = this.track(createProgram(this.gl, "InstancedGlyphLayer", GLYPH_VERTEX_SHADER, GLYPH_FRAGMENT_SHADER));
      this.pointSizeLimits = pointSizeRange(this.gl);
      this.initialized = true;
    }
    if (this.needsUpload) this.upload();
    return true;
  }

  upload() {
    const gl = this.gl;
    this.count = inferInstanceCount(this.channels, ["position", "targetPosition", "glyphType", "color", "size", "recordId"]);
    const positions = positionChannel(this.channels, "position", this.count);
    const target = getChannel(this.channels, "targetPosition")
      ? positionChannel(this.channels, "targetPosition", this.count)
      : positions;
    const colors = colorChannel(this.channels, this.count, { defaultColor: this.options.defaultColor || [0.26, 0.45, 0.68, 1] });
    const sizes = scalarChannel(this.channels, ["size", "radius"], this.count, 1);
    const alpha = scalarChannel(this.channels, "alpha", this.count, 1);
    const glyphType = scalarChannel(this.channels, ["glyphType", "recordGlyphType"], this.count, 0);
    const glyphVariant = scalarChannel(this.channels, ["glyphVariant", "variant", "category"], this.count, 0);
    const glyphFeature = channelToFloat32(getChannel(this.channels, ["glyphFeature", "payloadFeature"]), 4, this.count, [0, 0, 0, 0]);
    const glyphGeometry = channelToFloat32(getChannel(this.channels, ["glyphGeometry", "glyphRenderGeometry"]), 4, this.count, [0, 1, 0.48, 0.08]);
    const glyphMaterial = channelToFloat32(getChannel(this.channels, ["glyphMaterial", "glyphRenderMaterial"]), 4, this.count, [0, 0.45, 0.62, 0.82]);
    const payloadComplexity = scalarChannel(this.channels, ["payloadComplexity", "complexity"], this.count, 0.35);
    this.baseFocus = combineScalarChannels(this.channels, ["focusWeight", "focus", "selection"], this.count, 0);
    this.recordIds = recordIdsForChannels(this.channels, this.count);
    this.pickColorRegistry = null;
    this.pickColorRegistrySize = -1;
    const focus = this.selectionFocusData();

    this.replaceBuffer("position", positions);
    this.replaceBuffer("targetPosition", target);
    this.replaceBuffer("color", colors);
    this.replaceBuffer("size", sizes);
    this.replaceBuffer("alpha", alpha);
    this.replaceBuffer("focus", focus);
    this.replaceBuffer("glyphType", glyphType);
    this.replaceBuffer("glyphVariant", glyphVariant);
    this.replaceBuffer("glyphFeature", glyphFeature);
    this.replaceBuffer("glyphGeometry", glyphGeometry);
    this.replaceBuffer("glyphMaterial", glyphMaterial);
    this.replaceBuffer("payloadComplexity", payloadComplexity);
    this.needsUpload = false;
  }

  render(context = {}) {
    if (this.disposed || this.visible === false || !this.ensureResources() || this.count <= 0) return this;
    const gl = this.gl;
    const material = this.material || {};
    const geometry = this.geometry || {};
    const animation = this.animation || {};
    const pointScale = numberOption(material.pointPixelScale, geometry.pointPixelScale, this.options.pointPixelScale, 16);
    const minSize = numberOption(material.minPointSize, geometry.minPointSize, this.options.minPointSize, this.pointSizeLimits[0] || 1);
    const maxSize = numberOption(material.maxPointSize, geometry.maxPointSize, this.options.maxPointSize, this.pointSizeLimits[1] || 96);

    configureDrawState(gl, material, {
      blend: true,
      cullFace: false,
      depthWrite: material.depthWrite === true,
    });

    this.program.use();
    setCameraUniforms(this.program, context, this.renderer);
    this.program.setUniform("uTime", frameTimeSeconds(context));
    this.program.setUniform("uMorph", animationProgress(animation, context));
    this.program.setUniform("uPointPixelScale", pointScale);
    this.program.setUniform("uMinPointSize", minSize);
    this.program.setUniform("uMaxPointSize", maxSize);
    this.program.setUniform("uGlobalAlpha", numberOption(material.alpha, geometry.alpha, 1));
    this.program.setUniform("uFocusBoost", numberOption(material.focusBoost, 0.38));
    this.program.setUniform("uGlyphStroke", numberOption(material.glyphStroke, 0.68));
    this.program.setUniform("uGlyphInk", color3(material.glyphInk || material.inkColor, [0.08, 0.12, 0.14]));
    this.program.setUniform("uFocusColor", color3(material.focusColor, [1.0, 0.82, 0.28]));

    bindAttribute(gl, this.program, this.capabilities, "aPosition", this.buffers.position, 3);
    bindAttribute(gl, this.program, this.capabilities, "aTargetPosition", this.buffers.targetPosition, 3);
    bindAttribute(gl, this.program, this.capabilities, "aColor", this.buffers.color, 4);
    bindAttribute(gl, this.program, this.capabilities, "aSize", this.buffers.size, 1);
    bindAttribute(gl, this.program, this.capabilities, "aAlpha", this.buffers.alpha, 1);
    bindAttribute(gl, this.program, this.capabilities, "aFocus", this.buffers.focus, 1);
    bindAttribute(gl, this.program, this.capabilities, "aGlyphType", this.buffers.glyphType, 1);
    bindAttribute(gl, this.program, this.capabilities, "aGlyphVariant", this.buffers.glyphVariant, 1);
    bindAttribute(gl, this.program, this.capabilities, "aGlyphFeature", this.buffers.glyphFeature, 4);
    bindAttribute(gl, this.program, this.capabilities, "aGlyphGeometry", this.buffers.glyphGeometry, 4);
    bindAttribute(gl, this.program, this.capabilities, "aGlyphMaterial", this.buffers.glyphMaterial, 4);
    bindAttribute(gl, this.program, this.capabilities, "aPayloadComplexity", this.buffers.payloadComplexity, 1);

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
    const pointScale = numberOption(material.pointPixelScale, geometry.pointPixelScale, this.options.pointPixelScale, 16);
    const minSize = numberOption(material.minPointSize, geometry.minPointSize, this.options.minPointSize, this.pointSizeLimits[0] || 1);
    const maxSize = numberOption(material.maxPointSize, geometry.maxPointSize, this.options.maxPointSize, this.pointSizeLimits[1] || 96);

    if (!this.pickProgram) {
      this.pickProgram = this.track(createProgram(gl, "InstancedGlyphLayerPicking", GLYPH_PICK_VERTEX_SHADER, GLYPH_PICK_FRAGMENT_SHADER));
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
    bindAttribute(gl, this.pickProgram, this.capabilities, "aGlyphGeometry", this.buffers.glyphGeometry, 4);
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

  setSelection(selection = {}) {
    const recordId = selection?.recordId ?? selection?.record_id ?? null;
    const next = recordId == null ? null : String(recordId);
    if (this.selectedRecordId === next) return this;
    this.selectedRecordId = next;
    this.updateFocusBuffer();
    return this;
  }

  selectRecord(recordId) {
    return this.setSelection({ recordId });
  }

  clearSelection() {
    return this.setSelection({ recordId: null });
  }

  selectionFocusData() {
    const focus = new Float32Array(this.baseFocus.length || this.count);
    if (this.baseFocus.length) focus.set(this.baseFocus);
    if (this.selectedRecordId == null || !this.recordIds.length) return focus;
    for (let index = 0; index < Math.min(this.recordIds.length, focus.length); index += 1) {
      if (String(this.recordIds[index]) === this.selectedRecordId) focus[index] = Math.max(focus[index], 1);
    }
    return focus;
  }

  updateFocusBuffer() {
    if (!this.gl || !this.buffers.focus || this.needsUpload) {
      this.needsUpload = true;
      return;
    }
    this.replaceBuffer("focus", this.selectionFocusData());
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
  const recordChannel = getChannel(channels, ["recordId", "record_id", "id"]);
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

function numberOption(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number)) return number;
  }
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

const GLYPH_VERTEX_SHADER = `
attribute vec3 aPosition;
attribute vec3 aTargetPosition;
attribute vec4 aColor;
attribute float aSize;
attribute float aAlpha;
attribute float aFocus;
attribute float aGlyphType;
attribute float aGlyphVariant;
attribute vec4 aGlyphFeature;
attribute vec4 aGlyphGeometry;
attribute vec4 aGlyphMaterial;
attribute float aPayloadComplexity;

uniform mat4 uViewProjectionMatrix;
uniform float uTime;
uniform float uMorph;
uniform float uPointPixelScale;
uniform float uMinPointSize;
uniform float uMaxPointSize;
uniform float uFocusBoost;

varying vec4 vColor;
varying float vAlpha;
varying float vFocus;
varying float vGlyphType;
varying float vGlyphVariant;
varying vec4 vGlyphFeature;
varying vec4 vGlyphGeometry;
varying vec4 vGlyphMaterial;
varying float vPayloadComplexity;

void main() {
  vec3 position = mix(aPosition, aTargetPosition, clamp(uMorph, 0.0, 1.0));
  float focus = clamp(aFocus, 0.0, 1.0);
  gl_Position = uViewProjectionMatrix * vec4(position, 1.0);
  gl_PointSize = clamp(max(aSize, 0.001) * uPointPixelScale * (1.0 + focus * uFocusBoost), uMinPointSize, uMaxPointSize);
  vColor = aColor;
  vAlpha = aAlpha;
  vFocus = focus;
  vGlyphType = aGlyphType;
  vGlyphVariant = aGlyphVariant;
  vGlyphFeature = aGlyphFeature;
  vGlyphGeometry = aGlyphGeometry;
  vGlyphMaterial = aGlyphMaterial;
  vPayloadComplexity = aPayloadComplexity;
}
`;

const GLYPH_FRAGMENT_SHADER = `
precision mediump float;

uniform float uGlobalAlpha;
uniform float uTime;
uniform float uGlyphStroke;
uniform vec3 uGlyphInk;
uniform vec3 uFocusColor;

varying vec4 vColor;
varying float vAlpha;
varying float vFocus;
varying float vGlyphType;
varying float vGlyphVariant;
varying vec4 vGlyphFeature;
varying vec4 vGlyphGeometry;
varying vec4 vGlyphMaterial;
varying float vPayloadComplexity;

float rectAlpha(vec2 p, vec2 halfSize, float feather) {
  vec2 d = abs(p) - halfSize;
  float outside = length(max(d, vec2(0.0)));
  float inside = min(max(d.x, d.y), 0.0);
  return 1.0 - smoothstep(0.0, feather, outside + inside * 0.15);
}

float roundedRectAlpha(vec2 p, vec2 halfSize, float radius, float feather) {
  vec2 d = abs(p) - halfSize + vec2(radius);
  float outside = length(max(d, vec2(0.0))) - radius;
  float inside = min(max(d.x, d.y), 0.0);
  return 1.0 - smoothstep(0.0, feather, outside + inside * 0.12);
}

float circleAlpha(vec2 p, float radius, float feather) {
  return 1.0 - smoothstep(radius - feather, radius, length(p));
}

float diamondAlpha(vec2 p, float radius, float feather) {
  float d = abs(p.x) + abs(p.y);
  return 1.0 - smoothstep(radius - feather, radius, d);
}

float lineMask(float distanceValue, float width) {
  return 1.0 - smoothstep(width, width + 0.035, abs(distanceValue));
}

float bandMask(vec2 p, vec2 center, vec2 halfSize) {
  float a = rectAlpha(p - center, halfSize, 0.035);
  return smoothstep(0.35, 0.9, a);
}

float featureAt(float slot) {
  if (slot < 0.5) return vGlyphFeature.x;
  if (slot < 1.5) return vGlyphFeature.y;
  if (slot < 2.5) return vGlyphFeature.z;
  return vGlyphFeature.w;
}

vec3 typedTint(float slot) {
  if (slot < 0.5) return vec3(0.18, 0.38, 0.70);
  if (slot < 1.5) return vec3(0.12, 0.58, 0.54);
  if (slot < 2.5) return vec3(0.90, 0.52, 0.20);
  return vec3(0.46, 0.34, 0.64);
}

void main() {
  vec2 p = gl_PointCoord * 2.0 - 1.0;
  float geometry = floor(vGlyphGeometry.x + 0.5);
  float material = floor(vGlyphMaterial.x + 0.5);
  float aspect = clamp(vGlyphGeometry.y, 0.55, 1.65);
  float corner = clamp(vGlyphGeometry.z, 0.0, 0.55);
  float relief = clamp(vGlyphGeometry.w, 0.0, 0.5);
  float lightingMix = clamp(vGlyphMaterial.y, 0.0, 1.0);
  float markContrast = clamp(vGlyphMaterial.z, 0.2, 1.35);
  float roughness = clamp(vGlyphMaterial.w, 0.0, 1.0);
  float alpha = 0.0;
  float mark = 0.0;
  vec3 markTint = uGlyphInk;
  vec3 base = vColor.rgb;

  if (geometry < 0.5) {
    alpha = circleAlpha(p, 0.95, 0.08);
    mark = circleAlpha(p, 0.38 + vPayloadComplexity * 0.22, 0.08) * 0.22;
  } else if (geometry < 1.5) {
    alpha = roundedRectAlpha(p, vec2(0.78, 0.54), corner, 0.075);
    float line1 = lineMask(p.y - 0.21, 0.035) * step(-0.52, p.x) * step(p.x, -0.52 + 0.88 * max(0.25, vGlyphFeature.x));
    float line2 = lineMask(p.y, 0.035) * step(-0.52, p.x) * step(p.x, -0.52 + 1.04 * max(0.2, vGlyphFeature.y));
    float line3 = lineMask(p.y + 0.21, 0.035) * step(-0.52, p.x) * step(p.x, -0.52 + 0.72 + vGlyphFeature.z * 0.22);
    mark = max(line1, max(line2, line3));
  } else if (geometry < 2.5) {
    alpha = circleAlpha(p, 0.96, 0.08);
    float wave = sin((p.x * 2.0 + vGlyphVariant * 1.7 + uTime * 0.08) * 3.14159265359) * (0.12 + vGlyphFeature.y * 0.26);
    wave += (vGlyphFeature.z - 0.5) * p.x * 0.28;
    mark = lineMask(p.y - wave, 0.045) * step(-0.72, p.x) * step(p.x, 0.72);
    markTint = vec3(0.02, 0.18, 0.20);
  } else if (geometry < 3.5) {
    alpha = roundedRectAlpha(p, vec2(0.69, 0.69), corner, 0.075);
    for (float slot = 0.0; slot < 4.0; slot += 1.0) {
      float x = -0.45 + slot * 0.30;
      float h = 0.16 + featureAt(slot) * 0.72;
      float bar = bandMask(p, vec2(x, -0.46 + h * 0.5), vec2(0.09, h * 0.5));
      mark = max(mark, bar);
    }
    markTint = vec3(0.18, 0.10, 0.04);
  } else if (geometry < 4.5) {
    alpha = roundedRectAlpha(p, vec2(0.76, 0.58), corner, 0.065);
    vec2 cell = floor((p + vec2(0.72, 0.52)) * vec2(3.2, 4.3));
    float checker = mod(cell.x + cell.y, 2.0);
    float scan = lineMask(fract((p.y + 0.66) * 5.0) - 0.5, 0.11);
    float frame = 1.0 - roundedRectAlpha(p, vec2(0.58, 0.42), corner * 0.55, 0.055);
    mark = max(checker * 0.28 + scan * (0.18 + vGlyphFeature.y * 0.32), frame * 0.55);
    markTint = vec3(0.08, 0.10, 0.12);
  } else if (geometry < 5.5) {
    alpha = diamondAlpha(p, 0.98, 0.08);
    float axisX = lineMask(p.y, 0.035) * step(abs(p.x), 0.68);
    float axisY = lineMask(p.x, 0.035) * step(abs(p.y), 0.68);
    float diag = lineMask(p.y - p.x * (0.35 + vGlyphFeature.y * 0.65), 0.035) * step(abs(p.x), 0.56);
    mark = max(max(axisX, axisY), diag);
    markTint = vec3(0.12, 0.06, 0.18);
  } else {
    alpha = roundedRectAlpha(p, vec2(0.77, 0.77), corner, 0.075);
    float panelA = bandMask(p, vec2(-0.32, 0.32), vec2(0.25, 0.25)) * max(0.22, vGlyphFeature.x);
    float panelB = bandMask(p, vec2(0.32, 0.32), vec2(0.25, 0.25)) * max(0.22, vGlyphFeature.y);
    float panelC = bandMask(p, vec2(-0.32, -0.32), vec2(0.25, 0.25)) * max(0.22, vGlyphFeature.z);
    float panelD = bandMask(p, vec2(0.32, -0.32), vec2(0.25, 0.25)) * max(0.22, vGlyphFeature.w);
    float panel = max(max(panelA, panelB), max(panelC, panelD));
    mark = panel;
    float slot = panelA >= max(max(panelB, panelC), panelD) ? 0.0 : (panelB >= max(panelC, panelD) ? 1.0 : (panelC >= panelD ? 2.0 : 3.0));
    markTint = typedTint(slot);
  }

  if (alpha <= 0.01) discard;
  float radial = clamp(length(p), 0.0, 1.0);
  float rim = 1.0 - circleAlpha(vec2(p.x / aspect, p.y), 0.82, 0.22);
  float surfaceLift = 1.0 + (1.0 - radial) * relief * (0.5 + lightingMix * 0.65);
  float edgeShade = mix(0.72, 0.48, lightingMix) * rim * (0.24 + relief);
  vec3 color = mix(base, markTint, clamp(mark * uGlyphStroke * markContrast, 0.0, 1.0));
  color *= mix(1.0, surfaceLift, 1.0 - roughness * 0.45);
  color = mix(color, color * vec3(0.58, 0.62, 0.67), clamp(edgeShade, 0.0, 0.62));
  color = mix(color, vec3(dot(color, vec3(0.299, 0.587, 0.114))), material == 4.0 ? 0.18 : 0.0);
  color = mix(color, uFocusColor, vFocus * 0.36);
  gl_FragColor = vec4(color, vColor.a * vAlpha * alpha * uGlobalAlpha);
}
`;

const GLYPH_PICK_VERTEX_SHADER = `
attribute vec3 aPosition;
attribute vec3 aTargetPosition;
attribute float aSize;
attribute vec4 aGlyphGeometry;
attribute vec4 aPickColor;

uniform mat4 uViewProjectionMatrix;
uniform float uMorph;
uniform float uPointPixelScale;
uniform float uMinPointSize;
uniform float uMaxPointSize;

varying vec4 vPickColor;
varying vec4 vGlyphGeometry;

void main() {
  vec3 position = mix(aPosition, aTargetPosition, clamp(uMorph, 0.0, 1.0));
  gl_Position = uViewProjectionMatrix * vec4(position, 1.0);
  gl_PointSize = clamp(max(aSize, 0.001) * uPointPixelScale, uMinPointSize, uMaxPointSize);
  vPickColor = aPickColor;
  vGlyphGeometry = aGlyphGeometry;
}
`;

const GLYPH_PICK_FRAGMENT_SHADER = `
precision mediump float;

varying vec4 vPickColor;
varying vec4 vGlyphGeometry;

float roundedRectAlpha(vec2 p, vec2 halfSize, float radius, float feather) {
  vec2 d = abs(p) - halfSize + vec2(radius);
  float outside = length(max(d, vec2(0.0))) - radius;
  float inside = min(max(d.x, d.y), 0.0);
  return 1.0 - smoothstep(0.0, feather, outside + inside * 0.12);
}

float circleAlpha(vec2 p, float radius, float feather) {
  return 1.0 - smoothstep(radius - feather, radius, length(p));
}

float diamondAlpha(vec2 p, float radius, float feather) {
  float d = abs(p.x) + abs(p.y);
  return 1.0 - smoothstep(radius - feather, radius, d);
}

float glyphPickAlpha(vec2 p) {
  float geometry = floor(vGlyphGeometry.x + 0.5);
  float corner = clamp(vGlyphGeometry.z, 0.0, 0.55);
  if (geometry < 0.5) return circleAlpha(p, 0.95, 0.08);
  if (geometry < 1.5) return roundedRectAlpha(p, vec2(0.78, 0.54), corner, 0.075);
  if (geometry < 2.5) return circleAlpha(p, 0.96, 0.08);
  if (geometry < 3.5) return roundedRectAlpha(p, vec2(0.69, 0.69), corner, 0.075);
  if (geometry < 4.5) return roundedRectAlpha(p, vec2(0.76, 0.58), corner, 0.065);
  if (geometry < 5.5) return diamondAlpha(p, 0.98, 0.08);
  return roundedRectAlpha(p, vec2(0.77, 0.77), corner, 0.075);
}

void main() {
  vec2 centered = gl_PointCoord * 2.0 - 1.0;
  if (glyphPickAlpha(centered) <= 0.01) discard;
  gl_FragColor = vPickColor;
}
`;
