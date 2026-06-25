#!/usr/bin/env node

import { readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import {
  RECORD_GLYPH_GEOMETRY_CODES,
  RECORD_GLYPH_DIAGNOSTICS_SCHEMA,
  RECORD_GLYPH_FAMILIES,
  RECORD_GLYPH_MATERIAL_CODES,
  RECORD_GLYPH_RENDER_SCHEMA,
  RECORD_GLYPH_TYPES,
  createRecordGlyphGrammar,
} from "../src/glyphs/index.js";
import { MixedRecordView } from "../src/views/index.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const MIXED_ASSET = resolve(ROOT, "docs", "examples", "assets", "mixed-records", "metric.visual.json");

function fail(message, details = {}) {
  console.error(JSON.stringify({ ok: false, message, details }, null, 2));
  process.exit(1);
}

function assert(message, ok, details = {}) {
  if (!ok) fail(message, details);
}

const document = JSON.parse(await readFile(MIXED_ASSET, "utf8"));
const view = MixedRecordView.fromVisualSpace(document, {
  coordinateId: "mixed-finite-records-family-severity-3d",
  colorProperty: "family",
  labelProperty: "family",
  pointSize: 1.4,
  relationId: "mixed-finite-records-composite-metric",
  crossTypeRelations: true,
  topK: 5,
});
const descriptors = view.toLayerDescriptors();
const glyphDescriptor = descriptors.find((descriptor) => descriptor.primitive === "InstancedGlyphLayer");
const edgeDescriptor = descriptors.find((descriptor) => descriptor.primitive === "RelationEdgeLayer");

assert("mixed record view emits a primary typed glyph descriptor", Boolean(glyphDescriptor), {
  primitives: descriptors.map((descriptor) => descriptor.primitive || descriptor.kind),
});
assert("mixed record grammar is owned by MixedRecordView",
  view.kind === "mixed-records"
    && glyphDescriptor.source?.viewKind === "mixed-records"
    && glyphDescriptor.source?.viewId === view.id,
  { viewKind: view.kind, source: glyphDescriptor.source });
assert("mixed record view does not reduce composed records to a primary point cloud",
  !descriptors.some((descriptor) => descriptor.primitive === "InstancedPointLayer"),
  { primitives: descriptors.map((descriptor) => descriptor.primitive || descriptor.kind) });
assert("glyph descriptor preserves one record id per exported record",
  glyphDescriptor.channels.recordId?.count === document.records.length,
  { count: glyphDescriptor.channels.recordId?.count, records: document.records.length });
assert("glyph descriptor carries record payload kind metadata",
  glyphDescriptor.channels.payloadKind?.array?.every((value) => value === "composed"),
  { payloadKinds: Array.from(new Set(glyphDescriptor.channels.payloadKind?.array || [])) });
assert("glyph descriptor carries record type metadata",
  glyphDescriptor.channels.recordType?.array?.every((value) => value === "mixed_structured_record"),
  { recordTypes: Array.from(new Set(glyphDescriptor.channels.recordType?.array || [])) });
assert("composed records map to composed-record glyph family",
  glyphDescriptor.channels.glyphFamily?.array?.every((value) => value === RECORD_GLYPH_FAMILIES.composed),
  { families: Array.from(new Set(glyphDescriptor.channels.glyphFamily?.array || [])) });
assert("composed records map to composed glyph type code",
  Array.from(glyphDescriptor.channels.glyphType?.array || []).every((value) => value === RECORD_GLYPH_TYPES.composed),
  { glyphTypes: Array.from(new Set(glyphDescriptor.channels.glyphType?.array || [])) });
assert("typed glyph descriptor carries render geometry shader attributes",
  glyphDescriptor.channels.glyphGeometry?.schema === RECORD_GLYPH_RENDER_SCHEMA
    && glyphDescriptor.channels.glyphGeometry?.itemSize === 4
    && glyphDescriptor.channels.glyphGeometry?.count === document.records.length,
  {
    schema: glyphDescriptor.channels.glyphGeometry?.schema,
    itemSize: glyphDescriptor.channels.glyphGeometry?.itemSize,
    count: glyphDescriptor.channels.glyphGeometry?.count,
  });
assert("typed glyph descriptor carries render material shader attributes",
  glyphDescriptor.channels.glyphMaterial?.schema === RECORD_GLYPH_RENDER_SCHEMA
    && glyphDescriptor.channels.glyphMaterial?.itemSize === 4
    && glyphDescriptor.channels.glyphMaterial?.count === document.records.length,
  {
    schema: glyphDescriptor.channels.glyphMaterial?.schema,
    itemSize: glyphDescriptor.channels.glyphMaterial?.itemSize,
    count: glyphDescriptor.channels.glyphMaterial?.count,
  });
assert("native composed records map to composed dashboard geometry metadata",
  geometryCodes(glyphDescriptor).every((value) => value === RECORD_GLYPH_GEOMETRY_CODES.composedDashboard),
  { geometryCodes: Array.from(new Set(geometryCodes(glyphDescriptor))) });
assert("native composed records map to composed instrument material metadata",
  materialCodes(glyphDescriptor).every((value) => value === RECORD_GLYPH_MATERIAL_CODES.composedInstrument),
  { materialCodes: Array.from(new Set(materialCodes(glyphDescriptor))) });

const feature = glyphDescriptor.channels.glyphFeature?.array || [];
const componentMax = [0, 0, 0, 0];
for (let index = 0; index < feature.length; index += 4) {
  for (let component = 0; component < 4; component += 1) {
    componentMax[component] = Math.max(componentMax[component], feature[index + component]);
  }
}
assert("composed glyph features expose text, time-series, histogram/image and vector components",
  componentMax.every((value) => value > 0),
  { componentMax });
assert("glyph grammar exposes label anchors without HTML overlays",
  glyphDescriptor.channels.labelAnchor?.count === document.records.length
    && glyphDescriptor.geometry?.labelAnchors === true,
  {
    count: glyphDescriptor.channels.labelAnchor?.count,
    labelAnchors: glyphDescriptor.geometry?.labelAnchors,
  });
assert("typed glyph descriptor remains pickable by record id",
  glyphDescriptor.picking?.mode === "record-id" && glyphDescriptor.picking?.channel === "recordId",
  { picking: glyphDescriptor.picking });
assert("typed glyph descriptor declares typed record glyphs as primary grammar",
  glyphDescriptor.metadata?.primaryGrammar === "typed-record-glyphs"
    && glyphDescriptor.metadata?.role === "typed-glyphs",
  { metadata: glyphDescriptor.metadata });
assert("typed glyph descriptor declares type-specific semantic channels",
  glyphDescriptor.metadata?.typedGlyphSemantics?.recordIdentity === "recordId"
    && glyphDescriptor.metadata?.typedGlyphSemantics?.glyphType === "glyphType"
    && glyphDescriptor.metadata?.typedGlyphSemantics?.glyphFamily === "glyphFamily"
    && glyphDescriptor.metadata?.typedGlyphSemantics?.payloadKind === "payloadKind",
  { semantics: glyphDescriptor.metadata?.typedGlyphSemantics });
assert("mixed record view emits cross-type relation edges from native relation evidence",
  edgeDescriptor?.primitive === "RelationEdgeLayer"
    && edgeDescriptor.source?.viewKind === "mixed-records"
    && edgeDescriptor.source?.relationId === "mixed-finite-records-composite-metric"
    && edgeDescriptor.metadata?.primaryGrammar === "cross-type-relation-edges"
    && edgeDescriptor.metadata?.graph?.native === true
    && edgeDescriptor.metadata?.graph?.edgeCount > 0,
  {
    primitive: edgeDescriptor?.primitive,
    source: edgeDescriptor?.source,
    metadata: edgeDescriptor?.metadata,
  });

const familyGrammar = createRecordGlyphGrammar([
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
for (const family of [
  RECORD_GLYPH_FAMILIES.text,
  RECORD_GLYPH_FAMILIES.timeSeries,
  RECORD_GLYPH_FAMILIES.histogram,
  RECORD_GLYPH_FAMILIES.image,
  RECORD_GLYPH_FAMILIES.vector,
  RECORD_GLYPH_FAMILIES.composed,
]) {
  assert(`record glyph grammar supports ${family}`, familyGrammar.families.includes(family), {
    families: familyGrammar.families,
  });
}
const familyGeometryCodes = channelCodes(familyGrammar.channels.glyphGeometry);
const familyMaterialCodes = channelCodes(familyGrammar.channels.glyphMaterial);
assert("direct heterogeneous payloads map to distinct render geometry codes",
  new Set(familyGeometryCodes).size >= 6
    && familyGeometryCodes.includes(RECORD_GLYPH_GEOMETRY_CODES.textCard)
    && familyGeometryCodes.includes(RECORD_GLYPH_GEOMETRY_CODES.timeSeriesRibbon)
    && familyGeometryCodes.includes(RECORD_GLYPH_GEOMETRY_CODES.histogramPanel)
    && familyGeometryCodes.includes(RECORD_GLYPH_GEOMETRY_CODES.imageTile)
    && familyGeometryCodes.includes(RECORD_GLYPH_GEOMETRY_CODES.vectorDiamond)
    && familyGeometryCodes.includes(RECORD_GLYPH_GEOMETRY_CODES.composedDashboard),
  { familyGeometryCodes });
assert("direct heterogeneous payloads map to distinct render material codes",
  new Set(familyMaterialCodes).size >= 6
    && familyMaterialCodes.includes(RECORD_GLYPH_MATERIAL_CODES.paperInk)
    && familyMaterialCodes.includes(RECORD_GLYPH_MATERIAL_CODES.signalGlass)
    && familyMaterialCodes.includes(RECORD_GLYPH_MATERIAL_CODES.histogramCeramic)
    && familyMaterialCodes.includes(RECORD_GLYPH_MATERIAL_CODES.imageLumaTile)
    && familyMaterialCodes.includes(RECORD_GLYPH_MATERIAL_CODES.vectorMetal)
    && familyMaterialCodes.includes(RECORD_GLYPH_MATERIAL_CODES.composedInstrument),
  { familyMaterialCodes });
assert("histogram and image payloads no longer collapse to identical render geometry",
  familyGeometryCodes[2] === RECORD_GLYPH_GEOMETRY_CODES.histogramPanel
    && familyGeometryCodes[3] === RECORD_GLYPH_GEOMETRY_CODES.imageTile
    && familyGeometryCodes[2] !== familyGeometryCodes[3],
  { histogramGeometry: familyGeometryCodes[2], imageGeometry: familyGeometryCodes[3] });
assert("render atlas declares geometry metadata for glyph families and payload kinds",
  familyGrammar.renderSemantics?.schema === RECORD_GLYPH_RENDER_SCHEMA
    && familyGrammar.renderSemantics?.families?.length >= 6
    && familyGrammar.renderSemantics?.payloadKinds?.length >= 6
    && familyGrammar.renderSemantics?.shaderAttributes?.glyphGeometry?.components?.includes("geometryCode")
    && familyGrammar.renderSemantics?.shaderAttributes?.glyphMaterial?.components?.includes("materialCode"),
  { renderSemantics: familyGrammar.renderSemantics });
assert("record glyph grammar exposes typed diagnostics and material contracts",
  familyGrammar.diagnostics?.schema === RECORD_GLYPH_DIAGNOSTICS_SCHEMA
    && familyGrammar.diagnostics?.typedRecordCount === 6
    && familyGrammar.diagnostics?.fallbackRecordCount === 0
    && familyGrammar.diagnostics?.distinctGeometryCount >= 6
    && familyGrammar.diagnostics?.distinctMaterialCount >= 6
    && familyGrammar.diagnostics?.identityContract?.fallbackPreservesRecordIdentity === true
    && familyGrammar.diagnostics?.renderContract?.geometryShaderAttribute === "aGlyphGeometry"
    && familyGrammar.diagnostics?.renderContract?.materialShaderAttribute === "aGlyphMaterial"
    && familyGrammar.diagnostics?.labelContract?.htmlOverlayRequired === false,
  { diagnostics: familyGrammar.diagnostics });

console.log(JSON.stringify({
  ok: true,
  asset: "docs/examples/assets/mixed-records/metric.visual.json",
  records: document.records.length,
  glyphFamilies: Array.from(new Set(glyphDescriptor.channels.glyphFamily.array)),
  payloadKinds: Array.from(new Set(glyphDescriptor.channels.payloadKind.array)),
  geometryCodes: Array.from(new Set(geometryCodes(glyphDescriptor))),
  materialCodes: Array.from(new Set(materialCodes(glyphDescriptor))),
  edgeCount: edgeDescriptor.metadata.graph.edgeCount,
  primitive: "InstancedGlyphLayer",
}, null, 2));

function geometryCodes(descriptor) {
  return channelCodes(descriptor.channels.glyphGeometry);
}

function materialCodes(descriptor) {
  return channelCodes(descriptor.channels.glyphMaterial);
}

function channelCodes(channel) {
  const array = channel?.array || [];
  const out = [];
  for (let offset = 0; offset < array.length; offset += 4) {
    out.push(Math.round(Number(array[offset]) || 0));
  }
  return out;
}
