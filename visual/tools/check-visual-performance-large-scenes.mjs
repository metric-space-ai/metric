#!/usr/bin/env node
/*
 * Browser-backed large-scene performance gate.
 *
 * Uses the existing visual/tools/perf-harness.html with a dependency-free CDP
 * runner. It records record count, descriptor count, runtime layer count, frame
 * timing, and GPU buffer/draw diagnostics. Missing Chrome is a failed missing
 * gate, not a pass.
 *
 * Artifacts:
 *   output/visual/check-visual-performance-large-scenes/results.json
 */

import { createServer } from "node:http";
import { mkdtemp, mkdir, readFile, rm, writeFile } from "node:fs/promises";
import { existsSync } from "node:fs";
import { tmpdir } from "node:os";
import { dirname, extname, join, resolve } from "node:path";
import { spawn } from "node:child_process";
import { fileURLToPath } from "node:url";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const OUT_DIR = resolve(process.env.METRIC_VISUAL_PERF_OUT || join(ROOT, "output", "visual", "check-visual-performance-large-scenes"));
const COUNTS = parseCounts(process.env.METRIC_VISUAL_PERF_COUNTS || "1000,10000,60000");
const VIEWPORT = parseViewport(process.env.METRIC_VISUAL_PERF_VIEWPORT || "1000x700");
const READY_TIMEOUT_MS = Number(process.env.METRIC_VISUAL_PERF_READY_TIMEOUT_MS || 90000);
const DEFAULT_MEDIAN_BUDGET_MS = Number(process.env.METRIC_VISUAL_PERF_MEDIAN_BUDGET_MS || 150);
const DEFAULT_MIN_FRAMES = Number(process.env.METRIC_VISUAL_PERF_MIN_FRAMES || 10);
const FORCE_SWIFTSHADER = process.env.METRIC_VISUAL_FORCE_SWIFTSHADER === "1";

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

const GPU_DIAGNOSTICS_SCRIPT = String.raw`
(() => {
  if (window.__metricVisualGpuDiagnostics) return;
  const diagnostics = {
    bufferDataCalls: 0,
    totalBufferBytes: 0,
    drawArraysCalls: 0,
    drawElementsCalls: 0,
    instancedDrawCalls: 0,
    drawCalls: 0,
    drawVertices: 0,
    drawInstances: 0,
  };
  Object.defineProperty(window, "__metricVisualGpuDiagnostics", { value: diagnostics, configurable: false });
  const byteLength = (value) => {
    if (typeof value === "number") return value;
    if (value && typeof value.byteLength === "number") return value.byteLength;
    return 0;
  };
  const patch = (proto, name, wrap) => {
    if (!proto || typeof proto[name] !== "function" || proto[name].__metricVisualPatched) return;
    const original = proto[name];
    const next = wrap(original);
    next.__metricVisualPatched = true;
    proto[name] = next;
  };
  const patchProto = (proto) => {
    patch(proto, "bufferData", (original) => function bufferDataPatched(target, dataOrSize, usage) {
      diagnostics.bufferDataCalls += 1;
      diagnostics.totalBufferBytes += byteLength(dataOrSize);
      return original.apply(this, arguments);
    });
    patch(proto, "drawArrays", (original) => function drawArraysPatched(mode, first, count) {
      diagnostics.drawArraysCalls += 1;
      diagnostics.drawCalls += 1;
      diagnostics.drawVertices += Number(count) || 0;
      return original.apply(this, arguments);
    });
    patch(proto, "drawElements", (original) => function drawElementsPatched(mode, count, type, offset) {
      diagnostics.drawElementsCalls += 1;
      diagnostics.drawCalls += 1;
      diagnostics.drawVertices += Number(count) || 0;
      return original.apply(this, arguments);
    });
    patch(proto, "drawArraysInstanced", (original) => function drawArraysInstancedPatched(mode, first, count, instanceCount) {
      diagnostics.instancedDrawCalls += 1;
      diagnostics.drawCalls += 1;
      diagnostics.drawVertices += (Number(count) || 0) * Math.max(1, Number(instanceCount) || 0);
      diagnostics.drawInstances += Number(instanceCount) || 0;
      return original.apply(this, arguments);
    });
    patch(proto, "drawElementsInstanced", (original) => function drawElementsInstancedPatched(mode, count, type, offset, instanceCount) {
      diagnostics.instancedDrawCalls += 1;
      diagnostics.drawCalls += 1;
      diagnostics.drawVertices += (Number(count) || 0) * Math.max(1, Number(instanceCount) || 0);
      diagnostics.drawInstances += Number(instanceCount) || 0;
      return original.apply(this, arguments);
    });
    patch(proto, "getExtension", (original) => function getExtensionPatched(name) {
      const extension = original.apply(this, arguments);
      if (extension && String(name).toLowerCase() === "angle_instanced_arrays") {
        patch(extension, "drawArraysInstancedANGLE", (drawOriginal) => function drawArraysInstancedAnglePatched(mode, first, count, instanceCount) {
          diagnostics.instancedDrawCalls += 1;
          diagnostics.drawCalls += 1;
          diagnostics.drawVertices += (Number(count) || 0) * Math.max(1, Number(instanceCount) || 0);
          diagnostics.drawInstances += Number(instanceCount) || 0;
          return drawOriginal.apply(this, arguments);
        });
        patch(extension, "drawElementsInstancedANGLE", (drawOriginal) => function drawElementsInstancedAnglePatched(mode, count, type, offset, instanceCount) {
          diagnostics.instancedDrawCalls += 1;
          diagnostics.drawCalls += 1;
          diagnostics.drawVertices += (Number(count) || 0) * Math.max(1, Number(instanceCount) || 0);
          diagnostics.drawInstances += Number(instanceCount) || 0;
          return drawOriginal.apply(this, arguments);
        });
      }
      return extension;
    });
  };
  patchProto(window.WebGLRenderingContext && window.WebGLRenderingContext.prototype);
  patchProto(window.WebGL2RenderingContext && window.WebGL2RenderingContext.prototype);
})();
`;

