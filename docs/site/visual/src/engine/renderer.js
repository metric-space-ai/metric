import { createGLContext, detectGLCapabilities } from "./capabilities.js";
import { VisualCamera } from "./camera.js";
import { PostProcessPipeline, clearCurrentTarget } from "./pass.js";
import { bindRenderTarget, RenderTarget } from "./render-target.js";
import { RenderLoop } from "./render-loop.js";
import { DisposableSet } from "./resources.js";
import { VisualScene } from "./scene.js";

export class VisualRenderer {
  constructor(options = {}) {
    this.options = options;
    this.canvas = resolveCanvas(options);
    this.gl = options.gl || createGLContext(this.canvas, options);
    this.capabilities = options.capabilities || detectGLCapabilities(this.gl);
    this.scene = options.scene || new VisualScene(options.sceneOptions);
    this.camera = options.camera || new VisualCamera(options.cameraOptions);
    this.pipeline = options.pipeline || new PostProcessPipeline();
    this.resources = new DisposableSet();
    this.events = new Map();
    this.loop = new RenderLoop((time) => this.render(time), options.loop || {});
    this.autoClear = options.autoClear !== false;
    this.clearColor = options.clearColor ? new Float32Array(options.clearColor) : null;
    this.pixelRatio = 1;
    this.maxPixelRatio = options.maxPixelRatio || 2;
    this.size = {
      width: 1,
      height: 1,
      drawingBufferWidth: 1,
      drawingBufferHeight: 1,
      pixelRatio: 1,
      aspect: 1,
    };
    this.pointer = {
      x: 0,
      y: 0,
      uvX: 0,
      uvY: 0,
      normalizedX: 0,
      normalizedY: 0,
      buttons: 0,
      primaryDown: false,
      pointerId: null,
      inside: false,
    };
    this.context = {
      renderer: this,
      gl: this.gl,
      capabilities: this.capabilities,
      canvas: this.canvas,
      scene: this.scene,
      camera: this.camera,
      pipeline: this.pipeline,
      size: this.size,
      time: this.loop.time,
      pointer: this.pointer,
      pass: null,
      target: null,
    };

    this._resizeObserver = null;
    this._resizeTarget = null;
    this._queuedRender = false;
    this._disposed = false;
    this._boundResize = () => this.resizeToDisplaySize();
    this._boundPointerMove = (event) => this.updatePointerFromEvent(event);
    this._boundPointerDown = (event) => this.handlePointerDown(event);
    this._boundPointerUp = (event) => this.handlePointerUp(event);
    this._boundPointerLeave = (event) => this.handlePointerLeave(event);

    this.setPixelRatio(options.pixelRatio ?? getDevicePixelRatio(), { resize: false });
    const initialSize = measureInitialSize(this.canvas, options);
    this.setSize(initialSize.width, initialSize.height, { updateStyle: options.updateStyle === true });

    if (options.pointer !== false) {
      this.addPointerListeners();
    }
    if (options.autoResize !== false) {
      this.connectResize(options.resizeTarget);
    }
    if (options.autoStart) {
      this.start();
    }
  }

  mount(container, options = {}) {
    const target = typeof container === "string" && typeof document !== "undefined"
      ? document.querySelector(container)
      : container;
    if (!target || typeof target.appendChild !== "function") {
      throw new Error("VisualRenderer.mount() requires a DOM element or selector.");
    }
    if (options.before) {
      target.insertBefore(this.canvas, options.before);
    } else {
      target.appendChild(this.canvas);
    }
    if (options.resize !== false) {
      this.connectResize(options.resizeTarget || target);
      this.resizeToDisplaySize();
    }
    return this;
  }

  unmount() {
    if (this.canvas.parentNode) {
      this.canvas.parentNode.removeChild(this.canvas);
    }
    return this;
  }

  setScene(scene) {
    this.scene = scene;
    this.context.scene = scene;
    return this;
  }

