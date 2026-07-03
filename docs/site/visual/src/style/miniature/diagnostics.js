import { createMiniaturePhotographicRigContract } from "./rig-contract.js";

const REQUIRED_MATERIAL_FAMILIES = Object.freeze(["sample", "mesh", "surface", "field", "line", "ground"]);

/**
 * Inspects whether a miniature style application contains the reusable
 * photographic contracts expected by METRIC Visual surfaces.
 *
 * The diagnostic is renderer-facing only. It does not inspect records,
 * recompute metrics, or infer algorithmic evidence.
 *
 * @param {object} [input]
 * @param {object} [input.style]
 * @param {object} [input.stage]
 * @param {object} [input.bundle]
 * @param {object[]} [input.descriptors]
 * @param {object} [input.runtime]
 * @returns {object}
 */
export function inspectMiniatureStyleContract(input = {}) {
  const style = input.style || null;
  const stage = input.stage || style?.stage || input.bundle?.stage || null;
  const bundle = input.bundle || style?.lastBundle || null;
  const descriptors = toArray(input.descriptors || bundle?.layers || []);
  const runtime = input.runtime || style?.runtime || null;
  const styleMotion = input.styleMotion || style?.styleMotionController?.toJSON?.() || null;
  const rigContract = createMiniaturePhotographicRigContract({
    style,
    stage,
    bundle,
    descriptors,
    runtime,
    styleMotion,
  });

  const checks = {
    camera: checkCamera(stage),
    lighting: checkLighting(stage, descriptors),
    focus: checkFocus(stage, runtime),
    depthOfField: checkDepthOfField(stage, runtime),
    materials: checkMaterials(stage, descriptors),
    ground: checkGround(stage, descriptors),
    morphRoom: checkMorphRoom(rigContract),
    postprocess: checkPostprocess(stage, runtime),
    isometricStage: checkIsometricStage(stage),
    animation: checkAnimation(styleMotion, descriptors, runtime),
    photographicStyle: checkPhotographicStyle(stage, descriptors, runtime, styleMotion),
    runtime: checkRuntime(runtime),
  };
  const warnings = Object.entries(checks)
    .filter(([, check]) => !check.ok)
    .map(([key, check]) => ({
      key,
      message: check.message,
      missing: check.missing || [],
    }));
  return {
    schema: "metric.visual.miniature_style_contract_diagnostics.v1",
    ok: warnings.length === 0,
    status: warnings.length === 0 ? "ready" : "partial",
    rigContract,
    checks,
    warnings,
    coverage: {
      descriptorCount: descriptors.length,
      styledDescriptorCount: descriptors.filter((descriptor) => descriptor?.metadata?.miniature === true).length,
      primitiveFamilies: primitiveFamilies(descriptors),
      layerPrimitives: descriptors.map((descriptor) => descriptor?.primitive || descriptor?.kind).filter(Boolean),
    },
  };
}

function checkCamera(stage) {
  const camera = stage?.camera || {};
  const missing = [];
  if (!stage) missing.push("stage");
  if (!camera.position) missing.push("camera.position");
  if (!camera.target && !camera.lookAt) missing.push("camera.target");
  if (!camera.up) missing.push("camera.up");
  if (!camera.fov) missing.push("camera.fov");
  return {
    ok: missing.length === 0,
    message: missing.length ? "Missing miniature camera composition fields." : "Camera composition is defined.",
    missing,
    mode: camera.mode || camera.projection || null,
  };
}

function checkLighting(stage, descriptors) {
  const rig = stage?.lightRig || {};
  const hasAmbient = Boolean(rig.ambient);
  const hasKeyOrPoints = Boolean(rig.key || rig.fill || (Array.isArray(rig.points) && rig.points.length));
  const litDescriptors = descriptors.filter((descriptor) => {
    const lighting = descriptor?.material?.lighting;
    return lighting && lighting !== "unlit" && lighting !== "projection";
  }).length;
  const missing = [];
  if (!hasAmbient) missing.push("lightRig.ambient");
  if (!hasKeyOrPoints) missing.push("lightRig.key|points|fill");
  return {
    ok: missing.length === 0,
    message: missing.length ? "Missing stage light rig roles." : "Stage light rig is defined.",
    missing,
    litDescriptorCount: litDescriptors,
  };
}

