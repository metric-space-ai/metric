export const MINIATURE_PHOTOGRAPHIC_RIG_SCHEMA = "metric.visual.miniature_photographic_rig.v1";

/**
 * Builds a serializable contract for the photographic miniature rig.
 *
 * The contract is intentionally renderer-facing. It does not compute data,
 * clustering, metrics, embeddings or projections. It describes whether an
 * already exported METRIC visual is staged with the reusable camera, light,
 * floor, material, depth-of-field, frame and motion system.
 *
 * @param {object} [input]
 * @param {object} [input.stage]
 * @param {object} [input.runtime]
 * @param {object} [input.style]
 * @param {object} [input.bundle]
 * @param {object[]} [input.descriptors]
 * @param {object} [input.styleMotion]
 * @returns {object}
 */
export function createMiniaturePhotographicRigContract(input = {}) {
  const style = input.style || null;
  const stage = input.stage || style?.stage || input.bundle?.stage || null;
  const runtime = input.runtime || style?.runtime || null;
  const descriptors = toArray(input.descriptors || input.bundle?.layers || style?.lastBundle?.layers || []);
  const runtimeState = runtime?.getState?.() || null;
  const postprocess = runtimeState?.postprocess || runtime?.postprocess?.getState?.() || null;
  const styleMotion = input.styleMotion || style?.styleMotionController?.toJSON?.() || null;

  const sections = {
    camera: inspectCamera(stage),
    light: inspectLight(stage),
    focus: inspectFocus(stage, runtimeState, postprocess),
    depthOfField: inspectDepthOfField(stage, postprocess),
    frame: inspectFrame(stage, postprocess),
    ground: inspectGround(stage, descriptors),
    morphRoom: inspectMorphRoom(input.bundle || style?.lastBundle || null, descriptors, stage),
    materials: inspectMaterials(stage, descriptors),
    motion: inspectMotion(styleMotion, descriptors),
  };
  const requirements = Object.entries(sections).map(([name, section]) => ({
    name,
    ok: section.ok,
    missing: section.missing || [],
    evidence: section.evidence || {},
  }));
  const missing = requirements
    .filter((requirement) => !requirement.ok)
    .map((requirement) => ({
      name: requirement.name,
      missing: requirement.missing,
    }));

  return {
    schema: MINIATURE_PHOTOGRAPHIC_RIG_SCHEMA,
    ok: missing.length === 0,
    status: missing.length === 0 ? "ready" : "partial",
    sections,
    requirements,
    missing,
    summary: {
      descriptorCount: descriptors.length,
      styledDescriptorCount: descriptors.filter((descriptor) => descriptor?.metadata?.miniature === true).length,
      runtimeAttached: Boolean(runtime),
      depthTexture: Boolean(postprocess?.sceneDepthTexture),
      depthEncoding: postprocess?.sceneDepthEncoding || null,
      postFxPasses: postprocess?.postFx?.enabledPasses || [],
      focusDistance: finiteOrNull(postprocess?.focusDistance ?? postprocess?.postFx?.cameraDof?.focusDistance),
      focusTarget: runtimeState?.focusTarget || null,
      hoverFocusEnabled: runtimeState?.hoverFocus?.enabled === true,
      morphRoom: sections.morphRoom.evidence,
    },
  };
}

export function isMiniaturePhotographicRigReady(contract) {
  return Boolean(contract && contract.schema === MINIATURE_PHOTOGRAPHIC_RIG_SCHEMA && contract.ok === true);
}

function inspectCamera(stage) {
  const camera = stage?.camera || {};
  const coordinateSystem = camera.coordinateSystem || stage?.coordinateSystem || "";
  const position = point3(camera.position);
  const target = point3(camera.target || camera.lookAt);
  const up = point3(camera.up);
  const fov = finiteOrNull(camera.fov ?? camera.fovDegrees);
  const missing = [];
  if (camera.projection !== "perspective") missing.push("camera.projection=perspective");
  if (!isMetricXzGroundCoordinateSystem(coordinateSystem)) missing.push("camera.coordinateSystem=metric-xz-ground");
  if (!position) missing.push("camera.position");
  if (!target) missing.push("camera.target");
  if (!up) missing.push("camera.up");
  if (!(fov > 0)) missing.push("camera.fov");
  const isOblique = position && target
    ? Math.abs(position[1] - target[1]) > 0.1 && Math.abs(position[2] - target[2]) > 0.1
    : false;
  if (!isOblique) missing.push("camera.oblique");
  return {
    ok: missing.length === 0,
    missing,
    evidence: {
      projection: camera.projection || null,
      coordinateSystem: coordinateSystem || null,
      fov,
      position,
      target,
      up,
      isOblique,
      orbitDamping: Boolean(camera.orbit?.enableDamping),
    },
  };
}

