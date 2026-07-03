import { detectGLCapabilities } from "../engine/capabilities.js";
import { ShaderProgram } from "../engine/shader.js";

export const IDENTITY_MAT4 = new Float32Array([
  1, 0, 0, 0,
  0, 1, 0, 0,
  0, 0, 1, 0,
  0, 0, 0, 1,
]);

export function resolveRenderContext(rendererOrGl, options = {}) {
  const renderer = rendererOrGl && rendererOrGl.gl ? rendererOrGl : options.renderer || null;
  const gl = renderer?.gl || rendererOrGl?.gl || rendererOrGl || options.gl;
  if (!gl || typeof gl.createBuffer !== "function") {
    throw new Error("Layer rendering requires a WebGL context or VisualRenderer.");
  }
  const capabilities = options.capabilities || renderer?.capabilities || detectGLCapabilities(gl);
  return { renderer, gl, capabilities };
}

export function createProgram(gl, label, vertex, fragment, defines) {
  return new ShaderProgram(gl, { label, vertex, fragment, defines });
}

export function createBuffer(gl, data, usage = gl.STATIC_DRAW) {
  const buffer = gl.createBuffer();
  if (!buffer) throw new Error("Unable to create WebGL buffer.");
  gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
  gl.bufferData(gl.ARRAY_BUFFER, data, usage);
  return buffer;
}

export function updateArrayBuffer(gl, buffer, data, usage = gl.STATIC_DRAW) {
  gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
  gl.bufferData(gl.ARRAY_BUFFER, data, usage);
  return buffer;
}

export function createIndexBuffer(gl, data, usage = gl.STATIC_DRAW) {
  const buffer = gl.createBuffer();
  if (!buffer) throw new Error("Unable to create WebGL index buffer.");
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, buffer);
  gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, data, usage);
  return buffer;
}

export function bindAttribute(gl, program, capabilities, name, buffer, itemSize, options = {}) {
  const location = program.getAttribLocation(name);
  if (location == null || location < 0 || !buffer) return false;
  gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
  gl.enableVertexAttribArray(location);
  gl.vertexAttribPointer(
    location,
    itemSize,
    options.type || gl.FLOAT,
    Boolean(options.normalized),
    options.stride || 0,
    options.offset || 0,
  );
  setAttribDivisor(capabilities, location, options.divisor || 0);
  return true;
}

export function setAttribDivisor(capabilities, location, divisor) {
  if (!capabilities?.instancing?.supported) return;
  capabilities.instancing.vertexAttribDivisor(location, divisor);
}

export function drawArraysInstanced(gl, capabilities, mode, first, vertexCount, instanceCount) {
  if (!instanceCount) return;
  if (!capabilities?.instancing?.supported) {
    throw new Error("Instanced rendering requires WebGL2 or ANGLE_instanced_arrays.");
  }
  capabilities.instancing.drawArraysInstanced(mode, first, vertexCount, instanceCount);
}

export function setCameraUniforms(program, context, fallbackRenderer) {
  const camera = context?.camera || fallbackRenderer?.camera;
  program.setUniform("uViewProjectionMatrix", camera?.viewProjectionMatrix || IDENTITY_MAT4);
  program.setUniform("uViewMatrix", camera?.viewMatrix || IDENTITY_MAT4);
  program.setUniform("uProjectionMatrix", camera?.projectionMatrix || IDENTITY_MAT4);
  program.setUniform("uCameraPosition", camera?.position || [0, 0, 1]);
}

export function frameTimeSeconds(context) {
  const time = context?.time;
  if (typeof time === "number") return time * 0.001;
  if (typeof time?.elapsed === "number") return time.elapsed * 0.001;
  if (typeof time?.elapsedMs === "number") return time.elapsedMs * 0.001;
  if (typeof time?.now === "number") return time.now * 0.001;
  return 0;
}

export function configureDrawState(gl, material = {}, options = {}) {
  if (options.depthTest === false || material.depthTest === false) {
    gl.disable(gl.DEPTH_TEST);
  } else {
    gl.enable(gl.DEPTH_TEST);
  }

  if (options.cullFace === false || material.backFace === "double-sided" || material.cullFace === false) {
    gl.disable(gl.CULL_FACE);
  } else if (options.cullFace === true || material.cullFace === true) {
    gl.enable(gl.CULL_FACE);
    gl.cullFace(gl.BACK);
  }

  if (options.blend || material.alphaMode === "blend" || material.transparent === true) {
    gl.enable(gl.BLEND);
    gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
  } else {
    gl.disable(gl.BLEND);
  }

  gl.depthMask(options.depthWrite !== false && material.depthWrite !== false);
}

export function restoreDepthWrite(gl) {
  gl.depthMask(true);
}

export function pointSizeRange(gl) {
  const range = gl.getParameter(gl.ALIASED_POINT_SIZE_RANGE);
  return range || [1, 64];
}

export function disposeGLResources(gl, resources) {
  if (!gl || !Array.isArray(resources)) return;
  for (let index = resources.length - 1; index >= 0; index -= 1) {
    const resource = resources[index];
    if (!resource) continue;
    if (typeof resource.dispose === "function") {
      resource.dispose();
    } else if (resource.kind === "buffer") {
      gl.deleteBuffer(resource.handle);
    } else if (resource.kind === "texture") {
      gl.deleteTexture(resource.handle);
    } else if (resource.kind === "framebuffer") {
      gl.deleteFramebuffer(resource.handle);
    } else if (resource.kind === "renderbuffer") {
      gl.deleteRenderbuffer(resource.handle);
    }
  }
}

export function trackBuffer(layer, handle) {
  return layer.track({ kind: "buffer", handle });
}

export function safeLineWidth(gl, width) {
  const range = gl.getParameter(gl.ALIASED_LINE_WIDTH_RANGE) || [1, 1];
  const next = Math.max(range[0], Math.min(range[1], Number(width) || 1));
  gl.lineWidth(next);
  return next;
}

