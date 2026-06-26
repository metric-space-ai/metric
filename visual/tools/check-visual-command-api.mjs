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
  showProcessCurves,
  showMixedRecords,
  showCrossSpace,
  showRelationMatrixNeighborhood,
  showSolverTrace,
  showPreview,
  captureHeroFrame,
} from "../src/index.js";
import {
  EXPLICIT_NATIVE_METRIC_VISUAL_EXPORT_KEYS,
  isExplicitNativeMetricVisualExport,
} from "../src/data/provenance.js";

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
  "showProcessCurves",
  "showMixedRecords",
  "showCrossSpace",
  "showRelationMatrixNeighborhood",
  "showSolverTrace",
  "showPreview",
  "captureHeroFrame",
];

const EXPORTED_COMMANDS = {
  showMetricSpace,
  showRelationMatrix,
  showNeighborhoodGraph,
  showSpaceProperties,
  showMapping,
  showDynamics,
  showConditionMonitoring,
  showProcessCurves,
  showMixedRecords,
  showCrossSpace,
  showRelationMatrixNeighborhood,
  showSolverTrace,
  showPreview,
  captureHeroFrame,
};

const PUBLIC_REFERENCE_EXEMPTIONS = new Set([
  "grae10-metric-engine",
]);

const PUBLIC_COMMAND_EXEMPTIONS = new Set([
  ...PUBLIC_REFERENCE_EXEMPTIONS,
]);

