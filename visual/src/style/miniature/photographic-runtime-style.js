import {
  applyMiniatureSceneBundle,
  createMiniatureSceneBundle,
} from "./composition.js";
import { createMiniatureFocusPlane } from "./focus.js";
import { createMiniatureHeroStage } from "./hero.js";
import { createMetricMiniaturePalette } from "./palette.js";
import { createMiniatureRuntimeOptions } from "./runtime-options.js";
import { createMiniaturePhotographicStyle } from "./style.js";
import { createPhotographicCameraDof } from "../../postfx/spatial-focus.js";

export const PHOTOGRAPHIC_RUNTIME_STYLE_SCHEMA = "metric.visual.photographic_runtime_style.v1";
export const PHOTOGRAPHIC_RUNTIME_SETUP_SCHEMA = "metric.visual.photographic_runtime_setup.v1";
export const HOLD_MORPH_HOLD_TIMELINE_SCHEMA = "metric.visual.hold_morph_hold_timeline.v1";

export const DEMO5_TILT_SHIFT_SOURCE_CONTRACT = deepFreeze({
  source: "/tmp/metric-trois-site/src/components/demos/Demo5.vue",
  renderer: {
    resize: true,
    pointer: {
      onMove: "updateTilt",
      normalizedPosition: true,
      focusAxis: "viewport-y",
    },
  },
  camera: {
    position: [0, -20, 10],
    target: [0, 0, 0],
    up: [0, 1, 0],
  },
  scene: {
    background: "#ffffff",
  },
  lights: {
    ambient: {
      enabled: true,
    },
    point: {
      position: [0, 0, 20],
    },
  },
  material: {
    primitive: "InstancedMesh",
    material: "PhongMaterial",
    vertexColors: true,
  },
  postprocess: {
    effectComposer: ["RenderPass", "FXAAPass", "TiltShiftPass"],
    tiltShift: {
      gradientRadius: "size.height / 3",
      y: "size.height / 2",
      start: [0, "tiltY"],
      end: [100, "tiltY"],
      pointerY: "(positionN.y + 1) * 0.5 * size.height",
    },
  },
});

const DEFAULT_CAMERA = deepFreeze({
  mode: "miniature-photographic",
  coordinateSystem: "metric-y-up-xz-ground",
  projection: "perspective",
  fov: 42,
  near: 0.05,
  far: 2000,
  position: [0, -20, 10],
  target: [0, 0, 0],
  lookAt: [0, 0, 0],
  up: [0, 1, 0],
  orbit: {
    enableDamping: true,
    dampingFactor: 0.055,
    autoRotate: false,
    minPitch: -1.24,
    maxPitch: 1.18,
    minDistance: 2,
    maxDistance: 2000,
  },
});

const DEFAULT_FOCUS = deepFreeze({
  axis: "horizontal",
  yRatio: 0.5,
  radiusRatio: 1 / 3,
  focusBandRatio: 0.045,
  blurRadius: 5.6,
  blurCurve: 1.32,
  pointer: {
    enabled: true,
    axis: "horizontal",
    mode: "viewport-y-focus-line",
    normalizedInput: true,
    preserveRadius: true,
  },
});

const DEFAULT_GROUNDING = deepFreeze({
  enabled: true,
  plane: "xz",
  groundY: -0.62,
  ground: {
    alpha: 0.38,
    gridScale: 8,
    gridAlpha: 0.18,
    axisAlpha: 0.12,
    fade: 0.9,
    stageTilt: -0.08,
    stageScale: [0.92, 0.8],
    sheen: 0.04,
    matte: 0.88,
    contactShade: 0.12,
    horizonFade: 0.1,
  },
  projection: {
    alpha: 0.46,
    softness: 0.82,
    pointPixelScale: 12,
    minPointSize: 1.5,
    maxPointSize: 96,
    footprintStretch: 1.8,
    footprintSkew: -0.28,
    shadowDensity: 1.08,
    coreDensity: 0.36,
    colorMix: 0.58,
    surfaceLift: 0.018,
  },
  contact: {
    enabled: true,
    alpha: 0.38,
    radiusScale: 1.12,
    shadowDensity: 1.18,
    coreDensity: 0.38,
    pointPixelScale: 14,
    includeSamples: false,
  },
});

