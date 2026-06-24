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

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const SITE = resolve(ROOT, "docs", "site", "index.html");
const PROGRESS = resolve(ROOT, "docs", "visual", "metric-visual-progress.md");
const EXAMPLES = resolve(ROOT, "visual", "examples");
const GRAE_INDEX = resolve(EXAMPLES, "grae10-metric-engine", "index.html");
const GRAE_BASELINE = resolve(ROOT, "visual", "regression-baselines", "grae10-metric-engine.sha256");

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
    if (evidence?.provenance?.synthetic === true) {
      result.push(entry.name);
    }
  }
  return result.sort();
}

function sha256(text) {
  return createHash("sha256").update(text).digest("hex");
}

function includesExampleReference(text, name) {
  return text.includes(`visual/examples/${name}/`)
    || text.includes(`visual/examples/${name}/index.html`)
    || text.includes(`../examples/${name}/`)
    || text.includes(`../../visual/examples/${name}/`);
}

async function main() {
  const issues = [];
  const site = await maybeRead(SITE);
  const progress = await maybeRead(PROGRESS);

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
  const publicSynthetic = synthetic.filter((name) => includesExampleReference(site, name));
  for (const name of publicSynthetic) {
    issues.push({
      code: "synthetic_example_on_public_site",
      example: name,
      evidence: `visual/examples/${name}/evidence.json`,
      page: "docs/site/index.html",
    });
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
    syntheticExamples: synthetic,
    publicSynthetic,
    syntheticDone,
    issues,
  }, null, 2));
  if (!ok) process.exitCode = 1;
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
