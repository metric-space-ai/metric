import {
  MINIATURE_DESCRIPTOR_TAG,
  MINIATURE_LAYER_ROLES,
  MINIATURE_STYLE_ID,
  MINIATURE_STYLE_SCHEMA,
} from "./constants.js";
import {
  createMiniatureFocusPlane,
  createMiniatureTiltShiftOptions,
} from "./focus.js";
import { createMiniatureGroundingPreset } from "./grounding.js";
import { createMetricMiniaturePalette, miniatureColor } from "./palette.js";
import { createPhotographicCameraDof } from "../../postfx/spatial-focus.js";

const DEFAULT_CAMERA = Object.freeze({
  mode: "miniature-photographic",
  coordinateSystem: "metric-xz-ground",
  projection: "perspective",
  fov: 42,
  near: 0.05,
  far: 2000,
  position: Object.freeze([0, -5.2, 3.2]),
  target: Object.freeze([0, 0, 0]),
  up: Object.freeze([0, 0, 1]),
  orbit: Object.freeze({
    enableDamping: true,
    dampingFactor: 0.05,
    autoRotate: false,
    minPitch: -1.42,
    maxPitch: 1.42,
  }),
});

const DARK_CAMERA = Object.freeze({
  ...DEFAULT_CAMERA,
  position: Object.freeze([0, 0, 100]),
  target: Object.freeze([0, 0, 0]),
  up: Object.freeze([0, 0, 1]),
});

const HIGH_OBLIQUE_CAMERA = Object.freeze({
  ...DEFAULT_CAMERA,
  position: Object.freeze([0, -8.5, 8.5]),
  target: Object.freeze([0, 0, 0]),
  up: Object.freeze([0, 0, 1]),
  orbit: Object.freeze({
    ...DEFAULT_CAMERA.orbit,
    autoRotate: true,
  }),
});

const MATERIAL_FAMILIES = Object.freeze({
  sample: Object.freeze({
    family: "sample",
    lighting: "phong-like",
    dataColor: true,
    ambient: 0.54,
    pointLight: 0.34,
    specular: 0.08,
    sphereShade: 0.62,
    gloss: 0.22,
    edgeShade: 0.38,
    saturation: 0.98,
    shadowDensity: 0.44,
    highlightSharpness: 0.36,
    rimLight: 0.16,
    highlightLift: 0.22,
    shadowTint: "#d8dfdd",
    alpha: 0.94,
    alphaMode: "blend",
    pointPixelScale: 9,
    minPointSize: 2,
    maxPointSize: 42,
    focusBoost: 0.34,
    phasePulse: 0.05,
  }),
  mesh: Object.freeze({
    family: "mesh",
    lighting: "phong-like",
    dataColor: true,
    vertexColors: true,
    ambient: 0.58,
    pointLight: 0.44,
    specular: 0.1,
    specularPower: 42,
    metalness: 0.18,
    roughness: 0.52,
    saturation: 0.96,
    shadowSoftness: 0.32,
    baseLift: 0.035,
    shadowTint: "#c2cccf",
    highlightColor: "#fff6df",
    groundOcclusion: 0.18,
    faceContrast: 0.24,
    verticalGradient: 0.14,
    rimLight: 0.08,
    rimColor: "#ffffff",
    alpha: 1,
    alphaMode: "opaque",
  }),
  surface: Object.freeze({
    family: "surface",
    lighting: "physical-like",
    dataColor: true,
    ambient: 0.52,
    pointLight: 0.42,
    specular: 0.16,
    specularPower: 54,
    metalness: 0.28,
    roughness: 0.34,
    saturation: 0.98,
    shadowSoftness: 0.24,
    baseLift: 0.025,
    shadowTint: "#bdc7cb",
    highlightColor: "#fff7e6",
    displacementTexture: "style-noise",
    normalTexture: "style-noise-normal",
    alpha: 0.96,
  }),
  field: Object.freeze({
    family: "field",
    lighting: "screen-space",
    alpha: 0.82,
    alphaMode: "blend",
    scalarColorMap: "miniature-heat",
    feather: 0.28,
    contour: 0.18,
    glow: 0.12,
  }),
  line: Object.freeze({
    family: "line",
    lighting: "screen-space-ribbon",
    ambient: 0.42,
    pointLight: 0.5,
    emission: 0.18,
    rimLight: 0.26,
    coreGlow: 0.14,
    edgeFeather: 0.32,
    tubeShade: 0.62,
    flowStrength: 0.08,
    flowScale: 2.4,
    flowSpeed: 0.18,
    saturation: 1,
    depthShade: 0.2,
    shadowTint: "#93a7ad",
    highlightColor: "#fff5db",
    alpha: 0.68,
    alphaMode: "blend",
    thickness: 1.2,
    feather: 0.32,
  }),
  ground: Object.freeze({
    family: "ground",
    lighting: "unlit",
    alpha: 0.42,
    alphaMode: "blend",
    gridScale: 8,
    gridWidth: 0.018,
    axisWidth: 0.012,
    gridAlpha: 0.28,
    axisAlpha: 0.24,
    fade: 0.88,
    stageTilt: -0.12,
    stageScale: Object.freeze([0.92, 0.78]),
    sheen: 0.06,
    matte: 0.78,
    contactShade: 0.16,
    horizonFade: 0.12,
    horizonColor: Object.freeze([0.94, 0.95, 0.91]),
    sheenColor: Object.freeze([1, 0.98, 0.9]),
  }),
});