function checkFocus(stage, runtime) {
  const focus = stage?.focus || {};
  const runtimeFocus = runtime?.focusLine?.toJSON?.() || null;
  const missing = [];
  if (!focus.kind && !focus.axis) missing.push("stage.focus");
  if (focus.enabled === false) missing.push("focus.enabled");
  return {
    ok: missing.length === 0,
    message: missing.length ? "Missing focus-plane contract." : "Focus-plane contract is defined.",
    missing,
    axis: focus.axis || runtimeFocus?.axis || null,
    runtimeRadius: runtimeFocus?.radius ?? null,
    runtimeFocusBand: runtimeFocus?.focusBand ?? null,
    runtimeBlurCurve: runtimeFocus?.blurCurve ?? null,
  };
}

function checkDepthOfField(stage, runtime) {
  const postprocess = stage?.postprocess || {};
  const postFx = postprocess.postFx || {};
  const cameraDof = postFx.cameraDof || postprocess.cameraDof;
  const tiltShift = postprocess.tiltShift;
  const nativePasses = runtime?.postprocess?.native?.passes?.map((pass) => pass?.constructor?.name).filter(Boolean) || [];
  const stack = runtime?.postprocess?.postFxStack || null;
  const cameraDofPass = stack?.cameraDof || runtime?.postprocess?.cameraDofPass || null;
  const tiltShiftPass = runtime?.postprocess?.tiltShiftPass || null;
  const hasCameraDof = cameraDof !== false && (Boolean(cameraDof) || nativePasses.includes("MiniatureCameraDofPass") || Boolean(cameraDofPass));
  const hasTiltShift = tiltShift !== false && (Boolean(tiltShift) || nativePasses.includes("TiltShiftPass"));
  const missing = hasCameraDof || hasTiltShift ? [] : ["postFx.cameraDof|MiniatureCameraDofPass"];
  return {
    ok: missing.length === 0,
    message: missing.length ? "Missing camera-depth depth-of-field pass." : "Depth-of-field contract is active.",
    missing,
    nativePasses,
    cameraDofEnabled: cameraDofPass?.enabled ?? cameraDof?.enabled ?? null,
    cameraDofFocusDistance: cameraDofPass?.dof?.focusDistance ?? cameraDof?.focusDistance ?? null,
    cameraDofDepthNear: cameraDofPass?.dof?.depthNear ?? cameraDof?.depthNear ?? null,
    cameraDofDepthFar: cameraDofPass?.dof?.depthFar ?? cameraDof?.depthFar ?? null,
    cameraDofAperture: cameraDofPass?.dof?.aperture ?? cameraDof?.aperture ?? null,
    cameraDofHasDepthTexture: Boolean(cameraDofPass?.depthTexture || cameraDofPass?.currentDepthTexture || runtime?.postprocess?.cameraDepthTexture),
    blurRadius: tiltShiftPass?.blurRadius ?? tiltShift?.blurRadius ?? postprocess.blurRadius ?? null,
    gradientRadius: tiltShiftPass?.gradientRadius ?? tiltShift?.gradientRadius ?? postprocess.gradientRadius ?? null,
    focusBand: tiltShiftPass?.focusBand ?? tiltShift?.focusBand ?? null,
    blurCurve: tiltShiftPass?.blurCurve ?? tiltShift?.blurCurve ?? null,
  };
}

function checkMaterials(stage, descriptors) {
  const materialFamilies = stage?.materials || {};
  const missing = REQUIRED_MATERIAL_FAMILIES.filter((family) => !materialFamilies[family]);
  const styled = descriptors.filter((descriptor) => descriptor?.metadata?.miniature === true).length;
  return {
    ok: missing.length === 0 && (!descriptors.length || styled > 0),
    message: missing.length
      ? "Missing material family presets."
      : "Material family presets are defined.",
    missing,
    styledDescriptorCount: styled,
    families: Object.keys(materialFamilies),
  };
}

