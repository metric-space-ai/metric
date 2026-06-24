#!/usr/bin/env node
/*
 * Performance benchmark matrix.
 *
 * Loads the synthetic point-cloud harness at a range of record counts in
 * headless Chromium and records the median frame time / FPS for each. Writes a
 * report to runtime/visual-checks/perf-matrix.json and prints a table.
 *
 *   node visual/tools/perf-matrix.mjs
 *   METRIC_PERF_COUNTS=1000,10000,60000,120000 node visual/tools/perf-matrix.mjs
 *
 * Note: headless swiftshader is a CPU rasterizer, so absolute numbers are a
 * floor, not GPU performance — the matrix is for relative scaling + regressions.
 */

import { createServer } from "node:http";
import { readFile, writeFile, mkdir } from "node:fs/promises";
import { existsSync, readdirSync } from "node:fs";
import { createRequire } from "node:module";
import { homedir } from "node:os";
import { dirname, extname, join, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO_ROOT = resolve(HERE, "..", "..");
const OUT = resolve(REPO_ROOT, "runtime", "visual-checks", "perf-matrix.json");
const COUNTS = (process.env.METRIC_PERF_COUNTS || "1000,10000,30000,60000,100000").split(",").map((n) => Number(n.trim())).filter((n) => n > 0);
const MIME = { ".html": "text/html", ".js": "text/javascript", ".mjs": "text/javascript", ".json": "application/json", ".css": "text/css" };

async function importPlaywright() {
  const require = createRequire(import.meta.url);
  try {
    return await import("playwright");
  } catch (importError) {
    try {
      return require("playwright");
    } catch {
      // Continue with explicit development-runtime candidates below.
    }
    const tried = [];
    for (const candidate of playwrightModuleCandidates()) {
      if (!candidate || !existsSync(candidate)) continue;
      tried.push(candidate);
      try {
        return require(candidate);
      } catch {
        // Keep searching; the final error lists every explicit candidate.
      }
    }
    const message = [
      "Cannot find module 'playwright'.",
      "Install Playwright for browser-backed performance checks or set METRIC_VISUAL_PLAYWRIGHT_MODULE to an installed playwright package directory.",
      `Original dynamic import error: ${importError instanceof Error ? importError.message : String(importError)}`,
      tried.length ? `Tried explicit candidates: ${tried.join(", ")}` : "No explicit candidates existed.",
    ].join(" ");
    throw new Error(message);
  }
}

function playwrightModuleCandidates() {
  const candidates = [
    process.env.METRIC_VISUAL_PLAYWRIGHT_MODULE,
    resolve(REPO_ROOT, "node_modules", "playwright"),
    resolve(REPO_ROOT, "visual", "node_modules", "playwright"),
  ];
  const codexRuntimeRoot = join(homedir(), ".cache", "codex-runtimes");
  if (existsSync(codexRuntimeRoot)) {
    for (const entry of readdirSync(codexRuntimeRoot, { withFileTypes: true })) {
      if (!entry.isDirectory()) continue;
      candidates.push(join(codexRuntimeRoot, entry.name, "dependencies", "node", "node_modules", "playwright"));
    }
  }
  return candidates.filter(Boolean);
}

function startServer(rootDir) {
  return new Promise((resolveServer) => {
    const server = createServer(async (request, response) => {
      try {
        let pathname = decodeURIComponent(new URL(request.url, "http://127.0.0.1").pathname);
        if (pathname.endsWith("/")) pathname += "index.html";
        const data = await readFile(resolve(rootDir, `.${pathname}`));
        response.writeHead(200, { "content-type": MIME[extname(pathname)] || "application/octet-stream" }).end(data);
      } catch { response.writeHead(404).end("nf"); }
    });
    server.listen(0, "127.0.0.1", () => resolveServer(server));
  });
}

async function main() {
  const server = await startServer(REPO_ROOT);
  const baseUrl = `http://127.0.0.1:${server.address().port}`;
  const { chromium } = await importPlaywright();
  const browser = await chromium.launch({ headless: true, args: ["--enable-webgl", "--ignore-gpu-blocklist", "--use-gl=swiftshader"] });

  const rows = [];
  for (const count of COUNTS) {
    const context = await browser.newContext({ viewport: { width: 1000, height: 700 } });
    const page = await context.newPage();
    let row = { n: count, ok: false };
    try {
      await page.goto(`${baseUrl}/visual/tools/perf-harness.html?n=${count}`, { waitUntil: "domcontentloaded", timeout: 30000 });
      await page.waitForFunction(() => document.documentElement.dataset.metricPerfReady === "true", null, { timeout: 60000 });
      const perf = await page.evaluate(() => window.metricPerf);
      row = { n: count, ok: true, medianMs: perf.medianMs, fps: perf.fps, instances: perf.layerInstanceCount };
    } catch (error) {
      row = { n: count, ok: false, error: error.message };
    } finally {
      await context.close();
    }
    rows.push(row);
  }

  await browser.close();
  server.close();

  await mkdir(dirname(OUT), { recursive: true });
  const report = { renderer: "swiftshader (headless CPU)", generatedCounts: COUNTS, rows };
  await writeFile(OUT, JSON.stringify(report, null, 2), "utf8");

  const ok = rows.every((row) => row.ok);
  console.log("METRIC Visual performance matrix (headless swiftshader floor)\n");
  console.log("    records |  median ms |    fps | instances");
  console.log("  ----------+------------+--------+----------");
  for (const row of rows) {
    if (row.ok) {
      console.log(`  ${String(row.n).padStart(9)} | ${String(row.medianMs).padStart(10)} | ${String(row.fps).padStart(6)} | ${String(row.instances).padStart(8)}`);
    } else {
      console.log(`  ${String(row.n).padStart(9)} | ${"error".padStart(10)} | ${row.error}`);
    }
  }
  console.log(`\nreport: ${OUT}`);
  if (!ok) process.exitCode = 1;
}

main().catch((error) => { console.error(error); process.exitCode = 1; });
