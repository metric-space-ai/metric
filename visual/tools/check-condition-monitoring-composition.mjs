#!/usr/bin/env node

import { readFile, stat } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import { MetricVisualSurface } from "../src/metric-visual.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const EVIDENCE_PATH = resolve(ROOT, "docs/examples/assets/condition-monitoring/metric.visual.json");
const PAGE_PATH = resolve(ROOT, "visual/examples/condition-monitoring-hero/index.html");
const PAGE_SCREENSHOT = resolve(ROOT, "output/visual/check-visual-regression-public-examples/condition-monitoring-hero.png");
const CANVAS_SCREENSHOT = resolve(ROOT, "output/visual/check-visual-regression-public-examples/condition-monitoring-hero.canvas.png");
const HIERARCHY_SCHEMA = "metric.visual.layer_hierarchy.v1";

const checks = [];
const document_ = JSON.parse(await readFile(EVIDENCE_PATH, "utf8"));
const page = await readFile(PAGE_PATH, "utf8");

const surface = createHeadlessSurface(document_);
surface.showConditionMonitoring({
  targetCoordinateId: "process-state-trajectory-3d",
  sourceCoordinateId: "landmark-3d",
  colorBy: "metric-anomaly-severity",
  groundField: "local-density",
  labels: "truth-regime",
  relationId: "condition-monitoring-transition",
  graphId: "process-window-trajectory",
  propertyField: "metric-anomaly-severity",
  useGraphTrajectory: true,
  includeRecordSkyline: false,
  includeNeighborhood: false,
  includeMatrix: false,
  trackMode: "ribbon",
  trackWidth: 3.7,
  trackWidthRange: [2.2, 4.9],
  trackAlpha: 0.78,
  fieldAlpha: 0.54,
  fieldRadius: 0.3,
  fieldPointPixelScale: 70,
  projectionAlpha: 0.28,
  projectionSize: 2.1,
  labelFontSize: 18,
  labelLift: 0.38,
  labelOffsetRadius: 0.28,
  labelSize: 0.16,
  preview: false,
});

const descriptors = surface.descriptors || [];
const primitives = new Set(descriptors.map((descriptor) => descriptor.primitive || descriptor.kind).filter(Boolean));
const field = descriptors.find((descriptor) => descriptor.primitive === "HeatFieldLayer");
const projection = descriptors.find((descriptor) => descriptor.primitive === "GroundProjectionLayer");
const trajectory = descriptors.find((descriptor) => descriptor.primitive === "CurveRibbonLayer" || descriptor.primitive === "CurveTubeMeshLayer");
const points = descriptors.find((descriptor) => descriptor.primitive === "InstancedPointLayer");
const labels = descriptors.find((descriptor) => descriptor.primitive === "BillboardLabelLayer");

assert("selected view kind is condition-monitoring",
  surface.lastCommandDiagnostics?.selectedViewKind === "condition-monitoring"
    && surface.lastCommandDiagnostics?.command === "showConditionMonitoring",
  surface.lastCommandDiagnostics);

for (const primitive of ["HeatFieldLayer", "GroundProjectionLayer", "CurveRibbonLayer", "InstancedPointLayer"]) {
  assert(`descriptors include ${primitive}`, primitives.has(primitive), { primitives: [...primitives].sort() });
}

assert("page remains an acceptance fixture for the public command",
  page.includes(".showConditionMonitoring(")
    && !page.includes(".showProcessCurves(")
    && !/new\s+MetricVisualRuntime|createLayerFromDescriptor|createProcessCurveMiniatureSceneBundle/.test(page),
  { page: "visual/examples/condition-monitoring-hero/index.html" });

assert("dominant field uses exported anomaly property",
  field?.metadata?.viewClass === "PropertyFieldView"
    && field.metadata?.propertyId === "metric-anomaly-severity"
    && field.metadata?.propertySemantic === "anomaly"
    && field.metadata?.nativeEvidence?.source === "exported-record-property"
    && field.metadata?.nativeEvidence?.propertyId === "metric-anomaly-severity"
    && field.metadata?.algorithmicComputation === false,
  summarizeDescriptor(field));

assert("density support uses exported density property",
  projection?.metadata?.role === "density-support-projection"
    && projection.metadata?.propertyId === "local-density"
    && projection.metadata?.propertySemantic === "density"
    && projection.source?.propertyId === "local-density",
  summarizeDescriptor(projection));

assert("trajectory is backed by exported graph or transition evidence",
  trajectory?.metadata?.viewClass === "TrajectoryPathView"
    && trajectory.metadata?.nativeEvidence?.evidenceClaim === true
    && trajectory.metadata?.nativeEvidence?.fallback !== true
    && (
      trajectory.metadata?.nativeEvidence?.graphId === "process-window-trajectory"
      || trajectory.metadata?.nativeEvidence?.relationId === "condition-monitoring-transition"
    )
    && trajectory.metadata?.pathVisualEncoding?.source === "exported-record-property"
    && trajectory.metadata?.pathVisualEncoding?.colorPropertyId === "metric-anomaly-severity",
  summarizeDescriptor(trajectory));

assert("record points remain available for hover and preview resolution",
  points?.metadata?.recordCount === document_.records.length
    && points.picking?.mode === "record-id",
  summarizeDescriptor(points));