function checkGround(stage, descriptors) {
  const grounding = stage?.grounding || {};
  const hasGroundLayer = descriptors.some((descriptor) => descriptor?.primitive === "GroundPlaneLayer");
  const hasProjection = descriptors.some((descriptor) => descriptor?.primitive === "GroundProjectionLayer");
  const missing = [];
  if (grounding.enabled === false || !grounding.kind) missing.push("stage.grounding");
  if (!hasGroundLayer && descriptors.length) missing.push("GroundPlaneLayer");
  return {
    ok: missing.length === 0,
    message: missing.length ? "Missing miniature ground relation." : "Ground relation is present.",
    missing,
    hasGroundLayer,
    hasProjection,
    groundY: grounding.groundY ?? null,
  };
}

function checkMorphRoom(rigContract) {
  const section = rigContract?.sections?.morphRoom || {};
  const missing = section.missing || [];
  return {
    ok: section.ok !== false,
    message: missing.length
      ? "Missing same-room morph staging for flat 2D/3D endpoints."
      : "Morph endpoints share the miniature room.",
    missing,
    morphLayerCount: section.evidence?.morphLayerCount ?? 0,
    sameRoomMorph: section.evidence?.sameRoomMorph || null,
  };
}

function checkPostprocess(stage, runtime) {
  const postFx = stage?.postprocess?.postFx || {};
  const stack = runtime?.postprocess?.postFxStack || null;
  const framePass = stack?.miniatureFrame || null;
  const cameraDofPass = stack?.cameraDof || null;
  const bloomPass = stack?.bloom || null;
  const stackPasses = stack?.passes?.().map((pass) => pass?.constructor?.name).filter(Boolean) || [];
  const runtimeSize = runtimePostprocessSize(runtime);
  const unsizedPasses = stack?.passes?.()
    .filter((pass) => pass && pass.enabled !== false)
    .filter((pass) => !passSizeMatches(pass, runtimeSize))
    .map((pass) => pass.constructor?.name || "UnknownPass") || [];
  const missing = [];
  if (postFx.enabled === false && !stack) missing.push("postprocess.postFx");
  if (!postFx.miniatureFrame && !stackPasses.includes("MiniatureFramePass")) missing.push("postFx.miniatureFrame");
  if (!postFx.grade && !stackPasses.includes("ColorGradePass")) missing.push("postFx.grade");
  if (!postFx.vignette && !stackPasses.includes("VignettePass")) missing.push("postFx.vignette");
  if (unsizedPasses.length) missing.push("postFx.passSize");
  return {
    ok: missing.length === 0,
    message: missing.length ? "Missing final photographic post-FX components." : "Final post-FX stack is defined.",
    missing,
    stackPasses,
    runtimeSize,
    unsizedPasses,
    cameraDofEnabled: cameraDofPass?.enabled ?? postFx.cameraDof?.enabled ?? null,
    cameraDofFocusDistance: cameraDofPass?.dof?.focusDistance ?? postFx.cameraDof?.focusDistance ?? null,
    cameraDofDepthNear: cameraDofPass?.dof?.depthNear ?? postFx.cameraDof?.depthNear ?? null,
    cameraDofDepthFar: cameraDofPass?.dof?.depthFar ?? postFx.cameraDof?.depthFar ?? null,
    bloomEnabled: bloomPass?.enabled ?? postFx.bloom?.enabled ?? null,
    bloomIntensity: bloomPass?.settings?.intensity ?? postFx.bloom?.intensity ?? postFx.bloom?.strength ?? null,
    bloomThreshold: bloomPass?.settings?.threshold ?? postFx.bloom?.threshold ?? null,
    miniatureFrameFocusBand: framePass?.uniformValues?.uFocusBand ?? null,
    miniatureFrameBlurCurve: framePass?.uniformValues?.uFocusBlurCurve ?? null,
    miniatureFrameStageSpotlight: framePass?.uniformValues?.uStageSpotlight ?? null,
    miniatureFrameStageShadow: framePass?.uniformValues?.uStageShadow ?? null,
    miniatureFrameLensAberration: framePass?.uniformValues?.uLensAberration ?? postFx.miniatureFrame?.lensAberration ?? null,
    miniatureFrameApertureVignette: framePass?.uniformValues?.uApertureVignette ?? postFx.miniatureFrame?.apertureVignette ?? null,
    miniatureFrameFocusDesaturation: framePass?.uniformValues?.uFocusDesaturation ?? postFx.miniatureFrame?.focusDesaturation ?? null,
  };
}