function isMetricXzGroundCoordinateSystem(value) {
  const text = String(value || "").toLowerCase();
  return text === "metric-xz-ground" || (text.includes("metric") && text.includes("xz-ground"));
}

function inspectLight(stage) {
  const rig = stage?.lightRig || {};
  const pointCount = Array.isArray(rig.points) ? rig.points.length : 0;
  const lightCount = (rig.key ? 1 : 0) + (rig.fill ? 1 : 0) + pointCount;
  const missing = [];
  if (!rig.ambient) missing.push("lightRig.ambient");
  if (lightCount < 1) missing.push("lightRig.key|fill|points");
  return {
    ok: missing.length === 0,
    missing,
    evidence: {
      mode: rig.mode || null,
      ambientIntensity: finiteOrNull(rig.ambient?.intensity),
      lightCount,
      pointCount,
    },
  };
}

function inspectFocus(stage, runtimeState, postprocess) {
  const focus = stage?.focus || {};
  const runtimeTarget = runtimeState?.focusTarget || null;
  const runtimeDistance = finiteOrNull(postprocess?.focusDistance ?? postprocess?.postFx?.cameraDof?.focusDistance);
  const missing = [];
  if (!focus.axis && !focus.kind) missing.push("stage.focus");
  if (focus.enabled === false) missing.push("focus.enabled");
  if (postprocess && !(runtimeDistance > 0)) missing.push("runtime.focusDistance");
  return {
    ok: missing.length === 0,
    missing,
    evidence: {
      axis: focus.axis || null,
      yRatio: finiteOrNull(focus.yRatio),
      radiusRatio: finiteOrNull(focus.radiusRatio),
      focusBandRatio: finiteOrNull(focus.focusBandRatio),
      runtimeDistance,
      runtimeTarget,
    },
  };
}

function inspectDepthOfField(stage, postprocess) {
  const stageDof = stage?.postprocess?.postFx?.cameraDof || stage?.postprocess?.cameraDof || null;
  const runtimeDof = postprocess?.postFx?.cameraDof || null;
  const enabled = runtimeDof?.enabled ?? stageDof?.enabled;
  const model = runtimeDof?.model || stageDof?.model || (enabled === true ? "camera-depth-circle-of-confusion" : null);
  const focusModel = runtimeDof?.focusModel || stageDof?.focusModel || (enabled === true ? "camera-space-distance" : null);
  const cocModel = runtimeDof?.cocModel || stageDof?.cocModel || (enabled === true ? "thin-lens-circle-of-confusion" : null);
  const depthMode = runtimeDof?.depthMode || stageDof?.depthMode || (enabled === true ? "raw-camera-depth" : null);
  const requiresDepthTexture = runtimeDof?.requiresDepthTexture ?? stageDof?.requiresDepthTexture ?? (enabled === true);
  const usesCameraNearFar = runtimeDof?.usesCameraNearFar ?? stageDof?.usesCameraNearFar ?? (enabled === true);
  const focusDistance = finiteOrNull(postprocess?.focusDistance ?? runtimeDof?.focusDistance ?? stageDof?.focusDistance);
  const aperture = finiteOrNull(runtimeDof?.aperture ?? stageDof?.aperture);
  const focalLength = finiteOrNull(runtimeDof?.focalLength ?? stageDof?.focalLength);
  const fStop = finiteOrNull(runtimeDof?.fStop ?? stageDof?.fStop);
  const sensorScale = finiteOrNull(runtimeDof?.sensorScale ?? stageDof?.sensorScale);
  const maxBlur = finiteOrNull(runtimeDof?.maxBlur ?? stageDof?.maxBlur);
  const focalRange = finiteOrNull(runtimeDof?.focalRange ?? stageDof?.focalRange);
  const cameraNear = finiteOrNull(runtimeDof?.cameraNear ?? stageDof?.cameraNear);
  const cameraFar = finiteOrNull(runtimeDof?.cameraFar ?? stageDof?.cameraFar);
  const missing = [];
  if (enabled !== true) missing.push("postFx.cameraDof.enabled");
  if (model !== "camera-depth-circle-of-confusion") missing.push("cameraDof.model=camera-depth-circle-of-confusion");
  if (focusModel !== "camera-space-distance") missing.push("cameraDof.focusModel=camera-space-distance");
  if (cocModel !== "thin-lens-circle-of-confusion") missing.push("cameraDof.cocModel=thin-lens-circle-of-confusion");
  if (depthMode !== "raw-camera-depth") missing.push("cameraDof.depthMode=raw-camera-depth");
  if (requiresDepthTexture !== true) missing.push("cameraDof.requiresDepthTexture");
  if (usesCameraNearFar !== true) missing.push("cameraDof.usesCameraNearFar");
  if (!(focusDistance > 0)) missing.push("cameraDof.focusDistance");
  if (!(aperture > 0)) missing.push("cameraDof.aperture");
  if (!(focalLength > 0)) missing.push("cameraDof.focalLength");
  if (!(fStop > 0)) missing.push("cameraDof.fStop");
  if (!(sensorScale > 0)) missing.push("cameraDof.sensorScale");
  if (!(maxBlur > 0)) missing.push("cameraDof.maxBlur");
  if (!(focalRange > 0)) missing.push("cameraDof.focalRange");
  if (postprocess && postprocess.sceneDepthTexture !== true) missing.push("sceneDepthTexture");
  if (postprocess && postprocess.sceneDepthEncoding !== "raw-depth") missing.push("sceneDepthEncoding=raw-depth");
  return {
    ok: missing.length === 0,
    missing,
    evidence: {
      enabled,
      model,
      focusModel,
      cocModel,
      depthMode,
      requiresDepthTexture,
      usesCameraNearFar,
      focusDistance,
      aperture,
      focalLength,
      fStop,
      sensorScale,
      maxBlur,
      focalRange,
      cameraNear,
      cameraFar,
      sceneDepthTexture: postprocess?.sceneDepthTexture ?? null,
      sceneDepthEncoding: postprocess?.sceneDepthEncoding || null,
    },
  };
}

