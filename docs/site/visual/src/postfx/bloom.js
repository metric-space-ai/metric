import { FullscreenQuadPass, RenderTarget } from "../native-postprocess/index.js";
import {
  BLOOM_COMPOSITE_FRAGMENT_SHADER,
  HIGHLIGHT_EXTRACT_FRAGMENT_SHADER,
  SEPARABLE_BLUR_FRAGMENT_SHADER,
} from "./shaders.js";

export const RESTRAINED_HIGHLIGHT_BLOOM = Object.freeze({
  enabled: true,
  threshold: 0.78,
  knee: 0.16,
  intensity: 0.18,
  radius: 1.15,
  iterations: 3,
  downsample: 2,
  saturation: 0.72,
  warmth: 0.1,
  clamp: 0.8,
  sourceIntensity: 1,
});

export class HighlightBloomPass {
  constructor(gl, options = {}) {
    this.gl = gl;
    this.enabled = options.enabled !== false;
    this.width = Math.max(1, Math.floor(options.width || gl.canvas?.width || 1));
    this.height = Math.max(1, Math.floor(options.height || gl.canvas?.height || 1));
    this.settings = normalizeBloomOptions({ ...RESTRAINED_HIGHLIGHT_BLOOM, ...options });
    this.targets = createBloomTargets(gl, this.width, this.height, this.settings.downsample);

    this.extractPass = new FullscreenQuadPass(gl, {
      fragmentShader: HIGHLIGHT_EXTRACT_FRAGMENT_SHADER,
    });
    this.blurPass = new FullscreenQuadPass(gl, {
      fragmentShader: SEPARABLE_BLUR_FRAGMENT_SHADER,
    });
    this.compositePass = new FullscreenQuadPass(gl, {
      fragmentShader: BLOOM_COMPOSITE_FRAGMENT_SHADER,
      beforeRender: (context, pass) => {
        const bloomTexture = context.bloomTexture || this.targets.blurA.texture;
        gl.activeTexture(gl.TEXTURE1);
        gl.bindTexture(gl.TEXTURE_2D, bloomTexture);
        gl.uniform1i(pass.uniforms.uBloomTexture, 1);
      },
    });

    this.setSize(this.width, this.height);
    this.updateUniforms();
  }

  setSize(width, height) {
    this.width = Math.max(1, Math.floor(width));
    this.height = Math.max(1, Math.floor(height));
    const bloomSize = bloomDimensions(this.width, this.height, this.settings.downsample);
    for (const target of Object.values(this.targets)) {
      target.setSize(bloomSize.width, bloomSize.height);
    }
    this.extractPass.setSize(bloomSize.width, bloomSize.height);
    this.blurPass.setSize(bloomSize.width, bloomSize.height);
    this.compositePass.setSize(this.width, this.height);
  }

  setBloom(options = {}) {
    const previousDownsample = this.settings.downsample;
    if ("enabled" in options) this.enabled = options.enabled !== false;
    this.settings = normalizeBloomOptions({ ...this.settings, ...options });
    if (this.settings.downsample !== previousDownsample) this.setSize(this.width, this.height);
    this.updateUniforms();
    return this;
  }

  setIntensity(value) {
    this.settings.intensity = clampNumber(value, 0, 1.25, this.settings.intensity);
    this.compositePass.setUniform("uBloomIntensity", this.settings.intensity);
    return this;
  }

  setThreshold(value) {
    this.settings.threshold = clampNumber(value, 0, 4, this.settings.threshold);
    this.extractPass.setUniform("uThreshold", this.settings.threshold);
    return this;
  }

  render(context, inputTexture, outputTarget = null) {
    const width = Math.max(1, Math.floor(context.width || this.width));
    const height = Math.max(1, Math.floor(context.height || this.height));
    if (width !== this.width || height !== this.height) this.setSize(width, height);

    const bloomSize = bloomDimensions(this.width, this.height, this.settings.downsample);
    const bloomContext = { ...context, width: bloomSize.width, height: bloomSize.height };
    this.updateUniforms();

    this.extractPass.render(bloomContext, inputTexture, this.targets.extract);

    let sourceTexture = this.targets.extract.texture;
    for (let index = 0; index < this.settings.iterations; index++) {
      const radius = this.settings.radius * (1 + index * 0.55);
      this.blurPass
        .setUniform("uDirection", [1, 0])
        .setUniform("uRadius", radius);
      this.blurPass.render(bloomContext, sourceTexture, this.targets.blurB);

      this.blurPass
        .setUniform("uDirection", [0, 1])
        .setUniform("uRadius", radius);
      this.blurPass.render(bloomContext, this.targets.blurB.texture, this.targets.blurA);
      sourceTexture = this.targets.blurA.texture;
    }

    this.compositePass.render({ ...context, bloomTexture: sourceTexture }, inputTexture, outputTarget);
  }

  dispose() {
    this.extractPass.dispose();
    this.blurPass.dispose();
    this.compositePass.dispose();
    for (const target of Object.values(this.targets)) target.dispose();
  }

  updateUniforms() {
    this.extractPass
      .setUniform("uThreshold", this.settings.threshold)
      .setUniform("uKnee", this.settings.knee)
      .setUniform("uSourceIntensity", this.settings.sourceIntensity);

    this.blurPass
      .setUniform("uDirection", [1, 0])
      .setUniform("uRadius", this.settings.radius);

    this.compositePass
      .setUniform("uBloomIntensity", this.settings.intensity)
      .setUniform("uBloomSaturation", this.settings.saturation)
      .setUniform("uBloomWarmth", this.settings.warmth)
      .setUniform("uBloomClamp", this.settings.clamp);
  }
}

export function createRestrainedHighlightBloom(overrides = {}) {
  return normalizeBloomOptions({ ...RESTRAINED_HIGHLIGHT_BLOOM, ...overrides });
}

function createBloomTargets(gl, width, height, downsample) {
  const bloomSize = bloomDimensions(width, height, downsample);
  return {
    extract: new RenderTarget(gl, bloomSize.width, bloomSize.height),
    blurA: new RenderTarget(gl, bloomSize.width, bloomSize.height),
    blurB: new RenderTarget(gl, bloomSize.width, bloomSize.height),
  };
}

function bloomDimensions(width, height, downsample) {
  const scale = Math.max(1, Math.floor(downsample));
  return {
    width: Math.max(1, Math.floor(width / scale)),
    height: Math.max(1, Math.floor(height / scale)),
  };
}

function normalizeBloomOptions(options) {
  return {
    threshold: clampNumber(options.threshold, 0, 4, 0.78),
    knee: clampNumber(options.knee, 0.001, 2, 0.16),
    intensity: clampNumber(options.intensity, 0, 1.25, 0.18),
    radius: clampNumber(options.radius, 0.05, 8, 1.15),
    iterations: Math.max(1, Math.min(8, Math.floor(Number(options.iterations) || 3))),
    downsample: Math.max(1, Math.min(8, Math.floor(Number(options.downsample) || 2))),
    saturation: clampNumber(options.saturation, 0, 2, 0.72),
    warmth: clampNumber(options.warmth, -1, 1, 0.1),
    clamp: clampNumber(options.clamp, 0.05, 4, 0.8),
    sourceIntensity: clampNumber(options.sourceIntensity, 0, 4, 1),
  };
}

function clampNumber(value, min, max, fallback) {
  const number = Number(value);
  if (!Number.isFinite(number)) return fallback;
  return Math.min(max, Math.max(min, number));
}
