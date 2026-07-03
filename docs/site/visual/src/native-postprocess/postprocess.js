/*
 * Native METRIC postprocessing helpers.
 *
 * Dependency-free WebGL 1 pipeline for applying fullscreen passes to any
 * renderer that can draw into the currently bound framebuffer.
 */

export const FULLSCREEN_QUAD_VERTEX_SHADER = `
  attribute vec2 aPosition;
  attribute vec2 aUv;
  varying vec2 vUv;

  void main() {
    vUv = aUv;
    gl_Position = vec4(aPosition, 0.0, 1.0);
  }
`;

export const COPY_FRAGMENT_SHADER = `
  precision mediump float;

  uniform sampler2D uTexture;
  varying vec2 vUv;

  void main() {
    gl_FragColor = texture2D(uTexture, vUv);
  }
`;

export const FXAA_LIKE_FRAGMENT_SHADER = `
  precision mediump float;

  uniform sampler2D uTexture;
  uniform vec2 uTexelSize;
  uniform float uStrength;
  uniform float uEdgeThreshold;
  uniform float uEdgeThresholdMin;
  varying vec2 vUv;

  float luma(vec3 color) {
    return dot(color, vec3(0.299, 0.587, 0.114));
  }

  void main() {
    vec4 center = texture2D(uTexture, vUv);
    float m = luma(center.rgb);
    float n = luma(texture2D(uTexture, vUv + vec2(0.0, uTexelSize.y)).rgb);
    float s = luma(texture2D(uTexture, vUv - vec2(0.0, uTexelSize.y)).rgb);
    float e = luma(texture2D(uTexture, vUv + vec2(uTexelSize.x, 0.0)).rgb);
    float w = luma(texture2D(uTexture, vUv - vec2(uTexelSize.x, 0.0)).rgb);

    float rangeMin = min(m, min(min(n, s), min(e, w)));
    float rangeMax = max(m, max(max(n, s), max(e, w)));
    float range = rangeMax - rangeMin;
    float threshold = max(uEdgeThresholdMin, rangeMax * uEdgeThreshold);

    if (range < threshold) {
      gl_FragColor = center;
      return;
    }

    vec2 gradient = vec2(w - e, s - n);
    vec2 direction = normalize(gradient + vec2(0.00001)) * uTexelSize;
    vec4 edgeAverage = 0.5 * (
      texture2D(uTexture, vUv + direction) +
      texture2D(uTexture, vUv - direction)
    );

    float blend = clamp((range - threshold) / max(range, 0.00001), 0.0, 1.0);
    gl_FragColor = mix(center, edgeAverage, blend * uStrength);
  }
`;

export class RenderTarget {
  constructor(gl, width = 1, height = 1, options = {}) {
    this.gl = gl;
    this.width = Math.max(1, Math.floor(width));
    this.height = Math.max(1, Math.floor(height));
    this.depth = Boolean(options.depth);
    this.depthTextureRequested = Boolean(options.depthTexture);
    this.depthTextureConfig = this.depth && this.depthTextureRequested
      ? resolveDepthTextureConfig(gl)
      : null;
    this.depthEncoding = this.depthTextureConfig ? "raw-depth" : null;
    this.format = options.format || gl.RGBA;
    this.type = options.type || gl.UNSIGNED_BYTE;
    this.minFilter = options.minFilter || gl.LINEAR;
    this.magFilter = options.magFilter || gl.LINEAR;
    this.wrapS = options.wrapS || gl.CLAMP_TO_EDGE;
    this.wrapT = options.wrapT || gl.CLAMP_TO_EDGE;
    this.texture = gl.createTexture();
    this.framebuffer = gl.createFramebuffer();
    this.depthTexture = this.depthTextureConfig ? gl.createTexture() : null;
    this.depthBuffer = this.depth && !this.depthTexture ? gl.createRenderbuffer() : null;
    this.setSize(this.width, this.height);
  }

