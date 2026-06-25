#!/usr/bin/env node

import { createServer } from "node:http";
import { mkdir, readFile, stat } from "node:fs/promises";
import { existsSync, readdirSync } from "node:fs";
import { createRequire } from "node:module";
import { homedir } from "node:os";
import { dirname, extname, join, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const OUT_DIR = resolve(ROOT, "visual", "output");
const VIEWPORT = { width: 1280, height: 820 };
const NAV_TIMEOUT_MS = 30000;
const READY_TIMEOUT_MS = 45000;
const FRAME_TIMEOUT_MS = Number(process.env.METRIC_VISUAL_FRAME_TIMEOUT_MS || 90000);
const SETTLE_MS = 1200;
const CLOSE_TIMEOUT_MS = 5000;

const FRAMES = [
  {
    id: "source",
    expectedProgress: "0.000",
    url: "/visual/examples/mapping-dimensionality-hero/index.html?frame=source&verify=1",
    screenshot: "W6-mapping-dimensionality-hero-source.png",
  },
  {
    id: "transition",
    expectedProgress: "0.500",
    url: "/visual/examples/mapping-dimensionality-hero/index.html?frame=transition&verify=1",
    screenshot: "W6-mapping-dimensionality-hero-transition.png",
  },
  {
    id: "target",
    expectedProgress: "1.000",
    url: "/visual/examples/mapping-dimensionality-hero/index.html?frame=target&verify=1",
    screenshot: "W6-mapping-dimensionality-hero-target.png",
  },
];

const MIME = {
  ".html": "text/html; charset=utf-8",
  ".js": "text/javascript; charset=utf-8",
  ".mjs": "text/javascript; charset=utf-8",
  ".json": "application/json; charset=utf-8",
  ".css": "text/css; charset=utf-8",
  ".png": "image/png",
  ".jpg": "image/jpeg",
  ".jpeg": "image/jpeg",
  ".svg": "image/svg+xml",
  ".wasm": "application/wasm",
};

async function main() {
  const server = await startStaticServer(ROOT, Number(process.env.METRIC_VISUAL_PORT || 0));
  const baseUrl = `http://127.0.0.1:${server.address().port}`;
  let browser = null;
  const results = [];
  try {
    const { chromium } = await importPlaywright();
    browser = await chromium.launch({
      headless: true,
      args: ["--enable-webgl", "--ignore-gpu-blocklist", "--use-gl=swiftshader"],
    });
    for (const frame of FRAMES) {
      results.push(await captureFrame(browser, baseUrl, frame));
    }
  } finally {
    if (browser) await closeBrowser(browser);
    await closeServer(server);
  }

  const failures = results.filter((result) => !result.ok);
  const ok = failures.length === 0;
  console.log(JSON.stringify({
    ok,
    baseUrl,
    total: results.length,
    failed: failures.length,
    results,
  }, null, 2));
  if (!ok) process.exitCode = 1;
}

async function captureFrame(browser, baseUrl, frame) {
  const context = await browser.newContext({ viewport: VIEWPORT, deviceScaleFactor: 1 });
  const page = await context.newPage();
  page.setDefaultNavigationTimeout(Math.min(NAV_TIMEOUT_MS, FRAME_TIMEOUT_MS));
  page.setDefaultTimeout(Math.min(READY_TIMEOUT_MS, FRAME_TIMEOUT_MS));
  const consoleErrors = [];
  const pageErrors = [];
  page.on("console", (message) => {
    if (message.type() === "error") consoleErrors.push(message.text());
  });
  page.on("pageerror", (error) => pageErrors.push(error.message));

  const screenshotPath = join(OUT_DIR, frame.screenshot);
  const result = {
    frame: frame.id,
    url: `${baseUrl}${frame.url}`,
    screenshot: screenshotPath,
    ok: false,
  };
  const deadline = setTimeout(() => {
    result.timedOut = true;
    page.close({ runBeforeUnload: false }).catch(() => {});
    context.close().catch(() => {});
  }, FRAME_TIMEOUT_MS);

  try {
    const response = await page.goto(result.url, { waitUntil: "domcontentloaded", timeout: NAV_TIMEOUT_MS });
    result.httpStatus = response?.status() ?? null;
    result.httpOk = Boolean(response?.ok());
    await page.waitForFunction(() => document.documentElement.dataset.metricMappingHero === "ready", null, { timeout: READY_TIMEOUT_MS });
    await page.waitForFunction(() => {
      const state = window.metricMappingHero?.getState?.();
      return Number(state?.runtime?.layerInstanceCount ?? state?.layerInstanceCount ?? 0) > 0;
    }, null, { timeout: READY_TIMEOUT_MS });
    await page.waitForTimeout(SETTLE_MS);
    result.probe = await page.evaluate(() => {
      const state = window.metricMappingHero?.getState?.();
      const descriptors = Array.isArray(state?.descriptors) ? state.descriptors : [];
      const residualLayers = descriptors
        .filter((descriptor) => descriptor?.metadata?.role === "residual/error")
        .map((descriptor) => ({
          id: descriptor.id,
          primitive: descriptor.primitive,
          recordCount: descriptor.metadata?.recordCount ?? descriptor.channels?.recordId?.count ?? null,
          residualPropertyId: descriptor.metadata?.residualPropertyId ?? null,
          residualChannelCount: descriptor.channels?.residual?.count ?? null,
        }));
      return {
        title: document.title,
        dataset: { ...document.documentElement.dataset },
        descriptorCount: descriptors.length,
        primitives: descriptors.map((descriptor) => descriptor.primitive || descriptor.kind).filter(Boolean),
        residualLayers,
        runtimeLayerCount: state?.runtime?.layerInstanceCount ?? state?.layerInstanceCount ?? null,
        canvas: Array.from(document.querySelectorAll("canvas")).map((canvas) => ({
          width: canvas.clientWidth,
          height: canvas.clientHeight,
          backingWidth: canvas.width,
          backingHeight: canvas.height,
        }))[0] || null,
      };
    });
    await mkdir(OUT_DIR, { recursive: true });
    await page.screenshot({ path: screenshotPath, fullPage: false, timeout: Math.min(FRAME_TIMEOUT_MS, 60000) });
    const screenshotStat = await stat(screenshotPath);
    result.screenshotBytes = screenshotStat.size;
    result.consoleErrors = consoleErrors;
    result.pageErrors = pageErrors;
    result.ok = result.httpOk
      && consoleErrors.length === 0
      && pageErrors.length === 0
      && result.probe?.dataset?.metricMappingHero === "ready"
      && result.probe?.dataset?.metricRecordCount === "1000"
      && result.probe?.dataset?.metricMorphProgress === frame.expectedProgress
      && result.probe?.residualLayers?.some((layer) => layer.recordCount === 1000 && layer.residualChannelCount === 1000)
      && screenshotStat.size > 10000;
  } catch (error) {
    result.error = error instanceof Error ? error.message : String(error);
    result.consoleErrors = consoleErrors;
    result.pageErrors = pageErrors;
  } finally {
    clearTimeout(deadline);
    await closeContext(context);
  }
  return result;
}

async function closeContext(context) {
  try {
    await withTimeout(context.close(), CLOSE_TIMEOUT_MS, "browser context close timed out");
  } catch {
    // The check already captured page errors above; shutdown failures should
    // not hang the process after the verification result is known.
  }
}

async function closeBrowser(browser) {
  try {
    await withTimeout(browser.close(), CLOSE_TIMEOUT_MS, "browser close timed out");
  } catch {
    try {
      browser.process?.()?.kill?.();
    } catch {
      // Best-effort cleanup only.
    }
  }
}

function closeServer(server) {
  return new Promise((resolveClose) => {
    server.close(() => resolveClose());
  });
}

function withTimeout(promise, timeoutMs, label) {
  return Promise.race([
    promise,
    new Promise((_, reject) => {
      setTimeout(() => reject(new Error(label)), timeoutMs);
    }),
  ]);
}

async function importPlaywright() {
  const require = createRequire(import.meta.url);
  try {
    return await import("playwright");
  } catch (importError) {
    try {
      return require("playwright");
    } catch {
      // Continue with explicit candidates below.
    }
    const tried = [];
    for (const candidate of playwrightModuleCandidates()) {
      if (!candidate || !existsSync(candidate)) continue;
      tried.push(candidate);
      try {
        return require(candidate);
      } catch {
        // Keep searching.
      }
    }
    throw new Error([
      "Cannot find module 'playwright'.",
      "Install Playwright for browser-backed visual checks or set METRIC_VISUAL_PLAYWRIGHT_MODULE to an installed playwright package directory.",
      `Original dynamic import error: ${importError instanceof Error ? importError.message : String(importError)}`,
      tried.length ? `Tried explicit candidates: ${tried.join(", ")}` : "No explicit candidates existed.",
    ].join(" "));
  }
}

function playwrightModuleCandidates() {
  const candidates = [
    process.env.METRIC_VISUAL_PLAYWRIGHT_MODULE,
    resolve(ROOT, "node_modules", "playwright"),
    resolve(ROOT, "visual", "node_modules", "playwright"),
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

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
