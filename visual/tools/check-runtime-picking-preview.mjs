#!/usr/bin/env node

import assert from "node:assert/strict";
import { readFileSync } from "node:fs";
import { VisualSpace } from "../src/data/index.js";
import { MetricVisualRuntime } from "../src/runtime/runtime.js";
import { PickingRegistry } from "../src/picking/index.js";
import {
  buildMetricPairPreview,
  buildMetricRecordPreview,
} from "../src/interaction/record-preview.js";

const document = {
  schema: "metric.visual.v1",
  datasets: [{ id: "toy", title: "Toy" }],
  records: [
    { id: "a", dataset_id: "toy", record_type: "vector", label: "Alpha", payload: { values: [1, 2, 3], features: { group: "left" } } },
    { id: "b", dataset_id: "toy", record_type: "vector", label: "Beta", payload: { values: [4, 5, 6] } },
    { id: "c", dataset_id: "toy", record_type: "vector", label: "Gamma", payload: { values: [7, 8, 9] } },
  ],
  relations: [{
    id: "toy-distance",
    dataset_id: "toy",
    name: "Toy exported distance",
    relation_type: "distance",
    value_type: "number",
    record_ids: ["a", "b", "c"],
    storage: "dense",
    values: [
      0, 1, 2,
      1, 0, 3,
      2, 3, 0,
    ],
  }],
  spaces: [],
  properties: [{
    id: "pair-note",
    dataset_id: "toy",
    target_type: "pair",
    name: "source",
    values: [{ relation_id: "toy-distance", row_id: "a", column_id: "b", value: "exported" }],
  }],
  graphs: [],
  coordinates: [],
  timelines: [],
  views: [],
};

const matrix = {
  kind: "dense-relation-matrix",
  size: 3,
  width: 3,
  height: 3,
  recordIds: ["a", "b", "c"],
  values: new Float32Array([
    0, 1, 2,
    1, 0, 3,
    2, 3, 0,
  ]),
  present: new Uint8Array(9).fill(1),
};

const runtime = Object.create(MetricVisualRuntime.prototype);
runtime.events = new Map();
runtime.warnings = [];
runtime.document = document;
runtime.visualSpace = VisualSpace.fromDocument(document, { validate: false });
runtime.canvas = {
  getBoundingClientRect() {
    return { left: 0, top: 0, width: 200, height: 160 };
  },
};
runtime.renderer = {
  loop: { running: false },
  size: { width: 200, height: 160, drawingBufferWidth: 200, drawingBufferHeight: 160, pixelRatio: 1 },
};
runtime.camera = {
  updateMatrices() {},
  projectToPixel(position, out = {}) {
    out.x = position[0];
    out.y = position[1];
    out.depth = position[2] || 0;
    out.visible = true;
    return out;
  },
};
runtime.layers = [];
runtime.viewDescriptors = [];
runtime.layerDescriptors = [
  {
    id: "points",
    primitive: "InstancedGlyphLayer",
    channels: {
      recordId: { data: ["a", "b", "c"], size: 1 },
      position: { data: new Float32Array([10, 10, 0, 120, 10, 0, 180, 140, 0]), size: 3 },
    },
  },
  {
    id: "matrix",
    primitive: "RelationMatrixLayer",
    source: { relationId: "toy-distance" },
    geometry: { rect: [0.5, 0, 0.5, 0.5] },
    metadata: { matrix },
  },
  {
    id: "edges",
    primitive: "RelationEdgeLayer",
    source: { relationId: "toy-distance", graphId: "toy-graph" },
    channels: {
      sourcePosition: { data: new Float32Array([10, 120, 0]), size: 3 },
      targetPosition: { data: new Float32Array([120, 120, 0]), size: 3 },
      relationValue: { data: new Float32Array([1]), size: 1 },
    },
    metadata: {
      graph: {
        id: "toy-graph",
        relationId: "toy-distance",
        edges: [{ source: "a", target: "b", sourceIndex: 0, targetIndex: 1, value: 1 }],
      },
    },
  },
];
runtime.layerState = { status: "ready", descriptors: runtime.layerDescriptors.length, instances: 0, warning: null, errors: [] };
runtime.selection = { recordId: null, pair: null, record: null, preview: null, source: null, pickSource: null };
runtime.focusTarget = null;
runtime.hoverFocusOptions = { enabled: false };
runtime.hoverFocusState = { target: null };
runtime.inspectionOptions = {
  enabled: true,
  hover: true,
  click: true,
  gpu: true,
  relationMatrix: true,
  graph: true,
  cpuFallback: true,
  thresholdPx: 20,
  edgeThresholdPx: 8,
  clearOnMiss: false,
  selectOnClick: true,
  maxCandidates: 100,
};
runtime.inspectionState = { source: "none", hover: null, selection: null, lastResult: null };
runtime.pickingRegistry = new PickingRegistry();
runtime.pickingIndex = { recordPoints: [], relationMatrixPickers: [], graphEdges: [], gpuLayerCount: 0, availableSources: [] };
runtime.postprocess = { getState: () => null };
runtime.disposed = false;
runtime.warnings = [];
runtime.emit = () => runtime;
runtime.requestRender = () => runtime;
runtime.setFocusTarget = function setFocusTarget(target) {
  this.focusTarget = target;
  return this;
};

runtime.refreshPickingIndex();
assert.deepEqual(runtime.pickingIndex.availableSources, ["relation-matrix-picking", "graph-picking", "cpu-fallback"]);

