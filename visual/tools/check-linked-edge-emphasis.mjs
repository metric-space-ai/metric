#!/usr/bin/env node

import assert from "node:assert/strict";
import { RelationEdgeLayer } from "../src/layers/RelationEdgeLayer.js";
import { buildLinkedSelectionPresentation } from "../src/selection/index.js";

const graphDescriptor = {
  id: "graph-edges",
  primitive: "RelationEdgeLayer",
  source: { relationId: "toy-distance", graphId: "toy-graph" },
  channels: {
    sourcePosition: { data: new Float32Array([0, 0, 0, 1, 0, 0]), size: 3 },
    targetPosition: { data: new Float32Array([1, 0, 0, 2, 0, 0]), size: 3 },
    color: { data: new Float32Array([0.2, 0.3, 0.4, 0.5, 0.2, 0.3, 0.4, 0.5]), size: 4 },
  },
  metadata: {
    graph: {
      id: "toy-graph",
      relationId: "toy-distance",
      edges: [
        { id: "edge-a-b", source: "a", target: "b", value: 1 },
        { id: "edge-b-c", source: "b", target: "c", value: 2 },
      ],
    },
  },
};

const graphSelection = {
  pair: {
    relationId: "toy-distance",
    graphId: "toy-graph",
    rowId: "a",
    columnId: "b",
  },
};
graphSelection.presentation = buildLinkedSelectionPresentation(graphSelection, {
  layerDescriptors: [graphDescriptor],
});

const graphLayer = createUploadedLayer(graphDescriptor);
assert.deepEqual(Array.from(graphLayer.edgeEmphasis), [0, 0]);
assert.equal(graphLayer.selectionActive, false);

graphLayer.updatePickColorBuffer({ registry: createRegistry() });
const pickColorBuffer = graphLayer.buffers.pickColor;
graphLayer.setSelection(graphSelection);
assert.deepEqual(Array.from(graphLayer.edgeEmphasis), [1, 0]);
assert.deepEqual(Array.from(graphLayer.buffers.emphasis.data), [1, 1, 0, 0]);
assert.equal(graphLayer.selectionActive, true);
assert.equal(graphLayer.buffers.pickColor, pickColorBuffer);

const recordSelection = { recordId: "b" };
recordSelection.presentation = buildLinkedSelectionPresentation(recordSelection, {
  layerDescriptors: [graphDescriptor],
});
graphLayer.setSelection(recordSelection);
assert.deepEqual(Array.from(graphLayer.edgeEmphasis), [0.6800000071525574, 0.6800000071525574]);

const bridgeDescriptor = {
  id: "paired-bridge",
  primitive: "RelationEdgeLayer",
  source: {
    role: "dependence bridge",
    graphId: "pairset",
    pairSetId: "pairset",
  },
  channels: {
    edgeId: { data: ["bridge-0", "bridge-1"], size: 1 },
    sourceId: { data: ["obs-00", "obs-01"], size: 1 },
    targetId: { data: ["obs-00", "obs-02"], size: 1 },
    rowId: { data: ["obs-00", "obs-01"], size: 1 },
    columnId: { data: ["obs-00", "obs-02"], size: 1 },
    sourcePosition: { data: new Float32Array([0, 1, 0, 1, 1, 0]), size: 3 },
    targetPosition: { data: new Float32Array([0, 2, 0, 1, 2, 0]), size: 3 },
  },
  metadata: {
    role: "dependence bridge",
    primaryGrammar: "paired-space",
    graph: {
      kind: "paired-space-linked-pairs",
      id: "pairset",
      edges: [
        {
          id: "bridge-0",
          source: "obs-00",
          target: "obs-00",
          pairId: "pair-0",
          sourceSpaceId: "left-space",
          targetSpaceId: "right-space",
        },
        {
          id: "bridge-1",
          source: "obs-01",
          target: "obs-02",
          pairId: "pair-1",
          sourceSpaceId: "left-space",
          targetSpaceId: "right-space",
        },
      ],
    },
    selectionModel: {
      kind: "paired-space-linked-selection",
      pairSetId: "pairset",
    },
  },
};

const bridgeSelection = {
  pair: {
    pairSetId: "pairset",
    rowId: "obs-00",
    columnId: "obs-00",
  },
};
bridgeSelection.presentation = buildLinkedSelectionPresentation(bridgeSelection, {
  layerDescriptors: [bridgeDescriptor],
});

const bridgeLayer = createUploadedLayer(bridgeDescriptor);
bridgeLayer.setSelection(bridgeSelection);
assert.deepEqual(Array.from(bridgeLayer.edgeEmphasis), [1, 0]);
assert.deepEqual(Array.from(bridgeLayer.buffers.emphasis.data), [1, 1, 0, 0]);
assert.equal(bridgeLayer.selectionActive, true);

console.log("Linked edge emphasis contract passed.");

function createUploadedLayer(descriptor) {
  const layer = new RelationEdgeLayer(descriptor);
  layer.gl = createBufferOnlyGl();
  layer.capabilities = { instancing: { supported: false } };
  layer.upload();
  return layer;
}

function createRegistry() {
  return {
    size: 0,
    registerEdge() {
      this.size += 1;
      return this.size;
    },
  };
}

function createBufferOnlyGl() {
  let nextId = 0;
  return {
    ARRAY_BUFFER: 0x8892,
    STATIC_DRAW: 0x88e4,
    createBuffer() {
      nextId += 1;
      return { id: nextId, data: null, deleted: false };
    },
    bindBuffer(target, buffer) {
      this.boundBuffer = buffer;
    },
    bufferData(target, data) {
      if (!this.boundBuffer) return;
      this.boundBuffer.data = ArrayBuffer.isView(data) ? new data.constructor(data) : data;
    },
    deleteBuffer(buffer) {
      if (buffer) buffer.deleted = true;
    },
  };
}
