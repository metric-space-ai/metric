#!/usr/bin/env node

import { readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import { MappingView } from "../src/views/MappingView.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");

async function main() {
  const document = JSON.parse(await readFile(
    resolve(ROOT, "docs/examples/assets/mapping-dimensionality/metric.visual.json"),
    "utf8",
  ));
  const view = MappingView.fromVisualSpace(document, {
    sourceCoordinateId: "phate-target-2d",
    targetCoordinateId: "source-feature-layout-3d",
    residualProperty: "local-mapping-distortion",
    labels: "process-family",
    morphDurationMs: 6400,
  });
  const descriptors = view.toLayerDescriptors();
  const point = descriptors.find((descriptor) => descriptor.primitive === "InstancedPointLayer");
  const residual = descriptors.find((descriptor) => descriptor.metadata?.role === "residual/error");
  const labels = descriptors.find((descriptor) => descriptor.primitive === "BillboardLabelLayer");
  const checks = [
    ["mapping consumes source coordinate", view.sourceCoordinateId === "phate-target-2d", view.sourceCoordinateId],
    ["mapping consumes target coordinate", view.targetCoordinateId === "source-feature-layout-3d", view.targetCoordinateId],
    ["mapping consumes residual property", view.propertyId === "local-mapping-distortion", view.propertyId],
    ["mapping has one record per exported source point", view.recordIds.length === 15, view.recordIds.length],
    ["mapping declares coordinate morph", point?.animation?.mode === "coordinate-morph", point?.animation],
    ["mapping point layer carries targetPosition", point?.channels?.targetPosition?.count === 15, point?.channels?.targetPosition?.count],
    ["mapping renders residual/error vectors", residual?.primitive === "RelationEdgeLayer", residual?.primitive],
    ["mapping residual vector count matches records", residual?.metadata?.recordCount === 15, residual?.metadata],
    ["mapping renders exported label property", Boolean(labels), descriptors.map((descriptor) => descriptor.primitive)],
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
