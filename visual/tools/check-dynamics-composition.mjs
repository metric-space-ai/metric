#!/usr/bin/env node

import { readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import { MetricVisualSurface } from "../src/metric-visual.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const EVIDENCE_PATH = "docs/examples/assets/dynamics-noise/metric.visual.json";
const PAGE_PATH = "visual/examples/dynamics-noise-hero/index.html";

const HERO_OPTIONS = {
  timelineId: "reverse-reconstruction",
  field: true,
  preview: "state-history",
  loop: true,
  timelineProgress: 0.5,
  timelineFieldPropertyId: "reverse-mse-to-clean",
  fieldPropertyId: "best-reconstruction-error",
  fieldMode: "lifted",
  fieldAlpha: 0.42,
  fieldRadius: 0.2,
  fieldLift: 0.58,
  fieldMaterial: { contour: 0.22, glow: 0.16 },
  pathColorPropertyId: "reconstruction-improvement",
  pathWidthPropertyId: "best-reconstruction-error",
  trajectoryPathLimit: 192,
  trajectoryPathSelection: "deterministic-record-stride",
  timelineContext: true,
  timelineContextStops: [0, 0.5, 1],
  timelineContextAlpha: 0.24,
  timelineContextPointSize: 1.18,
  pathWidth: 5.2,
  pathAlpha: 0.82,
  pointSize: 2.08,
  timelineControl: { root: {} },
};

async function main() {
  const document = JSON.parse(await readFile(resolve(ROOT, EVIDENCE_PATH), "utf8"));
  const pageSource = await readFile(resolve(ROOT, PAGE_PATH), "utf8");
  const surface = createHeadlessSurface(document);
  surface.showDynamics(HERO_OPTIONS);

  const state = surface.getState();
  const diagnostics = surface.getDiagnostics();
  const descriptors = state.descriptors;
  const primitives = new Set(descriptors.map((descriptor) => descriptor.primitive));
  const trajectory = descriptors.find((descriptor) => descriptor.primitive === "CurveRibbonLayer"
    && descriptor.metadata?.evidenceRole === "trajectory/path");
  const field = descriptors.find((descriptor) => descriptor.primitive === "HeatFieldLayer"
    && descriptor.metadata?.evidenceRole === "timeline-ground-field");
  const point = descriptors.find((descriptor) => descriptor.primitive === "InstancedPointLayer"
    && descriptor.metadata?.role === "current-timeline-state");
  const contexts = descriptors.filter((descriptor) => descriptor.metadata?.role === "timeline-state-history-context");
  const timelineControl = point?.metadata?.timelineControl || trajectory?.metadata?.timelineControl;
  const fetchTargets = Array.from(pageSource.matchAll(/\bfetch\(([^)]+)\)/g)).map((match) => match[1]);

  const checks = [];
  assert(checks, "selected view kind is dynamics", diagnostics.selectedViewKind === "dynamics", diagnostics);
  assert(checks, "composition emits CurveRibbonLayer, HeatFieldLayer and InstancedPointLayer",
    ["CurveRibbonLayer", "HeatFieldLayer", "InstancedPointLayer"].every((primitive) => primitives.has(primitive)),
    Array.from(primitives));
  assert(checks, "trajectory is primary DynamicsView state-history evidence",
    trajectory?.metadata?.pathSource === "exported-timeline-states"
      && trajectory?.metadata?.motionGrammar === "timeline-trajectory-state-history"
      && trajectory?.metadata?.nativeEvidence?.source === "exported-timeline-states"
      && trajectory?.metadata?.nativeEvidence?.timelineId === "reverse-reconstruction"
      && trajectory?.metadata?.coordinateIds?.length === 41,
    trajectory?.metadata);
  assert(checks, "trajectory display is thinned without mutating native evidence",
    trajectory?.metadata?.trajectorySelection?.source === "exported-timeline-states"
      && trajectory?.metadata?.trajectorySelection?.candidateCount === 512
      && trajectory?.metadata?.trajectorySelection?.selectedCount === 192
      && trajectory?.metadata?.trajectorySelection?.evidenceMutation === false
      && trajectory?.metadata?.trajectorySelection?.algorithmicComputation === false,
    trajectory?.metadata?.trajectorySelection);
  assert(checks, "trajectory path color and width use exported record properties",
    trajectory?.metadata?.pathColorPropertyId === "reconstruction-improvement"
      && trajectory?.metadata?.pathWidthPropertyId === "best-reconstruction-error"
      && trajectory?.metadata?.pathVisualEncoding?.source === "exported-record-property"
      && trajectory?.metadata?.pathVisualEncoding?.algorithmicComputation === false,
    trajectory?.metadata?.pathVisualEncoding);
  assert(checks, "field uses exported record property plus exported timeline property state",
    field?.metadata?.activeFieldPropertyId === "best-reconstruction-error"
      && field?.metadata?.nativeEvidence?.source === "exported-record-property"
      && field?.metadata?.nativeEvidence?.timelinePropertyId === "reverse-mse-to-clean"
      && field?.metadata?.timelineFieldState?.propertyId === "reverse-mse-to-clean"
      && field?.metadata?.timelineFieldState?.source === "exported-property"
      && field?.metadata?.supportEvidenceRole === "propagation-or-uncertainty-field",
    field?.metadata);
  assert(checks, "timeline context shows exported past and future states",
    contexts.some((descriptor) => descriptor.metadata?.timelineContextRole === "past")
      && contexts.some((descriptor) => descriptor.metadata?.timelineContextRole === "future")
      && contexts.every((descriptor) => descriptor.channels?.position?.count === 512
        && descriptor.metadata?.algorithmicComputation === false),
    contexts.map((descriptor) => descriptor.metadata));
  assert(checks, "current state resolves exported timeline and record preview metadata",
    point?.metadata?.timelineState?.schema === "metric.visual.timeline_state_sample.v1"
      && point?.metadata?.timelineState?.activeCoordinateId === "coord-reverse-20"
      && point?.metadata?.timelineFieldState?.propertyId === "reverse-mse-to-clean"
      && point?.picking?.mode === "record-id",
    point?.metadata);
  assert(checks, "timeline controls are metadata-driven user-facing controls",
    timelineControl?.schema === "metric.visual.timeline_control.v1"
      && timelineControl?.userFacing === true
      && timelineControl?.controls?.map((control) => control.role).join(",") === "timeline-state,timeline-playback,timeline-reset"
      && timelineControl?.controls?.[0]?.binding?.source === "exported-timeline",
    timelineControl);
  assert(checks, "page loads only the native dynamics evidence path",
    fetchTargets.length === 1 && fetchTargets[0].includes(EVIDENCE_PATH),
    fetchTargets);
  assert(checks, "page stays on createMetricVisual/showDynamics without a local renderer",
    pageSource.includes("createMetricVisual")
      && /\.showDynamics\(/.test(pageSource)
      && !/\b(?:new\s+THREE|requestAnimationFrame|\.getContext\s*\(|drawArrays|drawElements|function\s+render)\b/.test(pageSource),
    null);
  assert(checks, "page does not declare debug timeline controls",
    !/<input\b/i.test(pageSource)
      && !/<button\b/i.test(pageSource)
      && !/debug|devtools|synthetic/i.test(pageSource),
    null);
  assert(checks, "native evidence is not synthetic",
    document.provenance?.native_export === true
      && document.provenance?.synthetic_js === false
      && document.provenance?.synthetic !== true,
    document.provenance);

  report(checks);
}

function createHeadlessSurface(document) {
  const runtime = new FakeRuntime();
  return new MetricVisualSurface({
    document,
    canvas: {
      parentElement: {},
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

  setDocument(document) {
    this.document = document;
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

  on() {
    return () => {};
  }

  getState() {
    return {
      layerInstanceCount: this.layers.length,
      layerDescriptorCount: this.descriptors.length,
      layerState: { count: this.layers.length, status: "headless" },
    };
  }
}

function assert(checks, message, ok, details = {}) {
  checks.push({ ok: Boolean(ok), message, details: ok ? undefined : details });
}

function report(checks) {
  const failures = checks
    .filter((check) => !check.ok)
    .map(({ message, details }) => ({ message, details }));
  console.log(JSON.stringify({
    ok: failures.length === 0,
    evidence: EVIDENCE_PATH,
    page: PAGE_PATH,
    total: checks.length,
    failed: failures.length,
    failures,
  }, null, 2));
  if (failures.length) process.exitCode = 1;
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
