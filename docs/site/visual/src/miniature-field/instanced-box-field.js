/*
 * Internal native instanced box field renderer.
 *
 * This module keeps the core renderer ideas from the reference instancing scene - a perspective
 * camera at y=-20,z=10, a 25x25 box field, per-instance color, and per-frame
 * noise-driven transforms - but exposes them as data channels for a
 * dependency-free METRIC visualization engine. It is retained for diagnostics
 * only; public METRIC Visual callers should use createMetricVisual() and
 * semantic show*() commands.
 */

const DEFAULT_OPTIONS = {
  pixelRatioCap: 2,
  background: [1, 1, 1, 1],
  autoStart: true,
  grid: {
    nx: 25,
    ny: 25,
    size: 1.6,
    padding: 1,
  },
  marker: {
    defaultSize: 1.6,
    defaultHeight: 1.6,
    minSize: 0.001,
    minHeight: 0.001,
    anchor: "center",
  },
  camera: {
    position: [0, -20, 10],
    target: [0, 0, 0],
    up: [0, 0, 1],
    fov: 45,
    near: 0.1,
    far: 250,
  },
  light: {
    ambientColor: [0.38, 0.38, 0.38],
    pointPosition: [0, 0, 20],
    pointColor: [1, 1, 1],
    pointIntensity: 1.18,
    shininess: 30,
    specularStrength: 0.24,
    attenuation: 0.0025,
  },
  animation: {
    enabled: true,
    seed: 1337,
    speed: 0.0001,
    noiseScale: 0.005,
    rotationAmplitude: Math.PI,
    liftAmplitude: 0.12,
    scaleAmplitude: 0.035,
  },
  style: {
    tiltShift: true,
    focusY: 0.52,
    focusRadius: 0.2,
    blurPixels: 5.5,
    saturation: 1.08,
    contrast: 1.04,
    vignette: 0.1,
  },
};

const DEFAULT_CHANNELS = {
  position: "position",
  x: "x",
  y: "y",
  z: "z",
  color: "color",
  size: "size",
  height: "height",
  scale: "scale",
  animation: "animation",
};

const CUBE_VERTEX_COUNT = 36;

export class InstancedBoxField {
  constructor(canvas, options = {}) {
    if (!canvas || typeof canvas.getContext !== "function") {
      throw new Error("InstancedBoxField requires a canvas element");
    }

    this.canvas = canvas;
    this.options = mergeOptions(DEFAULT_OPTIONS, withoutKeys(options, ["channels", "data"]));
    this.channels = { ...DEFAULT_CHANNELS, ...(options.channels || {}) };
    this.data = Array.isArray(options.data) ? options.data : createDefaultGridData(this.options.grid);
    this.instanceCount = 0;
    this.running = false;
    this.destroyed = false;
    this.contextLost = false;
    this.lastFrameTime = 0;
    this.frameHandle = 0;
    this.renderWidth = 0;
    this.renderHeight = 0;
    this.basePositions = new Float32Array(0);
    this.baseScales = new Float32Array(0);
    this.dynamicPositions = new Float32Array(0);
    this.dynamicScales = new Float32Array(0);
    this.rotations = new Float32Array(0);
    this.colors = new Float32Array(0);
    this.animationWeights = new Float32Array(0);
    this.noise = new DeterministicNoise3D(this.options.animation.seed);
    this.projectionMatrix = mat4Identity();
    this.viewMatrix = mat4Identity();

    this.gl = this._createContext(canvas);
    this.instancing = createInstancingAdapter(this.gl);
    if (!this.instancing) {
      throw new Error("Instanced rendering is not available in this browser");
    }

    this._initResources();
    this._bindContextEvents();
    this.setData(this.data, this.channels);

    if (this.options.autoStart) {
      this.start();
    } else {
      this.render(0);
    }
  }

  setData(data, channels) {
    if (channels) {
      this.channels = { ...this.channels, ...channels };
    }
    this.data = Array.isArray(data) ? data : [];
    this._buildInstanceData();
    this._uploadInstanceBuffers();
    this.render(this.lastFrameTime);
    return this;
  }

  setChannels(channels = {}) {
    this.channels = { ...this.channels, ...channels };
    this._buildInstanceData();
    this._uploadInstanceBuffers();
    this.render(this.lastFrameTime);
    return this;
  }

  setOptions(options = {}) {
    this.options = mergeOptions(this.options, withoutKeys(options, ["channels", "data"]));
    if (options.channels) {
      this.channels = { ...this.channels, ...options.channels };
    }
    this.noise = new DeterministicNoise3D(this.options.animation.seed);
    this._buildInstanceData();
    this._uploadInstanceBuffers();
    this.render(this.lastFrameTime);
    return this;
  }

  setCamera(camera = {}) {
    this.options.camera = mergeOptions(this.options.camera, camera);
    this.render(this.lastFrameTime);
    return this;
  }

  start() {
    if (this.running || this.destroyed) return this;
    this.running = true;
    const tick = (time) => {
      if (!this.running || this.destroyed) return;
      this.render(time);
      this.frameHandle = requestAnimationFrame(tick);
    };
    this.frameHandle = requestAnimationFrame(tick);
    return this;
  }

  stop() {
    this.running = false;
    if (this.frameHandle) {
      cancelAnimationFrame(this.frameHandle);
      this.frameHandle = 0;
    }
    return this;
  }

