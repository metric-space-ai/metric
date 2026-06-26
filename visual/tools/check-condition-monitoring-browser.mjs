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
const SETTLE_MS = 1200;
const CLOSE_TIMEOUT_MS = 5000;

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
  let context = null;
  const result = {
    ok: false,
    url: `${baseUrl}/visual/examples/condition-monitoring-hero/index.html?verify=1`,
    evidence: "docs/examples/assets/condition-monitoring/metric.visual.json",
    screenshot: join(OUT_DIR, "W4-condition-monitoring-hero.png"),
  };

  try {
    const { chromium } = await importPlaywright();
    browser = await chromium.launch({
      headless: true,
      args: ["--enable-webgl", "--ignore-gpu-blocklist", "--use-gl=swiftshader"],
    });
    context = await browser.newContext({ viewport: VIEWPORT, deviceScaleFactor: 1 });
    const page = await context.newPage();
    page.setDefaultNavigationTimeout(NAV_TIMEOUT_MS);
    page.setDefaultTimeout(READY_TIMEOUT_MS);

    const consoleErrors = [];
    const pageErrors = [];
    page.on("console", (message) => {
      if (message.type() === "error") consoleErrors.push(message.text());
    });
    page.on("pageerror", (error) => pageErrors.push(error.message));

    const response = await page.goto(result.url, { waitUntil: "domcontentloaded", timeout: NAV_TIMEOUT_MS });
    result.httpStatus = response?.status() ?? null;
    result.httpOk = Boolean(response?.ok());
    await page.waitForFunction(() => document.documentElement.dataset.metricConditionHero === "ready", null, { timeout: READY_TIMEOUT_MS });
    await page.waitForFunction(() => {
      const state = window.metricConditionHero?.getState?.();
      return Number(state?.runtime?.layerInstanceCount ?? state?.layerInstanceCount ?? 0) > 0;
    }, null, { timeout: READY_TIMEOUT_MS });
    await page.waitForTimeout(SETTLE_MS);

    result.probe = await page.evaluate(conditionProbe);
    result.hoverTarget = await page.evaluate(selectVisibleConditionTarget);
    if (!result.hoverTarget?.visible) {
      throw new Error(`No visible condition-monitoring preview target: ${JSON.stringify(result.hoverTarget)}`);
    }

    await page.mouse.move(result.hoverTarget.x, result.hoverTarget.y);
    await page.waitForTimeout(220);
    result.hoverPreview = await page.evaluate(previewPanelProbe);

    await page.evaluate((recordId) => {
      window.metricConditionHero.runtime.selectRecord(recordId, { source: "condition-monitoring-browser-check" });
    }, result.hoverTarget.id);
    await page.waitForTimeout(220);
    result.selectionPreview = await page.evaluate(selectionProbe);

    await mkdir(OUT_DIR, { recursive: true });
    await page.screenshot({ path: result.screenshot, fullPage: false, timeout: 60000 });
    const screenshotStat = await stat(result.screenshot);
    result.screenshotBytes = screenshotStat.size;
    result.consoleErrors = consoleErrors;
    result.pageErrors = pageErrors;
    result.ok = result.httpOk
      && consoleErrors.length === 0
      && pageErrors.length === 0
      && result.probe?.recordCount === 528
      && result.probe?.previewRecordCount === 528
      && includesAll(result.probe?.regimes, ["normal", "drift", "fault", "recovery"])
      && includesAll(result.probe?.primitives, ["HeatFieldLayer", "CurveRibbonLayer", "BillboardLabelLayer"])
      && result.probe?.field?.viewClass === "PropertyFieldView"
      && result.probe?.field?.algorithmicComputation === false
      && result.probe?.trajectory?.viewClass === "TrajectoryPathView"
      && result.hoverPreview?.visible === true
      && result.hoverPreview?.sparklineCount >= 1
      && /process window/i.test(result.hoverPreview?.title || "")
      && result.selectionPreview?.selectedRecordId === result.hoverTarget.id
      && result.selectionPreview?.payloadKind === "time_series"
      && result.selectionPreview?.seriesLength >= 8
      && result.selectionPreview?.source === "condition-monitoring-browser-check"
      && result.probe?.debugUiCount === 0
      && screenshotStat.size > 10000;
  } catch (error) {
    result.error = error instanceof Error ? error.message : String(error);
  } finally {
    if (context) await closeWithTimeout(context.close(), "browser context close timed out");
    if (browser) await closeWithTimeout(browser.close(), "browser close timed out");
    await closeServer(server);
  }

  console.log(JSON.stringify(result, null, 2));
  if (!result.ok) process.exitCode = 1;
}