function inspectFrame(stage, postprocess) {
  const stageFrame = stage?.postprocess?.postFx?.miniatureFrame || null;
  const passes = postprocess?.postFx?.enabledPasses || [];
  const missing = [];
  if (!stageFrame && !passes.includes("MiniatureFramePass")) missing.push("postFx.miniatureFrame");
  if (!stage?.postprocess?.postFx?.grade && !passes.includes("ColorGradePass")) missing.push("postFx.grade");
  if (!stage?.postprocess?.postFx?.vignette && !passes.includes("VignettePass")) missing.push("postFx.vignette");
  return {
    ok: missing.length === 0,
    missing,
    evidence: {
      passes,
      frameAmount: finiteOrNull(stageFrame?.amount),
      stageSpotlight: finiteOrNull(stageFrame?.stageSpotlight),
      stageShadow: finiteOrNull(stageFrame?.stageShadow),
      subjectIsolation: finiteOrNull(stageFrame?.subjectIsolation),
      lensAberration: finiteOrNull(stageFrame?.lensAberration),
    },
  };
}

function inspectGround(stage, descriptors) {
  const grounding = stage?.grounding || {};
  const primitives = descriptors.map((descriptor) => descriptor?.primitive || descriptor?.kind).filter(Boolean);
  const hasGroundLayer = primitives.includes("GroundPlaneLayer");
  const hasProjectionLayer = primitives.includes("GroundProjectionLayer");
  const missing = [];
  if (!grounding.kind && grounding.enabled === false) missing.push("stage.grounding");
  if (descriptors.length && !hasGroundLayer) missing.push("GroundPlaneLayer");
  return {
    ok: missing.length === 0,
    missing,
    evidence: {
      groundY: finiteOrNull(grounding.groundY),
      hasGroundLayer,
      hasProjectionLayer,
      projectionAlpha: finiteOrNull(grounding.projection?.alpha),
      contactAlpha: finiteOrNull(grounding.contact?.alpha),
    },
  };
}

