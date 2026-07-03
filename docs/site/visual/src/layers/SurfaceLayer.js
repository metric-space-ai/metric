import { BaseLayer } from "./BaseLayer.js";
import {
  channelToFloat32,
  colorChannel,
  getChannel,
  getChannelArray,
  getChannelCount,
  inferInstanceCount,
  positionChannel,
} from "./channels.js";
import { computeVertexNormals, indexTypeFor, normalizeIndices } from "./geometry.js";
import {
  bindAttribute,
  configureDrawState,
  createBuffer,
  createIndexBuffer,
  createProgram,
  frameTimeSeconds,
  restoreDepthWrite,
  setCameraUniforms,
  trackBuffer,
} from "./gl-utils.js";

export class SurfaceLayer extends BaseLayer {
  constructor(descriptor = {}, rendererOrGl = null, options = {}) {
    super(descriptor, rendererOrGl, options);
    this.vertexCount = 0;
    this.indexCount = 0;
    this.indexType = null;
    this.buffers = {};
    this.program = null;
  }

  ensureResources() {
    if (!this.gl) return false;
    if (!this.program) {
      this.program = this.track(createProgram(this.gl, "SurfaceLayer", SURFACE_VERTEX_SHADER, SURFACE_FRAGMENT_SHADER));
    }
    if (this.needsUpload) this.upload();
    return true;
  }

  upload() {
    const gl = this.gl;
    this.vertexCount = inferInstanceCount(this.channels, ["position", "color", "scalar", "alpha"]);
    const positions = positionChannel(this.channels, "position", this.vertexCount);
    const indices = normalizeIndices(gl, this.geometry.indices, this.vertexCount, this.capabilities);
    const normals = this.resolveNormals(positions, indices);
    const colors = colorChannel(this.channels, this.vertexCount, { defaultColor: [0.30, 0.50, 0.58, 1] });

    this.replaceArrayBuffer("position", positions);
    this.replaceArrayBuffer("normal", normals);
    this.replaceArrayBuffer("color", colors);

    if (indices) {
      this.replaceIndexBuffer(indices);
      this.indexCount = indices.length;
      this.indexType = indexTypeFor(gl, indices);
    } else {
      this.deleteBuffer("index");
      this.indexCount = 0;
      this.indexType = null;
    }
    this.needsUpload = false;
  }

  resolveNormals(positions, indices) {
    const normalChannel = getChannel(this.channels, ["normal", "normals"]);
    if (normalChannel) {
      return channelToFloat32(normalChannel, 3, this.vertexCount, [0, 1, 0]);
    }
    if (this.geometry.normalPolicy === "renderer-compute") {
      return computeVertexNormals(positions, indices);
    }
    const normals = new Float32Array(this.vertexCount * 3);
    for (let index = 0; index < this.vertexCount; index += 1) {
      normals[index * 3 + 1] = 1;
    }
    return normals;
  }

