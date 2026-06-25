#!/usr/bin/env node

import { readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");

const METRIC_VISUAL = "visual/src/metric-visual.js";
const PROCESS_CURVE_PAGE = "visual/examples/process-curve-condition-monitoring/index.html";

const checks = [
  {
    name: "legacy public facade symbols are absent",
    files: [METRIC_VISUAL, PROCESS_CURVE_PAGE],
    pattern: /MetricScene3D|MetricRecordGallery|MetricHeatmap|MetricQueryInspector|MetricProcessCurveApp|MetricSelection|loadMetricEvidence|assertMetricEvidence/,
  },
  {
    name: "process-curve page does not load legacy evidence",
    files: [PROCESS_CURVE_PAGE],
    pattern: /metric\.evidence\.v1|evidence\.json/,
  },
  {
    name: "process-curve page does not assemble the renderer directly",
    files: [PROCESS_CURVE_PAGE],
    pattern: /MetricVisualRuntime|createLayerFromDescriptor|createMiniaturePhotographicStyle|createProcessCurveMiniatureSceneBundle|createMiniatureStyleProfile|createProcessCurveMiniatureStage/,
  },
  {
    name: "process-curve page does not install a page-local canvas renderer",
    files: [PROCESS_CURVE_PAGE],
    pattern: /\.getContext\s*\(|requestAnimationFrame\s*\(|<svg\b|createElementNS|new\s+Path2D|fillRect\s*\(/,
  },
];

const requiredPagePatterns = [
  {
    name: "loads native metric.visual asset",
    pattern: /docs\/examples\/assets\/process-curve-external\/metric\.visual\.json/,
  },
  {
    name: "creates the public visual surface",
    pattern: /\bcreateMetricVisual\s*\(/,
  },
  {
    name: "calls the public process-curve command",
    pattern: /\bvisual\s*\.\s*showProcessCurves\s*\(/,
  },
];

const failures = [];
const scanned = [];

for (const check of checks) {
  for (const file of check.files) {
    const text = await readFile(resolve(ROOT, file), "utf8");
    scanned.push(file);
    for (const match of matchingLines(text, check.pattern)) {
      failures.push({
        check: check.name,
        file,
        line: match.line,
        text: match.text,
      });
    }
  }
}

const pageText = await readFile(resolve(ROOT, PROCESS_CURVE_PAGE), "utf8");
for (const required of requiredPagePatterns) {
  if (!required.pattern.test(pageText)) {
    failures.push({
      check: required.name,
      file: PROCESS_CURVE_PAGE,
      line: null,
      text: "required pattern missing",
    });
  }
}

const canvasCount = (pageText.match(/<canvas\b/gi) || []).length;
if (canvasCount !== 1) {
  failures.push({
    check: "process-curve page owns exactly one runtime canvas",
    file: PROCESS_CURVE_PAGE,
    line: null,
    text: `canvas count: ${canvasCount}`,
  });
}

const result = {
  ok: failures.length === 0,
  scanned: [...new Set(scanned)].sort(),
  failures,
};

console.log(JSON.stringify(result, null, 2));
if (!result.ok) process.exitCode = 1;

function matchingLines(text, pattern) {
  return text
    .split(/\r?\n/)
    .map((lineText, index) => ({ line: index + 1, text: lineText.trim() }))
    .filter((entry) => pattern.test(entry.text));
}
