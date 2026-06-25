#!/usr/bin/env node

import { readFile } from "node:fs/promises";
import { resolve } from "node:path";

import { MetricVisualSurface } from "../src/index.js";
import { ProcessCurveSceneView } from "../src/views/index.js";

const ROOT = resolve(new URL("../..", import.meta.url).pathname);
const DOCUMENT_PATH = resolve(ROOT, "docs/examples/assets/process-curve-external/metric.visual.json");
const METRIC_VISUAL_PATH = resolve(ROOT, "visual/src/metric-visual.js");

const checks = [];

const document_ = JSON.parse(await readFile(DOCUMENT_PATH, "utf8"));
const baseOptions = {
  coordinateId: "process-curve-external-landmark-3d",
  relationId: "process-curve-external-aligned-metric",
  graphId: "process-curve-external-knn",
  labelPropertyId: "process-role",
  trackMode: "tube",
};

const view = ProcessCurveSceneView.fromVisualSpace(document_, {
  ...baseOptions,
  includeMatrix: true,
});
assert("ProcessCurveSceneView.fromVisualSpace produces process-curves view", view.kind === "process-curves", {
  kind: view.kind,
});
assert("ProcessCurveSceneView records resolved semantic metadata",
  view.metadata?.viewClass === "ProcessCurveSceneView" &&
  view.metadata?.visualGrammar === "process-curves" &&
  view.metadata?.algorithmicComputation === false &&
  view.metadata?.datasetId === "process-curve-external" &&
  view.metadata?.coordinateId === "process-curve-external-landmark-3d" &&
  view.metadata?.propertyId === "process-role" &&
  view.metadata?.relationId === "process-curve-external-aligned-metric",
  view.metadata);

const descriptors = view.toLayerDescriptors();
const primitives = descriptorPrimitives(descriptors);
for (const primitive of [
  "HeatFieldLayer",
  "GroundProjectionLayer",
  "InstancedBoxLayer",
  "InstancedPointLayer",
]) {
  assert(`ProcessCurveSceneView emits ${primitive}`, primitives.has(primitive), { primitives: Array.from(primitives) });
}
assert("ProcessCurveSceneView emits curve track primitive",
  primitives.has("CurveRibbonLayer") || primitives.has("CurveTubeMeshLayer"),
  { primitives: Array.from(primitives) });

const disabled = ProcessCurveSceneView.fromVisualSpace(document_, {
  ...baseOptions,
  includeNeighborhood: false,
  includeMatrix: false,
}).toLayerDescriptors();
const disabledPrimitives = descriptorPrimitives(disabled);
assert("optional graph support can be disabled", !disabledPrimitives.has("RelationEdgeLayer"), {
  primitives: Array.from(disabledPrimitives),
});
assert("optional matrix support can be disabled", !disabledPrimitives.has("RelationMatrixLayer"), {
  primitives: Array.from(disabledPrimitives),
});

const enabled = ProcessCurveSceneView.fromVisualSpace(document_, {
  ...baseOptions,
  includeNeighborhood: true,
  includeMatrix: true,
}).toLayerDescriptors();
const enabledPrimitives = descriptorPrimitives(enabled);
assert("optional graph support can be enabled", enabledPrimitives.has("RelationEdgeLayer"), {
  primitives: Array.from(enabledPrimitives),
});
assert("optional matrix support can be enabled", enabledPrimitives.has("RelationMatrixLayer"), {
  primitives: Array.from(enabledPrimitives),
});

const surface = createHeadlessSurface(document_);
surface.showProcessCurves({
  ...baseOptions,
  includeMatrix: true,
  preview: false,
});
assert("MetricVisualSurface.showProcessCurves records process-curves view",
  surface.views?.[0]?.kind === "process-curves",
  { views: surface.views?.map((entry) => entry.kind) });

const metricVisualSource = await readFile(METRIC_VISUAL_PATH, "utf8");
assert("metric-visual.js no longer imports or calls createProcessCurveMiniatureLayerDescriptors",
  !/\bcreateProcessCurveMiniatureLayerDescriptors\b/.test(metricVisualSource),
  {});

const failures = checks.filter((check) => !check.ok);
const summary = {
  ok: failures.length === 0,
  viewKind: view.kind,
  descriptorCount: descriptors.length,
  primitives: Array.from(primitives).sort(),
  enabledPrimitives: Array.from(enabledPrimitives).sort(),
  disabledPrimitives: Array.from(disabledPrimitives).sort(),
  failures,
};
console.log(JSON.stringify(summary, null, 2));
if (failures.length) process.exitCode = 1;

function descriptorPrimitives(items) {
  return new Set((items || []).map((descriptor) => descriptor.primitive || descriptor.kind).filter(Boolean));
}

function assert(message, ok, details = {}) {
  checks.push({ ok: Boolean(ok), message, details: ok ? undefined : details });
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
