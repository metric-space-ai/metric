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
const BRIEF_MANIFEST = resolve(ROOT, "visual", "hero-visual-briefs.manifest.json");
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
  const briefManifest = await loadOptionalManifest(BRIEF_MANIFEST);
  const manifestValidation = validateManifest(manifest);
  const accepted = acceptedMap(manifest, manifestValidation.issues);
  const visualBriefs = previewBriefMap(briefManifest);
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
    const row = await reviewRow(result, accepted, visualBriefs);
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
    issues.push(...await validateManifestContract(entry, result, visualBriefs));
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

  const reviewGallery = join(OUT_DIR, "index.html");
  const report = {
    ok: issues.length === 0,
    generatedAt: new Date().toISOString(),
    regressionReport: REGRESSION_REPORT,
    regressionGeneratedAt: regression.generatedAt ?? null,
    manifest: existsSync(REVIEW_MANIFEST) ? REVIEW_MANIFEST : null,
    visualBriefManifest: existsSync(BRIEF_MANIFEST) ? BRIEF_MANIFEST : null,
    manifestSchemaVersion: manifest?.schemaVersion ?? null,
    manifestIssueCount: manifestValidation.issues.length,
    acceptedHeroes: rows.filter((row) => row.acceptedByReview).map((row) => row.name),
    reviewPendingPreviews: rows.filter((row) => row.reviewStatus === "review-pending").map((row) => row.name),
    acceptedHeroCount: rows.filter((row) => row.acceptedByReview).length,
    reviewPendingCount: rows.filter((row) => row.reviewStatus === "review-pending").length,
    reviewGallery,
    rows,
    issues,
  };

  await mkdir(OUT_DIR, { recursive: true });
  await writeFile(reviewGallery, renderReviewGalleryHtml(report));
  await writeFile(join(OUT_DIR, "results.json"), JSON.stringify(report, null, 2));
  console.log(JSON.stringify(report, null, 2));
  if (!report.ok) process.exitCode = 1;
}

async function reviewRow(result, accepted, visualBriefs = new Map()) {
  const issues = [];
  const status = result.status || {};
  const render = result.render || {};
  const screenshot = render.screenshot || null;
  const canvasScreenshot = render.canvasScreenshot || null;
  const acceptance = accepted.get(result.name) || null;
  const brief = visualBriefs.get(result.name) || null;
  const acceptedReason = acceptance?.reason || null;
  const heroAccepted = status.heroAccepted === true || status.category === "hero-accepted";
  const acceptedByReview = acceptance != null;

  const screenshotOk = await fileLooksPresent(screenshot);
  const canvasScreenshotOk = await fileLooksPresent(canvasScreenshot);
  const visualScreenshotOk = screenshotOk || canvasScreenshotOk;
  if (!canvasScreenshotOk) {
    issues.push({ code: "missing-canvas-screenshot", example: result.name, canvasScreenshot });
  }
  if (!visualScreenshotOk) {
    issues.push({ code: "missing-review-screenshot", example: result.name, screenshot, canvasScreenshot });
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
    visualScreenshotOk,
    visualClaim: brief?.visualClaim || null,
    primaryVisualGrammar: brief?.primaryVisualGrammar || null,
    expectedPrimaryVisualGrammar: result.grammar?.expected || brief?.expectedPrimaryVisualGrammar || null,
    requiredPrimitives: Array.isArray(brief?.requiredPrimitives) ? brief.requiredPrimitives.slice() : [],
    acceptanceBlockers: Array.isArray(brief?.acceptanceBlockers) ? brief.acceptanceBlockers.slice() : [],
    nativeEvidence: result.evidence?.usesNativeEvidence === true || Boolean(result.evidence?.protectedGrae10),
    grammarOk: result.grammar?.ok === true,
    renderOk: render.ok === true,
    regressionOk: result.ok === true,
    issues,
  };
}