  setCamera(camera) {
    this.camera = camera;
    this.context.camera = camera;
    this.camera.setViewport(this.size.drawingBufferWidth, this.size.drawingBufferHeight);
    return this;
  }

  setPipeline(pipeline) {
    this.pipeline = pipeline;
    this.context.pipeline = pipeline;
    this.pipeline.setSize(this.size.drawingBufferWidth, this.size.drawingBufferHeight);
    return this;
  }

  setPixelRatio(pixelRatio, options = {}) {
    this.pixelRatio = Math.max(1, Math.min(this.maxPixelRatio, Number(pixelRatio) || 1));
    if (options.resize !== false) {
      this.setSize(this.size.width, this.size.height);
    }
    return this;
  }

  setSize(width, height, options = {}) {
    const cssWidth = Math.max(1, Math.floor(width || 1));
    const cssHeight = Math.max(1, Math.floor(height || 1));
    const drawingBufferWidth = Math.max(1, Math.floor(cssWidth * this.pixelRatio));
    const drawingBufferHeight = Math.max(1, Math.floor(cssHeight * this.pixelRatio));

    this.size.width = cssWidth;
    this.size.height = cssHeight;
    this.size.drawingBufferWidth = drawingBufferWidth;
    this.size.drawingBufferHeight = drawingBufferHeight;
    this.size.pixelRatio = this.pixelRatio;
    this.size.aspect = drawingBufferWidth / drawingBufferHeight;

    if (this.canvas.width !== drawingBufferWidth) {
      this.canvas.width = drawingBufferWidth;
    }
    if (this.canvas.height !== drawingBufferHeight) {
      this.canvas.height = drawingBufferHeight;
    }
    if (options.updateStyle && this.canvas.style) {
      this.canvas.style.width = `${cssWidth}px`;
      this.canvas.style.height = `${cssHeight}px`;
    }

    this.gl.viewport(0, 0, drawingBufferWidth, drawingBufferHeight);
    this.camera.setViewport(drawingBufferWidth, drawingBufferHeight);
    this.pipeline.setSize(drawingBufferWidth, drawingBufferHeight);
    this.resources.resize(drawingBufferWidth, drawingBufferHeight);
    this.emit("resize", { renderer: this, size: this.size });
    return this;
  }

  resizeToDisplaySize() {
    const measured = measureResizeTarget(this.canvas, this._resizeTarget);
    return this.setSize(measured.width, measured.height);
  }

  connectResize(target = null) {
    this.disconnectResize();
    this._resizeTarget = resolveResizeTarget(this.canvas, target);

    if (isElement(this._resizeTarget) && typeof globalThis.ResizeObserver === "function") {
      this._resizeObserver = new globalThis.ResizeObserver(this._boundResize);
      this._resizeObserver.observe(this._resizeTarget);
    } else if (typeof globalThis.addEventListener === "function") {
      globalThis.addEventListener("resize", this._boundResize);
    }

    return this;
  }

  disconnectResize() {
    if (this._resizeObserver) {
      this._resizeObserver.disconnect();
      this._resizeObserver = null;
    }
    if (typeof globalThis.removeEventListener === "function") {
      globalThis.removeEventListener("resize", this._boundResize);
    }
    return this;
  }

  addPointerListeners() {
    this.canvas.addEventListener("pointermove", this._boundPointerMove);
    this.canvas.addEventListener("pointerdown", this._boundPointerDown);
    this.canvas.addEventListener("pointerup", this._boundPointerUp);
    this.canvas.addEventListener("pointercancel", this._boundPointerUp);
    this.canvas.addEventListener("pointerleave", this._boundPointerLeave);
    return this;
  }

  removePointerListeners() {
    this.canvas.removeEventListener("pointermove", this._boundPointerMove);
    this.canvas.removeEventListener("pointerdown", this._boundPointerDown);
    this.canvas.removeEventListener("pointerup", this._boundPointerUp);
    this.canvas.removeEventListener("pointercancel", this._boundPointerUp);
    this.canvas.removeEventListener("pointerleave", this._boundPointerLeave);
    return this;
  }

