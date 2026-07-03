import { MINIATURE_DESCRIPTOR_TAG, MINIATURE_LAYER_ROLES, MINIATURE_STYLE_ID } from "./constants.js";
import {
  createMetricMiniaturePalette,
} from "./palette.js";
import {
  createMiniatureContactShadowDescriptors,
  createMiniatureGroundDescriptorOptions,
  createMiniatureProjectionStyle,
} from "./grounding.js";
import {
  createMiniatureGroundMaterial,
  createMiniatureMaterial,
  createMiniatureNoiseOptions,
  createMiniatureStagePreset,
} from "./presets.js";
import { applyMiniatureLightingToMaterial } from "./lights.js";
import { resolveMiniatureAnimationForFamily } from "./animation.js";

const PRIMITIVE_FAMILY = Object.freeze({
  InstancedPointLayer: "sample",
  InstancedGlyphLayer: "sample",
  InstancedBoxLayer: "mesh",
  SurfaceLayer: "surface",
  HeatFieldLayer: "field",
  GroundProjectionLayer: "field",
  GroundPlaneLayer: "ground",
  RelationEdgeLayer: "line",
  CurveRibbonLayer: "line",
  CurveTubeMeshLayer: "line",
  RelationMatrixLayer: "field",
  BillboardLabelLayer: "sample",
});

const PRIMITIVE_ROLE = Object.freeze({
  InstancedPointLayer: MINIATURE_LAYER_ROLES.sample,
  InstancedGlyphLayer: MINIATURE_LAYER_ROLES.glyph,
  InstancedBoxLayer: MINIATURE_LAYER_ROLES.mesh,
  SurfaceLayer: MINIATURE_LAYER_ROLES.surface,
  HeatFieldLayer: MINIATURE_LAYER_ROLES.field,
  GroundProjectionLayer: MINIATURE_LAYER_ROLES.projection,
  GroundPlaneLayer: MINIATURE_LAYER_ROLES.ground,
  RelationEdgeLayer: MINIATURE_LAYER_ROLES.edge,
  CurveRibbonLayer: MINIATURE_LAYER_ROLES.edge,
  CurveTubeMeshLayer: MINIATURE_LAYER_ROLES.edge,
  RelationMatrixLayer: MINIATURE_LAYER_ROLES.field,
  BillboardLabelLayer: MINIATURE_LAYER_ROLES.label,
});

/**
 * Applies the miniature photographic style to a list of layer descriptors.
 *
 * This is a style transform only. Channel arrays and metric-derived geometry
 * are preserved by reference; the function merges renderer-facing material,
 * geometry, animation, and metadata hints.
 *
 * @param {object[]|object} descriptors
 * @param {object} [options]
 * @returns {object[]}
 */
export function applyMiniatureStyleToDescriptors(descriptors, options = {}) {
  const source = Array.isArray(descriptors) ? descriptors : [descriptors];
  const palette = options.palette || createMetricMiniaturePalette(options.paletteOptions || {});
  const stage = options.stage || createMiniatureStagePreset({ palette, ...(options.stageOptions || {}) });
  return source
    .filter(Boolean)
    .map((descriptor, index) => applyMiniatureStyleToDescriptor(descriptor, {
      ...options,
      palette,
      stage,
      orderOffset: Number(options.orderOffset) || 0,
      layerIndex: index,
    }));
}

/**
 * Applies the miniature photographic style to one layer descriptor.
 *
 * @param {object} descriptor
 * @param {object} [options]
 * @returns {object}
 */
export function applyMiniatureStyleToDescriptor(descriptor, options = {}) {
  if (!descriptor || typeof descriptor !== "object") {
    throw new Error("applyMiniatureStyleToDescriptor() requires a layer descriptor object.");
  }
  const palette = options.palette || createMetricMiniaturePalette();
  const primitive = descriptor.primitive || descriptor.kind;
  const family = options.family || PRIMITIVE_FAMILY[primitive] || "sample";
  const role = options.role || PRIMITIVE_ROLE[primitive] || MINIATURE_LAYER_ROLES.sample;
  const material = materialForFamily(family, primitive, palette, options);
  const geometry = geometryForFamily(family, descriptor.geometry || {}, options);
  const animation = animationForFamily(family, descriptor.animation || {}, options);

  return {
    ...descriptor,
    visible: descriptor.visible !== false,
    order: Number.isFinite(Number(descriptor.order))
      ? Number(descriptor.order) + (Number(options.orderOffset) || 0)
      : Number(options.layerIndex) || 0,
    channels: descriptor.channels || {},
    geometry,
    material: {
      ...material,
      ...(descriptor.material || {}),
      metadata: {
        ...(material.metadata || {}),
        ...(descriptor.material?.metadata || {}),
      },
    },
    animation,
    metadata: {
      ...(descriptor.metadata || {}),
      style: MINIATURE_STYLE_ID,
      miniature: true,
      miniatureTag: MINIATURE_DESCRIPTOR_TAG,
      miniatureRole: role,
      miniatureFamily: family,
      nativeMetricVisualization: true,
    },
  };
}

