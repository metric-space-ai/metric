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
import { dirname, join, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const DEFAULT_REGRESSION_REPORT = resolve(ROOT, "output", "visual", "check-visual-regression-public-examples", "results.json");
const REVIEW_MANIFEST = resolve(ROOT, "docs", "visual", "hero-screenshot-review.json");
const OUT_DIR = resolve(process.env.METRIC_VISUAL_HERO_REVIEW_OUT || join(ROOT, "output", "visual", "check-hero-screenshot-review"));
const REGRESSION_REPORT = resolve(process.env.METRIC_VISUAL_REGRESSION_REPORT || DEFAULT_REGRESSION_REPORT);

const PROTECTED_ACCEPTED = new Map([
  ["grae10-metric-engine", "protected-grae10-golden-reference"],
]);

async function main() {
  const regression = JSON.parse(await readFile(REGRESSION_REPORT, "utf8"));
  const manifest = await loadOptionalManifest(REVIEW_MANIFEST);
  const accepted = acceptedMap(manifest);
  const issues = [];
  const rows = [];

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
  const acceptedReason = accepted.get(result.name) || null;
  const heroAccepted = status.heroAccepted === true || status.category === "hero-accepted";
  const acceptedByReview = acceptedReason != null;

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

  if (acceptedByReview && !heroAccepted) {
    issues.push({
      code: "review-accepted-but-regression-not-hero-accepted",
      example: result.name,
      statusCategory: status.category ?? null,
    });
  }

  return {
    name: result.name,
    statusCategory: status.category ?? null,
    heroAccepted,
    acceptedByReview,
    acceptedReason,
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

function acceptedMap(manifest) {
  const out = new Map(PROTECTED_ACCEPTED);
  for (const entry of manifest?.accepted || []) {
    if (!entry || entry.accepted !== true || !entry.example) continue;
    out.set(String(entry.example), String(entry.reason || "manual-screenshot-review"));
  }
  return out;
}

async function loadOptionalManifest(path) {
  if (!existsSync(path)) return null;
  return JSON.parse(await readFile(path, "utf8"));
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