function checkIsometricStage(stage) {
  const camera = stage?.camera || {};
  const up = point3(camera.up);
  const position = point3(camera.position);
  const target = point3(camera.target || camera.lookAt);
  const hasMetricGround = camera.coordinateSystem === "metric-xz-ground";
  const hasTabletopUp = Math.abs(up[2]) > Math.max(Math.abs(up[0]), Math.abs(up[1]));
  const hasObliqueView = Math.abs(position[1] - target[1]) > 0.1 && Math.abs(position[2] - target[2]) > 0.1;
  const missing = [];
  if (!hasMetricGround) missing.push("camera.coordinateSystem=metric-xz-ground");
  if (!hasTabletopUp) missing.push("camera.up tabletop z-axis");
  if (!hasObliqueView) missing.push("camera oblique position");
  return {
    ok: missing.length === 0,
    message: missing.length ? "Missing tabletop/isometric stage cues." : "Tabletop/isometric staging is defined.",
    missing,
  };
}

function checkAnimation(styleMotion, descriptors, runtime) {
  const motion = styleMotion?.motion || styleMotion || {};
  const styleMotionEnabled = isStyleMotionEnabled(styleMotion);
  const descriptorMotion = descriptors.filter((descriptor) => {
    const mode = descriptor?.animation?.mode;
    return mode && mode !== "none";
  }).length;
  const runtimeState = runtime?.getState?.() || {};
  const domains = styleMotion?.activeDomains || activeMotionDomains(motion);
  const activeDomainCount = Object.values(domains).filter(Boolean).length;
  const missing = [];
  if (!styleMotionEnabled && descriptorMotion === 0) missing.push("styleMotion|descriptor.animation");
  if (styleMotionEnabled && activeDomainCount === 0) missing.push("styleMotion.activeDomains");
  if (runtime && styleMotionEnabled && styleMotion?.hasRuntime !== true) missing.push("styleMotion.runtime");
  if (runtime && styleMotionEnabled && styleMotion?.hasBase !== true) missing.push("styleMotion.base");
  if (runtime && styleMotionEnabled && styleMotion?.hasBeforeRenderHook !== true) missing.push("styleMotion.beforeRenderHook");
  if (runtimeState.running === true && styleMotionEnabled && !styleMotion?.lastUpdate) missing.push("styleMotion.lastUpdate");
  return {
    ok: missing.length === 0,
    message: missing.length === 0
      ? "Animation contract is present."
      : animationDiagnosticMessage(missing),
    missing,
    styleMotionEnabled,
    activeDomains: domains,
    activeDomainCount,
    hasRuntime: styleMotion?.hasRuntime === true,
    hasBase: styleMotion?.hasBase === true,
    hasBeforeRenderHook: styleMotion?.hasBeforeRenderHook === true,
    lastUpdate: styleMotion?.lastUpdate || null,
    animatedDescriptorCount: descriptorMotion,
  };
}

