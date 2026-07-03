import { createMiniaturePhotographicRigContract } from "./rig-contract.js";

export const MINIATURE_RIG_REPORT_SCHEMA = "metric.visual.miniature_rig_report.v1";

/**
 * Creates a compact runtime-facing report for a miniature visual.
 *
 * The report is intentionally about the photographic visual rig only. It does
 * not inspect metric algorithms or recompute visual evidence.
 *
 * @param {object} [input]
 * @returns {object}
 */
export function createMiniatureRigReport(input = {}) {
  const resolved = resolveReportInput(input);
  const style = resolved.style || null;
  const runtime = resolved.runtime || style?.runtime || null;
  const bundle = resolved.bundle || style?.lastBundle || null;
  const stage = resolved.stage || style?.stage || bundle?.stage || null;
  const descriptors = toArray(resolved.descriptors || bundle?.layers || style?.lastBundle?.layers || []);
  const runtimeState = runtime?.getState?.() || {};
  const postprocess = runtimeState.postprocess || {};
  const rigContract = resolved.rigContract || createMiniaturePhotographicRigContract({
    style,
    stage,
    bundle,
    descriptors,
    runtime,
    styleMotion: resolved.styleMotion || style?.styleMotionController?.toJSON?.() || null,
  });
  const hoverTarget = runtimeState.hoverFocus?.target || null;
  const focusTarget = runtimeState.focusTarget || rigContract.summary?.focusTarget || null;
  const focusDistance = finiteOrNull(postprocess.focusDistance ?? postprocess.postFx?.cameraDof?.focusDistance);
  const sections = rigContract.sections || {};

  return {
    schema: MINIATURE_RIG_REPORT_SCHEMA,
    ready: Boolean(rigContract.ok),
    status: rigContract.status || "unknown",
    rigContract,
    runtime: {
      running: runtimeState.running === true,
      layerCount: finiteOrNull(runtimeState.layerInstanceCount),
      layerState: runtimeState.layerState || null,
      warnings: toArray(runtimeState.warnings),
    },
    postprocess: {
      cameraDof: Boolean(postprocess.postFx?.cameraDof?.enabled),
      dofModel: postprocess.postFx?.cameraDof?.model || "",
      dofFocusModel: postprocess.postFx?.cameraDof?.focusModel || "",
      dofCocModel: postprocess.postFx?.cameraDof?.cocModel || "",
      dofDepthMode: postprocess.postFx?.cameraDof?.depthMode || "",
      dofRequiresDepthTexture: postprocess.postFx?.cameraDof?.requiresDepthTexture ?? null,
      dofUsesCameraNearFar: postprocess.postFx?.cameraDof?.usesCameraNearFar ?? null,
      dofAperture: finiteOrNull(postprocess.postFx?.cameraDof?.aperture),
      dofFocalLength: finiteOrNull(postprocess.postFx?.cameraDof?.focalLength),
      dofFStop: finiteOrNull(postprocess.postFx?.cameraDof?.fStop),
      dofSensorScale: finiteOrNull(postprocess.postFx?.cameraDof?.sensorScale),
      dofMaxBlur: finiteOrNull(postprocess.postFx?.cameraDof?.maxBlur),
      dofFocalRange: finiteOrNull(postprocess.postFx?.cameraDof?.focalRange),
      depthTexture: Boolean(postprocess.sceneDepthTexture),
      depthEncoding: postprocess.sceneDepthEncoding || "",
      passes: toArray(postprocess.postFx?.enabledPasses),
      focusDistance,
    },
    camera: clonePlainObject(sections.camera?.evidence || {}),
    light: clonePlainObject(sections.light?.evidence || {}),
    ground: clonePlainObject(sections.ground?.evidence || {}),
    materials: clonePlainObject(sections.materials?.evidence || {}),
    motion: clonePlainObject(sections.motion?.evidence || {}),
    morphRoom: clonePlainObject(sections.morphRoom?.evidence || {}),
    focus: {
      distance: focusDistance,
      target: focusTarget ? clonePlainObject(focusTarget) : null,
      source: focusTarget?.source || "",
    },
    hoverFocus: {
      enabled: runtimeState.hoverFocus?.enabled === true,
      recordId: hoverTarget?.recordId || "",
      distancePx: finiteOrNull(hoverTarget?.screenDistancePx),
      target: hoverTarget ? clonePlainObject(hoverTarget) : null,
    },
    scene: {
      descriptorCount: descriptors.length,
      hasBundle: Boolean(bundle),
      hasStage: Boolean(stage),
    },
    missing: toArray(rigContract.missing).map((item) => item?.name || String(item)),
  };
}