const PERF_PROBE_SCRIPT = String.raw`
(() => {
  const perf = window.metricPerf || null;
  const runtime = window.metricPerfRuntime?.getState?.() || null;
  const canvas = document.querySelector("canvas");
  const rect = canvas?.getBoundingClientRect?.();
  return {
    ready: document.documentElement.dataset.metricPerfReady === "true",
    perf,
    runtime: runtime ? {
      layerDescriptorCount: runtime.layerDescriptorCount,
      layerInstanceCount: runtime.layerInstanceCount,
      layerState: runtime.layerState?.status || null,
      running: runtime.running,
      warnings: (runtime.warnings || []).map((warning) => warning?.code || String(warning)),
    } : null,
    gpuDiagnostics: window.__metricVisualGpuDiagnostics || null,
    canvas: rect ? {
      width: Math.round(rect.width),
      height: Math.round(rect.height),
      canvasWidth: canvas.width,
      canvasHeight: canvas.height,
    } : null,
  };
})();
`;

async function main() {
  await mkdir(OUT_DIR, { recursive: true });
  const chromeExecutable = findChromeExecutable();
  if (!chromeExecutable) {
    const report = {
      ok: false,
      reason: "browser-unavailable",
      message: "No Chrome/Chromium executable found; browser-backed performance gate is missing, not passing.",
      counts: COUNTS,
    };
    await writeReport(report);
    console.log(JSON.stringify(report, null, 2));
    process.exitCode = 1;
    return;
  }

  let server = null;
  let browser = null;
  const rows = [];
  try {
    server = await startStaticServer(ROOT);
    browser = await launchChrome(chromeExecutable);
    const baseUrl = `http://127.0.0.1:${server.address().port}`;
    for (const count of COUNTS) {
      rows.push(await checkCount(browser, `${baseUrl}/visual/tools/perf-harness.html?n=${count}`, count));
    }
  } catch (error) {
    rows.push({
      recordCount: null,
      ok: false,
      issues: [{ code: "runner-error", message: error instanceof Error ? error.message : String(error) }],
    });
  } finally {
    if (browser) await browser.close();
    if (server) server.close();
  }

  const report = {
    ok: rows.every((row) => row.ok),
    generatedAt: new Date().toISOString(),
    chromeExecutable,
    viewport: VIEWPORT,
    renderer: FORCE_SWIFTSHADER ? "headless Chrome with forced SwiftShader" : "headless Chrome using available WebGL backend",
    forcedSwiftShader: FORCE_SWIFTSHADER,
    budgets: {
      medianFrameMs: DEFAULT_MEDIAN_BUDGET_MS,
      minFrames: DEFAULT_MIN_FRAMES,
      requiredRuntimeLayers: 1,
      requiredDescriptorCount: 1,
      requiredGpuDrawCalls: 1,
      requiredBufferUploads: 1,
    },
    outDir: OUT_DIR,
    rows,
  };
  await writeReport(report);
  console.log(JSON.stringify(report, null, 2));
  if (!report.ok) process.exitCode = 1;
}

