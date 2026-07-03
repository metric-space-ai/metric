import { bindRenderTarget } from "./render-target.js";

export class RenderPass {
  constructor(options = {}) {
    this.name = options.name || "render-pass";
    this.enabled = options.enabled !== false;
    this.clear = options.clear !== false;
    this.clearColor = options.clearColor || null;
    this.clearDepth = options.clearDepth !== false;
    this.target = options.target || null;
    this.renderToScreen = Boolean(options.renderToScreen);
    this.onRender = options.render || null;
    this.width = 1;
    this.height = 1;
  }

  setSize(width, height) {
    this.width = Math.max(1, Math.floor(width));
    this.height = Math.max(1, Math.floor(height));
    if (this.target && typeof this.target.setSize === "function") {
      this.target.setSize(this.width, this.height);
    }
    return this;
  }

  bindOutput(context) {
    const target = this.renderToScreen ? null : this.target;
    bindRenderTarget(context.gl, target, context.size.drawingBufferWidth, context.size.drawingBufferHeight);
    if (this.clear) {
      clearCurrentTarget(context.gl, this.clearColor || context.scene.backgroundColor, this.clearDepth);
    }
    return target;
  }

  render(context, input) {
    const target = this.bindOutput(context);
    if (this.onRender) {
      return this.onRender(context, input, target);
    }
    return target;
  }

  dispose() {}
}

export class SceneRenderPass extends RenderPass {
  constructor(options = {}) {
    super({
      name: "scene-render-pass",
      ...options,
    });
  }

  render(context) {
    const target = this.bindOutput(context);
    context.scene.render(context);
    return target;
  }
}

export class PostProcessPipeline {
  constructor(options = {}) {
    this.passes = [];
    this.enabled = options.enabled !== false;
    this.width = 1;
    this.height = 1;
    if (options.passes) {
      for (const pass of options.passes) this.addPass(pass);
    }
  }

  get length() {
    return this.passes.length;
  }

  addPass(pass) {
    this.passes.push(pass);
    if (typeof pass.setSize === "function") {
      pass.setSize(this.width, this.height);
    }
    return pass;
  }

  removePass(pass) {
    const index = this.passes.indexOf(pass);
    if (index !== -1) {
      this.passes.splice(index, 1);
    }
    return pass;
  }

  clear({ dispose = false } = {}) {
    const passes = this.passes.slice();
    this.passes.length = 0;
    if (dispose) {
      for (const pass of passes) {
        if (typeof pass.dispose === "function") pass.dispose();
      }
    }
    return this;
  }

  setSize(width, height) {
    this.width = Math.max(1, Math.floor(width));
    this.height = Math.max(1, Math.floor(height));
    for (const pass of this.passes) {
      if (typeof pass.setSize === "function") {
        pass.setSize(this.width, this.height);
      }
    }
    return this;
  }

  render(context, input = null) {
    if (!this.enabled) return input;
    let current = input;
    for (const pass of this.passes) {
      if (!pass || pass.enabled === false) continue;
      context.pass = pass;
      const result = pass.render(context, current);
      if (result !== undefined) {
        current = result;
      }
    }
    context.pass = null;
    return current;
  }

  dispose() {
    this.clear({ dispose: true });
  }
}

export function clearCurrentTarget(gl, color, clearDepth = true) {
  gl.clearColor(color[0], color[1], color[2], color[3]);
  let mask = gl.COLOR_BUFFER_BIT;
  if (clearDepth) {
    gl.clearDepth(1);
    mask |= gl.DEPTH_BUFFER_BIT;
  }
  gl.clear(mask);
}