const LIGHT_RIGS = Object.freeze({
  whiteStage: Object.freeze({
    mode: "ambient-plus-key",
    ambient: Object.freeze({ color: "#ffffff", intensity: 0.58 }),
    key: Object.freeze({ color: "#ffffff", intensity: 0.84, position: Object.freeze([0, 0, 20]) }),
    fill: Object.freeze({ color: "#dce8ef", intensity: 0.18, position: Object.freeze([-18, -12, 24]) }),
  }),
  chromaticStudio: Object.freeze({
    mode: "chromatic-point-rig",
    ambient: Object.freeze({ color: "#808080", intensity: 0.32 }),
    points: Object.freeze([
      Object.freeze({ color: "#0E09DC", intensity: 0.62, position: Object.freeze([0, 0, 50]) }),
      Object.freeze({ color: "#1CD1E1", intensity: 0.54, position: Object.freeze([0, 0, 50]) }),
      Object.freeze({ color: "#18C02C", intensity: 0.46, position: Object.freeze([0, 0, 50]) }),
      Object.freeze({ color: "#ee3bcf", intensity: 0.48, position: Object.freeze([0, 0, 50]) }),
    ]),
  }),
  darkGloss: Object.freeze({
    mode: "dark-gloss-corners",
    ambient: Object.freeze({ color: "#151a22", intensity: 0.16 }),
    points: Object.freeze([
      Object.freeze({ color: "#b307b5", intensity: 0.5, position: Object.freeze([-2, -2, 2]) }),
      Object.freeze({ color: "#8132aa", intensity: 0.5, position: Object.freeze([-2, 2, 2]) }),
      Object.freeze({ color: "#5737d0", intensity: 0.5, position: Object.freeze([2, 2, 2]) }),
      Object.freeze({ color: "#0d25bb", intensity: 0.5, position: Object.freeze([2, -2, 2]) }),
    ]),
  }),
  cityDaylight: Object.freeze({
    mode: "low-city-daylight",
    ambient: Object.freeze({ color: "#eff7f4", intensity: 0.46 }),
    key: Object.freeze({ color: "#ffffff", intensity: 0.92, position: Object.freeze([-5, -8, 8]) }),
    fill: Object.freeze({ color: "#89d9df", intensity: 0.22, position: Object.freeze([5, 3, 7]) }),
  }),
  redShadowStage: Object.freeze({
    mode: "colored-shadow-stage",
    ambient: Object.freeze({ color: "#5a1610", intensity: 0.3 }),
    points: Object.freeze([
      Object.freeze({ color: "#ffffff", intensity: 0.58, position: Object.freeze([0, 150, 0]) }),
      Object.freeze({ color: "#ff2a1f", intensity: 0.52, position: Object.freeze([0, -150, 0]) }),
      Object.freeze({ color: "#ff9476", intensity: 0.22, position: Object.freeze([-80, -60, 48]) }),
    ]),
  }),
  areaBoard: Object.freeze({
    mode: "area-board-studio",
    ambient: Object.freeze({ color: "#1c1410", intensity: 0.18 }),
    points: Object.freeze([
      Object.freeze({ color: "#ff6000", intensity: 0.82, position: Object.freeze([0, 10, 1]) }),
      Object.freeze({ color: "#0060ff", intensity: 0.76, position: Object.freeze([10, 0, 1]) }),
      Object.freeze({ color: "#60ff60", intensity: 0.4, position: Object.freeze([-10, 0, 1]) }),
      Object.freeze({ color: "#ffffff", intensity: 0.92, position: Object.freeze([0, -10, 1]) }),
    ]),
  }),
  liquidSurface: Object.freeze({
    mode: "surface-ripple-studio",
    ambient: Object.freeze({ color: "#20242b", intensity: 0.24 }),
    points: Object.freeze([
      Object.freeze({ color: "#FFFF80", intensity: 0.52, position: Object.freeze([50, 50, 50]) }),
      Object.freeze({ color: "#DE3578", intensity: 0.5, position: Object.freeze([-50, 50, 50]) }),
      Object.freeze({ color: "#FF4040", intensity: 0.42, position: Object.freeze([-50, -50, 50]) }),
      Object.freeze({ color: "#0d25bb", intensity: 0.44, position: Object.freeze([50, -50, 50]) }),
    ]),
  }),
});