function inspectMorphRoom(bundle, descriptors, stage) {
  const morphLayers = descriptors.filter(isMorphLayer);
  const fit = bundle?.fit || null;
  const sameRoomMorph = fit?.sameRoomMorph || null;
  const groundY = finiteOrNull(sameRoomMorph?.groundY ?? stage?.grounding?.groundY);
  const flatY = finiteOrNull(sameRoomMorph?.flatY);
  const epsilon = positiveNumber(sameRoomMorph?.flatSpanEpsilon, 1e-5);
  const layerReports = morphLayers.map((descriptor) => inspectMorphLayerRoom(descriptor, {
    groundY,
    flatY,
    epsilon,
    projectionLayer: isProjectionLayer(descriptor),
    annotationLayer: isAnnotationLayer(descriptor),
  }));
  const missing = [];

  if (morphLayers.length && (!sameRoomMorph || sameRoomMorph.enabled === false)) {
    missing.push("fit.sameRoomMorph");
  }
  for (const report of layerReports) {
    if (!report.ok) missing.push(`${report.id}:${report.missing.join("|")}`);
  }

  return {
    ok: missing.length === 0,
    missing,
    evidence: {
      morphLayerCount: morphLayers.length,
      projectionMorphLayerCount: morphLayers.filter(isProjectionLayer).length,
      annotationMorphLayerCount: morphLayers.filter(isAnnotationLayer).length,
      sameRoomMorph: sameRoomMorph ? {
        enabled: sameRoomMorph.enabled !== false,
        groundY,
        flatY,
        channels: toArray(sameRoomMorph.channels),
      } : null,
      layers: layerReports,
    },
  };
}

function inspectMorphLayerRoom(descriptor, options) {
  if (options.annotationLayer === true) {
    const labels = Array.isArray(descriptor?.labels) ? descriptor.labels : [];
    const missing = [];
    if (!labels.length) missing.push("labels");
    const anchoredLabels = labels.filter((label) => label?.position && label?.targetPosition).length;
    if (labels.length && anchoredLabels !== labels.length) missing.push("labels.position|targetPosition");
    return {
      id: descriptor.id || descriptor.kind || "morph-layer",
      projectionLayer: false,
      annotationLayer: true,
      ok: missing.length === 0,
      missing,
      endpoints: [],
      labelCount: labels.length,
      anchoredLabelCount: anchoredLabels,
    };
  }
  const endpoints = ["position", "targetPosition"]
    .map((name) => inspectMorphEndpoint(descriptor, name, options))
    .filter(Boolean);
  const missing = [];
  if (!endpoints.length) missing.push("position|targetPosition");
  for (const endpoint of endpoints) {
    if (!endpoint.ok) missing.push(`${endpoint.channel}:${endpoint.missing.join("|")}`);
  }
  return {
    id: descriptor.id || descriptor.kind || "morph-layer",
    projectionLayer: options.projectionLayer === true,
    annotationLayer: options.annotationLayer === true,
    ok: missing.length === 0,
    missing,
    endpoints,
  };
}

function inspectMorphEndpoint(descriptor, name, options) {
  const channel = descriptor?.channels?.[name];
  const array = channelArray(channel);
  if (!array) return null;
  const itemSize = channelItemSize(channel, 3);
  if (itemSize < 3) {
    return {
      channel: name,
      ok: false,
      missing: ["itemSize>=3"],
      itemSize,
    };
  }
  const yStats = channelAxisStats(channel, 1);
  const missing = [];
  const isFlat = yStats && yStats.span <= options.epsilon;
  const channelMode = channel?.metadata?.miniatureSameRoomMorph || null;
  if (isFlat && options.projectionLayer !== true && options.annotationLayer !== true) {
    if (!Number.isFinite(options.flatY)) missing.push("fit.sameRoomMorph.flatY");
    if (Number.isFinite(options.flatY) && Math.abs(yStats.min - options.flatY) > Math.max(options.epsilon, 1e-5)) {
      missing.push("flatY");
    }
    if (Number.isFinite(options.groundY) && Number.isFinite(options.flatY) && !(options.flatY > options.groundY)) {
      missing.push("flatY>groundY");
    }
    if (!channelMode) missing.push("channel.metadata.miniatureSameRoomMorph");
  }
  return {
    channel: name,
    ok: missing.length === 0,
    missing,
    itemSize,
    y: yStats,
    flat: Boolean(isFlat),
    sameRoomMode: channelMode,
  };
}

function isMorphLayer(descriptor) {
  const animation = descriptor?.animation || {};
  return descriptor?.kind === "morph"
    || descriptor?.source?.viewKind === "morph"
    || String(animation.mode || "").includes("morph")
    || animation.channel === "targetPosition";
}

function isProjectionLayer(descriptor) {
  const text = [
    descriptor?.id,
    descriptor?.kind,
    descriptor?.primitive,
    descriptor?.metadata?.viewModel,
  ].filter(Boolean).join(" ").toLowerCase();
  return text.includes("projection") || text.includes("footprint");
}

function isAnnotationLayer(descriptor) {
  const text = [
    descriptor?.id,
    descriptor?.kind,
    descriptor?.primitive,
    descriptor?.metadata?.viewModel,
  ].filter(Boolean).join(" ").toLowerCase();
  return text.includes("label") || text.includes("annotation") || text.includes("billboard");
}