assert("semantic hierarchy keeps fields below trajectory, current state and labels",
  isHierarchy(field, "support-field")
    && isHierarchy(projection, "ground-projection")
    && isHierarchy(trajectory, "trajectory-path")
    && isHierarchy(points, "current-state")
    && field.metadata?.visualHierarchy?.drawsBelow?.includes("trajectory-path")
    && projection.metadata?.visualHierarchy?.drawsBelow?.includes("trajectory-path")
    && trajectory.metadata?.visualHierarchy?.drawsAbove?.includes("support-field")
    && trajectory.metadata?.visualHierarchy?.drawsBelow?.includes("current-state")
    && points.metadata?.visualHierarchy?.drawsAbove?.includes("trajectory-path"),
  {
    field: summarizeDescriptor(field),
    projection: summarizeDescriptor(projection),
    trajectory: summarizeDescriptor(trajectory),
    points: summarizeDescriptor(points),
  });

assert("regime labels use scene-structure anchors",
  labels?.metadata?.labelAnchorMode === "regime-structure-boundary"
    && labels.metadata?.propertyId === "truth-regime"
    && labels.metadata?.labelCount >= 4
    && labels.labels?.every((label) => label.anchor?.mode === "regime-structure-boundary"),
  summarizeDescriptor(labels));

assert("regime labels declare final scene-readable hierarchy",
  isHierarchy(labels, "scene-labels")
    && labels.metadata?.visualHierarchy?.drawsAbove?.includes("current-state")
    && labels.metadata?.visualHierarchy?.drawsAbove?.includes("trajectory-path"),
  summarizeDescriptor(labels));

const pageScreenshot = await screenshotStatus(PAGE_SCREENSHOT);
const canvasScreenshot = await screenshotStatus(CANVAS_SCREENSHOT);
assert("page screenshot is absent or non-empty when generated",
  pageScreenshot.ok || pageScreenshot.missing === true,
  pageScreenshot);
assert("canvas screenshot exists", canvasScreenshot.ok, canvasScreenshot);

const failures = checks.filter((check) => !check.ok);
const summary = {
  ok: failures.length === 0,
  evidence: "docs/examples/assets/condition-monitoring/metric.visual.json",
  page: "visual/examples/condition-monitoring-hero/index.html",
  screenshot: "output/visual/check-visual-regression-public-examples/condition-monitoring-hero.png",
  canvasScreenshot: "output/visual/check-visual-regression-public-examples/condition-monitoring-hero.canvas.png",
  pageScreenshotAvailable: pageScreenshot.ok,
  canvasScreenshotAvailable: canvasScreenshot.ok,
  selectedViewKind: surface.lastCommandDiagnostics?.selectedViewKind,
  primitives: [...primitives].sort(),
  field: summarizeDescriptor(field),
  projection: summarizeDescriptor(projection),
  trajectory: summarizeDescriptor(trajectory),
  labels: summarizeDescriptor(labels),
  failures,
};

console.log(JSON.stringify(summary, null, 2));
if (failures.length) process.exitCode = 1;

function assert(message, ok, details = {}) {
  checks.push({ ok: Boolean(ok), message, details: ok ? undefined : details });
}

async function screenshotStatus(path) {
  try {
    const result = await stat(path);
    return { path, ok: result.isFile() && result.size > 0, size: result.size };
  } catch (error) {
    return {
      path,
      ok: false,
      missing: error?.code === "ENOENT",
      error: error instanceof Error ? error.message : String(error),
    };
  }
}

function summarizeDescriptor(descriptor) {
  if (!descriptor) return null;
  return {
    id: descriptor.id,
    primitive: descriptor.primitive || descriptor.kind,
    role: descriptor.metadata?.role || null,
    viewClass: descriptor.metadata?.viewClass || null,
    propertyId: descriptor.metadata?.propertyId || descriptor.source?.propertyId || null,
    propertySemantic: descriptor.metadata?.propertySemantic || null,
    recordCount: descriptor.metadata?.recordCount || descriptor.channels?.recordId?.count || null,
    pathCount: descriptor.metadata?.pathCount || null,
    graphId: descriptor.metadata?.nativeEvidence?.graphId || descriptor.metadata?.graphId || null,
    relationId: descriptor.metadata?.nativeEvidence?.relationId || descriptor.metadata?.relationId || null,
    fallback: descriptor.metadata?.nativeEvidence?.fallback ?? descriptor.metadata?.fallback ?? null,
    pathVisualEncoding: descriptor.metadata?.pathVisualEncoding || null,
    labelAnchorMode: descriptor.metadata?.labelAnchorMode || null,
    visualHierarchy: descriptor.metadata?.visualHierarchy || null,
  };
}

function isHierarchy(descriptor, band) {
  const hierarchy = descriptor?.metadata?.visualHierarchy;
  return hierarchy?.schema === HIERARCHY_SCHEMA
    && hierarchy.band === band
    && hierarchy.algorithmicComputation === false;
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

function FakeRuntime() {
  this.layers = [];
  this.descriptors = [];

  this.setLayerDescriptors = (descriptors) => {
    this.descriptors = descriptors.slice();
    this.layers = this.descriptors.map((descriptor) => ({ id: descriptor.id, descriptor }));
  };

  this.renderOnce = () => {};

  this.start = () => {
    this.started = true;
  };

  this.setCameraOptions = (options) => {
    this.cameraOptions = { ...options };
  };

  this.getState = () => {
    return {
      layerInstanceCount: this.layers.length,
      layerState: { count: this.layers.length },
    };
  };

  this.on = () => () => {};

  this.selectPair = () => {};
}