  resize() {
    const ratio = Math.min(
      this.options.pixelRatioCap,
      typeof window === "undefined" ? 1 : window.devicePixelRatio || 1,
    );
    const cssWidth = this.canvas.clientWidth || this.canvas.width || 1;
    const cssHeight = this.canvas.clientHeight || this.canvas.height || 1;
    const width = Math.max(1, Math.floor(cssWidth * ratio));
    const height = Math.max(1, Math.floor(cssHeight * ratio));
    if (this.canvas.width !== width || this.canvas.height !== height) {
      this.canvas.width = width;
      this.canvas.height = height;
    }
    if (this.renderWidth !== width || this.renderHeight !== height) {
      this.renderWidth = width;
      this.renderHeight = height;
      this._resizePostTarget(width, height);
    }
    return [width, height];
  }

  render(time = 0) {
    if (this.destroyed || this.contextLost || !this.gl) return this;
    this.lastFrameTime = time;
    const gl = this.gl;
    const [width, height] = this.resize();
    const usePost = Boolean(this.options.style.tiltShift && this.postTarget && this.postTarget.complete);

    this._updateAnimatedInstances(time);
    this._updateMatrices(width / height);

    if (usePost) {
      gl.bindFramebuffer(gl.FRAMEBUFFER, this.postTarget.framebuffer);
    } else {
      gl.bindFramebuffer(gl.FRAMEBUFFER, null);
    }
    gl.viewport(0, 0, width, height);
    this._renderScene();

    if (usePost) {
      gl.bindFramebuffer(gl.FRAMEBUFFER, null);
      gl.viewport(0, 0, width, height);
      this._renderPostProcess();
    }
    return this;
  }

  destroy() {
    if (this.destroyed) return;
    this.stop();
    this.destroyed = true;
    this.canvas.removeEventListener("webglcontextlost", this._onContextLost);
    this.canvas.removeEventListener("webglcontextrestored", this._onContextRestored);
    this._deleteResources();
  }

  _createContext(canvas) {
    const attributes = {
      alpha: true,
      antialias: true,
      depth: true,
      stencil: false,
      premultipliedAlpha: false,
      preserveDrawingBuffer: false,
    };
    const gl = canvas.getContext("webgl2", attributes)
      || canvas.getContext("webgl", attributes)
      || canvas.getContext("experimental-webgl", attributes);
    if (!gl) {
      throw new Error("WebGL is not available");
    }
    return gl;
  }

  _initResources() {
    const gl = this.gl;
    this.postTarget = undefined;
    this.sceneProgram = createProgram(gl, SCENE_VERTEX_SHADER, SCENE_FRAGMENT_SHADER);
    this.postProgram = createProgram(gl, POST_VERTEX_SHADER, POST_FRAGMENT_SHADER);

    this.cubePositionBuffer = gl.createBuffer();
    this.cubeNormalBuffer = gl.createBuffer();
    this.instancePositionBuffer = gl.createBuffer();
    this.instanceScaleBuffer = gl.createBuffer();
    this.instanceRotationBuffer = gl.createBuffer();
    this.instanceColorBuffer = gl.createBuffer();
    this.quadPositionBuffer = gl.createBuffer();
    this.quadUvBuffer = gl.createBuffer();

    const cube = createCubeGeometry();
    gl.bindBuffer(gl.ARRAY_BUFFER, this.cubePositionBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, cube.positions, gl.STATIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.cubeNormalBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, cube.normals, gl.STATIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.quadPositionBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([
      -1, -1,
      1, -1,
      -1, 1,
      -1, 1,
      1, -1,
      1, 1,
    ]), gl.STATIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.quadUvBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([
      0, 0,
      1, 0,
      0, 1,
      0, 1,
      1, 0,
      1, 1,
    ]), gl.STATIC_DRAW);

    gl.enable(gl.DEPTH_TEST);
    gl.enable(gl.CULL_FACE);
    gl.cullFace(gl.BACK);
    gl.clearDepth(1);
  }

  _bindContextEvents() {
    this._onContextLost = (event) => {
      event.preventDefault();
      this.contextLost = true;
      this.stop();
    };
    this._onContextRestored = () => {
      this.contextLost = false;
      this.gl = this._createContext(this.canvas);
      this.instancing = createInstancingAdapter(this.gl);
      this._initResources();
      this._uploadInstanceBuffers();
      if (this.options.autoStart) {
        this.start();
      } else {
        this.render(this.lastFrameTime);
      }
    };
    this.canvas.addEventListener("webglcontextlost", this._onContextLost);
    this.canvas.addEventListener("webglcontextrestored", this._onContextRestored);
  }

  _buildInstanceData() {
    const data = this.data;
    const count = data.length;
    this.instanceCount = count;
    this.basePositions = new Float32Array(count * 3);
    this.baseScales = new Float32Array(count * 3);
    this.dynamicPositions = new Float32Array(count * 3);
    this.dynamicScales = new Float32Array(count * 3);
    this.rotations = new Float32Array(count * 3);
    this.colors = new Float32Array(count * 3);
    this.animationWeights = new Float32Array(count);

    for (let index = 0; index < count; index += 1) {
      const datum = data[index];
      const position = this._resolvePosition(datum, index, count);
      const scale = this._resolveScale(datum, index);
      const color = normalizeColor(
        readChannel(this.channels.color, datum, index, data, undefined),
        colorFromIndex(index),
      );
      const animation = readChannel(this.channels.animation, datum, index, data, 1);

      writeVec3(this.basePositions, index, position);
      writeVec3(this.baseScales, index, scale);
      writeVec3(this.dynamicPositions, index, position);
      writeVec3(this.dynamicScales, index, scale);
      writeVec3(this.colors, index, color);
      this.animationWeights[index] = clamp(toFiniteNumber(animation, 1), 0, 10);
    }
  }

