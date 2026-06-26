#!/usr/bin/env node

import assert from "node:assert/strict";
import { readFileSync } from "node:fs";
import {
  buildRelationMatrixTextureData,
  createRelationMatrixLayerDescriptor,
  createRelationMatrixPicker,
  createRelationMatrixReadabilityProfile,
  MATRIX_FRAGMENT_SHADER,
  RELATION_MATRIX_READABILITY_DIAGNOSTICS_SCHEMA,
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
assert.deepEqual(readability.blocks.labels, [
  "normal reference",
  "flat hold",
  "late ramp",
  "spike",
  "early ramp",
]);
assert.equal(readability.blocks.labeledCount, 5);
assert.deepEqual(readability.blocks.coverage, {
  state: "full",
  coveredRecords: 130,
  uncoveredRecords: 0,
  contiguous: true,
  startsAtZero: true,
  endsAtSize: true,
});
assert.equal(readability.blocks.shaderRangeLimit, 32);
assert.equal(readability.blocks.shaderBoundaryLimit, 32);

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
assert.equal(readability.lod.tileSummaryLod.enabled, true);
assert.equal(readability.lod.tileSummaryLod.strategy, "gpu-tile-summary-texture");
assert.equal(readability.lod.tileSummaryLod.source, "exported-relation-texture-downsample");
assert.equal(readability.lod.tileSummaryLod.tileSize, 32);
assert.equal(readability.lod.tileSummaryLod.strength, 0.68);
assert(
  readability.lod.levels.some((level) => level.id === "logical-tiles" && level.role === "tile-summary-texture"),
  "LOD metadata includes GPU tile-summary texture level",
);

assert.equal(descriptor.picking.mode, "semantic-matrix-picker");
assert.equal(descriptor.picking.preservesNativePairIdentity, true);
assert.equal(descriptor.picking.domFallback, false);
assert.equal(descriptor.picking.svgFallback, false);
assert.deepEqual(descriptor.metadata.selectionModel.selectedFeatures, ["row", "column", "cell"]);
assert.deepEqual(descriptor.metadata.selectionModel.selectedFeatureSemantics.selectedFeatures, ["row", "column", "cell"]);
assert.equal(descriptor.metadata.selectionModel.picker, "semantic-matrix-picker");
assert.equal(descriptor.metadata.readabilityDiagnostics.schema, RELATION_MATRIX_READABILITY_DIAGNOSTICS_SCHEMA);
assert.deepEqual(descriptor.metadata.readabilityDiagnostics.matrixDimensions, { width: 130, height: 130, size: 130 });
assert.equal(descriptor.metadata.readabilityDiagnostics.blockCount, 5);
assert.equal(descriptor.metadata.readabilityDiagnostics.blockLabelCount, 5);
assert.deepEqual(descriptor.metadata.readabilityDiagnostics.blocks.labels, [
  "normal reference",
  "flat hold",
  "late ramp",
  "spike",
  "early ramp",
]);
assert.deepEqual(
  descriptor.metadata.readabilityDiagnostics.blocks.ranges.map((range) => ({
    label: range.label,
    start: range.start,
    endExclusive: range.endExclusive,
    size: range.size,
    unitStart: Number(range.unitStart.toFixed(3)),
    unitEnd: Number(range.unitEnd.toFixed(3)),
  })),
  [
    { label: "normal reference", start: 0, endExclusive: 26, size: 26, unitStart: 0, unitEnd: 0.2 },
    { label: "flat hold", start: 26, endExclusive: 52, size: 26, unitStart: 0.2, unitEnd: 0.4 },
    { label: "late ramp", start: 52, endExclusive: 78, size: 26, unitStart: 0.4, unitEnd: 0.6 },
    { label: "spike", start: 78, endExclusive: 104, size: 26, unitStart: 0.6, unitEnd: 0.8 },
    { label: "early ramp", start: 104, endExclusive: 130, size: 26, unitStart: 0.8, unitEnd: 1 },
  ],
);
assert.equal(descriptor.metadata.readabilityDiagnostics.blocks.coverage.state, "full");
assert.equal(descriptor.metadata.readabilityDiagnostics.tileCount, 25);
assert.equal(descriptor.metadata.readabilityDiagnostics.tileSummarySource, "exported-relation-texture-downsample");
assert.equal(descriptor.metadata.readabilityDiagnostics.missingValueCount, 0);
assert.equal(descriptor.metadata.readabilityDiagnostics.selected.state, "none");
assert.equal(descriptor.metadata.diagnostics.matrixReadability.schema, RELATION_MATRIX_READABILITY_DIAGNOSTICS_SCHEMA);

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
assert.deepEqual(layer.describeReadability().shaderCapacity, {
  rangeLimit: 32,
  boundaryLimit: 32,
  rangeCount: 5,
  boundaryCount: 4,
  truncatedRangeCount: 0,
  truncatedBoundaryCount: 0,
  truncated: false,
});

layer.gl = createTextureOnlyGl();
layer.replaceTexture(nativeTexture);
assert.equal(layer.describeReadability().tileSummary.width, 5);
assert.equal(layer.describeReadability().tileSummary.height, 5);
assert.equal(layer.describeReadability().tileSummary.tileSize, 32);
assert.equal(layer.describeReadability().tileSummary.source, "exported-relation-texture-downsample");
assert(layer.tileSummaryPayload.data[3] > 0, "tile summary keeps visible alpha for dense native tiles");
assert(layer.tileSummaryPayload.data[0] > 0 || layer.tileSummaryPayload.data[1] > 0 || layer.tileSummaryPayload.data[2] > 0, "tile summary carries alpha-weighted color");

layer.setSelection({ pair: nativeCell });
assert.deepEqual(layer.selection, {
  row: 0,
  column: 1,
  rowActive: true,
  columnActive: true,
  active: true,
});
assert.deepEqual(layer.getDiagnostics().matrixDimensions, { width: 130, height: 130, size: 130 });
assert.equal(layer.getDiagnostics().blockCount, 5);
assert.equal(layer.getDiagnostics().blockLabelCount, 5);
assert.deepEqual(layer.getDiagnostics().blocks.labels, [
  "normal reference",
  "flat hold",
  "late ramp",
  "spike",
  "early ramp",
]);
assert.equal(layer.getDiagnostics().blocks.coverage.state, "full");
assert.equal(layer.getDiagnostics().tileCount, 25);
assert.equal(layer.getDiagnostics().tileSummarySource, "exported-relation-texture-downsample");
assert.equal(layer.getDiagnostics().missingValueCount, 0);
assert.deepEqual(layer.getDiagnostics().selected, {
  row: { index: 0, active: true },
  column: { index: 1, active: true },
  cell: { row: 0, column: 1, active: true },
  state: "cell",
  matrixDimensions: { width: 130, height: 130, size: 130 },
});
assert.equal(layer.getDiagnostics().selectedPair.kind, "pair");
assert.equal(layer.getDiagnostics().selectedPair.rowId, "pc-000");
assert.equal(layer.getDiagnostics().selectedPair.columnId, "pc-001");
assert.equal(layer.getDiagnostics().selectedPair.pairKey, `${nativeRelation.id}\u0000pc-000\u0000pc-001`);
assert.equal(layer.getDiagnostics().selectedPair.nativePairPresent, true);
assert.equal(layer.getDiagnostics().selectedPair.linkedGraph.present, false);
assert.equal(layer.getDiagnostics().shaderCapacity.truncated, false);
assert.equal(layer.describeReadability().selectedPair.rowId, "pc-000");

layer.setSelection({ rowId: "pc-004" });
assert.deepEqual(layer.selection, {
  row: 4,
  column: -1,
  rowActive: true,
  columnActive: false,
  active: false,
});
assert.equal(layer.getDiagnostics().selected.state, "row");
assert.equal(layer.getDiagnostics().selected.row.index, 4);
assert.equal(layer.getDiagnostics().selected.column.index, null);

layer.setSelection({ columnId: "pc-005" });
assert.deepEqual(layer.selection, {
  row: -1,
  column: 5,
  rowActive: false,
  columnActive: true,
  active: false,
});
assert.equal(layer.getDiagnostics().selected.state, "column");
assert.equal(layer.getDiagnostics().selected.row.index, null);
assert.equal(layer.getDiagnostics().selected.column.index, 5);

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
assert.equal(largeProfile.lod.tileSummaryLod.strategy, "gpu-tile-summary-texture");
assert.equal(largeProfile.lod.tileSummaryLod.tileSize, 64);

assert(MATRIX_FRAGMENT_SHADER.includes("uLodSmoothingStrength"), "shader exposes dense-cell LOD smoothing uniform");
assert(MATRIX_FRAGMENT_SHADER.includes("neighborhoodTexel"), "shader includes weighted neighborhood smoothing");
assert(MATRIX_FRAGMENT_SHADER.includes("uTileSize"), "shader exposes logical tile boundary uniform");
assert(MATRIX_FRAGMENT_SHADER.includes("uTileSummaryTexture"), "shader exposes tile-summary texture uniform");
assert(MATRIX_FRAGMENT_SHADER.includes("uTileSummaryGridSize"), "shader exposes tile-summary grid uniform");
assert(MATRIX_FRAGMENT_SHADER.includes("tileSummaryMix"), "shader mixes tile-summary LOD at collapsed cell footprints");
assert(MATRIX_FRAGMENT_SHADER.includes("uFocusBackdropAlpha"), "shader exposes selected-focus backdrop dimming uniform");
assert(MATRIX_FRAGMENT_SHADER.includes("focusBackdropHit"), "shader dims non-focused matrix texture noise during selection");
assert(MATRIX_FRAGMENT_SHADER.includes("uFocusBlockAlpha"), "shader exposes selected-block context uniform");
assert(MATRIX_FRAGMENT_SHADER.includes("selectedBlockHit"), "shader highlights selected row/column block context");
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

function createTextureOnlyGl() {
  let id = 0;
  return {
    TEXTURE_2D: 0x0DE1,
    TEXTURE_MIN_FILTER: 0x2801,
    TEXTURE_MAG_FILTER: 0x2800,
    TEXTURE_WRAP_S: 0x2802,
    TEXTURE_WRAP_T: 0x2803,
    LINEAR: 0x2601,
    CLAMP_TO_EDGE: 0x812F,
    RGBA: 0x1908,
    UNSIGNED_BYTE: 0x1401,
    UNPACK_ALIGNMENT: 0x0CF5,
    createTexture() {
      id += 1;
      return { id };
    },
    deleteTexture() {},
    bindTexture() {},
    pixelStorei() {},
    texParameteri() {},
    texImage2D() {},
  };
}

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
