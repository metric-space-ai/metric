#!/usr/bin/env node
/*
 * Hero screenshot review gate.
 *
 * This gate consumes the browser-backed public visual regression report and
 * keeps screenshot availability separate from hero acceptance. It is not a
 * visual taste judge. It prevents accidental promotion of public previews to
 * hero-accepted status without an explicit review manifest.
 *
 * Artifacts:
 *   output/visual/check-hero-screenshot-review/results.json
 */

import { mkdir, readFile, stat, writeFile } from "node:fs/promises";
import { existsSync } from "node:fs";
import { dirname, isAbsolute, join, relative, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const DEFAULT_REGRESSION_REPORT = resolve(ROOT, "output", "visual", "check-visual-regression-public-examples", "results.json");
const REVIEW_MANIFEST = resolve(ROOT, "visual", "hero-acceptance.manifest.json");
const OUT_DIR = resolve(process.env.METRIC_VISUAL_HERO_REVIEW_OUT || join(ROOT, "output", "visual", "check-hero-screenshot-review"));
const REGRESSION_REPORT = resolve(process.env.METRIC_VISUAL_REGRESSION_REPORT || DEFAULT_REGRESSION_REPORT);

const PROTECTED_ACCEPTED = new Map([
  ["grae10-metric-engine", {
    reason: "protected-grae10-golden-reference",
    source: "protected-grae10-golden-reference",
  }],
]);

async function main() {
  const regression = JSON.parse(await readFile(REGRESSION_REPORT, "utf8"));
  const manifest = await loadOptionalManifest(REVIEW_MANIFEST);
  const manifestValidation = validateManifest(manifest);
  const accepted = acceptedMap(manifest, manifestValidation.issues);
  const resultsByName = new Map((regression.results || []).map((result) => [result.name, result]));
  const issues = [];
  const rows = [];
  issues.push(...manifestValidation.issues);

  if (regression.ok !== true) {
    issues.push({
      code: "regression-report-not-green",
      report: REGRESSION_REPORT,
      failed: regression.failed ?? null,
    });
  }

  for (const result of regression.results || []) {
    const row = await reviewRow(result, accepted);
    rows.push(row);
    issues.push(...row.issues);
  }

  for (const entry of accepted.values()) {
    if (entry.source !== "manifest") continue;
    const result = resultsByName.get(entry.example);
    if (!result) {
      issues.push({
        code: "manifest-example-not-in-regression-report",
        example: entry.example,
      });
      continue;
    }
    issues.push(...await validateManifestContract(entry, result));
  }

  const unexpectedAccepted = rows.filter((row) => row.heroAccepted && !row.acceptedByReview);
  for (const row of unexpectedAccepted) {
    issues.push({
      code: "hero-accepted-without-review",
      example: row.name,
      statusCategory: row.statusCategory,
    });
  }

  const missingProtected = rows.find((row) => row.name === "grae10-metric-engine");
  if (!missingProtected?.heroAccepted || !missingProtected?.acceptedByReview) {
    issues.push({
      code: "protected-grae10-not-accepted",
      example: "grae10-metric-engine",
    });
  }

  const report = {
    ok: issues.length === 0,
    generatedAt: new Date().toISOString(),
    regressionReport: REGRESSION_REPORT,
    regressionGeneratedAt: regression.generatedAt ?? null,
    manifest: existsSync(REVIEW_MANIFEST) ? REVIEW_MANIFEST : null,
    manifestSchemaVersion: manifest?.schemaVersion ?? null,
    manifestIssueCount: manifestValidation.issues.length,
    acceptedHeroes: rows.filter((row) => row.acceptedByReview).map((row) => row.name),
    reviewPendingPreviews: rows.filter((row) => row.reviewStatus === "review-pending").map((row) => row.name),
    acceptedHeroCount: rows.filter((row) => row.acceptedByReview).length,
    reviewPendingCount: rows.filter((row) => row.reviewStatus === "review-pending").length,
    rows,
    issues,
  };

  await mkdir(OUT_DIR, { recursive: true });
  await writeFile(join(OUT_DIR, "results.json"), JSON.stringify(report, null, 2));
  console.log(JSON.stringify(report, null, 2));
  if (!report.ok) process.exitCode = 1;
}

async function reviewRow(result, accepted) {
  const issues = [];
  const status = result.status || {};
  const render = result.render || {};
  const screenshot = render.screenshot || null;
  const canvasScreenshot = render.canvasScreenshot || null;
  const acceptance = accepted.get(result.name) || null;
  const acceptedReason = acceptance?.reason || null;
  const heroAccepted = status.heroAccepted === true || status.category === "hero-accepted";
  const acceptedByReview = acceptance != null;

  const screenshotOk = await fileLooksPresent(screenshot);
  const canvasScreenshotOk = await fileLooksPresent(canvasScreenshot);
  if (!screenshotOk) {
    issues.push({ code: "missing-page-screenshot", example: result.name, screenshot });
  }
  if (!canvasScreenshotOk) {
    issues.push({ code: "missing-canvas-screenshot", example: result.name, canvasScreenshot });
  }
  if (render.ok !== true) {
    issues.push({ code: "render-not-green", example: result.name });
  }
  if (result.ok !== true) {
    issues.push({ code: "example-regression-not-green", example: result.name });
  }

  return {
    name: result.name,
    statusCategory: status.category ?? null,
    heroAccepted,
    acceptedByReview,
    acceptedReason,
    acceptanceSource: acceptance?.source || null,
    acceptanceDate: acceptance?.date || null,
    reviewStatus: acceptedByReview ? "accepted" : "review-pending",
    runtimeLayerCount: result.runtime?.state?.runtimeLayerCount ?? null,
    selectedViewKind: result.runtime?.state?.selectedViewKind ?? null,
    screenshot,
    canvasScreenshot,
    screenshotOk,
    canvasScreenshotOk,
    nativeEvidence: result.evidence?.usesNativeEvidence === true || Boolean(result.evidence?.protectedGrae10),
    grammarOk: result.grammar?.ok === true,
    renderOk: render.ok === true,
    regressionOk: result.ok === true,
    issues,
  };
}

function acceptedMap(manifest, issues) {
  const out = new Map(PROTECTED_ACCEPTED);
  for (const entry of manifest?.acceptedHeroes || []) {
    if (!entry || typeof entry !== "object" || !entry.exampleId) continue;
    if (entry.humanAcceptance?.status !== "accepted") continue;
    const example = String(entry.exampleId);
    if (out.has(example) && example !== "grae10-metric-engine") {
      issues.push({ code: "manifest-duplicate-acceptance", example });
      continue;
    }
    out.set(example, {
      example,
      source: "manifest",
      reason: String(entry.acceptanceReason || "manual-hero-acceptance"),
      date: entry.humanAcceptance.date || null,
      entry,
    });
  }
  return out;
}

async function loadOptionalManifest(path) {
  if (!existsSync(path)) return null;
  return JSON.parse(await readFile(path, "utf8"));
}

function validateManifest(manifest) {
  const issues = [];
  if (manifest == null) return { issues };
  if (!manifest || typeof manifest !== "object" || Array.isArray(manifest)) {
    return { issues: [{ code: "manifest-not-object" }] };
  }
  if (manifest.schemaVersion !== 1) {
    issues.push({
      code: "manifest-schema-version-unsupported",
      expected: 1,
      actual: manifest.schemaVersion ?? null,
    });
  }
  if (!Array.isArray(manifest.acceptedHeroes)) {
    issues.push({ code: "manifest-accepted-heroes-not-array" });
    return { issues };
  }

  const seen = new Set();
  for (const [index, entry] of manifest.acceptedHeroes.entries()) {
    if (!entry || typeof entry !== "object" || Array.isArray(entry)) {
      issues.push({ code: "manifest-entry-not-object", index });
      continue;
    }
    const example = typeof entry.exampleId === "string" ? entry.exampleId.trim() : "";
    if (!example) {
      issues.push({ code: "manifest-entry-missing-example-id", index });
    } else if (seen.has(example)) {
      issues.push({ code: "manifest-entry-duplicate-example-id", example });
    }
    seen.add(example);

    if (entry.requiresNativeEvidence !== true) {
      issues.push({ code: "manifest-entry-must-require-native-evidence", example: example || null });
    }
    if (!nonEmptyString(entry.expectedPrimaryVisualGrammar)) {
      issues.push({ code: "manifest-entry-missing-expected-primary-visual-grammar", example: example || null });
    }
    if (!entry.screenshot || typeof entry.screenshot !== "object" || Array.isArray(entry.screenshot)) {
      issues.push({ code: "manifest-entry-missing-screenshot-contract", example: example || null });
    } else if (!nonEmptyString(entry.screenshot.generatedReport) && !nonEmptyString(entry.screenshot.file)) {
      issues.push({ code: "manifest-entry-missing-screenshot-reference", example: example || null });
    }
    if (!entry.humanAcceptance || typeof entry.humanAcceptance !== "object" || Array.isArray(entry.humanAcceptance)) {
      issues.push({ code: "manifest-entry-missing-human-acceptance", example: example || null });
    } else {
      if (entry.humanAcceptance.status !== "accepted") {
        issues.push({
          code: "manifest-entry-human-acceptance-status-not-accepted",
          example: example || null,
          status: entry.humanAcceptance.status ?? null,
        });
      }
      if (!/^\d{4}-\d{2}-\d{2}$/.test(String(entry.humanAcceptance.date || ""))) {
        issues.push({ code: "manifest-entry-missing-human-acceptance-date", example: example || null });
      }
    }
    if (!nonEmptyString(entry.acceptanceReason)) {
      issues.push({ code: "manifest-entry-missing-acceptance-reason", example: example || null });
    } else if (entry.acceptanceReason.length > 240) {
      issues.push({ code: "manifest-entry-acceptance-reason-too-long", example: example || null });
    }
  }
  return { issues };
}

async function validateManifestContract(acceptance, result) {
  const entry = acceptance.entry;
  const issues = [];
  const render = result.render || {};
  const grammar = result.grammar || {};
  const evidence = result.evidence || {};
  const screenshot = entry.screenshot || {};
  const example = acceptance.example;

  const hasNativeEvidence = evidence.usesNativeEvidence === true || Boolean(evidence.protectedGrae10);
  if (entry.requiresNativeEvidence === true && !hasNativeEvidence) {
    issues.push({ code: "manifest-native-evidence-contract-mismatch", example });
  }
  if (entry.expectedPrimaryVisualGrammar !== grammar.expected) {
    issues.push({
      code: "manifest-grammar-contract-mismatch",
      example,
      expected: entry.expectedPrimaryVisualGrammar,
      actual: grammar.expected ?? null,
    });
  }
  if (screenshot.generatedReport) {
    const expectedReport = resolveManifestPath(screenshot.generatedReport);
    if (expectedReport !== DEFAULT_REGRESSION_REPORT && expectedReport !== REGRESSION_REPORT) {
      issues.push({
        code: "manifest-screenshot-report-mismatch",
        example,
        expected: [
          relativeFromRoot(DEFAULT_REGRESSION_REPORT),
          relativeFromRoot(REGRESSION_REPORT),
        ],
        actual: screenshot.generatedReport,
      });
    }
  }
  if (screenshot.file) {
    const expectedFile = resolveManifestPath(screenshot.file);
    const actualFile = render.screenshot ? resolve(render.screenshot) : null;
    if (actualFile !== expectedFile) {
      issues.push({
        code: "manifest-screenshot-file-mismatch",
        example,
        expected: screenshot.file,
        actual: render.screenshot ?? null,
      });
    }
  }
  if (screenshot.requiresPageScreenshot !== false && !(await fileLooksPresent(render.screenshot))) {
    issues.push({
      code: "manifest-page-screenshot-contract-mismatch",
      example,
      screenshot: render.screenshot ?? null,
    });
  }
  if (screenshot.requiresCanvasScreenshot !== false && !(await fileLooksPresent(render.canvasScreenshot))) {
    issues.push({
      code: "manifest-canvas-screenshot-contract-mismatch",
      example,
      canvasScreenshot: render.canvasScreenshot ?? null,
    });
  }
  const protectedGrae10Ok = evidence.protectedGrae10?.ok === true && result.status?.heroAccepted === true;
  if (example === "grae10-metric-engine" && !protectedGrae10Ok) {
    issues.push({ code: "manifest-grae10-protected-contract-mismatch", example });
  }
  return issues;
}

function resolveManifestPath(path) {
  return resolve(isAbsolute(path) ? path : join(ROOT, path));
}

function relativeFromRoot(path) {
  const resolved = resolve(path);
  const rel = relative(ROOT, resolved);
  return rel.startsWith("..") ? resolved : rel;
}

function nonEmptyString(value) {
  return typeof value === "string" && value.trim().length > 0;
}

async function fileLooksPresent(path) {
  if (!path) return false;
  try {
    const info = await stat(path);
    return info.isFile() && info.size > 1024;
  } catch {
    return false;
  }
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
