import {
  applyMiniatureSceneBundle,
  createMiniatureSceneBundle,
} from "./composition.js";
import {
  applyMiniatureStyleToDescriptors,
  createMiniatureDescriptorBundle,
} from "./descriptors.js";
import { inspectMiniatureStyleContract } from "./diagnostics.js";
import { createMiniatureStagePreset } from "./presets.js";
import {
  createMiniatureStyleProfile,
  normalizeMiniatureStyleProfile,
} from "./profile.js";
import { createMiniatureRuntimeOptions } from "./runtime-options.js";
import {
  createMiniatureStyleMotionController,
  createMiniatureStyleMotionPreset,
} from "./style-motion.js";

/**
 * Owns the reusable photographic miniature style contract for one visual
 * surface. It does not compute metric data. It coordinates camera, focus,
 * postprocess, material/light presets, ground relation and renderer motion
 * over already-exported METRIC visual evidence.
 */
export class MiniaturePhotographicStyle {
  constructor(options = {}) {
    this.profile = options.profile ? normalizeMiniatureStyleProfile(options.profile) : null;
    this.stageOptions = clonePlainObject(optionValue("stageOptions", options, this.profile, {}));
    this.stage = resolveStage(optionValue("stage", options, this.profile, null), this.stageOptions);
    this.runtime = options.runtime || null;
    this.layerFactory = options.layerFactory || options.layers?.factory || null;
    this.fit = clonePlainObject(optionValue("fit", options, this.profile, {}));
    this.ground = clonePlainObject(optionValue("ground", options, this.profile, {}));
    this.contacts = optionValue("contacts", options, this.profile, undefined);
    this.motion = optionValue("motion", options, this.profile, undefined);
    this.styleMotion = createMiniatureStyleMotionPreset(
      hasOwn(options, "styleMotion")
        ? options.styleMotion
        : hasOwn(options, "stageMotion")
          ? options.stageMotion
          : this.profile?.styleMotion || "none",
    );
    this.styleMotionController = createMiniatureStyleMotionController(this.styleMotion);
    this.metadata = {
      styleLayer: "metric.visual.style.miniature.photographic",
      reusableMiniatureStyle: true,
      ...(this.profile?.metadata || {}),
      ...(options.metadata || {}),
    };
    this.lastBundle = null;
  }

  /**
   * Creates `MetricVisualRuntime` constructor options from the current style.
   *
   * @param {object} [options]
   * @returns {{stage: object, runtimeOptions: object}}
   */
  createRuntimeOptions(options = {}) {
    return createMiniatureRuntimeOptions({
      ...options,
      stage: options.stage || this.stage,
      layerFactory: options.layerFactory || this.layerFactory || options.layers?.factory,
      layers: {
        ...(options.layers || {}),
        factory: options.layerFactory || options.layers?.factory || this.layerFactory || options.layers?.factory,
      },
    });
  }

  /**
   * Applies only material/light/animation style hints to descriptors.
   *
   * @param {object[]|object} descriptors
   * @param {object} [options]
   * @returns {object[]}
   */
  styleDescriptors(descriptors, options = {}) {
    return applyMiniatureStyleToDescriptors(descriptors, {
      ...options,
      stage: options.stage || this.stage,
      motion: options.motion ?? this.motion,
    });
  }

  /**
   * Creates a full descriptor bundle without fitting coordinates.
   *
   * @param {object[]|object} descriptors
   * @param {object} [options]
   * @returns {object}
   */
  createDescriptorBundle(descriptors, options = {}) {
    return createMiniatureDescriptorBundle(descriptors, {
      ...this.defaultBundleOptions(),
      ...options,
      stage: options.stage || this.stage,
      metadata: {
        ...this.metadata,
        ...(options.metadata || {}),
      },
    });
  }

  /**
   * Creates a staged miniature scene bundle for arbitrary METRIC descriptors.
   *
   * @param {object[]|object} descriptors
   * @param {object} [options]
   * @returns {object}
   */
  createSceneBundle(descriptors, options = {}) {
    const bundle = createMiniatureSceneBundle(descriptors, {
      ...this.defaultBundleOptions(),
      ...options,
      stage: options.stage || this.stage,
      fit: mergeOptions(this.fit, options.fit),
      ground: options.ground === false ? false : mergeOptions(this.ground, options.ground),
      contacts: options.contacts ?? this.contacts,
      motion: options.motion ?? this.motion,
      metadata: {
        ...this.metadata,
        ...(options.metadata || {}),
      },
    });
    this.lastBundle = bundle;
    return bundle;
  }

