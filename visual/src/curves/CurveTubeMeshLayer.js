import { BaseLayer } from "../layers/BaseLayer.js";
import {
  channelToFloat32,
  colorChannel,
  getChannel,
  getChannelCount,
  positionChannel,
  scalarChannel,
} from "../layers/channels.js";
import { indexTypeFor, normalizeIndices } from "../layers/geometry.js";
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
} from "../layers/gl-utils.js";
import { buildTubePathGeometry } from "./tessellation.js";

export class CurveTubeMeshLayer extends BaseLayer {
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
      this.program = this.track(createProgram(this.gl, "CurveTubeMeshLayer", TUBE_VERTEX_SHADER, TUBE_FRAGMENT_SHADER));
    }
    if (this.needsUpload) this.upload();
    return true;
  }

  upload() {
    const gl = this.gl;
    const buffers = this.resolveBuffers();
    this.vertexCount = buffers.vertexCount;

    this.replaceArrayBuffer("position", buffers.positions);
    this.replaceArrayBuffer("normal", buffers.normals);
    this.replaceArrayBuffer("color", buffers.colors);
    this.replaceArrayBuffer("distance", buffers.distances);
    this.replaceArrayBuffer("pathIndex", buffers.pathIndices);

    if (buffers.indices) {
      const indices = normalizeIndices(gl, buffers.indices, buffers.vertexCount, this.capabilities);
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

  resolveBuffers() {
    const positionCount = getChannelCount(getChannel(this.channels, "position"), 3);
    if (positionCount > 0) {
      return {
        vertexCount: positionCount,
        positions: positionChannel(this.channels, "position", positionCount),
        normals: channelToFloat32(getChannel(this.channels, ["normal", "normals"]), 3, positionCount, [0, 1, 0]),
        colors: colorChannel(this.channels, positionCount, { defaultColor: [0.18, 0.48, 0.66, 1] }),
        distances: scalarChannel(this.channels, "distance", positionCount, 0),
        pathIndices: scalarChannel(this.channels, "pathIndex", positionCount, 0),
        indices: this.geometry?.indices || this.source?.geometry?.indices || null,
      };
    }

    const evidence = this.source?.evidence || this.source?.geometry?.evidence || this.descriptor?.evidence;
    const geometry = buildTubePathGeometry(evidence, {
      ...this.geometry,
      ...this.material,
    });
    return {
      vertexCount: geometry.vertexCount,
      positions: geometry.positions,
      normals: geometry.normals,
      colors: geometry.colors,
      distances: geometry.distances,
      pathIndices: geometry.pathIndices,
      indices: geometry.indices,
    };
  }

  render(context = {}) {
    if (this.disposed || this.visible === false || !this.ensureResources() || this.vertexCount <= 0) return this;
    const gl = this.gl;
    const material = this.material || {};
    const alpha = numberOption(material.alpha, 1);

    configureDrawState(gl, material, {
      blend: material.alphaMode === "blend" || material.transparent === true || alpha < 0.999,
      cullFace: material.backFace !== "double-sided",
      depthWrite: material.depthWrite !== false,
    });

    this.program.use();
    setCameraUniforms(this.program, context, this.renderer);
    this.program.setUniform("uTime", frameTimeSeconds(context));
    this.program.setUniform("uAmbient", numberOption(material.ambient, 0.62));
    this.program.setUniform("uPointLight", numberOption(material.pointLight, 0.86));
    this.program.setUniform("uSpecular", numberOption(material.specular, 0.18));
    this.program.setUniform("uSpecularPower", numberOption(material.specularPower, material.shininess, 48));
    this.program.setUniform("uMaterialRoughness", numberOption(material.roughness, 0.42));
    this.program.setUniform("uMaterialSaturation", numberOption(material.saturation, 1));
    this.program.setUniform("uGlobalAlpha", alpha);
    this.program.setUniform("uEmission", numberOption(material.emission, 0.08));
    this.program.setUniform("uRimLight", numberOption(material.rimLight, 0.24));
    this.program.setUniform("uCoreGlow", numberOption(material.coreGlow, 0.14));
    this.program.setUniform("uFlowStrength", numberOption(material.flowStrength, 0.08));
    this.program.setUniform("uFlowScale", numberOption(material.flowScale, 2.4));
    this.program.setUniform("uFlowSpeed", numberOption(material.flowSpeed, 0.18));
    this.program.setUniform("uDepthShade", numberOption(material.depthShade, 0.18));
    this.program.setUniform("uAmbientColor", color3(material.ambientColor, [0.96, 0.97, 0.92]));
    this.program.setUniform("uLightColor", color3(material.lightColor, [1, 0.97, 0.86]));
    this.program.setUniform("uShadowTint", color3(material.shadowTint, [0.56, 0.64, 0.66]));
    this.program.setUniform("uHighlightColor", color3(material.highlightColor, [1, 0.94, 0.78]));
    const lights = lightArrays(material);
    this.program.setUniform("uLightDirections", lights.directions);
    this.program.setUniform("uLightColors", lights.colors);
    this.program.setUniform("uLightIntensities", lights.intensities);

    bindAttribute(gl, this.program, this.capabilities, "aPosition", this.buffers.position, 3);
    bindAttribute(gl, this.program, this.capabilities, "aNormal", this.buffers.normal, 3);
    bindAttribute(gl, this.program, this.capabilities, "aColor", this.buffers.color, 4);
    bindAttribute(gl, this.program, this.capabilities, "aDistance", this.buffers.distance, 1);
    bindAttribute(gl, this.program, this.capabilities, "aPathIndex", this.buffers.pathIndex, 1);

    if (this.buffers.index && this.indexCount > 0) {
      gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.buffers.index);
      gl.drawElements(gl.TRIANGLES, this.indexCount, this.indexType, 0);
    } else {
      gl.drawArrays(gl.TRIANGLES, 0, this.vertexCount);
    }
    restoreDepthWrite(gl);
    return this;
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

function clamp01(value) {
  return Math.max(0, Math.min(1, Number.isFinite(value) ? value : 0));
}

function direction3(value, fallback) {
  const color = color3(value, fallback);
  const length = Math.hypot(color[0], color[1], color[2]) || 1;
  return [color[0] / length, color[1] / length, color[2] / length];
}

function lightArrays(material = {}) {
  const fallbackDirection = direction3(material.lightDirection, [0.35, 0.85, 0.45]);
  const fallbackColor = color3(material.lightColor, [1, 0.97, 0.86]);
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

const TUBE_VERTEX_SHADER = `
precision mediump float;

attribute vec3 aPosition;
attribute vec3 aNormal;
attribute vec4 aColor;
attribute float aDistance;
attribute float aPathIndex;

uniform mat4 uViewProjectionMatrix;

varying vec3 vNormal;
varying vec3 vWorldPosition;
varying vec4 vColor;
varying float vDistance;
varying float vPathIndex;

void main() {
  vNormal = normalize(aNormal);
  vWorldPosition = aPosition;
  vColor = aColor;
  vDistance = aDistance;
  vPathIndex = aPathIndex;
  gl_Position = uViewProjectionMatrix * vec4(aPosition, 1.0);
}
`;

const TUBE_FRAGMENT_SHADER = `
precision mediump float;

uniform vec3 uCameraPosition;
uniform float uTime;
uniform float uAmbient;
uniform float uPointLight;
uniform float uSpecular;
uniform float uSpecularPower;
uniform float uMaterialRoughness;
uniform float uMaterialSaturation;
uniform float uGlobalAlpha;
uniform float uEmission;
uniform float uRimLight;
uniform float uCoreGlow;
uniform float uFlowStrength;
uniform float uFlowScale;
uniform float uFlowSpeed;
uniform float uDepthShade;
uniform vec3 uAmbientColor;
uniform vec3 uLightColor;
uniform vec3 uShadowTint;
uniform vec3 uHighlightColor;
uniform vec3 uLightDirections[4];
uniform vec3 uLightColors[4];
uniform float uLightIntensities[4];

varying vec3 vNormal;
varying vec3 vWorldPosition;
varying vec4 vColor;
varying float vDistance;
varying float vPathIndex;

float luma(vec3 color) {
  return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

vec3 adjustSaturation(vec3 color, float amount) {
  return mix(vec3(luma(color)), color, clamp(amount, 0.0, 2.0));
}

void main() {
  vec3 normal = normalize(vNormal);
  vec3 viewDir = normalize(uCameraPosition - vWorldPosition);
  vec3 baseColor = adjustSaturation(vColor.rgb, uMaterialSaturation);
  float roughness = clamp(uMaterialRoughness, 0.02, 1.0);
  float specularPower = clamp(uSpecularPower, 2.0, 160.0) * mix(1.16, 0.3, roughness);
  vec3 lit = baseColor * uAmbientColor * uAmbient;

  for (int index = 0; index < 4; index++) {
    vec3 lightDir = normalize(uLightDirections[index]);
    vec3 lightColor = uLightColors[index];
    float intensity = uLightIntensities[index] * uPointLight;
    vec3 halfDir = normalize(lightDir + viewDir);
    float diffuse = max(dot(normal, lightDir), 0.0);
    float specular = pow(max(dot(normal, halfDir), 0.0), specularPower) * uSpecular * intensity;
    lit += baseColor * lightColor * diffuse * intensity + uHighlightColor * specular;
  }

  float cameraFacing = max(dot(normal, viewDir), 0.0);
  float rim = pow(max(1.0 - cameraFacing, 0.0), 2.2) * clamp(uRimLight, 0.0, 1.0);
  float core = pow(cameraFacing, 0.42) * clamp(uCoreGlow, 0.0, 1.0);
  float flowWave = sin(vDistance * max(uFlowScale, 0.001) - uTime * uFlowSpeed * 6.28318530718 + vPathIndex * 1.618);
  float flow = smoothstep(0.64, 1.0, flowWave * 0.5 + 0.5) * clamp(uFlowStrength, 0.0, 1.0);
  float lowerHemisphere = 1.0 - smoothstep(-0.16, 0.54, normal.y);
  float depthShade = clamp(uDepthShade, 0.0, 0.8) * lowerHemisphere;

  lit *= 1.0 - depthShade * 0.42;
  lit = mix(lit, lit * uShadowTint, depthShade);
  lit += baseColor * clamp(uEmission, 0.0, 1.0);
  lit += uHighlightColor * (rim + core * 0.28 + flow);

  gl_FragColor = vec4(pow(max(lit, vec3(0.0)), vec3(1.0 / 2.2)), vColor.a * uGlobalAlpha);
}
`;
