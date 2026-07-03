import { BaseLayer } from "./BaseLayer.js";
import {
  colorChannel,
  combineScalarChannels,
  getChannel,
  getChannelArray,
  inferInstanceCount,
  numericDomain,
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
    this.pickProgram = null;
    this.pointSizeLimits = [1, 64];
    this.recordIds = [];
    this.baseFocus = new Float32Array();
    this.selectedRecordId = null;
    this.pickColorRegistry = null;
    this.pickColorRegistrySize = -1;
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
    const scalarSource = getChannel(this.channels, ["scalar", "value"]);
    const rawScalar = scalarChannel(this.channels, ["scalar", "value"], this.count, 0);
    const scalar = normalizeScalarChannel(rawScalar, scalarSource);
    const radiusChannel = getChannel(this.channels, "radius") || getChannel(this.channels, "size");
    const radius = radiusChannel
      ? scalarChannel(this.channels, radiusChannel === getChannel(this.channels, "radius") ? "radius" : "size", this.count, defaultRadius(this.geometry))
      : new Float32Array(this.count).fill(defaultRadius(this.geometry));
    this.baseFocus = combineScalarChannels(this.channels, ["focusWeight", "focus", "selection"], this.count, 0);
    this.recordIds = recordIdsForChannels(this.channels, this.count);
    this.pickColorRegistry = null;
    this.pickColorRegistrySize = -1;
    const focus = this.selectionFocusData();

    this.replaceBuffer("position", positions);
    this.replaceBuffer("color", colors);
    this.replaceBuffer("scalar", scalar);
    this.replaceBuffer("radius", radius);
    this.replaceBuffer("focus", focus);
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
    this.program.setUniform("uRampMode", rampMode(material.ramp || geometry.ramp || this.metadata?.propertySemantic));
    this.program.setUniform("uSelectionColor", color3(material.selectionColor, [1.0, 0.76, 0.22]));

    bindAttribute(gl, this.program, this.capabilities, "aPosition", this.buffers.position, 3);
    bindAttribute(gl, this.program, this.capabilities, "aColor", this.buffers.color, 4);
    bindAttribute(gl, this.program, this.capabilities, "aScalar", this.buffers.scalar, 1);
    bindAttribute(gl, this.program, this.capabilities, "aRadius", this.buffers.radius, 1);
    bindAttribute(gl, this.program, this.capabilities, "aFocus", this.buffers.focus, 1);
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

    if (!this.pickProgram) {
      this.pickProgram = this.track(createProgram(gl, "HeatFieldLayerPicking", HEAT_PICK_VERTEX_SHADER, HEAT_PICK_FRAGMENT_SHADER));
    }
    this.updatePickColorBuffer(context);

    configureDrawState(gl, { depthWrite: true, alphaMode: "opaque" }, {
      blend: false,
      cullFace: false,
      depthWrite: true,
    });

    this.pickProgram.use();
    setCameraUniforms(this.pickProgram, context, this.renderer);
    this.pickProgram.setUniform("uPointPixelScale", numberOption(material.pointPixelScale, geometry.pointPixelScale, this.options.pointPixelScale, 10));
    this.pickProgram.setUniform("uMinPointSize", numberOption(material.minPointSize, geometry.minPointSize, 1));
    this.pickProgram.setUniform("uMaxPointSize", numberOption(material.maxPointSize, geometry.maxPointSize, this.pointSizeLimits[1] || 128));
    this.pickProgram.setUniform("uRadial", geometry.mode === "grid" || material.shape === "cell" ? 0 : 1);

    bindAttribute(gl, this.pickProgram, this.capabilities, "aPosition", this.buffers.position, 3);
    bindAttribute(gl, this.pickProgram, this.capabilities, "aRadius", this.buffers.radius, 1);
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

  getGrammarDiagnostics() {
    const grammar = this.metadata?.propertyFieldGrammar || {};
    const diagnostics = grammar.diagnostics || this.metadata?.fieldDiagnostics || {};
    return {
      ...diagnostics,
      primitive: "HeatFieldLayer",
      layerId: this.id,
      recordCount: diagnostics.recordCount ?? this.channels?.recordId?.count ?? this.count,
      scalarChannel: "scalar",
      positionChannel: "position",
      recordIdChannel: "recordId",
      pickingMode: this.picking?.mode || "none",
      rampMode: heatFieldRampMode(this.material?.ramp || this.geometry?.ramp || this.metadata?.propertySemantic),
      propertySemantic: this.metadata?.propertySemantic,
      selectionLinked: this.picking?.mode === "record-id" && Boolean(this.channels?.selection),
    };
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

function normalizeScalarChannel(values, channel) {
  const domain = numericDomain(channel?.domain, values);
  const min = domain[0];
  const max = domain[1];
  const span = Math.max(0.000001, max - min);
  const out = new Float32Array(values.length);
  for (let index = 0; index < values.length; index += 1) {
    out[index] = Math.max(0, Math.min(1, (values[index] - min) / span));
  }
  return out;
}

export function heatFieldRampMode(value) {
  return rampMode(value);
}

function recordIdsForChannels(channels, count) {
  const recordChannel = getChannel(channels, ["recordId", "record_id", "id", "cellId"]);
  const ids = getChannelArray(recordChannel);
  if (!ids) return Array.from({ length: count }, (_, index) => String(index));
  return Array.from({ length: count }, (_, index) => String(ids[index] ?? index));
}

function encodePickIdRGBAFloatLocal(id, out = [0, 0, 0, 0], offset = 0) {
  const numericId = Math.max(0, Math.floor(Number(id) || 0));
  out[offset] = (numericId % 256) / 255;
  out[offset + 1] = (Math.floor(numericId / 256) % 256) / 255;
  out[offset + 2] = (Math.floor(numericId / 65536) % 256) / 255;
  out[offset + 3] = (Math.floor(numericId / 16777216) % 256) / 255;
  return out;
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
  return fallback;
}

function rampMode(value) {
  const text = String(value || "").toLowerCase();
  if (/density/.test(text)) return 1;
  if (/outlier/.test(text)) return 2;
  if (/anomaly|fault|noise/.test(text)) return 3;
  if (/entropy|uncertainty/.test(text)) return 4;
  if (/residual|error/.test(text)) return 5;
  return 0;
}

function clamp01(value) {
  return Math.max(0, Math.min(1, Number.isFinite(value) ? value : 0));
}

const HEAT_VERTEX_SHADER = `
attribute vec3 aPosition;
attribute vec4 aColor;
attribute float aScalar;
attribute float aRadius;
attribute float aFocus;

uniform mat4 uViewProjectionMatrix;
uniform float uPointPixelScale;
uniform float uMinPointSize;
uniform float uMaxPointSize;

varying vec4 vColor;
varying float vScalar;
varying float vFocus;

void main() {
  gl_Position = uViewProjectionMatrix * vec4(aPosition, 1.0);
  float focus = clamp(aFocus, 0.0, 1.0);
  gl_PointSize = clamp(max(aRadius, 0.001) * uPointPixelScale * (1.0 + focus * 0.25), uMinPointSize, uMaxPointSize);
  vColor = aColor;
  vScalar = aScalar;
  vFocus = focus;
}
`;

const HEAT_PICK_VERTEX_SHADER = `
attribute vec3 aPosition;
attribute float aRadius;
attribute vec4 aPickColor;

uniform mat4 uViewProjectionMatrix;
uniform float uPointPixelScale;
uniform float uMinPointSize;
uniform float uMaxPointSize;

varying vec4 vPickColor;

void main() {
  gl_Position = uViewProjectionMatrix * vec4(aPosition, 1.0);
  gl_PointSize = clamp(max(aRadius, 0.001) * uPointPixelScale, uMinPointSize, uMaxPointSize);
  vPickColor = aPickColor;
}
`;

const HEAT_PICK_FRAGMENT_SHADER = `
precision mediump float;

uniform float uRadial;
varying vec4 vPickColor;

void main() {
  if (uRadial > 0.5) {
    vec2 centered = gl_PointCoord * 2.0 - 1.0;
    if (dot(centered, centered) > 1.0) discard;
  }
  gl_FragColor = vPickColor;
}
`;

const HEAT_FRAGMENT_SHADER = `
precision mediump float;

uniform float uGlobalAlpha;
uniform float uRadial;
uniform float uTime;
uniform float uContour;
uniform float uGlow;
uniform float uRampMode;
uniform vec3 uSelectionColor;

varying vec4 vColor;
varying float vScalar;
varying float vFocus;

vec3 scalarRamp(float scalar, float mode) {
  float t = clamp(scalar, 0.0, 1.0);
  if (mode < 0.5) {
    return vColor.rgb;
  }
  if (mode < 1.5) {
    vec3 low = vec3(0.13, 0.24, 0.34);
    vec3 mid = vec3(0.12, 0.55, 0.50);
    vec3 high = vec3(0.78, 0.88, 0.62);
    return t < 0.55 ? mix(low, mid, t / 0.55) : mix(mid, high, (t - 0.55) / 0.45);
  }
  if (mode < 2.5) {
    vec3 low = vec3(0.18, 0.20, 0.24);
    vec3 mid = vec3(0.84, 0.46, 0.20);
    vec3 high = vec3(0.88, 0.13, 0.18);
    return t < 0.62 ? mix(low, mid, t / 0.62) : mix(mid, high, (t - 0.62) / 0.38);
  }
  if (mode < 3.5) {
    vec3 low = vec3(0.10, 0.25, 0.34);
    vec3 mid = vec3(0.90, 0.58, 0.23);
    vec3 high = vec3(0.78, 0.10, 0.16);
    return t < 0.58 ? mix(low, mid, t / 0.58) : mix(mid, high, (t - 0.58) / 0.42);
  }
  if (mode < 4.5) {
    vec3 low = vec3(0.18, 0.24, 0.44);
    vec3 mid = vec3(0.47, 0.42, 0.68);
    vec3 high = vec3(0.92, 0.72, 0.30);
    return t < 0.52 ? mix(low, mid, t / 0.52) : mix(mid, high, (t - 0.52) / 0.48);
  }
  vec3 low = vec3(0.12, 0.26, 0.42);
  vec3 mid = vec3(0.78, 0.80, 0.72);
  vec3 high = vec3(0.80, 0.24, 0.20);
  return t < 0.5 ? mix(low, mid, t * 2.0) : mix(mid, high, (t - 0.5) * 2.0);
}

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
  vec3 color = min(scalarRamp(scalar, uRampMode) + scalarLift + vec3(uGlow * scalar * (1.0 - radius)), vec3(1.0));
  color = mix(color, uSelectionColor, clamp(vFocus * 0.42, 0.0, 1.0));
  float fieldAlpha = alpha * (0.82 + scalar * 0.18 + contour * uContour * 0.08);
  fieldAlpha = min(1.0, fieldAlpha + vFocus * 0.24);
  gl_FragColor = vec4(color, vColor.a * fieldAlpha * uGlobalAlpha);
}
`;
