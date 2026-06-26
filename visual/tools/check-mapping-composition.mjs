#!/usr/bin/env node

import { readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import {
  MappingView,
  mappingMotionProgressAt,
} from "../src/views/MappingView.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const EVIDENCE_PATH = "docs/examples/assets/mapping-dimensionality/metric.visual.json";
const HERO_PATH = "visual/examples/mapping-dimensionality-hero/index.html";

const SOURCE_COORDINATE_ID = "source-coordinate-layout-3d";
const TARGET_COORDINATE_ID = "parametric-coordinate-latent-2d";
const RESIDUAL_PROPERTY_ID = "local-mapping-distortion";
const LABEL_PROPERTY_ID = "process-family";
const MOTION_TIMING = Object.freeze({
  profile: "source-hold-quick-transition-target-hold",
  sourceHoldMs: 1600,
  transitionMs: 720,
  targetHoldMs: 1700,
  resetHoldMs: 360,
  totalMs: 5200,
});

async function main() {
  const document = JSON.parse(await readFile(resolve(ROOT, EVIDENCE_PATH), "utf8"));
  const heroSource = await readFile(resolve(ROOT, HERO_PATH), "utf8");
  const view = MappingView.fromVisualSpace(document, {
    sourceCoordinateId: SOURCE_COORDINATE_ID,
    targetCoordinateId: TARGET_COORDINATE_ID,
    residualProperty: RESIDUAL_PROPERTY_ID,
    labels: LABEL_PROPERTY_ID,
    motionTiming: MOTION_TIMING,
    targetRadius: 1.86,
    groundScale: 2.25,
    groundProjectionAlpha: 0.34,
    residualVectorLimit: 120,
    residualVectorBucketCount: 5,
    residualVectorSectorCount: 18,
    residualVectorRadialBuckets: 3,
    residualVectorLengthScale: 0.78,
    residualVectorMaxLength: 0.62,
    residualVectorMinLength: 0.12,
    residualVectorScaleByMagnitude: true,
    residualVectorOrder: -1,
  });
  const descriptors = view.toLayerDescriptors();
  const primitives = descriptors.map((descriptor) => descriptor.primitive || descriptor.kind);
  const point = descriptors.find((descriptor) => descriptor.primitive === "InstancedPointLayer" || descriptor.primitive === "InstancedGlyphLayer");
  const projection = descriptors.find((descriptor) => descriptor.primitive === "GroundProjectionLayer");
  const residual = descriptors.find((descriptor) => descriptor.primitive === "RelationEdgeLayer" && descriptor.metadata?.role === "residual/error");
  const labels = descriptors.find((descriptor) => descriptor.primitive === "BillboardLabelLayer");
  const noResidualView = MappingView.fromVisualSpace(document, {
    sourceCoordinateId: SOURCE_COORDINATE_ID,
    targetCoordinateId: TARGET_COORDINATE_ID,
    labels: LABEL_PROPERTY_ID,
  });
  const noResidualLayer = noResidualView
    .toLayerDescriptors()
    .find((descriptor) => descriptor.metadata?.role === "residual/error" || descriptor.source?.role === "residual/error");
  const sourceHold = mappingMotionProgressAt(100, view.motionTiming);
  const transition = mappingMotionProgressAt(view.motionTiming.sourceHoldMs + view.motionTiming.transitionMs * 0.5, view.motionTiming);
  const targetHold = mappingMotionProgressAt(view.motionTiming.sourceHoldMs + view.motionTiming.transitionMs + 100, view.motionTiming);
  const mutationFindings = pageEvidenceMutationFindings(heroSource);

  const checks = [
    ["selected view kind is mapping", view.kind === "mapping", view.kind],
    ["native evidence has 1,000 records", Array.isArray(document.records) && document.records.length === 1000, document.records?.length],
    ["native residual property is present", hasPropertyValues(document, RESIDUAL_PROPERTY_ID, 1000), propertySummary(document, RESIDUAL_PROPERTY_ID)],
    ["descriptors include InstancedPointLayer", primitives.includes("InstancedPointLayer") || primitives.includes("InstancedGlyphLayer"), primitives],
    ["descriptors include GroundProjectionLayer", Boolean(projection), primitives],
    ["descriptors include RelationEdgeLayer", Boolean(residual), primitives],
    ["residual layer uses explicit native residual property id", residual?.metadata?.residualPropertyId === RESIDUAL_PROPERTY_ID && residual?.source?.propertyId === RESIDUAL_PROPERTY_ID, residual?.metadata],
    ["residual layer carries residual magnitudes", residual?.channels?.residual?.count > 0 && residual?.channels?.residual?.semantic === "residual-magnitude", residual?.channels?.residual],
    ["residual layer samples native representatives", residual?.metadata?.nativeResidualRecordCount === 1000 && residual?.metadata?.recordCount === 120 && residual?.metadata?.residualSelection?.strategy === "representative-residual-buckets", residual?.metadata?.residualSelection],
    ["residual layer is drawn behind point structure", Number(residual?.order) < Number(point?.order ?? 0), { residualOrder: residual?.order, pointOrder: point?.order ?? 0 }],
    ["residual layer uses shared edge legibility without lane faking", residual?.metadata?.edgeLegibility?.schema === "metric.visual.relation_edge_legibility.v1" && residual?.metadata?.edgeLegibility?.role === "residual/error" && residual?.metadata?.edgeLegibility?.laneStrategy === "none" && residual?.metadata?.edgeLegibility?.alphaSource === "per-edge-color-alpha-scaled-before-global-material-alpha", residual?.metadata?.edgeLegibility],
    ["labels help orientation", labels?.metadata?.labelCount > 0, labels?.metadata],
    ["mapping point layer declares coordinate morph", point?.animation?.mode === "coordinate-morph" && point?.channels?.targetPosition?.count === 1000, point?.animation],
    ["mapping motion timing has source hold", sourceHold.phase === "source-hold" && sourceHold.progress === 0, sourceHold],
    ["mapping motion timing has quick transition", transition.phase === "quick-transition" && transition.progress > 0.3 && transition.progress < 0.7, transition],
    ["mapping motion timing has target hold", targetHold.phase === "target-hold" && targetHold.progress === 1, targetHold],
    ["mapping without residual property emits no residual/error layer", !noResidualLayer, noResidualLayer],
    ["hero uses showMapping instead of a custom mapping renderer", /\.showMapping\(/.test(heroSource) && !/new\s+MappingView\b|RelationEdgeLayer\b|drawArrays\s*\(/.test(heroSource), null],
    ["hero does not mutate native evidence", mutationFindings.length === 0, mutationFindings],
  ];

  report(checks, {
    primitives,
    residual: residual ? {
      id: residual.id,
      recordCount: residual.metadata?.recordCount,
      nativeResidualRecordCount: residual.metadata?.nativeResidualRecordCount,
      residualPropertyId: residual.metadata?.residualPropertyId,
      residualSelection: residual.metadata?.residualSelection,
    } : null,
  });
}

function hasPropertyValues(document, propertyId, expectedCount) {
  const property = propertyById(document, propertyId);
  return Array.isArray(property?.values) && property.values.length === expectedCount;
}

function propertySummary(document, propertyId) {
  const property = propertyById(document, propertyId);
  return property ? {
    id: property.id,
    valueType: property.value_type ?? property.valueType,
    targetType: property.target_type ?? property.targetType,
    values: Array.isArray(property.values) ? property.values.length : null,
  } : null;
}

function propertyById(document, propertyId) {
  return (Array.isArray(document?.properties) ? document.properties : [])
    .find((property) => String(property.id) === String(propertyId));
}

function pageEvidenceMutationFindings(source) {
  const patterns = [
    ["delete nativeDocument", /delete\s+nativeDocument\b/],
    ["assign top-level native evidence collection", /nativeDocument\s*(?:\.\s*|\[\s*["'`])(?:records|relations|graphs|coordinates|properties|timelines|views)(?:\s*["'`]\s*\])?\s*=/],
    ["clear relation values", /\brelations?\b[\s\S]{0,120}\bvalues\b\s*=\s*\[\s*\]/],
    ["rewrite property values", /\bproperties?\b[\s\S]{0,120}\bvalues\b\s*=/],
    ["mutate native evidence collection", /nativeDocument\s*(?:\.\s*|\[\s*["'`])(?:records|relations|graphs|coordinates|properties|timelines|views)(?:\s*["'`]\s*\])?[\s\S]{0,80}\.(?:splice|push|pop|shift|unshift|sort|reverse)\s*\(/],
  ];
  return patterns
    .filter(([, pattern]) => pattern.test(source))
    .map(([label]) => label);
}

function report(checks, summary) {
  const failures = checks
    .filter(([, ok]) => !ok)
    .map(([message, , details]) => ({ message, details }));
  console.log(JSON.stringify({
    ok: failures.length === 0,
    total: checks.length,
    failed: failures.length,
    failures,
    summary,
  }, null, 2));
  if (failures.length) process.exitCode = 1;
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
