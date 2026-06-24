import { BaseLayer } from "./BaseLayer.js";
import {
  channelToFloat32,
  colorChannel,
  getChannel,
  getChannelItemSize,
  inferInstanceCount,
  positionChannel,
  scalarChannel,
} from "./channels.js";
import { CUBE_VERTEX_COUNT, createCubeGeometry } from "./geometry.js";
import {
  bindAttribute,
  configureDrawState,
  createBuffer,
  createProgram,
  drawArraysInstanced,
  frameTimeSeconds,
  restoreDepthWrite,
  setCameraUniforms,
  trackBuffer,
} from "./gl-utils.js";

export class InstancedBoxLayer extends BaseLayer {
  constructor(descriptor = {}, rendererOrGl = null, options = {}) {
    super(descriptor, rendererOrGl, options);
    this.count = 0;
    this.buffers = {};
    this.program = null;
  }

  ensureResources() {
    if (!this.gl) return false;
    if (!this.capabilities?.instancing?.supported) {
      throw new Error("InstancedBoxLayer requires WebGL2 or ANGLE_instanced_arrays.");
    }
    if (!this.program) {
      const cube = createCubeGeometry();
      this.program = this.track(createProgram(this.gl, "InstancedBoxLayer", BOX_VERTEX_SHADER, BOX_FRAGMENT_SHADER));
      this.buffers.cubePosition = createBuffer(this.gl, cube.positions, this.gl.STATIC_DRAW);
      this.buffers.cubeNormal = createBuffer(this.gl, cube.normals, this.gl.STATIC_DRAW);
      trackBuffer(this, this.buffers.cubePosition);
      trackBuffer(this, this.buffers.cubeNormal);
    }
    if (this.needsUpload) this.upload();
    return true;
  }

  upload() {
    const gl = this.gl;
    this.count = inferInstanceCount(this.channels, ["position", "scale", "size", "height", "color", "alpha"]);
    const positions = positionChannel(this.channels, "position", this.count);
    const scales = this.buildScaleArray(this.count);
    const colors = colorChannel(this.channels, this.count, { defaultColor: [0.26, 0.48, 0.62, 1] });
    const phase = scalarChannel(this.channels, ["animationPhase", "phase"], this.count, 0);
    const focus = scalarChannel(this.channels, ["focusWeight", "focus", "selection"], this.count, 0);

    this.replaceBuffer("instancePosition", positions);
    this.replaceBuffer("instanceScale", scales);
    this.replaceBuffer("instanceColor", colors);
    this.replaceBuffer("instancePhase", phase);
    this.replaceBuffer("instanceFocus", focus);
    this.needsUpload = false;
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
  }

  buildScaleArray(count) {
    const scaleChannel = getChannel(this.channels, "scale");
    if (scaleChannel) return channelToFloat32(scaleChannel, 3, count, [1, 1, 1]);

    const sizeChannel = getChannel(this.channels, "size");
    const height = scalarChannel(this.channels, "height", count, 1);
    const out = new Float32Array(count * 3);
    if (!sizeChannel) {
      for (let index = 0; index < count; index += 1) {
        out[index * 3] = 1;
        out[index * 3 + 1] = Math.max(0.001, Math.abs(height[index]));
        out[index * 3 + 2] = 1;
      }
      return out;
    }

    const sizeArray = channelToFloat32(sizeChannel, Math.min(3, Math.max(1, getChannelItemSize(sizeChannel, 1))), count, 1);
    const sourceSize = getChannelItemSize(sizeChannel, 1);
    for (let index = 0; index < count; index += 1) {
      const sourceOffset = index * Math.min(3, Math.max(1, sourceSize));
      const sx = Math.max(0.001, Math.abs(sizeArray[sourceOffset] || 1));
      const sz = sourceSize >= 2 ? Math.max(0.001, Math.abs(sizeArray[sourceOffset + 1] || sx)) : sx;
      const sy = Math.max(0.001, Math.abs(height[index] || (sourceSize >= 3 ? sizeArray[sourceOffset + 2] : 1)));
      out[index * 3] = sx;
      out[index * 3 + 1] = sy;
      out[index * 3 + 2] = sz;
    }
    return out;
  }