const DEFAULT_LIGHT_RIG = deepFreeze({
  mode: "demo5-ambient-point",
  ambient: {
    color: "#ffffff",
    intensity: 0.58,
  },
  key: {
    color: "#ffffff",
    intensity: 0.86,
    position: [0, 0, 20],
  },
  point: {
    color: "#ffffff",
    intensity: 0.86,
    position: [0, 0, 20],
  },
});

const DEFAULT_MATERIALS = deepFreeze({
  sample: {
    family: "sample",
    lighting: "phong-like",
    dataColor: true,
    vertexColors: true,
    ambient: 0.56,
    pointLight: 0.34,
    specular: 0.06,
    sphereShade: 0.56,
    gloss: 0.16,
    edgeShade: 0.34,
    saturation: 0.92,
    alpha: 0.9,
    alphaMode: "blend",
    pointPixelScale: 5.8,
    minPointSize: 1.25,
    maxPointSize: 26,
    focusBoost: 0.24,
    phasePulse: 0.025,
  },
  mesh: {
    family: "mesh",
    lighting: "phong-like",
    dataColor: true,
    vertexColors: true,
    ambient: 0.58,
    pointLight: 0.58,
    specular: 0.12,
    specularPower: 38,
    metalness: 0.04,
    roughness: 0.62,
    saturation: 0.94,
    shadowSoftness: 0.24,
    faceContrast: 0.28,
    verticalGradient: 0.14,
    alpha: 1,
    alphaMode: "opaque",
  },
  surface: {
    family: "surface",
    lighting: "physical-like",
    dataColor: true,
    ambient: 0.54,
    pointLight: 0.42,
    specular: 0.12,
    roughness: 0.54,
    metalness: 0.1,
    saturation: 0.94,
    alpha: 0.96,
  },
  field: {
    family: "field",
    lighting: "screen-space",
    alpha: 0.72,
    alphaMode: "blend",
    feather: 0.28,
    contour: 0.16,
    glow: 0.08,
  },
  line: {
    family: "line",
    lighting: "screen-space-ribbon",
    alpha: 0.72,
    alphaMode: "blend",
    ambient: 0.44,
    pointLight: 0.36,
    emission: 0.12,
    rimLight: 0.2,
    coreGlow: 0.12,
    thickness: 1.2,
    feather: 0.3,
    depthShade: 0.18,
  },
  ground: {
    family: "ground",
    lighting: "unlit",
    alpha: 0.38,
    gridScale: 8,
    gridAlpha: 0.18,
    axisAlpha: 0.12,
    fade: 0.9,
    matte: 0.88,
    sheen: 0.04,
    contactShade: 0.12,
    baseColor: "#f7f8f4",
    gridColor: "#cbd4cf",
    axisXColor: "#8fa9b2",
    axisZColor: "#b9a88e",
  },
});

