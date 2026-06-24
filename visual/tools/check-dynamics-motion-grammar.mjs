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
  const checks = [
    ["dynamics consumes reverse timeline", view.timelineId === "reverse-reconstruction", view.timelineId],
    ["dynamics consumes all exported timeline states", view.fittedStates.length === 41, view.fittedStates.length],
    ["dynamics has one record per exported state", view.recordIds.length === 28, view.recordIds.length],
    ["dynamics path layer uses reusable curve grammar", trajectory?.primitive === "CurveRibbonLayer", trajectory?.primitive],
    ["dynamics path count matches records", trajectory?.metadata?.pathCount === 28, trajectory?.metadata],
    ["dynamics path metadata preserves coordinate ids", trajectory?.metadata?.coordinateIds?.length === 41, trajectory?.metadata?.coordinateIds?.length],
    ["dynamics point layer carries targetPosition", point?.channels?.targetPosition?.count === 28, point?.channels?.targetPosition?.count],
    ["dynamics declares timeline animation schema", animation.schema === "metric.visual.timeline_animation.v1", animation],
    ["dynamics animation is render-loop driven", animation.clock === "render-loop", animation],
    ["dynamics animation is restrained", animation.durationMs >= 6800 && animation.durationMs <= 18000, animation.durationMs],
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