function checkRuntime(runtime) {
  if (!runtime) {
    return {
      ok: true,
      message: "Runtime not attached; static style contract inspected.",
      missing: [],
      attached: false,
    };
  }
  const state = runtime.getState?.() || {};
  const missing = [];
  if (!runtime.postprocess) missing.push("runtime.postprocess");
  if (!runtime.focusLine) missing.push("runtime.focusLine");
  return {
    ok: missing.length === 0,
    message: missing.length ? "Runtime is missing style hooks." : "Runtime style hooks are attached.",
    missing,
    attached: true,
    layerInstanceCount: state.layerInstanceCount ?? null,
  };
}

function checkPhotographicStyle(stage, descriptors, runtime, styleMotion) {
  const styleMotionEnabled = isStyleMotionEnabled(styleMotion);
  const camera = stage?.camera || {};
  const lightRig = stage?.lightRig || {};
  const focus = runtime?.focusLine?.toJSON?.() || stage?.focus || {};
  const tiltShiftPass = runtime?.postprocess?.tiltShiftPass || null;
  const postprocess = stage?.postprocess || {};
  const postFx = postprocess.postFx || {};
  const stack = runtime?.postprocess?.postFxStack || null;
  const framePass = stack?.miniatureFrame || null;
  const cameraDofPass = stack?.cameraDof || null;
  const bloomPass = stack?.bloom || null;
  const layerPrimitives = descriptors.map((descriptor) => descriptor?.primitive || descriptor?.kind).filter(Boolean);
  const hasCurveLayer = layerPrimitives.some((primitive) => isCurvePrimitive(primitive));
  const hasCurveContact = descriptors.some((descriptor) => {
    const primitive = descriptor?.metadata?.contactSourcePrimitive || descriptor?.source?.contactSourcePrimitive;
    return descriptor?.primitive === "GroundProjectionLayer"
      && descriptor?.metadata?.pathContactShadow === true
      && isCurvePrimitive(primitive);
  });
  const styledDescriptorCount = descriptors.filter((descriptor) => descriptor?.metadata?.miniature === true).length;
  const litDescriptorCount = descriptors.filter((descriptor) => {
    const lighting = descriptor?.material?.lighting;
    return lighting && lighting !== "unlit" && lighting !== "projection";
  }).length;
  const materialToneFamilies = materialToneFamilyCount(stage?.materials || {});
  const descriptorMotion = descriptors.filter((descriptor) => {
    const mode = descriptor?.animation?.mode;
    return mode && mode !== "none";
  }).length;
  const lightCount = lightRigCount(lightRig);
  const runtimeSize = runtimePostprocessSize(runtime);
  const focusBand = number(focus.focusBand ?? tiltShiftPass?.focusBand ?? stage?.focus?.focusBand);
  const blurCurve = number(focus.blurCurve ?? tiltShiftPass?.blurCurve ?? stage?.focus?.blurCurve);
  const frameFocusBand = number(framePass?.uniformValues?.uFocusBand);
  const frameBlurCurve = number(framePass?.uniformValues?.uFocusBlurCurve);
  const frameStageSpotlight = number(framePass?.uniformValues?.uStageSpotlight ?? postFx.miniatureFrame?.stageSpotlight);
  const frameStageShadow = number(framePass?.uniformValues?.uStageShadow ?? postFx.miniatureFrame?.stageShadow);
  const frameLensAberration = number(framePass?.uniformValues?.uLensAberration ?? postFx.miniatureFrame?.lensAberration);
  const frameApertureVignette = number(framePass?.uniformValues?.uApertureVignette ?? postFx.miniatureFrame?.apertureVignette);
  const frameFocusDesaturation = number(framePass?.uniformValues?.uFocusDesaturation ?? postFx.miniatureFrame?.focusDesaturation);
  const bloomEnabled = bloomPass?.enabled ?? postFx.bloom?.enabled ?? false;
  const bloomIntensity = number(bloomPass?.settings?.intensity ?? postFx.bloom?.intensity ?? postFx.bloom?.strength);
  const bloomThreshold = number(bloomPass?.settings?.threshold ?? postFx.bloom?.threshold);
  const cameraDofEnabled = cameraDofPass?.enabled ?? postFx.cameraDof?.enabled ?? false;
  const cameraDofFocusDistance = number(cameraDofPass?.dof?.focusDistance ?? postFx.cameraDof?.focusDistance);
  const cameraDofAperture = number(cameraDofPass?.dof?.aperture ?? postFx.cameraDof?.aperture);
  const cameraDofMaxBlur = number(cameraDofPass?.dof?.maxBlur ?? postFx.cameraDof?.maxBlur);
  const blurRadius = number(tiltShiftPass?.blurRadius ?? postprocess.blurRadius ?? postprocess.tiltShift?.blurRadius);
  const gradientRadius = number(
    focus.radius
      ?? tiltShiftPass?.gradientRadius
      ?? postprocess.gradientRadius
      ?? postprocess.tiltShift?.gradientRadius,
  );
  const cameraFov = number(camera.fov);
  const stackPasses = stack?.passes?.().map((pass) => pass?.constructor?.name).filter(Boolean) || [];
  const signals = {
    camera: Boolean(
      camera.projection === "perspective"
        && camera.coordinateSystem === "metric-xz-ground"
        && cameraFov >= 22
        && cameraFov <= 58
        && camera.position
        && (camera.target || camera.lookAt)
        && camera.up,
    ),
    lighting: Boolean(lightRig.ambient && lightCount >= 2),
    focusOptics: Boolean(
      (cameraDofEnabled !== false && cameraDofFocusDistance > 0 && cameraDofAperture > 0 && cameraDofMaxBlur > 0)
        || (focusBand > 0 && blurCurve >= 1 && blurRadius > 0 && gradientRadius > focusBand),
    ),
    frameFocusOptics: Boolean(!runtime || !framePass || (frameFocusBand > 0 && frameBlurCurve >= 1)),
    frameStageComposition: Boolean(!runtime || !framePass || (frameStageSpotlight > 0 && frameStageShadow > 0)),
    photographicLensCues: Boolean(!runtime || !framePass || (frameLensAberration >= 0 && frameApertureVignette > 0 && frameFocusDesaturation >= 0)),
    ground: Boolean(stage?.grounding?.kind && (!descriptors.length || layerPrimitives.includes("GroundPlaneLayer"))),
    groundProjection: Boolean(!descriptors.length || layerPrimitives.includes("GroundProjectionLayer")),
    curveGroundContact: Boolean(!hasCurveLayer || hasCurveContact),
    materials: Boolean(stage?.materials && (!descriptors.length || styledDescriptorCount === descriptors.length) && (!descriptors.length || litDescriptorCount > 0)),
    materialColorContinuity: materialToneFamilies >= 4,
    postprocess: Boolean(
      postFx.enabled !== false
        && (postFx.miniatureFrame || stackPasses.includes("MiniatureFramePass"))
        && (postFx.cameraDof || stackPasses.includes("MiniatureCameraDofPass") || tiltShiftPass)
        && (postFx.grade || stackPasses.includes("ColorGradePass"))
        && (postFx.vignette || stackPasses.includes("VignettePass")),
    ),
    highlightBloom: Boolean(!runtime || !stack || (bloomEnabled !== false && bloomIntensity > 0 && bloomThreshold >= 0)),
    postprocessSize: Boolean(!runtime || !stack || stack.passes?.().every((pass) => passSizeMatches(pass, runtimeSize))),
    animation: Boolean(
      styleMotionEnabled
        ? styleMotion?.hasRuntime !== false && styleMotion?.hasBeforeRenderHook !== false
        : descriptorMotion > 0 || !descriptors.length,
    ),
  };
  const missing = Object.entries(signals)
    .filter(([, ok]) => !ok)
    .map(([key]) => key);
  return {
    ok: missing.length === 0,
    message: missing.length
      ? "Photographic miniature style signals are incomplete."
      : "Photographic miniature style signals are present.",
    missing,
    signals,
    evidence: {
      cameraFov,
      lightCount,
      focusBand,
      blurCurve,
      frameFocusBand,
      frameBlurCurve,
      frameStageSpotlight,
      frameStageShadow,
      frameLensAberration,
      frameApertureVignette,
      frameFocusDesaturation,
      bloomEnabled,
      bloomIntensity,
      bloomThreshold,
      blurRadius,
      gradientRadius,
      styledDescriptorCount,
      descriptorCount: descriptors.length,
      litDescriptorCount,
      materialToneFamilies,
      hasCurveLayer,
      hasCurveContact,
      layerPrimitives,
      stackPasses,
      runtimeSize,
      descriptorMotion,
      styleMotionEnabled,
    },
  };
}

