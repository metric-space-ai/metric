#!/usr/bin/env node

import assert from "node:assert/strict";
import { readFileSync } from "node:fs";
import {
  buildRelationMatrixTextureData,
  createRelationMatrixLayerDescriptor,
  createRelationMatrixPicker,
  createRelationMatrixReadabilityProfile,
  MATRIX_FRAGMENT_SHADER,
  RelationMatrixLayer,
} from "../src/relational/index.js";

const nativeDocument = JSON.parse(readFileSync(new URL("../../docs/examples/assets/relation-matrix/metric.visual.json", import.meta.url), "utf8"));
const nativeRelation = nativeDocument.relations.find((relation) => relation.id === "process-curve-aligned-metric");
assert(nativeRelation, "native relation-matrix visual evidence is present");

const nativeTexture = buildRelationMatrixTextureData(nativeRelation, {
  symmetric: "mirror",
  recordIds: nativeRelation.record_ids,
});
const descriptor = createRelationMatrixLayerDescriptor(nativeTexture, {
  relationId: nativeRelation.id,
  relationName: nativeRelation.name,
});
const readability = descriptor.metadata.readability;

assert.equal(readability.kind, "relation-matrix-readability-profile");
assert.equal(readability.matrixSize, 130);
assert.equal(readability.densityClass, "dense");
assert.equal(readability.renderer.primitive, "RelationMatrixLayer");
assert.equal(readability.renderer.webglOnly, true);
assert.equal(readability.renderer.domFallback, false);
assert.equal(readability.renderer.svgFallback, false);
assert.equal(readability.renderer.semanticPicker, "relation-matrix-picking");
assert.equal(readability.renderer.preservesNativePairIdentity, true);

assert.deepEqual(
  readability.blocks.boundaries.map((boundary) => boundary.index),
  [26, 52, 78, 104],
);
assert.deepEqual(
  readability.blocks.boundaries.map((boundary) => Number(boundary.unit.toFixed(3))),
  [0.2, 0.4, 0.6, 0.8],
);
assert.equal(readability.blocks.ranges.length, 5);
assert.equal(readability.blocks.shaderBoundaryLimit, 16);

assert.equal(readability.tiles.kind, "relation-matrix-logical-tile-grid");
assert.equal(readability.tiles.tileSize, 32);
assert.equal(readability.tiles.rows, 5);
assert.equal(readability.tiles.columns, 5);
assert.equal(readability.tiles.count, 25);
assert.equal(readability.tiles.coverage, "full");
assert.deepEqual(readability.tiles.rowBoundaries.map((boundary) => boundary.index), [32, 64, 96, 128]);
assert(readability.tiles.summaries[0].presentRatio > 0.99, "first tile summarizes dense present cells");
assert(Number.isFinite(readability.tiles.summaries[0].mean), "first tile carries finite value summary");

assert.equal(readability.lod.kind, "shader-footprint-lod");
assert.equal(readability.lod.strategy, "cell-footprint-neighborhood-smoothing");
assert.equal(readability.lod.denseCellSmoothing.enabled, true);
assert.equal(readability.lod.denseCellSmoothing.kernel, "weighted-3x3");
assert.equal(readability.lod.denseCellSmoothing.smoothingCellPixels, 4.25);
assert.equal(readability.lod.denseCellSmoothing.lodSmoothingStrength, 0.46);
assert(readability.lod.levels.some((level) => level.id === "logical-tiles"), "LOD metadata includes logical tile level");

assert.equal(descriptor.picking.mode, "semantic-matrix-picker");
assert.equal(descriptor.picking.preservesNativePairIdentity, true);
assert.equal(descriptor.picking.domFallback, false);
assert.equal(descriptor.picking.svgFallback, false);
assert.deepEqual(descriptor.metadata.selectionModel.selectedFeatures, ["row", "column", "cell"]);
assert.deepEqual(descriptor.metadata.selectionModel.selectedFeatureSemantics.selectedFeatures, ["row", "column", "cell"]);
assert.equal(descriptor.metadata.selectionModel.picker, "semantic-matrix-picker");

const picker = createRelationMatrixPicker(descriptor);
const nativeCell = picker.cellAtNormalizedPoint((1.5 / 130), (0.5 / 130));
assert.equal(nativeCell.rowId, "pc-000");
assert.equal(nativeCell.columnId, "pc-001");
assert.equal(nativeCell.relationId, nativeRelation.id);
assert.equal(nativeCell.pairKey, `${nativeRelation.id}\u0000pc-000\u0000pc-001`);
assert(nativeCell.nativePair, "semantic picker preserves native pair evidence");

