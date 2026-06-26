#!/usr/bin/env node
/*
 * Cross-space composition acceptance gate.
 *
 * This checks that the public preview can be assembled from the reusable
 * CrossSpaceView command path with native paired-space evidence only.
 */

import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import { MetricVisualSurface } from "../src/metric-visual.js";
import { buildLinkedSelectionPresentation } from "../src/selection/index.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const EVIDENCE_PATH = "docs/examples/assets/cross-space-dependency/metric.visual.json";
const HERO_PATH = "visual/examples/cross-space-dependency-hero/index.html";
const SELECTED_ID = "obs-000";

const document = JSON.parse(await readFile(resolve(ROOT, EVIDENCE_PATH), "utf8"));
const heroSource = await readFile(resolve(ROOT, HERO_PATH), "utf8");
const surface = createCommandSurface(document);

MetricVisualSurface.prototype.showCrossSpace.call(surface, {
  viewId: "cross-space-paired-view",
  coordinateA: "event-log-landmark-3d",
  coordinateB: "process-curve-landmark-3d",
  dependenceProperty: "local-dependence-contribution",
  bridgeRelationId: "cross-space-dependence-bridge-relation",
  bridgeGraphId: "cross-space-dependence-bridges",
  preview: false,
});

const view = surface.views[0];
const descriptors = surface.descriptors;
const sourceRecords = findRecordDescriptor(descriptors, "source-space");
const targetRecords = findRecordDescriptor(descriptors, "target-space");
const bridge = descriptors.find((descriptor) => descriptor.kind === "paired-space-dependence");
const bridgeEvidence = bridge?.metadata?.bridgeEvidence;
const bridgeRelation = document.relations.find((relation) => relation.id === "cross-space-dependence-bridge-relation");
const presentation = buildLinkedSelectionPresentation({
  pair: {
    relationId: "cross-space-dependence-bridge-relation",
    pairSetId: view?.pairSetId,
    rowId: SELECTED_ID,
    columnId: SELECTED_ID,
  },
}, {
  document,
  layerDescriptors: descriptors,
});

assert.equal(surface.layerDescriptorOptions?.viewKind, "cross-space", "public command view kind should be cross-space");
assert.equal(view?.kind, "paired-space", "semantic view should remain the paired-space grammar");
assert.equal(view?.pairRecords?.length, 512, "view should keep all 512 exported paired records for selection");
assert.ok(sourceRecords, "descriptors should include a source-space record layer");
assert.ok(targetRecords, "descriptors should include a target-space record layer");
assert.equal(sourceRecords?.source?.pairedSpaceRole, "source-space");
assert.equal(targetRecords?.source?.pairedSpaceRole, "target-space");
assert.equal(bridge?.source?.relationId, "cross-space-dependence-bridge-relation", "bridge should name the exported relation");
assert.equal(bridge?.source?.graphId, "cross-space-dependence-bridges", "bridge should name the exported bridge graph id");
assert.equal(bridge?.metadata?.graph?.native, true, "bridge graph should declare native evidence");
assert.equal(bridgeEvidence?.schema, "metric.visual.cross_space_bridge_sampling.v1");
assert.equal(bridgeEvidence?.source, "exported-relation-values");
assert.equal(bridgeEvidence?.candidatePairCount, document.records.length);
assert.equal(bridgeEvidence?.exportedBridgeCount, bridgeRelation.values.length);
assert.equal(bridgeEvidence?.validBridgeCount, bridgeRelation.values.length);
assert.equal(bridgeEvidence?.renderedBridgeCount, bridgeRelation.values.length);
assert.equal(bridgeEvidence?.graphEdgeCount, document.records.length);
assert.equal(bridgeEvidence?.selectionGraph, "full-paired-record-ids");
assert.equal(bridgeEvidence?.nativeEvidenceOnly, true);
assert.equal(bridgeEvidence?.syntheticEvidence, false);
assert.equal(bridge.channels?.sourcePosition?.count, bridgeRelation.values.length, "visible bridge geometry should use exported bridge values");
assert.equal(bridge.channels?.recordId?.count, document.records.length, "record id channel should preserve full paired selection set");
assert.equal(bridge.metadata?.visibleEdgeCount, bridgeRelation.values.length);
assert.equal(bridge.metadata?.edgeCount, document.records.length);
assert.ok(Array.isArray(bridge.metadata?.graph?.visibleEdgeIds), "visible bridge ids should be explicit");
assert.equal(bridge.metadata.graph.visibleEdgeIds.length, bridgeRelation.values.length);
assert.equal(presentation.recordFeatures.length, 2, "pair selection should identify records in both spaces");
assert.equal(presentation.pairedSpaceBridges.length, 1, "pair selection should identify paired bridge evidence");
assert.equal(presentation.pairedSpaceBridges[0].rowId, SELECTED_ID);
assert.equal(presentation.pairedSpaceBridges[0].columnId, SELECTED_ID);
assert.equal(presentation.pairedSpaceBridges[0].selectionMatch.kind, "pair");
assert.ok(/docs\/examples\/assets\/cross-space-dependency\/metric\.visual\.json/.test(heroSource), "hero should load the native evidence asset");
assert.ok(/\.showCrossSpace\(/.test(heroSource), "hero should use the public showCrossSpace command");
assert.equal(pageLocalRendererFindings(heroSource).length, 0, "hero should not include page-local bridge rendering or synthetic evidence");

console.log(JSON.stringify({
  ok: true,
  publicViewKind: surface.layerDescriptorOptions.viewKind,
  semanticViewKind: view.kind,
  descriptorCount: descriptors.length,
  pairCount: view.pairRecords.length,
  visibleBridgeCount: bridgeEvidence.renderedBridgeCount,
  exportedBridgeCount: bridgeEvidence.exportedBridgeCount,
}, null, 2));

function findRecordDescriptor(descriptors, role) {
  return descriptors.find((descriptor) => (
    descriptor.source?.pairedSpaceRole === role
    && descriptor.channels?.recordId?.count > 0
    && (descriptor.primitive === "InstancedPointLayer" || descriptor.primitive === "InstancedGlyphLayer")
  ));
}

function pageLocalRendererFindings(source) {
  const patterns = [
    ["RelationEdgeLayer import", /\bRelationEdgeLayer\b/],
    ["manual WebGL/canvas context", /\.getContext\s*\(\s*["'`](?:webgl2?|2d)["'`]\s*\)/],
    ["manual bridge draw helper", /\b(?:draw|render|paint)(?:Dependence|CrossSpace|Bridge|Band|Edge)s?\b/],
    ["synthetic evidence literal", /\b(?:synthetic|mock|fake)\b/i],
    ["native evidence collection mutation", /document_\s*(?:\.\s*|\[\s*["'`])(?:records|relations|graphs|coordinates|properties|views)(?:\s*["'`]\s*\])?\s*=/],
    ["native relation value rewrite", /\brelations?\b[\s\S]{0,120}\bvalues\b\s*=/],
  ];
  return patterns
    .filter(([, pattern]) => pattern.test(source))
    .map(([label]) => label);
}

function createCommandSurface(document) {
  return {
    document,
    setup: {
      stage: { grounding: { groundY: -0.58 } },
    },
    views: [],
    descriptors: [],
    _setLayerDescriptors(descriptors, layerOptions) {
      this.descriptors = descriptors;
      this.layerDescriptorOptions = layerOptions;
      return this;
    },
    configurePreview(previewOptions) {
      this.previewOptions = previewOptions;
      return this;
    },
  };
}
