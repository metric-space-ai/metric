#!/usr/bin/env node

import { readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import { MetricVisualSurface } from "../src/metric-visual.js";
import { ProcessCurveSceneView } from "../src/views/ProcessCurveSceneView.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const EVIDENCE_PATH = resolve(ROOT, "docs/examples/assets/condition-monitoring/metric.visual.json");
const PAGE_PATH = resolve(ROOT, "visual/examples/condition-monitoring-hero/index.html");

const checks = [];

class FakeRuntime {
  constructor() {
    this.layers = [];
    this.descriptors = [];
  }

  setLayerDescriptors(descriptors) {
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
assert("page disables derived time-series skyline geometry",
  page.includes("includeRecordSkyline: false"),
  { page: "visual/examples/condition-monitoring-hero/index.html" });
assert("page renders through the condition-monitoring command only",
  page.includes(".showConditionMonitoring(") && !page.includes(".showProcessCurves("),
  {
    hasShowConditionMonitoring: page.includes(".showConditionMonitoring("),
    hasShowProcessCurves: page.includes(".showProcessCurves("),
  });

const view = ProcessCurveSceneView.fromVisualSpace(document_, {
  targetCoordinateId: "process-state-trajectory-3d",
  sourceCoordinateId: "landmark-3d",
  labelPropertyId: "truth-regime",
  relationId: "condition-monitoring-transition",
  graphId: "process-window-trajectory",
  groundField: "metric-anomaly-severity",
  useGraphTrajectory: true,
  includeRecordSkyline: false,
  includeNeighborhood: false,
  includeMatrix: false,
});
const descriptors = view.toLayerDescriptors();
const primitives = descriptors.map((descriptor) => descriptor.primitive || descriptor.kind);
const field = descriptors.find((descriptor) => descriptor.primitive === "HeatFieldLayer");
const path = descriptors.find((descriptor) => descriptor.primitive === "CurveRibbonLayer" || descriptor.primitive === "CurveTubeMeshLayer");
const points = descriptors.find((descriptor) => descriptor.primitive === "InstancedPointLayer");
const labels = descriptors.find((descriptor) => descriptor.primitive === "BillboardLabelLayer");
const skyline = descriptors.find(isDerivedTimeSeriesSkyline);

const surface = createHeadlessSurface(document_);
surface.showConditionMonitoring({
  targetCoordinateId: "process-state-trajectory-3d",
  sourceCoordinateId: "landmark-3d",
  labels: "truth-regime",
  relationId: "condition-monitoring-transition",
  graphId: "process-window-trajectory",
  groundField: "metric-anomaly-severity",
  useGraphTrajectory: true,
  includeRecordSkyline: false,
  preview: false,
});
const commandDescriptors = surface.descriptors;
const commandPrimitives = commandDescriptors.map((descriptor) => descriptor.primitive || descriptor.kind);
const commandPath = commandDescriptors.find((descriptor) => descriptor.primitive === "CurveRibbonLayer" || descriptor.primitive === "CurveTubeMeshLayer");
const commandSkyline = commandDescriptors.find(isDerivedTimeSeriesSkyline);

assert("ProcessCurveSceneView is the primary grammar", view.kind === "process-curves" && view.metadata?.visualGrammar === "process-curves", {
  kind: view.kind,
  metadata: view.metadata,
});
assert("showConditionMonitoring composes the ProcessCurveSceneView grammar once",
  surface.views.length === 1
    && surface.views[0]?.metadata?.viewClass === "ProcessCurveSceneView"
    && surface.lastCommandDiagnostics?.command === "showConditionMonitoring"
    && surface.lastCommandDiagnostics?.selectedViewKind === "condition-monitoring",
  {
    views: surface.views.map((entry) => entry?.metadata?.viewClass || entry?.kind),
    lastCommand: surface.lastCommandDiagnostics,
  });
assert("showConditionMonitoring final descriptors are not a point-cloud-only fallback",
  commandPrimitives.includes("HeatFieldLayer")
    && commandPrimitives.includes("CurveRibbonLayer")
    && commandPrimitives.includes("BillboardLabelLayer")
    && commandPath?.metadata?.viewClass === "TrajectoryPathView"
    && commandPath.metadata?.nativeEvidence?.graphId === "process-window-trajectory",
  {
    primitives: commandPrimitives,
    trajectory: summarizeDescriptor(commandPath),
  });
assert("condition hero does not derive time-series geometry in JavaScript",
  !skyline && !commandSkyline,
  {
    descriptors: descriptors.map(summarizeDescriptor),
    commandDescriptors: commandDescriptors.map(summarizeDescriptor),
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

function isDerivedTimeSeriesSkyline(descriptor) {
  return descriptor?.kind === "record-skyline"
    || descriptor?.metadata?.role === "miniature-record-volume"
    || /curve energy/i.test(String(descriptor?.metadata?.visualizes || ""));
}

function createHeadlessSurface(document) {
  const runtime = new FakeRuntime();
  return new MetricVisualSurface({
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
}