  render(context = {}) {
    if (this.disposed || this.visible === false || !this.ensureResources() || this.count <= 0) return this;
    const gl = this.gl;
    const material = this.material || {};
    const geometry = this.geometry || {};
    const animation = this.animation || {};

    configureDrawState(gl, material, {
      blend: material.alphaMode === "blend",
      cullFace: material.backFace !== "double-sided",
      depthWrite: material.alphaMode !== "blend",
    });

    this.program.use();
    setCameraUniforms(this.program, context, this.renderer);
    this.program.setUniform("uTime", frameTimeSeconds(context));
    this.program.setUniform("uAmbient", numberOption(material.ambient, 0.52));
    this.program.setUniform("uPointLight", numberOption(material.pointLight, 0.42));
    this.program.setUniform("uSpecular", numberOption(material.specular, 0.08));
    this.program.setUniform("uSpecularPower", numberOption(material.specularPower, material.shininess, 42));
    this.program.setUniform("uMaterialRoughness", numberOption(material.roughness, 0.52));
    this.program.setUniform("uMaterialMetalness", numberOption(material.metalness, 0.14));
    this.program.setUniform("uMaterialSaturation", numberOption(material.saturation, 0.96));
    this.program.setUniform("uShadowSoftness", numberOption(material.shadowSoftness, 0.32));
    this.program.setUniform("uBaseLift", numberOption(material.baseLift, 0.035));
    this.program.setUniform("uGlobalAlpha", numberOption(material.alpha, 1));
    this.program.setUniform("uAmbientColor", color3(material.ambientColor, [1, 1, 1]));
    this.program.setUniform("uLightColor", color3(material.lightColor, [1, 1, 1]));
    this.program.setUniform("uShadowTint", color3(material.shadowTint, [0.76, 0.8, 0.82]));
    this.program.setUniform("uHighlightColor", color3(material.highlightColor, color3(material.lightColor, [1, 1, 1])));
    this.program.setUniform("uLightDirection", direction3(material.lightDirection, [0.35, 0.85, 0.45]));
    this.program.setUniform("uGroundOcclusion", numberOption(material.groundOcclusion, material.ambientOcclusion, 0.16));
    this.program.setUniform("uFaceContrast", numberOption(material.faceContrast, 0.22));
    this.program.setUniform("uVerticalGradient", numberOption(material.verticalGradient, 0.14));
    this.program.setUniform("uRimLight", numberOption(material.rimLight, 0.08));
    this.program.setUniform("uRimColor", color3(material.rimColor, color3(material.lightColor, [1, 1, 1])));
    const lights = lightArrays(material);
    this.program.setUniform("uLightDirections", lights.directions);
    this.program.setUniform("uLightColors", lights.colors);
    this.program.setUniform("uLightIntensities", lights.intensities);
    this.program.setUniform("uAnchorBase", geometry.anchor === "base" || material.anchor === "base" ? 1 : 0);
    this.program.setUniform("uMotion", animation.mode === "attention" || animation.mode === "uncertainty" ? 1 : 0);

    bindAttribute(gl, this.program, this.capabilities, "aPosition", this.buffers.cubePosition, 3, { divisor: 0 });
    bindAttribute(gl, this.program, this.capabilities, "aNormal", this.buffers.cubeNormal, 3, { divisor: 0 });
    bindAttribute(gl, this.program, this.capabilities, "aInstancePosition", this.buffers.instancePosition, 3, { divisor: 1 });
    bindAttribute(gl, this.program, this.capabilities, "aInstanceScale", this.buffers.instanceScale, 3, { divisor: 1 });
    bindAttribute(gl, this.program, this.capabilities, "aInstanceColor", this.buffers.instanceColor, 4, { divisor: 1 });
    bindAttribute(gl, this.program, this.capabilities, "aInstancePhase", this.buffers.instancePhase, 1, { divisor: 1 });
    bindAttribute(gl, this.program, this.capabilities, "aInstanceFocus", this.buffers.instanceFocus, 1, { divisor: 1 });

    drawArraysInstanced(gl, this.capabilities, gl.TRIANGLES, 0, CUBE_VERTEX_COUNT, this.count);
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
  const fallbackColor = color3(material.lightColor, [1, 1, 1]);
  const fallbackIntensity = numberOption(material.lightIntensity, 1);
  const directions = new Float32Array(12);
  const colors = new Float32Array(12);
  const intensities = new Float32Array(4);
  const directionSource = material.pointLightDirections || material.lightDirections || null;
  const colorSource = material.pointLightColors || material.lightColors || null;
  const intensitySource = material.pointLightIntensities || material.lightIntensities || null;

  for (let index = 0; index < 4; index += 1) {
    const direction = readVec3(directionSource, index, index === 0 ? fallbackDirection : fallbackDirection);
    const color = readVec3(colorSource, index, index === 0 ? fallbackColor : fallbackColor);
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

const BOX_VERTEX_SHADER = `
precision mediump float;

attribute vec3 aPosition;
attribute vec3 aNormal;
attribute vec3 aInstancePosition;
attribute vec3 aInstanceScale;
attribute vec4 aInstanceColor;
attribute float aInstancePhase;
attribute float aInstanceFocus;

uniform mat4 uViewProjectionMatrix;
uniform float uTime;
uniform float uAnchorBase;
uniform float uMotion;

varying vec3 vNormal;
varying vec3 vWorldPosition;
varying vec3 vLocalPosition;
varying vec4 vColor;
varying float vFocus;

void main() {
  vec3 local = aPosition;
  if (uAnchorBase > 0.5) {
    local.y += 0.5;
  }
  float focus = clamp(aInstanceFocus, 0.0, 1.0);
  float phase = sin(uTime * 2.5 + aInstancePhase * 6.28318530718);
  vec3 scale = aInstanceScale * (1.0 + uMotion * phase * focus * 0.035);
  vec3 world = aInstancePosition + local * scale;
  world.y += uMotion * phase * focus * 0.035 * max(scale.y, 0.05);
  vNormal = normalize(aNormal);
  vWorldPosition = world;
  vLocalPosition = local;
  vColor = aInstanceColor;
  vFocus = focus;
  gl_Position = uViewProjectionMatrix * vec4(world, 1.0);
}
`;

const BOX_FRAGMENT_SHADER = `
precision mediump float;

uniform vec3 uCameraPosition;
uniform float uAmbient;
uniform float uPointLight;
uniform float uSpecular;
uniform float uSpecularPower;
uniform float uMaterialRoughness;
uniform float uMaterialMetalness;
uniform float uMaterialSaturation;
uniform float uShadowSoftness;
uniform float uBaseLift;
uniform float uGlobalAlpha;
uniform vec3 uAmbientColor;
uniform vec3 uLightColor;
uniform vec3 uShadowTint;
uniform vec3 uHighlightColor;
uniform vec3 uLightDirection;
uniform vec3 uLightDirections[4];
uniform vec3 uLightColors[4];
uniform float uLightIntensities[4];
uniform float uGroundOcclusion;
uniform float uFaceContrast;
uniform float uVerticalGradient;
uniform float uRimLight;
uniform vec3 uRimColor;
uniform float uAnchorBase;

varying vec3 vNormal;
varying vec3 vWorldPosition;
varying vec3 vLocalPosition;
varying vec4 vColor;
varying float vFocus;

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
  float metalness = clamp(uMaterialMetalness, 0.0, 1.0);
  float specularPower = clamp(uSpecularPower, 2.0, 160.0) * mix(1.18, 0.28, roughness);
  vec3 specularColor = mix(uHighlightColor, baseColor * uHighlightColor, metalness);
  vec3 lit = baseColor * uAmbientColor * uAmbient;
  for (int index = 0; index < 4; index++) {
    vec3 lightDir = normalize(uLightDirections[index]);
    vec3 lightColor = uLightColors[index];
    float lightIntensity = uLightIntensities[index] * uPointLight;
    vec3 halfDir = normalize(lightDir + viewDir);
    float diffuse = max(dot(normal, lightDir), 0.0);
    float gloss = pow(max(dot(normal, halfDir), 0.0), specularPower) * uSpecular * mix(1.15, 0.42, roughness) * lightIntensity;
    lit += baseColor * lightColor * diffuse * lightIntensity + specularColor * gloss;
  }
  float vertical = clamp(vLocalPosition.y + mix(0.5, 0.0, step(0.5, uAnchorBase)), 0.0, 1.0);
  float baseContact = 1.0 - smoothstep(0.02, 0.55, vertical);
  float sideMask = 1.0 - max(normal.y, 0.0);
  float faceLift = normal.y * 0.5 + 0.5;
  float faceShading = mix(1.0 - uFaceContrast * 0.42, 1.0 + uFaceContrast * 0.22, faceLift);
  float verticalLift = mix(1.0 - uVerticalGradient, 1.0, vertical);
  float contactOcclusion = 1.0 - clamp(uGroundOcclusion, 0.0, 0.85) * baseContact * (0.35 + 0.65 * sideMask);
  float rim = pow(max(1.0 - dot(viewDir, normal), 0.0), 2.6) * clamp(uRimLight, 0.0, 1.0);
  float softShadow = clamp(uShadowSoftness, 0.0, 1.0) * clamp((1.0 - faceLift) * 0.52 + baseContact * 0.38 + sideMask * 0.18, 0.0, 1.0);
  lit *= faceShading * verticalLift * contactOcclusion;
  lit = mix(lit, lit * uShadowTint, softShadow);
  lit += baseColor * clamp(uBaseLift, 0.0, 0.4) * (0.35 + vertical * 0.65);
  lit += uRimColor * rim;
  lit = mix(lit, vec3(1.0), vFocus * 0.08);
  gl_FragColor = vec4(pow(max(lit, vec3(0.0)), vec3(1.0 / 2.2)), vColor.a * uGlobalAlpha);
}
`;