function renderReviewGalleryHtml(report) {
  const rows = report.rows || [];
  return `<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>METRIC Visual Hero Screenshot Review</title>
  <style>
    :root {
      color-scheme: light;
      --paper: #f4f3ea;
      --ink: #20282b;
      --muted: #667175;
      --line: #cbd2cb;
      --accepted: #1d6f55;
      --pending: #9b5b17;
      --issue: #9c2f25;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      background: var(--paper);
      color: var(--ink);
      font: 14px/1.45 ui-sans-serif, system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
    }
    header {
      padding: 32px 36px 24px;
      border-bottom: 1px solid var(--line);
    }
    h1 {
      margin: 0 0 10px;
      font-size: 28px;
      line-height: 1.05;
      letter-spacing: 0;
    }
    header p {
      margin: 0;
      max-width: 74ch;
      color: var(--muted);
    }
    .summary {
      display: flex;
      flex-wrap: wrap;
      gap: 16px;
      padding: 18px 36px;
      border-bottom: 1px solid var(--line);
      color: var(--muted);
    }
    .summary strong { color: var(--ink); }
    main {
      display: grid;
      gap: 28px;
      padding: 28px 36px 44px;
    }
    section {
      border: 1px solid var(--line);
      background: rgba(255, 255, 250, 0.62);
    }
    .meta {
      display: grid;
      grid-template-columns: minmax(180px, 0.65fr) minmax(260px, 1.35fr);
      gap: 22px;
      padding: 22px;
      border-bottom: 1px solid var(--line);
    }
    h2 {
      margin: 0 0 8px;
      font-size: 20px;
      line-height: 1.15;
      letter-spacing: 0;
    }
    .status {
      display: inline-flex;
      align-items: center;
      gap: 8px;
      margin-top: 4px;
      font-size: 12px;
      font-weight: 700;
      text-transform: uppercase;
      letter-spacing: 0.08em;
    }
    .status.accepted { color: var(--accepted); }
    .status.pending { color: var(--pending); }
    .claim {
      margin: 0 0 12px;
      max-width: 78ch;
    }
    .details {
      display: grid;
      grid-template-columns: repeat(2, minmax(0, 1fr));
      gap: 10px 18px;
      margin-top: 14px;
      color: var(--muted);
      font-size: 12px;
    }
    .details b { color: var(--ink); }
    .blockers {
      margin: 14px 0 0;
      padding: 0;
      list-style: none;
      color: var(--issue);
      font-size: 12px;
    }
    .blockers li + li { margin-top: 4px; }
    .shots {
      display: grid;
      grid-template-columns: repeat(2, minmax(0, 1fr));
      gap: 1px;
      background: var(--line);
    }
    figure {
      margin: 0;
      background: #ecede5;
    }
    img {
      display: block;
      width: 100%;
      height: auto;
    }
    figcaption {
      padding: 9px 12px 11px;
      color: var(--muted);
      font-size: 12px;
      border-top: 1px solid var(--line);
      background: rgba(255, 255, 250, 0.72);
    }
    code {
      font-family: ui-monospace, SFMono-Regular, Menlo, Consolas, monospace;
      font-size: 0.94em;
    }
    @media (max-width: 900px) {
      .meta, .shots { grid-template-columns: 1fr; }
      header, .summary, main { padding-left: 18px; padding-right: 18px; }
    }
  </style>
</head>
<body>
  <header>
    <h1>METRIC Visual Hero Screenshot Review</h1>
    <p>This artifact is generated from the browser regression screenshots. It is evidence for human review, not automatic hero acceptance.</p>
  </header>
  <div class="summary">
    <span><strong>${escapeHtml(String(report.acceptedHeroCount ?? 0))}</strong> accepted</span>
    <span><strong>${escapeHtml(String(report.reviewPendingCount ?? 0))}</strong> review pending</span>
    <span>generated <strong>${escapeHtml(report.generatedAt || "")}</strong></span>
    <span>regression <code>${escapeHtml(relativeFromRoot(report.regressionReport || ""))}</code></span>
  </div>
  <main>
    ${rows.map(renderReviewRowHtml).join("\n")}
  </main>
</body>
</html>
`;
}