  _resolvePosition(datum, index, count) {
    const direct = readChannel(this.channels.position, datum, index, this.data, undefined);
    if (isVecLike(direct, 2)) {
      return [
        toFiniteNumber(direct[0], 0),
        toFiniteNumber(direct[1], 0),
        toFiniteNumber(direct[2], 0),
      ];
    }
    if (Array.isArray(datum) && datum.length >= 2 && this.channels.position === DEFAULT_CHANNELS.position) {
      return [
        toFiniteNumber(datum[0], 0),
        toFiniteNumber(datum[1], 0),
        toFiniteNumber(datum[2], 0),
      ];
    }
    const fallback = gridPositionForIndex(index, count, this.options.grid);
    return [
      toFiniteNumber(readChannel(this.channels.x, datum, index, this.data, fallback[0]), fallback[0]),
      toFiniteNumber(readChannel(this.channels.y, datum, index, this.data, fallback[1]), fallback[1]),
      toFiniteNumber(readChannel(this.channels.z, datum, index, this.data, fallback[2]), fallback[2]),
    ];
  }

  _resolveScale(datum, index) {
    const marker = this.options.marker;
    const scaleChannel = readChannel(this.channels.scale, datum, index, this.data, undefined);
    if (isVecLike(scaleChannel, 3)) {
      return [
        Math.max(marker.minSize, Math.abs(toFiniteNumber(scaleChannel[0], marker.defaultSize))),
        Math.max(marker.minSize, Math.abs(toFiniteNumber(scaleChannel[1], marker.defaultSize))),
        Math.max(marker.minHeight, Math.abs(toFiniteNumber(scaleChannel[2], marker.defaultHeight))),
      ];
    }
    if (isVecLike(scaleChannel, 1)) {
      const s = Math.max(marker.minSize, Math.abs(toFiniteNumber(scaleChannel[0], marker.defaultSize)));
      return [s, s, s];
    }
    if (Number.isFinite(Number(scaleChannel))) {
      const s = Math.max(marker.minSize, Math.abs(Number(scaleChannel)));
      return [s, s, s];
    }

    const sizeValue = readChannel(this.channels.size, datum, index, this.data, marker.defaultSize);
    const heightValue = readChannel(this.channels.height, datum, index, this.data, marker.defaultHeight);
    const size = normalizeSize(sizeValue, marker.defaultSize, marker.minSize);
    const height = Math.max(marker.minHeight, Math.abs(toFiniteNumber(heightValue, marker.defaultHeight)));
    return [size[0], size[1], height];
  }

  _uploadInstanceBuffers() {
    const gl = this.gl;
    if (!gl) return;
    gl.bindBuffer(gl.ARRAY_BUFFER, this.instancePositionBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, this.dynamicPositions, gl.DYNAMIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.instanceScaleBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, this.dynamicScales, gl.DYNAMIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.instanceRotationBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, this.rotations, gl.DYNAMIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.instanceColorBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, this.colors, gl.STATIC_DRAW);
  }

