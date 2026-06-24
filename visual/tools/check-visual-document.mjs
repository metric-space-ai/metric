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
 */

import { readFile } from "node:fs/promises";
import { resolve } from "node:path";

import { validateVisualDocument, VisualSpace } from "../src/data/index.js";

async function check(path) {
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
    schemaValid: validation.ok,
    schemaErrors: validation.errors.slice(0, 10),
    indexed,
    indexError,
  };
}

async function main() {
  const paths = process.argv.slice(2);
  if (paths.length === 0) {
    console.error("usage: node visual/tools/check-visual-document.mjs <file> [file...]");
    process.exitCode = 2;
    return;
  }
  const results = [];
  for (const path of paths) {
    results.push(await check(path));
  }
  const ok = results.every((result) => result.ok);
  console.log(JSON.stringify({ ok, total: results.length, results }, null, 2));
  if (!ok) process.exitCode = 1;
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
