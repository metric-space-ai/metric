#!/usr/bin/env node

import { readdirSync, readFileSync, statSync } from "node:fs";
import { join, relative } from "node:path";

const root = new URL("../..", import.meta.url).pathname;
const scannedRoots = ["visual/src", "visual/examples"];
const forbidden = [
  {
    pattern: /\bcreateMetricSpaceHeroScene\b/,
    message: "parallel metric-space scene helper",
  },
  {
    pattern: /\bcreateMetricSpaceHeroDescriptors\b/,
    message: "parallel metric-space descriptor helper",
  },
  {
    pattern: /\bMetricSpaceHeroScene\b/,
    message: "parallel metric-space render-path module",
  },
];

const heroPageForbiddenImports = [
  "PointCloudView",
  "MorphView",
  "GroundProjectionView",
  "DenseFieldView",
];

const metricVisualForbidden = [
  {
    pattern: /\brecordTrajectoryDescriptor\b/,
    message: "trajectory/path grammar must live in TrajectoryPathView, not metric-visual.js",
  },
  {
    pattern: /\bcreateTubeRibbonPathLayerDescriptor\b/,
    message: "trajectory tube/ribbon descriptors must be emitted by semantic views, not metric-visual.js",
  },
  {
    pattern: /\bpropertyFieldDescriptors\b/,
    message: "property-field grammar must live in PropertyFieldView, not metric-visual.js",
  },
  {
    pattern: /\bDenseFieldView\b/,
    message: "metric-visual.js must use PropertyFieldView for property fields, not direct DenseFieldView helpers",
  },
  {
    pattern: /\bcreateProcessCurveMiniatureLayerDescriptors\b/,
    message: "process-curve grammar must be emitted by ProcessCurveSceneView, not metric-visual.js",
  },
];

const failures = [];

for (const dir of scannedRoots) {
  for (const file of listFiles(join(root, dir))) {
    if (!/\.(js|mjs|html)$/.test(file)) continue;
    const rel = relative(root, file);
    const text = readFileSync(file, "utf8");
    for (const rule of forbidden) {
      if (rule.pattern.test(text)) {
        failures.push(`${rel}: ${rule.message}`);
      }
    }
    if (rel === "visual/src/metric-visual.js") {
      for (const rule of metricVisualForbidden) {
        if (rule.pattern.test(text)) failures.push(`${rel}: ${rule.message}`);
      }
    }
    if (rel.startsWith("visual/examples/") && /-hero\/index\.html$/.test(rel)) {
      for (const name of heroPageForbiddenImports) {
        const directImport = new RegExp(`\\b${name}\\b`);
        if (directImport.test(text)) {
          failures.push(`${rel}: hero page bypasses MetricSpaceView via ${name}`);
        }
      }
    }
  }
}

if (failures.length > 0) {
  console.error("Single render pipeline check failed:");
  for (const failure of failures) console.error(`- ${failure}`);
  process.exit(1);
}

console.log("Single render pipeline check passed.");

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