  _updateAnimatedInstances(time) {
    const animation = this.options.animation;
    const gl = this.gl;
    if (!animation.enabled || this.instanceCount === 0) {
      return;
    }

    const t = time * animation.speed;
    const scale = animation.noiseScale;
    const rotationAmplitude = animation.rotationAmplitude;
    const liftAmplitude = animation.liftAmplitude;
    const scaleAmplitude = animation.scaleAmplitude;

    for (let index = 0; index < this.instanceCount; index += 1) {
      const offset = index * 3;
      const x = this.basePositions[offset];
      const y = this.basePositions[offset + 1];
      const z = this.basePositions[offset + 2];
      const weight = this.animationWeights[index];
      const nx = x * scale;
      const ny = y * scale;

      // The reference scene used simplex.noise3D(nx, ny, time) for rx and swapped x/y for ry.
      // This seeded value-noise implementation keeps that motion without npm.
      const rx = this.noise.noise(nx, ny, t) * rotationAmplitude * weight;
      const ry = this.noise.noise(ny, nx, t) * rotationAmplitude * weight;
      const rz = this.noise.noise(nx + 17.13, ny - 9.71, t * 0.73) * rotationAmplitude * 0.15 * weight;
      const lift = this.noise.noise(nx - 5.5, ny + 3.25, t * 0.9) * liftAmplitude * weight;
      const pulse = 1 + this.noise.noise(nx + 31.7, ny + 11.9, t * 1.3) * scaleAmplitude * weight;

      this.rotations[offset] = rx;
      this.rotations[offset + 1] = ry;
      this.rotations[offset + 2] = rz;
      this.dynamicPositions[offset] = x;
      this.dynamicPositions[offset + 1] = y;
      this.dynamicPositions[offset + 2] = z + lift;
      this.dynamicScales[offset] = this.baseScales[offset] * pulse;
      this.dynamicScales[offset + 1] = this.baseScales[offset + 1] * pulse;
      this.dynamicScales[offset + 2] = this.baseScales[offset + 2] * Math.max(0.05, pulse);
    }

    gl.bindBuffer(gl.ARRAY_BUFFER, this.instancePositionBuffer);
    gl.bufferSubData(gl.ARRAY_BUFFER, 0, this.dynamicPositions);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.instanceScaleBuffer);
    gl.bufferSubData(gl.ARRAY_BUFFER, 0, this.dynamicScales);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.instanceRotationBuffer);
    gl.bufferSubData(gl.ARRAY_BUFFER, 0, this.rotations);
  }

  _updateMatrices(aspect) {
    const camera = this.options.camera;
    this.projectionMatrix = mat4Perspective(
      degreesToRadians(camera.fov),
      Math.max(0.0001, aspect),
      camera.near,
      camera.far,
    );
    this.viewMatrix = mat4LookAt(camera.position, camera.target, camera.up);
  }

  _renderScene() {
    const gl = this.gl;
    const bg = this.options.background;
    gl.clearColor(bg[0], bg[1], bg[2], bg[3]);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    if (this.instanceCount === 0) return;

    gl.useProgram(this.sceneProgram);
    gl.enable(gl.DEPTH_TEST);
    gl.enable(gl.CULL_FACE);

    setUniformMatrix(gl, this.sceneProgram, "uProjection", this.projectionMatrix);
    setUniformMatrix(gl, this.sceneProgram, "uView", this.viewMatrix);
    setUniform3(gl, this.sceneProgram, "uCameraPosition", this.options.camera.position);
    setUniform3(gl, this.sceneProgram, "uAmbientColor", this.options.light.ambientColor);
    setUniform3(gl, this.sceneProgram, "uPointLightPosition", this.options.light.pointPosition);
    setUniform3(gl, this.sceneProgram, "uPointLightColor", this.options.light.pointColor);
    setUniform1(gl, this.sceneProgram, "uPointLightIntensity", this.options.light.pointIntensity);
    setUniform1(gl, this.sceneProgram, "uShininess", this.options.light.shininess);
    setUniform1(gl, this.sceneProgram, "uSpecularStrength", this.options.light.specularStrength);
    setUniform1(gl, this.sceneProgram, "uAttenuation", this.options.light.attenuation);
    setUniform1(gl, this.sceneProgram, "uAnchorBase", this.options.marker.anchor === "base" ? 1 : 0);

    bindAttribute(gl, this.sceneProgram, this.instancing, "aPosition", this.cubePositionBuffer, 3, 0);
    bindAttribute(gl, this.sceneProgram, this.instancing, "aNormal", this.cubeNormalBuffer, 3, 0);
    bindAttribute(gl, this.sceneProgram, this.instancing, "aInstancePosition", this.instancePositionBuffer, 3, 1);
    bindAttribute(gl, this.sceneProgram, this.instancing, "aInstanceScale", this.instanceScaleBuffer, 3, 1);
    bindAttribute(gl, this.sceneProgram, this.instancing, "aInstanceRotation", this.instanceRotationBuffer, 3, 1);
    bindAttribute(gl, this.sceneProgram, this.instancing, "aInstanceColor", this.instanceColorBuffer, 3, 1);

    this.instancing.drawArraysInstanced(gl.TRIANGLES, 0, CUBE_VERTEX_COUNT, this.instanceCount);
  }

  _renderPostProcess() {
    const gl = this.gl;
    const style = this.options.style;
    gl.useProgram(this.postProgram);
    gl.disable(gl.DEPTH_TEST);
    gl.disable(gl.CULL_FACE);
    gl.clearColor(0, 0, 0, 0);
    gl.clear(gl.COLOR_BUFFER_BIT);

    bindAttribute(gl, this.postProgram, this.instancing, "aPosition", this.quadPositionBuffer, 2, 0);
    bindAttribute(gl, this.postProgram, this.instancing, "aUv", this.quadUvBuffer, 2, 0);

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, this.postTarget.texture);
    setUniform1i(gl, this.postProgram, "uScene", 0);
    setUniform2(gl, this.postProgram, "uTexel", [1 / this.renderWidth, 1 / this.renderHeight]);
    setUniform1(gl, this.postProgram, "uFocusY", style.focusY);
    setUniform1(gl, this.postProgram, "uFocusRadius", style.focusRadius);
    setUniform1(gl, this.postProgram, "uBlurPixels", style.blurPixels);
    setUniform1(gl, this.postProgram, "uSaturation", style.saturation);
    setUniform1(gl, this.postProgram, "uContrast", style.contrast);
    setUniform1(gl, this.postProgram, "uVignette", style.vignette);
    gl.drawArrays(gl.TRIANGLES, 0, 6);
  }

  _resizePostTarget(width, height) {
    const gl = this.gl;
    if (!this.postTarget) {
      this.postTarget = {
        framebuffer: gl.createFramebuffer(),
        texture: gl.createTexture(),
        depth: gl.createRenderbuffer(),
        width: 0,
        height: 0,
        complete: false,
      };
    }
    if (this.postTarget.width === width && this.postTarget.height === height) {
      return;
    }

    this.postTarget.width = width;
    this.postTarget.height = height;
    gl.bindTexture(gl.TEXTURE_2D, this.postTarget.texture);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, width, height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);

    gl.bindRenderbuffer(gl.RENDERBUFFER, this.postTarget.depth);
    gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, width, height);

    gl.bindFramebuffer(gl.FRAMEBUFFER, this.postTarget.framebuffer);
    gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, this.postTarget.texture, 0);
    gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.RENDERBUFFER, this.postTarget.depth);
    this.postTarget.complete = gl.checkFramebufferStatus(gl.FRAMEBUFFER) === gl.FRAMEBUFFER_COMPLETE;
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
  }

  _deleteResources() {
    const gl = this.gl;
    if (!gl) return;
    for (const program of [this.sceneProgram, this.postProgram]) {
      if (program) gl.deleteProgram(program);
    }
    for (const buffer of [
      this.cubePositionBuffer,
      this.cubeNormalBuffer,
      this.instancePositionBuffer,
      this.instanceScaleBuffer,
      this.instanceRotationBuffer,
      this.instanceColorBuffer,
      this.quadPositionBuffer,
      this.quadUvBuffer,
    ]) {
      if (buffer) gl.deleteBuffer(buffer);
    }
    if (this.postTarget) {
      gl.deleteFramebuffer(this.postTarget.framebuffer);
      gl.deleteTexture(this.postTarget.texture);
      gl.deleteRenderbuffer(this.postTarget.depth);
    }
  }
}

