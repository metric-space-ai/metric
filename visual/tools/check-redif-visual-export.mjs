#!/usr/bin/env node

import { readdir, readFile } from "node:fs/promises";
import { dirname, extname, join, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import { validateVisualDocument, VisualSpace } from "../src/data/index.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");

async function main() {
  const path = process.argv[2];
  if (!path) {
    console.error("usage: node visual/tools/check-redif-visual-export.mjs <metric.visual.json>");
    process.exitCode = 2;
    return;
  }

  const document = JSON.parse(await readFile(resolve(path), "utf8"));
  const validation = validateVisualDocument(document);
  let indexed = true;
  let indexError = null;
  try {
    VisualSpace.fromDocument(document);
  } catch (error) {
    indexed = false;
    indexError = error.message;
  }

  const diagnostic = document.diagnostics?.find((entry) => entry.id === "redif-native-evidence");
  const payload = diagnostic?.payload || {};
  const provenance = document.provenance || {};
  const sourceHits = await redifComputationHits(resolve(ROOT, "visual", "src"));
  const checks = [
    ["schema validates", validation.ok, validation.errors],
    ["document indexes", indexed, indexError],
    ["document is a native export", provenance.native_export === true, provenance],
    ["document is not marked synthetic", provenance.synthetic !== true, provenance],
    ["document was not synthesized in JavaScript", !("synthetic_js" in provenance) || provenance.synthetic_js === false, provenance],
    ["diagnostic exists", Boolean(diagnostic), document.diagnostics],
    ["diagnostic is native Redif evidence", String(payload.computed_by || "").startsWith("METRIC C++ redif_"), payload],
    ["visual layer does not compute Redif", payload.javascript_computation === false, payload],
    ["inverse measure paths are exported", Array.isArray(payload.inverse_measure_paths) && payload.inverse_measure_paths.length === 7, payload.inverse_measure_paths],
    ["forward measure paths are exported", Array.isArray(payload.forward_measure_paths) && payload.forward_measure_paths.length === 7, payload.forward_measure_paths],
    ["transport path property exists", document.properties?.some((entry) => entry.id === "redif-terminal-transport-path"), document.properties],
    ["entropy property exists", document.properties?.some((entry) => entry.id === "redif-terminal-entropy"), document.properties],
    ["measure-path timeline exists", document.timelines?.some((entry) => entry.id === "redif-measure-path" && entry.steps?.length === 5), document.timelines],
    ["visual/src has no Redif operator implementation", sourceHits.length === 0, sourceHits],
  ];
  const failures = checks
    .filter(([, ok]) => !ok)
    .map(([message, , details]) => ({ message, details }));
  console.log(JSON.stringify({
    ok: failures.length === 0,
    total: checks.length,
    failed: failures.length,
    failures,
  }, null, 2));
  if (failures.length) process.exitCode = 1;
}

async function redifComputationHits(root) {
  const hits = [];
  const entries = await readdir(root, { withFileTypes: true });
  for (const entry of entries) {
    const path = join(root, entry.name);
    if (entry.isDirectory()) {
      hits.push(...await redifComputationHits(path));
    } else if (entry.isFile() && [".js", ".mjs"].includes(extname(entry.name))) {
      const text = await readFile(path, "utf8");
      const forbidden = text.match(/redif_(remove|add|operator)|redifRemove|redifAdd|RedifOperator/g);
      if (forbidden) hits.push({ path, terms: [...new Set(forbidden)] });
    }
  }
  return hits;
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