const REFERENCE_LOOKS = deepFreeze({
  "white-tabletop": {
    id: "metric-miniature-look-white-tabletop",
    look: "white-tabletop",
    variant: "white",
    sourceExamples: ["metric focus-depth study", "metric ground-light study"],
    lightRig: "whiteStage",
    focus: {
      axis: "horizontal",
      yRatio: 0.5,
      radiusRatio: 0.3,
      focusBandRatio: 0.07,
      blurRadius: 5.2,
      blurCurve: 1.42,
    },
    grounding: {
      ground: {
        gridAlpha: 0.24,
        axisAlpha: 0.2,
        sheen: 0.08,
        matte: 0.82,
      },
      projection: {
        footprintStretch: 2.0,
        footprintSkew: -0.28,
        shadowDensity: 1.12,
        colorMix: 0.62,
      },
    },
    postprocess: {
      bloom: {
        intensity: 0.1,
        threshold: 0.94,
        radius: 1.05,
      },
      miniatureFrame: {
        amount: 0.42,
        floorSheen: 0.12,
        subjectIsolation: 0.24,
      },
    },
  },
  "chromatic-surface": {
    id: "metric-miniature-look-chromatic-surface",
    look: "chromatic-surface",
    variant: "high-oblique",
    sourceExamples: ["metric bright-tabletop study"],
    lightRig: "chromaticStudio",
    camera: {
      fov: 40,
      position: [0, -7.2, 3.4],
      target: [0, 0, 0.1],
    },
    materials: {
      surface: {
        metalness: 0.34,
        roughness: 0.26,
        specular: 0.2,
        displacementTexture: "style-noise",
        normalTexture: "style-noise-normal",
        saturation: 1.04,
      },
      mesh: {
        metalness: 0.42,
        roughness: 0.22,
        specular: 0.18,
        faceContrast: 0.3,
      },
    },
    postprocess: {
      bloom: {
        intensity: 0.16,
        threshold: 0.86,
        radius: 1.18,
        saturation: 0.82,
      },
      miniatureFrame: {
        amount: 0.34,
        stageGlow: 0.14,
        backlight: 0.2,
      },
    },
  },
  "dark-tube-field": {
    id: "metric-miniature-look-dark-tube-field",
    look: "dark-tube-field",
    variant: "dark",
    sourceExamples: ["metric curve-body study"],
    lightRig: "darkGloss",
    camera: {
      fov: 38,
      position: [0, -5.6, 2.35],
      target: [0, 0, 0.08],
    },
    focus: {
      axis: "horizontal",
      yRatio: 0.52,
      radiusRatio: 0.34,
      focusBandRatio: 0.08,
      blurRadius: 4.8,
      blurCurve: 1.32,
    },
    materials: {
      line: {
        alpha: 0.9,
        emission: 0.34,
        coreGlow: 0.34,
        edgeFeather: 0.42,
        tubeShade: 0.42,
        saturation: 1.16,
        depthShade: 0.12,
        flowStrength: 0.14,
      },
      field: {
        alpha: 0.68,
        glow: 0.24,
        contour: 0.12,
      },
      ground: {
        alpha: 0.2,
        gridAlpha: 0.1,
        axisAlpha: 0.08,
      },
    },
    postprocess: {
      bloom: {
        intensity: 0.46,
        threshold: 0.58,
        radius: 1.42,
        saturation: 0.96,
        clamp: 1.08,
      },
      miniatureFrame: {
        amount: 0.24,
        stageGlow: 0.08,
        groundShade: 0.02,
        backlight: 0.1,
      },
      vignette: {
        amount: 0.2,
        radius: 0.7,
      },
    },
  },
  "dark-gloss-studio": {
    id: "metric-miniature-look-dark-gloss-studio",
    look: "dark-gloss-studio",
    variant: "dark",
    sourceExamples: ["metric material study", "metric material-light study"],
    lightRig: "darkGloss",
    camera: {
      fov: 38,
      position: [0, -4.8, 2.2],
      target: [0, 0, 0.04],
    },
    materials: {
      mesh: {
        metalness: 0.66,
        roughness: 0.26,
        specular: 0.22,
        rimLight: 0.2,
        verticalGradient: 0.08,
      },
      surface: {
        metalness: 0.44,
        roughness: 0.22,
        specular: 0.24,
        shadowSoftness: 0.18,
      },
    },
    postprocess: {
      bloom: {
        intensity: 0.34,
        threshold: 0.64,
        radius: 1.3,
        saturation: 0.92,
      },
      miniatureFrame: {
        amount: 0.26,
        subjectIsolation: 0.3,
        microContrast: 0.22,
      },
    },
  },
  "metric-city": {
    id: "metric-miniature-look-metric-city",
    look: "metric-city",
    variant: "white",
    sourceExamples: ["metric depth-of-field study", "metric focus-depth study"],
    lightRig: "cityDaylight",
    paletteOptions: {
      colors: {
        sample: "#26b9c5",
        mesh: "#2dbbc4",
        meshHighlight: "#8ee9e6",
        ground: "#f7f8f4",
        groundGrid: "#c9d2cc",
        projection: "rgba(33, 44, 42, 0.42)",
        edge: "rgba(43, 84, 92, 0.64)",
        heatHigh: "rgba(36, 181, 190, 0.92)",
      },
    },
    camera: {
      fov: 34,
      position: [0, -6.7, 2.05],
      target: [0, 0, 0.18],
    },
    focus: {
      axis: "horizontal",
      yRatio: 0.43,
      radiusRatio: 0.24,
      focusBandRatio: 0.05,
      blurRadius: 5.8,
      blurCurve: 1.55,
    },
    grounding: {
      ground: {
        gridAlpha: 0.16,
        axisAlpha: 0.08,
        matte: 0.86,
        contactShade: 0.22,
      },
      projection: {
        footprintStretch: 2.18,
        footprintSkew: -0.42,
        shadowDensity: 1.66,
        colorMix: 0.52,
        shadowTail: 0.8,
        shadowTailStrength: 0.42,
      },
      contact: {
        alpha: 0.54,
        coreDensity: 0.58,
        contactHardness: 0.46,
      },
    },
    materials: {
      mesh: {
        faceContrast: 0.34,
        verticalGradient: 0.28,
        metalness: 0.08,
        roughness: 0.58,
        shadowSoftness: 0.22,
      },
      sample: {
        pointPixelScale: 7,
        focusBoost: 0.42,
        shadowDensity: 0.5,
      },
    },
    postprocess: {
      bloom: {
        intensity: 0.08,
        threshold: 0.9,
        radius: 0.9,
      },
      grade: {
        saturation: 1.02,
        vibrance: 0.18,
        temperature: 0.08,
        shadowTint: [0.74, 0.94, 0.98],
        highlightTint: [0.98, 1.04, 1.02],
        shadowTintStrength: 0.18,
        highlightTintStrength: 0.12,
      },
      miniatureFrame: {
        amount: 0.48,
        floorSheen: 0.08,
        subjectIsolation: 0.28,
        microContrast: 0.24,
        shadowColor: [0.54, 0.58, 0.55],
      },
      vignette: {
        amount: 0.06,
        radius: 0.88,
      },
    },
  },
  "colored-shadow-stage": {
    id: "metric-miniature-look-colored-shadow-stage",
    look: "colored-shadow-stage",
    variant: "high-oblique",
    sourceExamples: ["metric ground-light study", "metric instanced-field study"],
    lightRig: "redShadowStage",
    paletteOptions: {
      colors: {
        sample: "#dc493c",
        mesh: "#c6382d",
        meshHighlight: "#ff765f",
        ground: "#e8bd87",
        groundGrid: "#9b6655",
        projection: "rgba(122, 18, 12, 0.48)",
        edge: "rgba(118, 20, 14, 0.72)",
        glow: "#ffaf82",
        heatHigh: "rgba(255, 80, 48, 0.86)",
      },
    },
    camera: {
      fov: 38,
      position: [0, -5.4, 3.1],
      target: [0, 0, 0.06],
    },
    focus: {
      axis: "horizontal",
      yRatio: 0.5,
      radiusRatio: 0.34,
      focusBandRatio: 0.09,
      blurRadius: 4.2,
      blurCurve: 1.12,
    },
    grounding: {
      ground: {
        gridAlpha: 0.16,
        axisAlpha: 0.08,
        matte: 0.74,
        contactShade: 0.22,
        sheen: 0.1,
        horizonFade: 0.2,
        horizonColor: [0.92, 0.8, 0.66],
        sheenColor: [1, 0.78, 0.54],
      },
      projection: {
        footprintStretch: 1.82,
        shadowDensity: 1.58,
        colorMix: 0.82,
        edgeTint: 0.62,
        contactHardness: 0.46,
        neutralShadow: [0.38, 0.2, 0.16],
      },
    },
    materials: {
      mesh: {
        ambient: 0.4,
        pointLight: 0.62,
        specular: 0.12,
        faceContrast: 0.42,
        shadowTint: "#4e0805",
      },
      ground: {
        alpha: 0.34,
        contactShade: 0.28,
      },
    },
    postprocess: {
      bloom: {
        intensity: 0.18,
        threshold: 0.72,
        radius: 1.04,
        saturation: 0.98,
      },
      grade: {
        contrast: 1.12,
        saturation: 1.08,
        vibrance: 0.22,
        temperature: 0.18,
        tint: 0.02,
        lift: [0.012, 0.006, 0.002],
        gain: [1.08, 0.92, 0.84],
        shadowTint: [0.56, 0.2, 0.14],
        highlightTint: [1.12, 0.68, 0.48],
        shadowTintStrength: 0.2,
        highlightTintStrength: 0.2,
      },
      miniatureFrame: {
        amount: 0.46,
        warmth: 0.12,
        groundShade: 0.18,
        floorSheen: 0.12,
        backlight: 0.2,
        stageSpotlight: 0.28,
        stageShadow: 0.32,
        edgeFade: 0.14,
        subjectIsolation: 0.24,
        floorColor: [0.84, 0.68, 0.5],
        skyColor: [0.95, 0.83, 0.72],
        shadowColor: [0.34, 0.12, 0.08],
        highlightColor: [1, 0.72, 0.48],
      },
      vignette: {
        amount: 0.18,
        radius: 0.74,
        color: [0.3, 0.1, 0.06],
      },
    },
  },
  "studio-board": {
    id: "metric-miniature-look-studio-board",
    look: "studio-board",
    variant: "dark",
    sourceExamples: ["metric light-rig study"],
    lightRig: "areaBoard",
    paletteOptions: {
      background: "#000000",
      colors: {
        sample: "#1fa5ff",
        mesh: "#b7a483",
        meshHighlight: "#fff2cf",
        surface: "#f3efe2",
        surfaceDisplacement: "#9b7f57",
        ground: "#2d241c",
        groundGrid: "#826b52",
        projection: "rgba(28, 166, 255, 0.35)",
        edge: "rgba(250, 218, 160, 0.72)",
        glow: "#ffe5a5",
        heatHigh: "rgba(255, 170, 72, 0.92)",
      },
    },
    camera: {
      fov: 42,
      position: [0, -5.1, 2.85],
      target: [0, 0, 0.02],
    },
    grounding: {
      ground: {
        gridAlpha: 0.34,
        axisAlpha: 0.12,
        sheen: 0.28,
        matte: 0.36,
        contactShade: 0.22,
        horizonFade: 0.18,
        sheenColor: [1, 0.7, 0.32],
      },
      projection: {
        footprintStretch: 1.72,
        shadowDensity: 0.94,
        colorMix: 0.78,
        coreDensity: 0.34,
      },
    },
    materials: {
      surface: {
        metalness: 0.18,
        roughness: 0.18,
        specular: 0.34,
        displacementTexture: "style-noise",
        normalTexture: "style-noise-normal",
      },
      ground: {
        alpha: 0.54,
        sheen: 0.3,
        matte: 0.38,
      },
      field: {
        alpha: 0.72,
        glow: 0.28,
      },
    },
    postprocess: {
      bloom: {
        intensity: 0.32,
        threshold: 0.62,
        radius: 1.24,
        saturation: 0.9,
      },
      grade: {
        contrast: 1.12,
        saturation: 1.08,
        temperature: 0.22,
        tint: -0.04,
        lift: [0.006, 0.003, 0.0],
        gain: [1.12, 0.98, 0.82],
        shadowTint: [0.5, 0.28, 0.16],
        highlightTint: [1.3, 0.86, 0.48],
        shadowTintStrength: 0.22,
        highlightTintStrength: 0.26,
      },
      miniatureFrame: {
        amount: 0.3,
        stageGlow: 0.1,
        floorSheen: 0.34,
        floorRatio: 0.58,
        backlight: 0.2,
        shadowColor: [0.12, 0.08, 0.05],
        highlightColor: [1, 0.78, 0.42],
      },
      vignette: {
        amount: 0.2,
        radius: 0.74,
      },
    },
  },
  "surface-ripple": {
    id: "metric-miniature-look-surface-ripple",
    look: "surface-ripple",
    variant: "dark",
    sourceExamples: ["metric material study", "metric bright-tabletop study"],
    lightRig: "liquidSurface",
    paletteOptions: {
      background: "#05070a",
      colors: {
        sample: "#f0e9d4",
        mesh: "#d8c2a5",
        meshHighlight: "#fff6d4",
        surface: "#f7f5ee",
        surfaceDisplacement: "#cfc8b5",
        ground: "#10151b",
        groundGrid: "#50606d",
        projection: "rgba(237, 218, 164, 0.32)",
        edge: "rgba(226, 215, 184, 0.66)",
        glow: "#fff28a",
        heatHigh: "rgba(255, 82, 78, 0.82)",
      },
    },
    camera: {
      fov: 40,
      position: [0, -4.2, 3.05],
      target: [0, 0, 0],
    },
    focus: {
      axis: "horizontal",
      yRatio: 0.48,
      radiusRatio: 0.42,
      focusBandRatio: 0.1,
      blurRadius: 3.6,
      blurCurve: 1.08,
    },
    grounding: {
      ground: {
        gridAlpha: 0.22,
        axisAlpha: 0.08,
        sheen: 0.18,
        matte: 0.42,
      },
      projection: {
        alpha: 0.44,
        softness: 0.86,
        footprintStretch: 2.4,
        shadowDensity: 0.74,
        colorMix: 0.86,
        coreDensity: 0.24,
      },
      contact: {
        pathAlpha: 0.3,
        pathSoftness: 0.96,
        pathColorMix: 0.56,
      },
    },
    materials: {
      surface: {
        metalness: 0.64,
        roughness: 0.18,
        specular: 0.34,
        displacementTexture: "style-noise",
        normalTexture: "style-noise-normal",
        shadowSoftness: 0.16,
      },
      line: {
        emission: 0.26,
        coreGlow: 0.24,
        flowStrength: 0.18,
        saturation: 1.12,
      },
      field: {
        alpha: 0.64,
        glow: 0.32,
        contour: 0.16,
      },
    },
    postprocess: {
      bloom: {
        intensity: 0.38,
        threshold: 0.56,
        radius: 1.34,
        saturation: 0.86,
      },
      grade: {
        contrast: 1.1,
        saturation: 1.12,
        vibrance: 0.18,
        temperature: 0.1,
        tint: 0.04,
        lift: [0.002, 0.004, 0.008],
        gain: [1.08, 0.96, 0.92],
        shadowTint: [0.62, 0.66, 0.76],
        highlightTint: [1.16, 0.86, 0.62],
        shadowTintStrength: 0.2,
        highlightTintStrength: 0.2,
      },
      miniatureFrame: {
        amount: 0.22,
        stageGlow: 0.08,
        floorSheen: 0.24,
        subjectIsolation: 0.26,
        microContrast: 0.14,
      },
      vignette: {
        amount: 0.18,
        radius: 0.76,
      },
    },
  },
});

