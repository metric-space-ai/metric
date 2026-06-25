#!/usr/bin/env node
/*
 * Checks that every public preview has a concrete visual brief before it can be
 * discussed as a hero candidate. This is not a promotion gate. It verifies that
 * review-pending examples declare the visual claim, primary grammar, required
 * primitives, evidence expectations and explicit acceptance blockers.
 *
 * Artifacts:
 *   output/visual/check-hero-visual-briefs/results.json
 */

import { mkdir, readFile, writeFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const DEFAULT_REGRESSION_REPORT = resolve(ROOT, "output", "visual", "check-visual-regression-public-examples", "results.json");
const DEFAULT_BRIEF_MANIFEST = resolve(ROOT, "visual", "hero-visual-briefs.manifest.json");
const OUT_DIR = resolve(process.env.METRIC_VISUAL_BRIEF_OUT || resolve(ROOT, "output", "visual", "check-hero-visual-briefs"));
const REGRESSION_REPORT = resolve(process.env.METRIC_VISUAL_REGRESSION_REPORT || DEFAULT_REGRESSION_REPORT);
const BRIEF_MANIFEST = resolve(process.env.METRIC_VISUAL_BRIEF_MANIFEST || DEFAULT_BRIEF_MANIFEST);

async function main() {
  const regression = JSON.parse(await readFile(REGRESSION_REPORT, "utf8"));
  const manifest = JSON.parse(await readFile(BRIEF_MANIFEST, "utf8"));
  const issues = [];
  const rows = [];

  issues.push(...validateManifestShape(manifest));
  const briefs = new Map();
  for (const brief of manifest.previewBriefs || []) {
    if (brief?.exampleId) briefs.set(brief.exampleId, brief);
  }

  const regressionRows = new Map((regression.results || []).map((result) => [result.name, result]));
  const previewRows = (regression.results || []).filter((result) => result.status?.category === "public-preview-only");

  for (const result of previewRows) {
    const brief = briefs.get(result.name);
    const row = validateBriefAgainstResult(result, brief);
    rows.push(row);
    issues.push(...row.issues);
  }

  for (const [exampleId] of briefs) {
    const result = regressionRows.get(exampleId);
    if (!result) {
      issues.push({ code: "brief-example-not-in-regression-report", example: exampleId });
      continue;
    }
    if (result.status?.category !== "public-preview-only") {
      issues.push({
        code: "brief-example-is-not-public-preview",
        example: exampleId,
        status: result.status?.category ?? null,
      });
    }
  }

  const report = {
    ok: issues.length === 0,
    generatedAt: new Date().toISOString(),
    regressionReport: REGRESSION_REPORT,
    regressionGeneratedAt: regression.generatedAt ?? null,
    briefManifest: BRIEF_MANIFEST,
    publicPreviewCount: previewRows.length,
    briefCount: briefs.size,
    blockedByRecordCount: rows.filter((row) => row.blockers.includes("record-count-below-hero-minimum")).map((row) => row.name),
    rows,
    issues,
  };

  await mkdir(OUT_DIR, { recursive: true });
  await writeFile(resolve(OUT_DIR, "results.json"), JSON.stringify(report, null, 2));
  console.log(JSON.stringify(report, null, 2));
  if (!report.ok) process.exitCode = 1;
}

function validateManifestShape(manifest) {
  const issues = [];
  if (!manifest || typeof manifest !== "object" || Array.isArray(manifest)) {
    return [{ code: "brief-manifest-not-object" }];
  }
  if (manifest.schemaVersion !== 1) {
    issues.push({ code: "brief-manifest-schema-version-unsupported", expected: 1, actual: manifest.schemaVersion ?? null });
  }
  if (!Array.isArray(manifest.previewBriefs)) {
    issues.push({ code: "brief-manifest-preview-briefs-not-array" });
    return issues;
  }
  const seen = new Set();
  for (const [index, brief] of manifest.previewBriefs.entries()) {
    if (!brief || typeof brief !== "object" || Array.isArray(brief)) {
      issues.push({ code: "brief-entry-not-object", index });
      continue;
    }
    const example = stringField(brief.exampleId);
    if (!example) issues.push({ code: "brief-entry-missing-example-id", index });
    if (seen.has(example)) issues.push({ code: "brief-entry-duplicate-example-id", example });
    seen.add(example);
    if (brief.reviewStatus !== "review-pending") {
      issues.push({ code: "brief-entry-review-status-not-review-pending", example, status: brief.reviewStatus ?? null });
    }
    for (const field of ["expectedPrimaryVisualGrammar", "visualClaim", "primaryVisualGrammar"]) {
      if (!stringField(brief[field])) issues.push({ code: `brief-entry-missing-${field}`, example });
    }
    if (brief.requiresNativeEvidence !== true) {
      issues.push({ code: "brief-entry-must-require-native-evidence", example });
    }
    for (const field of ["requiredPrimitives", "requiredEvidence", "acceptanceBlockers"]) {
      if (!Array.isArray(brief[field]) || brief[field].length === 0 || brief[field].some((value) => !stringField(value))) {
        issues.push({ code: `brief-entry-invalid-${field}`, example });
      }
    }
    if (!brief.minimumEvidence || typeof brief.minimumEvidence !== "object" || Array.isArray(brief.minimumEvidence)) {
      issues.push({ code: "brief-entry-missing-minimum-evidence", example });
    }
  }
  return issues;
}

function validateBriefAgainstResult(result, brief) {
  const issues = [];
  const name = result.name;
  const primitives = result.runtime?.state?.descriptorPrimitives || [];
  const nativeEvidence = result.evidence?.nativeEvidence || [];
  const recordCount = maxNumber(nativeEvidence.map((entry) => entry.recordCount));
  const relationCount = maxNumber(nativeEvidence.map((entry) => entry.relationCount));
  const recordTypeCount = maxNumber(nativeEvidence.map((entry) => entry.recordTypeCount));

  if (!brief) {
    return {
      name,
      ok: false,
      grammar: result.grammar?.expected ?? null,
      recordCount,
      relationCount,
      blockers: [],
      issues: [{ code: "missing-preview-visual-brief", example: name }],
    };
  }

  if (brief.expectedPrimaryVisualGrammar !== result.grammar?.expected) {
    issues.push({
      code: "brief-grammar-mismatch",
      example: name,
      expected: brief.expectedPrimaryVisualGrammar,
      actual: result.grammar?.expected ?? null,
    });
  }
  if (brief.requiresNativeEvidence === true && result.evidence?.usesNativeEvidence !== true) {
    issues.push({ code: "brief-native-evidence-missing", example: name });
  }
  for (const primitive of brief.requiredPrimitives || []) {
    if (!primitives.includes(primitive)) {
      issues.push({ code: "brief-required-primitive-missing", example: name, primitive, primitives });
    }
  }

  const blockers = brief.acceptanceBlockers || [];
  const minRecordCount = numberField(brief.minimumEvidence?.minRecordCountForHero);
  if (minRecordCount != null && recordCount < minRecordCount && !blockers.includes("record-count-below-hero-minimum")) {
    issues.push({
      code: "brief-missing-record-count-blocker",
      example: name,
      recordCount,
      minRecordCountForHero: minRecordCount,
    });
  }
  if (minRecordCount != null && recordCount >= minRecordCount && blockers.includes("record-count-below-hero-minimum")) {
    issues.push({
      code: "brief-stale-record-count-blocker",
      example: name,
      recordCount,
      minRecordCountForHero: minRecordCount,
    });
  }

  const minRelationCount = numberField(brief.minimumEvidence?.minRelationCount);
  if (minRelationCount != null && relationCount < minRelationCount) {
    issues.push({
      code: "brief-relation-count-below-minimum",
      example: name,
      relationCount,
      minRelationCount,
    });
  }
  const minRecordTypes = numberField(brief.minimumEvidence?.minRecordTypesForHero);
  if (minRecordTypes != null && recordTypeCount < minRecordTypes && !blockers.includes("record-type-count-below-hero-minimum")) {
    issues.push({
      code: "brief-missing-record-type-count-blocker",
      example: name,
      recordTypeCount,
      minRecordTypesForHero: minRecordTypes,
    });
  }
  if (minRecordTypes != null && recordTypeCount >= minRecordTypes && blockers.includes("record-type-count-below-hero-minimum")) {
    issues.push({
      code: "brief-stale-record-type-count-blocker",
      example: name,
      recordTypeCount,
      minRecordTypesForHero: minRecordTypes,
    });
  }
  if (!blockers.includes("visual-composition-not-human-accepted")) {
    issues.push({ code: "brief-missing-human-composition-blocker", example: name });
  }

  return {
    name,
    ok: issues.length === 0,
    reviewStatus: brief.reviewStatus,
    grammar: result.grammar?.expected ?? null,
    visualClaim: brief.visualClaim,
    primaryVisualGrammar: brief.primaryVisualGrammar,
    recordCount,
    relationCount,
    recordTypeCount,
    minRecordCountForHero: minRecordCount,
    minRecordTypesForHero: minRecordTypes,
    primitives,
    requiredPrimitives: brief.requiredPrimitives || [],
    blockers,
    issues,
  };
}

function stringField(value) {
  return typeof value === "string" ? value.trim() : "";
}

function numberField(value) {
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
