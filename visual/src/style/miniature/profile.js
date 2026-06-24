import {
  MINIATURE_STYLE_ID,
  MINIATURE_STYLE_PROFILE_SCHEMA,
  TROIS_MINIATURE_SOURCE_AUDIT,
} from "./constants.js";
import { inspectMiniatureStyleContract } from "./diagnostics.js";
import { createMiniatureStagePreset } from "./presets.js";
import { createMiniatureStyleMotionPreset } from "./style-motion.js";

/**
 * Creates a JSON-serializable miniature style profile.
 *
 * A profile carries the photographic miniature contract as data: stage,
 * camera, light rig, focus, ground relation, post-FX, fitting defaults and
 * style motion. It intentionally carries no runtime, canvas, WebGL handle,
 * function, or metric evidence.
 *
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniatureStyleProfile(options = {}) {
  const source = isMiniatureStyleProfile(options) ? options : null;
  const stageOptions = cloneSerializable(options.stageOptions || source?.stageOptions || {});
  const stage = normalizeStage(options.stage || source?.stage, stageOptions);
  const styleMotion = createMiniatureStyleMotionPreset(options.styleMotion || source?.styleMotion || "none");
  const metadata = {
    reusableMiniatureStyle: true,
    sourceGrammar: "METRIC miniature and native METRIC miniature audit",
    ...(source?.metadata || {}),
    ...(options.metadata || {}),
  };
  const profile = {
    schema: MINIATURE_STYLE_PROFILE_SCHEMA,
    id: options.id || source?.id || stage?.id || "metric-miniature-style-profile",
    style: MINIATURE_STYLE_ID,
    variant: options.variant || source?.variant || stage?.variant || "white",
    stageOptions,
    stage: cloneSerializable(stage),
    fit: cloneSerializable(options.fit ?? source?.fit ?? {}),
    ground: cloneSerializable(options.ground ?? source?.ground ?? {}),
    contacts: cloneSerializable(options.contacts ?? source?.contacts ?? undefined),
    motion: cloneSerializable(options.motion ?? source?.motion ?? undefined),
    styleMotion: cloneSerializable(styleMotion),
    metadata: cloneSerializable(metadata),
    sourceAudit: cloneSerializable(options.sourceAudit || source?.sourceAudit || {
      extractedIngredients: TROIS_MINIATURE_SOURCE_AUDIT.extractedIngredients,
    }),
  };
  if (options.diagnostics) {
    profile.diagnostics = cloneSerializable(options.diagnostics);
  }
  return deepFreeze(removeUndefined(profile));
}

/**
 * Normalizes an existing profile or profile-like options object.
 *
 * @param {object} [profileOrOptions]
 * @returns {object}
 */
export function normalizeMiniatureStyleProfile(profileOrOptions = {}) {
  return createMiniatureStyleProfile(profileOrOptions);
}

/**
 * @param {unknown} value
 * @returns {boolean}
 */
export function isMiniatureStyleProfile(value) {
  return Boolean(value && typeof value === "object" && value.schema === MINIATURE_STYLE_PROFILE_SCHEMA);
}

/**
 * Runs the style contract diagnostic against a serializable profile.
 *
 * @param {object} profileOrOptions
 * @param {object} [options]
 * @returns {object}
 */
export function inspectMiniatureStyleProfile(profileOrOptions = {}, options = {}) {
  const profile = normalizeMiniatureStyleProfile(profileOrOptions);
  return inspectMiniatureStyleContract({
    stage: options.stage || profile.stage,
    bundle: options.bundle,
    descriptors: options.descriptors,
    runtime: options.runtime,
    styleMotion: profile.styleMotion,
  });
}

function normalizeStage(stage, stageOptions) {
  if (looksLikeStage(stage)) return stage;
  return createMiniatureStagePreset(stageOptions || {});
}

function looksLikeStage(value) {
  return Boolean(value && typeof value === "object" && (value.schema || value.camera || value.focus || value.postprocess || value.grounding));
}

function cloneSerializable(value) {
  if (value === undefined) return undefined;
  if (value == null || typeof value !== "object") {
    if (typeof value === "function") return undefined;
    return value;
  }
  if (Array.isArray(value)) return value.map(cloneSerializable).filter((entry) => entry !== undefined);
  if (ArrayBuffer.isView(value)) return Array.from(value);
  if (value instanceof Date) return value.toISOString();
  const out = {};
  for (const [key, entry] of Object.entries(value)) {
    if (typeof entry === "function") continue;
    const next = cloneSerializable(entry);
    if (next !== undefined) out[key] = next;
  }
  return out;
}

function removeUndefined(value) {
  if (!value || typeof value !== "object") return value;
  if (Array.isArray(value)) return value.map(removeUndefined);
  const out = {};
  for (const [key, entry] of Object.entries(value)) {
    if (entry !== undefined) out[key] = removeUndefined(entry);
  }
  return out;
}

function deepFreeze(value) {
  if (!value || typeof value !== "object" || Object.isFrozen(value)) return value;
  Object.freeze(value);
  for (const entry of Object.values(value)) deepFreeze(entry);
  return value;
}
