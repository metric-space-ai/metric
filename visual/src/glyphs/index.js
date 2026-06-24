import {
  createGlyphGeometry,
  createPointBillboardGeometry,
} from "../geometry/index.js";

export const GLYPH_SCHEMA = "metric.visual.glyph_registry.v1";

const GLYPH_DEFINITIONS = {
  record: {
    name: "record",
    label: "Record",
    geometryKind: "record",
    geometryOptions: { radius: 0.5, flat: false },
    defaults: {
      semantic: "record",
      scale: [1, 1, 1],
      color: [0.16, 0.38, 0.6, 1],
      opacity: 1,
      pickable: true,
      material: { lighting: "lambert", roughness: 0.8 },
    },
  },
  selectedRecord: {
    name: "selectedRecord",
    label: "Selected record",
    geometryKind: "selectedRecord",
    geometryOptions: { size: [1.08, 1.08, 1.08] },
    defaults: {
      semantic: "selected-record",
      scale: [1.12, 1.12, 1.12],
      color: [1, 0.76, 0.18, 1],
      opacity: 1,
      pickable: true,
      material: { lighting: "lambert", roughness: 0.55, outline: true },
    },
  },
  clusterRepresentative: {
    name: "clusterRepresentative",
    label: "Cluster representative",
    geometryKind: "clusterRepresentative",
    geometryOptions: { radius: 0.44, height: 1.1, segments: 10 },
    defaults: {
      semantic: "cluster-representative",
      scale: [1.1, 1.1, 1.25],
      color: [0.13, 0.56, 0.49, 1],
      opacity: 1,
      pickable: true,
      material: { lighting: "lambert", roughness: 0.7 },
    },
  },
  outlier: {
    name: "outlier",
    label: "Outlier",
    geometryKind: "outlier",
    geometryOptions: { radius: 0.52, flat: true },
    defaults: {
      semantic: "outlier",
      scale: [1, 1, 1],
      color: [0.86, 0.22, 0.28, 1],
      opacity: 1,
      pickable: true,
      material: { lighting: "flat", roughness: 0.9 },
    },
  },
  landmark: {
    name: "landmark",
    label: "Landmark",
    geometryKind: "landmark",
    geometryOptions: { radius: 0.42, height: 1.4, segments: 12, anchor: "bottom" },
    defaults: {
      semantic: "landmark",
      scale: [1, 1, 1],
      color: [0.42, 0.31, 0.62, 1],
      opacity: 1,
      pickable: true,
      material: { lighting: "lambert", roughness: 0.62 },
    },
  },
  trajectoryPoint: {
    name: "trajectoryPoint",
    label: "Trajectory point",
    geometryKind: "trajectoryPoint",
    geometryOptions: { size: 1, facing: "camera", sizeSpace: "screen" },
    defaults: {
      semantic: "trajectory-point",
      scale: [1, 1, 1],
      color: [0.2, 0.47, 0.82, 0.82],
      opacity: 0.82,
      pickable: true,
      material: { lighting: "unlit", alphaMode: "blend" },
    },
  },
};

export const GLYPH_REGISTRY = Object.freeze(Object.fromEntries(
  Object.entries(GLYPH_DEFINITIONS).map(([name, definition]) => [
    name,
    freezeGlyphDefinition({
      schema: GLYPH_SCHEMA,
      ...definition,
      geometry: definition.geometryKind === "trajectoryPoint"
        ? createPointBillboardGeometry(definition.geometryOptions)
        : createGlyphGeometry(definition.geometryKind, definition.geometryOptions),
    }),
  ]),
));

export const glyphRegistry = GLYPH_REGISTRY;

export function listGlyphNames() {
  return Object.keys(GLYPH_REGISTRY);
}

export function getGlyphDefinition(name) {
  return GLYPH_REGISTRY[name] || null;
}

export function createRegisteredGlyphGeometry(name, options = {}) {
  const definition = getGlyphDefinition(name);
  if (!definition) {
    throw new Error(`Unknown glyph: ${name}`);
  }
  const geometryOptions = {
    ...definition.geometryOptions,
    ...options,
  };
  return definition.geometryKind === "trajectoryPoint"
    ? createPointBillboardGeometry(geometryOptions)
    : createGlyphGeometry(definition.geometryKind, geometryOptions);
}

export function getGlyphDefaults(name) {
  const definition = getGlyphDefinition(name);
  return definition ? cloneDefaults(definition.defaults) : null;
}

function freezeGlyphDefinition(definition) {
  return Object.freeze({
    ...definition,
    geometryOptions: Object.freeze({ ...definition.geometryOptions }),
    defaults: freezeDefaults(definition.defaults),
  });
}

function freezeDefaults(defaults) {
  return Object.freeze({
    ...defaults,
    scale: Object.freeze([...defaults.scale]),
    color: Object.freeze([...defaults.color]),
    material: Object.freeze({ ...defaults.material }),
  });
}

function cloneDefaults(defaults) {
  return {
    ...defaults,
    scale: [...defaults.scale],
    color: [...defaults.color],
    material: { ...defaults.material },
  };
}
