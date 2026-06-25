#!/usr/bin/env node

import assert from "node:assert/strict";
import { readFileSync } from "node:fs";
import { VisualSpace } from "../src/data/index.js";
import { MetricVisualRuntime } from "../src/runtime/runtime.js";
import { CrossSpaceView } from "../src/views/index.js";

const document = JSON.parse(readFileSync(new URL("../../docs/examples/assets/cross-space-dependency/metric.visual.json", import.meta.url), "utf8"));
const visualSpace = VisualSpace.fromDocument(document);
const SELECTED_ID = "obs-000";
const view = CrossSpaceView.fromVisualSpace(document, {
  id: "linked-selection-check",
  leftCoordinateId: "event-log-landmark-3d",
  rightCoordinateId: "process-curve-landmark-3d",
  scalarProperty: "local-distance-profile-alignment",
  targetRadius: 1.05,
});

const runtime = Object.create(MetricVisualRuntime.prototype);
runtime.events = new Map();
runtime.warnings = [];
runtime.document = document;
runtime.visualSpace = visualSpace;
runtime.viewDescriptors = [];
runtime.layerDescriptors = view.toLayerDescriptors();
runtime.layers = [];
runtime.layerState = { status: "ready", descriptors: runtime.layerDescriptors.length, instances: 0, warning: null, errors: [] };
runtime.selection = { recordId: null, pair: null, record: null, preview: null, presentation: null, source: null, pickSource: null };
runtime.focusTarget = null;
runtime.renderer = { loop: { running: false } };
runtime.postprocess = { getState: () => null };
runtime.hoverFocusOptions = { enabled: false };
runtime.hoverFocusState = { target: null };
runtime.inspectionOptions = { enabled: true };
runtime.inspectionState = { source: "none", hover: null, selection: null, lastResult: null };
runtime.pickingIndex = {
  recordPoints: [],
  relationMatrixPickers: [],
  graphEdges: [],
  gpuLayerCount: 0,
  availableSources: [],
};
runtime.disposed = false;
runtime.emit = () => runtime;
runtime.requestRender = () => runtime;

runtime.selectRecord(SELECTED_ID, { source: "linked-selection-record", focus: false });
let state = runtime.getState();
assert.equal(state.selectedRecordId, SELECTED_ID);
assert.equal(state.selectionPresentation.kind, "record");
assert.equal(state.selectionPresentation.recordFeatures.length, 2);
assert.deepEqual(
  state.selectionPresentation.recordFeatures.map((feature) => feature.pairedSpaceRole).sort(),
  ["source-space", "target-space"],
);
assert.equal(state.selectionPresentation.pairedSpaceBridges.length, 1);
assert.equal(state.selectionPresentation.pairedSpaceBridges[0].rowId, SELECTED_ID);
assert.equal(state.selectionPresentation.pairedSpaceBridges[0].columnId, SELECTED_ID);
assert.equal(state.selectionPresentation.pairedSpaceBridges[0].selectionMatch.kind, "record-endpoint");
assert.equal(state.selectionPresentation.pairedSpaceBridges[0].pairSetId, "linked-selection-check:linked-pairs");
assert.equal(state.selectionPresentation.graphEdges.length, 0);

runtime.selectPair({
  relationId: "cross-space-dependence-bridge-relation",
  pairSetId: "linked-selection-check:linked-pairs",
  rowId: SELECTED_ID,
  columnId: SELECTED_ID,
}, { source: "linked-selection-pair" });
state = runtime.getState();
assert.equal(state.selectedPair.rowId, SELECTED_ID);
assert.equal(state.selectedPair.columnId, SELECTED_ID);
assert.equal(state.selectionPresentation.kind, "pair");
assert.equal(state.selectionPresentation.records.length, 1);
assert.deepEqual(state.selectionPresentation.records[0].roles, ["row", "source", "column", "target"]);
assert.equal(state.selectionPresentation.recordFeatures.length, 2);
assert.equal(state.selectionPresentation.pairedSpaceBridges.length, 1);
assert.equal(state.selectionPresentation.pairedSpaceBridges[0].selectionMatch.kind, "pair");
assert.equal(state.selectionPresentation.pairedSpaceBridges[0].sourceSpaceId, "event-log-space");
assert.equal(state.selectionPresentation.pairedSpaceBridges[0].targetSpaceId, "process-curve-space");
assert.equal(state.selectionPresentation.highlight.pairedSpaceBridgeIds.length, 1);
assert.equal(state.selectionFeatures.length, state.selectionPresentation.counts.features);

console.log("Linked selection presentation contract passed.");