  updatePointerFromEvent(event) {
    const rect = getCanvasRect(this.canvas);
    const width = Math.max(1, rect.width);
    const height = Math.max(1, rect.height);
    this.pointer.x = event.clientX - rect.left;
    this.pointer.y = event.clientY - rect.top;
    this.pointer.uvX = this.pointer.x / width;
    this.pointer.uvY = this.pointer.y / height;
    this.pointer.normalizedX = this.pointer.uvX * 2 - 1;
    this.pointer.normalizedY = 1 - this.pointer.uvY * 2;
    this.pointer.buttons = event.buttons || 0;
    this.pointer.inside = this.pointer.uvX >= 0 && this.pointer.uvX <= 1
      && this.pointer.uvY >= 0 && this.pointer.uvY <= 1;
    this.emit("pointer", { renderer: this, pointer: this.pointer, sourceEvent: event });
    return this.pointer;
  }

  handlePointerDown(event) {
    this.updatePointerFromEvent(event);
    this.pointer.primaryDown = true;
    this.pointer.pointerId = event.pointerId;
    if (typeof this.canvas.setPointerCapture === "function") {
      this.canvas.setPointerCapture(event.pointerId);
    }
    this.emit("pointerdown", { renderer: this, pointer: this.pointer, sourceEvent: event });
  }

  handlePointerUp(event) {
    this.updatePointerFromEvent(event);
    this.pointer.primaryDown = false;
    if (typeof this.canvas.releasePointerCapture === "function" && this.pointer.pointerId != null) {
      this.canvas.releasePointerCapture(this.pointer.pointerId);
    }
    this.pointer.pointerId = null;
    this.emit("pointerup", { renderer: this, pointer: this.pointer, sourceEvent: event });
  }

  handlePointerLeave(event) {
    this.updatePointerFromEvent(event);
    this.pointer.inside = false;
    this.emit("pointerleave", { renderer: this, pointer: this.pointer, sourceEvent: event });
  }

  on(type, listener) {
    if (!this.events.has(type)) {
      this.events.set(type, new Set());
    }
    this.events.get(type).add(listener);
    return () => this.off(type, listener);
  }

  off(type, listener) {
    const listeners = this.events.get(type);
    if (listeners) {
      listeners.delete(listener);
    }
    return this;
  }

  emit(type, payload) {
    const listeners = this.events.get(type);
    if (!listeners) return this;
    for (const listener of listeners) {
      listener(payload);
    }
    return this;
  }

  start() {
    this.loop.start();
    return this;
  }

  stop() {
    this.loop.stop();
    return this;
  }

  requestRender() {
    if (this.loop.running || this._queuedRender) return this;
    this._queuedRender = true;
    this.loop.requestFrame((time) => {
      this._queuedRender = false;
      this.render(time);
    });
    return this;
  }

  render(time = this.loop.now()) {
    if (this._disposed) return this;
    const timeState = typeof time === "number" ? this.loop.updateTime(time) : time;
    this.prepareFrame(timeState);
    this.emit("beforeRender", this.context);
    this.scene.update(this.context);

    if (this.pipeline.enabled && this.pipeline.length > 0) {
      this.pipeline.render(this.context, null);
    } else {
      this.renderScene(null);
      this.renderScenePhase("screen-readable-overlay", null, {
        clear: false,
        clearDepth: false,
      });
    }

    this.emit("afterRender", this.context);
    return this;
  }

  prepareFrame(time) {
    this.camera.updateMatrices();
    this.context.scene = this.scene;
    this.context.camera = this.camera;
    this.context.pipeline = this.pipeline;
    this.context.time = time;
    this.context.target = null;
    return this.context;
  }

