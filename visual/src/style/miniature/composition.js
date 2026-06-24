import { createMetricMiniaturePalette } from "./palette.js";
import { createMiniatureStagePreset } from "./presets.js";
import { createMiniatureDescriptorBundle } from "./descriptors.js";
import {
  fitMiniatureSceneDescriptors,
  miniatureGroundFromFit,
} from "./scene-fit.js";

/**
 * Creates a complete reusable miniature scene bundle.
 *
 * This is the high-level style/postprocess composition layer for arbitrary
 * METRIC visuals. It receives ordinary layer descriptors, stages their
 * coordinates for a miniature camera, applies material/light/focus presets,
 * optionally creates a ground plane, and returns runtime-ready stage/layers.
 *
 * @param {object[]|object} descriptors
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniatureSceneBundle(descriptors, options = {}) {
  const palette = options.palette || createMetricMiniaturePalette(options.paletteOptions || {});
  const stage = options.stage || createMiniatureStagePreset({
    palette,
    ...(options.stageOptions || {}),
  });
  const fitResult = fitMiniatureSceneDescriptors(descriptors, normalizeFitOptions(options.fit, stage, options));
  const ground = options.ground === false
    ? false
    : miniatureGroundFromFit(fitResult.fit, options.ground || {});
  const bundle = createMiniatureDescriptorBundle(fitResult.descriptors, {
    ...options,
    palette,
    stage,
    ground,
  });

  return {
    ...bundle,
    schema: "metric.visual.miniature_scene_bundle.v1",
    fit: fitResult.fit,
    metadata: {
      ...(bundle.metadata || {}),
      reusableMiniatureScene: true,
      stagedDescriptorCount: fitResult.descriptors.length,
    },
  };
}

/**
 * Applies a miniature scene bundle to a MetricVisualRuntime-like object.
 *
 * @param {object} runtime
 * @param {object} bundle
 * @param {object} [options]
 * @returns {object}
 */
export function applyMiniatureSceneBundle(runtime, bundle, options = {}) {
  if (!runtime || typeof runtime !== "object") {
    throw new Error("applyMiniatureSceneBundle() requires a runtime object.");
  }
  if (!bundle || typeof bundle !== "object") {
    throw new Error("applyMiniatureSceneBundle() requires a miniature scene bundle.");
  }
  if (bundle.stage && typeof runtime.applyStagePreset === "function") {
    runtime.applyStagePreset(bundle.stage);
  }
  if (Array.isArray(bundle.layers) && typeof runtime.setLayerDescriptors === "function") {
    runtime.setLayerDescriptors(bundle.layers, {
      source: options.source || "miniature-scene-bundle",
    });
  }
  return runtime;
}

function normalizeFitOptions(fit, stage, options = {}) {
  if (fit === false) return { enabled: false };
  const base = !fit || typeof fit !== "object" ? { targetSpan: 2 } : { ...fit };
  const groundOptions = options.ground && typeof options.ground === "object" ? options.ground : {};
  const groundY = stage?.grounding?.groundY ?? groundOptions.groundY;
  const existing = base.sameRoomMorph ?? base.morphRoom;
  base.sameRoomMorph = existing === false
    ? false
    : {
      groundY,
      ...(existing && typeof existing === "object" ? existing : {}),
    };
  return base;
}
