#!/usr/bin/env node
/*
 * Single visual example checker.
 *
 * Opens every visual example in headless Chromium and reports, per example:
 * schema/load status, console errors, page errors, render readiness (a sized
 * canvas with a live WebGL context) and a screenshot for visual review.
 *
 * It bundles a tiny static server rooted at the repo so it is one command:
 *
 *   node visual/tools/check-visual-examples.mjs
 *
 * playwright is loaded from the local install or, failing that, from the npm
 * cache via NODE_PATH. Chromium must be installed (npx playwright install
 * chromium). Screenshots are written to runtime/visual-checks/ by default.
 *
 * Options (env):
 *   METRIC_VISUAL_EXAMPLES   comma-separated example dir names to limit the run
 *   METRIC_VISUAL_OUT        screenshot output directory
 *   METRIC_VISUAL_PORT       static server port (default: ephemeral)
 */

import { createServer } from "node:http";
import { readFile, readdir, mkdir, stat } from "node:fs/promises";
import { existsSync, readdirSync } from "node:fs";
import { createRequire } from "node:module";
import { homedir } from "node:os";
import { dirname, extname, resolve, join } from "node:path";
import { fileURLToPath } from "node:url";

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO_ROOT = resolve(HERE, "..", "..");
const EXAMPLES_DIR = resolve(REPO_ROOT, "visual", "examples");
const OUT_DIR = process.env.METRIC_VISUAL_OUT
  ? resolve(process.env.METRIC_VISUAL_OUT)
  : resolve(REPO_ROOT, "runtime", "visual-checks");

const MIME = {
  ".html": "text/html; charset=utf-8",
  ".js": "text/javascript; charset=utf-8",
  ".mjs": "text/javascript; charset=utf-8",
  ".json": "application/json; charset=utf-8",
  ".css": "text/css; charset=utf-8",
  ".png": "image/png",
  ".jpg": "image/jpeg",
  ".svg": "image/svg+xml",
  ".wasm": "application/wasm",
};

const NAV_TIMEOUT_MS = 20000;
const RENDER_TIMEOUT_MS = 30000;
const SETTLE_MS = 2200;

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
      "Install Playwright for browser-backed visual checks or set METRIC_VISUAL_PLAYWRIGHT_MODULE to an installed playwright package directory.",
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

function startStaticServer(rootDir, port) {
  return new Promise((resolveServer, rejectServer) => {
    const server = createServer(async (request, response) => {
      try {
        const url = new URL(request.url, "http://127.0.0.1");
        let pathname = decodeURIComponent(url.pathname);
        if (pathname.endsWith("/")) pathname += "index.html";
        const filePath = resolve(rootDir, `.${pathname}`);
        if (!filePath.startsWith(rootDir)) {
          response.writeHead(403).end("forbidden");
          return;
        }
        const data = await readFile(filePath);
        response.writeHead(200, { "content-type": MIME[extname(filePath)] || "application/octet-stream" });
        response.end(data);
      } catch {
        response.writeHead(404).end("not found");
      }
    });
    server.on("error", rejectServer);
    server.listen(port || 0, "127.0.0.1", () => resolveServer(server));
  });
}

async function discoverExamples() {
  const limit = process.env.METRIC_VISUAL_EXAMPLES
    ? new Set(process.env.METRIC_VISUAL_EXAMPLES.split(",").map((name) => name.trim()).filter(Boolean))
    : null;
  const entries = await readdir(EXAMPLES_DIR, { withFileTypes: true });
  const examples = [];
  for (const entry of entries) {
    if (!entry.isDirectory()) continue;
    if (limit && !limit.has(entry.name)) continue;
    const indexPath = join(EXAMPLES_DIR, entry.name, "index.html");
    try {
      await stat(indexPath);
      examples.push(entry.name);
    } catch {
      // directory without an index.html (e.g. fixtures) is skipped
    }
  }
  return examples.sort();
}

// Non-destructive readiness probe. It must NOT call canvas.getContext(): in
// headless swiftshader, requesting a context (even the same type) can disturb
// the live WebGL context and blank the next presented frame. Readiness is a
// sized canvas; the actual render signal is the layer-readiness wait plus the
// console-error checks below.
const RENDER_PROBE = () => {
  const canvas = document.querySelector("canvas");
  if (!canvas) return { ready: false, reason: "no-canvas" };
  const width = canvas.clientWidth || canvas.width;
  const height = canvas.clientHeight || canvas.height;
  if (!width || !height) return { ready: false, reason: "zero-size" };
  const datasetKeys = Object.keys(document.documentElement.dataset).filter((key) => key.toLowerCase().startsWith("metric"));
  return {
    ready: true,
    reason: "ok",
    width,
    height,
    canvasWidth: canvas.width,
    canvasHeight: canvas.height,
    canvasCount: document.querySelectorAll("canvas").length,
    datasetKeys,
  };
};

