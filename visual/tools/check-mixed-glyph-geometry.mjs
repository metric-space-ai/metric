#!/usr/bin/env node

import { readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import { MetricVisualSurface } from "../src/index.js";
import {
  RECORD_GLYPH_GEOMETRY_CODES,
  RECORD_GLYPH_MATERIAL_CODES,
  RECORD_GLYPH_RENDER_SCHEMA,
  RECORD_GLYPH_TYPES,
  createRecordGlyphGrammar,
} from "../src/glyphs/index.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const MIXED_ASSET = resolve(ROOT, "docs", "examples", "assets", "mixed-records", "metric.visual.json");
const GLYPH_LAYER = resolve(ROOT, "visual", "src", "layers", "InstancedGlyphLayer.js");

function fail(message, details = {}) {
  console.error(JSON.stringify({ ok: false, message, details }, null, 2));
  process.exit(1);
}

function assert(message, ok, details = {}) {
  if (!ok) fail(message, details);
}

class FakeRuntime {
  constructor() {
    this.descriptors = [];
    this.layers = [];
    this.setCalls = 0;
  }

  setLayerDescriptors(descriptors) {
    this.setCalls += 1;
    this.descriptors = descriptors.slice();
    this.layers = this.descriptors.map((descriptor) => ({ id: descriptor.id, descriptor }));
  }

  renderOnce() {}

  start() {
    this.started = true;
  }

  setCameraOptions(options) {
    this.cameraOptions = { ...options };
  }

  getState() {
    return {
      layerInstanceCount: this.layers.length,
      layerState: { count: this.layers.length },
    };
  }

  on() {
    return () => {};
  }

  selectPair() {}
}

const document = JSON.parse(await readFile(MIXED_ASSET, "utf8"));
const runtime = new FakeRuntime();
const surface = new MetricVisualSurface({
  document,
  canvas: {
    getBoundingClientRect: () => ({ left: 0, top: 0, width: 960, height: 640 }),
    addEventListener() {},
    removeEventListener() {},
  },
  runtime,
  setup: {
    stage: { grounding: { groundY: -0.58 } },
    style: {
      setStyleMotion() {},
      detachStyleMotion() {},
    },
  },
  options: {},
});

surface.showMixedRecords({
  coordinateId: "mixed-finite-records-family-severity-3d",
  glyphBy: "family",
  labels: "family",
  relationId: "mixed-finite-records-composite-metric",
  topK: 5,
  preview: false,
});

const descriptors = runtime.descriptors;
const diagnostics = surface.getDiagnostics();
const command = diagnostics.commands.at(-1);
const glyphDescriptor = descriptors.find((descriptor) => descriptor.primitive === "InstancedGlyphLayer");

assert("showMixedRecords installs descriptors through one runtime call",
  runtime.setCalls === 1
    && command?.selectedCommand === "showMixedRecords"
    && command?.selectedViewKind === "mixed-records"
    && command?.runtimeLayerCount === command?.descriptorCount,
  { setCalls: runtime.setCalls, command });
assert("showMixedRecords stays on MixedRecordView typed-glyph pipeline",
  surface.views?.length === 1
    && surface.views[0]?.kind === "mixed-records"
    && Boolean(glyphDescriptor)
    && !descriptors.some((descriptor) => descriptor.primitive === "InstancedPointLayer"),
  { viewKinds: surface.views?.map((view) => view.kind), primitives: descriptors.map((descriptor) => descriptor.primitive) });
assert("mixed glyph descriptor exposes render geometry and material channels",
  glyphDescriptor.channels.glyphGeometry?.schema === RECORD_GLYPH_RENDER_SCHEMA
    && glyphDescriptor.channels.glyphMaterial?.schema === RECORD_GLYPH_RENDER_SCHEMA
    && glyphDescriptor.channels.glyphGeometry?.itemSize === 4
    && glyphDescriptor.channels.glyphMaterial?.itemSize === 4
    && glyphDescriptor.channels.glyphGeometry?.count === document.records.length
    && glyphDescriptor.channels.glyphMaterial?.count === document.records.length,
  {
    glyphGeometry: summarizeChannel(glyphDescriptor.channels.glyphGeometry),
    glyphMaterial: summarizeChannel(glyphDescriptor.channels.glyphMaterial),
  });
assert("native composed payloads render as composed dashboard/instrument instances",
  uniqueCodes(glyphDescriptor.channels.glyphGeometry).length === 1
    && uniqueCodes(glyphDescriptor.channels.glyphGeometry)[0] === RECORD_GLYPH_GEOMETRY_CODES.composedDashboard
    && uniqueCodes(glyphDescriptor.channels.glyphMaterial).length === 1
    && uniqueCodes(glyphDescriptor.channels.glyphMaterial)[0] === RECORD_GLYPH_MATERIAL_CODES.composedInstrument,
  {
    geometryCodes: uniqueCodes(glyphDescriptor.channels.glyphGeometry),
    materialCodes: uniqueCodes(glyphDescriptor.channels.glyphMaterial),
  });
assert("native composed payloads still expose component slots for typed sub-geometry",
  componentMax(glyphDescriptor.channels.glyphFeature?.array).every((value) => value > 0),
  { componentMax: componentMax(glyphDescriptor.channels.glyphFeature?.array) });

const direct = createRecordGlyphGrammar([
  { id: "text", record_type: "text_note", payload: { kind: "text", text: "operator note" } },
  { id: "series", record_type: "sensor_window", payload: { kind: "time_series", series: [0.1, 0.4, 0.2] } },
  { id: "hist", record_type: "histogram", payload: { kind: "histogram", bins: [1, 3, 2] } },
  { id: "image", record_type: "image_patch", payload: { kind: "image", pixels: [[0, 1], [1, 0]] } },
  { id: "vector", record_type: "embedding", payload: { kind: "vector", values: [0.2, 0.8, 0.5] } },
  { id: "composed", record_type: "mixed_structured_record", payload: { kind: "composed", fields: {
    note: { kind: "text", text: "fault" },
    curve: { kind: "time_series", series: [1, 2, 3] },
    bars: { kind: "histogram", bins: [2, 1, 4] },
    embedding: { kind: "vector", values: [0.3, 0.6] },
  } } },
]);

const directGlyphTypes = scalarValues(direct.channels.glyphType);
const directGeometryCodes = codes(direct.channels.glyphGeometry);
const directMaterialCodes = codes(direct.channels.glyphMaterial);

assert("direct payload kinds map to distinct shader geometry attributes",
  directGeometryCodes.join(",") === [
    RECORD_GLYPH_GEOMETRY_CODES.textCard,
    RECORD_GLYPH_GEOMETRY_CODES.timeSeriesRibbon,
    RECORD_GLYPH_GEOMETRY_CODES.histogramPanel,
    RECORD_GLYPH_GEOMETRY_CODES.imageTile,
    RECORD_GLYPH_GEOMETRY_CODES.vectorDiamond,
    RECORD_GLYPH_GEOMETRY_CODES.composedDashboard,
  ].join(","),
  { directGeometryCodes });
assert("direct payload kinds map to distinct shader material attributes",
  directMaterialCodes.join(",") === [
    RECORD_GLYPH_MATERIAL_CODES.paperInk,
    RECORD_GLYPH_MATERIAL_CODES.signalGlass,
    RECORD_GLYPH_MATERIAL_CODES.histogramCeramic,
    RECORD_GLYPH_MATERIAL_CODES.imageLumaTile,
    RECORD_GLYPH_MATERIAL_CODES.vectorMetal,
    RECORD_GLYPH_MATERIAL_CODES.composedInstrument,
  ].join(","),
  { directMaterialCodes });
assert("image and histogram no longer depend on the legacy shared glyph type",
  directGlyphTypes[2] === RECORD_GLYPH_TYPES.histogram
    && directGlyphTypes[3] === RECORD_GLYPH_TYPES.image
    && directGlyphTypes[2] === directGlyphTypes[3]
    && directGeometryCodes[2] !== directGeometryCodes[3]
    && directMaterialCodes[2] !== directMaterialCodes[3],
  {
    histogramType: directGlyphTypes[2],
    imageType: directGlyphTypes[3],
    histogramGeometry: directGeometryCodes[2],
    imageGeometry: directGeometryCodes[3],
    histogramMaterial: directMaterialCodes[2],
    imageMaterial: directMaterialCodes[3],
  });
assert("render atlas carries geometry metadata for families and payload kinds",
  direct.renderSemantics?.schema === RECORD_GLYPH_RENDER_SCHEMA
    && direct.renderSemantics?.families?.length === 6
    && direct.renderSemantics?.payloadKinds?.length === 6
    && direct.renderSemantics?.families?.every((entry) => Number.isInteger(entry.geometryCode) && Number.isInteger(entry.materialCode)),
  { renderSemantics: direct.renderSemantics });

const shader = await readFile(GLYPH_LAYER, "utf8");
assert("InstancedGlyphLayer shader declares glyph geometry and material attributes",
  /attribute\s+vec4\s+aGlyphGeometry\b/.test(shader)
    && /attribute\s+vec4\s+aGlyphMaterial\b/.test(shader)
    && /varying\s+vec4\s+vGlyphGeometry\b/.test(shader)
    && /varying\s+vec4\s+vGlyphMaterial\b/.test(shader),
  {});
assert("InstancedGlyphLayer picking consumes glyph geometry masks",
  /glyphPickAlpha\s*\(/.test(shader)
    && /bindAttribute\([^)]*"aGlyphGeometry"/.test(shader),
  {});

console.log(JSON.stringify({
  ok: true,
  asset: "docs/examples/assets/mixed-records/metric.visual.json",
  records: document.records.length,
  runtimeSetCalls: runtime.setCalls,
  descriptorCount: descriptors.length,
  nativeGeometryCodes: uniqueCodes(glyphDescriptor.channels.glyphGeometry),
  nativeMaterialCodes: uniqueCodes(glyphDescriptor.channels.glyphMaterial),
  directGeometryCodes,
  directMaterialCodes,
  shaderAttributes: ["aGlyphGeometry", "aGlyphMaterial"],
}, null, 2));

function summarizeChannel(channel) {
  return {
    schema: channel?.schema,
    itemSize: channel?.itemSize,
    count: channel?.count,
    semantic: channel?.semantic,
  };
}

function scalarValues(channel) {
  return Array.from(channel?.array || [], (value) => Math.round(Number(value) || 0));
}

function codes(channel) {
  const array = channel?.array || [];
  const out = [];
  for (let offset = 0; offset < array.length; offset += 4) {
    out.push(Math.round(Number(array[offset]) || 0));
  }
  return out;
}

function uniqueCodes(channel) {
  return Array.from(new Set(codes(channel))).sort((a, b) => a - b);
}

function componentMax(array = []) {
  const out = [0, 0, 0, 0];
  for (let offset = 0; offset < array.length; offset += 4) {
    for (let component = 0; component < 4; component += 1) {
      out[component] = Math.max(out[component], Number(array[offset + component]) || 0);
    }
  }
  return out;
}