export function createInstancedBoxField(canvas, options) {
  return new InstancedBoxField(canvas, options);
}

export class DeterministicNoise3D {
  constructor(seed = 1337) {
    this.seed = seed >>> 0;
  }

  noise(x, y, z) {
    const x0 = Math.floor(x);
    const y0 = Math.floor(y);
    const z0 = Math.floor(z);
    const xf = x - x0;
    const yf = y - y0;
    const zf = z - z0;
    const u = fade(xf);
    const v = fade(yf);
    const w = fade(zf);

    const n000 = this._hashValue(x0, y0, z0);
    const n100 = this._hashValue(x0 + 1, y0, z0);
    const n010 = this._hashValue(x0, y0 + 1, z0);
    const n110 = this._hashValue(x0 + 1, y0 + 1, z0);
    const n001 = this._hashValue(x0, y0, z0 + 1);
    const n101 = this._hashValue(x0 + 1, y0, z0 + 1);
    const n011 = this._hashValue(x0, y0 + 1, z0 + 1);
    const n111 = this._hashValue(x0 + 1, y0 + 1, z0 + 1);

    const x00 = lerp(n000, n100, u);
    const x10 = lerp(n010, n110, u);
    const x01 = lerp(n001, n101, u);
    const x11 = lerp(n011, n111, u);
    const y0v = lerp(x00, x10, v);
    const y1v = lerp(x01, x11, v);
    return lerp(y0v, y1v, w);
  }

  _hashValue(x, y, z) {
    let h = this.seed ^ Math.imul(x, 374761393) ^ Math.imul(y, 668265263) ^ Math.imul(z, 2147483647);
    h = Math.imul(h ^ (h >>> 13), 1274126177);
    h ^= h >>> 16;
    return ((h >>> 0) / 4294967295) * 2 - 1;
  }
}

function createInstancingAdapter(gl) {
  if (typeof gl.drawArraysInstanced === "function" && typeof gl.vertexAttribDivisor === "function") {
    return {
      vertexAttribDivisor: (location, divisor) => gl.vertexAttribDivisor(location, divisor),
      drawArraysInstanced: (mode, first, count, instanceCount) => {
        gl.drawArraysInstanced(mode, first, count, instanceCount);
      },
    };
  }
  const extension = gl.getExtension("ANGLE_instanced_arrays");
  if (!extension) return undefined;
  return {
    vertexAttribDivisor: (location, divisor) => extension.vertexAttribDivisorANGLE(location, divisor),
    drawArraysInstanced: (mode, first, count, instanceCount) => {
      extension.drawArraysInstancedANGLE(mode, first, count, instanceCount);
    },
  };
}

function createDefaultGridData(grid) {
  const size = grid.size;
  const step = grid.size + grid.padding;
  const width = grid.nx * step - grid.padding;
  const height = grid.ny * step - grid.padding;
  const x0 = -width / 2 + grid.padding;
  const y0 = -height / 2 + grid.padding;
  const data = [];
  for (let ix = 0; ix < grid.nx; ix += 1) {
    for (let iy = 0; iy < grid.ny; iy += 1) {
      const index = ix * grid.ny + iy;
      data.push({
        x: x0 + ix * step,
        y: y0 + iy * step,
        z: 0,
        size,
        height: size,
        animation: 1,
        color: colorFromIndex(index),
      });
    }
  }
  return data;
}

function gridPositionForIndex(index, count, grid) {
  const columns = grid.nx || Math.ceil(Math.sqrt(Math.max(1, count)));
  const rows = Math.ceil(Math.max(1, count) / columns);
  const step = grid.size + grid.padding;
  const x = (index % columns - (columns - 1) / 2) * step;
  const y = (Math.floor(index / columns) - (rows - 1) / 2) * step;
  return [x, y, 0];
}

function readChannel(channel, datum, index, data, fallback) {
  if (channel === undefined || channel === null) return fallback;
  if (typeof channel === "function") {
    const value = channel(datum, index, data);
    return value === undefined ? fallback : value;
  }
  if (typeof channel === "string") {
    const value = readPath(datum, channel);
    return value === undefined ? fallback : value;
  }
  return channel;
}

function readPath(object, path) {
  if (!object || typeof object !== "object") return undefined;
  if (Object.prototype.hasOwnProperty.call(object, path)) {
    return object[path];
  }
  const parts = path.split(".");
  let value = object;
  for (const part of parts) {
    if (!value || typeof value !== "object" || !Object.prototype.hasOwnProperty.call(value, part)) {
      return undefined;
    }
    value = value[part];
  }
  return value;
}

