#!/usr/bin/env node

import { readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import { validateVisualDocument, VisualSpace } from "../src/data/index.js";
import { createMetricVisualDocumentDiagnostics } from "../src/metric-visual.js";
import {
  isExplicitNativeMetricVisualExport,
  isSyntheticMetricVisualEvidence,
} from "../src/data/provenance.js";
import { DynamicsView } from "../src/views/DynamicsView.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const EVIDENCE_RELATIVE = "docs/examples/assets/redif-metric-dynamics/metric.visual.json";
const PAGE_RELATIVE = "visual/examples/redif-metric-dynamics-preview/index.html";
const EVIDENCE_PATH = resolve(ROOT, EVIDENCE_RELATIVE);
const PAGE_PATH = resolve(ROOT, PAGE_RELATIVE);
const TIMELINE_ID = "redif-measure-path";
const TRANSPORT_PROPERTY_ID = "redif-terminal-transport-path";
const ENTROPY_PROPERTY_ID = "redif-terminal-entropy";
const RELATION_ID = "redif-source-metric";

const FORBIDDEN_PAGE_TOKENS = [
  ["direct runtime construction", /\bnew\s+MetricVisualRuntime\b/],
  ["runtime factory", /\bcreate(?:MetricVisual|VisualEngine)Runtime\b/],
  ["layer factory", /\bcreateLayerFromDescriptor\b/],
  ["manual VisualLayer construction", /\bnew\s+VisualLayer\b/],
  ["manual layer primitive descriptor", /\bprimitive\s*:\s*["'][A-Za-z0-9_-]+Layer["']/],
  ["manual channel descriptor", /\bchannels\s*:\s*\{/],
  ["manual descriptor conversion", /\.toLayerDescriptors\s*\(/],
  ["manual descriptor installation", /\.(?:set|add)LayerDescriptors\s*\(/],
  ["private descriptor installation", /\._setLayerDescriptors\s*\(/],
  ["manual WebGL renderer", /\bWebGLRenderer\b|\bEffectComposer\b|\bnew\s+MGL\.(?:Scene|PerspectiveCamera|WebGLRenderer)\b/],
  ["manual canvas render context", /\.getContext\s*\(/],
  ["manual animation loop", /\brequestAnimationFrame\s*\(/],
  ["page-local synthetic padding", /\b(?:pad|padding|synthetic|fake|dummy|placeholder)(?:Point|Record|Atom|Node|Position|Coordinate)s?\b/i],
  ["page-local metric computation", /\b(?:compute|calculate|derive|recompute|estimate|infer)(?:[A-Za-z0-9_$]*(?:Metric|Distance|Entropy|Embedding|Coordinate|Redif|Barycenter|Measure))/],
  ["page-local Redif operator", /\bredif_(?:remove|add|operator)\b|\bredifRemove\b|\bredifAdd\b|\bRedifOperator\b/],
];

async function main() {
  const [documentText, pageText] = await Promise.all([
    readFile(EVIDENCE_PATH, "utf8"),
    readFile(PAGE_PATH, "utf8"),
  ]);
  const document_ = JSON.parse(documentText);
  const validation = validateVisualDocument(document_);
  const diagnostics = createMetricVisualDocumentDiagnostics(document_, { context: "redif-preview" });
  let indexed = true;
  let indexError = null;
  try {
    VisualSpace.fromDocument(document_);
  } catch (error) {
    indexed = false;
    indexError = error.message;
  }

  const timeline = document_.timelines?.find((entry) => entry.id === TIMELINE_ID);
  const diagnostic = document_.diagnostics?.find((entry) => entry.id === "redif-native-evidence");
  const payload = diagnostic?.payload || {};
  const view = DynamicsView.fromVisualSpace(document_, {
    timelineId: TIMELINE_ID,
    propertyField: TRANSPORT_PROPERTY_ID,
    scalarProperty: ENTROPY_PROPERTY_ID,
    colorProperty: TRANSPORT_PROPERTY_ID,
    relationId: RELATION_ID,
    timelineDurationMs: 9200,
    fieldMode: "lifted",
    fieldAlpha: 0.42,
    fieldRadius: 0.24,
    pathWidth: 3.6,
    pathAlpha: 0.72,
    pointSize: 1.86,
  });
  const descriptors = view.toLayerDescriptors();
  const trajectory = descriptors.find((descriptor) => descriptor.metadata?.evidenceRole === "trajectory/path");
  const field = descriptors.find((descriptor) => descriptor.metadata?.evidenceRole === "timeline-ground-field");
  const point = descriptors.find((descriptor) => descriptor.primitive === "InstancedPointLayer");
  const createVariables = createMetricVisualVariables(pageText);
  const commandCalls = createVariables.filter((name) => surfaceCommandPattern(name, "showDynamics").test(pageText));
  const forbiddenHits = FORBIDDEN_PAGE_TOKENS
    .filter(([, pattern]) => pattern.test(pageText))
    .map(([label]) => label);

  const checks = [
    ["evidence schema validates", validation.ok, validation.errors],
    ["evidence indexes", indexed, indexError],
    ["evidence has explicit native provenance", isExplicitNativeMetricVisualExport(document_.provenance), document_.provenance],
    ["evidence is not synthetic", !isSyntheticMetricVisualEvidence(document_.provenance), document_.provenance],
    ["document diagnostics classify native evidence", diagnostics.evidenceNative === true, diagnostics],
    ["native Redif diagnostic exists", Boolean(diagnostic), document_.diagnostics],
    ["native Redif diagnostic is C++ computed", String(payload.computed_by || "").startsWith("METRIC C++ redif_"), payload],
    ["native Redif diagnostic forbids JavaScript computation", payload.javascript_computation === false, payload],
    ["native inverse paths are exported", Array.isArray(payload.inverse_measure_paths) && payload.inverse_measure_paths.length === 7, payload.inverse_measure_paths],
    ["native forward paths are exported", Array.isArray(payload.forward_measure_paths) && payload.forward_measure_paths.length === 7, payload.forward_measure_paths],
    ["Redif timeline has exported states", timeline?.steps?.length === 5, timeline],
    ["transport-path property exists", document_.properties?.some((entry) => entry.id === TRANSPORT_PROPERTY_ID), document_.properties],
    ["entropy property exists", document_.properties?.some((entry) => entry.id === ENTROPY_PROPERTY_ID), document_.properties],
    ["source metric relation exists", document_.relations?.some((entry) => entry.id === RELATION_ID), document_.relations],
    ["preview page loads only the native Redif asset", pageText.includes(EVIDENCE_RELATIVE), extractFetchTargets(pageText)],
    ["preview page imports createMetricVisual", /\bimport\s*\{[\s\S]*\bcreateMetricVisual\b[\s\S]*\}\s*from\s*["']\.\.\/\.\.\/src\/index\.js["']/.test(pageText), null],
    ["preview page stores createMetricVisual surface", createVariables.length > 0, createVariables],
    ["preview page calls showDynamics on that surface", commandCalls.length > 0, { createVariables, commandCalls }],
    ["preview page does not use createMetricVisual view option", !createMetricVisualViewOptionPattern().test(pageText), null],
    ["preview page passes direct fetched evidence into createMetricVisual", createMetricVisualEvidenceUses(pageText).length > 0, createMetricVisualEvidenceUses(pageText)],
    ["preview page validates native provenance before rendering", pageText.includes("isExplicitNativeMetricVisualExport") && pageText.includes("isSyntheticMetricVisualEvidence"), null],
    ["preview page validates metric.visual document before rendering", pageText.includes("validateVisualDocument") && pageText.includes("VisualSpace.fromDocument"), null],
    ["preview page uses exported Redif timeline", pageText.includes(TIMELINE_ID), null],
    ["preview page uses exported path property", pageText.includes(TRANSPORT_PROPERTY_ID), null],
    ["preview page uses exported entropy property", pageText.includes(ENTROPY_PROPERTY_ID), null],
    ["preview page references exported relation as supporting evidence", pageText.includes(RELATION_ID), null],
    ["preview page has no page-local render or algorithm tokens", forbiddenHits.length === 0, forbiddenHits],
    ["showDynamics consumes exported timeline states", view.timelineId === TIMELINE_ID && view.fittedStates.length === 5, { timelineId: view.timelineId, states: view.fittedStates.length }],
    ["showDynamics preserves native record count", view.recordIds.length === 7, view.recordIds],
    ["showDynamics emits trajectory/path evidence", trajectory?.primitive === "CurveRibbonLayer" && trajectory?.metadata?.algorithmicComputation === false, trajectory?.metadata],
    ["showDynamics emits property field evidence", field?.primitive === "HeatFieldLayer" && field?.metadata?.activeFieldPropertyId === TRANSPORT_PROPERTY_ID, field?.metadata],
    ["showDynamics point layer remains native exported state", point?.primitive === "InstancedPointLayer" && point?.metadata?.nativeEvidence?.provenance?.native_export === true, point?.metadata],
  ];

  const failures = checks
    .filter(([, ok]) => !ok)
    .map(([message, , details]) => ({ message, details }));

  console.log(JSON.stringify({
    ok: failures.length === 0,
    evidence: EVIDENCE_RELATIVE,
    page: PAGE_RELATIVE,
    records: document_.records?.length ?? null,
    timeline: TIMELINE_ID,
    timelineStates: timeline?.steps?.length ?? null,
    descriptors: descriptors.map((descriptor) => ({
      id: descriptor.id,
      primitive: descriptor.primitive,
      evidenceRole: descriptor.metadata?.evidenceRole || null,
    })),
    failed: failures.length,
    failures,
  }, null, 2));
  if (failures.length) process.exitCode = 1;
}

function createMetricVisualVariables(text) {
  return Array.from(
    text.matchAll(/\b(?:const|let|var)\s+([A-Za-z_$][\w$]*)\s*=\s*await\s+createMetricVisual\s*\(/g),
    (match) => match[1],
  );
}

function surfaceCommandPattern(variableName, command) {
  return new RegExp(`\\b${escapeRegExp(variableName)}\\s*\\.\\s*${command}\\s*\\(`);
}

function createMetricVisualViewOptionPattern() {
  return /\bcreateMetricVisual\s*\(\s*\{[\s\S]*?\bview\s*:/;
}

function createMetricVisualEvidenceUses(html) {
  const script = extractInlineScriptText(html);
  const uses = [];
  const createPattern = /\bcreateMetricVisual\s*\(/g;
  for (const match of script.matchAll(createPattern)) {
    const window = script.slice(match.index, match.index + 5000);
    const property = /\b(evidence|document|input)\s*:\s*([A-Za-z_$][\w$]*)\b/.exec(window);
    if (!property) continue;
    uses.push({
      property: property[1],
      variable: property[2],
      index: match.index + property.index,
    });
  }
  return uses;
}

function extractInlineScriptText(html) {
  return Array.from(html.matchAll(/<script\b[^>]*>([\s\S]*?)<\/script>/gi), (match) => match[1]).join("\n");
}

function extractFetchTargets(html) {
  const script = extractInlineScriptText(html);
  const constants = new Map();
  for (const match of script.matchAll(/const\s+([A-Z0-9_]+)\s*=\s*["']([^"']+)["']/g)) {
    constants.set(match[1], match[2]);
  }
  const targets = [];
  for (const match of script.matchAll(/fetch\(\s*(?:(["'])([^"']+)\1|([A-Z0-9_]+))\s*\)/g)) {
    if (match[2]) targets.push(match[2]);
    if (match[3] && constants.has(match[3])) targets.push(constants.get(match[3]));
  }
  return targets;
}

function escapeRegExp(value) {
  return String(value).replace(/[.*+?^${}()|[\]\\]/g, "\\$&");
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