  setSize(width, height) {
    const nextWidth = Math.max(1, Math.floor(width));
    const nextHeight = Math.max(1, Math.floor(height));
    const gl = this.gl;
    this.width = nextWidth;
    this.height = nextHeight;

    gl.bindTexture(gl.TEXTURE_2D, this.texture);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, this.minFilter);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, this.magFilter);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, this.wrapS);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, this.wrapT);
    gl.texImage2D(gl.TEXTURE_2D, 0, this.format, nextWidth, nextHeight, 0, this.format, this.type, null);

    gl.bindFramebuffer(gl.FRAMEBUFFER, this.framebuffer);
    gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, this.texture, 0);

    if (this.depthTexture) {
      gl.bindTexture(gl.TEXTURE_2D, this.depthTexture);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
      gl.texImage2D(
        gl.TEXTURE_2D,
        0,
        this.depthTextureConfig.internalFormat,
        nextWidth,
        nextHeight,
        0,
        this.depthTextureConfig.format,
        this.depthTextureConfig.type,
        null,
      );
      gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.TEXTURE_2D, this.depthTexture, 0);
    } else if (this.depthBuffer) {
      gl.bindRenderbuffer(gl.RENDERBUFFER, this.depthBuffer);
      gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, nextWidth, nextHeight);
      gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.RENDERBUFFER, this.depthBuffer);
    }

    if (gl.checkFramebufferStatus(gl.FRAMEBUFFER) !== gl.FRAMEBUFFER_COMPLETE) {
      throw new Error("Unable to create native postprocess render target");
    }

    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
    gl.bindTexture(gl.TEXTURE_2D, null);
    gl.bindRenderbuffer(gl.RENDERBUFFER, null);
  }

  bind() {
    const gl = this.gl;
    gl.bindFramebuffer(gl.FRAMEBUFFER, this.framebuffer);
    gl.viewport(0, 0, this.width, this.height);
  }

  dispose() {
    const gl = this.gl;
    if (this.depthBuffer) gl.deleteRenderbuffer(this.depthBuffer);
    if (this.depthTexture) gl.deleteTexture(this.depthTexture);
    gl.deleteFramebuffer(this.framebuffer);
    gl.deleteTexture(this.texture);
    this.depthBuffer = null;
    this.depthTexture = null;
    this.framebuffer = null;
    this.texture = null;
  }
}

export function createRenderTarget(gl, width, height, options = {}) {
  return new RenderTarget(gl, width, height, options);
}

export class FullscreenQuad {
  constructor(gl) {
    this.gl = gl;
    this.buffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, this.buffer);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([
      -1, -1, 0, 0,
       1, -1, 1, 0,
      -1,  1, 0, 1,
      -1,  1, 0, 1,
       1, -1, 1, 0,
       1,  1, 1, 1,
    ]), gl.STATIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
  }

  draw(attributes) {
    const gl = this.gl;
    gl.bindBuffer(gl.ARRAY_BUFFER, this.buffer);
    if (attributes.position >= 0) {
      gl.enableVertexAttribArray(attributes.position);
      gl.vertexAttribPointer(attributes.position, 2, gl.FLOAT, false, 16, 0);
    }
    if (attributes.uv >= 0) {
      gl.enableVertexAttribArray(attributes.uv);
      gl.vertexAttribPointer(attributes.uv, 2, gl.FLOAT, false, 16, 8);
    }
    gl.drawArrays(gl.TRIANGLES, 0, 6);
  }

  dispose() {
    this.gl.deleteBuffer(this.buffer);
    this.buffer = null;
  }
}

export class FullscreenQuadPass {
  constructor(gl, options = {}) {
    this.gl = gl;
    this.enabled = options.enabled !== false;
    this.clear = Boolean(options.clear);
    this.textureUniform = options.textureUniform || "uTexture";
    this.vertexShader = options.vertexShader || FULLSCREEN_QUAD_VERTEX_SHADER;
    this.fragmentShader = options.fragmentShader || COPY_FRAGMENT_SHADER;
    this.program = createProgram(gl, this.vertexShader, this.fragmentShader);
    this.attributes = {
      position: gl.getAttribLocation(this.program, "aPosition"),
      uv: gl.getAttribLocation(this.program, "aUv"),
    };
    this.uniforms = collectUniformLocations(gl, this.program);
    this.uniformValues = { ...(options.uniforms || {}) };
    this.beforeRender = options.beforeRender;
    this.width = 1;
    this.height = 1;
  }