export function miniatureRigReportDataset(reportOrInput = {}) {
  const report = isMiniatureRigReport(reportOrInput)
    ? reportOrInput
    : createMiniatureRigReport(reportOrInput);
  return {
    metricCameraDof: String(Boolean(report.postprocess.cameraDof)),
    metricDofModel: report.postprocess.dofModel || "",
    metricDofFocusModel: report.postprocess.dofFocusModel || "",
    metricDofCocModel: report.postprocess.dofCocModel || "",
    metricDofDepthMode: report.postprocess.dofDepthMode || "",
    metricDofAperture: stringifyFinite(report.postprocess.dofAperture),
    metricDofMaxBlur: stringifyFinite(report.postprocess.dofMaxBlur),
    metricDofFocalRange: stringifyFinite(report.postprocess.dofFocalRange),
    metricDofRequiresDepthTexture: String(Boolean(report.postprocess.dofRequiresDepthTexture)),
    metricDofUsesCameraNearFar: String(Boolean(report.postprocess.dofUsesCameraNearFar)),
    metricDofFocalLength: stringifyFinite(report.postprocess.dofFocalLength),
    metricDofFStop: stringifyFinite(report.postprocess.dofFStop),
    metricDofSensorScale: stringifyFinite(report.postprocess.dofSensorScale),
    metricDepthTexture: String(Boolean(report.postprocess.depthTexture)),
    metricDepthEncoding: report.postprocess.depthEncoding || "",
    metricPostFxPasses: report.postprocess.passes.join(","),
    metricCameraProjection: report.camera.projection || "",
    metricCameraCoordinateSystem: report.camera.coordinateSystem || "",
    metricCameraOblique: String(Boolean(report.camera.isOblique)),
    metricCameraFov: stringifyFinite(report.camera.fov),
    metricLightMode: report.light.mode || "",
    metricLightCount: stringifyFinite(report.light.lightCount),
    metricLightPointCount: stringifyFinite(report.light.pointCount),
    metricLightAmbientIntensity: stringifyFinite(report.light.ambientIntensity),
    metricGroundY: stringifyFinite(report.ground.groundY),
    metricGroundLayer: String(Boolean(report.ground.hasGroundLayer)),
    metricGroundProjectionLayer: String(Boolean(report.ground.hasProjectionLayer)),
    metricGroundProjectionAlpha: stringifyFinite(report.ground.projectionAlpha),
    metricGroundContactAlpha: stringifyFinite(report.ground.contactAlpha),
    metricMaterialFamilies: toArray(report.materials.families).join(","),
    metricMaterialDescriptorCount: stringifyFinite(report.materials.descriptorCount),
    metricMaterialStyledDescriptors: stringifyFinite(report.materials.styledDescriptorCount),
    metricMaterialLitDescriptors: stringifyFinite(report.materials.litDescriptorCount),
    metricMotionEnabled: String(Boolean(report.motion.enabled)),
    metricMotionMode: report.motion.mode || "",
    metricMotionDescriptorCount: stringifyFinite(report.motion.descriptorMotionCount),
    metricMotionRuntime: String(Boolean(report.motion.hasRuntime)),
    metricMotionBeforeRender: String(Boolean(report.motion.hasBeforeRenderHook)),
    metricMorphRoomLayers: stringifyFinite(report.morphRoom.morphLayerCount),
    metricMorphRoomFlatY: stringifyFinite(report.morphRoom.sameRoomMorph?.flatY),
    metricFocusDistance: stringifyFinite(report.focus.distance),
    metricFocusTarget: formatPosition(report.focus.target?.position),
    metricFocusSource: report.focus.source || "",
    metricHoverFocusEnabled: String(Boolean(report.hoverFocus.enabled)),
    metricHoverFocusRecord: report.hoverFocus.recordId || "",
    metricHoverFocusDistancePx: stringifyFinite(report.hoverFocus.distancePx),
    metricMiniatureRigReady: String(Boolean(report.ready)),
    metricMiniatureRigStatus: report.status || "",
    metricMiniatureRigMissing: report.missing.join(","),
  };
}

export function publishMiniatureRigDataset(target, reportOrInput = {}) {
  const node = target || defaultDatasetTarget();
  if (!node?.dataset) return null;
  const report = isMiniatureRigReport(reportOrInput)
    ? reportOrInput
    : createMiniatureRigReport(reportOrInput);
  const dataset = miniatureRigReportDataset(report);
  for (const [key, value] of Object.entries(dataset)) {
    node.dataset[key] = value;
  }
  return report;
}

export function attachMiniatureRigReporter(input = {}) {
  const initial = resolveReportInput(input);
  const runtime = initial.runtime || initial.style?.runtime || null;
  const target = initial.target || initial.datasetTarget || defaultDatasetTarget();
  const resolve = () => resolveReportInput(input);
  const publish = () => publishMiniatureRigDataset(target, resolve());
  const subscriptions = [];
  const eventNames = [
    "hoverfocuschange",
    "focustargetchange",
    "camerafocuschange",
    "layerschange",
    "postprocesschange",
    "stagechange",
    "resize",
    "start",
    "stop",
  ];

  if (runtime?.on) {
    for (const eventName of eventNames) {
      const unsubscribe = runtime.on(eventName, publish);
      if (typeof unsubscribe === "function") subscriptions.push(unsubscribe);
    }
  }

  publish();

  return {
    schema: "metric.visual.miniature_rig_reporter.v1",
    publish,
    report: () => createMiniatureRigReport(resolve()),
    dispose() {
      while (subscriptions.length) {
        const unsubscribe = subscriptions.pop();
        try {
          unsubscribe();
        } catch (error) {
          // Ignore stale event unsubscriptions during page teardown.
        }
      }
    },
  };
}

export function isMiniatureRigReport(value) {
  return Boolean(value && value.schema === MINIATURE_RIG_REPORT_SCHEMA);
}

function resolveReportInput(input) {
  const dynamic = typeof input.resolve === "function" ? input.resolve() : null;
  return {
    ...input,
    ...(dynamic || {}),
  };
}

function defaultDatasetTarget() {
  return typeof document !== "undefined" ? document.documentElement : null;
}

function formatPosition(position) {
  if (!Array.isArray(position)) return "";
  return position.map((value) => {
    const number = Number(value);
    return Number.isFinite(number) ? number.toFixed(3) : "";
  }).join(",");
}

function stringifyFinite(value) {
  const number = Number(value);
  return Number.isFinite(number) ? String(number) : "";
}

function finiteOrNull(value) {
  const number = Number(value);
  return Number.isFinite(number) ? number : null;
}

function toArray(value) {
  return Array.isArray(value) ? value : [];
}

function clonePlainObject(value) {
  if (!value || typeof value !== "object") return value;
  try {
    return JSON.parse(JSON.stringify(value));
  } catch (error) {
    return { ...value };
  }
}
