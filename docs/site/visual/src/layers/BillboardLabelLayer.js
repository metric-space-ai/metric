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

export class BillboardLabelLayer extends BaseLayer {
  constructor(descriptor = {}, rendererOrGl = null, options = {}) {
    super(descriptor, rendererOrGl, options);
    this.program = null;
    this.texture = null;
    this.buffers = {};
    this.vertexCount = 0;
    this.labelCount = 0;
    this.atlasSize = [1, 1];
  }

  ensureResources() {
    if (!this.gl) return false;
    if (!this.program) {
      this.program = this.track(createProgram(this.gl, "BillboardLabelLayer", LABEL_VERTEX_SHADER, LABEL_FRAGMENT_SHADER));
      this.initialized = true;
    }
    if (this.needsUpload) this.upload();
    return true;
  }

  upload() {
    const labels = normalizeLabels(this.descriptor.labels || this.source?.labels || []);
    this.labelCount = labels.length;
    this.vertexCount = labels.length * 6;
    const atlas = buildLabelAtlas(labels, this.material, this.geometry);
    const geometry = buildLabelGeometry(labels, atlas);
    this.replaceTexture(atlas.canvas);
    this.replaceBuffer("position", geometry.positions);
    this.replaceBuffer("targetPosition", geometry.targetPositions);
    this.replaceBuffer("offset", geometry.offsets);
    this.replaceBuffer("uv", geometry.uvs);
    this.atlasSize = [atlas.canvas.width, atlas.canvas.height];
    this.needsUpload = false;
  }

  render(context = {}) {
    if (this.disposed || this.visible === false || !this.ensureResources() || this.vertexCount <= 0) return this;
    const gl = this.gl;
    const material = this.material || {};
    const cameraBasis = resolveCameraBasis(context?.camera || this.renderer?.camera);
    configureDrawState(gl, material, {
      blend: true,
      cullFace: false,
      depthWrite: material.depthWrite !== false,
    });

    this.program.use();
    setCameraUniforms(this.program, context, this.renderer);
    this.program.setUniform("uMorph", animationProgress(this.animation || {}, context));
    this.program.setUniform("uGlobalAlpha", clamp01(finite(material.alpha, 1)));
    this.program.setUniform("uCameraRight", cameraBasis.right);
    this.program.setUniform("uCameraUp", cameraBasis.up);
    this.program.bindTexture("uTexture", this.texture, 0);
    bindAttribute(gl, this.program, this.capabilities, "aPosition", this.buffers.position, 3);
    bindAttribute(gl, this.program, this.capabilities, "aTargetPosition", this.buffers.targetPosition, 3);
    bindAttribute(gl, this.program, this.capabilities, "aOffset", this.buffers.offset, 2);
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

  replaceTexture(canvas) {
    const gl = this.gl;
    if (this.texture) {
      gl.deleteTexture(this.texture);
      this.resources = this.resources.filter((resource) => resource.handle !== this.texture);
    }
    const texture = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    gl.pixelStorei(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL, false);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, canvas);
    gl.bindTexture(gl.TEXTURE_2D, null);
    this.texture = texture;
    this.track({ kind: "texture", handle: texture });
  }

  dispose() {
    this.buffers = {};
    this.texture = null;
    this.program = null;
    super.dispose();
  }
}

function normalizeLabels(labels) {
  return (Array.isArray(labels) ? labels : [])
    .map((label, index) => {
      const text = String(label?.text ?? label?.label ?? label?.id ?? index);
      const position = vec3(label?.position || label?.sourcePosition || label?.anchor, [0, 0, 0]);
      const targetPosition = vec3(label?.targetPosition || label?.position || label?.anchor, position);
      return {
        text,
        subtext: label?.subtext == null ? "" : String(label.subtext),
        position,
        targetPosition,
        color: color(label?.color, [0.12, 0.16, 0.18, 1]),
        background: color(label?.background || label?.backgroundColor, [1, 1, 1, 0.78]),
        border: color(label?.border || label?.borderColor, [0.12, 0.16, 0.18, 0.16]),
        size: positiveNumber(label?.size, 0.18),
      };
    })
    .filter((label) => label.text.length > 0);
}

