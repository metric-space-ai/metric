import { CopyPass, RenderTarget } from "../native-postprocess/index.js";
import { HighlightBloomPass } from "./bloom.js";
import { ColorGradePass } from "./color-grade.js";
import { FilmGrainPass } from "./film-grain.js";
import { MiniatureCameraDofPass } from "./spatial-focus.js";
import { MiniatureFramePass } from "./miniature-frame.js";
import { VignettePass } from "./vignette.js";

const DEFAULT_METRIC_POSTFX_ORDER = Object.freeze([
  "cameraDof",
  "bloom",
  "miniatureFrame",
  "colorGrade",
  "vignette",
  "filmGrain",
]);

export class MetricPostFxStack {
  constructor(gl, options = {}) {
    this.gl = gl;
    this.enabled = options.enabled !== false;
    this.width = Math.max(1, Math.floor(options.width || gl.canvas?.width || 1));
    this.height = Math.max(1, Math.floor(options.height || gl.canvas?.height || 1));
    this.copyPass = new CopyPass(gl);
    this.pingTarget = new RenderTarget(gl, this.width, this.height);
    this.pongTarget = new RenderTarget(gl, this.width, this.height);
    this.sizeInitialized = false;

    this.bloom = new HighlightBloomPass(gl, {
      ...(options.bloom || {}),
      enabled: options.bloom?.enabled !== false,
      width: this.width,
      height: this.height,
    });
    this.cameraDof = new MiniatureCameraDofPass(gl, {
      ...(options.cameraDof || options.spatialFocus || {}),
      enabled: (options.cameraDof || options.spatialFocus)?.enabled === true,
    });
    this.miniatureFrame = new MiniatureFramePass(gl, {
      ...(options.miniatureFrame || {}),
      enabled: options.miniatureFrame?.enabled !== false,
    });
    this.colorGrade = new ColorGradePass(gl, {
      ...(options.grade || {}),
      enabled: options.grade?.enabled !== false,
    });
    this.vignette = new VignettePass(gl, {
      ...(options.vignette || {}),
      enabled: options.vignette?.enabled !== false,
    });
    this.filmGrain = new FilmGrainPass(gl, {
      ...(options.filmGrain || {}),
      enabled: options.filmGrain?.enabled === true,
    });

    this.focusState = null;
    // Photographic pass contract: native compatibility passes have already run;
    // depth-aware camera DoF consumes the scene depth texture before grade/frame work.
    this.order = DEFAULT_METRIC_POSTFX_ORDER.slice();
    this.setSize(this.width, this.height);
  }

  setSize(width, height) {
    const nextWidth = Math.max(1, Math.floor(width));
    const nextHeight = Math.max(1, Math.floor(height));
    if (this.sizeInitialized && this.width === nextWidth && this.height === nextHeight) return;
    this.width = nextWidth;
    this.height = nextHeight;
    this.sizeInitialized = true;
    this.copyPass.setSize(nextWidth, nextHeight);
    this.pingTarget.setSize(nextWidth, nextHeight);
    this.pongTarget.setSize(nextWidth, nextHeight);
    for (const pass of this.passes()) {
      if (pass.setSize) pass.setSize(nextWidth, nextHeight);
    }
  }

  passes() {
    return this.order.map((name) => this[name]).filter(Boolean);
  }

  enabledPasses() {
    return this.passes().filter((pass) => pass.enabled !== false);
  }

  setOptions(options = {}) {
    if ("enabled" in options) this.enabled = options.enabled !== false;
    if (options.cameraDof || options.spatialFocus) this.cameraDof.setDof(options.cameraDof || options.spatialFocus);
    if (options.bloom) this.bloom.setBloom(options.bloom);
    if (options.miniatureFrame) this.miniatureFrame.setFrame(options.miniatureFrame);
    if (options.grade) this.colorGrade.setGrade(options.grade);
    if (options.vignette) this.vignette.setVignette(options.vignette);
    if (options.filmGrain) this.filmGrain.setGrain(options.filmGrain);
    if (options.order) this.setOrder(options.order);
    return this;
  }

  setOrder(order) {
    const allowed = new Set(["cameraDof", "bloom", "miniatureFrame", "colorGrade", "vignette", "filmGrain"]);
    const next = [];
    for (const name of order) {
      if (allowed.has(name) && !next.includes(name)) next.push(name);
    }
    for (const name of this.order) {
      if (!next.includes(name)) next.push(name);
    }
    this.order = next;
    return this;
  }

  setTime(seconds) {
    this.filmGrain.setTime(seconds);
    return this;
  }

  setFocusState(focusState) {
    this.focusState = focusState || null;
    if (this.miniatureFrame && typeof this.miniatureFrame.setFocusState === "function") {
      this.miniatureFrame.setFocusState(focusState);
    }
    return this;
  }

  render(context, inputTexture, outputTarget = null) {
    const width = Math.max(1, Math.floor(context.width || this.width));
    const height = Math.max(1, Math.floor(context.height || this.height));
    if (width !== this.width || height !== this.height) this.setSize(width, height);

    const passes = this.enabledPasses();
    if (!passes.length) {
      this.copyPass.render(context, inputTexture, outputTarget);
      return;
    }

    let input = inputTexture;
    let usePing = true;
    for (let index = 0; index < passes.length; index++) {
      const isLast = index === passes.length - 1;
      const target = isLast ? outputTarget : (usePing ? this.pingTarget : this.pongTarget);
      passes[index].render({
        ...context,
        width,
        height,
        focusState: context.focusState || this.focusState,
        stack: this,
        stackPassIndex: index,
      }, input, target);
      if (!isLast) {
        input = target.texture;
        usePing = !usePing;
      }
    }
  }

  dispose() {
    for (const pass of this.passes()) {
      if (pass.dispose) pass.dispose();
    }
    this.copyPass.dispose();
    this.pingTarget.dispose();
    this.pongTarget.dispose();
  }
}

export function createMetricPostFxStack(gl, options = {}) {
  return new MetricPostFxStack(gl, options);
}