/**
 * Creates a complete miniature stage preset.
 *
 * @param {object} [options]
 * @param {"white"|"dark"|"high-oblique"} [options.variant]
 * @param {string|object} [options.look]
 * @param {object} [options.palette]
 * @returns {object}
 */
export function createMiniatureStagePreset(options = {}) {
  const settings = options.look
    ? mergePlainDeep(createMiniatureReferenceLookOptions(options.look), copyWithout(options, ["look"]))
    : options;
  const variant = settings.variant || "white";
  const palette = settings.palette || createMetricMiniaturePalette({
    background: variant === "dark" ? "#000000" : undefined,
    ...(settings.paletteOptions || {}),
  });
  const camera = createMiniatureCameraOptions({
    variant: variant === "high-oblique" ? "high-oblique" : variant === "dark" ? "dark" : "white",
    ...(settings.camera || {}),
  });
  const focus = createMiniatureFocusPlane(settings.focus || settings.tiltShift || {});
  const grounding = createMiniatureGroundingPreset(settings.grounding || {});
  const lightRig = createMiniatureLightRig({
    variant: settings.lightRig || (variant === "dark" ? "darkGloss" : "whiteStage"),
  });

  return deepFreeze({
    schema: MINIATURE_STYLE_SCHEMA,
    id: settings.id || "metric-miniature-stage",
    style: MINIATURE_STYLE_ID,
    variant,
    look: settings.look || null,
    sourceExamples: settings.sourceExamples ? settings.sourceExamples.slice() : [],
    palette,
    scene: {
      background: settings.background || palette.background,
      clearColor: miniatureColor(settings.background || palette.background),
      tone: variant === "dark" ? "black-stage" : "white-stage",
    },
    camera,
    lightRig,
    focus,
    grounding,
    materials: createMiniatureMaterialSet({ palette, ...(settings.materials || {}) }),
    postprocess: createMiniaturePostprocessOptions({
      variant,
      focus,
      ...(settings.postprocess || {}),
    }),
    interaction: {
      pointerFocus: true,
      focusAxis: focus.axis,
      focusLine: focus,
      orbitDamping: true,
      ...(settings.interaction || {}),
    },
    metadata: {
      nativeMetricStyle: true,
      sourceGrammar: "METRIC miniature photographic miniature audit",
      ...(settings.metadata || {}),
    },
  });
}