function buildLabelAtlas(labels, material = {}, geometry = {}) {
  const scale = positiveNumber(material.textureScale, geometry.textureScale, 2);
  const fontFamily = material.fontFamily || "ui-sans-serif, system-ui, -apple-system, BlinkMacSystemFont, Segoe UI, sans-serif";
  const fontWeight = material.fontWeight || "700";
  const fontSize = Math.max(10, Math.round(positiveNumber(material.fontSize, geometry.fontSize, 30) * scale));
  const subFontSize = Math.max(8, Math.round(fontSize * 0.52));
  const paddingX = Math.round(positiveNumber(material.paddingX, 14) * scale);
  const paddingY = Math.round(positiveNumber(material.paddingY, 9) * scale);
  const gap = Math.round(positiveNumber(material.atlasGap, 8) * scale);
  const maxWidth = Math.max(512, Math.floor(positiveNumber(material.maxAtlasRowWidth, 2048)));
  const measureCanvas = document.createElement("canvas");
  const measure = measureCanvas.getContext("2d");
  measure.font = `${fontWeight} ${fontSize}px ${fontFamily}`;

  const entries = labels.map((label) => {
    const textWidth = measure.measureText(label.text).width;
    let subWidth = 0;
    if (label.subtext) {
      measure.font = `600 ${subFontSize}px ${fontFamily}`;
      subWidth = measure.measureText(label.subtext).width;
      measure.font = `${fontWeight} ${fontSize}px ${fontFamily}`;
    }
    const width = Math.ceil(Math.max(textWidth, subWidth) + paddingX * 2);
    const height = Math.ceil(fontSize + paddingY * 2 + (label.subtext ? subFontSize * 0.9 : 0));
    return { label, width, height, x: 0, y: 0 };
  });

  let x = gap;
  let y = gap;
  let rowHeight = 0;
  let atlasWidth = maxWidth;
  for (const entry of entries) {
    if (x + entry.width + gap > maxWidth && x > gap) {
      x = gap;
      y += rowHeight + gap;
      rowHeight = 0;
    }
    entry.x = x;
    entry.y = y;
    x += entry.width + gap;
    rowHeight = Math.max(rowHeight, entry.height);
  }
  const atlasHeight = Math.max(64, nextPowerOfTwo(y + rowHeight + gap));
  atlasWidth = nextPowerOfTwo(Math.min(maxWidth, Math.max(64, entries.reduce((max, entry) => Math.max(max, entry.x + entry.width + gap), 64))));

  const canvas = document.createElement("canvas");
  canvas.width = atlasWidth;
  canvas.height = atlasHeight;
  const ctx = canvas.getContext("2d");
  ctx.clearRect(0, 0, atlasWidth, atlasHeight);
  ctx.textBaseline = "middle";
  ctx.textAlign = "center";

  for (const entry of entries) {
    const { label } = entry;
    const radius = Math.min(entry.height * 0.38, Math.round(10 * scale));
    roundedRect(ctx, entry.x, entry.y, entry.width, entry.height, radius);
    ctx.fillStyle = cssColor(label.background);
    ctx.fill();
    ctx.strokeStyle = cssColor(label.border);
    ctx.lineWidth = Math.max(1, Math.round(1 * scale));
    ctx.stroke();

    ctx.fillStyle = cssColor(label.color);
    ctx.font = `${fontWeight} ${fontSize}px ${fontFamily}`;
    const centerX = entry.x + entry.width * 0.5;
    const mainY = entry.y + (label.subtext ? entry.height * 0.43 : entry.height * 0.5);
    ctx.fillText(label.text, centerX, mainY);
    if (label.subtext) {
      ctx.globalAlpha = 0.72;
      ctx.font = `600 ${subFontSize}px ${fontFamily}`;
      ctx.fillText(label.subtext, centerX, entry.y + entry.height * 0.74);
      ctx.globalAlpha = 1;
    }
  }
  return { canvas, entries, scale };
}

