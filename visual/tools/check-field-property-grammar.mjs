#!/usr/bin/env node

import { readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import { SpacePropertiesView } from "../src/views/index.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const CONDITION_ASSET = resolve(ROOT, "docs", "examples", "assets", "condition-monitoring", "metric.visual.json");

function fail(message, details = {}) {
  console.error(JSON.stringify({ ok: false, message, details }, null, 2));
  process.exit(1);
}

function assert(message, ok, details = {}) {
  if (!ok) fail(message, details);
}

function propertyField(document, propertyId, options = {}) {
  const view = SpacePropertiesView.fromVisualSpace(document, {
    coordinateId: "process-state-trajectory-3d",
    propertyId,
    ...options,
  });
  const descriptors = view.toLayerDescriptors();
  const field = descriptors.find((descriptor) => descriptor.primitive === "HeatFieldLayer" && descriptor.metadata?.role === "property-field");
  return { view, descriptors, field };
}

const document = JSON.parse(await readFile(CONDITION_ASSET, "utf8"));
const density = propertyField(document, "local-density");
const anomaly = propertyField(document, "metric-anomaly-severity", { fieldMode: "lifted" });

for (const [label, result] of [["density", density], ["anomaly", anomaly]]) {
  assert(`${label} field emits HeatFieldLayer`, Boolean(result.field));
  assert(`${label} field uses exported record ids`,
    result.field.channels.recordId?.count === document.records.length,
    { count: result.field.channels.recordId?.count, records: document.records.length });
  assert(`${label} field uses exported scalar property`,
    result.field.channels.scalar?.count === document.records.length
      && result.field.source?.scalarSource === "exported-property",
    { source: result.field.source, scalarCount: result.field.channels.scalar?.count });
  assert(`${label} field remains linked to record selection`,
    result.field.picking?.mode === "record-id" && result.field.channels.selection?.count === document.records.length,
    { picking: result.field.picking, selectionCount: result.field.channels.selection?.count });
  assert(`${label} field has scalar domain metadata`,
    Number.isFinite(result.field.metadata?.scalarDomain?.min)
      && Number.isFinite(result.field.metadata?.scalarDomain?.max),
    { domain: result.field.metadata?.scalarDomain });
}

assert("density property maps to density ramp", density.field.metadata.propertySemantic === "density", {
  semantic: density.field.metadata.propertySemantic,
});
assert("anomaly property maps to anomaly ramp", anomaly.field.metadata.propertySemantic === "anomaly", {
  semantic: anomaly.field.metadata.propertySemantic,
});
assert("lifted field mode is represented in geometry and metadata",
  anomaly.field.geometry.mode === "lifted-property-field" && anomaly.field.metadata.fieldMode === "lifted",
  { geometry: anomaly.field.geometry, fieldMode: anomaly.field.metadata.fieldMode });

console.log(JSON.stringify({
  ok: true,
  asset: "docs/examples/assets/condition-monitoring/metric.visual.json",
  records: document.records.length,
  fields: [
    {
      propertyId: density.field.metadata.propertyId,
      semantic: density.field.metadata.propertySemantic,
      mode: density.field.geometry.mode,
    },
    {
      propertyId: anomaly.field.metadata.propertyId,
      semantic: anomaly.field.metadata.propertySemantic,
      mode: anomaly.field.geometry.mode,
    },
  ],
}, null, 2));