const RUNTIME_STATE_PROBE = () => {
  // Recognize any example's runtime handle generically: window.metric* objects
  // that expose getState() (directly or via .runtime).
  const handles = Object.keys(window)
    .filter((key) => /^metric/i.test(key))
    .map((key) => window[key])
    .filter((value) => value && typeof value === "object");
  for (const handle of handles) {
    const runtime = typeof handle.getState === "function" ? handle : handle.runtime || null;
    const state = runtime?.getState?.();
    if (state) {
      return {
        layerInstanceCount: state.layerInstanceCount ?? null,
        layerDescriptorCount: state.layerDescriptorCount ?? null,
        layerState: state.layerState?.status ?? null,
        layerErrors: (state.layerState?.errors || []).map((entry) => ({
          id: entry?.descriptor?.id || "",
          message: entry?.error?.message || String(entry?.error || entry),
        })),
        warnings: (state.warnings || []).map((warning) => warning.code),
      };
    }
  }
  return null;
};

async function checkExample(browser, baseUrl, name) {
  const url = `${baseUrl}/visual/examples/${name}/index.html?verify=1`;
  const context = await browser.newContext({ viewport: { width: 1280, height: 820 }, deviceScaleFactor: 1 });
  const page = await context.newPage();
  const consoleErrors = [];
  const pageErrors = [];
  page.on("console", (message) => {
    if (message.type() === "error") consoleErrors.push(message.text());
  });
  page.on("pageerror", (error) => pageErrors.push(error.message));

  const result = { name, url, ok: false };
  try {
    const response = await page.goto(url, { waitUntil: "domcontentloaded", timeout: NAV_TIMEOUT_MS });
    result.httpStatus = response ? response.status() : null;
    result.httpOk = Boolean(response && response.ok());

    let probe = { ready: false, reason: "not-evaluated" };
    const deadline = Date.now() + RENDER_TIMEOUT_MS;
    while (Date.now() < deadline) {
      probe = await page.evaluate(RENDER_PROBE);
      if (probe.ready) break;
      await page.waitForTimeout(250);
    }
    // When the example exposes a runtime handle, wait until its layers are
    // actually instantiated (the layer factory loads asynchronously) so the
    // screenshot reflects a drawn scene, not an empty frame.
    while (Date.now() < deadline) {
      const state = await page.evaluate(RUNTIME_STATE_PROBE);
      if (!state || (state.layerInstanceCount || 0) > 0) break;
      await page.waitForTimeout(200);
    }
    await page.waitForTimeout(SETTLE_MS);
    // Re-probe after the scene settles so reported size/dataset flags reflect
    // the final frame, not the first frame at context creation.
    result.render = await page.evaluate(RENDER_PROBE);
    result.runtimeState = await page.evaluate(RUNTIME_STATE_PROBE);

    await mkdir(OUT_DIR, { recursive: true });
    const screenshotPath = join(OUT_DIR, `${name}.png`);
    await page.screenshot({ path: screenshotPath, fullPage: false });
    result.screenshot = screenshotPath;

    result.consoleErrors = consoleErrors;
    result.pageErrors = pageErrors;
    result.ok = result.httpOk && probe.ready && consoleErrors.length === 0 && pageErrors.length === 0;
  } catch (error) {
    result.error = error.message;
    result.consoleErrors = consoleErrors;
    result.pageErrors = pageErrors;
  } finally {
    await context.close();
  }
  return result;
}

async function main() {
  const examples = await discoverExamples();
  if (!examples.length) {
    console.log(JSON.stringify({ ok: false, reason: "no-examples-found", examplesDir: EXAMPLES_DIR }, null, 2));
    process.exitCode = 1;
    return;
  }

  const port = process.env.METRIC_VISUAL_PORT ? Number(process.env.METRIC_VISUAL_PORT) : 0;
  const server = await startStaticServer(REPO_ROOT, port);
  const baseUrl = `http://127.0.0.1:${server.address().port}`;

  let browser = null;
  const results = [];
  try {
    const { chromium } = await importPlaywright();
    browser = await chromium.launch({
      headless: true,
      args: ["--enable-webgl", "--ignore-gpu-blocklist", "--use-gl=swiftshader"],
    });
    for (const name of examples) {
      results.push(await checkExample(browser, baseUrl, name));
    }
  } catch (error) {
    console.log(JSON.stringify({ ok: false, reason: "runner-error", message: error.message }, null, 2));
    process.exitCode = 1;
    if (browser) await browser.close();
    server.close();
    return;
  } finally {
    if (browser) await browser.close();
    server.close();
  }

  const failures = results.filter((entry) => !entry.ok);
  const ok = failures.length === 0;
  console.log(JSON.stringify({
    ok,
    baseUrl,
    outDir: OUT_DIR,
    total: results.length,
    failed: failures.length,
    results: results.map((entry) => ({
      name: entry.name,
      ok: entry.ok,
      httpStatus: entry.httpStatus,
      render: entry.render,
      runtimeState: entry.runtimeState,
      consoleErrors: entry.consoleErrors,
      pageErrors: entry.pageErrors,
      error: entry.error,
      screenshot: entry.screenshot,
    })),
  }, null, 2));
  if (!ok) process.exitCode = 1;
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
