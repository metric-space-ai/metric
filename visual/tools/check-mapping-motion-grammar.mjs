#!/usr/bin/env node

import { readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import {
  MappingView,
  mappingMotionProgressAt,
} from "../src/views/MappingView.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");

async function main() {
  const document = JSON.parse(await readFile(
    resolve(ROOT, "docs/examples/assets/mapping-dimensionality/metric.visual.json"),
    "utf8",
  ));
  const sourceCoordinateId = "source-coordinate-layout-3d";
  const targetCoordinateId = "parametric-coordinate-latent-2d";
  const residualPropertyId = "local-mapping-distortion";
  const view = MappingView.fromVisualSpace(document, {
    sourceCoordinateId,
    targetCoordinateId,
    residualProperty: residualPropertyId,
    labels: "process-family",
    morphDurationMs: 5200,
    motionTiming: {
      profile: "source-hold-quick-transition-target-hold",
      sourceHoldMs: 1600,
      transitionMs: 720,
      targetHoldMs: 1700,
      resetHoldMs: 360,
      totalMs: 5200,
    },
  });
  const descriptors = view.toLayerDescriptors();
  const point = descriptors.find((descriptor) => descriptor.primitive === "InstancedPointLayer" || descriptor.primitive === "InstancedGlyphLayer");
  const residual = descriptors.find((descriptor) => descriptor.metadata?.role === "residual/error");
  const labels = descriptors.find((descriptor) => descriptor.primitive === "BillboardLabelLayer");
  const noResidualView = MappingView.fromVisualSpace(document, {
    sourceCoordinateId,
    targetCoordinateId,
    labels: "process-family",
  });
  const noResidualDescriptors = noResidualView.toLayerDescriptors();
  const noResidualPoint = noResidualDescriptors.find((descriptor) => descriptor.primitive === "InstancedPointLayer" || descriptor.primitive === "InstancedGlyphLayer");
  const noResidualLayer = noResidualDescriptors.find((descriptor) => descriptor.metadata?.role === "residual/error");
  const timing = point?.metadata?.mappingMotionTiming;
  const transitionFrame = timing
    ? mappingMotionProgressAt(timing.sourceHoldMs + timing.transitionMs * 0.5, timing)
    : { progress: 0, phase: "missing" };
  const checks = [
    ["mapping consumes native source coordinate", view.sourceCoordinateId === sourceCoordinateId, view.sourceCoordinateId],
    ["mapping consumes native target coordinate", view.targetCoordinateId === targetCoordinateId, view.targetCoordinateId],
    ["mapping consumes residual property", view.propertyId === residualPropertyId, view.propertyId],
    ["mapping has one record per exported source point", view.recordIds.length === 1000, view.recordIds.length],
    ["mapping declares coordinate morph", point?.animation?.mode === "coordinate-morph", point?.animation],
    ["mapping morph uses hold/quick/hold timing", point?.animation?.timingProfile === "source-hold-quick-transition-target-hold" && timing?.transitionShare < 0.2, point?.animation],
    ["mapping timing holds source before transition", timing && mappingMotionProgressAt(100, timing).progress === 0, timing],
    ["mapping timing enters quick transition", transitionFrame.progress > 0.3 && transitionFrame.progress < 0.7 && transitionFrame.phase === "quick-transition", transitionFrame],
    ["mapping timing holds target after transition", timing && mappingMotionProgressAt(timing.sourceHoldMs + timing.transitionMs + 100, timing).progress === 1, timing],
    ["mapping point layer carries targetPosition", point?.channels?.targetPosition?.count === 1000, point?.channels?.targetPosition?.count],
    ["mapping renders residual/error vectors", residual?.primitive === "RelationEdgeLayer", residual?.primitive],
    ["mapping residual vector count matches records", residual?.metadata?.recordCount === 1000, residual?.metadata],
    ["mapping residual vector layer carries record ids", residual?.channels?.recordId?.count === 1000, residual?.channels?.recordId],
    ["mapping residual vector layer carries residual magnitudes", residual?.channels?.residual?.count === 1000 && residual?.channels?.residual?.semantic === "residual-magnitude", residual?.channels?.residual],
    ["mapping residual layer declares motion evidence", residual?.metadata?.mappingEvidence?.schema === "metric.visual.mapping_motion_evidence.v1" && residual?.metadata?.diagnosticLayer === true, residual?.metadata?.mappingEvidence],
    ["mapping morph layer declares mapping grammar", point?.metadata?.mappingEvidence?.schema === "metric.visual.mapping_motion_evidence.v1" && point?.metadata?.motionGrammar === "mapping-coordinate-morph", point?.metadata],
    ["mapping declares no JavaScript algorithmic computation", point?.metadata?.algorithmicComputation === false && residual?.metadata?.algorithmicComputation === false, { point: point?.metadata, residual: residual?.metadata }],
    ["mapping preservation summary covers native records", point?.metadata?.mappingEvidence?.preservationSummary?.count === 1000, point?.metadata?.mappingEvidence?.preservationSummary],
    ["mapping renders exported label property", Boolean(labels), descriptors.map((descriptor) => descriptor.primitive)],
    ["mapping without residual property emits no residual/error vectors", !noResidualLayer, noResidualLayer],
    ["mapping without residual property reports no residual motion layer", noResidualPoint?.metadata?.mappingEvidence?.motionContract?.residualLayer === null, noResidualPoint?.metadata?.mappingEvidence],
    ["mapping without residual property reports no preservation summary", noResidualView.preservationSummary().count === 0, noResidualView.preservationSummary()],
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