function primitiveFamilies(descriptors) {
  const out = {};
  for (const descriptor of descriptors) {
    const family = descriptor?.metadata?.miniatureFamily || descriptor?.metadata?.miniatureRole || "unknown";
    out[family] = (out[family] || 0) + 1;
  }
  return out;
}

function point3(value) {
  if (Array.isArray(value) || ArrayBuffer.isView(value)) {
    return [number(value[0]), number(value[1]), number(value[2])];
  }
  return [number(value?.x), number(value?.y), number(value?.z)];
}

function runtimePostprocessSize(runtime) {
  const size = runtime?.renderer?.size || {};
  const width = number(size.drawingBufferWidth ?? runtime?.canvas?.width);
  const height = number(size.drawingBufferHeight ?? runtime?.canvas?.height);
  return {
    width: Math.max(1, Math.floor(width || 1)),
    height: Math.max(1, Math.floor(height || 1)),
  };
}

function passSizeMatches(pass, size) {
  if (!pass || !size) return true;
  const width = number(pass.width);
  const height = number(pass.height);
  return width === size.width && height === size.height;
}

function activeMotionDomains(motion = {}) {
  return {
    focus: Boolean(motion?.focus && motion.focus.enabled !== false),
    camera: Boolean(motion?.camera && motion.camera.enabled !== false),
    postprocess: Boolean(motion?.postprocess && motion.postprocess.enabled !== false),
  };
}

