#!/usr/bin/env node

import { readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import { ProcessCurveSceneView } from "../src/views/ProcessCurveSceneView.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const EVIDENCE_PATH = resolve(ROOT, "docs/examples/assets/condition-monitoring/metric.visual.json");
const PAGE_PATH = resolve(ROOT, "visual/examples/condition-monitoring-hero/index.html");

const checks = [];

const document_ = JSON.parse(await readFile(EVIDENCE_PATH, "utf8"));
const page = await readFile(PAGE_PATH, "utf8");

assert("uses metric.visual.v1", document_.schema === "metric.visual.v1", { schema: document_.schema });
assert("uses native condition-monitoring export", document_.provenance?.native_export === true && document_.provenance?.synthetic_js !== true, document_.provenance);
assert("record density meets W4 minimum", Array.isArray(document_.records) && document_.records.length >= 500, { records: document_.records?.length });
assert("relations and graph are exported", (document_.relations || []).length >= 1 && (document_.graphs || []).some((graph) => graph.id === "process-window-trajectory"), {
  relations: document_.relations?.map((relation) => relation.id),
  graphs: document_.graphs?.map((graph) => graph.id),
});

const requiredProperties = ["metric-anomaly-severity", "local-density", "truth-regime"];
for (const propertyId of requiredProperties) {
  const property = propertyById(document_, propertyId);
  assert(`has exported property ${propertyId}`, Boolean(property), { properties: document_.properties?.map((entry) => entry.id) });
  assert(`${propertyId} covers every record`, property?.values?.length === document_.records.length, {
    propertyId,
    values: property?.values?.length,
    records: document_.records.length,
  });
}

const regimes = new Set(propertyById(document_, "truth-regime").values.map((entry) => String(entry.value)));
for (const regime of ["normal", "drift", "fault", "recovery"]) {
  assert(`regime ${regime} is exported`, regimes.has(regime), { regimes: [...regimes].sort() });
}
assert("time-series preview payloads are exported",
  document_.records.every((record) => record.payload?.kind === "time_series" && Array.isArray(record.payload?.series) && record.payload.series.length >= 8),
  { sample: document_.records[0]?.payload });

assert("page loads only the native condition-monitoring visual asset",
  page.includes("docs/examples/assets/condition-monitoring/metric.visual.json") && !/fetch\(\s*["'][^"']*evidence\.json/.test(page),
  { page: "visual/examples/condition-monitoring-hero/index.html" });
assert("page renders through ProcessCurveSceneView command path",
  page.includes(".showProcessCurves(") && page.includes("useGraphTrajectory: true"),
  { hasShowProcessCurves: page.includes(".showProcessCurves(") });
assert("page keeps public preview condition command marker without using local evidence",
  page.includes(".showConditionMonitoring("),
  { hasShowConditionMonitoring: page.includes(".showConditionMonitoring(") });

const view = ProcessCurveSceneView.fromVisualSpace(document_, {
  targetCoordinateId: "process-state-trajectory-3d",
  sourceCoordinateId: "landmark-3d",
  labelPropertyId: "truth-regime",
  relationId: "condition-monitoring-transition",
  graphId: "process-window-trajectory",
  groundField: "metric-anomaly-severity",
  useGraphTrajectory: true,
  includeNeighborhood: false,
  includeMatrix: false,
});
const descriptors = view.toLayerDescriptors();
const primitives = descriptors.map((descriptor) => descriptor.primitive || descriptor.kind);
const field = descriptors.find((descriptor) => descriptor.primitive === "HeatFieldLayer");
const path = descriptors.find((descriptor) => descriptor.primitive === "CurveRibbonLayer" || descriptor.primitive === "CurveTubeMeshLayer");
const points = descriptors.find((descriptor) => descriptor.primitive === "InstancedPointLayer");
const labels = descriptors.find((descriptor) => descriptor.primitive === "BillboardLabelLayer");

assert("ProcessCurveSceneView is the primary grammar", view.kind === "process-curves" && view.metadata?.visualGrammar === "process-curves", {
  kind: view.kind,
  metadata: view.metadata,
});
assert("exported anomaly field is emitted by PropertyFieldView", field?.metadata?.viewClass === "PropertyFieldView"
  && field.metadata?.propertyId === "metric-anomaly-severity"
  && field.metadata?.algorithmicComputation === false
  && field.metadata?.recordCount === document_.records.length,
  { field: summarizeDescriptor(field) });
assert("transition trajectory is emitted by TrajectoryPathView", path?.metadata?.viewClass === "TrajectoryPathView"
  && path.metadata?.nativeEvidence?.graphId === "process-window-trajectory"
  && path.metadata?.recordCount === document_.records.length
  && path.metadata?.algorithmicComputation === false,
  { path: summarizeDescriptor(path) });
assert("supporting metric-space records are present", points?.metadata?.recordCount === document_.records.length, {
  points: summarizeDescriptor(points),
});
assert("3D regime labels are emitted inside the scene", labels?.metadata?.viewClass === "ProcessCurveSceneView"
  && labels.metadata?.role === "region-labels"
  && labels.metadata?.propertyId === "truth-regime"
  && labels.metadata?.labelCount >= 4,
  { labels: summarizeDescriptor(labels) });
assert("scene is not point-cloud-only", primitives.includes("HeatFieldLayer") && primitives.includes("CurveRibbonLayer") && primitives.includes("BillboardLabelLayer"), {
  primitives,
});

const failures = checks.filter((check) => !check.ok);
if (failures.length) {
  console.error(JSON.stringify({ ok: false, failures }, null, 2));
  process.exit(1);
}

console.log(JSON.stringify({
  ok: true,
  evidence: "docs/examples/assets/condition-monitoring/metric.visual.json",
  page: "visual/examples/condition-monitoring-hero/index.html",
  records: document_.records.length,
  regimes: [...regimes].sort(),
  primitives,
  field: summarizeDescriptor(field),
  trajectory: summarizeDescriptor(path),
  labels: summarizeDescriptor(labels),
}, null, 2));

function propertyById(document, id) {
  return (document.properties || []).find((property) => property.id === id);
}

function assert(message, ok, details = {}) {
  checks.push({ ok: Boolean(ok), message, details: ok ? undefined : details });
}

function summarizeDescriptor(descriptor) {
  if (!descriptor) return null;
  return {
    id: descriptor.id,
    primitive: descriptor.primitive || descriptor.kind,
    role: descriptor.metadata?.role || null,
    viewClass: descriptor.metadata?.viewClass || null,
    propertyId: descriptor.metadata?.propertyId || descriptor.source?.propertyId || null,
    recordCount: descriptor.metadata?.recordCount || descriptor.channels?.recordId?.count || null,
    pathCount: descriptor.metadata?.pathCount || null,
    graphId: descriptor.metadata?.nativeEvidence?.graphId || descriptor.metadata?.graphId || null,
    labelCount: descriptor.metadata?.labelCount || null,
  };
}
