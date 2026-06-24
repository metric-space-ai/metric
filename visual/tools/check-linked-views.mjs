#!/usr/bin/env node
/*
 * Interaction regression check for the linked-metric-views example.
 *
 * Loads the example in headless Chromium and verifies the linked hover/selection
 * contract: hovering a projected record shows its preview and selects it, the
 * pair inspector lists nearest neighbors, and hovering the relation matrix
 * drives selection bidirectionally. Requires playwright resolvable (NODE_PATH).
 *
 *   node visual/tools/check-linked-views.mjs
 */
import { createServer } from "node:http";
import { readFile } from "node:fs/promises";
import { createRequire } from "node:module";
import { dirname, extname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const MIME = { ".html": "text/html", ".js": "text/javascript", ".mjs": "text/javascript", ".json": "application/json", ".css": "text/css", ".png": "image/png" };

const server = createServer(async (req, res) => {
  try {
    let p = decodeURIComponent(new URL(req.url, "http://x").pathname);
    if (p.endsWith("/")) p += "index.html";
    const data = await readFile(resolve(ROOT, `.${p}`));
    res.writeHead(200, { "content-type": MIME[extname(p)] || "application/octet-stream" }).end(data);
  } catch { res.writeHead(404).end("nf"); }
});
await new Promise((r) => server.listen(0, "127.0.0.1", r));
const base = `http://127.0.0.1:${server.address().port}`;

const require = createRequire(import.meta.url);
const { chromium } = require("playwright");
const browser = await chromium.launch({ headless: true, args: ["--use-gl=swiftshader", "--ignore-gpu-blocklist"] });
const page = await browser.newContext({ viewport: { width: 1280, height: 820 } }).then((c) => c.newPage());
const errors = [];
page.on("pageerror", (e) => errors.push(e.message));
await page.goto(`${base}/visual/examples/linked-metric-views/index.html`, { waitUntil: "domcontentloaded" });
await page.waitForTimeout(2000);

// Project a known record to screen so we can hover it precisely.
const target = await page.evaluate(() => {
  const h = window.metricLinkedViews;
  const id = h.metricSpaceView.recordIds[0];
  const pos = h.metricSpaceView.positions.get(String(id));
  const out = {};
  h.runtime.camera.projectToPixel(pos, out);
  const ratio = h.runtime.camera.viewport.pixelRatio || 1;
  return { id, x: out.x / ratio, y: out.y / ratio, visible: out.visible };
});

await page.mouse.move(target.x, target.y);
await page.waitForTimeout(150);
const afterPointHover = await page.evaluate(() => ({
  previewVisible: document.querySelector('.mtrc-record-preview')?.dataset.visible === "true",
  previewTitle: document.querySelector('.mtrc-record-preview strong')?.textContent || null,
  inspectorVisible: document.getElementById("inspector")?.dataset.visible === "true",
  pairCount: document.querySelectorAll('#inspector-pairs li').length,
  selected: document.documentElement.dataset.metricSelectedRecord || null,
}));

// Hover the relation matrix region (top-right) to drive matrix->selection.
await page.mouse.move(1000, 150);
await page.waitForTimeout(150);
const afterMatrixHover = await page.evaluate(() => ({
  inspectorVisible: document.getElementById("inspector")?.dataset.visible === "true",
  inspectorMeta: document.getElementById("inspector-meta")?.textContent || null,
  selected: document.documentElement.dataset.metricSelectedRecord || null,
}));

const ok = target.visible
  && afterPointHover.previewVisible
  && afterPointHover.selected
  && afterPointHover.pairCount > 0
  && afterMatrixHover.inspectorVisible
  && /↔/.test(afterMatrixHover.inspectorMeta || "")
  && errors.length === 0;

console.log(JSON.stringify({ ok, target, afterPointHover, afterMatrixHover, errors }, null, 2));
await browser.close();
server.close();
if (!ok) process.exitCode = 1;
