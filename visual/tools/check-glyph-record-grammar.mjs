#!/usr/bin/env node

import { readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import { RECORD_GLYPH_FAMILIES, RECORD_GLYPH_TYPES } from "../src/glyphs/index.js";
import { MetricSpaceView } from "../src/views/index.js";

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
const view = MetricSpaceView.fromVisualSpace(document, {
  coordinateId: "mixed-finite-records-family-severity-3d",
  colorProperty: "family",
  labelProperty: "family",
  pointSize: 1.4,
});
const pointDescriptor = view.toLayerDescriptors().find((descriptor) => descriptor.primitive === "InstancedPointLayer");
const glyphDescriptor = pointDescriptor
  ? {
    ...pointDescriptor,
    kind: "typed-glyph-scene",
    primitive: "InstancedGlyphLayer",
  }
  : null;

assert("mixed record view emits a glyph-ready record descriptor", Boolean(glyphDescriptor));
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

console.log(JSON.stringify({
  ok: true,
  asset: "docs/examples/assets/mixed-records/metric.visual.json",
  records: document.records.length,
  glyphFamilies: Array.from(new Set(glyphDescriptor.channels.glyphFamily.array)),
  payloadKinds: Array.from(new Set(glyphDescriptor.channels.payloadKind.array)),
  primitive: "InstancedGlyphLayer",
}, null, 2));
