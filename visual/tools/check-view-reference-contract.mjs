#!/usr/bin/env node

import assert from "node:assert/strict";
import { readFileSync } from "node:fs";
import { join } from "node:path";
import {
  MappingView,
  MetricSpaceView,
  NeighborhoodGraphView,
  RelationMatrixView,
} from "../src/views/index.js";

const root = new URL("../..", import.meta.url).pathname;

const condition = readVisualAsset("condition-monitoring");
const mapping = readVisualAsset("mapping-dimensionality");
const relationMatrix = readVisualAsset("relation-matrix");

assert.throws(
  () => MetricSpaceView.fromVisualSpace(condition, {
    coordinateId: "missing-coordinate",
  }),
  /Unknown coordinate reference: missing-coordinate/,
);

assert.throws(
  () => MetricSpaceView.fromVisualSpace(condition, {
    coordinateId: "process-state-trajectory-3d",
    labelProperty: "missing-label-property",
  }),
  /Unknown label property reference: missing-label-property/,
);

assert.throws(
  () => RelationMatrixView.fromVisualSpace(relationMatrix, {
    relationId: "missing-relation",
  }),
  /Unknown relation reference: missing-relation/,
);

assert.throws(
  () => NeighborhoodGraphView.fromVisualSpace(relationMatrix, {
    relationId: "missing-graph-relation",
  }),
  /Unknown relation reference: missing-graph-relation/,
);

assert.throws(
  () => MappingView.fromVisualSpace(mapping, {
    sourceCoordinateId: "source-coordinate-layout-3d",
    targetCoordinateId: "parametric-coordinate-latent-2d",
    labels: "missing-process-family",
  }),
  /Unknown label property reference: missing-process-family/,
);

const mappingView = MappingView.fromVisualSpace(mapping, {
  sourceCoordinateId: "source-coordinate-layout-3d",
  targetCoordinateId: "parametric-coordinate-latent-2d",
  residualProperty: "local-mapping-distortion",
  labels: "process-family",
});
const labelDescriptor = mappingView.toLayerDescriptors().find((descriptor) => descriptor.primitive === "BillboardLabelLayer");
assert.ok(labelDescriptor, "MappingView should emit a label layer when labels references a property.");
assert.ok(labelDescriptor.metadata?.labelCount > 0, "MappingView label layer should contain labels.");

console.log("View reference contract passed.");

function readVisualAsset(name) {
  return JSON.parse(readFileSync(join(root, "docs/examples/assets", name, "metric.visual.json"), "utf8"));
}