const DEFAULT_POSTPROCESS = deepFreeze({
  enabled: true,
  antialias: "fxaa",
  fxaa: {
    enabled: true,
  },
  bloom: {
    enabled: false,
    intensity: 0.08,
    strength: 0.08,
    radius: 0.9,
    threshold: 0.94,
  },
  grade: {
    enabled: true,
    exposure: 0,
    contrast: 1.03,
    saturation: 0.94,
    vibrance: 0.06,
    temperature: 0.035,
    tint: -0.01,
    highlightCompression: 0.12,
  },
  vignette: {
    enabled: true,
    amount: 0.06,
    radius: 0.86,
    softness: 0.74,
    color: [0.88, 0.9, 0.86],
    exposurePreserve: 0.22,
  },
  miniatureFrame: {
    enabled: true,
    amount: 0.32,
    focusClarity: 0.16,
    focusLift: 0.024,
    stageGlow: 0.06,
    groundShade: 0.08,
    edgeFade: 0.08,
    warmth: 0.04,
    floorRatio: 0.52,
    stageCenter: [0.5, 0.5],
    stageScale: [0.9, 0.74],
    stageTilt: -0.08,
    subjectIsolation: 0.22,
    microContrast: 0.16,
    lensAberration: 0.004,
    apertureVignette: 0.07,
    focusDesaturation: 0.03,
  },
  filmGrain: {
    enabled: false,
    intensity: 0.01,
    size: 1.2,
  },
});

const DEFAULT_CAMERA_DOF = deepFreeze({
  enabled: true,
  amount: 0.72,
  focusDistance: 22.36,
  depthNear: 0.05,
  depthFar: 2000,
  cameraNear: 0.05,
  cameraFar: 2000,
  aperture: 0.54,
  maxBlur: 5.2,
  focalRange: 0.12,
  quietDesaturation: 0.12,
  quietExposure: 0.045,
  focusClarity: 0.035,
  vignette: 0.08,
});

const DEFAULT_RUNTIME_RENDERER = deepFreeze({
  autoResize: true,
  autoStart: false,
  pointer: true,
  updateStyle: false,
});

const DEFAULT_RUNTIME_CONTROLS = deepFreeze({
  enabled: true,
  mode: "miniaturePerspective",
  preventDefault: true,
  updateFocusOnMove: true,
  focusAxis: "horizontal",
});

const DEFAULT_HOVER_FOCUS = deepFreeze({
  enabled: false,
  thresholdPx: 36,
  throttleMs: 48,
  maxCandidates: 60000,
  clearOnLeave: false,
  clearOnMiss: false,
  focusWhileDragging: false,
  positionChannels: ["position", "targetPosition", "sourcePosition"],
  recordChannel: "recordId",
});

const DEFAULT_FIT = deepFreeze({
  enabled: true,
  targetSpan: 9.6,
  axes: {
    x: 0,
    y: 1,
    z: 2,
  },
  preserveY: true,
  sameRoomMorph: {
    groundY: DEFAULT_GROUNDING.groundY,
    flatY: DEFAULT_GROUNDING.groundY + 0.09,
    reprojectFlatZ: true,
    channels: ["position", "targetPosition"],
  },
});

const DEFAULT_GROUND = deepFreeze({
  groundY: DEFAULT_GROUNDING.groundY,
  y: DEFAULT_GROUNDING.groundY,
  padding: 1.8,
  plane: "xz",
});

const DEFAULT_TIMELINE = deepFreeze({
  hold2dMs: 2200,
  morphTo3dMs: 850,
  hold3dMs: 3200,
  morphTo2dMs: 850,
  easing: "smoothstep",
  loop: true,
});

export function createDemo5TiltShiftSourceContract(options = {}) {
  return deepFreeze(deepMerge(DEMO5_TILT_SHIFT_SOURCE_CONTRACT, options));
}

export function createPhotographicMetricFocusOptions(options = {}) {
  return createMiniatureFocusPlane(deepMerge(DEFAULT_FOCUS, options));
}