function buildLabelGeometry(labels, atlas) {
  const vertexCount = labels.length * 6;
  const positions = new Float32Array(vertexCount * 3);
  const targetPositions = new Float32Array(vertexCount * 3);
  const offsets = new Float32Array(vertexCount * 2);
  const uvs = new Float32Array(vertexCount * 2);
  let cursor = 0;
  for (let labelIndex = 0; labelIndex < labels.length; labelIndex += 1) {
    const label = labels[labelIndex];
    const entry = atlas.entries[labelIndex];
    const aspect = entry.width / Math.max(1, entry.height);
    const height = label.size;
    const width = height * aspect;
    const corners = [
      [-0.5 * width, -0.5 * height, entry.x, entry.y + entry.height],
      [ 0.5 * width, -0.5 * height, entry.x + entry.width, entry.y + entry.height],
      [-0.5 * width,  0.5 * height, entry.x, entry.y],
      [-0.5 * width,  0.5 * height, entry.x, entry.y],
      [ 0.5 * width, -0.5 * height, entry.x + entry.width, entry.y + entry.height],
      [ 0.5 * width,  0.5 * height, entry.x + entry.width, entry.y],
    ];
    for (const corner of corners) {
      positions.set(label.position, cursor * 3);
      targetPositions.set(label.targetPosition, cursor * 3);
      offsets[cursor * 2] = corner[0];
      offsets[cursor * 2 + 1] = corner[1];
      uvs[cursor * 2] = corner[2] / atlas.canvas.width;
      uvs[cursor * 2 + 1] = corner[3] / atlas.canvas.height;
      cursor += 1;
    }
  }
  return { positions, targetPositions, offsets, uvs };
}

function resolveCameraBasis(camera) {
  const position = camera?.position || [0, 0, 1];
  const target = camera?.target || [0, 0, 0];
  const upSource = camera?.up || [0, 1, 0];
  const forward = normalize([
    target[0] - position[0],
    target[1] - position[1],
    target[2] - position[2],
  ], [0, 0, -1]);
  let right = normalize(cross(forward, upSource), [1, 0, 0]);
  let up = normalize(cross(right, forward), [0, 1, 0]);
  if (!Number.isFinite(right[0]) || !Number.isFinite(up[0])) {
    right = [1, 0, 0];
    up = [0, 1, 0];
  }
  return { right, up };
}

function animationProgress(animation = {}, context = {}) {
  if (Number.isFinite(Number(animation.progress))) return Math.max(0, Math.min(1, Number(animation.progress)));
  if (!animation.loop || !Number.isFinite(Number(animation.durationMs))) return 0;
  const duration = Math.max(1, Number(animation.durationMs));
  const elapsed = typeof context?.time?.elapsed === "number"
    ? context.time.elapsed
    : (typeof performance !== "undefined" ? performance.now() : 0);
  const phase = ((elapsed % duration) + duration) % duration;
  const t = phase / duration;
  if (animation.direction === "alternate") return t < 0.5 ? t * 2 : 2 - t * 2;
  return t;
}

function roundedRect(ctx, x, y, width, height, radius) {
  const r = Math.max(0, Math.min(radius, width * 0.5, height * 0.5));
  ctx.beginPath();
  ctx.moveTo(x + r, y);
  ctx.lineTo(x + width - r, y);
  ctx.quadraticCurveTo(x + width, y, x + width, y + r);
  ctx.lineTo(x + width, y + height - r);
  ctx.quadraticCurveTo(x + width, y + height, x + width - r, y + height);
  ctx.lineTo(x + r, y + height);
  ctx.quadraticCurveTo(x, y + height, x, y + height - r);
  ctx.lineTo(x, y + r);
  ctx.quadraticCurveTo(x, y, x + r, y);
  ctx.closePath();
}