  render(context = {}) {
    if (this.disposed || this.visible === false || !this.ensureResources() || this.vertexCount <= 0) return this;
    const gl = this.gl;
    const material = this.material || {};

    configureDrawState(gl, material, {
      blend: material.alphaMode === "blend",
      cullFace: material.backFace !== "double-sided",
      depthWrite: material.alphaMode !== "blend",
    });

    this.program.use();
    setCameraUniforms(this.program, context, this.renderer);
    this.program.setUniform("uAmbient", numberOption(material.ambient, 0.48));
    this.program.setUniform("uPointLight", numberOption(material.pointLight, 0.42));
    this.program.setUniform("uSpecular", numberOption(material.specular, 0.08));
    this.program.setUniform("uSpecularPower", numberOption(material.specularPower, material.shininess, 54));
    this.program.setUniform("uMaterialRoughness", numberOption(material.roughness, 0.34));
    this.program.setUniform("uMaterialSaturation", numberOption(material.saturation, 0.98));
    this.program.setUniform("uShadowSoftness", numberOption(material.shadowSoftness, 0.24));
    this.program.setUniform("uBaseLift", numberOption(material.baseLift, 0.025));
    this.program.setUniform("uRimLight", numberOption(material.rimLight, 0.08));
    this.program.setUniform("uGlobalAlpha", numberOption(material.alpha, 1));
    this.program.setUniform("uAmbientColor", color3(material.ambientColor, [1, 1, 1]));
    this.program.setUniform("uLightColor", color3(material.lightColor, [1, 1, 1]));
    this.program.setUniform("uShadowTint", color3(material.shadowTint, [0.74, 0.8, 0.82]));
    this.program.setUniform("uHighlightColor", color3(material.highlightColor, color3(material.lightColor, [1, 1, 1])));
    this.program.setUniform("uLightDirection", direction3(material.lightDirection, [0.32, 0.82, 0.46]));
    const lights = lightArrays(material);
    this.program.setUniform("uLightDirections", lights.directions);
    this.program.setUniform("uLightColors", lights.colors);
    this.program.setUniform("uLightIntensities", lights.intensities);
    this.program.setUniform("uTime", frameTimeSeconds(context));
    this.program.setUniform("uDisplacementScale", numberOption(material.displacementScale, this.geometry?.texture?.styleNoise?.displacementScale, 0));
    this.program.setUniform("uNoiseCoef", numberOption(material.noiseCoef, this.geometry?.texture?.styleNoise?.noiseCoef, 1.8));
    this.program.setUniform("uNoiseSpeed", numberOption(material.timeCoef, this.geometry?.texture?.styleNoise?.timeCoef, 0.18));

    bindAttribute(gl, this.program, this.capabilities, "aPosition", this.buffers.position, 3);
    bindAttribute(gl, this.program, this.capabilities, "aNormal", this.buffers.normal, 3);
    bindAttribute(gl, this.program, this.capabilities, "aColor", this.buffers.color, 4);

    if (this.buffers.index && this.indexCount > 0) {
      gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.buffers.index);
      gl.drawElements(gl.TRIANGLES, this.indexCount, this.indexType, 0);
    } else {
      gl.drawArrays(gl.TRIANGLES, 0, this.vertexCount);
    }
    restoreDepthWrite(gl);
    return this;
  }

  getBounds() {
    return super.getBounds();
  }

  replaceArrayBuffer(name, data) {
    const gl = this.gl;
    this.deleteBuffer(name);
    this.buffers[name] = createBuffer(gl, data, gl.STATIC_DRAW);
    trackBuffer(this, this.buffers[name]);
  }

  replaceIndexBuffer(data) {
    const gl = this.gl;
    this.deleteBuffer("index");
    this.buffers.index = createIndexBuffer(gl, data, gl.STATIC_DRAW);
    trackBuffer(this, this.buffers.index);
  }

  deleteBuffer(name) {
    if (!this.buffers[name]) return;
    this.gl.deleteBuffer(this.buffers[name]);
    this.resources = this.resources.filter((resource) => resource.handle !== this.buffers[name]);
    delete this.buffers[name];
  }

  dispose() {
    this.buffers = {};
    this.program = null;
    super.dispose();
  }
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
  return fallback;
}

function direction3(value, fallback) {
  const out = vec3(value, fallback);
  const length = Math.hypot(out[0], out[1], out[2]) || 1;
  return [out[0] / length, out[1] / length, out[2] / length];
}

function vec3(value, fallback) {
  if (Array.isArray(value) || ArrayBuffer.isView(value)) {
    return [
      numberOption(value[0], fallback[0]),
      numberOption(value[1], fallback[1]),
      numberOption(value[2], fallback[2]),
    ];
  }
  if (value && typeof value === "object") {
    return [
      numberOption(value.x, fallback[0]),
      numberOption(value.y, fallback[1]),
      numberOption(value.z, fallback[2]),
    ];
  }
  return fallback.slice();
}

function lightArrays(material = {}) {
  const fallbackDirection = direction3(material.lightDirection, [0.32, 0.82, 0.46]);
  const fallbackColor = color3(material.lightColor, [1, 1, 1]);
  const fallbackIntensity = numberOption(material.lightIntensity, 1);
  const directions = new Float32Array(12);
  const colors = new Float32Array(12);
  const intensities = new Float32Array(4);
  const directionSource = material.pointLightDirections || material.lightDirections || null;
  const colorSource = material.pointLightColors || material.lightColors || null;
  const intensitySource = material.pointLightIntensities || material.lightIntensities || null;

  for (let index = 0; index < 4; index += 1) {
    const direction = readVec3(directionSource, index, fallbackDirection);
    const color = readVec3(colorSource, index, fallbackColor);
    const intensity = readScalar(intensitySource, index, index === 0 ? fallbackIntensity : 0);
    directions.set(direction3(direction, fallbackDirection), index * 3);
    colors.set(color3(color, fallbackColor), index * 3);
    intensities[index] = Math.max(0, intensity);
  }

  return { directions, colors, intensities };
}

function readVec3(source, index, fallback) {
  if (!source) return fallback;
  if (Array.isArray(source[index]) || ArrayBuffer.isView(source[index])) {
    return [
      numberOption(source[index][0], fallback[0]),
      numberOption(source[index][1], fallback[1]),
      numberOption(source[index][2], fallback[2]),
    ];
  }
  const offset = index * 3;
  if (Number.isFinite(Number(source[offset]))) {
    return [
      numberOption(source[offset], fallback[0]),
      numberOption(source[offset + 1], fallback[1]),
      numberOption(source[offset + 2], fallback[2]),
    ];
  }
  return fallback;
}