function conditionProbe() {
  const handle = window.metricConditionHero;
  const state = handle?.getState?.() || {};
  const descriptors = Array.isArray(state.descriptors) ? state.descriptors : [];
  const records = Array.isArray(state.document?.records) ? state.document.records : [];
  const truthProperty = (state.document?.properties || []).find((property) => property.id === "truth-regime");
  const regimes = Array.from(new Set((truthProperty?.values || []).map((entry) => String(entry.value)))).sort();
  const field = descriptors.find((descriptor) => descriptor?.primitive === "HeatFieldLayer");
  const trajectory = descriptors.find((descriptor) => descriptor?.primitive === "CurveRibbonLayer" || descriptor?.primitive === "CurveTubeMeshLayer");
  const labels = descriptors.find((descriptor) => descriptor?.primitive === "BillboardLabelLayer");
  return {
    dataset: { ...document.documentElement.dataset },
    recordCount: records.length,
    previewRecordCount: handle?.visual?.previewRecords?.length || 0,
    regimes,
    primitives: descriptors.map((descriptor) => descriptor?.primitive || descriptor?.kind).filter(Boolean),
    field: field ? {
      propertyId: field.metadata?.propertyId || field.source?.propertyId || null,
      viewClass: field.metadata?.viewClass || null,
      algorithmicComputation: field.metadata?.algorithmicComputation,
      recordCount: field.metadata?.recordCount || null,
    } : null,
    trajectory: trajectory ? {
      viewClass: trajectory.metadata?.viewClass || null,
      graphId: trajectory.metadata?.nativeEvidence?.graphId || trajectory.metadata?.graphId || null,
      recordCount: trajectory.metadata?.recordCount || null,
    } : null,
    labelCount: labels?.metadata?.labelCount || 0,
    debugUiCount: document.querySelectorAll("[data-debug], .debug, #debug, .devtools, #devtools").length,
    loadingText: Array.from(document.querySelectorAll(".loading")).map((node) => node.textContent.trim()).join(" | "),
  };
}

function selectVisibleConditionTarget() {
  const handle = window.metricConditionHero;
  const view = handle?.visual?.views?.[0];
  const state = handle?.getState?.() || {};
  const truthProperty = (state.document?.properties || []).find((property) => property.id === "truth-regime");
  const regimeByRecord = new Map((truthProperty?.values || []).map((entry) => [String(entry.record_id), String(entry.value)]));
  const priority = ["fault", "drift", "recovery", "normal"];
  const recordIds = Array.isArray(view?.recordIds) ? view.recordIds.map(String) : [];
  const positions = view?.positions;
  const rect = document.getElementById("scene")?.getBoundingClientRect() || { left: 0, top: 0, width: innerWidth, height: innerHeight };
  const pixelRatio = handle?.runtime?.renderer?.size?.pixelRatio || devicePixelRatio || 1;
  const candidates = [];
  for (const recordId of recordIds) {
    const position = positions?.get?.(recordId);
    if (!position) continue;
    const projected = handle.runtime.camera.projectToPixel(position, {});
    if (!projected || projected.visible === false || !Number.isFinite(projected.x) || !Number.isFinite(projected.y)) continue;
    const x = rect.left + projected.x / pixelRatio;
    const y = rect.top + projected.y / pixelRatio;
    const visible = x >= 48 && y >= 80 && x <= innerWidth - 48 && y <= innerHeight - 48;
    if (!visible) continue;
    const regime = regimeByRecord.get(recordId) || "";
    candidates.push({ id: recordId, regime, x, y, visible: true, rank: priority.indexOf(regime) });
  }
  candidates.sort((a, b) => (a.rank < 0 ? 99 : a.rank) - (b.rank < 0 ? 99 : b.rank));
  return candidates[0] || { visible: false, recordIds: recordIds.length };
}

function previewPanelProbe() {
  const panel = document.querySelector(".mtrc-record-preview");
  return {
    visible: panel?.dataset.visible === "true",
    title: panel?.querySelector("strong")?.textContent || null,
    text: panel?.textContent?.replace(/\s+/g, " ").trim().slice(0, 300) || "",
    sparklineCount: panel?.querySelectorAll(".mtrc-record-preview__sparkline").length || 0,
  };
}

function selectionProbe() {
  const state = window.metricConditionHero?.runtime?.getState?.() || {};
  const preview = state.selectedRecordPreview || state.selectionPreview || {};
  const record = state.selectedRecord || {};
  return {
    selectedRecordId: state.selectedRecordId || null,
    source: state.selectionSource || null,
    payloadKind: preview.payloadKind || record.payload?.kind || null,
    seriesLength: Array.isArray(record.payload?.series)
      ? record.payload.series.length
      : Number(preview.diagnostics?.seriesCount || 0),
    previewKind: preview.kind || null,
    title: preview.title || record.label || null,
  };
}

function includesAll(values, required) {
  const set = new Set(Array.isArray(values) ? values : []);
  return required.every((value) => set.has(value));
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
      "Install Playwright for browser-backed condition-monitoring checks or set METRIC_VISUAL_PLAYWRIGHT_MODULE to an installed playwright package directory.",
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

function closeServer(server) {
  return new Promise((resolveClose) => {
    server.close(() => resolveClose());
  });
}

async function closeWithTimeout(promise, label) {
  try {
    await Promise.race([
      promise,
      new Promise((_, reject) => setTimeout(() => reject(new Error(label)), CLOSE_TIMEOUT_MS)),
    ]);
  } catch {
    // Shutdown failures should not hide the browser verification result.
  }
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