export function createPhotographicMetricPostprocessOptions(options = {}) {
  const focus = options.focus?.kind === "miniature-focus-plane"
    ? options.focus
    : createPhotographicMetricFocusOptions(options.focus || options.tiltShift || {});
  const camera = deepMerge(DEFAULT_CAMERA, options.camera || {});
  const cameraDofOverride = options.cameraDof === false || options.depthOfField === false
    ? { enabled: false }
    : (options.cameraDof || options.depthOfField || {});
  const cameraDof = createPhotographicCameraDof(deepMerge(DEFAULT_CAMERA_DOF, {
    cameraNear: camera.near,
    cameraFar: camera.far,
    ...cameraDofOverride,
  }));
  const tiltShift = {
    enabled: true,
    kind: "tilt-shift",
    focusKind: focus.kind,
    passes: 2,
    blurRadius: finiteNumber(options.blurRadius, options.tiltShift?.blurRadius, focus.blurRadius, DEFAULT_FOCUS.blurRadius),
    gradientRadius: finiteNumber(options.gradientRadius, options.tiltShift?.gradientRadius, focus.gradientRadius, focus.radius),
    focusBand: finiteNumber(options.focusBand, options.tiltShift?.focusBand, focus.focusBand, 0),
    focusBandRatio: finiteNumber(options.focusBandRatio, options.tiltShift?.focusBandRatio, focus.focusBandRatio, 0),
    blurCurve: finiteNumber(options.blurCurve, options.tiltShift?.blurCurve, focus.blurCurve, DEFAULT_FOCUS.blurCurve),
    start: clonePlain(focus.start),
    end: clonePlain(focus.end),
    axis: focus.axis,
    pointer: clonePlain(focus.pointer),
    ...(isPlainObject(options.tiltShift) ? options.tiltShift : {}),
  };
  const postFx = options.postFx === false ? false : deepMerge({
    enabled: true,
    bloom: deepMerge(DEFAULT_POSTPROCESS.bloom, options.bloom || {}),
    cameraDof,
    miniatureFrame: deepMerge(DEFAULT_POSTPROCESS.miniatureFrame, options.miniatureFrame || {}),
    grade: deepMerge(DEFAULT_POSTPROCESS.grade, options.grade || options.colorGrade || {}),
    vignette: deepMerge(DEFAULT_POSTPROCESS.vignette, options.vignette || {}),
    filmGrain: deepMerge(DEFAULT_POSTPROCESS.filmGrain, options.filmGrain || options.film || {}),
  }, isPlainObject(options.postFx) ? options.postFx : {});

  return deepFreeze(deepMerge(DEFAULT_POSTPROCESS, {
    enabled: options.enabled !== false,
    fxaa: normalizeEnabledObject(options.fxaa, DEFAULT_POSTPROCESS.fxaa),
    tiltShift,
    blurRadius: tiltShift.blurRadius,
    gradientRadius: tiltShift.gradientRadius,
    focusBand: tiltShift.focusBand,
    blurCurve: tiltShift.blurCurve,
    start: clonePlain(tiltShift.start),
    end: clonePlain(tiltShift.end),
    cameraDof,
    postFx,
  }));
}