function cssColor(value) {
  const c = color(value, [0, 0, 0, 1]);
  return `rgba(${Math.round(c[0] * 255)}, ${Math.round(c[1] * 255)}, ${Math.round(c[2] * 255)}, ${c[3]})`;
}

function color(value, fallback) {
  if (Array.isArray(value) || ArrayBuffer.isView(value)) {
    const divisor = Math.max(value[0] || 0, value[1] || 0, value[2] || 0) > 1 ? 255 : 1;
    return [
      finite(value[0], fallback[0] * divisor) / divisor,
      finite(value[1], fallback[1] * divisor) / divisor,
      finite(value[2], fallback[2] * divisor) / divisor,
      clamp01(finite(value[3], fallback[3])),
    ];
  }
  if (typeof value === "string" && /^#([0-9a-f]{3}|[0-9a-f]{6})$/i.test(value.trim())) {
    const hex = value.trim();
    const full = hex.length === 4 ? `#${hex[1]}${hex[1]}${hex[2]}${hex[2]}${hex[3]}${hex[3]}` : hex;
    return [
      parseInt(full.slice(1, 3), 16) / 255,
      parseInt(full.slice(3, 5), 16) / 255,
      parseInt(full.slice(5, 7), 16) / 255,
      fallback[3],
    ];
  }
  return fallback.slice();
}

function vec3(value, fallback) {
  const source = Array.isArray(value) || ArrayBuffer.isView(value) ? value : [];
  return [
    finite(source[0], fallback[0]),
    finite(source[1], fallback[1]),
    finite(source[2], fallback[2]),
  ];
}

function cross(a, b) {
  return [
    a[1] * b[2] - a[2] * b[1],
    a[2] * b[0] - a[0] * b[2],
    a[0] * b[1] - a[1] * b[0],
  ];
}

function normalize(value, fallback) {
  const length = Math.hypot(value[0], value[1], value[2]);
  if (!Number.isFinite(length) || length <= 1e-8) return fallback.slice();
  return [value[0] / length, value[1] / length, value[2] / length];
}

function nextPowerOfTwo(value) {
  let power = 1;
  while (power < value) power *= 2;
  return power;
}

function positiveNumber(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number) && number > 0) return number;
  }
  return 1;
}

function finite(value, fallback = 0) {
  const number = Number(value);
  return Number.isFinite(number) ? number : fallback;
}

function clamp01(value) {
  const number = Number(value);
  return Number.isFinite(number) ? Math.max(0, Math.min(1, number)) : 0;
}

const LABEL_VERTEX_SHADER = `
attribute vec3 aPosition;
attribute vec3 aTargetPosition;
attribute vec2 aOffset;
attribute vec2 aUv;

uniform mat4 uViewProjectionMatrix;
uniform float uMorph;
uniform vec3 uCameraRight;
uniform vec3 uCameraUp;

varying vec2 vUv;

void main() {
  vec3 anchor = mix(aPosition, aTargetPosition, clamp(uMorph, 0.0, 1.0));
  vec3 worldPosition = anchor + uCameraRight * aOffset.x + uCameraUp * aOffset.y;
  gl_Position = uViewProjectionMatrix * vec4(worldPosition, 1.0);
  vUv = aUv;
}
`;

const LABEL_FRAGMENT_SHADER = `
precision mediump float;

uniform sampler2D uTexture;
uniform float uGlobalAlpha;

varying vec2 vUv;

void main() {
  vec4 color = texture2D(uTexture, vUv);
  if (color.a < 0.02) discard;
  gl_FragColor = vec4(color.rgb, color.a * clamp(uGlobalAlpha, 0.0, 1.0));
}
`;
