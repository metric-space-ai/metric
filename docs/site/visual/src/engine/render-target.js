export class RenderTarget {
  constructor(gl, width = 1, height = 1, options = {}) {
    this.gl = gl;
    this.label = options.label || "render-target";
    this.width = 1;
    this.height = 1;
    this.format = options.format || gl.RGBA;
    this.internalFormat = options.internalFormat || this.format;
    this.type = options.type || gl.UNSIGNED_BYTE;
    this.minFilter = options.minFilter || gl.LINEAR;
    this.magFilter = options.magFilter || gl.LINEAR;
    this.wrapS = options.wrapS || gl.CLAMP_TO_EDGE;
    this.wrapT = options.wrapT || gl.CLAMP_TO_EDGE;
    this.depth = Boolean(options.depth);
    this.stencil = Boolean(options.stencil);
    this.texture = gl.createTexture();
    this.framebuffer = gl.createFramebuffer();
    this.depthBuffer = this.depth ? gl.createRenderbuffer() : null;
    this.disposed = false;

    if (!this.texture || !this.framebuffer || (this.depth && !this.depthBuffer)) {
      this.dispose();
      throw new Error(`Unable to allocate ${this.label}.`);
    }

    this.setSize(width, height);
  }

  setSize(width, height) {
    const nextWidth = Math.max(1, Math.floor(width));
    const nextHeight = Math.max(1, Math.floor(height));
    const gl = this.gl;
    const maxSize = gl.getParameter(gl.MAX_RENDERBUFFER_SIZE);
    if (nextWidth > maxSize || nextHeight > maxSize) {
      throw new Error(`${this.label} exceeds MAX_RENDERBUFFER_SIZE (${maxSize}).`);
    }

    this.width = nextWidth;
    this.height = nextHeight;

    gl.bindTexture(gl.TEXTURE_2D, this.texture);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, this.minFilter);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, this.magFilter);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, this.wrapS);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, this.wrapT);
    gl.texImage2D(
      gl.TEXTURE_2D,
      0,
      this.internalFormat,
      nextWidth,
      nextHeight,
      0,
      this.format,
      this.type,
      null,
    );

    gl.bindFramebuffer(gl.FRAMEBUFFER, this.framebuffer);
    gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, this.texture, 0);

    if (this.depthBuffer) {
      gl.bindRenderbuffer(gl.RENDERBUFFER, this.depthBuffer);
      if (this.stencil) {
        gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_STENCIL, nextWidth, nextHeight);
        gl.framebufferRenderbuffer(
          gl.FRAMEBUFFER,
          gl.DEPTH_STENCIL_ATTACHMENT,
          gl.RENDERBUFFER,
          this.depthBuffer,
        );
      } else {
        gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, nextWidth, nextHeight);
        gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.RENDERBUFFER, this.depthBuffer);
      }
    }

    assertFramebufferComplete(gl, this.label);
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
    gl.bindTexture(gl.TEXTURE_2D, null);
    gl.bindRenderbuffer(gl.RENDERBUFFER, null);
    return this;
  }

  bind() {
    const gl = this.gl;
    gl.bindFramebuffer(gl.FRAMEBUFFER, this.framebuffer);
    gl.viewport(0, 0, this.width, this.height);
    return this;
  }

  dispose() {
    if (this.disposed) return;
    const gl = this.gl;
    if (this.depthBuffer) gl.deleteRenderbuffer(this.depthBuffer);
    if (this.framebuffer) gl.deleteFramebuffer(this.framebuffer);
    if (this.texture) gl.deleteTexture(this.texture);
    this.depthBuffer = null;
    this.framebuffer = null;
    this.texture = null;
    this.disposed = true;
  }
}

export class RenderTargetPair {
  constructor(gl, width = 1, height = 1, options = {}) {
    this.targets = [
      new RenderTarget(gl, width, height, { ...options, label: `${options.label || "target"}-a` }),
      new RenderTarget(gl, width, height, { ...options, label: `${options.label || "target"}-b` }),
    ];
    this.readIndex = 0;
  }

  get read() {
    return this.targets[this.readIndex];
  }

  get write() {
    return this.targets[1 - this.readIndex];
  }

  get width() {
    return this.read.width;
  }

  get height() {
    return this.read.height;
  }

  swap() {
    this.readIndex = 1 - this.readIndex;
    return this;
  }

  setSize(width, height) {
    this.targets[0].setSize(width, height);
    this.targets[1].setSize(width, height);
    return this;
  }

  dispose() {
    this.targets[0].dispose();
    this.targets[1].dispose();
  }
}

export function bindRenderTarget(gl, target, width, height) {
  if (target) {
    target.bind();
  } else {
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
    gl.viewport(0, 0, Math.max(1, width), Math.max(1, height));
  }
}

export function assertFramebufferComplete(gl, label = "framebuffer") {
  const status = gl.checkFramebufferStatus(gl.FRAMEBUFFER);
  if (status !== gl.FRAMEBUFFER_COMPLETE) {
    throw new Error(`${label} is incomplete: ${framebufferStatusName(gl, status)}.`);
  }
}

function framebufferStatusName(gl, status) {
  switch (status) {
    case gl.FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
      return "FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
    case gl.FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
      return "FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
    case gl.FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
      return "FRAMEBUFFER_INCOMPLETE_DIMENSIONS";
    case gl.FRAMEBUFFER_UNSUPPORTED:
      return "FRAMEBUFFER_UNSUPPORTED";
    default:
      return `0x${status.toString(16)}`;
  }
}
