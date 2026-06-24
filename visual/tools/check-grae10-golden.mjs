import { createHash } from "node:crypto";
import { readFile } from "node:fs/promises";
import { fileURLToPath } from "node:url";
import path from "node:path";

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "../..");
const targetPath = path.join(root, "visual/examples/grae10-metric-engine/index.html");
const expectedPath = path.join(root, "visual/regression-baselines/grae10-metric-engine.sha256");

const [bytes, expectedText] = await Promise.all([
  readFile(targetPath),
  readFile(expectedPath, "utf8"),
]);

const expected = expectedText.trim().split(/\s+/)[0];
const actual = createHash("sha256").update(bytes).digest("hex");

if (actual !== expected) {
  throw new Error(
    [
      "GRAE10 golden reference changed.",
      `expected: ${expected}`,
      `actual:   ${actual}`,
      "Do not overwrite visual/examples/grae10-metric-engine/index.html.",
      "Develop new engine work in a separate page until it reproduces the golden visual.",
    ].join("\n"),
  );
}

console.log(`GRAE10 golden reference OK: ${actual}`);