  /**
   * Keeps a runtime reference and optionally pushes the current stage into it.
   *
   * @param {object} runtime
   * @param {object} [options]
   * @returns {MiniaturePhotographicStyle}
   */
  attachRuntime(runtime, options = {}) {
    this.runtime = requireRuntime(runtime);
    if (options.applyStage !== false) {
      this.applyStage(this.runtime, options.stage || this.stage);
    }
    if (options.styleMotion !== false) {
      this.attachStyleMotion(options.styleMotion || this.styleMotion);
    }
    return this;
  }

  /**
   * Applies the current or supplied stage to a runtime.
   *
   * @param {object} [runtime]
   * @param {object} [stage]
   * @returns {object}
   */
  applyStage(runtime = this.runtime, stage = this.stage) {
    const target = requireRuntime(runtime);
    if (typeof target.applyStagePreset === "function") {
      target.applyStagePreset(stage);
    } else {
      if (stage.scene && typeof target.setSceneOptions === "function") target.setSceneOptions(stage.scene);
      if (stage.camera && typeof target.setCameraOptions === "function") target.setCameraOptions(stage.camera);
      if (stage.postprocess && typeof target.setPostprocessOptions === "function") target.setPostprocessOptions(stage.postprocess);
      if (stage.focus && typeof target.setFocusOptions === "function") target.setFocusOptions(stage.focus);
    }
    if (this.styleMotionController) {
      this.styleMotionController.stage = stage;
      this.styleMotionController.resetBase();
    }
    return target;
  }

  /**
   * Applies a staged scene bundle to a runtime.
   *
   * @param {object} [runtime]
   * @param {object} [bundle]
   * @param {object} [options]
   * @returns {object}
   */
  applyBundle(runtime = this.runtime, bundle = this.lastBundle, options = {}) {
    const target = requireRuntime(runtime);
    if (!bundle || typeof bundle !== "object") {
      throw new Error("MiniaturePhotographicStyle.applyBundle() requires a scene bundle.");
    }
    this.lastBundle = bundle;
    return applyMiniatureSceneBundle(target, bundle, {
      source: options.source || "miniature-photographic-style",
    });
  }

  /**
   * Applies stage and, when descriptors or a bundle are supplied, the layer
   * bundle in one call.
   *
   * @param {object} [runtime]
   * @param {object} [options]
   * @returns {object}
   */
  applyToRuntime(runtime = this.runtime, options = {}) {
    const target = requireRuntime(runtime);
    this.runtime = target;
    if (options.stage !== false) {
      this.applyStage(target, options.stage || this.stage);
    }
    const bundle = options.bundle || (options.descriptors
      ? this.createSceneBundle(options.descriptors, options)
      : null);
    if (bundle) {
      this.applyBundle(target, bundle, options);
    }
    return target;
  }

  /**
   * Replaces the stage. Passing a full stage object preserves it; passing plain
   * stage options creates a new stage preset.
   *
   * @param {object} stageOrOptions
   * @param {object} [options]
   * @returns {MiniaturePhotographicStyle}
   */
  setStage(stageOrOptions = {}, options = {}) {
    if (looksLikeStage(stageOrOptions)) {
      this.stage = stageOrOptions;
    } else {
      this.stageOptions = mergeOptions(this.stageOptions, stageOrOptions);
      this.stage = createMiniatureStagePreset(this.stageOptions);
    }
    if (options.apply !== false && this.runtime) {
      this.applyStage(this.runtime, this.stage);
    }
    return this;
  }

  /**
   * Updates the focus contract in the runtime without rebuilding layers.
   *
   * @param {object} focus
   * @param {object} [runtime]
   * @returns {object}
   */
  applyFocus(focus = this.stage.focus, runtime = this.runtime) {
    const target = requireRuntime(runtime);
    if (typeof target.setFocusOptions === "function") {
      target.setFocusOptions(focus, { source: "miniature-photographic-style" });
    }
    return target;
  }

  /**
   * Updates final photographic postprocess without rebuilding layers.
   *
   * @param {object} postprocess
   * @param {object} [runtime]
   * @returns {object}
   */
  applyPostprocess(postprocess = this.stage.postprocess, runtime = this.runtime) {
    const target = requireRuntime(runtime);
    if (typeof target.setPostprocessOptions === "function") {
      target.setPostprocessOptions(postprocess);
    }
    return target;
  }

  setStyleMotion(motion = "none", options = {}) {
    this.styleMotion = createMiniatureStyleMotionPreset(motion);
    this.styleMotionController.setMotion(this.styleMotion);
    if (options.attach !== false && this.runtime) {
      this.attachStyleMotion(this.styleMotion);
    }
    return this;
  }

  attachStyleMotion(motion = this.styleMotion) {
    this.styleMotion = createMiniatureStyleMotionPreset(motion);
    this.styleMotionController.setMotion(this.styleMotion);
    if (this.runtime) {
      this.styleMotionController.attach(this.runtime, this.stage);
    }
    return this;
  }