  setSize(width, height) {
    this.width = Math.max(1, Math.floor(width));
    this.height = Math.max(1, Math.floor(height));
  }

  setUniform(name, value) {
    this.uniformValues[name] = value;
    return this;
  }

  render(context, inputTexture, outputTarget = null) {
    const gl = this.gl;
    const width = context.width || this.width;
    const height = context.height || this.height;

    bindOutputTarget(gl, outputTarget, width, height);
    gl.disable(gl.DEPTH_TEST);
    gl.depthMask(false);
    gl.disable(gl.CULL_FACE);
    gl.disable(gl.BLEND);
    if (this.clear) gl.clear(gl.COLOR_BUFFER_BIT);

    gl.useProgram(this.program);
    if (this.textureUniform && inputTexture) {
      gl.activeTexture(gl.TEXTURE0);
      gl.bindTexture(gl.TEXTURE_2D, inputTexture);
      setUniformValue(gl, this.uniforms[this.textureUniform], 0, "int");
    }
    setAutoUniforms(gl, this.uniforms, width, height);

    if (this.beforeRender) this.beforeRender(context, this);
    for (const [name, value] of Object.entries(this.uniformValues)) {
      setUniformValue(gl, this.uniforms[name], value);
    }

    context.quad.draw(this.attributes);
    gl.depthMask(true);
  }

  dispose() {
    this.gl.deleteProgram(this.program);
    this.program = null;
  }
}

export class CopyPass extends FullscreenQuadPass {
  constructor(gl, options = {}) {
    super(gl, {
      ...options,
      fragmentShader: options.fragmentShader || COPY_FRAGMENT_SHADER,
    });
  }
}

export class FxaaPass extends FullscreenQuadPass {
  constructor(gl, options = {}) {
    super(gl, {
      ...options,
      fragmentShader: options.fragmentShader || FXAA_LIKE_FRAGMENT_SHADER,
      uniforms: {
        uStrength: options.strength ?? 0.72,
        uEdgeThreshold: options.edgeThreshold ?? 0.125,
        uEdgeThresholdMin: options.edgeThresholdMin ?? 0.03125,
        ...(options.uniforms || {}),
      },
    });
  }

  setStrength(value) {
    return this.setUniform("uStrength", value);
  }
}

export class NativePostProcessPipeline {
  constructor(gl, options = {}) {
    this.gl = gl;
    this.width = Math.max(1, Math.floor(options.width || gl.canvas?.width || 1));
    this.height = Math.max(1, Math.floor(options.height || gl.canvas?.height || 1));
    this.ownsQuad = !options.quad;
    this.quad = options.quad || new FullscreenQuad(gl);
    const sceneDepthTexture = options.sceneDepthTexture === true
      || options.cameraDepthTexture === true
      || options.depthTexture === true;
    this.sceneTarget = new RenderTarget(gl, this.width, this.height, {
      depth: options.sceneDepth !== false,
      depthTexture: sceneDepthTexture,
    });
    this.pingTarget = new RenderTarget(gl, this.width, this.height);
    this.pongTarget = new RenderTarget(gl, this.width, this.height);
    this.copyPass = new CopyPass(gl);
    this.passes = [];
  }

  setSize(width, height) {
    const nextWidth = Math.max(1, Math.floor(width));
    const nextHeight = Math.max(1, Math.floor(height));
    if (this.width === nextWidth && this.height === nextHeight) return;
    this.width = nextWidth;
    this.height = nextHeight;
    this.sceneTarget.setSize(nextWidth, nextHeight);
    this.pingTarget.setSize(nextWidth, nextHeight);
    this.pongTarget.setSize(nextWidth, nextHeight);
    this.copyPass.setSize(nextWidth, nextHeight);
    for (const pass of this.passes) {
      if (pass.setSize) pass.setSize(nextWidth, nextHeight);
    }
  }

