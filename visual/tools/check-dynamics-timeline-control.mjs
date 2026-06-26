#!/usr/bin/env node

import { readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import {
  TimelineModel,
  createTimelineControlDescriptor,
  sampleTimelineState,
} from "../src/timeline/index.js";
import { findTimelineControlDescriptor } from "../src/interaction/index.js";
import { DynamicsView } from "../src/views/DynamicsView.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const HIERARCHY_SCHEMA = "metric.visual.layer_hierarchy.v1";

async function main() {
  const document = JSON.parse(await readFile(
    resolve(ROOT, "docs/examples/assets/dynamics-noise/metric.visual.json"),
    "utf8",
  ));
  const model = new TimelineModel(document, {
    timelineId: "reverse-reconstruction",
    timelinePropertyId: "reverse-mse-to-clean",
    loop: true,
  });
  const control = createTimelineControlDescriptor(model, {
    timelinePropertyId: "reverse-mse-to-clean",
    loop: true,
  });
  const states = [
    sampleTimelineState(model, { id: "start", normalized: 0, timelinePropertyId: "reverse-mse-to-clean" }),
    sampleTimelineState(model, { id: "middle", normalized: 0.5, timelinePropertyId: "reverse-mse-to-clean" }),
    sampleTimelineState(model, { id: "end", normalized: 1, timelinePropertyId: "reverse-mse-to-clean" }),
  ];
  const sampledViews = states.map((state) => {
    const view = DynamicsView.fromVisualSpace(document, {
      timelineId: "reverse-reconstruction",
      timelineProgress: state.normalized,
      timelineFieldPropertyId: "reverse-mse-to-clean",
      propertyField: "best-reconstruction-error",
    });
    const descriptors = view.toLayerDescriptors();
    const field = descriptors.find((descriptor) => descriptor.primitive === "HeatFieldLayer"
      && descriptor.metadata?.evidenceRole === "timeline-ground-field");
    const point = descriptors.find((descriptor) => descriptor.primitive === "InstancedPointLayer"
      && descriptor.metadata?.role === "current-timeline-state");
    const timelineScalar = field?.channels?.timelineScalar?.array?.[0];
    return {
      normalized: state.normalized,
      activeStep: view.activeStep,
      pointCoordinateId: point?.metadata?.activeCoordinateId,
      fieldCoordinateId: field?.metadata?.activeCoordinateId,
      fieldPrimitive: field?.primitive,
      fieldHierarchy: field?.metadata?.visualHierarchy,
      pointHierarchy: point?.metadata?.visualHierarchy,
      fieldState: field?.metadata?.timelineFieldState,
      fieldStateSource: field?.metadata?.fieldStateSource,
      activeFieldPropertyId: field?.metadata?.activeFieldPropertyId,
      algorithmicComputation: field?.metadata?.algorithmicComputation,
      timelineScalar,
      timelineScalarCount: field?.channels?.timelineScalar?.count,
      recordCount: field?.metadata?.recordCount,
    };
  });
  const emittedControl = findTimelineControlDescriptor(DynamicsView.fromVisualSpace(document, {
    timelineId: "reverse-reconstruction",
    timelineFieldPropertyId: "reverse-mse-to-clean",
    propertyField: "best-reconstruction-error",
  }).toLayerDescriptors());
  const markCoordinates = control.marks.map((mark) => mark.activeCoordinateId);
  const fieldValues = sampledViews.map((sample) => Number(sample.fieldState?.value));
  const checks = [
    ["timeline control remains schema-backed", control.schema === "metric.visual.timeline_control.v1", control],
    ["timeline control is marked user-facing", control.userFacing === true && control.presentation?.kind === "timeline-scrubber", control.presentation],
    ["timeline control exposes range, play and reset controls", control.controls.map((entry) => entry.role).join(",") === "timeline-state,timeline-playback,timeline-reset", control.controls],
    ["timeline range control binds normalized state", control.controls[0]?.binding?.source === "exported-timeline" && control.controls[0]?.display?.kind === "scrubber", control.controls[0]],
    ["timeline control descriptor is discoverable from DynamicsView descriptors", emittedControl?.schema === "metric.visual.timeline_control.v1" && emittedControl?.controls?.[0]?.binding?.source === "exported-timeline", emittedControl],
    ["timeline control marks start/middle/end exported coordinates", markCoordinates.join(",") === "coord-reverse-00,coord-reverse-20,coord-reverse-40", markCoordinates],
    ["timeline marks carry exported property samples", control.marks.every((mark) => mark.activePropertySample?.propertyId === "reverse-mse-to-clean" && mark.activePropertySample.algorithmicComputation === false), control.marks],
    ["timeline states select exported property samples", states.every((state) => state.activePropertySample?.propertyId === "reverse-mse-to-clean" && state.activePropertySample.source === "exported-property"), states],
    ["dynamics sampled views select exported start/middle/end coordinates", sampledViews.map((sample) => sample.pointCoordinateId).join(",") === "coord-reverse-00,coord-reverse-20,coord-reverse-40", sampledViews],
    ["dynamics field descriptors are reusable HeatFieldLayer descriptors", sampledViews.every((sample) => sample.fieldPrimitive === "HeatFieldLayer" && sample.recordCount === 512), sampledViews],
    ["dynamics sampled field descriptors keep support hierarchy", sampledViews.every((sample) => isHierarchy(sample.fieldHierarchy, "support-field") && sample.fieldHierarchy.drawsBelow?.includes("trajectory-path") && sample.fieldHierarchy.drawsBelow?.includes("current-state")), sampledViews.map((sample) => sample.fieldHierarchy)],
    ["dynamics sampled current-state descriptors keep current hierarchy", sampledViews.every((sample) => isHierarchy(sample.pointHierarchy, "current-state") && sample.pointHierarchy.drawsAbove?.includes("support-field") && sample.pointHierarchy.drawsBelow?.includes("scene-labels")), sampledViews.map((sample) => sample.pointHierarchy)],
    ["dynamics field state follows exported reverse MSE samples", roughly(fieldValues[0], 0.0153362) && roughly(fieldValues[1], 0.0050846) && roughly(fieldValues[2], 0.0103288), fieldValues],
    ["dynamics field scalar channel changes with timeline state", sampledViews.map((sample) => Number(sample.timelineScalar).toFixed(7)).join(",") === "0.0153362,0.0050846,0.0103288", sampledViews],
    ["dynamics field uses selected exported record property without recomputing dynamics", sampledViews.every((sample) => sample.activeFieldPropertyId === "best-reconstruction-error" && sample.algorithmicComputation === false), sampledViews],
    ["dynamics field state is exported property evidence", sampledViews.every((sample) => sample.fieldState?.source === "exported-property" && sample.fieldState?.algorithmicComputation === false), sampledViews],
  ];
  report(checks);
}

function roughly(actual, expected) {
  return Math.abs(Number(actual) - expected) < 0.0000005;
}

function isHierarchy(hierarchy, band) {
  return hierarchy?.schema === HIERARCHY_SCHEMA
    && hierarchy.band === band
    && hierarchy.algorithmicComputation === false;
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
