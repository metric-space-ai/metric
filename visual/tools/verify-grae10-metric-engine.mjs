#!/usr/bin/env node

import { createHash } from "node:crypto";
import { readFile } from "node:fs/promises";
import { resolve } from "node:path";

const ROOT = resolve(new URL("../..", import.meta.url).pathname);
const EXAMPLE = resolve(ROOT, "visual/examples/grae10-metric-engine/index.html");
const DATA = resolve(ROOT, "visual/examples/grae10-metric-engine/grae10-data.json");
const GOLDEN = resolve(ROOT, "visual/regression-baselines/grae10-metric-engine.sha256");
const HTTP_PATH = "/visual/examples/grae10-metric-engine/index.html";

async function main() {
  const [html, dataText, goldenText] = await Promise.all([
    readFile(EXAMPLE, "utf8"),
    readFile(DATA, "utf8"),
    readFile(GOLDEN, "utf8"),
  ]);
  const expectedHash = goldenText.trim().split(/\s+/)[0];
  const actualHash = sha256(html);
  const checks = [];

  check(checks, actualHash === expectedHash, "index.html must match the GRAE10 golden hash", {
    expectedHash,
    actualHash,
  });

  const data = JSON.parse(dataText);
  check(checks, data.schema === "metric.visual.grae10.dataset.v1", "GRAE10 data schema must match", {
    actual: data.schema,
  });
  check(checks, data.recordCount === 60000, "GRAE10 data must contain 60,000 records", {
    actual: data.recordCount,
  });
  check(checks, typeof data.labels === "string" && data.labels.length === 60000, "label string must contain 60,000 entries", {
    actual: typeof data.labels === "string" ? data.labels.length : null,
  });
  check(checks, Array.isArray(data.p2) && data.p2.length === 120000, "2D coordinates must contain 60,000 pairs", {
    actual: Array.isArray(data.p2) ? data.p2.length : null,
  });
  check(checks, Array.isArray(data.p3) && data.p3.length === 180000, "3D coordinates must contain 60,000 triples", {
    actual: Array.isArray(data.p3) ? data.p3.length : null,
  });

  const requiredTokens = [
    "metric-webgl.module.js",
    "new BokehPass",
    "labelScene",
    "renderer.render(labelScene, camera)",
    "addFloor",
    "addGroundProjection",
    "TIMELINE",
    "requestAnimationFrame",
    "RecordPreviewPanel",
  ];
  for (const token of requiredTokens) {
    check(checks, html.includes(token), `index.html must keep ${token}`, {});
  }
  check(checks, !html.includes("visual/vendor/") && !html.includes("three-r127"), "index.html must not import vendored paths", {});

  const baseUrl = process.env.METRIC_VISUAL_BASE_URL;
  if (baseUrl) {
    const url = new URL(HTTP_PATH, normalizeBaseUrl(baseUrl));
    const served = await fetchText(url);
    check(checks, sha256(served) === expectedHash, "served GRAE10 page must match the golden hash byte-for-byte", {
      url: url.toString(),
      servedHash: sha256(served),
      expectedHash,
    });
  }

  const ok = checks.every((entry) => entry.ok);
  const result = { ok, checks };
  if (!ok) {
    console.error(JSON.stringify(result, null, 2));
    process.exit(1);
  }

  console.log(JSON.stringify(result, null, 2));
}

function check(checks, ok, message, details) {
  checks.push({ ok: Boolean(ok), message, details });
}

function sha256(text) {
  return createHash("sha256").update(text).digest("hex");
}

function normalizeBaseUrl(value) {
  const url = String(value || "").trim();
  if (!url) return "http://127.0.0.1:8011";
  return url.endsWith("/") ? url : `${url}/`;
}

async function fetchText(url) {
  const response = await fetch(url);
  if (!response.ok) {
    throw new Error(`HTTP ${response.status} for ${url.toString()}`);
  }
  return response.text();
}

main().catch((error) => {
  console.error(JSON.stringify({
    ok: false,
    message: error instanceof Error ? error.message : String(error),
  }, null, 2));
  process.exit(1);
});
