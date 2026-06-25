#!/usr/bin/env node
/*
 * Command-level public API gate.
 *
 * This check keeps public examples on the command-driven METRIC Visual API:
 * examples may load metric.visual.v1 evidence and call createMetricVisual plus
 * a semantic command, but they must not assemble renderers, descriptors, or a
 * second canvas render path themselves.
 */

import { readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import {
  MetricVisualSurface,
  createMetricVisual,
  showMetricSpace,
  showRelationMatrix,
  showNeighborhoodGraph,
  showSpaceProperties,
  showMapping,
  showDynamics,
  showConditionMonitoring,
  showMixedRecords,
  showCrossSpace,
  showRelationMatrixNeighborhood,
  showSolverTrace,
  showPreview,
} from "../src/index.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const SITE = resolve(ROOT, "docs", "site", "index.html");

const COMMANDS = [
  "showMetricSpace",
  "showRelationMatrix",
  "showNeighborhoodGraph",
  "showSpaceProperties",
  "showMapping",
  "showDynamics",
  "showConditionMonitoring",
  "showMixedRecords",
  "showCrossSpace",
  "showRelationMatrixNeighborhood",
  "showSolverTrace",
  "showPreview",
];

const EXPORTED_COMMANDS = {
  showMetricSpace,
  showRelationMatrix,
  showNeighborhoodGraph,
  showSpaceProperties,
  showMapping,
  showDynamics,
  showConditionMonitoring,
  showMixedRecords,
  showCrossSpace,
  showRelationMatrixNeighborhood,
  showSolverTrace,
  showPreview,
};

const PUBLIC_REFERENCE_EXEMPTIONS = new Set([
  "grae10-metric-engine",
]);

const PUBLIC_EXAMPLE_EXPECTED_COMMANDS = new Map([
  ["condition-monitoring-hero", "showConditionMonitoring"],
  ["mixed-record-hero", "showMixedRecords"],
  ["cross-space-dependency-hero", "showCrossSpace"],
  ["relation-matrix-neighborhood", "showRelationMatrixNeighborhood"],
  ["dynamics-noise-hero", "showDynamics"],
  ["mapping-dimensionality-hero", "showMapping"],
]);

const PUBLIC_EXAMPLE_FORBIDDEN = [
  ["new MetricVisualRuntime", /\bnew\s+MetricVisualRuntime\b/],
  ["runtime factory", /\bcreate(?:MetricVisual|VisualEngine)Runtime\b/],
  ["layer factory", /\bcreateLayerFromDescriptor\b/],
  ["manual descriptor conversion", /\.toLayerDescriptors\s*\(/],
  ["manual descriptor installation", /\.(?:set|add)LayerDescriptors\s*\(/],
  ["manual WebGL renderer", /\bWebGLRenderer\b|\bEffectComposer\b|\bnew\s+MGL\.(?:Scene|PerspectiveCamera|WebGLRenderer)\b/],
  ["manual canvas render context", /\.getContext\s*\(/],
  ["manual animation loop", /\brequestAnimationFrame\s*\(/],
  ["direct runtime import", /from\s+["'][^"']*\/src\/runtime(?:\/index)?\.js["']/],
  ["direct layer import", /from\s+["'][^"']*\/src\/layers(?:\/index)?\.js["']/],
  ["direct view import", /from\s+["'][^"']*\/src\/views(?:\/index)?\.js["']/],
  ["direct engine import", /from\s+["'][^"']*\/src\/engine(?:\/index)?\.js["']/],
  ["direct miniature-engine import", /from\s+["'][^"']*\/src\/miniature-engine(?:\/index)?\.js["']/],
];

async function main() {
  const failures = [];
  const commandResults = [];

  checkExportSurface(failures);
  commandResults.push(...await checkCommandDiagnostics(failures));
  await checkPublicExamples(failures);

  const ok = failures.length === 0;
  console.log(JSON.stringify({
    ok,
    commands: commandResults,
    failures,
  }, null, 2));
  if (!ok) process.exitCode = 1;
}

function checkExportSurface(failures) {
  if (typeof createMetricVisual !== "function") {
    failures.push("src/index.js: createMetricVisual export is missing");
  }
  for (const command of COMMANDS) {
    if (typeof EXPORTED_COMMANDS[command] !== "function") {
      failures.push(`src/index.js: ${command} export is missing`);
    }
    if (typeof MetricVisualSurface.prototype[command] !== "function") {
      failures.push(`MetricVisualSurface: ${command} method is missing`);
    }
  }
}

async function checkCommandDiagnostics(failures) {
  const metricFixture = await readJson("visual/examples/fixtures/metric-space.visual.json");
  const nativeCondition = await readJson("docs/examples/assets/condition-monitoring/metric.visual.json");
  const nativeMixed = await readJson("docs/examples/assets/mixed-records/metric.visual.json");
  const nativeCrossSpace = await readJson("docs/examples/assets/cross-space-dependency/metric.visual.json");
  const nativeRelation = await readJson("docs/examples/assets/relation-matrix/metric.visual.json");
  const nativeDynamics = await readJson("docs/examples/assets/dynamics-noise/metric.visual.json");
  const nativeMapping = await readJson("docs/examples/assets/mapping-dimensionality/metric.visual.json");

  const cases = [
    {
      command: "showMetricSpace",
      viewKind: "metric-space",
      evidenceKind: "synthetic_fixture",
      document: metricFixture,
      options: { coordinateId: "landmark-3d", colorBy: "anomaly", preview: false },
    },
    {
      command: "showRelationMatrix",
      viewKind: "relation-matrix",
      evidenceKind: "synthetic_fixture",
      document: metricFixture,
      options: { relationId: "sensor-metric", preview: false },
    },
    {
      command: "showNeighborhoodGraph",
      viewKind: "neighborhood-graph",
      evidenceKind: "synthetic_fixture",
      document: metricFixture,
      options: { coordinateId: "landmark-3d", relationId: "sensor-metric", topK: 4, preview: false },
    },
    {
      command: "showSpaceProperties",
      viewKind: "space-properties",
      evidenceKind: "synthetic_fixture",
      document: metricFixture,
      options: { coordinateId: "landmark-3d", propertyId: "density", preview: false },
    },
    {
      command: "showMapping",
      viewKind: "mapping",
      evidenceKind: "native",
      document: nativeMapping,
      options: {
        sourceCoordinateId: "phate-target-2d",
        targetCoordinateId: "source-feature-layout-3d",
        residualProperty: "local-mapping-distortion",
        labels: "process-family",
        loop: false,
        progress: 0.4,
        preview: false,
      },
    },
    {
      command: "showDynamics",
      viewKind: "dynamics",
      evidenceKind: "native",
      document: nativeDynamics,
      options: {
        timelineId: "reverse-reconstruction",
        propertyField: "best-reconstruction-error",
        preview: false,
      },
    },
    {
      command: "showConditionMonitoring",
      viewKind: "condition-monitoring",
      evidenceKind: "native",
      document: nativeCondition,
      options: {
        coordinateId: "process-state-trajectory-3d",
        colorBy: "metric-anomaly-severity",
        groundField: "local-density",
        labels: "diagnosis-state",
        preview: false,
      },
    },
    {
      command: "showMixedRecords",
      viewKind: "mixed-records",
      evidenceKind: "native",
      document: nativeMixed,
      options: {
        coordinateId: "mixed-finite-records-family-severity-3d",
        glyphBy: "family",
        labels: "family",
        relationId: "mixed-finite-records-composite-metric",
        topK: 5,
        preview: false,
      },
    },
    {
      command: "showCrossSpace",
      viewKind: "cross-space",
      evidenceKind: "native",
      document: nativeCrossSpace,
      options: {
        coordinateA: "event-log-landmark-3d",
        coordinateB: "process-curve-landmark-3d",
        dependenceProperty: "local-dependence-contribution",
        preview: false,
      },
    },
    {
      command: "showRelationMatrixNeighborhood",
      viewKind: "relation-matrix-neighborhood",
      evidenceKind: "native",
      document: nativeRelation,
      options: {
        coordinateId: "process-curve-block-layout-3d",
        relationId: "process-curve-aligned-metric",
        colorProperty: "process-family",
        matrixRect: [0.635, 0.30, 0.33, 0.44],
        preview: false,
      },
    },
    {
      command: "showSolverTrace",
      viewKind: "solver-trace",
      evidenceKind: "synthetic_fixture",
      document: metricFixture,
      options: {
        series: [
          { iteration: 0, residual: 1 },
          { iteration: 1, residual: 0.4 },
          { iteration: 2, residual: 0.08 },
          { iteration: 3, residual: 0.008 },
        ],
        traceLabel: "PCG residual",
        preview: false,
      },
    },
    {
      command: "showPreview",
      viewKind: "record-preview",
      evidenceKind: "synthetic_fixture",
      document: metricFixture,
      options: {
        coordinateId: "landmark-3d",
        colorBy: "anomaly",
        mode: "record",
      },
    },
  ];

  const results = [];
  for (const testCase of cases) {
    const surface = createHeadlessSurface(testCase.document);
    surface[testCase.command](testCase.options);
    const diagnostics = surface.getDiagnostics();
    const command = diagnostics.commands.at(-1);
    const result = {
      command: testCase.command,
      viewKind: command?.selectedViewKind,
      descriptorCount: command?.descriptorCount,
      runtimeLayerCount: command?.runtimeLayerCount,
      evidenceKind: command?.evidenceKind,
    };
    results.push(result);

    assert(failures, `${testCase.command}: selectedCommand is reported`, command?.selectedCommand === testCase.command, command);
    assert(failures, `${testCase.command}: selected view kind is reported`, command?.selectedViewKind === testCase.viewKind, command);
    assert(failures, `${testCase.command}: evidence schema is reported`, command?.evidenceSchema === "metric.visual.v1", command);
    assert(failures, `${testCase.command}: record count is reported`, Number.isInteger(command?.recordCount) && command.recordCount > 0, command);
    assert(failures, `${testCase.command}: descriptor count is reported`, Number.isInteger(command?.descriptorCount) && command.descriptorCount > 0, command);
    assert(failures, `${testCase.command}: runtime layer count tracks descriptors`, command?.runtimeLayerCount === command?.descriptorCount, command);
    assert(failures, `${testCase.command}: evidence kind is classified`, command?.evidenceKind === testCase.evidenceKind, command);
    assert(failures, `${testCase.command}: diagnostics snapshot mirrors selected command`, diagnostics.selectedCommand === testCase.command, diagnostics);
    assert(failures, `${testCase.command}: diagnostics snapshot mirrors evidence kind`, diagnostics.evidenceKind === testCase.evidenceKind, diagnostics);
  }
  return results;
}

async function checkPublicExamples(failures) {
  const site = await readText(SITE);
  const publicExamples = extractPublicExampleNames(site);
  for (const name of publicExamples) {
    if (PUBLIC_REFERENCE_EXEMPTIONS.has(name)) continue;
    const relativePath = `visual/examples/${name}/index.html`;
    const text = await readText(resolve(ROOT, relativePath));
    const expectedCommand = PUBLIC_EXAMPLE_EXPECTED_COMMANDS.get(name);
    const commandCalls = COMMANDS.filter((command) => commandCallPattern(command).test(text));
    const canvasCount = (text.match(/<canvas\b/gi) || []).length;

    assert(failures, `${relativePath}: uses createMetricVisual`, /\bcreateMetricVisual\s*\(/.test(text), { commandCalls });
    assert(failures, `${relativePath}: calls a semantic command`, commandCalls.length > 0, { commandCalls });
    if (expectedCommand) {
      assert(failures, `${relativePath}: calls ${expectedCommand}`, commandCalls.includes(expectedCommand), { commandCalls });
    }
    assert(failures, `${relativePath}: owns at most one render canvas`, canvasCount <= 1, { canvasCount });

    for (const [label, pattern] of PUBLIC_EXAMPLE_FORBIDDEN) {
      assert(failures, `${relativePath}: does not use ${label}`, !pattern.test(text));
    }
  }
}

function createHeadlessSurface(document) {
  const runtime = new FakeRuntime();
  return new MetricVisualSurface({
    document,
    canvas: {
      getBoundingClientRect: () => ({ left: 0, top: 0, width: 960, height: 640 }),
      addEventListener() {},
      removeEventListener() {},
    },
    runtime,
    setup: {
      stage: { grounding: { groundY: -0.58 } },
      style: {
        setStyleMotion() {},
        detachStyleMotion() {},
      },
    },
    options: {},
  });
}

class FakeRuntime {
  constructor() {
    this.layers = [];
    this.descriptors = [];
  }

  setLayerDescriptors(descriptors) {
    this.descriptors = descriptors.slice();
    this.layers = this.descriptors.map((descriptor) => ({ id: descriptor.id, descriptor }));
  }

  renderOnce() {}

  start() {
    this.started = true;
  }

  setCameraOptions(options) {
    this.cameraOptions = { ...options };
  }

  getState() {
    return {
      layerInstanceCount: this.layers.length,
      layerState: { count: this.layers.length },
    };
  }

  on() {
    return () => {};
  }

  selectPair() {}
}

function commandCallPattern(command) {
  return new RegExp(`(?:\\.|\\b)${command}\\s*\\(`);
}

function extractPublicExampleNames(text) {
  return Array.from(
    new Set(Array.from(text.matchAll(/visual\/examples\/([^/"']+)\/index\.html/g), (match) => match[1])),
  ).sort();
}

async function readJson(path) {
  return JSON.parse(await readText(resolve(ROOT, path)));
}

async function readText(path) {
  return readFile(path, "utf8");
}

function assert(failures, message, ok, details = undefined) {
  if (ok) return;
  if (details === undefined) {
    failures.push(message);
  } else {
    failures.push(`${message}: ${JSON.stringify(details)}`);
  }
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
