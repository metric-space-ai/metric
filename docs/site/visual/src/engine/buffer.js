import { requireInstancing } from "./capabilities.js";

export class GLBuffer {
  constructor(gl, options = {}) {
    this.gl = gl;
    this.target = options.target || gl.ARRAY_BUFFER;
    this.usage = options.usage || gl.STATIC_DRAW;
    this.label = options.label || "buffer";
    this.buffer = gl.createBuffer();
    this.byteLength = 0;
    this.version = 0;
    this.disposed = false;

    if (!this.buffer) {
      throw new Error(`Unable to create WebGL ${this.label}.`);
    }
    if (options.data != null) {
      this.setData(options.data, options.usage);
    }
  }

  bind() {
    this.gl.bindBuffer(this.target, this.buffer);
    return this;
  }

  setData(data, usage = this.usage) {
    this.usage = usage;
    this.bind();
    this.gl.bufferData(this.target, data, usage);
    this.byteLength = typeof data === "number" ? data : data.byteLength;
    this.version += 1;
    return this;
  }

  updateSubData(data, offset = 0) {
    this.bind();
    this.gl.bufferSubData(this.target, offset, data);
    this.version += 1;
    return this;
  }

  dispose() {
    if (this.disposed) return;
    this.gl.deleteBuffer(this.buffer);
    this.buffer = null;
    this.disposed = true;
  }
}

export class AttributeBuffer extends GLBuffer {
  constructor(gl, options = {}) {
    const data = options.data;
    super(gl, {
      ...options,
      target: options.target || gl.ARRAY_BUFFER,
      data: null,
    });
    this.itemSize = options.itemSize || 3;
    this.type = options.type || gl.FLOAT;
    this.normalized = Boolean(options.normalized);
    this.stride = options.stride || 0;
    this.offset = options.offset || 0;
    this.divisor = options.divisor || 0;
    this.integer = Boolean(options.integer);
    this.count = options.count || 0;
    if (data != null) {
      this.setData(data, options.usage);
    }
  }

  setData(data, usage = this.usage) {
    super.setData(data, usage);
    this.count = inferAttributeCount(data, this.itemSize, this.stride);
    return this;
  }

  bindToLocation(location, capabilities) {
    if (location == null || location < 0) return this;
    const gl = this.gl;
    this.bind();
    gl.enableVertexAttribArray(location);

    if (this.integer && typeof gl.vertexAttribIPointer === "function") {
      gl.vertexAttribIPointer(location, this.itemSize, this.type, this.stride, this.offset);
    } else {
      gl.vertexAttribPointer(location, this.itemSize, this.type, this.normalized, this.stride, this.offset);
    }

    if (this.divisor) {
      requireInstancing(capabilities);
      capabilities.instancing.vertexAttribDivisor(location, this.divisor);
    } else if (capabilities.instancing.supported) {
      capabilities.instancing.vertexAttribDivisor(location, 0);
    }

    return this;
  }
}

export class IndexBuffer extends GLBuffer {
  constructor(gl, options = {}) {
    const data = options.data;
    super(gl, {
      ...options,
      target: gl.ELEMENT_ARRAY_BUFFER,
      data: null,
    });
    this.type = options.type || inferIndexType(gl, data);
    this.count = options.count || 0;
    if (data != null) {
      this.setData(data, options.usage);
    }
  }

  setData(data, usage = this.usage) {
    super.setData(data, usage);
    this.type = inferIndexType(this.gl, data);
    this.count = data ? data.length : 0;
    return this;
  }
}

export class VertexArray {
  constructor(gl, capabilities) {
    this.gl = gl;
    this.capabilities = capabilities;
    this.handle = capabilities.vertexArray.supported ? capabilities.vertexArray.create() : null;
    this.disposed = false;
  }

  bind() {
    if (this.handle) {
      this.capabilities.vertexArray.bind(this.handle);
    }
    return this;
  }

  unbind() {
    if (this.handle) {
      this.capabilities.vertexArray.bind(null);
    }
    return this;
  }

  capture(setup) {
    this.bind();
    setup();
    this.unbind();
    return this;
  }

