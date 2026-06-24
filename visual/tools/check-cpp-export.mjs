#!/usr/bin/env node
/*
 * C++ export round-trip check.
 *
 * Compiles visual/cpp/export_example.cpp (header-only mtrc::visual), runs it,
 * and validates the emitted metric.visual.v1 document through the same JS
 * validator the runtime uses + builds a VisualSpace. This proves C++ can export
 * evidence with no JavaScript in the path and that the output loads directly.
 *
 * Run: node visual/tools/check-cpp-export.mjs
 * Skips gracefully (exit 0) if no C++ compiler is available.
 */

import { spawnSync } from "node:child_process";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";
import { existsSync } from "node:fs";

import { validateVisualDocument, VisualSpace } from "../src/data/index.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const CPP_DIR = resolve(HERE, "..", "cpp");
const SOURCE = resolve(CPP_DIR, "export_example.cpp");
const BIN = resolve("/tmp", "mtrc_visual_export_example");

function findCompiler() {
  for (const candidate of ["c++", "clang++", "g++"]) {
    const probe = spawnSync(candidate, ["--version"], { encoding: "utf8" });
    if (probe.status === 0) return candidate;
  }
  return null;
}

function main() {
  const compiler = findCompiler();
  if (!compiler) {
    console.log(JSON.stringify({ ok: true, skipped: true, reason: "no C++ compiler available" }, null, 2));
    return;
  }

  const compile = spawnSync(compiler, ["-std=c++17", `-I${CPP_DIR}`, SOURCE, "-o", BIN], { encoding: "utf8" });
  if (compile.status !== 0) {
    console.log(JSON.stringify({ ok: false, stage: "compile", compiler, stderr: compile.stderr }, null, 2));
    process.exitCode = 1;
    return;
  }

  const run = spawnSync(BIN, [], { encoding: "utf8", maxBuffer: 32 * 1024 * 1024 });
  if (run.status !== 0) {
    console.log(JSON.stringify({ ok: false, stage: "run", stderr: run.stderr }, null, 2));
    process.exitCode = 1;
    return;
  }

  let document;
  try {
    document = JSON.parse(run.stdout);
  } catch (error) {
    console.log(JSON.stringify({ ok: false, stage: "parse", error: error.message, head: run.stdout.slice(0, 200) }, null, 2));
    process.exitCode = 1;
    return;
  }

  const result = validateVisualDocument(document);
  let spaceOk = false;
  let recordCount = 0;
  let relationValue = null;
  try {
    const space = VisualSpace.fromDocument(document);
    recordCount = space.records.length;
    relationValue = space.relationValue("metric", "r0", "r1");
    spaceOk = recordCount > 0;
  } catch (error) {
    spaceOk = false;
  }

  const ok = result.ok && spaceOk;
  console.log(JSON.stringify({
    ok,
    compiler,
    binaryExists: existsSync(BIN),
    schemaValid: result.ok,
    schemaErrors: result.errors.slice(0, 5),
    recordCount,
    coordinateCount: document.coordinates.length,
    relationCount: document.relations.length,
    timelineCount: document.timelines.length,
    eventCount: document.events.length,
    viewCount: document.views.length,
    rawAppendCoverage: {
      timelines: document.timelines.length === 1,
      events: document.events.length === 1,
      views: document.views.length === 1,
    },
    relationValueLookup: relationValue,
  }, null, 2));
  if (!ok || document.timelines.length !== 1 || document.events.length !== 1 || document.views.length !== 1) {
    process.exitCode = 1;
  }
}

main();