  detachStyleMotion() {
    this.styleMotionController.detach();
    return this;
  }

  inspectContract(options = {}) {
    return inspectMiniatureStyleContract({
      style: this,
      stage: options.stage || this.stage,
      bundle: options.bundle || this.lastBundle,
      descriptors: options.descriptors,
      runtime: options.runtime || this.runtime,
      styleMotion: this.styleMotionController.toJSON(),
    });
  }

  toProfile(options = {}) {
    const profile = createMiniatureStyleProfile({
      id: options.id || this.profile?.id || this.stage?.id,
      variant: options.variant || this.stage?.variant || this.profile?.variant,
      stageOptions: options.stageOptions ?? this.stageOptions,
      stage: options.stage || this.stage,
      fit: options.fit ?? this.fit,
      ground: options.ground ?? this.ground,
      contacts: options.contacts ?? this.contacts,
      motion: options.motion ?? this.motion,
      styleMotion: options.styleMotion || this.styleMotionController.toJSON(),
      metadata: {
        ...this.metadata,
        ...(options.metadata || {}),
      },
      sourceAudit: options.sourceAudit || this.profile?.sourceAudit,
      diagnostics: options.includeDiagnostics ? this.inspectContract(options) : undefined,
    });
    this.profile = profile;
    return profile;
  }

  getState() {
    return {
      schema: "metric.visual.miniature_photographic_style_state.v1",
      style: this.stage?.style,
      stageId: this.stage?.id,
      hasRuntime: Boolean(this.runtime),
      hasLayerFactory: Boolean(this.layerFactory),
      hasBundle: Boolean(this.lastBundle),
      fit: clonePlainObject(this.fit),
      ground: clonePlainObject(this.ground),
      contacts: this.contacts,
      motion: this.motion,
      styleMotion: this.styleMotionController.toJSON(),
      contract: this.inspectContract(),
      metadata: clonePlainObject(this.metadata),
    };
  }

  defaultBundleOptions() {
    return {
      fit: clonePlainObject(this.fit),
      ground: clonePlainObject(this.ground),
      contacts: this.contacts,
      motion: this.motion,
    };
  }
}

export function createMiniaturePhotographicStyle(options = {}) {
  return new MiniaturePhotographicStyle(options);
}

export function createMiniaturePhotographicStyleFromProfile(profile, options = {}) {
  return new MiniaturePhotographicStyle({
    ...options,
    profile,
  });
}

export function applyMiniaturePhotographicStyle(runtime, descriptors, options = {}) {
  const style = createMiniaturePhotographicStyle(options);
  style.attachRuntime(runtime, { applyStage: options.applyStage !== false });
  if (descriptors) {
    style.applyToRuntime(runtime, {
      ...options,
      descriptors,
      source: options.source || "miniature-photographic-style",
    });
  }
  return { style, runtime };
}

function resolveStage(stage, stageOptions) {
  if (looksLikeStage(stage)) return stage;
  return createMiniatureStagePreset(stageOptions || {});
}

function optionValue(key, options, profile, fallback) {
  if (hasOwn(options, key)) return options[key];
  if (profile && hasOwn(profile, key)) return profile[key];
  return fallback;
}

function hasOwn(object, key) {
  return Object.prototype.hasOwnProperty.call(object || {}, key);
}

function looksLikeStage(value) {
  return Boolean(value && typeof value === "object" && (value.schema || value.camera || value.focus || value.postprocess || value.grounding));
}

function requireRuntime(runtime) {
  if (!runtime || typeof runtime !== "object") {
    throw new Error("MiniaturePhotographicStyle requires a runtime object.");
  }
  return runtime;
}

function mergeOptions(base, override) {
  if (override === false) return false;
  if (override == null) return clonePlainObject(base || {});
  if (base == null || base === false) return clonePlainObject(override || {});
  if (!isPlainObject(base) || !isPlainObject(override)) return clonePlainObject(override);
  const next = { ...base };
  for (const [key, value] of Object.entries(override)) {
    next[key] = isPlainObject(value) && isPlainObject(base[key])
      ? mergeOptions(base[key], value)
      : clonePlainObject(value);
  }
  return next;
}

function clonePlainObject(value) {
  if (value == null || typeof value !== "object") return value;
  if (Array.isArray(value)) return value.map(clonePlainObject);
  if (ArrayBuffer.isView(value)) return value;
  const next = {};
  for (const [key, entry] of Object.entries(value)) {
    next[key] = clonePlainObject(entry);
  }
  return next;
}

function isPlainObject(value) {
  return Boolean(value && typeof value === "object" && !Array.isArray(value) && !ArrayBuffer.isView(value));
}
