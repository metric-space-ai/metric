import {
  MINIATURE_STYLE_ID,
  MINIATURE_STYLE_PROFILE_SCHEMA,
  MINIATURE_STYLE_SCHEMA,
} from "./constants.js";
import {
  createMiniatureReferenceLookOptions,
  createMiniatureStagePreset,
  listMiniatureReferenceLooks,
} from "./presets.js";
import { createMiniatureSceneBundle } from "./composition.js";
import {
  createMiniatureStyleProfile,
  inspectMiniatureStyleProfile,
} from "./profile.js";
import { selectMiniatureStyleMotionForLook } from "./style-motion.js";

const LOOK_ATLAS_SCHEMA = "metric.visual.miniature_look_atlas.v1";

/**
 * Creates a serializable atlas of native METRIC miniature looks.
 *
 * The atlas is a renderer-facing style contract. It contains no runtime, no
 * WebGL objects, no DOM nodes and no metric evidence. Its job is to make the
 * reusable photographic looks inspectable as complete bundles of camera,
 * focus, light, material, grounding, post-FX and style-motion choices.
 *
 * @param {object} [options]
 * @param {Array<string|object>} [options.looks]
 * @param {object} [options.overrides]
 * @param {object} [options.stageOptions]
 * @param {string|object} [options.styleMotion]
 * @param {object[]} [options.descriptors]
 * @returns {object}
 */
export function createMiniatureLookAtlas(options = {}) {
  const looks = Array.isArray(options.looks) && options.looks.length
    ? options.looks
    : listMiniatureReferenceLooks().map((look) => look.look);
  const entries = looks.map((look) => createMiniatureLookAtlasEntry(look, options));
  return deepFreeze({
    schema: LOOK_ATLAS_SCHEMA,
    style: MINIATURE_STYLE_ID,
    id: options.id || "metric-miniature-look-atlas",
    entries,
    summary: {
      entryCount: entries.length,
      readyCount: entries.filter((entry) => entry.diagnostics.status === "ready").length,
      looks: entries.map((entry) => entry.look),
    },
    metadata: {
      nativeMetricStyle: true,
      reusablePhotographicLooks: true,
      ...(options.metadata || {}),
    },
  });
}