let result = runtime.inspectAt({ x: 10, y: 10 }, { select: true, source: "check-cpu" });
assert.equal(result.source, "cpu-nearest");
let state = runtime.getState();
assert.equal(state.selectedRecordId, "a");
assert.equal(state.selectedRecord.label, "Alpha");
assert.equal(state.selectedRecordPreview.kind, "record");
assert.equal(state.selectedRecordPreview.recordId, "a");
assert.equal(state.selectedRecordPreview.payloadSnippet.find((field) => field.label === "group").value, "left");
assert.equal(state.selectionPreview.kind, "record");
assert.equal(state.inspection.selection.preview.kind, "record");

result = runtime.inspectAt({ x: 150, y: 10 }, { select: true, source: "check-matrix" });
assert.equal(result.source, "relation-matrix-picking");
state = runtime.getState();
assert.equal(state.selectedPair.rowId, "a");
assert.equal(state.selectedPair.columnId, "b");
assert.equal(state.selectionPickSource, "relation-matrix-picking");
assert.equal(state.selectedPairPreview.kind, "pair");
assert.equal(state.selectedPairPreview.pair.relationId, "toy-distance");
assert.equal(state.selectedPairPreview.pair.relationName, "Toy exported distance");
assert.equal(state.selectedPairPreview.pair.rowId, "a");
assert.equal(state.selectedPairPreview.pair.columnId, "b");
assert.equal(state.selectedPairPreview.pair.value, 1);
assert.equal(state.selectedPairPreview.pair.properties.find((field) => field.label === "source").value, "exported");
assert.equal(state.inspection.selection.preview.pair.relationId, "toy-distance");

result = runtime.inspectAt({ x: 50, y: 120 }, { select: true, source: "check-graph" });
assert.equal(result.source, "graph-picking");
state = runtime.getState();
assert.equal(state.selectedPair.rowId, "a");
assert.equal(state.selectedPair.columnId, "b");
assert.equal(state.selectedPairPreview.pair.value, 1);
assert.equal(state.selectedPairPreview.pair.properties.find((field) => field.label === "source").value, "exported");
assert.equal(state.inspection.graphPicking.edgeCount, 1);
assert.deepEqual(state.inspection.runtimeStateKeys, {
  selectedRecord: "selectedRecord",
  selectedRecordId: "selectedRecordId",
  selectedRecordPreview: "selectedRecordPreview",
  selectedPair: "selectedPair",
  selectedPairPreview: "selectedPairPreview",
  selectionPreview: "selectionPreview",
});

const recordPreview = buildMetricRecordPreview({ recordId: "a" }, { visualSpace: runtime.visualSpace, document });
assert.equal(recordPreview.title, "Alpha");
assert.equal(recordPreview.fields.find((field) => field.label === "group").value, "left");

const pairPreview = buildMetricPairPreview(runtime.getState().selectedPair, { visualSpace: runtime.visualSpace, document });
assert.equal(pairPreview.fields.find((field) => field.label === "value").value, "1.0000");
assert.equal(pairPreview.fields.find((field) => field.label === "relation id").value, "toy-distance");
assert.equal(pairPreview.fields.find((field) => field.label === "row id").value, "a");
assert.equal(pairPreview.fields.find((field) => field.label === "column id").value, "b");
assert.equal(pairPreview.pair.properties.find((field) => field.label === "source").value, "exported");

const nativeCrossSpaceDocument = JSON.parse(readFileSync(new URL("../../docs/examples/assets/cross-space-dependency/metric.visual.json", import.meta.url), "utf8"));
runtime.document = nativeCrossSpaceDocument;
runtime.visualSpace = VisualSpace.fromDocument(nativeCrossSpaceDocument);

runtime.selectRecord("obs-00", { source: "native-record-preview", focus: false });
state = runtime.getState();
assert.equal(state.selectedRecordId, "obs-00");
assert.equal(state.selectedRecordPreview.kind, "record");
assert.equal(state.selectedRecordPreview.payloadKind, "composed");
assert.equal(state.selectedRecordPreview.linkedViews.length, 2);
assert.equal(
  state.selectedRecordPreview.record.properties.find((field) => field.label === "local distance-profile alignment").value,
  0.986754245804,
);
assert.equal(state.selectionPreview.recordId, "obs-00");

runtime.selectPair({
  relationId: "event-log-edit-distance",
  rowId: "obs-00",
  columnId: "obs-01",
}, { source: "native-pair-preview" });
state = runtime.getState();
assert.equal(state.selectedPairPreview.kind, "pair");
assert.equal(state.selectedPairPreview.pair.value, 6);
assert.equal(
  state.selectedPairPreview.pair.properties.find((field) => field.label === "pair distance-profile z-product contribution").value,
  0.705432747484,
);
assert.equal(state.selectedPairPreview.records.row.payloadKind, "composed");
assert.equal(state.selectedPairPreview.records.column.payloadKind, "composed");
assert.equal(state.selectionPreview.pair.relationId, "event-log-edit-distance");

runtime.selectPair({
  relationId: null,
  rowId: "obs-00",
  columnId: "obs-01",
}, { source: "native-linked-pair-preview" });
state = runtime.getState();
assert.equal(state.selectedPairPreview.pair.relationId, null);
assert.equal(state.selectedPairPreview.pair.present, true);
assert.equal(
  state.selectedPairPreview.pair.properties.find((field) => field.label === "pair distance-profile z-product contribution").value,
  0.705432747484,
);
assert.equal(state.selectionPreview.pair.relationId, null);

let disposedPickingTarget = false;
runtime.pickingPass = { name: "test-picking-pass" };
runtime.pickingTarget = { dispose: () => { disposedPickingTarget = true; } };
runtime.disposePickingResources();
assert.equal(disposedPickingTarget, true);
assert.equal(runtime.pickingPass, null);
assert.equal(runtime.pickingTarget, null);
assert.deepEqual(runtime.pickingIndex.availableSources, []);

console.log("Runtime picking and evidence preview contract passed.");