const layer = new RelationMatrixLayer(descriptor);
layer.updateBlockBoundaries(nativeTexture);
assert.equal(layer.blockBoundaryCount, 4);
assert.equal(layer.blockRangeCount, 5);
assert.equal(layer.describeReadability().domFallback, false);
assert.equal(layer.describeReadability().svgFallback, false);
assert.equal(layer.describeReadability().readability.tiles.tileSize, 32);

layer.setSelection({ pair: nativeCell });
assert.deepEqual(layer.selection, {
  row: 0,
  column: 1,
  rowActive: true,
  columnActive: true,
  active: true,
});

layer.setSelection({ rowId: "pc-004" });
assert.deepEqual(layer.selection, {
  row: 4,
  column: -1,
  rowActive: true,
  columnActive: false,
  active: false,
});

layer.setSelection({ columnId: "pc-005" });
assert.deepEqual(layer.selection, {
  row: -1,
  column: 5,
  rowActive: false,
  columnActive: true,
  active: false,
});

layer.setSelection({ recordId: "pc-006" });
assert.deepEqual(layer.selection, {
  row: 6,
  column: 6,
  rowActive: true,
  columnActive: true,
  active: true,
});

layer.setSelection({ row: 7, column: 8 });
assert.deepEqual(layer.selection, {
  row: 7,
  column: 8,
  rowActive: true,
  columnActive: true,
  active: true,
});

const largeMatrix = buildDenseMatrixEvidence(384);
const largeProfile = createRelationMatrixReadabilityProfile(largeMatrix);
assert.equal(largeProfile.matrixSize, 384);
assert.equal(largeProfile.densityClass, "dense");
assert.equal(largeProfile.tiles.tileSize, 64);
assert.equal(largeProfile.tiles.rows, 6);
assert.equal(largeProfile.tiles.columns, 6);
assert.equal(largeProfile.tiles.count, 36);
assert.equal(largeProfile.tiles.coverage, "full");
assert.equal(largeProfile.lod.denseCellSmoothing.kernel, "weighted-3x3");

assert(MATRIX_FRAGMENT_SHADER.includes("uLodSmoothingStrength"), "shader exposes dense-cell LOD smoothing uniform");
assert(MATRIX_FRAGMENT_SHADER.includes("neighborhoodTexel"), "shader includes weighted neighborhood smoothing");
assert(MATRIX_FRAGMENT_SHADER.includes("uTileSize"), "shader exposes logical tile boundary uniform");
assert(MATRIX_FRAGMENT_SHADER.includes("uSelectionRowColor"), "shader separates row selection color");
assert(MATRIX_FRAGMENT_SHADER.includes("uSelectionColumnColor"), "shader separates column selection color");
assert(MATRIX_FRAGMENT_SHADER.includes("uSelectionCellColor"), "shader separates cell selection color");

for (const [label, source] of Object.entries({
  RelationMatrixLayer: readFileSync(new URL("../src/relational/RelationMatrixLayer.js", import.meta.url), "utf8"),
  descriptors: readFileSync(new URL("../src/relational/descriptors.js", import.meta.url), "utf8"),
  readability: readFileSync(new URL("../src/relational/matrix-readability.js", import.meta.url), "utf8"),
})) {
  assert(!/\bdocument\./.test(source), `${label}: no page DOM access`);
  assert(!/\bcreateElement\b/.test(source), `${label}: no DOM element fallback`);
  assert(!/<svg\b/i.test(source), `${label}: no SVG fallback markup`);
  assert(!/\bSVGElement\b/.test(source), `${label}: no SVG runtime dependency`);
  assert(!/\bCanvasRenderingContext2D\b/.test(source), `${label}: no 2D canvas fallback`);
}

console.log("Relation matrix readability contract passed.");

function buildDenseMatrixEvidence(size) {
  const recordIds = Array.from({ length: size }, (_, index) => `large-${String(index).padStart(3, "0")}`);
  const values = new Float32Array(size * size);
  const present = new Uint8Array(size * size);
  for (let row = 0; row < size; row += 1) {
    for (let column = 0; column < size; column += 1) {
      const offset = row * size + column;
      values[offset] = row === column ? 0 : Math.abs(row - column) + ((row + column) % 11) * 0.01;
      present[offset] = 1;
    }
  }
  return {
    kind: "dense-relation-matrix",
    size,
    width: size,
    height: size,
    recordIds,
    values,
    present,
    presentValueCount: size * size,
    blockRanges: [
      { start: 0, end_exclusive: 96, label: "A" },
      { start: 96, end_exclusive: 192, label: "B" },
      { start: 192, end_exclusive: 288, label: "C" },
      { start: 288, end_exclusive: 384, label: "D" },
    ],
  };
}
