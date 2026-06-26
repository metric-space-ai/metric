#!/usr/bin/env node
/*
 * Cross-space linked-selection contract gate.
 *
 * This verifies that showCrossSpace routes native evidence through a reusable
 * paired-space semantic view and exposes bridge metadata that runtime picking
 * can interpret as paired evidence. It does not compute dependence statistics.
 */

import { readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import { MetricVisualSurface } from "../src/metric-visual.js";
import { CrossSpaceView } from "../src/views/CrossSpaceView.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");

const failures = [];

const nativeCrossSpace = await readJson("docs/examples/assets/cross-space-dependency/metric.visual.json");
const options = {
  coordinateA: "event-log-landmark-3d",
  coordinateB: "process-curve-landmark-3d",
  dependenceProperty: "local-dependence-contribution",
  preview: false,
};

const directView = CrossSpaceView.fromVisualSpace(nativeCrossSpace, options);
const directDescriptors = directView.toLayerDescriptors();
checkViewContract("CrossSpaceView.fromVisualSpace", directView, directDescriptors);

const surface = createCommandSurface(nativeCrossSpace);
MetricVisualSurface.prototype.showCrossSpace.call(surface, options);
checkSurfaceContract(surface);
checkViewContract("MetricVisualSurface.showCrossSpace", surface.views[0], surface.descriptors);
checkRuntimePipeline(surface);

const ok = failures.length === 0;
console.log(JSON.stringify({
  ok,
  viewKind: surface.views[0]?.kind || null,
  descriptorCount: surface.descriptors.length,
  pairCount: surface.views[0]?.pairRecords?.length || 0,
  bridge: describeBridge(surface.descriptors),
  failures,
}, null, 2));

if (!ok) process.exitCode = 1;

function checkSurfaceContract(surface) {
  assert("showCrossSpace installs descriptors with command source", surface.layerDescriptorOptions?.source === "showCrossSpace", surface.layerDescriptorOptions);
  assert("showCrossSpace keeps public view kind", surface.layerDescriptorOptions?.viewKind === "cross-space", surface.layerDescriptorOptions);
  assert("showCrossSpace stores one paired-space semantic view", surface.views.length === 1 && surface.views[0] instanceof CrossSpaceView, {
    viewCount: surface.views.length,
    viewKind: surface.views[0]?.kind,
  });
}

function checkViewContract(label, view, descriptors) {
  assert(`${label}: view kind is paired-space`, view?.kind === "paired-space", view?.toViewDescriptor?.());
  assert(`${label}: left side exists`, view?.left?.kind === "metric-space", view?.left);
  assert(`${label}: right side exists`, view?.right?.kind === "metric-space", view?.right);
  assert(`${label}: pair records are exported ids`, view?.pairRecords?.length === nativeCrossSpace.records.length, {
    pairCount: view?.pairRecords?.length,
    recordCount: nativeCrossSpace.records.length,
  });
  assert(`${label}: contract exposes linked selection`, view?.metadata?.contract?.linkedSelection?.kind === "paired-space-linked-selection", view?.metadata);
  assert(`${label}: contract exposes dependence property`, view?.metadata?.contract?.dependence?.propertyId === options.dependenceProperty, view?.metadata?.contract);
  assert(`${label}: contract exposes native bridge relation`, view?.metadata?.contract?.dependence?.bridgeRelationId === "cross-space-dependence-bridge-relation", view?.metadata?.contract?.dependence);
  assert(`${label}: contract exposes native bridge graph`, view?.metadata?.contract?.dependence?.bridgeGraphId === "cross-space-dependence-bridges", view?.metadata?.contract?.dependence);
  assert(`${label}: contract carries native global dependence`, Number.isFinite(Number(view?.metadata?.contract?.dependence?.global?.statistic)), view?.metadata?.contract?.dependence);

  const leftPoint = descriptors.find((descriptor) => descriptor.source?.pairedSpaceRole === "source-space" && descriptor.channels?.recordId);
  const rightPoint = descriptors.find((descriptor) => descriptor.source?.pairedSpaceRole === "target-space" && descriptor.channels?.recordId);
  assert(`${label}: left record layer is marked as paired-space side`, Boolean(leftPoint), sideDescriptorSummary(descriptors));
  assert(`${label}: right record layer is marked as paired-space side`, Boolean(rightPoint), sideDescriptorSummary(descriptors));
  assert(`${label}: record layers keep record selection as linked-pair evidence`, leftPoint?.metadata?.selectionModel?.linkedPairInterpretation?.includes("paired observation"), leftPoint?.metadata?.selectionModel);

  const bridge = bridgeDescriptor(descriptors);
  assert(`${label}: bridge descriptor exists`, Boolean(bridge), descriptors.map((descriptor) => ({ id: descriptor.id, kind: descriptor.kind, primitive: descriptor.primitive })));
  if (!bridge) return;

  assert(`${label}: bridge uses runtime edge primitive`, bridge.primitive === "RelationEdgeLayer", bridge);
  assert(`${label}: bridge is paired-space dependence`, bridge.kind === "paired-space-dependence", bridge);
  assert(`${label}: bridge source names native bridge relation`, bridge.source?.relationId === "cross-space-dependence-bridge-relation", bridge.source);
  assert(`${label}: bridge source names native bridge graph`, bridge.source?.graphId === "cross-space-dependence-bridges", bridge.source);
  assert(`${label}: bridge graph names native bridge relation`, bridge.metadata?.graph?.relationId === "cross-space-dependence-bridge-relation", bridge.metadata?.graph);
  assert(`${label}: bridge responds to record and pair selection`, sameMembers(bridge.metadata?.selectionModel?.respondsTo, ["record", "pair"]), bridge.metadata?.selectionModel);
  assert(`${label}: bridge exposes source/target id channels`, hasChannels(bridge, ["recordId", "sourceId", "targetId", "rowId", "columnId", "edgeId"]), Object.keys(bridge.channels || {}));
  assert(`${label}: bridge source ids align with target ids for this native pairing`, sameArray(bridge.channels.sourceId.array, bridge.channels.targetId.array), {
    source: bridge.channels.sourceId.array.slice(0, 4),
    target: bridge.channels.targetId.array.slice(0, 4),
  });
  assert(`${label}: bridge pair count matches view pairs`, bridge.channels.recordId.count === view.pairRecords.length, {
    bridgeCount: bridge.channels.recordId.count,
    pairCount: view.pairRecords.length,
  });
  assert(`${label}: bridge graph metadata backs CPU/runtime pair picking`, bridge.metadata?.graph?.edges?.length === view.pairRecords.length, bridge.metadata?.graph?.diagnostics);

  const firstEdge = bridge.metadata?.graph?.edges?.[0];
  assert(`${label}: graph edge has pair endpoints`, Boolean(firstEdge?.rowId && firstEdge?.columnId && firstEdge?.source && firstEdge?.target), firstEdge);
  assert(`${label}: graph edge preserves source/target space ids`, firstEdge?.sourceSpaceId === "event-log-space" && firstEdge?.targetSpaceId === "process-curve-space", firstEdge);
  assert(`${label}: graph edge carries exported local dependence value`, Number.isFinite(Number(firstEdge?.value)), firstEdge);
  assert(`${label}: graph edge properties name exported dependence property`, firstEdge?.properties?.[0]?.id === options.dependenceProperty, firstEdge?.properties);
}

