import {
  createGlyphGeometry,
  createPointBillboardGeometry,
} from "../geometry/index.js";

export const GLYPH_SCHEMA = "metric.visual.glyph_registry.v1";
export const RECORD_GLYPH_GRAMMAR_SCHEMA = "metric.visual.record_glyph_grammar.v1";

export const RECORD_GLYPH_TYPES = Object.freeze({
  neutral: 0,
  text: 1,
  timeSeries: 2,
  histogram: 3,
  image: 3,
  vector: 4,
  composed: 5,
});

export const RECORD_GLYPH_FAMILIES = Object.freeze({
  neutral: "neutral-record",
  text: "text-record",
  timeSeries: "time-series-record",
  histogram: "histogram-record",
  image: "image-record",
  vector: "vector-record",
  composed: "composed-record",
});

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
  neutralRecord: {
    name: "neutralRecord",
    label: "Neutral record",
    geometryKind: "record",
    geometryOptions: { radius: 0.42, flat: false },
    defaults: {
      semantic: "neutral-record",
      scale: [1, 1, 1],
      color: [0.34, 0.42, 0.47, 1],
      opacity: 1,
      pickable: true,
      material: { lighting: "lambert", roughness: 0.82 },
    },
  },
  textRecord: {
    name: "textRecord",
    label: "Text record",
    geometryKind: "plane",
    geometryOptions: { size: [0.82, 0.54], z: 0 },
    defaults: {
      semantic: "text-record",
      scale: [1, 1, 1],
      color: [0.20, 0.39, 0.62, 1],
      opacity: 1,
      pickable: true,
      material: { lighting: "unlit", alphaMode: "blend" },
    },
  },
  timeSeriesRecord: {
    name: "timeSeriesRecord",
    label: "Time-series record",
    geometryKind: "trajectoryPoint",
    geometryOptions: { size: 1.05, facing: "camera", sizeSpace: "screen" },
    defaults: {
      semantic: "time-series-record",
      scale: [1, 1, 1],
      color: [0.12, 0.55, 0.55, 1],
      opacity: 1,
      pickable: true,
      material: { lighting: "unlit", alphaMode: "blend" },
    },
  },
  histogramRecord: {
    name: "histogramRecord",
    label: "Histogram/image record",
    geometryKind: "selectedRecord",
    geometryOptions: { size: [0.86, 0.86, 0.18] },
    defaults: {
      semantic: "histogram-record",
      scale: [1, 1, 1],
      color: [0.84, 0.47, 0.18, 1],
      opacity: 1,
      pickable: true,
      material: { lighting: "flat", roughness: 0.72 },
    },
  },
  vectorRecord: {
    name: "vectorRecord",
    label: "Vector record",
    geometryKind: "landmark",
    geometryOptions: { radius: 0.34, height: 0.92, segments: 4, anchor: "bottom" },
    defaults: {
      semantic: "vector-record",
      scale: [1, 1, 1],
      color: [0.46, 0.32, 0.63, 1],
      opacity: 1,
      pickable: true,
      material: { lighting: "lambert", roughness: 0.68 },
    },
  },
  composedRecord: {
    name: "composedRecord",
    label: "Composed record",
    geometryKind: "selectedRecord",
    geometryOptions: { size: [0.92, 0.92, 0.28] },
    defaults: {
      semantic: "composed-record",
      scale: [1, 1, 1],
      color: [0.80, 0.28, 0.34, 1],
      opacity: 1,
      pickable: true,
      material: { lighting: "lambert", roughness: 0.58, outline: true },
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

export function createRecordGlyphGrammar(records = [], recordIds = null, options = {}) {
  const ids = (recordIds || records.map((record, index) => recordId(record) ?? String(index))).map(String);
  const byId = recordMap(records);
  const glyphType = new Float32Array(ids.length);
  const glyphVariant = new Float32Array(ids.length);
  const glyphFeature = new Float32Array(ids.length * 4);
  const payloadComplexity = new Float32Array(ids.length);
  const familyValues = [];
  const recordTypeValues = [];
  const payloadKindValues = [];
  const labelValues = [];
  const families = new Set();
  const payloadKinds = new Set();

  for (let index = 0; index < ids.length; index += 1) {
    const record = byId.get(ids[index]) || null;
    const glyph = inferRecordGlyph(record, options);
    glyphType[index] = glyph.typeCode;
    glyphVariant[index] = glyph.variant;
    glyphFeature.set(glyph.features, index * 4);
    payloadComplexity[index] = glyph.complexity;
    familyValues.push(glyph.family);
    recordTypeValues.push(glyph.recordType);
    payloadKindValues.push(glyph.payloadKind);
    labelValues.push(glyph.label);
    families.add(glyph.family);
    payloadKinds.add(glyph.payloadKind);
  }

  return {
    schema: RECORD_GLYPH_GRAMMAR_SCHEMA,
    recordCount: ids.length,
    families: Array.from(families),
    payloadKinds: Array.from(payloadKinds),
    channels: {
      glyphType: numericChannel(glyphType, 1, "record-glyph-type", {
        grammar: RECORD_GLYPH_GRAMMAR_SCHEMA,
        labels: { ...RECORD_GLYPH_FAMILIES },
      }),
      glyphVariant: numericChannel(glyphVariant, 1, "record-glyph-variant"),
      glyphFeature: numericChannel(glyphFeature, 4, "record-glyph-feature"),
      payloadComplexity: numericChannel(payloadComplexity, 1, "record-payload-complexity"),
      glyphFamily: stringChannel(familyValues, "record-glyph-family"),
      recordType: stringChannel(recordTypeValues, "record-type"),
      payloadKind: stringChannel(payloadKindValues, "record-payload-kind"),
      labelText: stringChannel(labelValues, "record-label"),
    },
  };
}

export function inferRecordGlyph(record, options = {}) {
  const payload = record && typeof record === "object" ? record.payload || record.value || record.values || null : null;
  const recordType = stringOr(record?.record_type ?? record?.recordType ?? record?.type, "record");
  const payloadKind = normalizeKind(payload?.kind ?? payload?.type ?? recordType);
  const label = stringOr(record?.label ?? record?.name ?? recordId(record), "");
  const explicitKind = normalizeKind(options.recordKind || options.kind || "");
  const kind = explicitKind || payloadKind || normalizeKind(recordType);

  if (kind === "composed" || hasObjectFields(payload) || /mixed|composed|structured/.test(recordType)) {
    return composedGlyph(record, payload, recordType, payloadKind || "composed", label);
  }
  if (kind === "time-series" || kind === "timeseries" || kind === "series" || Array.isArray(payload?.series)) {
    return timeSeriesGlyph(record, payload, recordType, payloadKind || "time_series", label);
  }
  if (kind === "histogram" || kind === "spectrum" || Array.isArray(payload?.bins)) {
    return histogramGlyph(record, payload, recordType, payloadKind || "histogram", label, RECORD_GLYPH_FAMILIES.histogram, RECORD_GLYPH_TYPES.histogram);
  }
  if (kind === "image" || kind === "image-like" || Array.isArray(payload?.pixels) || Array.isArray(payload?.matrix)) {
    return histogramGlyph(record, payload, recordType, payloadKind || "image", label, RECORD_GLYPH_FAMILIES.image, RECORD_GLYPH_TYPES.image);
  }
  if (kind === "vector" || kind === "embedding" || Array.isArray(payload?.values) || Array.isArray(payload?.vector)) {
    return vectorGlyph(record, payload, recordType, payloadKind || "vector", label);
  }
  if (kind === "string" || kind === "text" || kind === "code" || typeof payload?.text === "string" || typeof payload === "string") {
    return textGlyph(record, payload, recordType, payloadKind || "string", label);
  }
  return neutralGlyph(record, recordType, payloadKind || "unknown", label);
}

function neutralGlyph(record, recordType, payloadKind, label) {
  return glyphResult({
    typeCode: RECORD_GLYPH_TYPES.neutral,
    family: RECORD_GLYPH_FAMILIES.neutral,
    recordType,
    payloadKind,
    label,
    variant: hashUnit(recordId(record) ?? label),
    features: [0.35, 0.45, 0.55, 0.25],
    complexity: payloadComplexity(record?.payload),
  });
}

function textGlyph(record, payload, recordType, payloadKind, label) {
  const text = String(payload?.text ?? payload?.value ?? payload ?? label ?? "");
  const length = Math.min(1, text.length / 96);
  const words = text.trim() ? text.trim().split(/\s+/).length : 0;
  return glyphResult({
    typeCode: RECORD_GLYPH_TYPES.text,
    family: RECORD_GLYPH_FAMILIES.text,
    recordType,
    payloadKind,
    label,
    variant: hashUnit(text),
    features: [length, Math.min(1, words / 16), hashUnit(text.slice(0, 12)), 0.18],
    complexity: Math.max(0.1, length),
  });
}

function timeSeriesGlyph(record, payload, recordType, payloadKind, label) {
  const values = numericArray(payload?.series ?? payload?.values ?? payload);
  const stats = numericStats(values);
  return glyphResult({
    typeCode: RECORD_GLYPH_TYPES.timeSeries,
    family: RECORD_GLYPH_FAMILIES.timeSeries,
    recordType,
    payloadKind,
    label,
    variant: hashUnit(recordId(record) ?? label),
    features: [
      Math.min(1, values.length / 64),
      stats.spread,
      stats.trend,
      stats.center,
    ],
    complexity: Math.min(1, values.length / 48),
  });
}

function histogramGlyph(record, payload, recordType, payloadKind, label, family, typeCode) {
  const values = numericArray(payload?.bins ?? payload?.pixels ?? payload?.matrix ?? payload?.values ?? payload);
  const stats = numericStats(values);
  return glyphResult({
    typeCode,
    family,
    recordType,
    payloadKind,
    label,
    variant: hashUnit(recordId(record) ?? label),
    features: [
      Math.min(1, values.length / 32),
      stats.peak,
      stats.center,
      stats.spread,
    ],
    complexity: Math.min(1, values.length / 32),
  });
}

function vectorGlyph(record, payload, recordType, payloadKind, label) {
  const values = numericArray(payload?.vector ?? payload?.values ?? payload?.features ?? payload);
  const stats = numericStats(values);
  return glyphResult({
    typeCode: RECORD_GLYPH_TYPES.vector,
    family: RECORD_GLYPH_FAMILIES.vector,
    recordType,
    payloadKind,
    label,
    variant: hashUnit(recordId(record) ?? label),
    features: [
      Math.min(1, values.length / 16),
      stats.energy,
      stats.center,
      stats.spread,
    ],
    complexity: Math.min(1, values.length / 16),
  });
}

function composedGlyph(record, payload, recordType, payloadKind, label) {
  const fields = payload?.fields && typeof payload.fields === "object" ? payload.fields : {};
  let hasText = 0;
  let hasSeries = 0;
  let hasHistogram = 0;
  let hasVector = 0;
  for (const value of Object.values(fields)) {
    const glyph = inferRecordGlyph({ ...record, payload: value, record_type: value?.kind || recordType });
    hasText = Math.max(hasText, glyph.typeCode === RECORD_GLYPH_TYPES.text ? 1 : 0);
    hasSeries = Math.max(hasSeries, glyph.typeCode === RECORD_GLYPH_TYPES.timeSeries ? 1 : 0);
    hasHistogram = Math.max(hasHistogram, glyph.typeCode === RECORD_GLYPH_TYPES.histogram ? 1 : 0);
    hasVector = Math.max(hasVector, glyph.typeCode === RECORD_GLYPH_TYPES.vector ? 1 : 0);
  }
  const fieldCount = Object.keys(fields).length;
  if (!fieldCount) {
    hasText = typeof payload?.text === "string" ? 1 : 0;
    hasSeries = Array.isArray(payload?.series) ? 1 : 0;
    hasHistogram = Array.isArray(payload?.bins) || Array.isArray(payload?.pixels) ? 1 : 0;
    hasVector = Array.isArray(payload?.values) || Array.isArray(payload?.vector) ? 1 : 0;
  }
  return glyphResult({
    typeCode: RECORD_GLYPH_TYPES.composed,
    family: RECORD_GLYPH_FAMILIES.composed,
    recordType,
    payloadKind,
    label,
    variant: hashUnit(recordId(record) ?? label),
    features: [hasText, hasSeries, hasHistogram, Math.max(hasVector, Math.min(1, fieldCount / 6))],
    complexity: Math.min(1, Math.max(fieldCount / 6, payloadComplexity(payload))),
  });
}

function glyphResult(result) {
  return {
    ...result,
    recordType: stringOr(result.recordType, "record"),
    payloadKind: stringOr(result.payloadKind, "unknown"),
    label: stringOr(result.label, ""),
    variant: clamp01(result.variant),
    complexity: clamp01(result.complexity),
    features: vector4(result.features),
  };
}

function recordMap(records) {
  const out = new Map();
  for (let index = 0; index < records.length; index += 1) {
    const id = recordId(records[index]);
    if (id != null) out.set(String(id), records[index]);
  }
  return out;
}

function recordId(record) {
  if (record == null) return undefined;
  if (typeof record === "string" || typeof record === "number") return record;
  return record.id ?? record.record_id ?? record.recordId;
}

function hasObjectFields(payload) {
  return Boolean(payload?.fields && typeof payload.fields === "object" && !Array.isArray(payload.fields));
}

function stringOr(value, fallback) {
  return value == null || value === "" ? fallback : String(value);
}

function normalizeKind(value) {
  return String(value || "")
    .trim()
    .replace(/_/g, "-")
    .replace(/\s+/g, "-")
    .toLowerCase();
}

function numericArray(value) {
  const out = [];
  collectNumbers(value, out, 96);
  return out;
}

function collectNumbers(value, out, limit) {
  if (out.length >= limit || value == null) return;
  if (typeof value === "number") {
    if (Number.isFinite(value)) out.push(value);
    return;
  }
  if (ArrayBuffer.isView(value) || Array.isArray(value)) {
    for (const entry of value) {
      collectNumbers(entry, out, limit);
      if (out.length >= limit) return;
    }
  }
}

function numericStats(values) {
  if (!values.length) {
    return { center: 0.5, spread: 0, trend: 0.5, peak: 0, energy: 0 };
  }
  let min = Infinity;
  let max = -Infinity;
  let sum = 0;
  let energy = 0;
  let peak = 0;
  for (const value of values) {
    min = Math.min(min, value);
    max = Math.max(max, value);
    sum += value;
    energy += value * value;
    peak = Math.max(peak, Math.abs(value));
  }
  const span = max - min || 1;
  const mean = sum / values.length;
  const first = values[0];
  const last = values[values.length - 1];
  return {
    center: clamp01((mean - min) / span),
    spread: clamp01(span / Math.max(1, Math.abs(max) + Math.abs(min))),
    trend: clamp01(((last - first) / span) * 0.5 + 0.5),
    peak: clamp01(peak / Math.max(1, peak)),
    energy: clamp01(Math.sqrt(energy / values.length) / Math.max(1, peak)),
  };
}

function payloadComplexity(payload) {
  if (payload == null) return 0;
  if (typeof payload === "string") return Math.min(1, payload.length / 96);
  if (typeof payload === "number") return 0.12;
  if (ArrayBuffer.isView(payload) || Array.isArray(payload)) return Math.min(1, payload.length / 64);
  if (typeof payload === "object") return Math.min(1, Object.keys(payload).length / 8);
  return 0.1;
}

function vector4(values) {
  const out = [0, 0, 0, 0];
  const source = Array.isArray(values) || ArrayBuffer.isView(values) ? values : [];
  for (let index = 0; index < 4; index += 1) out[index] = clamp01(Number(source[index]) || 0);
  return out;
}

function numericChannel(array, itemSize, semantic, extra = {}) {
  return {
    semantic,
    itemSize,
    count: itemSize > 0 ? Math.floor(array.length / itemSize) : 0,
    array,
    ...extra,
  };
}

function stringChannel(values, semantic) {
  return {
    semantic,
    itemSize: 1,
    count: values.length,
    array: values.map((value) => value == null ? "" : String(value)),
  };
}

function hashUnit(value) {
  const text = String(value ?? "");
  let hash = 2166136261;
  for (let index = 0; index < text.length; index += 1) {
    hash ^= text.charCodeAt(index);
    hash = Math.imul(hash, 16777619);
  }
  return (hash >>> 0) / 4294967295;
}

function clamp01(value) {
  return Math.max(0, Math.min(1, Number.isFinite(Number(value)) ? Number(value) : 0));
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