function normalizeSize(value, fallback, minSize) {
  if (isVecLike(value, 2)) {
    return [
      Math.max(minSize, Math.abs(toFiniteNumber(value[0], fallback))),
      Math.max(minSize, Math.abs(toFiniteNumber(value[1], fallback))),
    ];
  }
  const size = Math.max(minSize, Math.abs(toFiniteNumber(value, fallback)));
  return [size, size];
}

function normalizeColor(value, fallback) {
  if (typeof value === "string") {
    const parsed = parseHexColor(value);
    return parsed || fallback;
  }
  if (isVecLike(value, 3)) {
    const max = Math.max(Math.abs(Number(value[0])), Math.abs(Number(value[1])), Math.abs(Number(value[2])));
    const divisor = max > 1 ? 255 : 1;
    return [
      clamp(toFiniteNumber(value[0], fallback[0]) / divisor, 0, 1),
      clamp(toFiniteNumber(value[1], fallback[1]) / divisor, 0, 1),
      clamp(toFiniteNumber(value[2], fallback[2]) / divisor, 0, 1),
    ];
  }
  if (Number.isInteger(value) && value > 1 && value <= 0xffffff) {
    return [
      ((value >> 16) & 255) / 255,
      ((value >> 8) & 255) / 255,
      (value & 255) / 255,
    ];
  }
  if (Number.isFinite(Number(value))) {
    return scalarRamp(clamp(Number(value), 0, 1));
  }
  return fallback;
}

function parseHexColor(value) {
  const normalized = value.trim();
  const short = /^#([0-9a-f]{3})$/i.exec(normalized);
  if (short) {
    return short[1].split("").map((part) => parseInt(part + part, 16) / 255);
  }
  const full = /^#([0-9a-f]{6})$/i.exec(normalized);
  if (full) {
    const hex = parseInt(full[1], 16);
    return [
      ((hex >> 16) & 255) / 255,
      ((hex >> 8) & 255) / 255,
      (hex & 255) / 255,
    ];
  }
  return undefined;
}

function scalarRamp(t) {
  const a = [0.12, 0.2, 0.34];
  const b = [0.02, 0.58, 0.62];
  const c = [0.94, 0.7, 0.25];
  const d = [0.88, 0.24, 0.18];
  if (t < 0.33) return mixColor(a, b, t / 0.33);
  if (t < 0.72) return mixColor(b, c, (t - 0.33) / 0.39);
  return mixColor(c, d, (t - 0.72) / 0.28);
}

function colorFromIndex(index) {
  const hue = (index * 0.618033988749895) % 1;
  return hslToRgb(hue, 0.46, 0.56);
}

function hslToRgb(h, s, l) {
  if (s === 0) return [l, l, l];
  const q = l < 0.5 ? l * (1 + s) : l + s - l * s;
  const p = 2 * l - q;
  return [
    hueToRgb(p, q, h + 1 / 3),
    hueToRgb(p, q, h),
    hueToRgb(p, q, h - 1 / 3),
  ];
}

function hueToRgb(p, q, t) {
  let value = t;
  if (value < 0) value += 1;
  if (value > 1) value -= 1;
  if (value < 1 / 6) return p + (q - p) * 6 * value;
  if (value < 1 / 2) return q;
  if (value < 2 / 3) return p + (q - p) * (2 / 3 - value) * 6;
  return p;
}

function mixColor(a, b, t) {
  return [
    lerp(a[0], b[0], t),
    lerp(a[1], b[1], t),
    lerp(a[2], b[2], t),
  ];
}

function createCubeGeometry() {
  const vertices = [
    // +Z
    [-0.5, -0.5, 0.5], [0.5, -0.5, 0.5], [0.5, 0.5, 0.5],
    [-0.5, -0.5, 0.5], [0.5, 0.5, 0.5], [-0.5, 0.5, 0.5],
    // -Z
    [0.5, -0.5, -0.5], [-0.5, -0.5, -0.5], [-0.5, 0.5, -0.5],
    [0.5, -0.5, -0.5], [-0.5, 0.5, -0.5], [0.5, 0.5, -0.5],
    // +Y
    [-0.5, 0.5, 0.5], [0.5, 0.5, 0.5], [0.5, 0.5, -0.5],
    [-0.5, 0.5, 0.5], [0.5, 0.5, -0.5], [-0.5, 0.5, -0.5],
    // -Y
    [0.5, -0.5, 0.5], [-0.5, -0.5, 0.5], [-0.5, -0.5, -0.5],
    [0.5, -0.5, 0.5], [-0.5, -0.5, -0.5], [0.5, -0.5, -0.5],
    // +X
    [0.5, 0.5, 0.5], [0.5, -0.5, 0.5], [0.5, -0.5, -0.5],
    [0.5, 0.5, 0.5], [0.5, -0.5, -0.5], [0.5, 0.5, -0.5],
    // -X
    [-0.5, -0.5, 0.5], [-0.5, 0.5, 0.5], [-0.5, 0.5, -0.5],
    [-0.5, -0.5, 0.5], [-0.5, 0.5, -0.5], [-0.5, -0.5, -0.5],
  ];
  const normals = [
    [0, 0, 1], [0, 0, 1], [0, 0, 1],
    [0, 0, 1], [0, 0, 1], [0, 0, 1],
    [0, 0, -1], [0, 0, -1], [0, 0, -1],
    [0, 0, -1], [0, 0, -1], [0, 0, -1],
    [0, 1, 0], [0, 1, 0], [0, 1, 0],
    [0, 1, 0], [0, 1, 0], [0, 1, 0],
    [0, -1, 0], [0, -1, 0], [0, -1, 0],
    [0, -1, 0], [0, -1, 0], [0, -1, 0],
    [1, 0, 0], [1, 0, 0], [1, 0, 0],
    [1, 0, 0], [1, 0, 0], [1, 0, 0],
    [-1, 0, 0], [-1, 0, 0], [-1, 0, 0],
    [-1, 0, 0], [-1, 0, 0], [-1, 0, 0],
  ];
  return {
    positions: new Float32Array(vertices.flat()),
    normals: new Float32Array(normals.flat()),
  };
}

