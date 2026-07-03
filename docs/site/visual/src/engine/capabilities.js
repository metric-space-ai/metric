export function createGLContext(canvas, options = {}) {
  if (!canvas || typeof canvas.getContext !== "function") {
    throw new Error("VisualRenderer requires an HTMLCanvasElement with getContext().");
  }

  const attributes = {
    alpha: options.alpha ?? false,
    antialias: options.antialias ?? true,
    depth: options.depth ?? true,
    stencil: options.stencil ?? false,
    premultipliedAlpha: options.premultipliedAlpha ?? true,
    preserveDrawingBuffer: options.preserveDrawingBuffer ?? false,
    powerPreference: options.powerPreference ?? "high-performance",
    ...(options.contextAttributes || {}),
  };

  const preferWebGL2 = options.preferWebGL2 !== false;
  const names = preferWebGL2
    ? ["webgl2", "webgl", "experimental-webgl"]
    : ["webgl", "experimental-webgl", "webgl2"];

  for (const name of names) {
    const gl = canvas.getContext(name, attributes);
    if (gl) {
      return gl;
    }
  }

  throw new Error("WebGL is not available in this browser context.");
}

export function detectGLCapabilities(gl) {
  const isWebGL2 = isWebGL2Context(gl);
  const getExtension = (...names) => {
    for (const name of names) {
      const extension = gl.getExtension(name);
      if (extension) return extension;
    }
    return null;
  };

  const instancingExtension = isWebGL2 ? null : getExtension("ANGLE_instanced_arrays");
  const vertexArrayExtension = isWebGL2 ? null : getExtension("OES_vertex_array_object");
  const anisotropyExtension = getExtension(
    "EXT_texture_filter_anisotropic",
    "MOZ_EXT_texture_filter_anisotropic",
    "WEBKIT_EXT_texture_filter_anisotropic",
  );

  const capabilities = {
    gl,
    isWebGL2,
    version: gl.getParameter(gl.VERSION),
    shadingLanguageVersion: gl.getParameter(gl.SHADING_LANGUAGE_VERSION),
    gpuVendor: gl.getParameter(gl.VENDOR),
    renderer: gl.getParameter(gl.RENDERER),
    limits: {
      maxTextureSize: gl.getParameter(gl.MAX_TEXTURE_SIZE),
      maxCubeMapTextureSize: gl.getParameter(gl.MAX_CUBE_MAP_TEXTURE_SIZE),
      maxRenderbufferSize: gl.getParameter(gl.MAX_RENDERBUFFER_SIZE),
      maxTextureImageUnits: gl.getParameter(gl.MAX_TEXTURE_IMAGE_UNITS),
      maxVertexTextureImageUnits: gl.getParameter(gl.MAX_VERTEX_TEXTURE_IMAGE_UNITS),
      maxCombinedTextureImageUnits: gl.getParameter(gl.MAX_COMBINED_TEXTURE_IMAGE_UNITS),
      maxVertexAttribs: gl.getParameter(gl.MAX_VERTEX_ATTRIBS),
      maxVaryingVectors: gl.getParameter(gl.MAX_VARYING_VECTORS),
      maxViewportDims: gl.getParameter(gl.MAX_VIEWPORT_DIMS),
      maxSamples: isWebGL2 ? gl.getParameter(gl.MAX_SAMPLES) : 0,
      maxAnisotropy: anisotropyExtension
        ? gl.getParameter(anisotropyExtension.MAX_TEXTURE_MAX_ANISOTROPY_EXT)
        : 1,
    },
    extensions: {
      instancing: instancingExtension,
      vertexArray: vertexArrayExtension,
      depthTexture: isWebGL2 ? true : getExtension("WEBGL_depth_texture"),
      elementIndexUint: isWebGL2 ? true : getExtension("OES_element_index_uint"),
      textureFloat: isWebGL2 ? true : getExtension("OES_texture_float"),
      textureFloatLinear: getExtension("OES_texture_float_linear"),
      textureHalfFloat: isWebGL2 ? true : getExtension("OES_texture_half_float"),
      textureHalfFloatLinear: getExtension("OES_texture_half_float_linear"),
      colorBufferFloat: isWebGL2
        ? getExtension("EXT_color_buffer_float")
        : getExtension("WEBGL_color_buffer_float", "EXT_color_buffer_half_float"),
      drawBuffers: isWebGL2 ? true : getExtension("WEBGL_draw_buffers"),
      anisotropy: anisotropyExtension,
    },
  };

  capabilities.instancing = createInstancingFacade(gl, isWebGL2, instancingExtension);
  capabilities.vertexArray = createVertexArrayFacade(gl, isWebGL2, vertexArrayExtension);

  return capabilities;
}

export function isWebGL2Context(gl) {
  if (typeof WebGL2RenderingContext !== "undefined" && gl instanceof WebGL2RenderingContext) {
    return true;
  }
  const version = gl.getParameter(gl.VERSION);
  return typeof version === "string" && version.includes("WebGL 2");
}

export function requireInstancing(capabilities) {
  if (!capabilities.instancing.supported) {
    throw new Error("Instanced rendering requires WebGL2 or ANGLE_instanced_arrays.");
  }
}

function createInstancingFacade(gl, isWebGL2, extension) {
  if (isWebGL2) {
    return {
      supported: true,
      native: true,
      extension: null,
      vertexAttribDivisor: gl.vertexAttribDivisor.bind(gl),
      drawArraysInstanced: gl.drawArraysInstanced.bind(gl),
      drawElementsInstanced: gl.drawElementsInstanced.bind(gl),
    };
  }

  if (extension) {
    return {
      supported: true,
      native: false,
      extension,
      vertexAttribDivisor: extension.vertexAttribDivisorANGLE.bind(extension),
      drawArraysInstanced: extension.drawArraysInstancedANGLE.bind(extension),
      drawElementsInstanced: extension.drawElementsInstancedANGLE.bind(extension),
    };
  }

  return {
    supported: false,
    native: false,
    extension: null,
    vertexAttribDivisor() {
      requireInstancing({ instancing: this });
    },
    drawArraysInstanced() {
      requireInstancing({ instancing: this });
    },
    drawElementsInstanced() {
      requireInstancing({ instancing: this });
    },
  };
}

function createVertexArrayFacade(gl, isWebGL2, extension) {
  if (isWebGL2) {
    return {
      supported: true,
      native: true,
      extension: null,
      create: gl.createVertexArray.bind(gl),
      bind: gl.bindVertexArray.bind(gl),
      delete: gl.deleteVertexArray.bind(gl),
    };
  }

  if (extension) {
    return {
      supported: true,
      native: false,
      extension,
      create: extension.createVertexArrayOES.bind(extension),
      bind: extension.bindVertexArrayOES.bind(extension),
      delete: extension.deleteVertexArrayOES.bind(extension),
    };
  }

  return {
    supported: false,
    native: false,
    extension: null,
    create: () => null,
    bind: () => {},
    delete: () => {},
  };
}
