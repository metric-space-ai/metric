#!/usr/bin/env node

import { readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import { DynamicsView } from "../src/views/DynamicsView.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");

async function main() {
  const document = JSON.parse(await readFile(
    resolve(ROOT, "docs/examples/assets/dynamics-noise/metric.visual.json"),
    "utf8",
  ));
  const view = DynamicsView.fromVisualSpace(document, {
    timelineId: "reverse-reconstruction",
    propertyField: "best-reconstruction-error",
    pointSize: 1.7,
  });
  const descriptors = view.toLayerDescriptors();
  const trajectory = descriptors.find((descriptor) => descriptor.metadata?.evidenceRole === "trajectory/path");
  const point = descriptors.find((descriptor) => descriptor.primitive === "InstancedPointLayer");
  const animation = point?.animation || {};
  const timelineSamples = point?.metadata?.timelineSamples || [];
  const sampledStates = [
    ["start", 0, "coord-reverse-00", 0],
    ["middle", 0.5, "coord-reverse-20", 20],
    ["end", 1, "coord-reverse-40", 40],
  ].map(([label, timelineProgress, coordinateId, stepOrder]) => {
    const sampledView = DynamicsView.fromVisualSpace(document, {
      timelineId: "reverse-reconstruction",
      timelineProgress,
    });
    const sampledPoint = sampledView.toLayerDescriptors()
      .find((descriptor) => descriptor.primitive === "InstancedPointLayer");
    return {
      label,
      timelineProgress,
      expectedCoordinateId: coordinateId,
      expectedStepOrder: stepOrder,
      activeStep: sampledView.activeStep,
      activeCoordinateId: sampledPoint?.metadata?.activeCoordinateId,
      sampledCoordinateId: sampledPoint?.metadata?.sampledCoordinateId,
      state: sampledPoint?.metadata?.timelineState,
    };
  });
  const checks = [
    ["dynamics consumes reverse timeline", view.timelineId === "reverse-reconstruction", view.timelineId],
    ["dynamics consumes all exported timeline states", view.fittedStates.length === 41, view.fittedStates.length],
    ["dynamics has one record per exported state", view.recordIds.length === 512, view.recordIds.length],
    ["dynamics path layer uses reusable curve grammar", trajectory?.primitive === "CurveRibbonLayer", trajectory?.primitive],
    ["dynamics path count matches records", trajectory?.metadata?.pathCount === 512, trajectory?.metadata],
    ["dynamics path metadata preserves coordinate ids", trajectory?.metadata?.coordinateIds?.length === 41, trajectory?.metadata?.coordinateIds?.length],
    ["dynamics point layer carries targetPosition", point?.channels?.targetPosition?.count === 512, point?.channels?.targetPosition?.count],
    ["dynamics declares timeline animation schema", animation.schema === "metric.visual.timeline_animation.v1", animation],
    ["dynamics animation is render-loop driven", animation.clock === "render-loop", animation],
    ["dynamics animation is restrained", animation.durationMs >= 6800 && animation.durationMs <= 18000, animation.durationMs],
    ["dynamics point descriptor carries timeline state schema", point?.metadata?.timelineStateSchema === "metric.visual.timeline_state_sample.v1", point?.metadata],
    ["dynamics point descriptor carries timeline control schema", point?.metadata?.timelineControlSchema === "metric.visual.timeline_control.v1", point?.metadata?.timelineControl],
    ["dynamics point descriptor carries timeline evidence schema", point?.metadata?.timelineEvidenceSchema === "metric.visual.timeline_evidence.v1", point?.metadata?.timelineEvidence],
    ["dynamics timeline evidence carries exported coordinate ids", point?.metadata?.timelineEvidence?.coordinateIds?.length === 41, point?.metadata?.timelineEvidence?.coordinateIds],
    ["dynamics trajectory descriptor carries timeline control schema", trajectory?.metadata?.timelineControlSchema === "metric.visual.timeline_control.v1", trajectory?.metadata?.timelineControl],
    ["dynamics trajectory declares state-history motion grammar", trajectory?.metadata?.motionGrammar === "timeline-trajectory-state-history" && trajectory?.metadata?.timelineEvidence?.schema === "metric.visual.timeline_evidence.v1", trajectory?.metadata],
    ["dynamics animation carries stateful timeline control", animation.control?.schema === "metric.visual.timeline_control.v1" && animation.state?.schema === "metric.visual.timeline_state_sample.v1", animation],
    ["dynamics animation carries timeline evidence contract", animation.evidence?.schema === "metric.visual.timeline_evidence.v1" && animation.evidence?.motionContract?.controlledBy === "metric.visual.timeline_control.v1", animation.evidence],
    ["dynamics descriptor samples start/middle/end states", timelineSamples.map((sample) => sample.activeCoordinateId).join(",") === "coord-reverse-00,coord-reverse-20,coord-reverse-40", timelineSamples],
    ["dynamics start/middle/end active states are deterministic", sampledStates.every((sample) => (
      sample.activeStep === sample.expectedStepOrder
      && sample.activeCoordinateId === sample.expectedCoordinateId
      && sample.sampledCoordinateId === sample.expectedCoordinateId
      && sample.state?.selection === "nearest-exported-step"
    )), sampledStates],
  ];
  report(checks);
}

function report(checks) {
  const failures = checks
    .filter(([, ok]) => !ok)
    .map(([message, , details]) => ({ message, details }));
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