function createProgram(gl, vertexSource, fragmentSource) {
  const vertexShader = compileShader(gl, gl.VERTEX_SHADER, vertexSource);
  const fragmentShader = compileShader(gl, gl.FRAGMENT_SHADER, fragmentSource);
  const program = gl.createProgram();
  gl.attachShader(program, vertexShader);
  gl.attachShader(program, fragmentShader);
  gl.linkProgram(program);
  gl.deleteShader(vertexShader);
  gl.deleteShader(fragmentShader);
  if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
    const message = gl.getProgramInfoLog(program) || "Unknown shader program link error";
    gl.deleteProgram(program);
    throw new Error(message);
  }
  return program;
}

function compileShader(gl, type, source) {
  const shader = gl.createShader(type);
  gl.shaderSource(shader, source);
  gl.compileShader(shader);
  if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
    const message = gl.getShaderInfoLog(shader) || "Unknown shader compile error";
    gl.deleteShader(shader);
    throw new Error(message);
  }
  return shader;
}

function bindAttribute(gl, program, instancing, name, buffer, size, divisor) {
  const location = gl.getAttribLocation(program, name);
  if (location < 0) return;
  gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
  gl.enableVertexAttribArray(location);
  gl.vertexAttribPointer(location, size, gl.FLOAT, false, 0, 0);
  instancing.vertexAttribDivisor(location, divisor);
}

function setUniformMatrix(gl, program, name, matrix) {
  const location = gl.getUniformLocation(program, name);
  if (location) gl.uniformMatrix4fv(location, false, matrix);
}

function setUniform3(gl, program, name, value) {
  const location = gl.getUniformLocation(program, name);
  if (location) gl.uniform3fv(location, value);
}

function setUniform2(gl, program, name, value) {
  const location = gl.getUniformLocation(program, name);
  if (location) gl.uniform2fv(location, value);
}

function setUniform1(gl, program, name, value) {
  const location = gl.getUniformLocation(program, name);
  if (location) gl.uniform1f(location, value);
}

function setUniform1i(gl, program, name, value) {
  const location = gl.getUniformLocation(program, name);
  if (location) gl.uniform1i(location, value);
}

function mat4Identity() {
  return new Float32Array([
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
  ]);
}

function mat4Perspective(fovy, aspect, near, far) {
  const f = 1 / Math.tan(fovy / 2);
  const nf = 1 / (near - far);
  return new Float32Array([
    f / aspect, 0, 0, 0,
    0, f, 0, 0,
    0, 0, (far + near) * nf, -1,
    0, 0, 2 * far * near * nf, 0,
  ]);
}

function mat4LookAt(eye, target, up) {
  const z = normalize([
    eye[0] - target[0],
    eye[1] - target[1],
    eye[2] - target[2],
  ]);
  const x = normalize(cross(up, z));
  const y = cross(z, x);
  return new Float32Array([
    x[0], y[0], z[0], 0,
    x[1], y[1], z[1], 0,
    x[2], y[2], z[2], 0,
    -dot(x, eye), -dot(y, eye), -dot(z, eye), 1,
  ]);
}

function normalize(v) {
  const length = Math.hypot(v[0], v[1], v[2]) || 1;
  return [v[0] / length, v[1] / length, v[2] / length];
}

function cross(a, b) {
  return [
    a[1] * b[2] - a[2] * b[1],
    a[2] * b[0] - a[0] * b[2],
    a[0] * b[1] - a[1] * b[0],
  ];
}

function dot(a, b) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

function writeVec3(array, index, value) {
  const offset = index * 3;
  array[offset] = value[0];
  array[offset + 1] = value[1];
  array[offset + 2] = value[2];
}

function isVecLike(value, minLength) {
  return value && typeof value.length === "number" && value.length >= minLength;
}

function toFiniteNumber(value, fallback) {
  const number = Number(value);
  return Number.isFinite(number) ? number : fallback;
}

function clamp(value, min, max) {
  return Math.min(max, Math.max(min, value));
}

function lerp(a, b, t) {
  return a + (b - a) * t;
}

function fade(t) {
  return t * t * t * (t * (t * 6 - 15) + 10);
}

function degreesToRadians(degrees) {
  return degrees * Math.PI / 180;
}

function mergeOptions(base, override) {
  const result = Array.isArray(base) ? base.slice() : { ...base };
  for (const [key, value] of Object.entries(override || {})) {
    if (isPlainObject(value) && isPlainObject(result[key])) {
      result[key] = mergeOptions(result[key], value);
    } else if (Array.isArray(value)) {
      result[key] = value.slice();
    } else {
      result[key] = value;
    }
  }
  return result;
}

