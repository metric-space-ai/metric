#!/usr/bin/env node
/*
 * Public gallery evidence gate.
 *
 * This check protects the project page from two regressions:
 * 1. replacing the accepted 60k GRAE10 visual with a different page, and
 * 2. publishing synthetic visual fixtures as real hero applications.
 *
 * Synthetic fixtures are allowed to exist for engine development. They are not
 * allowed to be linked from the public project page as live hero evidence.
 */

import { createHash } from "node:crypto";
import { readFile, readdir, stat } from "node:fs/promises";
import { dirname, join, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import {
  isExplicitNativeMetricVisualExport,
  isSyntheticMetricVisualEvidence,
} from "../src/data/provenance.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const SITE = resolve(ROOT, "docs", "site", "index.html");
const PROGRESS = resolve(ROOT, "docs", "visual", "metric-visual-progress.md");
const EXAMPLES = resolve(ROOT, "visual", "examples");
const GRAE_INDEX = resolve(EXAMPLES, "grae10-metric-engine", "index.html");
const GRAE_BASELINE = resolve(ROOT, "visual", "regression-baselines", "grae10-metric-engine.sha256");
const PUBLIC_REFERENCE_EXEMPTIONS = new Set(["grae10-metric-engine"]);

async function maybeRead(path) {
  try {
    return await readFile(path, "utf8");
  } catch {
    return "";
  }
}

async function fileExists(path) {
  try {
    await stat(path);
    return true;
  } catch {
    return false;
  }
}

async function syntheticExamples() {
  const entries = await readdir(EXAMPLES, { withFileTypes: true });
  const result = [];
  for (const entry of entries) {
    if (!entry.isDirectory()) continue;
    const evidencePath = join(EXAMPLES, entry.name, "evidence.json");
    if (!(await fileExists(evidencePath))) continue;
    const evidence = JSON.parse(await readFile(evidencePath, "utf8"));
    if (isSyntheticMetricVisualEvidence(evidence?.provenance)) {
      result.push(entry.name);
    }
  }
  return result.sort();
}

function sha256(text) {
  return createHash("sha256").update(text).digest("hex");
}

function extractPublicExampleNames(text) {
  return Array.from(
    new Set(Array.from(text.matchAll(/visual\/examples\/([^/"']+)\/index\.html/g), (match) => match[1])),
  ).sort();
}

function resolveExampleFetchTarget(name, target) {
  if (!target || /^https?:\/\//.test(target)) return null;
  if (target.startsWith("/")) return resolve(ROOT, `.${target}`);
  return resolve(EXAMPLES, name, target);
}

async function publicSyntheticEvidenceReferencesForExample(name) {
  const index = await maybeRead(resolve(EXAMPLES, name, "index.html"));
  const references = [];
  for (const target of extractFetchTargets(index)) {
    if (!/\.json(?:[?#].*)?$/i.test(target)) continue;
    const assetPath = resolveExampleFetchTarget(name, target);
    if (!assetPath || !assetPath.startsWith(ROOT) || !(await fileExists(assetPath))) continue;
    try {
      const document = JSON.parse(await readFile(assetPath, "utf8"));
      if (isSyntheticMetricVisualEvidence(document?.provenance)) {
        references.push({ target, path: assetPath });
      }
    } catch {
      // Unreadable JSON is reported by the native asset pass if it is a public
      // native target. Non-native JSON is outside this evidence gate.
    }
  }
  return references;
}

function extractFetchTargets(index) {
  const constants = new Map();
  const constantPattern = /const\s+([A-Z0-9_]+)\s*=\s*["']([^"']+)["']/g;
  for (const match of index.matchAll(constantPattern)) {
    constants.set(match[1], match[2]);
  }

  const targets = [];
  const fetchPattern = /fetch\(\s*(?:(["'])([^"']+)\1|([A-Z0-9_]+))\s*\)/g;
  for (const match of index.matchAll(fetchPattern)) {
    if (match[2]) {
      targets.push(match[2]);
    } else if (match[3] && constants.has(match[3])) {
      targets.push(constants.get(match[3]));
    }
  }
  return targets;
}

async function publicNativeAssetsForExample(name) {
  const index = await maybeRead(resolve(EXAMPLES, name, "index.html"));
  const targets = extractFetchTargets(index);
  const assets = [];
  for (const target of targets) {
    if (!/docs\/examples\/assets\/.+\/metric\.visual\.json$/.test(target)) continue;
    const assetPath = resolveExampleFetchTarget(name, target);
    if (!assetPath || !assetPath.startsWith(ROOT)) continue;
    assets.push({ target, assetPath });
  }
  return assets;
}

async function main() {
  const issues = [];
  const site = await maybeRead(SITE);
  const progress = await maybeRead(PROGRESS);
  const publicExamples = extractPublicExampleNames(site).filter((name) => !PUBLIC_REFERENCE_EXEMPTIONS.has(name));

  const expectedHash = (await maybeRead(GRAE_BASELINE)).trim().split(/\s+/)[0] || "";
  const actualHash = sha256(await readFile(GRAE_INDEX));
  if (!expectedHash) {
    issues.push({ code: "missing_grae10_baseline", path: GRAE_BASELINE });
  } else if (actualHash !== expectedHash) {
    issues.push({
      code: "grae10_reference_changed",
      path: GRAE_INDEX,
      expected: expectedHash,
      actual: actualHash,
    });
  }

  const synthetic = await syntheticExamples();
  const publicSynthetic = [];
  const publicNativeAssets = [];
  for (const name of publicExamples) {
    const syntheticReferences = await publicSyntheticEvidenceReferencesForExample(name);
    for (const reference of syntheticReferences) {
      publicSynthetic.push({ example: name, target: reference.target });
      issues.push({
        code: "synthetic_example_on_public_site",
        example: name,
        evidence: reference.target,
        page: "docs/site/index.html",
      });
    }
    const nativeAssets = await publicNativeAssetsForExample(name);
    if (!nativeAssets.length) {
      issues.push({
        code: "public_example_without_native_metric_visual_asset",
        example: name,
        page: `visual/examples/${name}/index.html`,
      });
      continue;
    }
    for (const asset of nativeAssets) {
      publicNativeAssets.push({ example: name, target: asset.target });
      try {
        const document = JSON.parse(await readFile(asset.assetPath, "utf8"));
        if (document?.schema !== "metric.visual.v1") {
          issues.push({
            code: "public_native_asset_wrong_schema",
            example: name,
            target: asset.target,
            schema: document?.schema ?? null,
          });
        }
        if (isSyntheticMetricVisualEvidence(document?.provenance)) {
          issues.push({
            code: "public_native_asset_marked_synthetic",
            example: name,
            target: asset.target,
          });
        }
        if (!isExplicitNativeMetricVisualExport(document?.provenance)) {
          issues.push({
            code: "public_native_asset_missing_explicit_native_export",
            example: name,
            target: asset.target,
          });
        }
      } catch (error) {
        issues.push({
          code: "public_native_asset_unreadable",
          example: name,
          target: asset.target,
          message: error.message,
        });
      }
    }
  }

  const syntheticDone = synthetic.filter((name) => {
    const index = progress.indexOf(name);
    if (index < 0) return false;
    const lineStart = progress.lastIndexOf("\n", index) + 1;
    const lineEnd = progress.indexOf("\n", index);
    const line = progress.slice(lineStart, lineEnd < 0 ? undefined : lineEnd);
    return /\bdone\b/i.test(line);
  });
  for (const name of syntheticDone) {
    issues.push({
      code: "synthetic_example_marked_done",
      example: name,
      report: "docs/visual/metric-visual-progress.md",
    });
  }

  const ok = issues.length === 0;
  console.log(JSON.stringify({
    ok,
    grae10Hash: actualHash,
    publicExamples,
    syntheticExamples: synthetic,
    publicSynthetic,
    publicNativeAssets,
    syntheticDone,
    issues,
  }, null, 2));
  if (!ok) process.exitCode = 1;
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