function readScalar(source, index, fallback) {
  if (!source) return fallback;
  return numberOption(source[index], fallback);
}

function clamp01(value) {
  return Math.max(0, Math.min(1, Number.isFinite(value) ? value : 0));
}

const SURFACE_VERTEX_SHADER = `
attribute vec3 aPosition;
attribute vec3 aNormal;
attribute vec4 aColor;

uniform mat4 uViewProjectionMatrix;
uniform float uTime;
uniform float uDisplacementScale;
uniform float uNoiseCoef;
uniform float uNoiseSpeed;

varying vec3 vNormal;
varying vec3 vWorldPosition;
varying vec4 vColor;

float hash21(vec2 p) {
  p = fract(p * vec2(123.34, 456.21));
  p += dot(p, p + 45.32);
  return fract(p.x * p.y);
}

float valueNoise(vec2 p) {
  vec2 i = floor(p);
  vec2 f = fract(p);
  vec2 u = f * f * (3.0 - 2.0 * f);
  float a = hash21(i);
  float b = hash21(i + vec2(1.0, 0.0));
  float c = hash21(i + vec2(0.0, 1.0));
  float d = hash21(i + vec2(1.0, 1.0));
  return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

void main() {
  vec3 normal = normalize(aNormal);
  vec3 position = aPosition;
  if (uDisplacementScale > 0.0) {
    float n = valueNoise(position.xz * uNoiseCoef + vec2(uTime * uNoiseSpeed, -uTime * uNoiseSpeed * 0.57));
    position += normal * ((n - 0.5) * uDisplacementScale);
  }
  vNormal = normal;
  vWorldPosition = position;
  vColor = aColor;
  gl_Position = uViewProjectionMatrix * vec4(position, 1.0);
}
`;

const SURFACE_FRAGMENT_SHADER = `
precision mediump float;

uniform vec3 uCameraPosition;
uniform float uAmbient;
uniform float uPointLight;
uniform float uSpecular;
uniform float uSpecularPower;
uniform float uMaterialRoughness;
uniform float uMaterialSaturation;
uniform float uShadowSoftness;
uniform float uBaseLift;
uniform float uRimLight;
uniform float uGlobalAlpha;
uniform vec3 uAmbientColor;
uniform vec3 uLightColor;
uniform vec3 uShadowTint;
uniform vec3 uHighlightColor;
uniform vec3 uLightDirection;
uniform vec3 uLightDirections[4];
uniform vec3 uLightColors[4];
uniform float uLightIntensities[4];

varying vec3 vNormal;
varying vec3 vWorldPosition;
varying vec4 vColor;

float luma(vec3 color) {
  return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

vec3 adjustSaturation(vec3 color, float amount) {
  return mix(vec3(luma(color)), color, clamp(amount, 0.0, 2.0));
}

void main() {
  vec3 normal = normalize(vNormal);
  vec3 viewDir = normalize(uCameraPosition - vWorldPosition);
  float roughness = clamp(uMaterialRoughness, 0.02, 1.0);
  float specularPower = clamp(uSpecularPower, 2.0, 160.0) * mix(1.18, 0.32, roughness);
  vec3 baseColor = adjustSaturation(vColor.rgb, uMaterialSaturation);
  vec3 color = baseColor * uAmbientColor * uAmbient;
  float strongestDiffuse = 0.0;
  for (int index = 0; index < 4; index++) {
    vec3 lightDir = normalize(uLightDirections[index]);
    vec3 lightColor = uLightColors[index];
    float lightIntensity = uLightIntensities[index] * uPointLight;
    vec3 halfDir = normalize(lightDir + viewDir);
    float diffuse = max(dot(normal, lightDir), 0.0);
    float specular = pow(max(dot(normal, halfDir), 0.0), specularPower) * uSpecular * mix(1.1, 0.42, roughness) * lightIntensity;
    color += baseColor * lightColor * diffuse * lightIntensity + uHighlightColor * specular;
    strongestDiffuse = max(strongestDiffuse, diffuse * lightIntensity);
  }
  float rim = pow(max(1.0 - dot(viewDir, normal), 0.0), 2.4) * clamp(uRimLight, 0.0, 1.0);
  float shadow = clamp((1.0 - strongestDiffuse) * clamp(uShadowSoftness, 0.0, 1.0), 0.0, 1.0);
  color = mix(color, color * uShadowTint, shadow);
  color += baseColor * clamp(uBaseLift, 0.0, 0.4);
  color += uHighlightColor * rim * 0.42;
  gl_FragColor = vec4(pow(max(color, vec3(0.0)), vec3(1.0 / 2.2)), vColor.a * uGlobalAlpha);
}
`;