function withoutKeys(object, keys) {
  const result = { ...object };
  for (const key of keys) {
    delete result[key];
  }
  return result;
}

function isPlainObject(value) {
  return Boolean(value) && Object.prototype.toString.call(value) === "[object Object]";
}

const SCENE_VERTEX_SHADER = `
attribute vec3 aPosition;
attribute vec3 aNormal;
attribute vec3 aInstancePosition;
attribute vec3 aInstanceScale;
attribute vec3 aInstanceRotation;
attribute vec3 aInstanceColor;

uniform mat4 uProjection;
uniform mat4 uView;
uniform float uAnchorBase;

varying vec3 vColor;
varying vec3 vNormal;
varying vec3 vWorldPosition;

vec3 rotateXYZ(vec3 p, vec3 r) {
  float cx = cos(r.x);
  float sx = sin(r.x);
  float cy = cos(r.y);
  float sy = sin(r.y);
  float cz = cos(r.z);
  float sz = sin(r.z);

  p = vec3(p.x, p.y * cx - p.z * sx, p.y * sx + p.z * cx);
  p = vec3(p.x * cy + p.z * sy, p.y, -p.x * sy + p.z * cy);
  p = vec3(p.x * cz - p.y * sz, p.x * sz + p.y * cz, p.z);
  return p;
}

void main() {
  vec3 local = aPosition;
  if (uAnchorBase > 0.5) {
    local.z += 0.5;
  }
  vec3 world = rotateXYZ(local * aInstanceScale, aInstanceRotation) + aInstancePosition;
  vNormal = normalize(rotateXYZ(aNormal, aInstanceRotation));
  vWorldPosition = world;
  vColor = aInstanceColor;
  gl_Position = uProjection * uView * vec4(world, 1.0);
}
`;

const SCENE_FRAGMENT_SHADER = `
precision mediump float;

uniform vec3 uCameraPosition;
uniform vec3 uAmbientColor;
uniform vec3 uPointLightPosition;
uniform vec3 uPointLightColor;
uniform float uPointLightIntensity;
uniform float uShininess;
uniform float uSpecularStrength;
uniform float uAttenuation;

varying vec3 vColor;
varying vec3 vNormal;
varying vec3 vWorldPosition;

void main() {
  vec3 normal = normalize(vNormal);
  vec3 lightVector = uPointLightPosition - vWorldPosition;
  float lightDistance = length(lightVector);
  vec3 lightDir = normalize(lightVector);
  vec3 viewDir = normalize(uCameraPosition - vWorldPosition);
  vec3 reflected = reflect(-lightDir, normal);

  float diffuse = max(dot(normal, lightDir), 0.0);
  float specular = pow(max(dot(viewDir, reflected), 0.0), uShininess) * uSpecularStrength;
  float attenuation = 1.0 / (1.0 + lightDistance * lightDistance * uAttenuation);
  vec3 light = uAmbientColor + uPointLightColor * diffuse * attenuation * uPointLightIntensity;
  vec3 color = vColor * light + uPointLightColor * specular * attenuation;
  gl_FragColor = vec4(pow(max(color, vec3(0.0)), vec3(1.0 / 2.2)), 1.0);
}
`;

const POST_VERTEX_SHADER = `
attribute vec2 aPosition;
attribute vec2 aUv;

varying vec2 vUv;

void main() {
  vUv = aUv;
  gl_Position = vec4(aPosition, 0.0, 1.0);
}
`;

const POST_FRAGMENT_SHADER = `
precision mediump float;

uniform sampler2D uScene;
uniform vec2 uTexel;
uniform float uFocusY;
uniform float uFocusRadius;
uniform float uBlurPixels;
uniform float uSaturation;
uniform float uContrast;
uniform float uVignette;

varying vec2 vUv;

vec4 sampleScene(vec2 uv) {
  return texture2D(uScene, clamp(uv, vec2(0.0), vec2(1.0)));
}

void main() {
  vec4 base = sampleScene(vUv);
  vec2 radius = uTexel * uBlurPixels;
  vec4 blur = base * 0.22;
  blur += sampleScene(vUv + vec2(radius.x * 1.5, 0.0)) * 0.11;
  blur += sampleScene(vUv - vec2(radius.x * 1.5, 0.0)) * 0.11;
  blur += sampleScene(vUv + vec2(0.0, radius.y * 1.5)) * 0.11;
  blur += sampleScene(vUv - vec2(0.0, radius.y * 1.5)) * 0.11;
  blur += sampleScene(vUv + radius * 1.2) * 0.085;
  blur += sampleScene(vUv - radius * 1.2) * 0.085;
  blur += sampleScene(vUv + vec2(radius.x, -radius.y) * 1.2) * 0.085;
  blur += sampleScene(vUv + vec2(-radius.x, radius.y) * 1.2) * 0.085;

  float distanceToFocus = abs(vUv.y - uFocusY);
  float blurAmount = smoothstep(uFocusRadius, uFocusRadius + 0.34, distanceToFocus);
  vec3 color = mix(base.rgb, blur.rgb, blurAmount);
  float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
  color = mix(vec3(luma), color, uSaturation);
  color = (color - 0.5) * uContrast + 0.5;
  float vignette = 1.0 - smoothstep(0.32, 0.78, distance(vUv, vec2(0.5)));
  color *= mix(1.0 - uVignette, 1.0, vignette);
  gl_FragColor = vec4(clamp(color, 0.0, 1.0), base.a);
}
`;
