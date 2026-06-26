#!/usr/bin/env node

import { readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import { MetricVisualSurface } from "../src/metric-visual.js";
import { TrajectoryPathView } from "../src/views/index.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");

async function readJson(path) {
  return JSON.parse(await readFile(resolve(ROOT, path), "utf8"));
}

async function main() {
  const condition = await readJson("docs/examples/assets/condition-monitoring/metric.visual.json");
  const dynamics = await readJson("docs/examples/assets/dynamics-noise/metric.visual.json");
  const process = await readJson("docs/examples/assets/process-curve-external/metric.visual.json");
  const checks = [];

  {
    const view = TrajectoryPathView.fromVisualSpace(condition, {
      id: "gate:condition-graph-path",
      coordinateId: "process-state-trajectory-3d",
      graphId: "process-window-trajectory",
      width: 3,
    });
    const [descriptor] = view.toLayerDescriptors();
    assert(checks, "graph transition creates a CurveRibbonLayer", descriptor?.primitive === "CurveRibbonLayer", descriptor);
    assert(checks, "graph transition comes from TrajectoryPathView", descriptor?.metadata?.viewClass === "TrajectoryPathView", descriptor?.metadata);
    assert(checks, "graph transition preserves native graph evidence", descriptor?.metadata?.nativeEvidence?.graphId === "process-window-trajectory", descriptor?.metadata?.nativeEvidence);
    assert(checks, "graph transition reports record/path counts", descriptor?.metadata?.recordCount === 528 && descriptor?.metadata?.pathCount === 1, descriptor?.metadata);
    assert(checks, "graph transition does not claim JS algorithmic computation", descriptor?.metadata?.algorithmicComputation === false, descriptor?.metadata);
  }

  {
    const view = TrajectoryPathView.fromVisualSpace(condition, {
      id: "gate:explicit-record-path",
      coordinateId: "process-state-trajectory-3d",
      pathRecordIds: [["window-0000", "window-0001", "window-0002"]],
      width: 3,
    });
    const [descriptor] = view.toLayerDescriptors();
    assert(checks, "explicit ids create a CurveRibbonLayer", descriptor?.primitive === "CurveRibbonLayer", descriptor);
    assert(checks, "explicit ids report record/path counts", descriptor?.metadata?.recordCount === 3 && descriptor?.metadata?.pathCount === 1, descriptor?.metadata);
    assert(checks, "explicit ids keep trajectory/path role", descriptor?.metadata?.role === "trajectory/path", descriptor?.metadata);
  }

  {
    const surface = createHeadlessSurface(condition);
    surface.showConditionMonitoring({
      coordinateId: "process-state-trajectory-3d",
      colorBy: "metric-anomaly-severity",
      groundField: "local-density",
      labels: "truth-regime",
      preview: false,
    });
    const descriptor = trajectoryDescriptor(surface.descriptors, "CurveRibbonLayer");
    assert(checks, "showConditionMonitoring uses TrajectoryPathView", descriptor?.metadata?.viewClass === "TrajectoryPathView", descriptor?.metadata);
    assert(checks, "showConditionMonitoring keeps trajectory/path metadata", descriptor?.metadata?.role === "trajectory/path" && descriptor?.metadata?.nativeEvidence?.graphId === "process-window-trajectory", descriptor?.metadata);
  }

  {
    const surface = createHeadlessSurface(dynamics);
    surface.showDynamics({
      timelineId: "reverse-reconstruction",
      propertyField: "best-reconstruction-error",
      preview: false,
    });
    const descriptor = trajectoryDescriptor(surface.descriptors, "CurveRibbonLayer");
    assert(checks, "showDynamics uses TrajectoryPathView", descriptor?.metadata?.viewClass === "TrajectoryPathView", descriptor?.metadata);
    assert(checks, "showDynamics emits one path per record through the engine pipeline", descriptor?.metadata?.pathCount === 512 && descriptor?.metadata?.recordCount === 512, descriptor?.metadata);
    assert(checks, "showDynamics preserves timeline native evidence", descriptor?.metadata?.nativeEvidence?.timelineId === "reverse-reconstruction", descriptor?.metadata?.nativeEvidence);
  }

  {
    const surface = createHeadlessSurface(process);
    surface.showProcessCurves({
      coordinateId: "process-curve-external-landmark-3d",
      relationId: "process-curve-external-aligned-metric",
      graphId: "process-curve-external-knn",
      labelPropertyId: "process-role",
      includeMatrix: true,
      preview: false,
    });
    const descriptor = trajectoryDescriptor(surface.descriptors, "CurveTubeMeshLayer");
    assert(checks, "showProcessCurves uses TrajectoryPathView tube grammar", descriptor?.metadata?.viewClass === "TrajectoryPathView", descriptor?.metadata);
    assert(checks, "showProcessCurves keeps record-track engine view kind", descriptor?.source?.viewKind === "record-track", descriptor?.source);
    assert(checks, "showProcessCurves reports native record/path counts", descriptor?.metadata?.recordCount === 48 && descriptor?.metadata?.pathCount > 0, descriptor?.metadata);
  }

  report(checks);
}

function trajectoryDescriptor(descriptors, primitive) {
  return descriptors.find((descriptor) => (
    descriptor?.primitive === primitive &&
    descriptor?.metadata?.role === "trajectory/path" &&
    descriptor?.metadata?.viewClass === "TrajectoryPathView"
  ));
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
}

function assert(checks, message, ok, details = {}) {
  checks.push({ ok: Boolean(ok), message, details: ok ? undefined : details });
}

function report(checks) {
  const failures = checks
    .filter((check) => !check.ok)
    .map(({ message, details }) => ({ message, details }));
  console.log(JSON.stringify({
    ok: failures.length === 0,
    total: checks.length,
    failed: failures.length,
    failures,
  }, null, 2));
  if (failures.length) process.exitCode = 1;
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