/**
 * Returns the named native METRIC look options extracted from inspected source visual references
 * examples. A look is a reusable stage recipe: camera, lights, materials,
 * grounding and postprocess. It is not a demo subject and does not compute
 * metric data.
 *
 * @param {string|object} [look]
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniatureReferenceLookOptions(look = "white-tabletop", options = {}) {
  const name = typeof look === "string" ? look : look?.name || look?.look || "white-tabletop";
  const base = REFERENCE_LOOKS[name] || REFERENCE_LOOKS["white-tabletop"];
  const embeddedOptions = look && typeof look === "object" ? copyWithout(look, ["name", "look"]) : {};
  return deepFreeze(mergePlainDeep(mergePlainDeep(base, embeddedOptions), options));
}

/**
 * Lists the reusable native METRIC visual looks available to the style layer.
 *
 * @returns {object[]}
 */
export function listMiniatureReferenceLooks() {
  return Object.values(REFERENCE_LOOKS).map((entry) => clonePlain(entry));
}

/**
 * Creates camera composition options for miniature scenes.
 *
 * @param {object} [options]
 * @param {"white"|"dark"|"high-oblique"} [options.variant]
 * @returns {object}
 */
export function createMiniatureCameraOptions(options = {}) {
  const base = options.variant === "dark"
    ? DARK_CAMERA
    : options.variant === "high-oblique"
      ? HIGH_OBLIQUE_CAMERA
      : DEFAULT_CAMERA;
  return deepFreeze(mergePlain(base, options, ["variant"]));
}

/**
 * Creates a photographic miniature light rig descriptor using native METRIC plain objects.
 *
 * @param {object} [options]
 * @param {string} [options.variant]
 * @returns {object}
 */
export function createMiniatureLightRig(options = {}) {
  const rig = LIGHT_RIGS[options.variant] || LIGHT_RIGS.whiteStage;
  return deepFreeze(mergePlain(rig, options, ["variant"]));
}

/**
 * Creates tilt-shift focus options compatible with the native TiltShiftPass.
 *
 * @param {object} [options]
 * @param {number} [options.width]
 * @param {number} [options.height]
 * @returns {object}
 */
export function createTiltShiftOptions(options = {}) {
  return createMiniatureTiltShiftOptions(options);
}