export function createPhotographicMetricStage(options = {}) {
  const stageOptions = options.stageOptions || {};
  const palette = options.palette || createMetricMiniaturePalette({
    background: "#ffffff",
    ...(options.paletteOptions || stageOptions.paletteOptions || {}),
  });
  const focus = createPhotographicMetricFocusOptions(
    deepMerge(stageOptions.focus || stageOptions.tiltShift || {}, options.focus || options.tiltShift || {}),
  );
  const camera = deepMerge(DEFAULT_CAMERA, deepMerge(stageOptions.camera || {}, options.camera || {}));
  const grounding = deepMerge(DEFAULT_GROUNDING, deepMerge(stageOptions.grounding || {}, options.grounding || {}));
  const materials = deepMerge(DEFAULT_MATERIALS, deepMerge(stageOptions.materials || {}, options.materials || {}));
  const sourceContract = createDemo5TiltShiftSourceContract(options.demo5Contract || options.sourceContract || {});
  const baseStage = createMiniatureHeroStage({
    ...stageOptions,
    id: options.id || stageOptions.id || "metric-photographic-runtime-stage",
    variant: options.variant || stageOptions.variant || "white",
    palette,
    background: options.background || stageOptions.background || "#ffffff",
    camera,
    focus,
    grounding,
    materials,
    postprocess: deepMerge(DEFAULT_POSTPROCESS, deepMerge(stageOptions.postprocess || {}, options.postprocess || {})),
    interaction: deepMerge({
      pointerFocus: true,
      orbitDamping: true,
      focusLine: focus,
    }, deepMerge(stageOptions.interaction || {}, options.interaction || {})),
    metadata: deepMerge({
      reusablePhotographicRuntimeStyle: true,
      coordinateSystem: "metric-y-up-xz-ground",
      sourceContracts: {
        demo5TiltShift: sourceContract,
      },
    }, deepMerge(stageOptions.metadata || {}, options.metadata || {})),
  });
  const stage = deepMerge(baseStage, {
    schema: PHOTOGRAPHIC_RUNTIME_STYLE_SCHEMA,
    camera,
    focus,
    lightRig: deepMerge(DEFAULT_LIGHT_RIG, normalizeLightRigOptions(stageOptions.lightRig, options.lightRig)),
    grounding,
    materials,
    postprocess: createPhotographicMetricPostprocessOptions({
      focus,
      camera,
      ...(stageOptions.postprocess || {}),
      ...(options.postprocess || {}),
    }),
    hoverFocus: normalizeHoverFocus(options.hoverFocus ?? stageOptions.hoverFocus, DEFAULT_HOVER_FOCUS),
    sourceContracts: {
      demo5TiltShift: sourceContract,
    },
    metadata: deepMerge(baseStage.metadata || {}, {
      reusablePhotographicRuntimeStyle: true,
      coordinateSystem: "metric-y-up-xz-ground",
      groundPlane: "xz",
      sourceContracts: {
        demo5TiltShift: sourceContract,
      },
    }),
  });
  return deepFreeze(stage);
}

export function createPhotographicMetricStyle(options = {}) {
  const stage = options.stage || createPhotographicMetricStage(options);
  return createMiniaturePhotographicStyle({
    ...options,
    stage,
    fit: options.fit === false ? false : deepMerge(DEFAULT_FIT, options.fit || {}),
    ground: options.ground === false ? false : deepMerge(DEFAULT_GROUND, options.ground || {}),
    contacts: options.contacts ?? true,
    motion: options.motion ?? false,
    metadata: deepMerge({
      reusablePhotographicRuntimeStyle: true,
      coordinateSystem: "metric-y-up-xz-ground",
      sourceContracts: stage.sourceContracts,
    }, options.metadata || {}),
  });
}

export function createPhotographicMetricRuntimeOptions(options = {}) {
  const stage = options.stage || createPhotographicMetricStage(options);
  const style = options.style || createPhotographicMetricStyle({
    ...options,
    stage,
  });
  const setup = createMiniatureRuntimeOptions({
    ...options,
    stage,
    layerFactory: options.layerFactory || options.layers?.factory || style.layerFactory,
    pixelRatioCap: finiteNumber(options.pixelRatioCap, 1.5),
    camera: deepMerge(stage.camera || {}, options.runtimeCamera || {}),
    controls: normalizeControls(options.controls, stage),
    hoverFocus: normalizeHoverFocus(options.hoverFocus, stage.hoverFocus || DEFAULT_HOVER_FOCUS),
    renderer: deepMerge(DEFAULT_RUNTIME_RENDERER, options.renderer || {}),
    postprocess: options.runtimePostprocess || stage.postprocess,
    layers: deepMerge({
      autoLoadFactory: false,
      disposeOnRebuild: true,
      warnOnMissingFactory: true,
      factory: options.layerFactory || options.layers?.factory || style.layerFactory,
    }, options.layers || {}),
  });

  return {
    schema: PHOTOGRAPHIC_RUNTIME_SETUP_SCHEMA,
    style,
    stage: setup.stage,
    runtimeOptions: setup.runtimeOptions,
    sourceContracts: stage.sourceContracts,
  };
}