  addPass(pass) {
    this.passes.push(pass);
    if (pass.setSize) pass.setSize(this.width, this.height);
    return pass;
  }

  removePass(pass) {
    const index = this.passes.indexOf(pass);
    if (index >= 0) this.passes.splice(index, 1);
    return pass;
  }

  clearPasses() {
    this.passes.length = 0;
  }

  renderScene(drawScene, options = {}) {
    const width = Math.max(1, Math.floor(options.width || this.gl.canvas?.width || this.width));
    const height = Math.max(1, Math.floor(options.height || this.gl.canvas?.height || this.height));
    this.setSize(width, height);

    const enabledPasses = this.passes.filter((pass) => pass.enabled !== false);
    if (!enabledPasses.length) {
      bindOutputTarget(this.gl, options.target || null, width, height);
      drawScene({ gl: this.gl, width, height, target: options.target || null, pipeline: this });
      return null;
    }

    this.sceneTarget.bind();
    drawScene({ gl: this.gl, width, height, target: this.sceneTarget, pipeline: this });
    const sceneDepthTexture = this.sceneTarget.depthTexture || null;
    return this.processTexture(this.sceneTarget.texture, {
      ...options,
      width,
      height,
      passes: enabledPasses,
      cameraDepthTexture: options.cameraDepthTexture
        || options.depthTexture
        || options.cameraDepthTarget?.texture
        || options.depthTarget?.texture
        || sceneDepthTexture,
      cameraDepthEncoding: options.cameraDepthEncoding
        || options.depthEncoding
        || (sceneDepthTexture ? this.sceneTarget.depthEncoding : null),
      cameraDepthTarget: options.cameraDepthTarget
        || options.depthTarget
        || (sceneDepthTexture ? this.sceneTarget : null),
    });
  }

  processTexture(sourceTexture, options = {}) {
    const width = Math.max(1, Math.floor(options.width || this.gl.canvas?.width || this.width));
    const height = Math.max(1, Math.floor(options.height || this.gl.canvas?.height || this.height));
    this.setSize(width, height);

    const passes = options.passes || this.passes.filter((pass) => pass.enabled !== false);
    const context = {
      gl: this.gl,
      quad: this.quad,
      width,
      height,
      pipeline: this,
      focusState: options.focusState || null,
      camera: options.camera || null,
      cameraDepthTexture: options.cameraDepthTexture || options.depthTexture || options.cameraDepthTarget?.texture || options.depthTarget?.texture || null,
      cameraDepthEncoding: options.cameraDepthEncoding || options.depthEncoding || null,
      cameraDepthTarget: options.cameraDepthTarget || options.depthTarget || null,
    };
    if (!passes.length) {
      this.copyPass.render(context, sourceTexture, options.target || null);
      return options.target || null;
    }

    let inputTexture = sourceTexture;
    let usePing = true;
    for (let index = 0; index < passes.length; index++) {
      const isLast = index === passes.length - 1;
      const outputTarget = isLast ? (options.target || null) : (usePing ? this.pingTarget : this.pongTarget);
      passes[index].render({ ...context, passIndex: index }, inputTexture, outputTarget);
      if (!isLast) {
        inputTexture = outputTarget.texture;
        usePing = !usePing;
      }
    }
    return options.target || null;
  }

  renderTexture(sourceTexture, options = {}) {
    return this.processTexture(sourceTexture, options);
  }

  dispose() {
    for (const pass of this.passes) {
      if (pass.dispose) pass.dispose();
    }
    this.copyPass.dispose();
    this.sceneTarget.dispose();
    this.pingTarget.dispose();
    this.pongTarget.dispose();
    if (this.ownsQuad) this.quad.dispose();
  }
}

