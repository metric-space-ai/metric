#!/usr/bin/env node

import { readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import {
  PROPERTY_FIELD_GRAMMAR_SCHEMA,
  SpacePropertiesView,
} from "../src/views/SpacePropertiesView.js";
import { HeatFieldLayer, heatFieldRampMode } from "../src/layers/HeatFieldLayer.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const CONDITION_ASSET = resolve(ROOT, "docs", "examples", "assets", "condition-monitoring", "metric.visual.json");
const METRIC_SPACE_FIXTURE = resolve(ROOT, "visual", "examples", "fixtures", "metric-space.visual.json");

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
  return { document, view, descriptors, field };
}

const document = JSON.parse(await readFile(CONDITION_ASSET, "utf8"));
const metricSpaceFixture = JSON.parse(await readFile(METRIC_SPACE_FIXTURE, "utf8"));
const density = propertyField(document, "local-density");
const anomaly = propertyField(document, "metric-anomaly-severity", { fieldMode: "lifted" });
const outlier = propertyField(document, "dbscan-density-outlier-flag");
const entropy = propertyField(metricSpaceFixture, "entropy", {
  coordinateId: "landmark-3d",
});

for (const [label, result] of [["density", density], ["entropy", entropy], ["anomaly", anomaly], ["outlier", outlier]]) {
  assert(`${label} field emits HeatFieldLayer`, Boolean(result.field));
  assert(`${label} field uses exported record ids`,
    result.field.channels.recordId?.count === result.document.records.length,
    { count: result.field.channels.recordId?.count, records: result.document.records.length });
  assert(`${label} field uses exported scalar property`,
    result.field.channels.scalar?.count === result.document.records.length
      && result.field.source?.scalarSource === "exported-property",
    { source: result.field.source, scalarCount: result.field.channels.scalar?.count });
  assert(`${label} field remains linked to record selection`,
    result.field.picking?.mode === "record-id" && result.field.channels.selection?.count === result.document.records.length,
    { picking: result.field.picking, selectionCount: result.field.channels.selection?.count });
  assert(`${label} field has scalar domain metadata`,
    Number.isFinite(result.field.metadata?.scalarDomain?.min)
      && Number.isFinite(result.field.metadata?.scalarDomain?.max),
    { domain: result.field.metadata?.scalarDomain });
  assert(`${label} field declares a first-class property field grammar`,
    result.field.geometry?.grammar === PROPERTY_FIELD_GRAMMAR_SCHEMA
      && result.field.metadata?.propertyFieldGrammar?.schema === PROPERTY_FIELD_GRAMMAR_SCHEMA
      && result.field.metadata?.propertyFieldGrammar?.layerContract?.primitive === "HeatFieldLayer"
      && result.field.metadata?.propertyFieldGrammar?.layerContract?.pickingMode === "record-id",
    {
      geometry: result.field.geometry,
      grammar: result.field.metadata?.propertyFieldGrammar,
    });
  assert(`${label} field diagnostics reject generic point-cloud collapse`,
    result.field.metadata?.fieldDiagnostics?.notGenericPointCloud === true
      && result.field.metadata?.fieldDiagnostics?.source === "exported-scalar-property"
      && result.field.metadata?.fieldDiagnostics?.selectionLinked === true
      && result.field.metadata?.fieldDiagnostics?.finiteScalarCount === result.document.records.length,
    { diagnostics: result.field.metadata?.fieldDiagnostics });
  assert(`${label} field style contract is semantic, not page-specific`,
    result.field.material?.fieldStyle?.ramp === result.field.metadata?.propertySemantic
      && result.field.material?.fieldStyle?.scalarEncoding === "normalized-exported-property"
      && result.field.material?.fieldStyle?.selection === "record-linked-highlight",
    { fieldStyle: result.field.material?.fieldStyle });
  const layer = new HeatFieldLayer(result.field);
  const layerDiagnostics = layer.getGrammarDiagnostics();
  assert(`${label} HeatFieldLayer exposes grammar diagnostics`,
    layerDiagnostics.primitive === "HeatFieldLayer"
      && layerDiagnostics.propertySemantic === result.field.metadata?.propertySemantic
      && layerDiagnostics.recordCount === result.document.records.length
      && layerDiagnostics.rampMode === heatFieldRampMode(result.field.metadata?.propertySemantic)
      && layerDiagnostics.selectionLinked === true,
    { layerDiagnostics });
}

assert("density property maps to density ramp", density.field.metadata.propertySemantic === "density", {
  semantic: density.field.metadata.propertySemantic,
});
assert("entropy property maps to entropy ramp", entropy.field.metadata.propertySemantic === "entropy", {
  semantic: entropy.field.metadata.propertySemantic,
});
assert("anomaly property maps to anomaly ramp", anomaly.field.metadata.propertySemantic === "anomaly", {
  semantic: anomaly.field.metadata.propertySemantic,
});
assert("outlier property maps to outlier ramp", outlier.field.metadata.propertySemantic === "outlier", {
  semantic: outlier.field.metadata.propertySemantic,
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
      propertyId: entropy.field.metadata.propertyId,
      semantic: entropy.field.metadata.propertySemantic,
      mode: entropy.field.geometry.mode,
    },
    {
      propertyId: anomaly.field.metadata.propertyId,
      semantic: anomaly.field.metadata.propertySemantic,
      mode: anomaly.field.geometry.mode,
    },
    {
      propertyId: outlier.field.metadata.propertyId,
      semantic: outlier.field.metadata.propertySemantic,
      mode: outlier.field.geometry.mode,
    },
  ],
}, null, 2));