export function createPhotographicMetricRuntimeSetup(options = {}) {
  const setup = createPhotographicMetricRuntimeOptions(options);
  const style = setup.style;
  const descriptors = options.descriptors || options.layers?.descriptors || null;
  const bundle = descriptors
    ? createPhotographicMetricSceneBundle(descriptors, {
      ...options,
      stage: setup.stage,
      style,
    })
    : null;
  const runtime = createRuntimeInstance(options, setup.runtimeOptions);

  if (runtime) {
    style.attachRuntime?.(runtime, { applyStage: options.applyStage !== false });
    if (bundle && options.applyBundle !== false) {
      applyMiniatureSceneBundle(runtime, bundle, {
        source: options.source || "photographic-metric-runtime-setup",
      });
    }
  }

  return {
    ...setup,
    runtime,
    bundle,
    createSceneBundle(nextDescriptors, bundleOptions = {}) {
      return createPhotographicMetricSceneBundle(nextDescriptors, {
        ...options,
        ...bundleOptions,
        stage: bundleOptions.stage || setup.stage,
        style,
      });
    },
    applySceneBundle(targetRuntime = runtime, nextBundle = bundle, applyOptions = {}) {
      return applyPhotographicMetricSceneBundle(targetRuntime, nextBundle, applyOptions);
    },
  };
}

export function createPhotographicMetricSceneBundle(descriptors, options = {}) {
  const stage = options.stage || createPhotographicMetricStage(options);
  const style = options.style || createPhotographicMetricStyle({
    ...options,
    stage,
  });
  const fit = options.fit === false
    ? false
    : deepMerge(DEFAULT_FIT, deepMerge({
      sameRoomMorph: {
        groundY: stage.grounding?.groundY ?? DEFAULT_GROUNDING.groundY,
        flatY: finiteNumber(stage.grounding?.groundY, DEFAULT_GROUNDING.groundY) + 0.09,
      },
    }, options.fit || {}));
  const ground = options.ground === false
    ? false
    : deepMerge(DEFAULT_GROUND, {
      groundY: stage.grounding?.groundY ?? DEFAULT_GROUNDING.groundY,
      y: stage.grounding?.groundY ?? DEFAULT_GROUNDING.groundY,
      ...(options.ground || {}),
    });

  const bundle = createMiniatureSceneBundle(descriptors, {
    ...options,
    stage,
    fit,
    ground,
    contacts: options.contacts ?? true,
    motion: options.motion ?? false,
    metadata: deepMerge({
      reusablePhotographicRuntimeStyle: true,
      coordinateSystem: "metric-y-up-xz-ground",
      groundPlane: "xz",
      sourceContracts: stage.sourceContracts,
    }, options.metadata || {}),
  });
  style.lastBundle = bundle;
  return bundle;
}

export function applyPhotographicMetricSceneBundle(runtime, bundle, options = {}) {
  if (!runtime || typeof runtime !== "object") {
    throw new Error("applyPhotographicMetricSceneBundle() requires a runtime object.");
  }
  const sceneBundle = bundle || options.bundle;
  if (!sceneBundle || typeof sceneBundle !== "object") {
    throw new Error("applyPhotographicMetricSceneBundle() requires a scene bundle.");
  }
  return applyMiniatureSceneBundle(runtime, sceneBundle, {
    source: options.source || "photographic-metric-scene-bundle",
  });
}

