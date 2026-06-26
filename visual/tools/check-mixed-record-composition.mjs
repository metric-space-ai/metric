#!/usr/bin/env node

import { readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import { MetricVisualSurface } from "../src/index.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const MIXED_ASSET = resolve(ROOT, "docs", "examples", "assets", "mixed-records", "metric.visual.json");
const HERO_PAGE = resolve(ROOT, "visual", "examples", "mixed-record-hero", "index.html");

function fail(message, details = {}) {
  console.error(JSON.stringify({ ok: false, message, details }, null, 2));
  process.exit(1);
}

function assert(message, ok, details = {}) {
  if (!ok) fail(message, details);
}

class FakeRuntime {
  constructor() {
    this.descriptors = [];
    this.layers = [];
    this.setCalls = 0;
  }

  setLayerDescriptors(descriptors) {
    this.setCalls += 1;
    this.descriptors = descriptors.slice();
    this.layers = this.descriptors.map((descriptor) => ({ id: descriptor.id, descriptor }));
  }

  renderOnce() {}

  start() {
    this.started = true;
  }

  getState() {
    return {
      layerInstanceCount: this.layers.length,
      layerState: { count: this.layers.length },
    };
  }

  on() {
    return () => {};
  }
}

const document_ = JSON.parse(await readFile(MIXED_ASSET, "utf8"));
const heroSource = await readFile(HERO_PAGE, "utf8");
const runtime = new FakeRuntime();
const surface = new MetricVisualSurface({
  document: document_,
  canvas: {
    getBoundingClientRect: () => ({ left: 0, top: 0, width: 1280, height: 820 }),
    addEventListener() {},
    removeEventListener() {},
  },
  runtime,
  setup: {
    stage: { grounding: { groundY: -0.58 } },
    style: {
      setStyleMotion() {},
      detachStyleMotion() {},
    },
  },
  options: {},
});

surface.showMixedRecords({
  coordinateId: "mixed-finite-records-family-severity-3d",
  glyphBy: "record_type",
  labels: "record_type",
  relationId: "mixed-finite-records-composite-metric",
  crossTypeProperty: "record_type",
  crossTypeRelations: true,
  topK: 8,
  preview: false,
  pointSize: 2.05,
});

const diagnostics = surface.getDiagnostics();
const command = diagnostics.commands.at(-1);
const descriptors = runtime.descriptors;
const primitives = descriptors.map((descriptor) => descriptor.primitive);
const glyph = descriptors.find((descriptor) => descriptor.primitive === "InstancedGlyphLayer");
const edges = descriptors.find((descriptor) => descriptor.primitive === "RelationEdgeLayer");
const projection = descriptors.find((descriptor) => descriptor.primitive === "GroundProjectionLayer");
const labels = descriptors.find((descriptor) => descriptor.primitive === "BillboardLabelLayer");
const nativeGraph = document_.graphs.find((graph) => graph.id === "mixed-finite-records-knn") || document_.graphs[0];
const exportedRecordTypes = new Set(document_.records.map((record) => String(record.record_type || record.type || "")).filter(Boolean));
const glyphRecordTypes = new Set(Array.from(glyph?.channels?.recordType?.array || []).filter(Boolean).map(String));
const payloadFamilies = new Set(document_.records.map((record) => String(record.payload?.kind || "")).filter(Boolean));

assert("selected view kind is mixed-records",
  command?.selectedViewKind === "mixed-records" && surface.views[0]?.kind === "mixed-records",
  { command, viewKinds: surface.views.map((view) => view.kind) });

for (const primitive of ["InstancedGlyphLayer", "RelationEdgeLayer", "GroundProjectionLayer"]) {
  assert(`descriptors include ${primitive}`, primitives.includes(primitive), { primitives });
}

assert("typed glyphs represent at least four exported record types",
  exportedRecordTypes.size >= 4
    && glyphRecordTypes.size >= 4
    && Array.from(exportedRecordTypes).every((type) => glyphRecordTypes.has(type)),
  { exportedRecordTypes: Array.from(exportedRecordTypes).sort(), glyphRecordTypes: Array.from(glyphRecordTypes).sort() });

assert("typed glyph descriptor exposes deterministic type color encoding",
  glyph?.metadata?.typeColorEncoding?.length >= 4
    && new Set(glyph.metadata.typeColorEncoding.map((entry) => JSON.stringify(entry.color))).size >= 4,
  glyph?.metadata?.typeColorEncoding);

assert("type labels orient the mixed-record scene",
  labels?.metadata?.role === "type-labels"
    && labels?.metadata?.labelAnchorMode === "mixed-record-type-centroids"
    && labels?.metadata?.labelCount >= 4
    && labels.labels?.some((label) => /text code/i.test(label.text || ""))
    && labels.labels?.some((label) => /histogram spectrum/i.test(label.text || "")),
  { labels: labels?.labels, metadata: labels?.metadata });

assert("ground projection is typed by record structure",
  projection?.metadata?.primaryGrammar === "typed-record-ground-projection"
    && projection?.channels?.color?.source === "mixed-record-category-order-palette"
    && projection?.channels?.size?.count === document_.records.length,
  { projection: summarizeDescriptor(projection) });

assert("relation edges come from native relation evidence",
  edges?.metadata?.graph?.native === true
    && edges?.metadata?.graph?.edgeCount === nativeGraph.edges.length
    && edges?.source?.nativeGraph === true
    && edges?.source?.relationId === "mixed-finite-records-composite-metric"
    && edges?.metadata?.selectionModel?.pairSource === "native-graph-evidence",
  { source: edges?.source, metadata: edges?.metadata });

assert("relation edge composition encodes native cross-type structure without synthetic topology",
  edges?.channels?.edgeTypePair?.count === nativeGraph.edges.length
    && edges?.channels?.edgeEmphasis?.count === nativeGraph.edges.length
    && edges?.channels?.color?.source === "mixed-record-native-edge-type-pair"
    && edges?.metadata?.nativeEvidence?.sourceEdgeCount === nativeGraph.edges.length
    && edges?.metadata?.relationStructureEncoding?.typePairCount >= 4,
  {
    channels: {
      edgeTypePair: summarizeChannel(edges?.channels?.edgeTypePair),
      edgeEmphasis: summarizeChannel(edges?.channels?.edgeEmphasis),
      color: summarizeChannel(edges?.channels?.color),
    },
    relationStructureEncoding: edges?.metadata?.relationStructureEncoding,
  });

assert("native mixed evidence is not synthetic",
  document_.provenance?.native_export === true
    && document_.provenance?.synthetic !== true
    && document_.provenance?.synthetic_js !== true,
  document_.provenance);

assert("hover previews use the shared native evidence resolver path",
  /createMetricEvidencePreviewResolver/.test(heroSource)
    && /previewOptions/.test(heroSource)
    && !/function\s+create.*Preview|function\s+render.*Preview/.test(heroSource),
  {});

assert("hero page uses only the native mixed-record evidence target",
  /docs\/examples\/assets\/mixed-records\/metric\.visual\.json/.test(heroSource)
    && !/evidence\.json/.test(heroSource)
    && !/synthetic|fixture/i.test(heroSource),
  {});

assert("hero page does not define a local renderer or descriptor factory",
  /\.showMixedRecords\(/.test(heroSource)
    && !/new\s+MixedRecordView\b|createRecordGlyphGrammar\b|createRelationGraphEdgeLayerDescriptor\b|drawArrays\s*\(|getContext\s*\(/.test(heroSource),
  {});

console.log(JSON.stringify({
  ok: true,
  asset: "docs/examples/assets/mixed-records/metric.visual.json",
  recordCount: document_.records.length,
  recordTypes: Array.from(exportedRecordTypes).sort(),
  payloadFamilies: Array.from(payloadFamilies).sort(),
  descriptorPrimitives: primitives,
  selectedViewKind: command?.selectedViewKind,
  edgeCount: edges?.metadata?.graph?.edgeCount,
  typePairCount: edges?.metadata?.relationStructureEncoding?.typePairCount,
  previewPath: "createMetricEvidencePreviewResolver",
}, null, 2));

function summarizeDescriptor(descriptor) {
  return {
    primitive: descriptor?.primitive,
    metadata: descriptor?.metadata,
    channels: Object.fromEntries(Object.entries(descriptor?.channels || {}).map(([key, channel]) => [key, summarizeChannel(channel)])),
  };
}

function summarizeChannel(channel) {
  return {
    semantic: channel?.semantic,
    count: channel?.count,
    itemSize: channel?.itemSize,
    source: channel?.source,
  };
}