function inspectMaterials(stage, descriptors) {
  const materials = stage?.materials || {};
  const required = ["sample", "mesh", "surface", "field", "line", "ground"];
  const missing = required.filter((name) => !materials[name]);
  const styled = descriptors.filter((descriptor) => descriptor?.metadata?.miniature === true).length;
  if (descriptors.length && styled !== descriptors.length) missing.push("descriptor.metadata.miniature");
  return {
    ok: missing.length === 0,
    missing,
    evidence: {
      families: Object.keys(materials),
      descriptorCount: descriptors.length,
      styledDescriptorCount: styled,
      litDescriptorCount: descriptors.filter((descriptor) => {
        const lighting = descriptor?.material?.lighting;
        return lighting && lighting !== "unlit" && lighting !== "projection";
      }).length,
    },
  };
}

function inspectMotion(styleMotion, descriptors) {
  const motion = styleMotion?.motion || styleMotion || {};
  const enabled = styleMotion?.enabled === true || Boolean(motion.mode && motion.mode !== "none");
  const descriptorMotionCount = descriptors.filter((descriptor) => {
    const mode = descriptor?.animation?.mode;
    return mode && mode !== "none";
  }).length;
  const missing = [];
  if (!enabled && descriptorMotionCount === 0) missing.push("styleMotion|descriptor.animation");
  if (enabled && styleMotion?.hasRuntime === false) missing.push("styleMotion.runtime");
  if (enabled && styleMotion?.hasBeforeRenderHook === false) missing.push("styleMotion.beforeRenderHook");
  return {
    ok: missing.length === 0,
    missing,
    evidence: {
      enabled,
      mode: motion.mode || null,
      activeDomains: styleMotion?.activeDomains || activeMotionDomains(motion),
      hasRuntime: styleMotion?.hasRuntime ?? null,
      hasBeforeRenderHook: styleMotion?.hasBeforeRenderHook ?? null,
      descriptorMotionCount,
      lastUpdate: styleMotion?.lastUpdate || null,
    },
  };
}

function activeMotionDomains(motion = {}) {
  return {
    focus: Boolean(motion.focus && motion.focus.enabled !== false),
    camera: Boolean(motion.camera && motion.camera.enabled !== false),
    postprocess: Boolean(motion.postprocess && motion.postprocess.enabled !== false),
  };
}

function channelArray(channel) {
  if (!channel) return null;
  if (ArrayBuffer.isView(channel) || Array.isArray(channel)) return channel;
  if (ArrayBuffer.isView(channel.array) || Array.isArray(channel.array)) return channel.array;
  if (ArrayBuffer.isView(channel.data) || Array.isArray(channel.data)) return channel.data;
  return null;
}

function channelItemSize(channel, fallback = 1) {
  const itemSize = Number(channel?.itemSize ?? channel?.size ?? channel?.components);
  return Number.isFinite(itemSize) && itemSize > 0 ? Math.floor(itemSize) : fallback;
}

function channelCount(channel, fallbackItemSize = 1) {
  const count = Number(channel?.count);
  if (Number.isFinite(count) && count >= 0) return Math.floor(count);
  const array = channelArray(channel);
  if (!array) return 0;
  return Math.floor(array.length / channelItemSize(channel, fallbackItemSize));
}

function channelAxisStats(channel, axis) {
  const array = channelArray(channel);
  if (!array) return null;
  const itemSize = channelItemSize(channel, 3);
  const count = channelCount(channel, itemSize);
  if (!count || itemSize <= axis) return null;
  let min = Infinity;
  let max = -Infinity;
  for (let index = 0; index < count; index += 1) {
    const value = Number(array[index * itemSize + axis]);
    if (!Number.isFinite(value)) continue;
    min = Math.min(min, value);
    max = Math.max(max, value);
  }
  if (!Number.isFinite(min) || !Number.isFinite(max)) return null;
  return {
    min,
    max,
    span: Math.max(0, max - min),
  };
}

function point3(value) {
  if (!value) return null;
  const x = Number(value[0] ?? value.x);
  const y = Number(value[1] ?? value.y);
  const z = Number(value[2] ?? value.z);
  return [x, y, z].every(Number.isFinite) ? [x, y, z] : null;
}

function finiteOrNull(value) {
  const number = Number(value);
  return Number.isFinite(number) ? number : null;
}

function positiveNumber(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number) && number > 0) return number;
  }
  return 1;
}

function toArray(value) {
  return Array.isArray(value) ? value.filter(Boolean) : (value ? [value] : []);
}
