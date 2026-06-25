#!/usr/bin/env node

import { readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import {
  TimelineModel,
  createTimelineAnimationDescriptor,
  createTimelineControlDescriptor,
  sampleTimelineState,
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
  const control = createTimelineControlDescriptor(model, {
    loop: true,
  });
  const interpolation = model.seek(0.125).currentInterpolation();
  const startState = sampleTimelineState(model, { id: "start", normalized: 0 });
  const middleState = sampleTimelineState(model, { id: "middle", normalized: 0.5 });
  const endState = sampleTimelineState(model, { id: "end", normalized: 1 });
  const controlSampleCoordinates = control.samples.map((sample) => sample.activeCoordinateId);
  const checks = [
    ["timeline model resolves id", model.id === "reverse-reconstruction", model.id],
    ["timeline normalizes exported steps", model.steps.length === 41, model.steps.length],
    ["timeline interpolation keeps coordinate refs", interpolation.coordinate.fromId === "coord-reverse-05", interpolation.coordinate],
    ["timeline animation has schema", descriptor.schema === "metric.visual.timeline_animation.v1", descriptor.schema],
    ["timeline animation is render-loop driven", descriptor.clock === "render-loop", descriptor.clock],
    ["timeline animation carries keyframes", descriptor.keyframes.length === 41, descriptor.keyframes.length],
    ["timeline animation duration is restrained", descriptor.durationMs >= 6800 && descriptor.durationMs <= 18000, descriptor.durationMs],
    ["timeline control has schema", control.schema === "metric.visual.timeline_control.v1", control],
    ["timeline control is stateful normalized time", control.stateful === true && control.valueType === "normalized-time", control],
    ["timeline control exposes range metadata", control.range.min === 0 && control.range.max === 1 && control.range.step > 0, control.range],
    ["timeline control samples start/middle/end exported states", controlSampleCoordinates.join(",") === "coord-reverse-00,coord-reverse-20,coord-reverse-40", controlSampleCoordinates],
    ["timeline sampling start is deterministic", startState.activeCoordinateId === "coord-reverse-00" && startState.activeStepOrder === 0, startState],
    ["timeline sampling middle is deterministic", middleState.activeCoordinateId === "coord-reverse-20" && middleState.activeStepOrder === 20, middleState],
    ["timeline sampling end is deterministic", endState.activeCoordinateId === "coord-reverse-40" && endState.activeStepOrder === 40, endState],
    ["timeline sampling uses exported-state selection", [startState, middleState, endState].every((state) => state.selection === "nearest-exported-step" && state.algorithmicComputation === false), [startState, middleState, endState]],
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
