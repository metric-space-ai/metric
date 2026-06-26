#!/usr/bin/env node

import assert from "node:assert/strict";
import { readFileSync } from "node:fs";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import { VisualScene } from "../src/engine/index.js";
import { MetricVisualSurface } from "../src/metric-visual.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");

const condition = readJson("docs/examples/assets/condition-monitoring/metric.visual.json");
const dynamics = readJson("docs/examples/assets/dynamics-noise/metric.visual.json");

const conditionDescriptors = descriptorsFor(condition, (surface) => surface.showConditionMonitoring({
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
}));

const dynamicsDescriptors = descriptorsFor(dynamics, (surface) => surface.showDynamics({
  timelineId: "reverse-reconstruction",
  field: true,
  preview: "state-history",
  loop: true,
  timelineProgress: 0.5,
  timelineFieldPropertyId: "reverse-mse-to-clean",
  fieldPropertyId: "best-reconstruction-error",
  fieldMode: "lifted",
  fieldAlpha: 0.42,
  fieldRadius: 0.2,
  fieldLift: 0.58,
  fieldMaterial: { contour: 0.22, glow: 0.16 },
  pathColorPropertyId: "reconstruction-improvement",
  pathWidthPropertyId: "best-reconstruction-error",
  trajectoryPathLimit: 192,
  trajectoryPathSelection: "deterministic-record-stride",
  timelineContext: true,
  timelineContextStops: [0, 0.5, 1],
  timelineContextAlpha: 0.24,
  timelineContextPointSize: 1.18,
  pathWidth: 5.2,
  pathAlpha: 0.82,
  pointSize: 2.08,
  timelineControl: { root: {} },
}));

const conditionOrder = sortedIds(conditionDescriptors);
const dynamicsOrder = sortedIds(dynamicsDescriptors);

const conditionField = find(conditionDescriptors, (descriptor) => descriptor.primitive === "HeatFieldLayer");
const conditionProjection = find(conditionDescriptors, (descriptor) => descriptor.primitive === "GroundProjectionLayer");
const conditionPath = find(conditionDescriptors, (descriptor) => descriptor.metadata?.viewClass === "TrajectoryPathView");
const conditionCurrent = find(conditionDescriptors, (descriptor) => descriptor.metadata?.role === "current-state-records");
const conditionLabels = find(conditionDescriptors, (descriptor) => descriptor.primitive === "BillboardLabelLayer");

assertBefore(conditionOrder, conditionField, conditionPath, "condition field renders before trajectory");
assertBefore(conditionOrder, conditionProjection, conditionPath, "condition projection renders before trajectory");
assertBefore(conditionOrder, conditionPath, conditionCurrent, "condition trajectory renders before current records");
assertBefore(conditionOrder, conditionCurrent, conditionLabels, "condition labels render after current records");
assert.equal(conditionPath.material?.depthTest, false, "condition ribbon trajectory bypasses point-cloud depth rejection");
assert.equal(conditionPath.metadata?.nativeEvidence?.fallback, false, "condition trajectory remains native evidence");
assert.equal(conditionPath.metadata?.visualPriority?.semanticOverlay, true, "condition trajectory exposes semantic overlay priority");

const dynamicsField = find(dynamicsDescriptors, (descriptor) => descriptor.primitive === "HeatFieldLayer");
const dynamicsPath = find(dynamicsDescriptors, (descriptor) => descriptor.metadata?.evidenceRole === "trajectory/path");
const dynamicsCurrent = find(dynamicsDescriptors, (descriptor) => descriptor.metadata?.role === "current-timeline-state");
const dynamicsContext = dynamicsDescriptors.filter((descriptor) => descriptor.metadata?.role === "timeline-state-history-context");

assertBefore(dynamicsOrder, dynamicsField, dynamicsPath, "dynamics field renders before trajectory");
for (const context of dynamicsContext) {
  assertBefore(dynamicsOrder, context, dynamicsPath, "dynamics context state renders before trajectory");
}
assertBefore(dynamicsOrder, dynamicsPath, dynamicsCurrent, "dynamics trajectory renders before current state");
assert.equal(dynamicsPath.material?.depthTest, false, "dynamics ribbon trajectory bypasses point-cloud depth rejection");
assert.equal(dynamicsPath.metadata?.nativeEvidence?.fallback, false, "dynamics trajectory remains native evidence");
assert.equal(dynamicsPath.metadata?.visualPriority?.semanticOverlay, true, "dynamics trajectory exposes semantic overlay priority");

console.log(JSON.stringify({
  ok: true,
  condition: summarize(conditionDescriptors, conditionOrder),
  dynamics: summarize(dynamicsDescriptors, dynamicsOrder),
}, null, 2));

function readJson(path) {
  return JSON.parse(readFileSync(resolve(ROOT, path), "utf8"));
}

function descriptorsFor(document, command) {
  const runtime = createHeadlessRuntime();
  const surface = new MetricVisualSurface({
    document,
    canvas: null,
    runtime,
    setup: {},
    options: { context: "trajectory-field-hierarchy-check" },
  });
  command(surface);
  return surface.descriptors || [];
}

function sortedIds(descriptors) {
  const scene = new VisualScene();
  for (const descriptor of descriptors) {
    scene.add({
      id: descriptor.id,
      primitive: descriptor.primitive,
      kind: descriptor.kind,
      metadata: descriptor.metadata || {},
      material: descriptor.material || {},
      animation: descriptor.animation || {},
      descriptor,
      renderOrder: Number.isFinite(Number(descriptor.order)) ? Number(descriptor.order) : 0,
      render() {},
    });
  }
  scene.sort();
  return scene.children.map((child) => child.id);
}

function find(descriptors, predicate) {
  const descriptor = descriptors.find(predicate);
  assert(descriptor, "expected descriptor missing");
  return descriptor;
}

function assertBefore(order, earlier, later, message) {
  const left = order.indexOf(earlier.id);
  const right = order.indexOf(later.id);
  assert(left >= 0, `${message}: missing earlier descriptor ${earlier.id}`);
  assert(right >= 0, `${message}: missing later descriptor ${later.id}`);
  assert(left < right, `${message}: ${earlier.id} should render before ${later.id}`);
}

function summarize(descriptors, order) {
  return order.map((id) => {
    const descriptor = descriptors.find((entry) => entry.id === id);
    return {
      id,
      primitive: descriptor?.primitive || descriptor?.kind,
      role: descriptor?.metadata?.role || descriptor?.metadata?.evidenceRole || null,
      order: descriptor?.order ?? 0,
      depthTest: descriptor?.material?.depthTest ?? null,
    };
  });
}

function createHeadlessRuntime() {
  return {
    layerDescriptors: [],
    setLayerDescriptors(descriptors) {
      this.layerDescriptors = descriptors.slice();
    },
    setCameraOptions() {
      return this;
    },
    setMotion() {
      return this;
    },
    setPostprocessOptions() {
      return this;
    },
    setStage() {
      return this;
    },
    setFocusTarget() {
      return this;
    },
    clearFocusTarget() {
      return this;
    },
    start() {
      return this;
    },
    stop() {
      return this;
    },
    renderOnce() {},
    getState() {
      return {
        layerInstanceCount: this.layerDescriptors.length,
        layerState: { status: "headless" },
        inspection: {},
      };
    },
    on() {
      return () => {};
    },
  };
}