/**
 * Creates one atlas entry for a named or inline reference look.
 *
 * @param {string|object} look
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniatureLookAtlasEntry(look, options = {}) {
  const lookOptions = createMiniatureReferenceLookOptions(look, overridesForLook(look, options.overrides));
  const stage = createMiniatureStagePreset({
    look: lookOptions,
    ...(options.stageOptions || {}),
  });
  const styleMotion = options.styleMotion || lookOptions.styleMotion || selectMiniatureStyleMotionForLook(lookOptions.look);
  const profile = createMiniatureStyleProfile({
    id: `${lookOptions.id || stage.id || lookOptions.look}-profile`,
    stage,
    styleMotion,
    metadata: {
      look: lookOptions.look,
      sourceExamples: lookOptions.sourceExamples || [],
      reusablePhotographicLook: true,
    },
  });
  const diagnostics = inspectMiniatureStyleProfile(profile, {
    descriptors: options.descriptors || [],
  });
  return deepFreeze({
    schema: "metric.visual.miniature_look_atlas_entry.v1",
    style: MINIATURE_STYLE_ID,
    look: lookOptions.look,
    id: lookOptions.id || stage.id,
    variant: stage.variant,
    sourceExamples: (lookOptions.sourceExamples || []).slice(),
    profileSchema: MINIATURE_STYLE_PROFILE_SCHEMA,
    stageSchema: MINIATURE_STYLE_SCHEMA,
    profile,
    stage: profile.stage,
    styleMotion: profile.styleMotion,
    contract: summarizeLookContract(stage, profile.styleMotion),
    diagnostics: summarizeDiagnostics(diagnostics),
  });
}

/**
 * Applies every reference look to one descriptor set and returns a serializable
 * scene atlas. This is the reusable visual-regression surface for the
 * photographic layer: the input descriptors stay the same, while camera,
 * light, focus, ground, material, post-FX and style-motion contracts vary by
 * look.
 *
 * @param {object[]|object} descriptors
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniatureLookSceneAtlas(descriptors, options = {}) {
  const looks = Array.isArray(options.looks) && options.looks.length
    ? options.looks
    : listMiniatureReferenceLooks().map((look) => look.look);
  const entries = looks.map((look) => createMiniatureLookSceneAtlasEntry(look, descriptors, options));
  return deepFreeze({
    schema: "metric.visual.miniature_look_scene_atlas.v1",
    style: MINIATURE_STYLE_ID,
    id: options.id || "metric-miniature-look-scene-atlas",
    entries,
    summary: {
      entryCount: entries.length,
      readyCount: entries.filter((entry) => entry.diagnostics.status === "ready").length,
      looks: entries.map((entry) => entry.look),
      descriptorCount: descriptorCount(descriptors),
    },
    metadata: {
      nativeMetricStyle: true,
      reusablePhotographicLooks: true,
      visualRegressionSurface: true,
      ...(options.metadata || {}),
    },
  });
}

/**
 * Applies one reference look to one descriptor set.
 *
 * @param {string|object} look
 * @param {object[]|object} descriptors
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniatureLookSceneAtlasEntry(look, descriptors, options = {}) {
  const lookOptions = createMiniatureReferenceLookOptions(look, overridesForLook(look, options.overrides));
  const stage = createMiniatureStagePreset({
    look: lookOptions,
    ...(options.stageOptions || {}),
  });
  const styleMotion = options.styleMotion || lookOptions.styleMotion || selectMiniatureStyleMotionForLook(lookOptions.look);
  const profile = createMiniatureStyleProfile({
    id: `${lookOptions.id || stage.id || lookOptions.look}-scene-profile`,
    stage,
    styleMotion,
    metadata: {
      look: lookOptions.look,
      sourceExamples: lookOptions.sourceExamples || [],
      reusablePhotographicLook: true,
      visualRegressionSurface: true,
    },
  });
  const bundle = createMiniatureSceneBundle(descriptors, {
    stage,
    fit: options.fit || { targetSpan: 2.08 },
    ground: options.ground === false
      ? false
      : {
        groundY: stage.grounding?.groundY,
        padding: 0.54,
        ...(options.ground || {}),
      },
    contacts: options.contacts,
    motion: options.motion ?? false,
    metadata: {
      look: lookOptions.look,
      source: "miniature-look-scene-atlas",
      ...(options.bundleMetadata || {}),
    },
  });
  const diagnostics = inspectMiniatureStyleProfile(profile, {
    bundle,
    descriptors: bundle.layers,
  });
  return deepFreeze({
    schema: "metric.visual.miniature_look_scene_atlas_entry.v1",
    style: MINIATURE_STYLE_ID,
    look: lookOptions.look,
    id: lookOptions.id || stage.id,
    variant: stage.variant,
    sourceExamples: (lookOptions.sourceExamples || []).slice(),
    descriptorCount: descriptorCount(descriptors),
    profile,
    stage: profile.stage,
    styleMotion: profile.styleMotion,
    bundle,
    fit: bundle.fit || null,
    contract: summarizeLookContract(stage, profile.styleMotion),
    diagnostics: summarizeDiagnostics(diagnostics),
  });
}

function summarizeLookContract(stage, styleMotion) {
  return {
    camera: {
      fov: stage.camera?.fov ?? null,
      position: cloneSerializable(stage.camera?.position || null),
      target: cloneSerializable(stage.camera?.target || stage.camera?.lookAt || null),
      coordinateSystem: stage.camera?.coordinateSystem || null,
    },
    focus: {
      axis: stage.focus?.axis || null,
      radiusRatio: stage.focus?.radiusRatio ?? null,
      focusBandRatio: stage.focus?.focusBandRatio ?? null,
      blurRadius: stage.focus?.blurRadius ?? null,
      blurCurve: stage.focus?.blurCurve ?? null,
    },
    lighting: {
      mode: stage.lightRig?.mode || null,
      ambient: stage.lightRig?.ambient ? true : false,
      pointCount: Array.isArray(stage.lightRig?.points) ? stage.lightRig.points.length : 0,
      hasKey: Boolean(stage.lightRig?.key),
      hasFill: Boolean(stage.lightRig?.fill),
    },
    grounding: {
      plane: stage.grounding?.plane || null,
      groundY: stage.grounding?.groundY ?? null,
      contact: stage.grounding?.contact?.enabled !== false,
      projection: stage.grounding?.projection?.enabled !== false,
    },
    materials: Object.keys(stage.materials || {}),
    postprocess: {
      tiltShift: stage.postprocess?.tiltShift !== false,
      miniatureFrame: stage.postprocess?.postFx?.miniatureFrame?.enabled !== false,
      cameraDof: stage.postprocess?.postFx?.cameraDof?.enabled === true,
      grade: stage.postprocess?.postFx?.grade?.enabled !== false,
      vignette: stage.postprocess?.postFx?.vignette?.enabled !== false,
      bloom: stage.postprocess?.postFx?.bloom?.enabled !== false,
      bloomIntensity: stage.postprocess?.postFx?.bloom?.intensity ?? null,
    },
    animation: {
      mode: styleMotion?.mode || "none",
      focus: styleMotion?.focus?.enabled !== false,
      camera: styleMotion?.camera?.enabled !== false,
      postprocess: styleMotion?.postprocess?.enabled !== false,
    },
  };
}

function summarizeDiagnostics(diagnostics) {
  return {
    schema: diagnostics.schema,
    ok: diagnostics.ok,
    status: diagnostics.status,
    warnings: diagnostics.warnings,
    photographicStyle: diagnostics.checks?.photographicStyle || null,
    coverage: diagnostics.coverage,
  };
}

function overridesForLook(look, overrides) {
  if (!overrides || typeof overrides !== "object") return {};
  const name = typeof look === "string" ? look : look?.look || look?.name;
  return name && overrides[name] && typeof overrides[name] === "object"
    ? overrides[name]
    : {};
}

function cloneSerializable(value) {
  if (value === undefined) return undefined;
  if (value == null || typeof value !== "object") return value;
  if (Array.isArray(value)) return value.map(cloneSerializable);
  if (ArrayBuffer.isView(value)) return Array.from(value);
  const out = {};
  for (const [key, entry] of Object.entries(value)) {
    if (typeof entry !== "function") out[key] = cloneSerializable(entry);
  }
  return out;
}

function descriptorCount(descriptors) {
  return Array.isArray(descriptors) ? descriptors.length : (descriptors ? 1 : 0);
}

function deepFreeze(value) {
  if (ArrayBuffer.isView(value)) return value;
  if (!value || typeof value !== "object" || Object.isFrozen(value)) return value;
  Object.freeze(value);
  for (const entry of Object.values(value)) deepFreeze(entry);
  return value;
}
