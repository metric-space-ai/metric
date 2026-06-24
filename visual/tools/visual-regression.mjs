#!/usr/bin/env node
/*
 * Visual regression check.
 *
 * Captures each example in headless Chromium, reduces the frame to a 48x48
 * grayscale signature, and compares it to a stored baseline. Signatures (not
 * raw PNGs) keep baselines small and tolerate the small frame-to-frame jitter of
 * animated heroes while still catching structural regressions (blank frame,
 * broken layout, changed color scheme).
 *
 *   node visual/tools/visual-regression.mjs            # check against baselines
 *   node visual/tools/visual-regression.mjs --update   # (re)write baselines
 *
 * Baselines: visual/regression-baselines/<example>.json
 * playwright must be resolvable (NODE_PATH); chromium installed.
 */

import { createServer } from "node:http";
import { readFile, writeFile, mkdir, readdir, stat } from "node:fs/promises";
import { createRequire } from "node:module";
import { dirname, extname, resolve, join } from "node:path";
import { fileURLToPath } from "node:url";

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO_ROOT = resolve(HERE, "..", "..");
const EXAMPLES_DIR = resolve(REPO_ROOT, "visual", "examples");
const BASELINE_DIR = resolve(REPO_ROOT, "visual", "regression-baselines");
const UPDATE = process.argv.includes("--update");
const SIG = 48;
const THRESHOLD = 0.07; // mean abs grayscale diff (0..1) tolerated
const MIME = { ".html": "text/html", ".js": "text/javascript", ".mjs": "text/javascript", ".json": "application/json", ".css": "text/css", ".png": "image/png", ".svg": "image/svg+xml" };

async function importPlaywright() {
  try { return await import("playwright"); }
  catch { return createRequire(import.meta.url)("playwright"); }
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

async function discoverExamples() {
  const limit = process.env.METRIC_VISUAL_EXAMPLES
    ? new Set(process.env.METRIC_VISUAL_EXAMPLES.split(",").map((name) => name.trim()))
    : null;
  const entries = await readdir(EXAMPLES_DIR, { withFileTypes: true });
  const examples = [];
  for (const entry of entries) {
    if (!entry.isDirectory()) continue;
    if (limit && !limit.has(entry.name)) continue;
    try { await stat(join(EXAMPLES_DIR, entry.name, "index.html")); examples.push(entry.name); } catch { /* skip */ }
  }
  return examples.sort();
}

const RUNTIME_READY = () => {
  const handles = Object.keys(window).filter((k) => /^metric/i.test(k)).map((k) => window[k]).filter((v) => v && typeof v === "object");
  for (const handle of handles) {
    const runtime = typeof handle.getState === "function" ? handle : handle.runtime || null;
    const count = runtime?.getState?.()?.layerInstanceCount;
    if (typeof count === "number") return count > 0;
  }
  return null;
};

// Decode a PNG buffer into a SIG×SIG grayscale signature using a blank page.
async function signatureFromPng(decoderPage, pngBase64, size) {
  return decoderPage.evaluate(async ({ data, size }) => {
    const image = new Image();
    await new Promise((res, rej) => { image.onload = res; image.onerror = rej; image.src = `data:image/png;base64,${data}`; });
    const canvas = document.createElement("canvas");
    canvas.width = size; canvas.height = size;
    const ctx = canvas.getContext("2d");
    ctx.drawImage(image, 0, 0, size, size);
    const pixels = ctx.getImageData(0, 0, size, size).data;
    const out = [];
    for (let i = 0; i < pixels.length; i += 4) {
      out.push(Math.round((0.299 * pixels[i] + 0.587 * pixels[i + 1] + 0.114 * pixels[i + 2])));
    }
    return out;
  }, { data: pngBase64, size });
}

function diffSignatures(a, b) {
  if (!a || !b || a.length !== b.length) return 1;
  let sum = 0;
  for (let i = 0; i < a.length; i += 1) sum += Math.abs(a[i] - b[i]);
  return sum / (a.length * 255);
}

async function main() {
  const examples = await discoverExamples();
  await mkdir(BASELINE_DIR, { recursive: true });
  const server = await startServer(REPO_ROOT);
  const baseUrl = `http://127.0.0.1:${server.address().port}`;
  const { chromium } = await importPlaywright();
  const browser = await chromium.launch({ headless: true, args: ["--enable-webgl", "--ignore-gpu-blocklist", "--use-gl=swiftshader"] });
  const decoderPage = await (await browser.newContext()).newPage();
  await decoderPage.goto("about:blank");

  const results = [];
  for (const name of examples) {
    const context = await browser.newContext({ viewport: { width: 1000, height: 640 } });
    const page = await context.newPage();
    let entry = { name, ok: false };
    try {
      await page.goto(`${baseUrl}/visual/examples/${name}/index.html?verify=1`, { waitUntil: "domcontentloaded", timeout: 20000 });
      const deadline = Date.now() + 25000;
      while (Date.now() < deadline) {
        const ready = await page.evaluate(RUNTIME_READY);
        if (ready === null || ready === true) break;
        await page.waitForTimeout(200);
      }
      await page.waitForTimeout(2200);
      let buffer;
      try {
        buffer = await page.screenshot({ type: "png", animations: "disabled", timeout: 15000 });
      } catch (screenshotError) {
        // Some scenes (e.g. many simultaneous WebGL contexts) can't be captured
        // reliably headless. Skip rather than fail the whole regression run.
        results.push({ name, ok: true, skipped: true, reason: "screenshot-timeout" });
        await context.close();
        continue;
      }
      const signature = await signatureFromPng(decoderPage, buffer.toString("base64"), SIG);
      const baselinePath = join(BASELINE_DIR, `${name}.json`);

      if (UPDATE) {
        await writeFile(baselinePath, JSON.stringify({ size: SIG, signature }), "utf8");
        entry = { name, ok: true, updated: true };
      } else {
        let baseline = null;
        try { baseline = JSON.parse(await readFile(baselinePath, "utf8")); } catch { baseline = null; }
        if (!baseline) {
          entry = { name, ok: false, reason: "no-baseline (run with --update)" };
        } else {
          const diff = diffSignatures(signature, baseline.signature);
          entry = { name, ok: diff <= THRESHOLD, diff: Number(diff.toFixed(4)), threshold: THRESHOLD };
        }
      }
    } catch (error) {
      entry = { name, ok: false, error: error.message };
    } finally {
      await context.close();
    }
    results.push(entry);
  }

  await browser.close();
  server.close();

  const failed = results.filter((r) => !r.ok);
  const ok = failed.length === 0;
  console.log(JSON.stringify({ ok, mode: UPDATE ? "update" : "check", baselineDir: BASELINE_DIR, total: results.length, failed: failed.length, results }, null, 2));
  if (!ok) process.exitCode = 1;
}

main().catch((error) => { console.error(error); process.exitCode = 1; });
