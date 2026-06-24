import { BaseLayer } from "./BaseLayer.js";
import {
  colorChannel,
  getChannel,
  getChannelArray,
  getChannelCount,
  getChannelItemSize,
  positionChannel,
} from "./channels.js";
import {
  bindAttribute,
  configureDrawState,
  createBuffer,
  createProgram,
  restoreDepthWrite,
  safeLineWidth,
  setCameraUniforms,
  trackBuffer,
} from "./gl-utils.js";

export class RelationEdgeLayer extends BaseLayer {
  constructor(descriptor = {}, rendererOrGl = null, options = {}) {
    super(descriptor, rendererOrGl, options);
    this.edgeCount = 0;
    this.vertexCount = 0;
    this.buffers = {};
    this.program = null;
  }

  ensureResources() {
    if (!this.gl) return false;
    if (!this.program) {
      this.program = this.track(createProgram(this.gl, "RelationEdgeLayer", EDGE_VERTEX_SHADER, EDGE_FRAGMENT_SHADER));
    }
    if (this.needsUpload) this.upload();
    return true;
  }

  upload() {
    const edgeCount = inferEdgeCount(this.channels);
    const positions = buildEdgePositions(this.channels, edgeCount);
    const colors = buildEdgeColors(this.channels, edgeCount);
    this.edgeCount = edgeCount;
    this.vertexCount = edgeCount * 2;
    this.replaceBuffer("position", positions);
    this.replaceBuffer("color", colors);
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
    safeLineWidth(gl, this.geometry.width || material.width || 1);

    this.program.use();
    setCameraUniforms(this.program, context, this.renderer);
    this.program.setUniform("uGlobalAlpha", numberOption(material.alpha, 1));
    bindAttribute(gl, this.program, this.capabilities, "aPosition", this.buffers.position, 3);
    bindAttribute(gl, this.program, this.capabilities, "aColor", this.buffers.color, 4);
    gl.drawArrays(gl.LINES, 0, this.vertexCount);
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

function inferEdgeCount(channels) {
  const source = getChannel(channels, ["sourcePosition", "source"]);
  const target = getChannel(channels, ["targetPosition", "target"]);
  if (source && target) return Math.min(getChannelCount(source, 3), getChannelCount(target, 3));
  const position = getChannel(channels, "position");
  const itemSize = getChannelItemSize(position, 3);
  if (itemSize >= 6) return getChannelCount(position, itemSize);
  return Math.floor(getChannelCount(position, 3) / 2);
}

function buildEdgePositions(channels, edgeCount) {
  const source = getChannel(channels, ["sourcePosition", "source"]);
  const target = getChannel(channels, ["targetPosition", "target"]);
  const out = new Float32Array(edgeCount * 2 * 3);
  if (source && target) {
    const sources = positionChannel(channels, ["sourcePosition", "source"], edgeCount);
    const targets = positionChannel(channels, ["targetPosition", "target"], edgeCount);
    for (let index = 0; index < edgeCount; index += 1) {
      out.set(sources.subarray(index * 3, index * 3 + 3), index * 6);
      out.set(targets.subarray(index * 3, index * 3 + 3), index * 6 + 3);
    }
    return out;
  }

  const position = getChannel(channels, "position");
  const array = getChannelArray(position);
  const itemSize = getChannelItemSize(position, 3);
  if (!array) return out;
  for (let index = 0; index < edgeCount; index += 1) {
    const targetOffset = index * 6;
    if (itemSize >= 6) {
      const sourceOffset = index * itemSize;
      for (let component = 0; component < 6; component += 1) {
        out[targetOffset + component] = Number(array[sourceOffset + component]) || 0;
      }
    } else {
      const sourceOffset = index * 6;
      for (let component = 0; component < 6; component += 1) {
        out[targetOffset + component] = Number(array[sourceOffset + component]) || 0;
      }
    }
  }
  return out;
}

function buildEdgeColors(channels, edgeCount) {
  const perEdge = colorChannel(channels, edgeCount, { defaultColor: [0.18, 0.30, 0.38, 0.42] });
  const out = new Float32Array(edgeCount * 2 * 4);
  for (let index = 0; index < edgeCount; index += 1) {
    const color = perEdge.subarray(index * 4, index * 4 + 4);
    out.set(color, index * 8);
    out.set(color, index * 8 + 4);
  }
  return out;
}

function numberOption(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number)) return number;
  }
  return 0;
}

const EDGE_VERTEX_SHADER = `
attribute vec3 aPosition;
attribute vec4 aColor;

uniform mat4 uViewProjectionMatrix;

varying vec4 vColor;

void main() {
  gl_Position = uViewProjectionMatrix * vec4(aPosition, 1.0);
  vColor = aColor;
}
`;

const EDGE_FRAGMENT_SHADER = `
precision mediump float;

uniform float uGlobalAlpha;
varying vec4 vColor;

void main() {
  gl_FragColor = vec4(vColor.rgb, vColor.a * uGlobalAlpha);
}
`;