/**
 * Creates a miniature ground layer descriptor that can be prepended to styled
 * layer descriptors.
 *
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniatureGroundDescriptor(options = {}) {
  const palette = options.palette || createMetricMiniaturePalette();
  const groundOptions = createMiniatureGroundDescriptorOptions(options.stage?.grounding || {}, options);
  return {
    schema: "metric.visual.layer_descriptor.v1",
    id: options.id || "miniature-ground",
    kind: "miniature-ground",
    primitive: "GroundPlaneLayer",
    visible: options.visible !== false,
    order: Number.isFinite(Number(options.order)) ? Number(options.order) : -100,
    channels: {},
    geometry: {
      size: positiveNumber(groundOptions.size, 5),
      width: positiveNumber(groundOptions.width, groundOptions.size, 5),
      depth: positiveNumber(groundOptions.depth, groundOptions.size, 5),
      groundY: finiteNumber(groundOptions.groundY, groundOptions.y, -0.75),
      y: finiteNumber(groundOptions.y, groundOptions.groundY, -0.75),
      gridScale: positiveNumber(groundOptions.gridScale, 8),
    },
    material: createMiniatureGroundMaterial({ palette, ...(groundOptions.material || {}) }),
    animation: { mode: "none" },
    picking: { mode: "none" },
    metadata: {
      style: MINIATURE_STYLE_ID,
      miniature: true,
      miniatureRole: MINIATURE_LAYER_ROLES.ground,
      nativeMetricVisualization: true,
      ...(options.metadata || {}),
    },
  };
}

/**
 * Creates a full style bundle containing stage options and styled descriptors.
 *
 * @param {object[]|object} descriptors
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniatureDescriptorBundle(descriptors, options = {}) {
  const palette = options.palette || createMetricMiniaturePalette(options.paletteOptions || {});
  const stage = options.stage || createMiniatureStagePreset({ palette, ...(options.stageOptions || {}) });
  const layers = applyMiniatureStyleToDescriptors(descriptors, { ...options, palette, stage });
  const contacts = options.contacts === false || stage.grounding?.contact?.enabled === false
    ? []
    : createMiniatureContactShadowDescriptors(layers, {
      plane: stage.grounding?.plane,
      groundY: stage.grounding?.groundY,
      projection: stage.grounding?.projection,
      contact: {
        ...(stage.grounding?.contact || {}),
        ...(options.contacts || {}),
      },
      includeSamples: options.contacts?.includeSamples === true,
    });
  const stagedLayers = contacts.length ? [...contacts, ...layers] : layers;
  return {
    schema: "metric.visual.miniature_descriptor_bundle.v1",
    style: MINIATURE_STYLE_ID,
    stage,
    layers: options.ground === false
      ? stagedLayers
      : [createMiniatureGroundDescriptor({ palette, stage, ...(options.ground || {}) }), ...stagedLayers],
    metadata: {
      nativeMetricVisualization: true,
      adapterWrapper: false,
      ...(options.metadata || {}),
    },
  };
}

function materialForFamily(family, primitive, palette, options) {
  if (family === "ground") {
    return createMiniatureGroundMaterial({
      palette,
      ...(options.stage?.materials?.ground || {}),
      ...(options.materials?.ground || {}),
      ...(options.material || {}),
    });
  }
  const projection = primitive === "GroundProjectionLayer"
    ? createMiniatureProjectionStyle({
      ...(options.stage?.grounding?.projection || {}),
      ...(options.materials?.projection || {}),
    })
    : {};
  const material = createMiniatureMaterial(family, {
    palette,
    ...(options.stage?.materials?.[family] || {}),
    ...(options.materials?.[family] || {}),
    ...projection,
    ...(options.material || {}),
  });
  return applyMiniatureLightingToMaterial(material, options.stage?.lightRig);
}

function geometryForFamily(family, geometry, options) {
  const next = { ...geometry };
  if (family === "sample") {
    next.shape = next.shape || "disc";
    next.pointPixelScale = finiteNumber(next.pointPixelScale, options.pointPixelScale, 9);
  }
  if (family === "mesh") {
    next.shading = next.shading || "miniature-lit";
  }
  if (family === "surface") {
    next.texture = {
      styleNoise: createMiniatureNoiseOptions(options.noise || {}),
      ...(next.texture || {}),
    };
  }
  if (family === "field") {
    next.composite = next.composite || "soft-screen";
  }
  if (family === "line") {
    next.screenSpace = next.screenSpace !== false && next.mode !== "tube";
  }
  return next;
}

function animationForFamily(family, animation, options) {
  const next = resolveMiniatureAnimationForFamily(family, animation, {
    motion: options.motion,
    animationOptions: options.animation,
  });
  if (family === "surface" && next.mode === "style-noise" && next.noise == null) {
    next.noise = createMiniatureNoiseOptions(options.noise || {});
  }
  return next;
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
