#!/usr/bin/env node
/*
 * Schema fixture smoke test.
 *
 * Validates every metric.visual.v1 fixture through the real data API:
 * - valid fixtures must pass validation and build a VisualSpace
 * - invalid fixtures must fail with the expected error code
 *
 * Run: node visual/tools/check-schema-fixtures.mjs
 */

import { readFile, readdir } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import { validateVisualDocument, VisualSpace } from "../src/data/index.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const FIXTURE_DIR = resolve(HERE, "..", "examples", "fixtures");

// Invalid fixtures and the error code each is expected to surface.
const EXPECTED_INVALID = {
  "invalid-missing-record-id.visual.json": "required_field",
  "invalid-relation-ref.visual.json": "record_ref",
};

async function loadFixture(name) {
  const path = resolve(FIXTURE_DIR, name);
  return JSON.parse(await readFile(path, "utf8"));
}

async function main() {
  const files = (await readdir(FIXTURE_DIR)).filter((name) => name.endsWith(".visual.json")).sort();
  const results = [];
  let failures = 0;

  for (const name of files) {
    const document = await loadFixture(name);
    const result = validateVisualDocument(document);
    const isInvalidFixture = name.startsWith("invalid-");

    if (isInvalidFixture) {
      const expectedCode = EXPECTED_INVALID[name];
      const codes = result.errors.map((error) => error.code);
      const ok = !result.ok && (!expectedCode || codes.includes(expectedCode));
      if (!ok) failures += 1;
      results.push({
        name,
        kind: "invalid",
        ok,
        expectedCode,
        sawCodes: codes.slice(0, 5),
        errorCount: result.errors.length,
      });
      continue;
    }

    let spaceOk = false;
    let spaceError = null;
    try {
      const space = VisualSpace.fromDocument(document);
      spaceOk = Array.isArray(space.records) && space.records.length > 0;
    } catch (error) {
      spaceError = error.message;
    }
    const ok = result.ok && spaceOk;
    if (!ok) failures += 1;
    results.push({
      name,
      kind: "valid",
      ok,
      errorCount: result.errors.length,
      warningCount: result.warnings.length,
      firstError: result.errors[0] || null,
      spaceError,
    });
  }

  const ok = failures === 0 && files.length > 0;
  console.log(JSON.stringify({ ok, fixtureDir: FIXTURE_DIR, count: files.length, failures, results }, null, 2));
  if (!ok) process.exitCode = 1;
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