/**
 * Creates native postprocess preset options extracted from source visual references effects.
 *
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniaturePostprocessOptions(options = {}) {
  const focus = options.focus?.kind === "miniature-focus-plane"
    ? options.focus
    : createMiniatureFocusPlane(options.focus || options.tiltShift || {});
  const bloom = createMiniatureBloomOptions(options);
  const grade = {
    enabled: options.grade?.enabled !== false && options.colorGrade?.enabled !== false,
    exposure: finiteNumber(options.grade?.exposure, options.colorGrade?.exposure, 0),
    contrast: finiteNumber(options.grade?.contrast, options.colorGrade?.contrast, options.variant === "dark" ? 1.08 : 1.04),
    contrastPivot: finiteNumber(options.grade?.contrastPivot, options.colorGrade?.contrastPivot, 0.48),
    saturation: finiteNumber(options.grade?.saturation, options.colorGrade?.saturation, options.variant === "dark" ? 1.04 : 0.96),
    vibrance: finiteNumber(options.grade?.vibrance, options.colorGrade?.vibrance, 0.08),
    temperature: finiteNumber(options.grade?.temperature, options.colorGrade?.temperature, options.variant === "dark" ? 0.02 : 0.06),
    tint: finiteNumber(options.grade?.tint, options.colorGrade?.tint, -0.015),
    gamma: finiteNumber(options.grade?.gamma, options.colorGrade?.gamma, 1),
    blackPoint: finiteNumber(options.grade?.blackPoint, options.colorGrade?.blackPoint, 0.006),
    whitePoint: finiteNumber(options.grade?.whitePoint, options.colorGrade?.whitePoint, 1),
    highlightCompression: finiteNumber(options.grade?.highlightCompression, options.colorGrade?.highlightCompression, 0.12),
    lift: normalizeColor3(options.grade?.lift || options.colorGrade?.lift, [0.002, 0.002, 0.004]),
    gain: normalizeColor3(options.grade?.gain || options.colorGrade?.gain, [1.02, 1.01, 0.99]),
    shadowTint: normalizeColor3(options.grade?.shadowTint || options.colorGrade?.shadowTint, [0.94, 0.98, 1.04]),
    highlightTint: normalizeColor3(options.grade?.highlightTint || options.colorGrade?.highlightTint, [1.04, 1.015, 0.965]),
    shadowTintStrength: finiteNumber(options.grade?.shadowTintStrength, options.colorGrade?.shadowTintStrength, 0.08),
    highlightTintStrength: finiteNumber(options.grade?.highlightTintStrength, options.colorGrade?.highlightTintStrength, 0.1),
  };
  const vignette = {
    enabled: options.vignette?.enabled !== false,
    amount: finiteNumber(options.vignette?.amount, options.vignetteAmount, options.variant === "dark" ? 0.16 : 0.08),
    radius: finiteNumber(options.vignette?.radius, options.vignetteRadius, options.variant === "dark" ? 0.72 : 0.84),
    softness: finiteNumber(options.vignette?.softness, options.vignetteSoftness, 0.72),
    roundness: finiteNumber(options.vignette?.roundness, options.vignetteRoundness, 0.42),
    color: normalizeColor3(options.vignette?.color, options.variant === "dark" ? [0.08, 0.1, 0.14] : [0.86, 0.88, 0.84]),
    exposurePreserve: finiteNumber(options.vignette?.exposurePreserve, options.vignetteExposurePreserve, 0.22),
  };
  const miniatureFrame = {
    enabled: options.miniatureFrame?.enabled !== false,
    amount: finiteNumber(options.miniatureFrame?.amount, options.miniatureFrameAmount, options.variant === "dark" ? 0.28 : 0.42),
    focusClarity: finiteNumber(options.miniatureFrame?.focusClarity, options.focusClarity, options.variant === "dark" ? 0.12 : 0.18),
    focusLift: finiteNumber(options.miniatureFrame?.focusLift, options.focusLift, options.variant === "dark" ? 0.018 : 0.035),
    stageGlow: finiteNumber(options.miniatureFrame?.stageGlow, options.stageGlow, options.variant === "dark" ? 0.04 : 0.12),
    groundShade: finiteNumber(options.miniatureFrame?.groundShade, options.groundShade, options.variant === "dark" ? 0.02 : 0.08),
    edgeFade: finiteNumber(options.miniatureFrame?.edgeFade, options.edgeFade, options.variant === "dark" ? 0.1 : 0.08),
    warmth: finiteNumber(options.miniatureFrame?.warmth, options.frameWarmth, options.variant === "dark" ? -0.02 : 0.05),
    floorRatio: finiteNumber(options.miniatureFrame?.floorRatio, options.floorRatio, 0.52),
    floorColor: normalizeColor3(options.miniatureFrame?.floorColor, options.variant === "dark" ? [0.1, 0.11, 0.14] : [0.86, 0.88, 0.84]),
    skyColor: normalizeColor3(options.miniatureFrame?.skyColor, options.variant === "dark" ? [0.08, 0.09, 0.12] : [0.98, 0.97, 0.92]),
    stageCenter: normalizePoint2(options.miniatureFrame?.stageCenter, [0.5, 0.52]),
    stageScale: normalizePoint2(options.miniatureFrame?.stageScale, [0.9, 0.74]),
    stageTilt: finiteNumber(options.miniatureFrame?.stageTilt, options.stageTilt, options.variant === "dark" ? 0 : -0.1),
    floorSheen: finiteNumber(options.miniatureFrame?.floorSheen, options.floorSheen, options.variant === "dark" ? 0.06 : 0.12),
    floorFalloff: finiteNumber(options.miniatureFrame?.floorFalloff, options.floorFalloff, 0.78),
    backlight: finiteNumber(options.miniatureFrame?.backlight, options.backlight, options.variant === "dark" ? 0.06 : 0.16),
    stageSpotlight: finiteNumber(options.miniatureFrame?.stageSpotlight, options.stageSpotlight, options.variant === "dark" ? 0.1 : 0.18),
    stageShadow: finiteNumber(options.miniatureFrame?.stageShadow, options.stageShadow, options.variant === "dark" ? 0.2 : 0.16),
    subjectIsolation: finiteNumber(options.miniatureFrame?.subjectIsolation, options.subjectIsolation, 0.24),
    microContrast: finiteNumber(options.miniatureFrame?.microContrast, options.microContrast, 0.18),
    lensAberration: finiteNumber(options.miniatureFrame?.lensAberration, options.lensAberration, options.variant === "dark" ? 0.008 : 0.006),
    apertureVignette: finiteNumber(options.miniatureFrame?.apertureVignette, options.apertureVignette, options.variant === "dark" ? 0.12 : 0.08),
    focusDesaturation: finiteNumber(options.miniatureFrame?.focusDesaturation, options.focusDesaturation, options.variant === "dark" ? 0.02 : 0.04),
    shadowColor: normalizeColor3(options.miniatureFrame?.shadowColor, options.variant === "dark" ? [0.08, 0.09, 0.12] : [0.67, 0.73, 0.77]),
    highlightColor: normalizeColor3(options.miniatureFrame?.highlightColor, options.variant === "dark" ? [0.58, 0.62, 0.74] : [1, 0.965, 0.88]),
  };
  const filmGrain = {
    enabled: options.filmGrain === true || options.film === true || options.filmGrain?.enabled === true || options.film?.enabled === true,
    intensity: finiteNumber(options.filmGrain?.intensity, options.film?.intensity, options.noiseIntensity, 0.014),
    size: finiteNumber(options.filmGrain?.size, options.film?.size, 1.4),
    lumaResponse: finiteNumber(options.filmGrain?.lumaResponse, options.film?.lumaResponse, 0.65),
  };
  const cameraDofSource = normalizeCameraDofSource(options);
  const cameraDof = createPhotographicCameraDof({
    ...cameraDofSource,
    enabled: options.cameraDof !== false
      && options.depthOfField !== false
      && cameraDofSource.enabled !== false,
    amount: finiteNumber(cameraDofSource.amount, options.dofAmount, 0.84),
    focusDistance: finiteNumber(cameraDofSource.focusDistance, options.focusDistance, 9),
    depthNear: finiteNumber(cameraDofSource.depthNear, options.depthNear, 1.2),
    depthFar: finiteNumber(cameraDofSource.depthFar, options.depthFar, 18),
    cameraNear: finiteNumber(cameraDofSource.cameraNear, options.cameraNear, cameraDofSource.depthNear, options.depthNear, 1.2),
    cameraFar: finiteNumber(cameraDofSource.cameraFar, options.cameraFar, cameraDofSource.depthFar, options.depthFar, 18),
    aperture: finiteNumber(cameraDofSource.aperture, options.aperture, 0.92),
    maxBlur: finiteNumber(cameraDofSource.maxBlur, options.maxBlur, 6.2),
    focalRange: finiteNumber(cameraDofSource.focalRange, options.focalRange, 0.135),
    quietDesaturation: finiteNumber(cameraDofSource.quietDesaturation, 0.16),
    quietExposure: finiteNumber(cameraDofSource.quietExposure, 0.09),
    focusClarity: finiteNumber(cameraDofSource.focusClarity, 0.045),
    vignette: finiteNumber(cameraDofSource.vignette, options.variant === "dark" ? 0.16 : 0.12),
  });
  return deepFreeze({
    enabled: options.enabled !== false,
    antialias: options.antialias || "fxaa",
    fxaa: { enabled: options.fxaa !== false },
    smaa: { enabled: options.smaa === true },
    tiltShift: createTiltShiftOptions({
      ...focus,
      ...(options.tiltShift && typeof options.tiltShift === "object" ? options.tiltShift : {}),
    }),
    bokeh: {
      enabled: options.bokeh === true,
      focus: finiteNumber(options.bokehFocus, 1),
      aperture: finiteNumber(options.aperture, 0.025),
      maxblur: finiteNumber(options.maxblur, 0.01),
    },
    bloom,
    cameraDof,
    grade,
    colorGrade: grade,
    miniatureFrame,
    vignette,
    filmGrain,
    film: filmGrain,
    halftone: {
      enabled: options.halftone === true,
      shape: finiteNumber(options.halftoneShape, 1),
      radius: finiteNumber(options.halftoneRadius, 1),
      scatter: finiteNumber(options.halftoneScatter, 0),
    },
    ssao: {
      enabled: options.ssao === true,
      intensity: finiteNumber(options.ssaoIntensity, 0.18),
    },
    zoomBlur: {
      enabled: options.zoomBlur === true,
      center: normalizePoint2(options.zoomCenter, [0.5, 0.5]),
      strength: finiteNumber(options.zoomStrength, 0.04),
    },
    postFx: {
      enabled: options.postFx !== false,
      bloom,
      cameraDof,
      miniatureFrame,
      grade,
      vignette,
      filmGrain,
      ...(options.postFx && typeof options.postFx === "object" ? options.postFx : {}),
    },
  });
}

function createMiniatureBloomOptions(options = {}) {
  const source = options.bloom && typeof options.bloom === "object" ? options.bloom : {};
  const variant = options.variant || "white";
  const enabled = options.bloom === false || source.enabled === false ? false : true;
  const intensity = finiteNumber(
    source.intensity,
    source.strength,
    options.bloomIntensity,
    options.bloomStrength,
    variant === "dark" ? 0.42 : 0.1,
  );
  return {
    enabled,
    intensity,
    strength: intensity,
    threshold: finiteNumber(source.threshold, options.bloomThreshold, variant === "dark" ? 0.62 : 0.94),
    knee: finiteNumber(source.knee, options.bloomKnee, 0.16),
    radius: finiteNumber(source.radius, options.bloomRadius, variant === "dark" ? 1.35 : 1.05),
    iterations: Math.max(1, Math.min(8, Math.floor(finiteNumber(source.iterations, options.bloomIterations, 3)))),
    downsample: Math.max(1, Math.min(8, Math.floor(finiteNumber(source.downsample, options.bloomDownsample, 2)))),
    saturation: finiteNumber(source.saturation, options.bloomSaturation, variant === "dark" ? 0.9 : 0.72),
    warmth: finiteNumber(source.warmth, options.bloomWarmth, variant === "dark" ? 0.02 : 0.08),
    clamp: finiteNumber(source.clamp, options.bloomClamp, variant === "dark" ? 1.1 : 0.78),
    sourceIntensity: finiteNumber(source.sourceIntensity, options.bloomSourceIntensity, 1),
  };
}

/**
 * Creates one material preset for a renderer descriptor family.
 *
 * @param {string} family sample, mesh, surface, field, line, or ground.
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniatureMaterial(family = "sample", options = {}) {
  const palette = options.palette || createMetricMiniaturePalette();
  const defaults = MATERIAL_FAMILIES[family] || MATERIAL_FAMILIES.sample;
  const color = materialColorForFamily(family, palette);
  return deepFreeze({
    ...defaults,
    ...copyWithout(options, ["palette"]),
    family,
    style: MINIATURE_STYLE_ID,
    baseColor: options.baseColor || color,
    focusColor: options.focusColor || palette.sampleFocus,
    paletteId: palette.id,
    metadata: {
      miniatureRole: family,
      sourceMaterialGrammar: materialGrammarForFamily(family),
      ...(options.metadata || {}),
    },
  });
}

/**
 * Creates the point/sample material family used for point sprites and glyphs.
 *
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniaturePointMaterial(options = {}) {
  return createMiniatureMaterial("sample", options);
}

/**
 * Creates a generic lit mesh material preset for arbitrary instanced or solid
 * METRIC geometry.
 *
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniatureMeshMaterial(options = {}) {
  return createMiniatureMaterial("mesh", options);
}

/**
 * Creates a glossy/noisy surface material preset.
 *
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniatureSurfaceMaterial(options = {}) {
  return createMiniatureMaterial("surface", options);
}

/**
 * Creates a translucent field material preset.
 *
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniatureFieldMaterial(options = {}) {
  return createMiniatureMaterial("field", options);
}

/**
 * Creates an unlit transparent relation/line material preset.
 *
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniatureLineMaterial(options = {}) {
  return createMiniatureMaterial("line", options);
}

/**
 * Creates a ground-plane material preset.
 *
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniatureGroundMaterial(options = {}) {
  const palette = options.palette || createMetricMiniaturePalette();
  return createMiniatureMaterial("ground", {
    palette,
    baseColor: options.baseColor || palette.ground,
    gridColor: options.gridColor || palette.groundGrid,
    axisXColor: options.axisXColor || palette.axisX,
    axisZColor: options.axisZColor || palette.axisZ,
    ...copyWithout(options, ["palette", "baseColor", "gridColor", "axisXColor", "axisZColor"]),
  });
}

/**
 * Creates a style noise descriptor for native shader passes.
 *
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniatureNoiseOptions(options = {}) {
  return deepFreeze({
    enabled: options.enabled !== false,
    kind: "style-noise",
    source: "simplex-like-screen-or-surface-displacement",
    timeCoef: finiteNumber(options.timeCoef, 0.0003),
    noiseCoef: finiteNumber(options.noiseCoef, 2),
    displacementScale: finiteNumber(options.displacementScale, 0.06),
    normalDelta: finiteNumber(options.normalDelta, 1 / 512),
    pointerCoupling: finiteNumber(options.pointerCoupling, 0.12),
  });
}

/**
 * Creates the default material set for a miniature stage.
 *
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniatureMaterialSet(options = {}) {
  const palette = options.palette || createMetricMiniaturePalette();
  return deepFreeze({
    sample: createMiniatureMaterial("sample", { palette, ...(options.sample || {}) }),
    mesh: createMiniatureMaterial("mesh", { palette, ...(options.mesh || {}) }),
    surface: createMiniatureMaterial("surface", { palette, ...(options.surface || {}) }),
    field: createMiniatureMaterial("field", { palette, ...(options.field || {}) }),
    line: createMiniatureMaterial("line", { palette, ...(options.line || {}) }),
    ground: createMiniatureGroundMaterial({ palette, ...(options.ground || {}) }),
  });
}

function materialColorForFamily(family, palette) {
  if (family === "mesh") return palette.mesh;
  if (family === "surface") return palette.surface;
  if (family === "field") return palette.heatHigh;
  if (family === "line") return palette.edge;
  if (family === "ground") return palette.ground;
  return palette.sample;
}

function materialGrammarForFamily(family) {
  if (family === "mesh") return "Phong/Standard vertex-color lighting";
  if (family === "surface") return "Physical glossy material plus shader displacement texture";
  if (family === "field") return "screen-space translucent scalar field treatment";
  if (family === "line") return "screen-space lit ribbon and relation geometry";
  if (family === "ground") return "white-stage grid plane";
  return "PointsMaterial/Phong-like sample treatment";
}

function normalizeCameraDofSource(options = {}) {
  const cameraDof = options.cameraDof && typeof options.cameraDof === "object" ? options.cameraDof : {};
  const depthOfField = options.depthOfField && typeof options.depthOfField === "object" ? options.depthOfField : {};
  return {
    ...depthOfField,
    ...cameraDof,
  };
}

function mergePlain(base, options, excluded = []) {
  const out = clonePlain(base);
  for (const [key, value] of Object.entries(options || {})) {
    if (excluded.includes(key)) continue;
    out[key] = clonePlain(value);
  }
  return out;
}

function mergePlainDeep(base, options, excluded = []) {
  const out = clonePlain(base);
  for (const [key, value] of Object.entries(options || {})) {
    if (excluded.includes(key)) continue;
    if (isPlainObject(out[key]) && isPlainObject(value)) {
      out[key] = mergePlainDeep(out[key], value);
    } else {
      out[key] = clonePlain(value);
    }
  }
  return out;
}

function copyWithout(input, excluded) {
  const out = {};
  for (const [key, value] of Object.entries(input || {})) {
    if (!excluded.includes(key)) out[key] = value;
  }
  return out;
}

function clonePlain(value) {
  if (Array.isArray(value)) return value.map(clonePlain);
  if (!value || typeof value !== "object") return value;
  const out = {};
  for (const [key, entry] of Object.entries(value)) out[key] = clonePlain(entry);
  return out;
}

function isPlainObject(value) {
  return Boolean(value) && typeof value === "object" && !Array.isArray(value) && !ArrayBuffer.isView(value);
}

function normalizePoint2(value, fallback) {
  if (Array.isArray(value) || ArrayBuffer.isView(value)) {
    return [finiteNumber(value[0], fallback[0]), finiteNumber(value[1], fallback[1])];
  }
  if (value && typeof value === "object") {
    return [finiteNumber(value.x, fallback[0]), finiteNumber(value.y, fallback[1])];
  }
  return fallback.slice();
}

function normalizeColor3(value, fallback) {
  if (Array.isArray(value) || ArrayBuffer.isView(value)) {
    return [
      finiteNumber(value[0], fallback[0]),
      finiteNumber(value[1], fallback[1]),
      finiteNumber(value[2], fallback[2]),
    ];
  }
  if (value && typeof value === "object") {
    return [
      finiteNumber(value.r, value.x, fallback[0]),
      finiteNumber(value.g, value.y, fallback[1]),
      finiteNumber(value.b, value.z, fallback[2]),
    ];
  }
  return fallback.slice();
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

function deepFreeze(value) {
  if (!value || typeof value !== "object" || Object.isFrozen(value)) return value;
  Object.freeze(value);
  for (const entry of Object.values(value)) deepFreeze(entry);
  return value;
}

export { MINIATURE_LAYER_ROLES };