function renderReviewRowHtml(row) {
  const accepted = row.acceptedByReview === true;
  const blockers = Array.isArray(row.acceptanceBlockers) ? row.acceptanceBlockers : [];
  return `<section id="${escapeHtml(row.name)}">
  <div class="meta">
    <div>
      <h2>${escapeHtml(row.name)}</h2>
      <div class="status ${accepted ? "accepted" : "pending"}">${accepted ? "accepted" : "review pending"}</div>
    </div>
    <div>
      <p class="claim">${escapeHtml(row.visualClaim || "No visual claim recorded.")}</p>
      <div class="details">
        <span><b>Grammar:</b> ${escapeHtml(row.expectedPrimaryVisualGrammar || row.primaryVisualGrammar || "unknown")}</span>
        <span><b>View:</b> ${escapeHtml(row.selectedViewKind || "unknown")}</span>
        <span><b>Runtime layers:</b> ${escapeHtml(String(row.runtimeLayerCount ?? "unknown"))}</span>
        <span><b>Native evidence:</b> ${row.nativeEvidence ? "yes" : "no"}</span>
        <span><b>Grammar gate:</b> ${row.grammarOk ? "pass" : "fail"}</span>
        <span><b>Render gate:</b> ${row.renderOk ? "pass" : "fail"}</span>
      </div>
      ${blockers.length ? `<ul class="blockers">${blockers.map((blocker) => `<li>${escapeHtml(blocker)}</li>`).join("")}</ul>` : ""}
    </div>
  </div>
  <div class="shots">
    ${renderFigure("Page screenshot", row.screenshot, row.screenshotOk)}
    ${renderFigure("Canvas screenshot", row.canvasScreenshot, row.canvasScreenshotOk)}
  </div>
</section>`;
}

function renderFigure(label, path, ok) {
  if (!ok || !path) {
    return `<figure><figcaption>${escapeHtml(label)} missing</figcaption></figure>`;
  }
  return `<figure>
  <img src="${escapeAttribute(relativeAssetPath(path))}" alt="${escapeAttribute(label)}">
  <figcaption>${escapeHtml(label)}: <code>${escapeHtml(relativeFromRoot(path))}</code></figcaption>
</figure>`;
}

function relativeAssetPath(path) {
  if (!path) return "";
  const rel = relative(OUT_DIR, resolve(path));
  return rel.split(/[\\/]/g).map(encodeURIComponent).join("/");
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

function previewBriefMap(manifest) {
  const out = new Map();
  for (const entry of manifest?.previewBriefs || []) {
    if (!entry || typeof entry !== "object" || !entry.exampleId) continue;
    out.set(String(entry.exampleId), entry);
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

async function validateManifestContract(acceptance, result, visualBriefs = new Map()) {
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
  const brief = visualBriefs.get(example);
  const blockers = Array.isArray(brief?.acceptanceBlockers)
    ? brief.acceptanceBlockers.filter(nonEmptyString)
    : [];
  if (example !== "grae10-metric-engine" && blockers.length > 0) {
    issues.push({
      code: "manifest-accepts-open-visual-brief-blockers",
      example,
      blockers,
    });
  }
  if (example !== "grae10-metric-engine" && brief?.reviewStatus === "review-pending") {
    issues.push({
      code: "manifest-accepts-review-pending-visual-brief",
      example,
      reviewStatus: brief.reviewStatus,
    });
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

function escapeHtml(value) {
  return String(value ?? "")
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;")
    .replaceAll('"', "&quot;")
    .replaceAll("'", "&#39;");
}

function escapeAttribute(value) {
  return escapeHtml(value).replaceAll("`", "&#96;");
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