function resolveDepthTextureConfig(gl) {
  const isWebGL2 = isWebGL2Context(gl);
  if (isWebGL2) {
    return {
      internalFormat: gl.DEPTH_COMPONENT16,
      format: gl.DEPTH_COMPONENT,
      type: gl.UNSIGNED_SHORT,
    };
  }
  const extension = gl.getExtension("WEBGL_depth_texture");
  if (!extension) return null;
  return {
    internalFormat: gl.DEPTH_COMPONENT,
    format: gl.DEPTH_COMPONENT,
    type: gl.UNSIGNED_SHORT,
  };
}

function isWebGL2Context(gl) {
  if (typeof WebGL2RenderingContext !== "undefined" && gl instanceof WebGL2RenderingContext) return true;
  const version = gl.getParameter(gl.VERSION);
  return typeof version === "string" && version.includes("WebGL 2");
}

export function bindOutputTarget(gl, target, width, height) {
  if (target && target.framebuffer) {
    gl.bindFramebuffer(gl.FRAMEBUFFER, target.framebuffer);
    gl.viewport(0, 0, target.width || width, target.height || height);
  } else {
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
    gl.viewport(0, 0, width, height);
  }
}

export function createProgram(gl, vertexSource, fragmentSource) {
  const vertex = compileShader(gl, gl.VERTEX_SHADER, vertexSource);
  const fragment = compileShader(gl, gl.FRAGMENT_SHADER, fragmentSource);
  const program = gl.createProgram();
  gl.attachShader(program, vertex);
  gl.attachShader(program, fragment);
  gl.linkProgram(program);
  gl.deleteShader(vertex);
  gl.deleteShader(fragment);
  if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
    const message = gl.getProgramInfoLog(program);
    gl.deleteProgram(program);
    throw new Error(message || "Unable to link native postprocess program");
  }
  return program;
}

export function compileShader(gl, type, source) {
  const shader = gl.createShader(type);
  gl.shaderSource(shader, source);
  gl.compileShader(shader);
  if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
    const message = gl.getShaderInfoLog(shader);
    gl.deleteShader(shader);
    throw new Error(message || "Unable to compile native postprocess shader");
  }
  return shader;
}

function collectUniformLocations(gl, program) {
  const uniforms = {};
  const count = gl.getProgramParameter(program, gl.ACTIVE_UNIFORMS);
  for (let index = 0; index < count; index++) {
    const info = gl.getActiveUniform(program, index);
    if (!info) continue;
    const name = info.name.replace(/\[0\]$/, "");
    uniforms[name] = gl.getUniformLocation(program, name);
  }
  return uniforms;
}

function setAutoUniforms(gl, uniforms, width, height) {
  setUniformValue(gl, uniforms.uResolution, [width, height]);
  setUniformValue(gl, uniforms.uTexelSize, [1 / width, 1 / height]);
  setUniformValue(gl, uniforms.uTextureSize, [width, height]);
}

function setUniformValue(gl, location, value, hint) {
  if (!location || value === undefined || value === null) return;
  if (hint === "int") {
    gl.uniform1i(location, value);
    return;
  }
  if (typeof value === "number") {
    gl.uniform1f(location, value);
    return;
  }
  if (typeof value === "boolean") {
    gl.uniform1i(location, value ? 1 : 0);
    return;
  }
  if (Array.isArray(value) || value instanceof Float32Array || value instanceof Int32Array) {
    if (value.length === 2) gl.uniform2f(location, value[0], value[1]);
    else if (value.length === 3) gl.uniform3f(location, value[0], value[1], value[2]);
    else if (value.length === 4) gl.uniform4f(location, value[0], value[1], value[2], value[3]);
    else if (value.length === 9) gl.uniformMatrix3fv(location, false, value);
    else if (value.length === 16) gl.uniformMatrix4fv(location, false, value);
    return;
  }
  if (typeof value === "object" && "x" in value && "y" in value) {
    gl.uniform2f(location, value.x, value.y);
  }
}
