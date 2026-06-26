#!/usr/bin/env node

import assert from "node:assert/strict";
import { readFileSync } from "node:fs";
import { MetricVisualSurface } from "../src/metric-visual.js";
import { createRelationMatrixPicker, RelationMatrixLayer } from "../src/relational/index.js";

const MATRIX_RECT = [0.055, 0.16, 0.49, 0.76];
const PAIR_PREVIEW_OFFSET = [760, 18];
const VIEWPORT = { width: 1280, height: 800 };
const PREVIEW_PANEL = { width: 320, height: 120, margin: 14 };

const nativeDocument = JSON.parse(readFileSync(new URL("../../docs/examples/assets/relation-matrix/metric.visual.json", import.meta.url), "utf8"));
const pageSource = readFileSync(new URL("../examples/relation-matrix-neighborhood/index.html", import.meta.url), "utf8");

assert(pageSource.includes("showRelationMatrixNeighborhood"), "example uses reusable relation-matrix-neighborhood command");
assert(!/\bgetContext\s*\(\s*['\"]2d['\"]\s*\)/.test(pageSource), "example does not use a 2D canvas matrix renderer");
assert(!/<svg\b/i.test(pageSource), "example does not use an SVG matrix renderer");
assert(!/\bnew\s+RelationMatrixLayer\b/.test(pageSource), "example does not instantiate a page-local matrix layer");
assert(!/\bcreateRelationMatrixLayerDescriptor\b/.test(pageSource), "example does not assemble a page-local matrix descriptor");
assert(pageSource.includes("PAIR_PREVIEW_OFFSET"), "example declares preview offset instead of relying on pointer-center overlay");

const runtime = createHeadlessRuntime();
const surface = new MetricVisualSurface({
  document: nativeDocument,
  canvas: null,
  runtime,
  setup: {},
  options: { context: "relation-matrix-composition-check" },
});

surface.showRelationMatrixNeighborhood({
  coordinateId: "process-curve-block-layout-3d",
  relationId: "process-curve-aligned-metric",
  matrixRect: MATRIX_RECT,
  symmetric: true,
  colorProperty: "process-family",
  labels: "process-family",
  graph: "neighborhood",
  pairPreview: true,
  previewOptions: { panel: { offset: PAIR_PREVIEW_OFFSET } },
  topK: 3,
  pointSize: 1.25,
});

const diagnostics = surface.getDiagnostics();
assert.equal(diagnostics.selectedViewKind, "relation-matrix-neighborhood");

const primitives = new Set(surface.descriptors.map((descriptor) => descriptor.primitive || descriptor.kind));
assert(primitives.has("RelationMatrixLayer"), "descriptors include RelationMatrixLayer");
assert(primitives.has("RelationEdgeLayer"), "descriptors include RelationEdgeLayer");
assert(primitives.has("InstancedPointLayer"), "descriptors include InstancedPointLayer");

const roles = new Set(surface.descriptors.map((descriptor) => descriptor.metadata?.role || descriptor.source?.role).filter(Boolean));
assert(roles.has("primary-relation-matrix"), "matrix descriptor exposes its primary semantic role");
assert(roles.has("neighborhood-graph-nodes"), "graph node descriptor exposes its semantic role");
assert(roles.has("neighborhood-graph-edges"), "graph edge descriptor exposes its semantic role");

const matrixDescriptor = surface.descriptors.find((descriptor) => descriptor.primitive === "RelationMatrixLayer");
assert(matrixDescriptor, "matrix descriptor is present");
assert.deepEqual(matrixDescriptor.geometry.rect, MATRIX_RECT);
assert.equal(matrixDescriptor.metadata.matrix.size, 130);
assert.equal(matrixDescriptor.metadata.role, "primary-relation-matrix");
assert.equal(matrixDescriptor.metadata.composition.role, "primary-relation-matrix");
assert.equal(matrixDescriptor.metadata.selectionModel.previewPlacement.strategy, "avoid-matrix-center");

const readability = matrixDescriptor.metadata.readabilityDiagnostics;
assert(readability.blockCount >= 2, "matrix readability has at least two blocks");
assert(readability.blockLabelCount >= 2, "matrix readability has at least two labelled blocks");
assert.equal(readability.blocks.coverage.state, "full");
assert.equal(readability.tileSummarySource, "exported-relation-texture-downsample");

const centerRegion = rectToPixels(matrixDescriptor.metadata.composition.centerRegion, VIEWPORT);
const matrixCenterPoint = rectCenter(rectToPixels(MATRIX_RECT, VIEWPORT));
const previewRect = previewRectFor(matrixCenterPoint, PAIR_PREVIEW_OFFSET, VIEWPORT, PREVIEW_PANEL);
assert(!rectsOverlap(previewRect, centerRegion), "default pair preview placement avoids matrix center region");

const picker = createRelationMatrixPicker(matrixDescriptor);
const selectedCell = picker.cellAtNormalizedPoint(
  MATRIX_RECT[0] + (1.5 / 130) * MATRIX_RECT[2],
  MATRIX_RECT[1] + (0.5 / 130) * MATRIX_RECT[3],
);
assert(selectedCell, "interaction probe selects a matrix cell");

const layer = new RelationMatrixLayer(matrixDescriptor);
layer.updateBlockBoundaries(matrixDescriptor.source.texture);
layer.setSelection({ pair: selectedCell });
const layerDiagnostics = layer.getDiagnostics();

assert.equal(layerDiagnostics.selected.state, "cell");
assert.equal(layerDiagnostics.selected.row.active, true);
assert.equal(layerDiagnostics.selected.column.active, true);
assert.equal(layerDiagnostics.selected.cell.active, true);
assert.equal(layerDiagnostics.selectedPair.rowId, "pc-000");
assert.equal(layerDiagnostics.selectedPair.columnId, "pc-001");
assert.equal(layerDiagnostics.blockCoverage.state, "full");
assert.equal(layerDiagnostics.tileSummarySource, "exported-relation-texture-downsample");
assert.equal(layerDiagnostics.blockTruncation.truncated, false);
assert.equal(layer.describeReadability().blockTruncation.truncated, false);
assert.equal(layer.describeReadability().diagnostics.tileSummarySource, "exported-relation-texture-downsample");

console.log("Relation matrix composition contract passed.");

function createHeadlessRuntime() {
  return {
    layerDescriptors: [],
    setLayerDescriptors(descriptors) {
      this.layerDescriptors = descriptors.slice();
    },
    renderOnce() {},
    getState() {
      return {
        layerInstanceCount: this.layerDescriptors.length,
        layerState: { status: "headless" },
        inspection: {},
      };
    },
    on() {
      return () => {};
    },
  };
}

function rectToPixels(rect, viewport) {
  return {
    x: rect[0] * viewport.width,
    y: rect[1] * viewport.height,
    width: rect[2] * viewport.width,
    height: rect[3] * viewport.height,
  };
}

function rectCenter(rect) {
  return {
    x: rect.x + rect.width * 0.5,
    y: rect.y + rect.height * 0.5,
  };
}

function previewRectFor(point, offset, viewport, panel) {
  return {
    x: clamp(point.x + offset[0], panel.margin, viewport.width - panel.width - panel.margin),
    y: clamp(point.y + offset[1], panel.margin, viewport.height - panel.height - panel.margin),
    width: panel.width,
    height: panel.height,
  };
}

function rectsOverlap(a, b) {
  return a.x < b.x + b.width
    && a.x + a.width > b.x
    && a.y < b.y + b.height
    && a.y + a.height > b.y;
}

function clamp(value, min, max) {
  return Math.min(max, Math.max(min, value));
}
