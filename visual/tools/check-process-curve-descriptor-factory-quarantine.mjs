#!/usr/bin/env node

import { readdirSync, readFileSync, statSync } from "node:fs";
import { join, relative, resolve } from "node:path";

const ROOT = resolve(new URL("../..", import.meta.url).pathname);
const EXAMPLES = join(ROOT, "visual", "examples");
const FACTORY = "createProcessCurveMiniatureLayerDescriptors";
const failures = [];

for (const file of listFiles(EXAMPLES)) {
  if (!/\.(html|js|mjs)$/.test(file)) continue;
  const rel = relative(ROOT, file);
  const text = readFileSync(file, "utf8");
  if (!new RegExp(`\\b${FACTORY}\\b`).test(text)) continue;

  const lines = text.split(/\r?\n/);
  for (let index = 0; index < lines.length; index += 1) {
    const line = lines[index];
    if (!new RegExp(`\\b${FACTORY}\\b`).test(line)) continue;
    const kind = new RegExp(`\\b${FACTORY}\\s*\\(`).test(line) ? "call" : "import/reference";
    failures.push(`${rel}:${index + 1}: forbidden process-curve descriptor factory ${kind}`);
  }
}

if (failures.length > 0) {
  console.error("Process-curve descriptor factory quarantine check failed:");
  for (const failure of failures) console.error(`- ${failure}`);
  process.exit(1);
}

console.log("Process-curve descriptor factory quarantine check passed.");

function listFiles(dir) {
  const entries = [];
  for (const name of readdirSync(dir)) {
    const path = join(dir, name);
    const stat = statSync(path);
    if (stat.isDirectory()) entries.push(...listFiles(path));
    else entries.push(path);
  }
  return entries;
}
