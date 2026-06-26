#!/usr/bin/env node
/*
 * Native hero evidence scale gate.
 *
 * This check reads the public preview briefs and their native
 * `docs/examples/assets/<example>/metric.visual.json` documents without
 * launching a browser. It does not promote heroes. It verifies that native
 * evidence size and manifest blockers agree before a browser screenshot review
 * is attempted.
 *
 * Artifacts:
 *   output/visual/check-native-hero-evidence-scale/results.json
 */

import { mkdir, readFile, writeFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import {
  explicitNativeMetricVisualExportSignal,
  isExplicitNativeMetricVisualExport,
  isNativeMetricVisualDocument,
  isSyntheticMetricVisualEvidence,
} from "../src/data/provenance.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const EXAMPLES_DIR = resolve(ROOT, "visual", "examples");
const DEFAULT_BRIEF_MANIFEST = resolve(ROOT, "visual", "hero-visual-briefs.manifest.json");
const OUT_DIR = resolve(process.env.METRIC_VISUAL_NATIVE_SCALE_OUT || resolve(ROOT, "output", "visual", "check-native-hero-evidence-scale"));
const BRIEF_MANIFEST = resolve(process.env.METRIC_VISUAL_BRIEF_MANIFEST || DEFAULT_BRIEF_MANIFEST);

async function main() {
  const manifest = JSON.parse(await readFile(BRIEF_MANIFEST, "utf8"));
  const issues = [];
  const rows = [];

  if (manifest?.schemaVersion !== 1 || !Array.isArray(manifest.previewBriefs)) {
    issues.push({ code: "brief-manifest-invalid", manifest: BRIEF_MANIFEST });
  }

  for (const brief of manifest.previewBriefs || []) {
    const row = await inspectBrief(brief);
    rows.push(row);
    issues.push(...row.issues);
  }

  const report = {
    ok: issues.length === 0,
    generatedAt: new Date().toISOString(),
    briefManifest: BRIEF_MANIFEST,
    rowCount: rows.length,
    nativeScaleReadyCount: rows.filter((row) => row.nativeScaleReady).length,
    blockedByRecordCount: rows.filter((row) => row.blockers.includes("record-count-below-hero-minimum")).map((row) => row.exampleId),
    rows,
    issues,
  };

  await mkdir(OUT_DIR, { recursive: true });
  await writeFile(resolve(OUT_DIR, "results.json"), JSON.stringify(report, null, 2));
  console.log(JSON.stringify(report, null, 2));
  if (!report.ok) process.exitCode = 1;
}

async function inspectBrief(brief) {
  const exampleId = stringField(brief?.exampleId);
  const issues = [];
  const blockers = Array.isArray(brief?.acceptanceBlockers) ? brief.acceptanceBlockers : [];
  const minimumEvidence = brief?.minimumEvidence && typeof brief.minimumEvidence === "object"
    ? brief.minimumEvidence
    : {};
  const minRecordCount = finiteNumberOrNull(minimumEvidence.minRecordCountForHero);
  const minRelationCount = finiteNumberOrNull(minimumEvidence.minRelationCount);
  const minRecordTypes = finiteNumberOrNull(minimumEvidence.minRecordTypesForHero);
  const assetTargets = exampleId ? await publicNativeAssetTargets(exampleId) : [];
  const assets = [];

  if (!exampleId) issues.push({ code: "brief-missing-example-id" });
  if (!assetTargets.length) issues.push({ code: "native-asset-target-missing", example: exampleId });

  for (const target of assetTargets) {
    const assetPath = resolve(EXAMPLES_DIR, exampleId, target);
    try {
      const document = JSON.parse(await readFile(assetPath, "utf8"));
      const asset = summarizeAsset(exampleId, target, assetPath, document);
      assets.push(asset);
      if (asset.schema !== "metric.visual.v1") {
        issues.push({ code: "native-asset-schema-invalid", example: exampleId, target, schema: asset.schema });
      }
      if (asset.synthetic === true) {
        issues.push({ code: "native-asset-marked-synthetic", example: exampleId, target });
      }
      if (asset.native !== true) {
        issues.push({ code: "native-asset-provenance-missing", example: exampleId, target, provenance: asset.provenance });
      }
      if (asset.nativeExportExplicit !== true) {
        issues.push({ code: "native-asset-native-export-flag-missing", example: exampleId, target, provenance: asset.provenance });
      }
    } catch (error) {
      issues.push({ code: "native-asset-unreadable", example: exampleId, target, message: error.message });
    }
  }

  const totalRecordCount = maxNumber(assets.map((asset) => asset.recordCount));
  const recordCount = maxNumber(assets.map((asset) => asset.scaleRecordCount ?? asset.recordCount));
  const relationCount = maxNumber(assets.map((asset) => asset.relationCount));
  const recordTypeCount = maxNumber(assets.map((asset) => asset.recordTypeCount));
  const recordCountReady = minRecordCount == null || recordCount >= minRecordCount;
  const relationCountReady = minRelationCount == null || relationCount >= minRelationCount;
  const recordTypesReady = minRecordTypes == null || recordTypeCount >= minRecordTypes;
  const nativeScaleReady = assets.length > 0 && recordCountReady && relationCountReady && recordTypesReady;

  if (!recordCountReady && !blockers.includes("record-count-below-hero-minimum")) {
    issues.push({ code: "missing-record-count-blocker", example: exampleId, recordCount, minRecordCountForHero: minRecordCount });
  }
  if (recordCountReady && blockers.includes("record-count-below-hero-minimum")) {
    issues.push({ code: "stale-record-count-blocker", example: exampleId, recordCount, minRecordCountForHero: minRecordCount });
  }
  if (!relationCountReady) {
    issues.push({ code: "relation-count-below-hero-minimum", example: exampleId, relationCount, minRelationCount });
  }
  if (!recordTypesReady && !blockers.includes("record-type-count-below-hero-minimum")) {
    issues.push({ code: "missing-record-type-count-blocker", example: exampleId, recordTypeCount, minRecordTypesForHero: minRecordTypes });
  }
  if (recordTypesReady && blockers.includes("record-type-count-below-hero-minimum")) {
    issues.push({ code: "stale-record-type-count-blocker", example: exampleId, recordTypeCount, minRecordTypesForHero: minRecordTypes });
  }

  return {
    exampleId,
    nativeScaleReady,
    recordCount,
    totalRecordCount,
    relationCount,
    recordTypeCount,
    minRecordCountForHero: minRecordCount,
    minRelationCount,
    minRecordTypesForHero: minRecordTypes,
    blockers,
    assets,
    issues,
  };
}

async function publicNativeAssetTargets(exampleId) {
  const index = await maybeRead(resolve(EXAMPLES_DIR, exampleId, "index.html"));
  const nativeAssetPattern = new RegExp("docs/examples/assets/.+/metric\\.visual\\.json$");
  return extractFetchTargets(index)
    .filter((target) => nativeAssetPattern.test(target));
}

function summarizeAsset(exampleId, target, assetPath, document) {
  const records = Array.isArray(document?.records) ? document.records : [];
  const relationRecordCount = maxRelationRecordCount(document);
  const scaleRecordCount = scaleRecordCountForExample(exampleId, records.length, relationRecordCount);
  return {
    target,
    path: assetPath,
    schema: document?.schema ?? null,
    synthetic: isSyntheticMetricVisualEvidence(document?.provenance),
    recordCount: records.length,
    scaleRecordCount,
    scaleRecordCountSource: scaleRecordCount === records.length ? "records" : "metric-relation-record-ids",
    relationRecordCount,
    relationCount: Array.isArray(document?.relations) ? document.relations.length : 0,
    coordinateCount: Array.isArray(document?.coordinates) ? document.coordinates.length : 0,
    viewKinds: Array.isArray(document?.views) ? document.views.map((view) => view?.kind).filter(Boolean) : [],
    recordTypeCount: countRecordTypes(records),
    native: isNativeMetricVisualDocument(document),
    nativeExportExplicit: isExplicitNativeMetricVisualExport(document?.provenance),
    provenance: {
      writer: document?.provenance?.writer ?? null,
      runtime: document?.provenance?.runtime ?? null,
      computation: document?.provenance?.computation ?? null,
      sourceExample: document?.provenance?.source_example ?? null,
      nativeExport: isExplicitNativeMetricVisualExport(document?.provenance),
      nativeExportSignal: explicitNativeMetricVisualExportSignal(document?.provenance),
    },
  };
}

function scaleRecordCountForExample(exampleId, recordCount, relationRecordCount) {
  if (exampleId === "process-curve-external-hero") {
    return relationRecordCount;
  }
  return recordCount;
}

function maxRelationRecordCount(document) {
  const relations = Array.isArray(document?.relations) ? document.relations : [];
  const counts = [];
  for (const relation of relations) {
    if (Array.isArray(relation?.record_ids)) {
      counts.push(relation.record_ids.length);
    } else if (Array.isArray(relation?.recordIds)) {
      counts.push(relation.recordIds.length);
    }
  }
  return maxNumber(counts);
}

function countRecordTypes(records) {
  const types = new Set();
  for (const record of records) {
    const type = record?.type ?? record?.record_type ?? record?.payload?.kind ?? null;
    if (type != null && String(type).trim()) types.add(String(type));
  }
  return types.size;
}

function extractFetchTargets(index) {
  const constants = new Map();
  const constantPattern = new RegExp("const\\s+([A-Z0-9_]+)\\s*=\\s*[\"']([^\"']+)[\"']", "g");
  for (const match of index.matchAll(constantPattern)) constants.set(match[1], match[2]);

  const targets = [];
  const fetchPattern = new RegExp("fetch\\(\\s*(?:([\"'])([^\"']+)\\1|([A-Z0-9_]+))\\s*\\)", "g");
  for (const match of index.matchAll(fetchPattern)) {
    if (match[2]) targets.push(match[2]);
    else if (match[3] && constants.has(match[3])) targets.push(constants.get(match[3]));
  }
  return targets;
}

async function maybeRead(path) {
  try {
    return await readFile(path, "utf8");
  } catch {
    return "";
  }
}

function stringField(value) {
  return typeof value === "string" ? value.trim() : "";
}

function finiteNumberOrNull(value) {
  const number = Number(value);
  return Number.isFinite(number) ? number : null;
}

function maxNumber(values) {
  const finite = values.map(Number).filter(Number.isFinite);
  return finite.length ? Math.max(...finite) : 0;
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
