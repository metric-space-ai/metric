#!/usr/bin/env node
/*
 * Direct-runtime harness quarantine gate.
 *
 * Public examples should enter METRIC Visual through createMetricVisual() and
 * semantic show*() commands. Direct MetricVisualRuntime setup and standalone
 * renderer modules are allowed only for explicitly listed diagnostics or the
 * protected GRAE10 reference.
 */

import { readFile, readdir } from "node:fs/promises";
import { dirname, join, relative, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const SITE = resolve(ROOT, "docs", "site", "index.html");
const ACCEPTANCE_MANIFEST = resolve(ROOT, "visual", "hero-acceptance.manifest.json");
const EXAMPLES = resolve(ROOT, "visual", "examples");
const PUBLIC_INDEX = resolve(ROOT, "visual", "src", "index.js");

const ALLOWED_DIRECT_RUNTIME_HARNESSES = new Map([
  ["visual/examples/grae10-metric-engine/index.html", {
    category: "protected-accepted-reference",
    reason: "Protected 60k GRAE10 golden page is the current accepted public reference.",
    allowedWhenPublic: true,
    allowedWhenAccepted: true,
  }],
  ["visual/examples/grae10-metric-engine/index.original.html", {
    category: "protected-reference-copy",
    reason: "Original GRAE10 reference copy retained for golden comparison only.",
    allowedWhenPublic: false,
    allowedWhenAccepted: false,
  }],
  ["visual/examples/grae10-engine-module/index.html", {
    category: "protected-module-diagnostic",
    reason: "GRAE10 module smoke page for the protected renderer.",
    allowedWhenPublic: false,
    allowedWhenAccepted: false,
  }],
  ["visual/examples/native-engine-probe/index.html", {
    category: "internal-diagnostic-harness",
    reason: "Dev-only process-curve miniature runtime probe.",
    allowedWhenPublic: false,
    allowedWhenAccepted: false,
    requiredMarker: /\bdata-metric-visual-surface=["']internal-diagnostic-harness["']|\bmetricVisualProbeMetadata\b/,
  }],
  ["visual/examples/miniature-hero-frame/index.html", {
    category: "internal-miniature-style-frame",
    reason: "Style/frame diagnostic that is not linked from the public project gallery.",
    allowedWhenPublic: false,
    allowedWhenAccepted: false,
  }],
  ["visual/examples/miniature-look-gallery/index.html", {
    category: "internal-miniature-style-atlas",
    reason: "Style atlas diagnostic that compares miniature looks outside the public gallery.",
    allowedWhenPublic: false,
    allowedWhenAccepted: false,
  }],
  ["visual/examples/process-curve-condition-monitoring/index.html", {
    category: "legacy-process-curve-dev-page",
    reason: "Legacy process-curve inspection page retained outside public hero acceptance.",
    allowedWhenPublic: false,
    allowedWhenAccepted: false,
  }],
]);

const DIRECT_RUNTIME_PATTERNS = [
  ["MetricVisualRuntime symbol", /\bMetricVisualRuntime\b/],
  ["MetricVisualRuntime construction", /\bnew\s+MetricVisualRuntime\b/],
  ["layer factory", /\bcreateLayerFromDescriptor\b/],
  ["miniature field renderer", /from\s+["'][^"']*\/src\/miniature-field(?:\/index)?\.js["']|\b(?:InstancedBoxField|createInstancedBoxField)\b/],
  ["miniature engine renderer", /from\s+["'][^"']*\/src\/miniature-engine(?:\/index)?\.js["']|\bmountGrae10MetricEngine\b/],
  ["metric-webgl renderer module", /from\s+["'][^"']*\/src\/runtime\/metric-webgl\//],
  ["manual metric-webgl renderer", /\bnew\s+MGL\.(?:Scene|PerspectiveCamera|WebGLRenderer)\b|\bEffectComposer\b/],
];

async function main() {
  const issues = [];
  const site = await readText(SITE);
  const manifest = JSON.parse(await readText(ACCEPTANCE_MANIFEST));
  const publicExamples = extractPublicExampleNames(site);
  const acceptedExamples = extractAcceptedExampleNames(manifest);
  const exampleFiles = await listHtmlFiles(EXAMPLES);
  const directRuntimeMatches = [];

  for (const file of exampleFiles) {
    const rel = normalize(relative(ROOT, file));
    const text = await readText(file);
    const matches = DIRECT_RUNTIME_PATTERNS
      .filter(([, pattern]) => pattern.test(text))
      .map(([label]) => label);
    if (!matches.length) continue;

    const exampleId = exampleIdFromRelativePath(rel);
    const isCanonicalIndex = /\/index\.html$/.test(rel);
    const allow = ALLOWED_DIRECT_RUNTIME_HARNESSES.get(rel) || null;
    const isPublic = isCanonicalIndex && publicExamples.includes(exampleId);
    const isAccepted = isCanonicalIndex && acceptedExamples.includes(exampleId);
    directRuntimeMatches.push({
      path: rel,
      exampleId,
      matches,
      allowed: Boolean(allow),
      category: allow?.category || null,
      publicExample: isPublic,
      acceptedHero: isAccepted,
    });

    if (!allow) {
      issues.push({ code: "unlisted-direct-runtime-harness", path: rel, matches });
      continue;
    }
    if (allow.requiredMarker && !allow.requiredMarker.test(text)) {
      issues.push({
        code: "allowed-harness-missing-internal-marker",
        path: rel,
        category: allow.category,
      });
    }
    if (isPublic && !allow.allowedWhenPublic) {
      issues.push({
        code: "public-gallery-direct-runtime-harness",
        path: rel,
        exampleId,
        category: allow.category,
      });
    }
    if (isAccepted && !allow.allowedWhenAccepted) {
      issues.push({
        code: "accepted-hero-direct-runtime-harness",
        path: rel,
        exampleId,
        category: allow.category,
      });
    }
  }

  const publicIndex = await readText(PUBLIC_INDEX);
  if (/export\s+\*\s+as\s+miniatureField\b/.test(publicIndex) || /from\s+["']\.\/miniature-field(?:\/index)?\.js["']/.test(publicIndex)) {
    issues.push({
      code: "public-miniature-field-export",
      path: "visual/src/index.js",
      message: "miniature-field must stay out of the root public export surface.",
    });
  }

  const report = {
    ok: issues.length === 0,
    publicExamples,
    acceptedExamples,
    allowedDirectRuntimeHarnesses: Array.from(ALLOWED_DIRECT_RUNTIME_HARNESSES, ([path, value]) => ({
      path,
      category: value.category,
      reason: value.reason,
      allowedWhenPublic: value.allowedWhenPublic,
      allowedWhenAccepted: value.allowedWhenAccepted,
    })),
    directRuntimeMatches,
    issues,
  };

  console.log(JSON.stringify(report, null, 2));
  if (!report.ok) process.exitCode = 1;
}

function extractPublicExampleNames(text) {
  return Array.from(
    new Set(Array.from(text.matchAll(/visual\/examples\/([^/"']+)\/index\.html/g), (match) => match[1])),
  ).sort();
}

function extractAcceptedExampleNames(manifest) {
  return (manifest?.acceptedHeroes || [])
    .map((entry) => entry?.exampleId)
    .filter((id) => typeof id === "string" && id.trim().length > 0)
    .sort();
}

function exampleIdFromRelativePath(path) {
  const match = path.match(/^visual\/examples\/([^/]+)\//);
  return match ? match[1] : null;
}

async function listHtmlFiles(dir) {
  const result = [];
  for (const entry of await readdir(dir, { withFileTypes: true })) {
    const path = join(dir, entry.name);
    if (entry.isDirectory()) {
      result.push(...await listHtmlFiles(path));
    } else if (entry.isFile() && path.endsWith(".html")) {
      result.push(path);
    }
  }
  return result.sort();
}

async function readText(path) {
  return readFile(path, "utf8");
}

function normalize(path) {
  return path.split("\\").join("/");
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