export function createHoldMorphHoldTimeline(options = {}) {
  const settings = deepMerge(DEFAULT_TIMELINE, options);
  const hold2dMs = positiveNumber(settings.hold2dMs, settings.hold2DMs, settings.fromHoldMs, DEFAULT_TIMELINE.hold2dMs);
  const morphTo3dMs = positiveNumber(settings.morphTo3dMs, settings.to3dMs, settings.morphMs, DEFAULT_TIMELINE.morphTo3dMs);
  const hold3dMs = positiveNumber(settings.hold3dMs, settings.hold3DMs, settings.toHoldMs, DEFAULT_TIMELINE.hold3dMs);
  const morphTo2dMs = positiveNumber(settings.morphTo2dMs, settings.to2dMs, settings.returnMorphMs, morphTo3dMs);
  const phases = [
    createTimelinePhase("hold-2d", 0, hold2dMs, 0, 0, "linear"),
    createTimelinePhase("morph-to-3d", hold2dMs, morphTo3dMs, 0, 1, settings.easing),
    createTimelinePhase("hold-3d", hold2dMs + morphTo3dMs, hold3dMs, 1, 1, "linear"),
    createTimelinePhase("morph-to-2d", hold2dMs + morphTo3dMs + hold3dMs, morphTo2dMs, 1, 0, settings.easing),
  ];
  const durationMs = phases.reduce((max, phase) => Math.max(max, phase.endMs), 0);
  return deepFreeze({
    schema: HOLD_MORPH_HOLD_TIMELINE_SCHEMA,
    mode: "hold-morph-hold",
    progressTarget: settings.progressTarget || "descriptor.animation.progress",
    loop: settings.loop !== false,
    durationMs,
    phases,
    metadata: {
      coordinateMorphOnly: true,
      rendersData: false,
      ...(settings.metadata || {}),
    },
  });
}

export function sampleHoldMorphHoldTimeline(timelineOrOptions = {}, timeMs = 0) {
  const timeline = timelineOrOptions.schema === HOLD_MORPH_HOLD_TIMELINE_SCHEMA
    ? timelineOrOptions
    : createHoldMorphHoldTimeline(timelineOrOptions);
  const durationMs = positiveNumber(timeline.durationMs, 1);
  const sourceTime = Math.max(0, finiteNumber(timeMs, 0));
  const localTimeMs = timeline.loop === false ? Math.min(sourceTime, durationMs) : sourceTime % durationMs;
  const phases = timeline.phases || [];
  const phase = phases.find((entry) => localTimeMs >= entry.startMs && localTimeMs <= entry.endMs) || phases[phases.length - 1];
  if (!phase) {
    return {
      timeMs: sourceTime,
      localTimeMs: 0,
      durationMs,
      phaseId: null,
      phaseIndex: -1,
      phaseProgress: 0,
      easedProgress: 0,
      progress: 0,
    };
  }
  const phaseDuration = Math.max(1, phase.durationMs);
  const phaseProgress = clamp01((localTimeMs - phase.startMs) / phaseDuration);
  const easedProgress = resolveTimelineEasing(phase.easing)(phaseProgress);
  const progress = phase.from + (phase.to - phase.from) * easedProgress;
  return {
    timeMs: sourceTime,
    localTimeMs,
    durationMs,
    phaseId: phase.id,
    phaseIndex: phases.indexOf(phase),
    phaseProgress,
    easedProgress,
    progress: clamp01(progress),
  };
}

export function applyHoldMorphHoldProgressToDescriptors(descriptors, timelineSample, options = {}) {
  const sample = Number.isFinite(Number(timelineSample))
    ? { progress: Number(timelineSample) }
    : timelineSample || {};
  const progress = clamp01(sample.progress);
  const source = Array.isArray(descriptors) ? descriptors : [descriptors];
  const channel = options.channel || "targetPosition";
  return source.filter(Boolean).map((descriptor) => {
    const animation = descriptor.animation || {};
    const isMorph = animation.mode === "coordinate-morph"
      || animation.mode === "morph"
      || animation.channel === channel
      || descriptor.kind === "morph"
      || descriptor.source?.viewKind === "morph";
    if (!isMorph && options.force !== true) return descriptor;
    return {
      ...descriptor,
      animation: {
        mode: "coordinate-morph",
        channel,
        requiresChannels: animation.requiresChannels || ["position", channel],
        ...animation,
        progress,
      },
    };
  });
}