  renderScene(target = null, options = {}) {
    this.context.target = target;
    const previousRenderPhase = this.context.renderPhase;
    const phase = options.phase || "scene";
    this.context.renderPhase = phase;
    bindRenderTarget(this.gl, target, this.size.drawingBufferWidth, this.size.drawingBufferHeight);
    if (options.clear !== false && this.autoClear) {
      clearCurrentTarget(this.gl, this.clearColor || this.scene.backgroundColor, options.clearDepth !== false);
    }
    this.scene.render(this.context, { phase });
    this.context.renderPhase = previousRenderPhase;
    return this;
  }

  renderScenePhase(phase, target = null, options = {}) {
    return this.renderScene(target, {
      ...options,
      phase,
    });
  }

  clear(color = this.clearColor || this.scene.backgroundColor, target = null, clearDepth = true) {
    bindRenderTarget(this.gl, target, this.size.drawingBufferWidth, this.size.drawingBufferHeight);
    clearCurrentTarget(this.gl, color, clearDepth);
    return this;
  }

  track(resource, options = {}) {
    return this.resources.add(resource, options);
  }

  untrack(resource) {
    return this.resources.delete(resource);
  }

  createRenderTarget(options = {}) {
    const target = new RenderTarget(
      this.gl,
      options.width || this.size.drawingBufferWidth,
      options.height || this.size.drawingBufferHeight,
      options,
    );
    return this.track(target, { resizable: options.resizable !== false });
  }

  dispose(options = {}) {
    if (this._disposed) return;
    this.stop();
    this.disconnectResize();
    this.removePointerListeners();
    if (options.disposePipeline !== false) {
      this.pipeline.dispose();
    }
    if (options.disposeScene !== false) {
      this.scene.dispose();
    }
    this.resources.dispose();
    this.events.clear();
    if (options.loseContext) {
      const loseContext = this.gl.getExtension("WEBGL_lose_context");
      if (loseContext) loseContext.loseContext();
    }
    this._disposed = true;
    return this;
  }
}

function resolveCanvas(options) {
  if (options.canvas) return options.canvas;
  if (options.gl && options.gl.canvas) return options.gl.canvas;
  if (typeof document === "undefined") {
    throw new Error("VisualRenderer requires a canvas when document is unavailable.");
  }
  return document.createElement("canvas");
}

function measureInitialSize(canvas, options) {
  if (options.width && options.height) {
    return { width: options.width, height: options.height };
  }
  return measureResizeTarget(canvas, options.resizeTarget);
}

function resolveResizeTarget(canvas, target) {
  if (target === "window") return globalThis;
  if (target) return target;
  return canvas.parentElement || canvas;
}

function measureResizeTarget(canvas, target) {
  const resolved = resolveResizeTarget(canvas, target);
  if (resolved === globalThis) {
    return {
      width: globalThis.innerWidth || canvas.width || 300,
      height: globalThis.innerHeight || canvas.height || 150,
    };
  }
  if (isElement(resolved)) {
    const rect = typeof resolved.getBoundingClientRect === "function"
      ? resolved.getBoundingClientRect()
      : null;
    return {
      width: Math.floor(rect?.width || resolved.clientWidth || canvas.clientWidth || canvas.width || 300),
      height: Math.floor(rect?.height || resolved.clientHeight || canvas.clientHeight || canvas.height || 150),
    };
  }
  return {
    width: canvas.clientWidth || canvas.width || 300,
    height: canvas.clientHeight || canvas.height || 150,
  };
}

function isElement(value) {
  return value && typeof value === "object" && typeof value.nodeType === "number";
}

function getCanvasRect(canvas) {
  if (typeof canvas.getBoundingClientRect === "function") {
    return canvas.getBoundingClientRect();
  }
  return {
    left: 0,
    top: 0,
    width: canvas.clientWidth || canvas.width || 1,
    height: canvas.clientHeight || canvas.height || 1,
  };
}

function getDevicePixelRatio() {
  return globalThis.devicePixelRatio || 1;
}