const PUBLIC_EXAMPLE_EXPECTED_COMMANDS = new Map([
  ["condition-monitoring-hero", "showConditionMonitoring"],
  ["process-curve-external-hero", "showProcessCurves"],
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
  ["manual VisualLayer construction", /\bnew\s+VisualLayer\b/],
  ["manual layer primitive descriptor", /\bprimitive\s*:\s*["'][A-Za-z0-9_-]+Layer["']/],
  ["manual channel descriptor", /\bchannels\s*:\s*\{/],
  ["manual descriptor serialization", /\.toDescriptor\s*\(/],
  ["manual descriptor conversion", /\.toLayerDescriptors\s*\(/],
  ["manual descriptor installation", /\.(?:set|add)LayerDescriptors\s*\(/],
  ["private descriptor installation", /\._setLayerDescriptors\s*\(/],
  ["manual WebGL renderer", /\bWebGLRenderer\b|\bEffectComposer\b|\bnew\s+MGL\.(?:Scene|PerspectiveCamera|WebGLRenderer)\b/],
  ["manual canvas render context", /\.getContext\s*\(/],
  ["manual animation loop", /\brequestAnimationFrame\s*\(/],
  ["direct runtime import", /from\s+["'][^"']*\/src\/runtime(?:\/index)?\.js["']/],
  ["direct layer import", /from\s+["'][^"']*\/src\/layers(?:\/index)?\.js["']/],
  ["direct view import", /from\s+["'][^"']*\/src\/views(?:\/index)?\.js["']/],
  ["direct engine import", /from\s+["'][^"']*\/src\/engine(?:\/index)?\.js["']/],
  ["direct miniature-engine import", /from\s+["'][^"']*\/src\/miniature-engine(?:\/index)?\.js["']/],
  ["page-local evidence rewrite helper", /\bcreate[A-Za-z0-9_$]*RuntimeDocument\b/],
  ["page-local relation value omission", /\bomitted_for_view\b|\bmapping_runtime_reference\b/],
];

async function main() {
  const failures = [];
  const commandResults = [];

  checkExportSurface(failures);
  checkNativeProvenanceHelper(failures);
  await checkDescriptorInjectionBoundary(failures);
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
  let delegatedOptions = null;
  const captured = captureHeroFrame({
    captureHeroFrame(options) {
      delegatedOptions = options;
      return "metric-hero-frame";
    },
  }, { at: "target-hold" });
  if (captured !== "metric-hero-frame" || delegatedOptions?.at !== "target-hold") {
    failures.push("src/index.js: captureHeroFrame must delegate to the visual surface method");
  }
}

function checkNativeProvenanceHelper(failures) {
  assert(
    failures,
    "native provenance helper documents accepted spellings",
    EXPLICIT_NATIVE_METRIC_VISUAL_EXPORT_KEYS.includes("native_export")
      && EXPLICIT_NATIVE_METRIC_VISUAL_EXPORT_KEYS.includes("nativeExport")
      && EXPLICIT_NATIVE_METRIC_VISUAL_EXPORT_KEYS.length === 2,
    EXPLICIT_NATIVE_METRIC_VISUAL_EXPORT_KEYS,
  );
  assert(
    failures,
    "native provenance helper accepts current snake_case writer flag",
    isExplicitNativeMetricVisualExport({ native_export: true }) === true,
  );
  assert(
    failures,
    "native provenance helper accepts only the centralized camelCase equivalent",
    isExplicitNativeMetricVisualExport({ nativeExport: true }) === true,
  );
  assert(
    failures,
    "native provenance helper rejects truthy strings",
    isExplicitNativeMetricVisualExport({ native_export: "true", nativeExport: "true" }) === false,
  );
  assert(
    failures,
    "native provenance helper ignores writer/runtime/computation text hints",
    isExplicitNativeMetricVisualExport({
      writer: "examples/engine/native_visual_export.cpp",
      runtime: "native-c++",
      computation: "native METRIC C++",
    }) === false,
  );
}

async function checkDescriptorInjectionBoundary(failures) {
  assert(
    failures,
    "MetricVisualSurface: public setLayerDescriptors method is removed",
    typeof MetricVisualSurface.prototype.setLayerDescriptors === "undefined",
  );
  assert(
    failures,
    "MetricVisualSurface: public addLayerDescriptors method is removed",
    typeof MetricVisualSurface.prototype.addLayerDescriptors === "undefined",
  );
  assert(
    failures,
    "MetricVisualSurface: internal descriptor helper remains available",
    typeof MetricVisualSurface.prototype._setLayerDescriptors === "function",
  );

  const surface = createHeadlessSurface(await readJson("visual/examples/fixtures/metric-space.visual.json"));
  const semanticView = {
    toLayerDescriptors() {
      return [{ id: "semantic-layer", primitive: "InstancedPointLayer", source: { viewKind: "test-view" } }];
    },
  };
  surface.setViews([semanticView], { source: "setViewsBoundaryCheck", viewKind: "test-view" });
  assert(
    failures,
    "MetricVisualSurface.setViews accepts semantic view objects",
    surface.descriptors.length === 1 && surface.descriptors[0]?.id === "semantic-layer",
    surface.descriptors,
  );

  let rejectedRawDescriptor = false;
  try {
    surface.setViews([{ id: "raw-layer", primitive: "InstancedPointLayer" }], { source: "rawDescriptorBoundaryCheck" });
  } catch (error) {
    rejectedRawDescriptor = /raw layer descriptors are internal-only/i.test(error?.message || "");
  }
  assert(
    failures,
    "MetricVisualSurface.setViews rejects raw layer descriptors",
    rejectedRawDescriptor,
  );
}

async function checkCommandDiagnostics(failures) {
  const metricFixture = await readJson("visual/examples/fixtures/metric-space.visual.json");
  const nativeCondition = await readJson("docs/examples/assets/condition-monitoring/metric.visual.json");
  const nativeProcessExternal = await readJson("docs/examples/assets/process-curve-external/metric.visual.json");
  const nativeMixed = await readJson("docs/examples/assets/mixed-records/metric.visual.json");
  const nativeCrossSpace = await readJson("docs/examples/assets/cross-space-dependency/metric.visual.json");
  const nativeRelation = await readJson("docs/examples/assets/relation-matrix/metric.visual.json");
  const nativeDynamics = await readJson("docs/examples/assets/dynamics-noise/metric.visual.json");
  const nativeMapping = await readJson("docs/examples/assets/mapping-dimensionality/metric.visual.json");
  const nativeSolverTrace = await readJson("docs/examples/assets/solver-trace/metric.visual.json");
  const mappingSourceCoordinate = findCoordinateId(nativeMapping, { dimension: 2, prefer: /target|latent|diffusion/i });
  const mappingTargetCoordinate = findCoordinateId(nativeMapping, { dimension: 3, prefer: /source|layout|feature/i });
  const mappingResidualProperty = findPropertyId(nativeMapping, { valueType: "scalar", prefer: /distortion|residual|error/i });
  const dynamicsTimeline = findTimelineId(nativeDynamics, { prefer: /reverse|reconstruction/i });
  const dynamicsProperty = findPropertyId(nativeDynamics, { targetType: "record", valueType: "scalar", prefer: /reconstruction|error/i });
  const conditionCoordinate = findCoordinateId(nativeCondition, { dimension: 3, prefer: /process|trajectory|state/i });
  const conditionScalar = findPropertyId(nativeCondition, { targetType: "record", valueType: "scalar", prefer: /anomaly|severity/i });
  const conditionField = findPropertyId(nativeCondition, { targetType: "record", valueType: "scalar", prefer: /density/i });
  const conditionLabel = findPropertyId(nativeCondition, { targetType: "record", valueType: "categorical", prefer: /diagnosis|state|regime/i });
  const processCoordinate = findCoordinateId(nativeProcessExternal, { dimension: 3, prefer: /process|landmark/i });
  const processRelation = findRelationId(nativeProcessExternal, { prefer: /aligned|metric/i });
  const processGraph = findGraphId(nativeProcessExternal, { prefer: /knn|neighbor/i });
  const processLabel = findPropertyId(nativeProcessExternal, { targetType: "record", valueType: "categorical", prefer: /role|state|regime/i });
  const mixedCoordinate = findCoordinateId(nativeMixed, { dimension: 3, prefer: /family|severity|mixed/i });
  const mixedRelation = findRelationId(nativeMixed, { prefer: /composite|metric/i });
  const mixedLabel = findPropertyId(nativeMixed, { targetType: "record", valueType: "categorical", prefer: /family|type/i });
  const crossLeftCoordinate = findCoordinateId(nativeCrossSpace, { dimension: 3, prefer: /event|left|space-a/i });
  const crossRightCoordinate = findCoordinateId(nativeCrossSpace, { dimension: 3, prefer: /process|right|space-b/i, exclude: crossLeftCoordinate });
  const crossProperty = findPropertyId(nativeCrossSpace, { targetType: "record", valueType: "scalar", prefer: /dependence|alignment/i });
  const relationCoordinate = findCoordinateId(nativeRelation, { dimension: 3, prefer: /block|layout|process/i });
  const relationId = findRelationId(nativeRelation, { prefer: /metric|aligned/i });
  const relationColor = findPropertyId(nativeRelation, { targetType: "record", valueType: "categorical", prefer: /family|block|group/i });
  const solverTimeline = findTimelineId(nativeSolverTrace, { prefer: /jacobi|solver|trace/i });
  const solverResidual = findPropertyId(nativeSolverTrace, { targetType: "timeline_step", valueType: "scalar", prefer: /residual/i });

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
      requiredPrimitives: ["InstancedPointLayer", "RelationEdgeLayer"],
      options: {
        sourceCoordinateId: mappingSourceCoordinate,
        targetCoordinateId: mappingTargetCoordinate,
        residualProperty: mappingResidualProperty,
        labels: findPropertyId(nativeMapping, { targetType: "record", valueType: "categorical", prefer: /family|class|label/i }),
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
      requiredPrimitives: ["HeatFieldLayer", "CurveRibbonLayer", "InstancedPointLayer"],
      options: {
        timelineId: dynamicsTimeline,
        propertyField: dynamicsProperty,
        preview: false,
      },
    },
    {
      command: "showConditionMonitoring",
      viewKind: "condition-monitoring",
      evidenceKind: "native",
      document: nativeCondition,
      requiredPrimitives: ["HeatFieldLayer", "CurveRibbonLayer", "InstancedPointLayer"],
      options: {
        coordinateId: conditionCoordinate,
        colorBy: conditionScalar,
        groundField: conditionField,
        labels: conditionLabel,
        preview: false,
      },
    },
    {
      command: "showProcessCurves",
      viewKind: "process-curves",
      evidenceKind: "native",
      document: nativeProcessExternal,
      requiredPrimitives: ["HeatFieldLayer", "CurveTubeMeshLayer", "InstancedBoxLayer", "RelationMatrixLayer"],
      requiredViewKinds: ["record-track", "record-skyline", "relation-matrix"],
      options: {
        coordinateId: processCoordinate,
        relationId: processRelation,
        graphId: processGraph,
        labelPropertyId: processLabel,
        includeMatrix: true,
        preview: false,
      },
    },
    {
      command: "showMixedRecords",
      viewKind: "mixed-records",
      evidenceKind: "native",
      document: nativeMixed,
      requiredPrimitives: ["InstancedGlyphLayer", "RelationEdgeLayer"],
      options: {
        coordinateId: mixedCoordinate,
        glyphBy: mixedLabel,
        labels: mixedLabel,
        relationId: mixedRelation,
        topK: 5,
        preview: false,
      },
    },
    {
      command: "showCrossSpace",
      viewKind: "cross-space",
      evidenceKind: "native",
      document: nativeCrossSpace,
      requiredPrimitives: ["InstancedGlyphLayer", "RelationEdgeLayer"],
      options: {
        coordinateA: crossLeftCoordinate,
        coordinateB: crossRightCoordinate,
        dependenceProperty: crossProperty,
        preview: false,
      },
    },
    {
      command: "showRelationMatrixNeighborhood",
      viewKind: "relation-matrix-neighborhood",
      evidenceKind: "native",
      document: nativeRelation,
      requiredPrimitives: ["InstancedPointLayer", "RelationEdgeLayer", "RelationMatrixLayer"],
      requiredViewKinds: ["neighborhood-graph", "relation-matrix"],
      options: {
        coordinateId: relationCoordinate,
        relationId,
        colorProperty: relationColor,
        matrixRect: [0.635, 0.30, 0.33, 0.44],
        preview: false,
      },
    },
    {
      command: "showSolverTrace",
      viewKind: "solver-trace",
      evidenceKind: "native",
      document: nativeSolverTrace,
      requiredPrimitives: ["CurveRibbonLayer"],
      options: {
        timelineId: solverTimeline,
        propertyId: solverResidual,
        traceLabel: "native Jacobi residual",
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
    assert(failures, `${testCase.command}: evidence kind signals are reported`, Array.isArray(command?.evidenceKindSignals) && command.evidenceKindSignals.length > 0, command);
    assert(failures, `${testCase.command}: evidence report is attached`, command?.evidenceReport?.schema === "metric.visual.public_evidence_report.v1", command);
    assert(failures, `${testCase.command}: evidence report mirrors record count`, command?.evidenceReport?.recordCount === command?.recordCount, command);
    assert(failures, `${testCase.command}: evidence report mirrors kind`, command?.evidenceReport?.kind === testCase.evidenceKind, command);
    assert(failures, `${testCase.command}: evidence report exposes relation count`, Number.isInteger(command?.evidenceReport?.relationCount), command);
    assert(failures, `${testCase.command}: descriptor kinds are reported`, command?.descriptorKinds && Object.keys(command.descriptorKinds.primitives || {}).length > 0, command);
    for (const primitive of testCase.requiredPrimitives || []) {
      assert(
        failures,
        `${testCase.command}: required primitive ${primitive} is present`,
        Number(command?.descriptorKinds?.primitives?.[primitive] || 0) > 0,
        command,
      );
    }
    for (const viewKind of testCase.requiredViewKinds || []) {
      assert(
        failures,
        `${testCase.command}: required view kind ${viewKind} is present`,
        Number(command?.descriptorKinds?.viewKinds?.[viewKind] || 0) > 0,
        command,
      );
    }
    assert(failures, `${testCase.command}: diagnostics snapshot mirrors selected command`, diagnostics.selectedCommand === testCase.command, diagnostics);
    assert(failures, `${testCase.command}: diagnostics snapshot mirrors evidence kind`, diagnostics.evidenceKind === testCase.evidenceKind, diagnostics);
    assert(failures, `${testCase.command}: diagnostics snapshot includes evidence report`, diagnostics.evidenceReport?.kind === testCase.evidenceKind, diagnostics);
  }
  return results;
}

async function checkPublicExamples(failures) {
  const site = await readText(SITE);
  const publicExamples = extractPublicExampleNames(site);
  const missingCommandCoverage = publicExamples.filter((name) => (
    !PUBLIC_COMMAND_EXEMPTIONS.has(name)
      && !PUBLIC_EXAMPLE_EXPECTED_COMMANDS.has(name)
  ));
  assert(
    failures,
    "docs/site public examples are covered by the expected command map",
    missingCommandCoverage.length === 0,
    {
      missingCommandCoverage,
      publicExamples,
      exemptions: Array.from(PUBLIC_COMMAND_EXEMPTIONS).sort(),
      configuredExamples: Array.from(PUBLIC_EXAMPLE_EXPECTED_COMMANDS.keys()).sort(),
    },
  );
  for (const name of publicExamples) {
    if (PUBLIC_COMMAND_EXEMPTIONS.has(name)) continue;
    const relativePath = `visual/examples/${name}/index.html`;
    const text = await readText(resolve(ROOT, relativePath));
    const expectedCommand = PUBLIC_EXAMPLE_EXPECTED_COMMANDS.get(name);
    const commandCalls = COMMANDS.filter((command) => commandCallPattern(command).test(text));
    const canvasCount = (text.match(/<canvas\b/gi) || []).length;
    const createVariables = createMetricVisualVariables(text);

    assert(failures, `${relativePath}: has expected public command mapping`, Boolean(expectedCommand), {
      expectedCommand,
      configuredExamples: Array.from(PUBLIC_EXAMPLE_EXPECTED_COMMANDS.keys()).sort(),
    });
    assert(failures, `${relativePath}: uses createMetricVisual`, /\bcreateMetricVisual\s*\(/.test(text), { commandCalls });
    assert(failures, `${relativePath}: stores createMetricVisual surface`, createVariables.length > 0, { createVariables });
    assert(failures, `${relativePath}: calls a semantic command`, commandCalls.length > 0, { commandCalls });
    if (expectedCommand) {
      assert(failures, `${relativePath}: calls ${expectedCommand}`, commandCalls.includes(expectedCommand), { commandCalls });
      assert(
        failures,
        `${relativePath}: calls ${expectedCommand} on the createMetricVisual surface`,
        createVariables.some((name) => surfaceCommandPattern(name, expectedCommand).test(text)),
        { createVariables, commandCalls },
      );
    }
    assert(failures, `${relativePath}: createMetricVisual is not used as a view-only renderer`, !createMetricVisualViewOptionPattern().test(text));
    assert(failures, `${relativePath}: owns at most one render canvas`, canvasCount <= 1, { canvasCount });
    assertPageLocalEvidenceMutation(failures, relativePath, text);

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

function createMetricVisualVariables(text) {
  return Array.from(
    text.matchAll(/\b(?:const|let|var)\s+([A-Za-z_$][\w$]*)\s*=\s*await\s+createMetricVisual\s*\(/g),
    (match) => match[1],
  );
}

function surfaceCommandPattern(variableName, command) {
  return new RegExp(`\\b${escapeRegExp(variableName)}\\s*\\.\\s*${command}\\s*\\(`);
}

function createMetricVisualViewOptionPattern() {
  return /\bcreateMetricVisual\s*\(\s*\{[\s\S]*?\bview\s*:/;
}

const VISUAL_DOCUMENT_FIELDS = [
  "schema",
  "provenance",
  "datasets",
  "records",
  "relations",
  "spaces",
  "properties",
  "graphs",
  "coordinates",
  "timelines",
  "events",
  "views",
  "diagnostics",
];

const VISUAL_DOCUMENT_ARRAY_FIELDS = VISUAL_DOCUMENT_FIELDS.filter((field) => (
  field !== "schema" && field !== "provenance"
));

function assertPageLocalEvidenceMutation(failures, relativePath, html) {
  const script = extractInlineScriptText(html);
  const evidenceUses = createMetricVisualEvidenceUses(script);
  assert(
    failures,
    `${relativePath}: createMetricVisual receives a direct fetched evidence variable`,
    evidenceUses.length > 0,
    { evidenceUses },
  );
  for (const use of evidenceUses) {
    const binding = findJsonBindingForVariable(script, use.variable, use.index);
    assert(
      failures,
      `${relativePath}: ${use.variable} is bound directly from response.json() before createMetricVisual`,
      Boolean(binding),
      { variable: use.variable, evidenceProperty: use.property },
    );
    if (!binding) continue;
    const segment = script.slice(binding.end, use.index);
    const mutations = findMaterialEvidenceMutations(segment, use.variable);
    assert(
      failures,
      `${relativePath}: ${use.variable} is not materially rewritten before createMetricVisual`,
      mutations.length === 0,
      { variable: use.variable, mutations },
    );
  }
}

function extractInlineScriptText(html) {
  return Array.from(html.matchAll(/<script\b[^>]*>([\s\S]*?)<\/script>/gi), (match) => match[1]).join("\n");
}

function createMetricVisualEvidenceUses(script) {
  const uses = [];
  const createPattern = /\bcreateMetricVisual\s*\(/g;
  for (const match of script.matchAll(createPattern)) {
    const window = script.slice(match.index, match.index + 5000);
    const property = /\b(evidence|document|input)\s*:\s*([A-Za-z_$][\w$]*)\b/.exec(window);
    if (!property) continue;
    uses.push({
      property: property[1],
      variable: property[2],
      index: match.index + property.index,
    });
  }
  return uses;
}

function findJsonBindingForVariable(script, variable, beforeIndex) {
  const bindingPattern = /\b(?:const|let|var)\s+([A-Za-z_$][\w$]*)\s*=\s*await\s+([A-Za-z_$][\w$]*)\.json\s*\(\s*\)\s*;?/g;
  let found = null;
  for (const match of script.matchAll(bindingPattern)) {
    if (match[1] !== variable || match.index >= beforeIndex) continue;
    found = {
      variable: match[1],
      responseVariable: match[2],
      index: match.index,
      end: match.index + match[0].length,
    };
  }
  return found;
}

function findMaterialEvidenceMutations(segment, variable) {
  const fieldAccess = fieldAccessPattern(variable, VISUAL_DOCUMENT_FIELDS);
  const arrayAccess = fieldAccessPattern(variable, VISUAL_DOCUMENT_ARRAY_FIELDS);
  const checks = [
    ["top-level field assignment", new RegExp(`${fieldAccess}\\s*=`, "m")],
    ["top-level field deletion", new RegExp(`\\bdelete\\s+${fieldAccess}`, "m")],
    ["top-level array entry assignment", new RegExp(`${arrayAccess}\\s*\\[[^\\]]+\\]\\s*=`, "m")],
    ["top-level array item field assignment", new RegExp(`${arrayAccess}\\s*\\[[^\\]]+\\]\\s*(?:\\.\\s*[A-Za-z_$][\\w$]*|\\[\\s*["'][^"']+["']\\s*\\])\\s*=`, "m")],
    ["top-level array mutation", new RegExp(`${arrayAccess}\\s*\\.\\s*(?:copyWithin|fill|pop|push|reverse|shift|sort|splice|unshift)\\s*\\(`, "m")],
    ["object mutation helper", new RegExp(`\\bObject\\.(?:assign|defineProperties|defineProperty)\\s*\\(\\s*${escapeRegExp(variable)}\\b`, "m")],
  ];
  return checks
    .filter(([, pattern]) => pattern.test(segment))
    .map(([label]) => label);
}

function fieldAccessPattern(variable, fields) {
  const fieldAlternation = fields.map(escapeRegExp).join("|");
  return `\\b${escapeRegExp(variable)}\\s*(?:\\.\\s*(?:${fieldAlternation})|\\[\\s*["'](?:${fieldAlternation})["']\\s*\\])`;
}

function extractPublicExampleNames(text) {
  return Array.from(
    new Set(Array.from(text.matchAll(/visual\/examples\/([^/"']+)\/index\.html/g), (match) => match[1])),
  ).sort();
}

function findCoordinateId(document, options = {}) {
  return findItemId(document?.coordinates, {
    ...options,
    dimension: options.dimension == null ? null : Number(options.dimension),
    matches: (coordinate) => options.dimension == null || Number(coordinate?.dimension) === Number(options.dimension),
  });
}

function findRelationId(document, options = {}) {
  return findItemId(document?.relations, options);
}

function findGraphId(document, options = {}) {
  return findItemId(document?.graphs, options);
}

function findTimelineId(document, options = {}) {
  return findItemId(document?.timelines, options);
}

function findPropertyId(document, options = {}) {
  return findItemId(document?.properties, {
    ...options,
    matches: (property) => {
      if (options.targetType && (property?.target_type || "record") !== options.targetType) return false;
      if (options.valueType && property?.value_type !== options.valueType) return false;
      return true;
    },
  });
}

function findItemId(items = [], options = {}) {
  const matches = (items || []).filter((item) => {
    if (!item?.id || item.id === options.exclude) return false;
    return options.matches ? options.matches(item) : true;
  });
  if (!matches.length) return null;
  const preferred = options.prefer
    ? matches.find((item) => options.prefer.test(`${item.id} ${item.name || ""} ${item.kind || ""}`))
    : null;
  return (preferred || matches[0]).id;
}

function escapeRegExp(value) {
  return String(value).replace(/[.*+?^${}()|[\]\\]/g, "\\$&");
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
