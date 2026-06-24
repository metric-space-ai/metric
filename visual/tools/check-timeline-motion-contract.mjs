#!/usr/bin/env node

import { readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import {
  TimelineModel,
  createTimelineAnimationDescriptor,
} from "../src/timeline/index.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");

async function main() {
  const document = JSON.parse(await readFile(
    resolve(ROOT, "docs/examples/assets/dynamics-noise/metric.visual.json"),
    "utf8",
  ));
  const model = new TimelineModel(document, {
    timelineId: "reverse-reconstruction",
    loop: true,
  });
  const descriptor = createTimelineAnimationDescriptor(model, {
    loop: true,
    stepDurationMs: 360,
    minDurationMs: 6800,
    maxDurationMs: 18000,
  });
  const interpolation = model.seek(0.125).currentInterpolation();
  const checks = [
    ["timeline model resolves id", model.id === "reverse-reconstruction", model.id],
    ["timeline normalizes exported steps", model.steps.length === 41, model.steps.length],
    ["timeline interpolation keeps coordinate refs", interpolation.coordinate.fromId === "coord-reverse-05", interpolation.coordinate],
    ["timeline animation has schema", descriptor.schema === "metric.visual.timeline_animation.v1", descriptor.schema],
    ["timeline animation is render-loop driven", descriptor.clock === "render-loop", descriptor.clock],
    ["timeline animation carries keyframes", descriptor.keyframes.length === 41, descriptor.keyframes.length],
    ["timeline animation duration is restrained", descriptor.durationMs >= 6800 && descriptor.durationMs <= 18000, descriptor.durationMs],
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
