#!/usr/bin/env node

import assert from "node:assert/strict";
import { createRelationMatrixPicker, pickRelationMatrixCell } from "../src/relational/index.js";

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

assert.throws(
  () => createRelationMatrixPicker({ metadata: { matrix: { kind: "dense-relation-matrix", size: 0 } } }),
  /positive integer matrix size/,
);

console.log("Relation matrix picker contract passed.");
