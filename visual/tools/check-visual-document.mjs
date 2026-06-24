#!/usr/bin/env node
/*
 * Validate one or more metric.visual.v1 documents.
 *
 * This is the bridge check for native C++ exporters: exporters write JSON,
 * this tool validates the schema and builds a VisualSpace index without running
 * any metric algorithm in JavaScript.
 *
 * Usage:
 *   node visual/tools/check-visual-document.mjs docs/examples/assets/foo/metric.visual.json
 *   node visual/tools/check-visual-document.mjs --public-gallery docs/examples/assets/foo/metric.visual.json
 */

import { readFile } from "node:fs/promises";
import { resolve } from "node:path";

import { validateVisualDocument, VisualSpace } from "../src/data/index.js";
import { createMetricVisualDocumentDiagnostics } from "../src/metric-visual.js";

async function check(path, options = {}) {
  const absolute = resolve(path);
  const text = await readFile(absolute, "utf8");
  let document;
  try {
    document = JSON.parse(text);
  } catch (error) {
    return {
      path,
      ok: false,
      stage: "parse",
      error: error.message,
    };
  }

  const validation = validateVisualDocument(document);
  const apiDiagnostics = createMetricVisualDocumentDiagnostics(document, options);
  let indexed = false;
  let indexError = null;
  try {
    VisualSpace.fromDocument(document);
    indexed = true;
  } catch (error) {
    indexError = error.message;
  }

  return {
    path,
    ok: validation.ok && indexed,
    schema: document.schema || null,
    records: Array.isArray(document.records) ? document.records.length : null,
    datasets: Array.isArray(document.datasets) ? document.datasets.length : null,
    spaces: Array.isArray(document.spaces) ? document.spaces.length : null,
    relations: Array.isArray(document.relations) ? document.relations.length : null,
    coordinates: Array.isArray(document.coordinates) ? document.coordinates.length : null,
    properties: Array.isArray(document.properties) ? document.properties.length : null,
    diagnostics: Array.isArray(document.diagnostics) ? document.diagnostics.length : null,
    synthetic: document.provenance?.synthetic === true,
    apiDiagnostics,
    schemaValid: validation.ok,
    schemaErrors: validation.errors.slice(0, 10),
    indexed,
    indexError,
  };
}

async function main() {
  const { paths, options } = parseArgs(process.argv.slice(2));
  if (paths.length === 0) {
    console.error("usage: node visual/tools/check-visual-document.mjs [--public-gallery] [--context <name>] <file> [file...]");
    process.exitCode = 2;
    return;
  }
  const results = [];
  for (const path of paths) {
    results.push(await check(path, options));
  }
  const ok = results.every((result) => result.ok);
  console.log(JSON.stringify({ ok, total: results.length, results }, null, 2));
  if (!ok) process.exitCode = 1;
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});

function parseArgs(argv) {
  const paths = [];
  const options = {};
  for (let index = 0; index < argv.length; index += 1) {
    const arg = argv[index];
    if (arg === "--public-gallery") {
      options.publicGallery = true;
    } else if (arg === "--context") {
      options.context = argv[index + 1] || "";
      index += 1;
    } else if (arg.startsWith("--context=")) {
      options.context = arg.slice("--context=".length);
    } else {
      paths.push(arg);
    }
  }
  return { paths, options };
}