function checkRuntimePipeline(surface) {
  assert("showCrossSpace uses descriptor pipeline", surface.setLayerDescriptorCalls === 1, {
    calls: surface.setLayerDescriptorCalls,
  });
  assert("showCrossSpace routes through preview configuration", surface.previewConfigured === true, surface.previewOptions);
}

function bridgeDescriptor(descriptors) {
  return descriptors.find((descriptor) => descriptor.kind === "paired-space-dependence");
}

function describeBridge(descriptors) {
  const bridge = bridgeDescriptor(descriptors);
  if (!bridge) return null;
  return {
    id: bridge.id,
    primitive: bridge.primitive,
    pairCount: bridge.channels?.recordId?.count || 0,
    channelNames: Object.keys(bridge.channels || {}),
    selectionModel: bridge.metadata?.selectionModel || null,
    relationId: bridge.source?.relationId || bridge.metadata?.graph?.relationId || null,
    graphId: bridge.source?.graphId || bridge.metadata?.graph?.id || null,
  };
}

function sideDescriptorSummary(descriptors) {
  return descriptors.map((descriptor) => ({
    id: descriptor.id,
    kind: descriptor.kind,
    primitive: descriptor.primitive,
    pairedSpaceRole: descriptor.source?.pairedSpaceRole || null,
    hasRecordId: Boolean(descriptor.channels?.recordId),
  }));
}

function hasChannels(descriptor, names) {
  return names.every((name) => descriptor.channels?.[name]?.count > 0);
}

function sameArray(a = [], b = []) {
  return a.length === b.length && a.every((value, index) => String(value) === String(b[index]));
}

function sameMembers(actual, expected) {
  if (!Array.isArray(actual)) return false;
  const set = new Set(actual.map(String));
  return expected.length === set.size && expected.every((value) => set.has(value));
}

function createCommandSurface(document) {
  return {
    document,
    setup: {
      stage: { grounding: { groundY: -0.58 } },
    },
    views: [],
    descriptors: [],
    setLayerDescriptorCalls: 0,
    _setLayerDescriptors(descriptors, layerOptions) {
      this.descriptors = descriptors;
      this.layerDescriptorOptions = layerOptions;
      this.setLayerDescriptorCalls += 1;
      return this;
    },
    setLayerDescriptors(descriptors, layerOptions) {
      return this._setLayerDescriptors(descriptors, layerOptions);
    },
    configurePreview(previewOptions) {
      this.previewOptions = previewOptions;
      this.previewConfigured = true;
      return this;
    },
  };
}

async function readJson(path) {
  return JSON.parse(await readFile(resolve(ROOT, path), "utf8"));
}

function assert(message, ok, details = undefined) {
  if (ok) return;
  if (details === undefined) failures.push(message);
  else failures.push(`${message}: ${JSON.stringify(details)}`);
}