function isStyleMotionEnabled(styleMotion) {
  if (!styleMotion) return false;
  if (styleMotion.enabled === true) return true;
  if (styleMotion.enabled === false) return false;
  const motion = styleMotion.motion || styleMotion;
  return Boolean(motion?.mode && motion.mode !== "none");
}

function lightRigCount(rig = {}) {
  let count = 0;
  if (rig.key) count += 1;
  if (rig.fill) count += 1;
  if (Array.isArray(rig.points)) count += rig.points.length;
  return count;
}

function materialToneFamilyCount(materials = {}) {
  let count = 0;
  for (const material of Object.values(materials || {})) {
    if (!material || typeof material !== "object") continue;
    const hasShadow = Boolean(material.shadowTint || material.shadowColor);
    const hasHighlight = Boolean(material.highlightColor || material.focusColor || material.glow);
    const hasSaturation = Number.isFinite(Number(material.saturation)) || Boolean(material.scalarColorMap || material.scalarMap);
    const hasFloorTone = Boolean(material.baseColor && (material.gridColor || material.sheenColor || material.horizonColor));
    if ((hasShadow && hasHighlight) || (hasHighlight && hasSaturation) || hasFloorTone) {
      count += 1;
    }
  }
  return count;
}

function isCurvePrimitive(primitive) {
  return primitive === "CurveTubeMeshLayer"
    || primitive === "CurveRibbonLayer"
    || primitive === "CurvePolylineLayer";
}

function animationDiagnosticMessage(missing) {
  return missing.includes("styleMotion|descriptor.animation")
    ? "No style or descriptor animation is active."
    : "Animation contract is incomplete.";
}

function number(value) {
  const next = Number(value);
  return Number.isFinite(next) ? next : 0;
}

function toArray(value) {
  return Array.isArray(value) ? value.filter(Boolean) : (value ? [value] : []);
}