async function checkCount(browser, url, count) {
  let page = null;
  const issues = [];
  let probe = null;
  try {
    page = await browser.newPage();
    page.consoleErrors = [];
    page.pageErrors = [];
    await page.prepare();
    await page.navigate(url);
    probe = await waitForPerfReady(page);
  } catch (error) {
    issues.push({ code: "browser-check-failed", message: error instanceof Error ? error.message : String(error) });
  } finally {
    if (page) await page.close().catch(() => {});
  }

  const perf = probe?.perf || {};
  const runtime = probe?.runtime || {};
  const gpu = probe?.gpuDiagnostics || {};
  if (!probe?.ready) issues.push({ code: "perf-not-ready" });
  const renderer = perf.renderer || {};
  const rendererText = [
    renderer.renderer,
    renderer.unmaskedRenderer,
    renderer.vendor,
    renderer.unmaskedVendor,
  ].filter(Boolean).join(" ").toLowerCase();
  const softwareRenderer = /swiftshader|software|llvmpipe/.test(rendererText);
  if (perf.n !== count) issues.push({ code: "record-count-mismatch", expected: count, actual: perf.n ?? null });
  if ((runtime.layerDescriptorCount || 0) < 1) issues.push({ code: "missing-layer-descriptor-count" });
  if ((runtime.layerInstanceCount || 0) < 1) issues.push({ code: "missing-runtime-layer-count" });
  if ((perf.frames || 0) < DEFAULT_MIN_FRAMES) issues.push({ code: "insufficient-frame-sample", min: DEFAULT_MIN_FRAMES, actual: perf.frames || 0 });
  if (!Number.isFinite(perf.medianMs) || perf.medianMs > DEFAULT_MEDIAN_BUDGET_MS) {
    issues.push({ code: "median-frame-budget-exceeded", budgetMs: DEFAULT_MEDIAN_BUDGET_MS, actualMs: perf.medianMs ?? null });
  }
  if ((gpu.bufferDataCalls || 0) < 1 || (gpu.totalBufferBytes || 0) < 1) issues.push({ code: "missing-gpu-buffer-diagnostics" });
  if ((gpu.drawCalls || 0) < 1) issues.push({ code: "missing-gpu-draw-diagnostics" });
  if (page?.consoleErrors?.length) issues.push({ code: "console-errors", count: page.consoleErrors.length, messages: page.consoleErrors });
  if (page?.pageErrors?.length) issues.push({ code: "page-errors", count: page.pageErrors.length, messages: page.pageErrors });

  return {
    recordCount: count,
    ok: issues.length === 0,
    descriptorCount: runtime.layerDescriptorCount ?? null,
    runtimeLayerCount: runtime.layerInstanceCount ?? null,
    runtimeLayerState: runtime.layerState ?? null,
    frameTimingSample: {
      frames: perf.frames ?? null,
      medianMs: perf.medianMs ?? null,
      fps: perf.fps ?? null,
    },
    renderer: {
      ...renderer,
      softwareRenderer,
    },
    gpuDiagnostics: {
      bufferDataCalls: gpu.bufferDataCalls ?? null,
      totalBufferBytes: gpu.totalBufferBytes ?? null,
      drawCalls: gpu.drawCalls ?? null,
      drawArraysCalls: gpu.drawArraysCalls ?? null,
      drawElementsCalls: gpu.drawElementsCalls ?? null,
      instancedDrawCalls: gpu.instancedDrawCalls ?? null,
      drawVertices: gpu.drawVertices ?? null,
      drawInstances: gpu.drawInstances ?? null,
    },
    canvas: probe?.canvas || null,
    issues,
  };
}

async function waitForPerfReady(page) {
  const deadline = Date.now() + READY_TIMEOUT_MS;
  let last = null;
  let lastError = null;
  while (Date.now() < deadline) {
    try {
      last = await evaluate(page, PERF_PROBE_SCRIPT, { timeoutMs: 30000 });
      if (last?.ready) return last;
    } catch (error) {
      lastError = error instanceof Error ? error.message : String(error);
    }
    await sleep(300);
  }
  throw new Error(`performance readiness timed out after ${READY_TIMEOUT_MS}ms: ${lastError || JSON.stringify(last)}`);
}

