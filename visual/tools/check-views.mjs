#!/usr/bin/env node
/*
 * Reusable view smoke test.
 *
 * Constructs each semantic view from metric.visual.v1 fixtures
 * and asserts the layer descriptors they emit are renderable: every primitive
 * is registered in the layer factory, point views carry one instance per
 * record, relation views carry texture/edge evidence, and morph/dynamics views
 * declare the right animation/segments. Runs headless (no GPU).
 *
 * Run: node visual/tools/check-views.mjs
 */

import { readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import {
  MetricSpaceView,
  RelationMatrixView,
  NeighborhoodGraphView,
  SpacePropertiesView,
  MappingView,
  DynamicsView,
  SolverTraceView,
  TrajectoryPathView,
} from "../src/views/index.js";
import { defaultLayerRegistry } from "../src/layers/index.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const FIXTURE_DIR = resolve(HERE, "..", "examples", "fixtures");
const REGISTRY = defaultLayerRegistry();

async function loadFixture(name) {
  return JSON.parse(await readFile(resolve(FIXTURE_DIR, name), "utf8"));
}

function instanceCount(descriptor) {
  const channel = descriptor?.channels?.position || descriptor?.channels?.sourcePosition;
  if (descriptor?.geometry?.instanceCount) return descriptor.geometry.instanceCount;
  if (!channel) return 0;
  const size = channel.itemSize || channel.size || 3;
  const array = channel.array || channel.data;
  return array ? Math.floor(array.length / size) : (channel.count || 0);
}

function checkRenderable(checks, label, descriptors) {
  assert(checks, `${label}: returns at least one descriptor`, Array.isArray(descriptors) && descriptors.length > 0, {
    count: Array.isArray(descriptors) ? descriptors.length : null,
  });
  for (const descriptor of descriptors || []) {
    const primitive = descriptor.primitive || descriptor.kind;
    assert(checks, `${label}: primitive "${primitive}" is registered`, REGISTRY.has(primitive), {
      id: descriptor.id,
      primitive,
      registered: Array.from(REGISTRY.keys()),
    });
    assert(checks, `${label}: descriptor "${descriptor.id}" has an id`, Boolean(descriptor.id), { descriptor: descriptor.id });
  }
}

function assert(checks, message, ok, details = {}) {
  checks.push({ ok: Boolean(ok), message, details: ok ? undefined : details });
}

async function main() {
  const metricSpace = await loadFixture("metric-space.visual.json");
  const matrixFixture = await loadFixture("relation-matrix.visual.json");
  const checks = [];

  // 1. MetricSpaceView (with 2D->3D morph)
  {
    const view = MetricSpaceView.fromVisualSpace(metricSpace, {
      spaceId: "sensor-space",
      coordinateId: "landmark-3d",
      targetCoordinateId: "landmark-2d",
      scalarProperty: "anomaly",
    });
    const descriptors = view.toLayerDescriptors();
    checkRenderable(checks, "MetricSpaceView", descriptors);
    const points = descriptors.find((d) => (d.primitive || d.kind) === "InstancedPointLayer");
    assert(checks, "MetricSpaceView: point layer present", Boolean(points));
    assert(checks, "MetricSpaceView: one instance per record (24)", instanceCount(points) === 24, { count: instanceCount(points) });
    assert(checks, "MetricSpaceView: morph animation declared", points?.animation?.mode === "coordinate-morph", { animation: points?.animation?.mode });
    assert(checks, "MetricSpaceView: emits a ground stage", descriptors.some((d) => (d.primitive || d.kind) === "GroundPlaneLayer"));
    assert(checks, "MetricSpaceView: emits a ground projection", descriptors.some((d) => (d.primitive || d.kind) === "GroundProjectionLayer"));
  }

  // 2. RelationMatrixView (sparse kNN) + dense fixture
  {
    const view = RelationMatrixView.fromVisualSpace(metricSpace, { relationId: "sensor-metric" });
    const [descriptor] = view.toLayerDescriptors();
    checkRenderable(checks, "RelationMatrixView", [descriptor]);
    assert(checks, "RelationMatrixView: produces matrix texture data", descriptor?.source?.texture?.kind === "relation-matrix-texture-data", {
      kind: descriptor?.source?.texture?.kind,
    });
    assert(checks, "RelationMatrixView: matrix is 24x24", descriptor?.source?.texture?.width === 24, { width: descriptor?.source?.texture?.width });

    const dense = RelationMatrixView.fromVisualSpace(matrixFixture, { relationId: "catalog-metric" });
    const [denseDescriptor] = dense.toLayerDescriptors();
    assert(checks, "RelationMatrixView: dense fixture mirrors symmetric pairs",
      denseDescriptor?.metadata?.matrix?.acceptedPairCount >= 60,
      { accepted: denseDescriptor?.metadata?.matrix?.acceptedPairCount });
  }

  // 3. NeighborhoodGraphView
  {
    const view = NeighborhoodGraphView.fromVisualSpace(metricSpace, {
      graphId: "sensor-knn",
      coordinateId: "landmark-3d",
      topK: 4,
    });
    const descriptors = view.toLayerDescriptors();
    checkRenderable(checks, "NeighborhoodGraphView", descriptors);
    const edges = descriptors.find((d) => (d.primitive || d.kind) === "RelationEdgeLayer");
    assert(checks, "NeighborhoodGraphView: emits edges", Boolean(edges));
    assert(checks, "NeighborhoodGraphView: edge count > 0", (edges?.metadata?.graph?.edgeCount || 0) > 0, {
      edgeCount: edges?.metadata?.graph?.edgeCount,
    });
    assert(checks, "NeighborhoodGraphView: emits nodes", descriptors.some((d) => (d.primitive || d.kind) === "InstancedPointLayer"));
  }

  // 4. SpacePropertiesView
  {
    const view = SpacePropertiesView.fromVisualSpace(metricSpace, { propertyId: "entropy", coordinateId: "landmark-3d" });
    const descriptors = view.toLayerDescriptors();
    checkRenderable(checks, "SpacePropertiesView", descriptors);
    const summary = view.summary();
    assert(checks, "SpacePropertiesView: summary covers all records", summary.count === 24, { count: summary.count });
    assert(checks, "SpacePropertiesView: histogram has bins", summary.histogram.length === 12, { bins: summary.histogram.length });
    assert(checks, "SpacePropertiesView: ranks records", summary.top.length > 0 && summary.bottom.length > 0);
  }

  // 5. MappingView
  {
    const view = MappingView.fromVisualSpace(metricSpace, {
      sourceCoordinateId: "landmark-2d",
      targetCoordinateId: "landmark-3d",
      residualPropertyId: "map-residual",
    });
    const descriptors = view.toLayerDescriptors();
    checkRenderable(checks, "MappingView", descriptors);
    const points = descriptors.find((d) => (d.primitive || d.kind) === "InstancedPointLayer");
    assert(checks, "MappingView: morph between coordinate states", points?.animation?.mode === "coordinate-morph", { animation: points?.animation?.mode });
    const preservation = view.preservationSummary();
    assert(checks, "MappingView: preservation summary present", preservation.count === 24, { count: preservation.count });
  }

  // 6. DynamicsView
  {
    const view = DynamicsView.fromVisualSpace(metricSpace, { timelineId: "condition-morph" });
    const descriptors = view.toLayerDescriptors();
    checkRenderable(checks, "DynamicsView", descriptors);
    assert(checks, "DynamicsView: emits trajectory/path segments", descriptors.some((d) => d.metadata?.role === "trajectory/path" && d.metadata?.viewClass === "TrajectoryPathView"));
    assert(checks, "DynamicsView: emits active state points", descriptors.some((d) => (d.primitive || d.kind) === "InstancedPointLayer"));
  }

  // 7. TrajectoryPathView (record-order path grammar)
  {
    const space = MetricSpaceView.fromVisualSpace(metricSpace, {
      spaceId: "sensor-space",
      coordinateId: "landmark-3d",
    });
    const view = TrajectoryPathView.fromMetricSpaceView(space, {
      id: "sensor-record-order:trajectory",
      pathCount: 4,
      width: 2.8,
    });
    const descriptors = view.toLayerDescriptors();
    checkRenderable(checks, "TrajectoryPathView", descriptors);
    const [trajectory] = descriptors;
    assert(checks, "TrajectoryPathView: emits reusable curve grammar", trajectory?.primitive === "CurveRibbonLayer", { primitive: trajectory?.primitive });
    assert(checks, "TrajectoryPathView: reports trajectory/path role", trajectory?.metadata?.role === "trajectory/path", trajectory?.metadata);
    assert(checks, "TrajectoryPathView: reports record and path counts", trajectory?.metadata?.recordCount === 24 && trajectory?.metadata?.pathCount === 4, trajectory?.metadata);
    assert(checks, "TrajectoryPathView: declares no JS algorithmic computation", trajectory?.metadata?.algorithmicComputation === false, trajectory?.metadata);
  }

  // 8. SolverTraceView (explicit converging residual series)
  {
    const series = Array.from({ length: 20 }, (_, index) => 1.0 * Math.pow(0.7, index));
    const view = SolverTraceView.fromVisualSpace(metricSpace, { series, logScale: true, traceLabel: "PCG residual" });
    const descriptors = view.toLayerDescriptors();
    checkRenderable(checks, "SolverTraceView", descriptors);
    const [trace] = descriptors;
    assert(checks, "SolverTraceView: emits reusable curve grammar", trace?.primitive === "CurveRibbonLayer", { primitive: trace?.primitive });
    assert(checks, "SolverTraceView: timeline curve has n-1 segments", trace?.metadata?.segmentCount === series.length - 1, { segments: trace?.metadata?.segmentCount });
    assert(checks, "SolverTraceView: trace evidence descriptor present", trace?.metadata?.traceEvidence?.schema === "metric.visual.solver_trace_evidence.v1", { evidence: trace?.metadata?.traceEvidence });
    const summary = view.summary();
    assert(checks, "SolverTraceView: reports convergence", summary.converged === true, { final: summary.final, reduction: summary.reduction });
  }

  const failures = checks.filter((check) => !check.ok);
  const ok = failures.length === 0;
  console.log(JSON.stringify({
    ok,
    total: checks.length,
    failed: failures.length,
    failures,
  }, null, 2));
  if (!ok) process.exitCode = 1;
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
