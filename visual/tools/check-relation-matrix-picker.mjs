#!/usr/bin/env node

import assert from "node:assert/strict";
import { readFileSync } from "node:fs";
import {
  buildRelationMatrixTextureData,
  createRelationGraphEdgeLayerDescriptor,
  createRelationMatrixPicker,
  pickRelationMatrixCell,
  RelationMatrixLayer,
} from "../src/relational/index.js";

const matrix = {
  kind: "dense-relation-matrix",
  size: 3,
  width: 3,
  height: 3,
  recordIds: ["a", "b", "c"],
  values: new Float32Array([
    0, 1, 2,
    3, 0, 4,
    5, 6, 0,
  ]),
  present: new Uint8Array([
    1, 1, 1,
    1, 1, 0,
    1, 1, 1,
  ]),
};

const descriptor = {
  primitive: "RelationMatrixLayer",
  geometry: { rect: [0.25, 0.2, 0.5, 0.6] },
  source: { relationId: "toy-metric" },
  metadata: { matrix },
};

const picker = createRelationMatrixPicker(descriptor);

assert.deepEqual(picker.describe(), {
  size: 3,
  recordCount: 3,
  rect: [0.25, 0.2, 0.5, 0.6],
});

assert.equal(picker.cellAtNormalizedPoint(0.24, 0.5), null);
assert.equal(picker.cellAtNormalizedPoint(0.51, 0.19), null);

let cell = picker.cellAtNormalizedPoint(0.26, 0.21);
assert.equal(cell.row, 0);
assert.equal(cell.column, 0);
assert.equal(cell.rowId, "a");
assert.equal(cell.columnId, "a");
assert.equal(cell.value, 0);
assert.equal(cell.present, true);

cell = picker.cellAtNormalizedPoint(0.74, 0.79);
assert.equal(cell.row, 2);
assert.equal(cell.column, 2);
assert.equal(cell.rowId, "c");
assert.equal(cell.columnId, "c");

cell = picker.cellAtCanvasPoint(250, 320, 1000, 800);
assert.equal(cell.rowId, "b");
assert.equal(cell.columnId, "a");
assert.equal(cell.value, 3);

cell = picker.cellAtNormalizedPoint(0.58, 0.50);
assert.equal(cell.rowId, "b");
assert.equal(cell.columnId, "b");
assert.equal(cell.present, true);

cell = picker.cellAtNormalizedPoint(0.74, 0.50);
assert.equal(cell.rowId, "b");
assert.equal(cell.columnId, "c");
assert.equal(cell.present, false);

const fakeCanvas = {
  getBoundingClientRect() {
    return { left: 10, top: 20, width: 1000, height: 800 };
  },
};
cell = createRelationMatrixPicker(descriptor, { canvas: fakeCanvas }).cellAtClientPoint(260, 340);
assert.equal(cell.rowId, "b");
assert.equal(cell.columnId, "a");

cell = pickRelationMatrixCell(descriptor, { normalized: true, x: 0.26, y: 0.79 });
assert.equal(cell.rowId, "c");
assert.equal(cell.columnId, "a");

const layer = new RelationMatrixLayer(descriptor);
layer.setSelection({
  pair: {
    relationId: "toy-metric",
    rowId: "b",
    columnId: "c",
    row: 1,
    column: 2,
    value: 4,
  },
});
assert.deepEqual(layer.selection, {
  row: 1,
  column: 2,
  active: true,
});

layer.setSelection({ recordId: "a" });
assert.deepEqual(layer.selection, {
  row: 0,
  column: 0,
  active: true,
});

layer.setSelection({ pair: { rowId: "missing", columnId: "a" } });
assert.deepEqual(layer.selection, {
  row: -1,
  column: 0,
  active: false,
});

assert.throws(
  () => createRelationMatrixPicker({ metadata: { matrix: { kind: "dense-relation-matrix", size: 0 } } }),
  /positive integer matrix size/,
);

const nativeDocument = JSON.parse(readFileSync(new URL("../../docs/examples/assets/relation-matrix/metric.visual.json", import.meta.url), "utf8"));
const nativeRelation = nativeDocument.relations.find((relation) => relation.id === "process-curve-aligned-metric");
const nativeGraph = nativeDocument.graphs.find((graph) => graph.edge_relation_id === nativeRelation.id);
const nativeTexture = buildRelationMatrixTextureData(nativeRelation, {
  symmetric: "mirror",
  recordIds: nativeRelation.record_ids,
});

assert.equal(nativeTexture.matrix.size, 130);
assert.equal(nativeTexture.matrix.blockRanges.length, 5);
assert.equal(nativeTexture.matrix.missingValueCount, 0);
assert.equal(nativeTexture.matrix.diagnostics.matrixDimensions.width, 130);
assert.equal(nativeTexture.matrix.diagnostics.metricLawDiagnostic.triangle, true);

const nativeLayer = new RelationMatrixLayer({ metadata: { matrix: nativeTexture.matrix } });
nativeLayer.updateBlockBoundaries(nativeTexture);
assert.equal(nativeLayer.blockBoundaryCount, 4);
assert.deepEqual(
  Array.from(nativeLayer.blockBoundaries.slice(0, nativeLayer.blockBoundaryCount)).map((value) => Number(value.toFixed(3))),
  [0.2, 0.4, 0.6, 0.8],
);

const graphDescriptor = createRelationGraphEdgeLayerDescriptor(nativeRelation, {
  graph: nativeGraph,
  recordIds: nativeRelation.record_ids,
});
assert.equal(graphDescriptor.metadata.graph.kind, "native-neighborhood-graph");
assert.equal(graphDescriptor.metadata.graph.mode, "native");
assert.equal(graphDescriptor.metadata.graph.relationId, nativeRelation.id);
assert.equal(graphDescriptor.metadata.graph.edgeCount, nativeGraph.edges.length);
assert.equal(graphDescriptor.source.nativeGraph, true);

console.log("Relation matrix picker contract passed.");