async function startStaticServer(rootDir) {
  return new Promise((resolveServer, rejectServer) => {
    const server = createServer(async (request, response) => {
      try {
        const url = new URL(request.url, "http://127.0.0.1");
        let pathname = decodeURIComponent(url.pathname);
        if (pathname === "/favicon.ico") {
          response.writeHead(204).end();
          return;
        }
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
    server.listen(0, "127.0.0.1", () => resolveServer(server));
  });
}

class Browser {
  constructor(process, port, userDataDir) {
    this.process = process;
    this.port = port;
    this.userDataDir = userDataDir;
  }

  async newPage() {
    const response = await fetch(`http://127.0.0.1:${this.port}/json/new?about:blank`, { method: "PUT" });
    if (!response.ok) throw new Error(`Chrome target create failed: ${response.status}`);
    const target = await response.json();
    return new CdpPage(target.webSocketDebuggerUrl, this.port, target.id);
  }

  async close() {
    if (this.process.exitCode == null) this.process.kill("SIGTERM");
    await new Promise((resolveClose) => {
      const timer = setTimeout(() => {
        if (this.process.exitCode == null) this.process.kill("SIGKILL");
        resolveClose();
      }, 1200);
      this.process.once("exit", () => {
        clearTimeout(timer);
        resolveClose();
      });
    });
    await rm(this.userDataDir, { recursive: true, force: true });
  }
}

class CdpPage {
  constructor(webSocketUrl, port, targetId) {
    this.webSocketUrl = webSocketUrl;
    this.port = port;
    this.targetId = targetId;
    this.nextId = 1;
    this.pending = new Map();
    this.listeners = new Map();
    this.consoleErrors = [];
    this.pageErrors = [];
    this.closed = false;
    this.socket = new WebSocket(webSocketUrl);
    this.ready = new Promise((resolveReady, rejectReady) => {
      const timer = setTimeout(() => rejectReady(new Error("CDP WebSocket connect timeout")), 10000);
      this.socket.addEventListener("open", () => {
        clearTimeout(timer);
        resolveReady();
      }, { once: true });
      this.socket.addEventListener("error", () => {
        clearTimeout(timer);
        rejectReady(new Error("CDP WebSocket connection failed"));
      }, { once: true });
    });
    this.socket.addEventListener("message", (event) => this.handleMessage(event.data));
    this.socket.addEventListener("close", () => {
      this.closed = true;
      for (const { reject } of this.pending.values()) reject(new Error("CDP WebSocket closed"));
      this.pending.clear();
    });
  }

  async prepare() {
    await this.ready;
    await this.send("Page.enable");
    await this.send("Runtime.enable");
    await this.send("Network.enable");
    await this.send("Log.enable").catch(() => {});
    await this.send("Emulation.setDeviceMetricsOverride", {
      width: VIEWPORT.width,
      height: VIEWPORT.height,
      deviceScaleFactor: 1,
      mobile: false,
    });
    await this.send("Page.addScriptToEvaluateOnNewDocument", { source: GPU_DIAGNOSTICS_SCRIPT });
    this.on("Runtime.consoleAPICalled", (event) => {
      if (event.type === "error") {
        this.consoleErrors.push((event.args || []).map((arg) => arg.value || arg.description || "").join(" "));
      }
    });
    this.on("Runtime.exceptionThrown", (event) => {
      this.pageErrors.push(event.exceptionDetails?.text || event.exceptionDetails?.exception?.description || "exception");
    });
    this.on("Log.entryAdded", (event) => {
      if (event.entry?.level === "error") this.consoleErrors.push(event.entry.text);
    });
  }

  async navigate(url) {
    const load = waitForEvent(this, "Page.loadEventFired", () => true, 45000);
    const result = await this.send("Page.navigate", { url });
    if (result.errorText) throw new Error(`navigation failed: ${result.errorText}`);
    await load;
  }

  send(method, params = {}) {
    const id = this.nextId++;
    const message = JSON.stringify({ id, method, params });
    const promise = new Promise((resolveSend, rejectSend) => {
      this.pending.set(id, { resolve: resolveSend, reject: rejectSend, method });
    });
    this.socket.send(message);
    return promise;
  }

  on(method, handler) {
    if (!this.listeners.has(method)) this.listeners.set(method, new Set());
    this.listeners.get(method).add(handler);
    return () => this.listeners.get(method)?.delete(handler);
  }

  handleMessage(data) {
    const message = JSON.parse(typeof data === "string" ? data : Buffer.from(data).toString("utf8"));
    if (message.id) {
      const pending = this.pending.get(message.id);
      if (!pending) return;
      this.pending.delete(message.id);
      if (message.error) pending.reject(new Error(`${pending.method}: ${message.error.message}`));
      else pending.resolve(message.result || {});
      return;
    }
    const handlers = this.listeners.get(message.method);
    if (handlers) {
      for (const handler of handlers) handler(message.params || {});
    }
  }

  async close() {
    if (!this.closed) this.socket.close();
    await fetch(`http://127.0.0.1:${this.port}/json/close/${this.targetId}`).catch(() => {});
  }
}

async function launchChrome(executable) {
  const port = await reservePort();
  const userDataDir = await mkdtemp(join(tmpdir(), "metric-visual-cdp-"));
  const args = [
    `--remote-debugging-port=${port}`,
    `--user-data-dir=${userDataDir}`,
    "--headless=new",
    "--no-first-run",
    "--no-default-browser-check",
    "--disable-background-networking",
    "--enable-webgl",
    "--ignore-gpu-blocklist",
    "about:blank",
  ];
  if (FORCE_SWIFTSHADER) {
    args.splice(args.length - 1, 0, "--enable-unsafe-swiftshader", "--use-angle=swiftshader");
  }
  const child = spawn(executable, args, { stdio: ["ignore", "ignore", "pipe"] });
  let stderr = "";
  child.stderr.on("data", (chunk) => {
    stderr += chunk.toString();
    if (stderr.length > 8000) stderr = stderr.slice(-8000);
  });
  const deadline = Date.now() + 15000;
  while (Date.now() < deadline) {
    try {
      const response = await fetch(`http://127.0.0.1:${port}/json/version`);
      if (response.ok) return new Browser(child, port, userDataDir);
    } catch {
      // keep polling
    }
    await sleep(150);
  }
  child.kill("SIGTERM");
  await rm(userDataDir, { recursive: true, force: true });
  throw new Error(`Chrome DevTools endpoint did not start. ${stderr.trim()}`);
}

async function evaluate(page, expression, options = {}) {
  const timeoutMs = options.timeoutMs || 10000;
  let timerId = null;
  const timer = new Promise((_, reject) => {
    timerId = setTimeout(() => reject(new Error("Runtime.evaluate timed out")), timeoutMs);
  });
  const call = page.send("Runtime.evaluate", {
    expression,
    awaitPromise: true,
    returnByValue: true,
    userGesture: true,
  });
  const result = await Promise.race([call, timer]).finally(() => clearTimeout(timerId));
  if (result.exceptionDetails) {
    throw new Error(result.exceptionDetails.text || result.exceptionDetails.exception?.description || "evaluation failed");
  }
  return result.result?.value;
}

function waitForEvent(page, method, predicate, timeoutMs) {
  return new Promise((resolveWait, rejectWait) => {
    const timer = setTimeout(() => {
      unsubscribe();
      rejectWait(new Error(`${method} timed out after ${timeoutMs}ms`));
    }, timeoutMs);
    const unsubscribe = page.on(method, (params) => {
      if (!predicate(params)) return;
      clearTimeout(timer);
      unsubscribe();
      resolveWait(params);
    });
  });
}

async function reservePort() {
  return new Promise((resolvePort, rejectPort) => {
    const server = createServer();
    server.on("error", rejectPort);
    server.listen(0, "127.0.0.1", () => {
      const port = server.address().port;
      server.close(() => resolvePort(port));
    });
  });
}

function findChromeExecutable() {
  const candidates = [
    process.env.METRIC_VISUAL_CHROME,
    "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome",
    "/Applications/Chromium.app/Contents/MacOS/Chromium",
    "/Applications/Microsoft Edge.app/Contents/MacOS/Microsoft Edge",
    "/usr/bin/google-chrome",
    "/usr/bin/chromium",
    "/usr/bin/chromium-browser",
  ].filter(Boolean);
  return candidates.find((candidate) => existsSync(candidate)) || null;
}

async function writeReport(report) {
  await writeFile(join(OUT_DIR, "results.json"), JSON.stringify(report, null, 2), "utf8");
}

function parseCounts(value) {
  return value.split(",").map((entry) => Number(entry.trim())).filter((entry) => Number.isFinite(entry) && entry > 0);
}

function parseViewport(value) {
  const match = /^(\d+)x(\d+)$/i.exec(value);
  return match ? { width: Number(match[1]), height: Number(match[2]) } : { width: 1000, height: 700 };
}

function sleep(ms) {
  return new Promise((resolveSleep) => setTimeout(resolveSleep, ms));
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