function createRuntimeInstance(options, runtimeOptions) {
  if (options.runtime) return options.runtime;
  if (typeof options.runtimeFactory === "function") {
    return options.runtimeFactory(runtimeOptions);
  }
  if (typeof options.RuntimeClass === "function") {
    return new options.RuntimeClass(runtimeOptions);
  }
  if (typeof options.runtimeClass === "function") {
    return new options.runtimeClass(runtimeOptions);
  }
  return null;
}

function createTimelinePhase(id, startMs, durationMs, from, to, easing) {
  const duration = Math.max(0, finiteNumber(durationMs, 0));
  return {
    id,
    startMs,
    durationMs: duration,
    endMs: startMs + duration,
    from,
    to,
    easing: easing || "linear",
  };
}

function normalizeControls(value, stage) {
  if (value === false) return false;
  const source = value && typeof value === "object" ? value : {};
  return deepMerge(DEFAULT_RUNTIME_CONTROLS, {
    focusAxis: stage.focus?.axis || DEFAULT_RUNTIME_CONTROLS.focusAxis,
    focusRadius: stage.focus?.radius,
    updateFocusOnMove: stage.focus?.pointer?.enabled !== false,
    ...source,
  });
}

function normalizeHoverFocus(value, fallback = DEFAULT_HOVER_FOCUS) {
  if (value === false) return { ...fallback, enabled: false };
  if (value === true) return { ...fallback, enabled: true };
  if (value && typeof value === "object") return deepMerge(fallback, value);
  return clonePlain(fallback);
}

function normalizeEnabledObject(value, fallback) {
  if (value === false) return { ...fallback, enabled: false };
  if (value === true) return { ...fallback, enabled: true };
  if (value && typeof value === "object") return deepMerge(fallback, value);
  return clonePlain(fallback);
}

function normalizeLightRigOptions(...values) {
  for (const value of values) {
    if (value && typeof value === "object") return value;
  }
  return {};
}

function deepMerge(base, override) {
  if (override === false) return false;
  if (!isPlainObject(base) || !isPlainObject(override)) {
    return clonePlain(override ?? base);
  }
  const next = clonePlain(base);
  for (const [key, value] of Object.entries(override || {})) {
    next[key] = isPlainObject(value) && isPlainObject(next[key])
      ? deepMerge(next[key], value)
      : clonePlain(value);
  }
  return next;
}

function clonePlain(value) {
  if (value == null || typeof value !== "object") return value;
  if (Array.isArray(value)) return value.map(clonePlain);
  if (ArrayBuffer.isView(value)) return value;
  const next = {};
  for (const [key, entry] of Object.entries(value)) {
    next[key] = clonePlain(entry);
  }
  return next;
}

function isPlainObject(value) {
  return Boolean(value && typeof value === "object" && !Array.isArray(value) && !ArrayBuffer.isView(value));
}

function finiteNumber(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number)) return number;
  }
  return 0;
}

function positiveNumber(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number) && number > 0) return number;
  }
  return 1;
}

function clamp01(value) {
  const number = Number(value);
  if (!Number.isFinite(number) || number <= 0) return 0;
  if (number >= 1) return 1;
  return number;
}

function resolveTimelineEasing(easing) {
  if (typeof easing === "function") return easing;
  if (easing === "smootherstep") {
    return (t) => {
      const x = clamp01(t);
      return x * x * x * (x * (x * 6 - 15) + 10);
    };
  }
  if (easing === "easeInOutQuad") {
    return (t) => {
      const x = clamp01(t);
      return x < 0.5 ? 2 * x * x : 1 - ((-2 * x + 2) ** 2) / 2;
    };
  }
  if (easing === "linear") return clamp01;
  return (t) => {
    const x = clamp01(t);
    return x * x * (3 - 2 * x);
  };
}

function deepFreeze(value) {
  if (!value || typeof value !== "object" || Object.isFrozen(value)) return value;
  Object.freeze(value);
  for (const entry of Object.values(value)) deepFreeze(entry);
  return value;
}