  dispose() {
    if (this.disposed) return;
    if (this.handle) {
      this.capabilities.vertexArray.delete(this.handle);
    }
    this.handle = null;
    this.disposed = true;
  }
}

export class Geometry {
  constructor(gl, capabilities, options = {}) {
    this.gl = gl;
    this.capabilities = capabilities;
    this.mode = options.mode || gl.TRIANGLES;
    this.attributes = new Map();
    this.index = null;
    this.vertexCount = options.vertexCount || 0;
    this.instanceCount = options.instanceCount || 0;
    this.label = options.label || "geometry";
    this.disposed = false;
  }

  setAttribute(name, attribute) {
    const buffer = attribute instanceof AttributeBuffer
      ? attribute
      : new AttributeBuffer(this.gl, attribute);
    this.attributes.set(name, buffer);
    if (!buffer.divisor && !this.vertexCount) {
      this.vertexCount = buffer.count;
    }
    return this;
  }

  getAttribute(name) {
    return this.attributes.get(name) || null;
  }

  removeAttribute(name) {
    const attribute = this.attributes.get(name);
    this.attributes.delete(name);
    return attribute;
  }

  setIndex(index) {
    this.index = index instanceof IndexBuffer ? index : new IndexBuffer(this.gl, index);
    return this;
  }

  getIndex() {
    return this.index;
  }

  setVertexCount(count) {
    this.vertexCount = Math.max(0, Math.floor(count));
    return this;
  }

  setInstanceCount(count) {
    this.instanceCount = Math.max(0, Math.floor(count));
    return this;
  }

  bind(program) {
    for (const [name, attribute] of this.attributes) {
      const location = getAttributeLocation(this.gl, program, name);
      attribute.bindToLocation(location, this.capabilities);
    }
    if (this.index) {
      this.index.bind();
    }
    return this;
  }

  draw(program, options = {}) {
    const gl = this.gl;
    const mode = options.mode || this.mode;
    const count = options.count || this.getDrawCount();
    const offset = options.offset || 0;
    const instanceCount = options.instanceCount ?? this.instanceCount;
    this.bind(program);

    if (instanceCount > 0) {
      requireInstancing(this.capabilities);
      if (this.index) {
        ensureUintIndexSupport(this.capabilities, this.index);
        this.capabilities.instancing.drawElementsInstanced(mode, count, this.index.type, offset, instanceCount);
      } else {
        this.capabilities.instancing.drawArraysInstanced(mode, offset, count, instanceCount);
      }
      return this;
    }

    if (this.index) {
      ensureUintIndexSupport(this.capabilities, this.index);
      gl.drawElements(mode, count, this.index.type, offset);
    } else {
      gl.drawArrays(mode, offset, count);
    }
    return this;
  }

  getDrawCount() {
    if (this.index) return this.index.count;
    if (this.vertexCount) return this.vertexCount;
    for (const attribute of this.attributes.values()) {
      if (!attribute.divisor) return attribute.count;
    }
    return 0;
  }

  dispose() {
    if (this.disposed) return;
    for (const attribute of this.attributes.values()) {
      attribute.dispose();
    }
    if (this.index) {
      this.index.dispose();
    }
    this.attributes.clear();
    this.index = null;
    this.disposed = true;
  }
}

function inferAttributeCount(data, itemSize, stride) {
  if (!data || typeof data.length !== "number") return 0;
  if (stride) return Math.floor(data.byteLength / stride);
  return Math.floor(data.length / itemSize);
}

function inferIndexType(gl, data) {
  if (data instanceof Uint32Array) return gl.UNSIGNED_INT;
  if (data instanceof Uint8Array) return gl.UNSIGNED_BYTE;
  return gl.UNSIGNED_SHORT;
}

function ensureUintIndexSupport(capabilities, index) {
  if (index.type === index.gl.UNSIGNED_INT && !capabilities.extensions.elementIndexUint) {
    throw new Error("Uint32 element indices require WebGL2 or OES_element_index_uint.");
  }
}

function getAttributeLocation(gl, program, name) {
  if (!program) return -1;
  if (typeof program.getAttribLocation === "function") {
    return program.getAttribLocation(name);
  }
  const rawProgram = program.program || program;
  return gl.getAttribLocation(rawProgram, name);
}
