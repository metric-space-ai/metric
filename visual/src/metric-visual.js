import { loadVisualDocument, normalizeVisualInput, VisualSpace } from "./data/index.js";
import { createLayerFromDescriptor } from "./layers/index.js";
import { MetricVisualRuntime } from "./runtime/index.js";
import { createMiniatureHeroRuntimeOptions } from "./style/miniature/index.js";
import { createTubeRibbonPathLayerDescriptor } from "./curves/index.js";
import { RecordPreviewPanel } from "./interaction/index.js";
import {
  DenseFieldView,
  DynamicsView,
  MappingView,
  MetricSpaceView,
  NeighborhoodGraphView,
  RelationMatrixView,
  SpacePropertiesView,
} from "./views/index.js";
import { VisualLayer } from "./views/VisualLayer.js";
import { createChannel, createStringChannel } from "./views/view-utils.js";

const METRIC_VISUAL_API_DIAGNOSTICS_SCHEMA = "metric.visual.public_api_diagnostics.v1";
const METRIC_VISUAL_COMMAND_DIAGNOSTICS_SCHEMA = "metric.visual.public_command_diagnostics.v1";
const METRIC_VISUAL_DOCUMENT_DIAGNOSTICS_SCHEMA = "metric.visual.public_document_diagnostics.v1";

/**
 * Create the public METRIC visual surface.
 *
 * This is the reusable engine entry point. It owns a MetricVisualRuntime and
 * exposes semantic commands; hero pages are expected to call these commands
 * instead of assembling renderers or page-specific layer pipelines.
 */
export async function createMetricVisual(options = {}) {
  const document = await resolveMetricVisualInput(options.evidence ?? options.document ?? options.url ?? options.input, options);
  const canvas = resolveMetricVisualCanvas(options);
  const setup = createMiniatureHeroRuntimeOptions({
    canvas,
    layerFactory: createLayerFromDescriptor,
    profileOptions: { id: options.profileId || "metric-visual-profile", ...(options.profileOptions || {}) },
    controls: { enabled: true, rotateSpeed: 0.0045, ...(options.controls || {}) },
    hoverFocus: {
      enabled: true,
      thresholdPx: 38,
      clearOnLeave: false,
      ...(options.hoverFocus || {}),
    },
    ...(options.runtime || {}),
  });
  const runtime = new MetricVisualRuntime(setup.runtimeOptions);
  setup.style?.attachRuntime?.(runtime);
  runtime.setDocument(document, { views: false });
  const surface = new MetricVisualSurface({ document, canvas, runtime, setup, options });

  if (options.view) {
    surface.show(options.view, options);
  }
  if (options.start !== false) runtime.start();
  runtime.renderOnce();
  return surface;
}

export async function showMetricSpace(evidence, options = {}) {
  const visual = await createMetricVisual({ ...options, evidence });
  return visual.showMetricSpace(options);
}

export async function showRelationMatrix(evidence, options = {}) {
  const visual = await createMetricVisual({ ...options, evidence });
  return visual.showRelationMatrix(options);
}

export async function showNeighborhoodGraph(evidence, options = {}) {
  const visual = await createMetricVisual({ ...options, evidence });
  return visual.showNeighborhoodGraph(options);
}

export async function showSpaceProperties(evidence, options = {}) {
  const visual = await createMetricVisual({ ...options, evidence });
  return visual.showSpaceProperties(options);
}

export async function showMapping(evidence, options = {}) {
  const visual = await createMetricVisual({ ...options, evidence });
  return visual.showMapping(options);
}

export async function showDynamics(evidence, options = {}) {
  const visual = await createMetricVisual({ ...options, evidence });
  return visual.showDynamics(options);
}

export function createMetricVisualDocumentDiagnostics(document, options = {}) {
  const context = describeMetricVisualContext(options);
  const warnings = createMetricVisualDocumentWarnings(document, context);
  return {
    schema: METRIC_VISUAL_DOCUMENT_DIAGNOSTICS_SCHEMA,
    loadedSchema: document?.schema || null,
    recordCount: countVisualRecords(document),
    synthetic: document?.provenance?.synthetic === true,
    context,
    warnings,
  };
}

export class MetricVisualSurface {
  constructor({ document, canvas, runtime, setup, options = {} }) {
    this.document = document;
    this.visualSpace = VisualSpace.fromDocument(document);
    this.canvas = canvas;
    this.runtime = runtime;
    this.setup = setup;
    this.views = [];
    this.descriptors = [];
    this.previewPanel = null;
    this.previewSubscriptions = [];
    this.documentDiagnostics = createMetricVisualDocumentDiagnostics(document, options);
    this.commandDiagnostics = [];
    this.lastCommandDiagnostics = null;
    this.diagnostics = null;
    this.recordCommandDiagnostics({ command: "createMetricVisual", viewKind: null });
  }

  show(kind, options = {}) {
    const normalized = String(kind).replace(/_/g, "-");
    if (normalized === "metric-space") return this.showMetricSpace(options);
    if (normalized === "relation-matrix") return this.showRelationMatrix(options);
    if (normalized === "neighborhood-graph") return this.showNeighborhoodGraph(options);
    if (normalized === "space-properties" || normalized === "properties") return this.showSpaceProperties(options);
    if (normalized === "mapping") return this.showMapping(options);
    if (normalized === "dynamics") return this.showDynamics(options);
    if (normalized === "condition-monitoring") return this.showConditionMonitoring(options);
    if (normalized === "mixed-records") return this.showMixedRecords(options);
    if (normalized === "cross-space") return this.showCrossSpace(options);
    if (normalized === "relation-matrix-neighborhood") return this.showRelationMatrixNeighborhood(options);
    throw new Error(`Unsupported METRIC visual view: ${kind}`);
  }

  setViews(views, options = {}) {
    const descriptors = [];
    for (const view of toMetricVisualArray(views)) {
      if (!view) continue;
      if (typeof view.toLayerDescriptors === "function") {
        descriptors.push(...view.toLayerDescriptors());
      } else {
        descriptors.push(view);
      }
    }
    return this.setLayerDescriptors(descriptors, options);
  }

  setLayerDescriptors(descriptors, options = {}) {
    const next = toMetricVisualArray(descriptors).filter(Boolean);
    this.descriptors = options.append ? this.descriptors.concat(next) : next;
    this.runtime.setLayerDescriptors(this.descriptors, { source: options.source || "metric-visual-surface" });
    this.updatePreviewIndex();
    this.runtime.renderOnce();
    this.recordCommandDiagnostics({
      command: options.source || "setLayerDescriptors",
      viewKind: options.viewKind,
      append: options.append === true,
    });
    return this;
  }

  addLayerDescriptors(descriptors, options = {}) {
    return this.setLayerDescriptors(descriptors, { ...options, append: true });
  }

  setMotion(motion = "studio-drift", options = {}) {
    if (motion === false || motion === "none" || motion == null) {
      this.setup.style?.detachStyleMotion?.();
      this.runtime.renderOnce();
      return this;
    }
    this.setup.style?.setStyleMotion?.(motion, { attach: options.attach !== false });
    if (options.start !== false) this.runtime.start();
    this.runtime.renderOnce();
    return this;
  }

  enableMotion(motion = "studio-drift", options = {}) {
    return this.setMotion(motion, options);
  }

  setCamera(options = {}) {
    if (options === false || options == null) return this;
    this.runtime.setCameraOptions(options);
    this.setup.style?.styleMotionController?.resetBase?.();
    this.runtime.renderOnce();
    return this;
  }

  showMetricSpace(options = {}) {
    const view = MetricSpaceView.fromVisualSpace(this.document, normalizeMetricViewOptions(this.document, options));
    this.views = [view];
    const descriptors = view.toLayerDescriptors();
    if (options.groundField) {
      descriptors.splice(1, 0, ...propertyFieldDescriptors(this.document, options, view));
    }
    this.setLayerDescriptors(descriptors, { source: "showMetricSpace", viewKind: "metric-space" });
    return this.configurePreview(options);
  }

  showRelationMatrix(options = {}) {
    const view = RelationMatrixView.fromVisualSpace(this.document, normalizeRelationOptions(this.document, options));
    this.views = [view];
    this.setViews([view], { source: "showRelationMatrix", viewKind: "relation-matrix" });
    return this.configurePreview(options);
  }

  showNeighborhoodGraph(options = {}) {
    const view = NeighborhoodGraphView.fromVisualSpace(this.document, normalizeGraphOptions(this.document, options));
    this.views = [view];
    this.setViews([view], { source: "showNeighborhoodGraph", viewKind: "neighborhood-graph" });
    return this.configurePreview(options);
  }

  showSpaceProperties(options = {}) {
    const normalized = normalizePropertyOptions(this.document, options);
    const view = SpacePropertiesView.fromVisualSpace(this.document, normalized);
    this.views = [view];
    const descriptors = view.toLayerDescriptors();
    descriptors.splice(1, 0, ...propertyFieldDescriptors(this.document, normalized, view.space));
    this.setLayerDescriptors(descriptors, { source: "showSpaceProperties", viewKind: "space-properties" });
    return this.configurePreview(options);
  }

  showMapping(options = {}) {
    const view = MappingView.fromVisualSpace(this.document, normalizeMappingOptions(this.document, options));
    this.views = [view];
    const descriptors = view.toLayerDescriptors();
    const residual = residualVectorDescriptor(view, options);
    if (residual) descriptors.push(residual);
    this.setLayerDescriptors(descriptors, { source: "showMapping", viewKind: "mapping" });
    return this.configurePreview(options);
  }

  showDynamics(options = {}) {
    const view = DynamicsView.fromVisualSpace(this.document, normalizeMetricViewOptions(this.document, options));
    this.views = [view];
    const descriptors = view.toLayerDescriptors();
    if (options.propertyField || options.groundField) {
      descriptors.unshift(...propertyFieldDescriptors(this.document, {
        ...options,
        coordinateId: view.fittedStates?.[view.activeStep]?.coordinateId,
        propertyId: options.propertyField || options.groundField,
      }));
    }
    this.setLayerDescriptors(descriptors, { source: "showDynamics", viewKind: "dynamics" });
    return this.configurePreview(options);
  }

  showConditionMonitoring(options = {}) {
    const normalized = normalizeMetricViewOptions(this.document, {
      coordinateId: options.coordinateId || options.coordinate || "condition-3d",
      scalarProperty: options.scalarProperty || options.colorBy || "anomaly",
      labelProperty: options.labelProperty || options.labels || "cluster",
      pointSize: options.pointSize ?? 1,
      targetRadius: options.targetRadius ?? 1.54,
      groundScale: options.groundScale ?? 1.82,
      groundGeometry: {
        gridScale: 10,
        ...(options.groundGeometry || {}),
      },
      groundMaterial: {
        alpha: 0.42,
        gridAlpha: 0.055,
        axisAlpha: 0.025,
        baseColor: [0.83, 0.86, 0.81],
        gridColor: [0.33, 0.4, 0.38],
        axisXColor: [0.36, 0.42, 0.41],
        axisZColor: [0.41, 0.37, 0.32],
        ...(options.groundMaterial || {}),
      },
      groundProjectionAlpha: options.groundProjectionAlpha ?? 0.16,
      labelFontSize: options.labelFontSize ?? 28,
      labelLift: options.labelLift ?? 0.46,
      labelOffsetRadius: options.labelOffsetRadius ?? 0.16,
      labelMap: {
        healthy: "normal regime",
        drift: "drift regime",
        fault: "fault regime",
        recovery: "recovery",
        ...(options.labelMap || {}),
      },
      ...options,
    });
    const space = MetricSpaceView.fromVisualSpace(this.document, normalized);
    this.views = [space];
    const descriptors = [
      ...space.toLayerDescriptors(),
      ...propertyFieldDescriptors(this.document, {
        ...normalized,
        propertyId: options.groundField || options.propertyField || "density",
        alpha: options.fieldAlpha ?? 0.2,
        radius: options.fieldRadius ?? 0.16,
        material: {
          contour: 0.1,
          glow: 0.04,
          ...(options.fieldMaterial || {}),
        },
      }, space),
    ];
    const path = recordTrajectoryDescriptor(space, {
      id: options.pathId || "condition-monitoring:trajectory",
      pathCount: options.pathCount || 16,
      width: options.pathWidth ?? 3.4,
      alpha: options.pathAlpha ?? 0.68,
      color: options.pathColor || [0.12, 0.36, 0.42, 0.68],
      order: 32,
    });
    if (path) descriptors.push(path);
    this.setLayerDescriptors(descriptors, { source: "showConditionMonitoring", viewKind: "condition-monitoring" });
    if (options.camera !== false) {
      this.setCamera(options.camera || conditionMonitoringCamera());
    }
    this.setMotion(options.motion ?? conditionMonitoringMotion(), { start: options.startMotion !== false });
    return this.configurePreview({ ...options, preview: options.preview ?? "time-series" });
  }

  showMixedRecords(options = {}) {
    const normalized = normalizeMetricViewOptions(this.document, {
      coordinateId: options.coordinateId || options.coordinate || "layout-3d",
      colorProperty: options.colorProperty || options.glyphBy || options.recordType || "family",
      labelProperty: options.labelProperty || options.labels || "family",
      pointSize: options.pointSize ?? 1.15,
      ...options,
    });
    const space = MetricSpaceView.fromVisualSpace(this.document, normalized);
    this.views = [space];
    const descriptors = space.toLayerDescriptors().map((descriptor) => (
      descriptor.primitive === "InstancedPointLayer"
        ? {
          ...descriptor,
          kind: "typed-glyph-scene",
          primitive: "InstancedGlyphLayer",
          metadata: { ...descriptor.metadata, role: "typed-glyphs", glyphBy: normalized.colorProperty || "record_type" },
        }
        : descriptor
    ));
    const graph = NeighborhoodGraphView.fromVisualSpace(this.document, {
      coordinateId: normalized.coordinateId,
      relationId: options.relationId || options.relation || "cross-metric",
      colorProperty: normalized.colorProperty,
      nodes: false,
      topK: options.topK ?? 4,
    });
    descriptors.push(...graph.toLayerDescriptors());
    this.setLayerDescriptors(descriptors, { source: "showMixedRecords", viewKind: "mixed-records" });
    return this.configurePreview({ ...options, preview: options.preview ?? "record" });
  }

  showCrossSpace(options = {}) {
    const left = MetricSpaceView.fromVisualSpace(this.document, normalizeMetricViewOptions(this.document, {
      coordinateId: options.leftCoordinateId || options.coordinateA || "space-a-3d",
      scalarProperty: options.scalarProperty || options.dependenceProperty || "local-dependence",
      ground: false,
      groundProjection: false,
      labels: false,
      targetRadius: 1.05,
    }));
    const right = MetricSpaceView.fromVisualSpace(this.document, normalizeMetricViewOptions(this.document, {
      coordinateId: options.rightCoordinateId || options.coordinateB || "space-b-3d",
      scalarProperty: options.scalarProperty || options.dependenceProperty || "local-dependence",
      ground: false,
      groundProjection: false,
      labels: false,
      targetRadius: 1.05,
    }));
    offsetPositionMap(left.positions, [-1.18, 0, 0]);
    offsetPositionMap(right.positions, [1.18, 0, 0]);
    this.views = [left, right];
    const descriptors = [
      sharedGroundDescriptor("cross-space:ground", { y: this.setup.stage?.grounding?.groundY ?? -0.58, width: 5.2, depth: 3.2 }),
      ...left.toLayerDescriptors(),
      ...right.toLayerDescriptors(),
    ];
    const bridge = pairedRecordBridgeDescriptor(left, right, options);
    if (bridge) descriptors.push(bridge);
    this.setLayerDescriptors(descriptors, { source: "showCrossSpace", viewKind: "cross-space" });
    return this.configurePreview({ ...options, preview: options.preview ?? "paired records" });
  }

  showRelationMatrixNeighborhood(options = {}) {
    const matrix = RelationMatrixView.fromVisualSpace(this.document, normalizeRelationOptions(this.document, {
      relationId: options.relationId || options.relation || "catalog-metric",
      rect: options.matrixRect || [0.57, 0.2, 0.39, 0.62],
      palette: options.palette || "metric",
      symmetric: options.symmetric ?? true,
      missingAlpha: 0,
      materialAlpha: 0.98,
    }));
    const graph = NeighborhoodGraphView.fromVisualSpace(this.document, normalizeGraphOptions(this.document, {
      coordinateId: options.coordinateId || options.coordinate || "catalog-3d",
      relationId: options.relationId || options.relation || "catalog-metric",
      colorProperty: options.colorProperty || "group",
      topK: options.topK ?? 5,
      size: options.pointSize ?? 1.2,
    }));
    this.views = [matrix, graph];
    this.setLayerDescriptors([
      ...graph.toLayerDescriptors(),
      ...matrix.toLayerDescriptors().map((descriptor) => ({ ...descriptor, order: 200 })),
    ], { source: "showRelationMatrixNeighborhood", viewKind: "relation-matrix-neighborhood" });
    return this.configurePreview({ ...options, preview: options.preview ?? "pair" });
  }

  recordCommandDiagnostics(options = {}) {
    const runtimeState = safeRuntimeState(this.runtime);
    const entry = {
      schema: METRIC_VISUAL_COMMAND_DIAGNOSTICS_SCHEMA,
      command: options.command || "metric-visual-command",
      loadedSchema: this.documentDiagnostics.loadedSchema,
      recordCount: this.documentDiagnostics.recordCount,
      selectedViewKind: options.viewKind ?? inferMetricVisualViewKind(this.views),
      layerDescriptorCount: this.descriptors.length,
      runtimeLayerCount: runtimeState.layerInstanceCount ?? countRuntimeLayers(this.runtime),
      runtimeLayerState: runtimeState.layerState ? { ...runtimeState.layerState } : null,
    };
    if (options.append === true) entry.append = true;
    this.lastCommandDiagnostics = entry;
    this.commandDiagnostics.push(entry);
    this.diagnostics = this.createDiagnosticsSnapshot(entry);
    return entry;
  }

  createDiagnosticsSnapshot(current = this.lastCommandDiagnostics) {
    const runtimeState = safeRuntimeState(this.runtime);
    return {
      schema: METRIC_VISUAL_API_DIAGNOSTICS_SCHEMA,
      loadedSchema: this.documentDiagnostics.loadedSchema,
      recordCount: this.documentDiagnostics.recordCount,
      selectedViewKind: current?.selectedViewKind ?? inferMetricVisualViewKind(this.views),
      layerDescriptorCount: current?.layerDescriptorCount ?? this.descriptors.length,
      runtimeLayerCount: current?.runtimeLayerCount ?? runtimeState.layerInstanceCount ?? countRuntimeLayers(this.runtime),
      warnings: cloneDiagnosticEntries(this.documentDiagnostics.warnings),
      commands: cloneDiagnosticEntries(this.commandDiagnostics),
    };
  }

  getDiagnostics() {
    this.diagnostics = this.createDiagnosticsSnapshot();
    return cloneMetricVisualDiagnostics(this.diagnostics);
  }

  configurePreview(options = {}) {
    if (options.preview === false) {
      this.disableRecordPreview();
    } else if (options.preview) {
      this.enableRecordPreview({ mode: options.preview, ...(options.previewOptions || {}) });
    }
    return this;
  }

  enableRecordPreview(options = {}) {
    if (!this.previewPanel) {
      this.previewPanel = new RecordPreviewPanel({
        root: options.root || document.body,
        resolver: (input) => buildRecordPreview(this, input),
        ...(options.panel || {}),
      });
    } else if (options.panel?.resolver) {
      this.previewPanel.setResolver(options.panel.resolver);
    }
    if (!this.previewSubscriptions.length) {
      this.previewSubscriptions.push(this.runtime.on("hoverfocuschange", ({ focusTarget, pointer }) => {
        if (!focusTarget?.recordId) {
          this.previewPanel?.hide();
          return;
        }
        const rect = this.canvas.getBoundingClientRect();
        this.previewPanel?.show({
          focusTarget,
          recordId: focusTarget.recordId,
          x: rect.left + (pointer?.x ?? pointer?.css?.x ?? 0),
          y: rect.top + (pointer?.y ?? pointer?.css?.y ?? 0),
        });
      }));
      this.previewSubscriptions.push(this.runtime.on("selectionchange", ({ selection }) => {
        if (!selection?.recordId) return;
        this.previewPanel?.show({
          recordId: selection.recordId,
          focusTarget: { recordId: selection.recordId },
          x: window.innerWidth * 0.58,
          y: window.innerHeight * 0.34,
        });
      }));
      this.previewPointerMoveBound ||= ((event) => this.handlePreviewPointerMove(event));
      this.hideRecordPreviewBound ||= (() => this.previewPanel?.hide());
      this.canvas.addEventListener("pointermove", this.previewPointerMoveBound);
      this.canvas.addEventListener("pointerleave", this.hideRecordPreviewBound);
    }
    this.previewMode = options.mode || "record";
    return this;
  }

  disableRecordPreview() {
    for (const unsubscribe of this.previewSubscriptions.splice(0)) unsubscribe?.();
    if (this.previewPointerMoveBound) this.canvas.removeEventListener("pointermove", this.previewPointerMoveBound);
    if (this.hideRecordPreviewBound) this.canvas.removeEventListener("pointerleave", this.hideRecordPreviewBound);
    this.previewPanel?.hide();
    return this;
  }

  updatePreviewIndex() {
    this.previewRecords = [];
    for (const view of this.views || []) {
      collectPreviewRecords(this.previewRecords, view);
    }
    return this;
  }

  handlePreviewPointerMove(event) {
    if (!this.previewPanel || !this.previewRecords?.length) return;
    const hit = this.pickPreviewRecord(event);
    if (!hit) {
      this.previewPanel.hide();
      return;
    }
    this.previewPanel.show({
      recordId: hit.recordId,
      focusTarget: { recordId: hit.recordId, position: hit.position },
      x: event.clientX,
      y: event.clientY,
    });
  }

  pickPreviewRecord(event) {
    const rect = this.canvas.getBoundingClientRect();
    const pixelRatio = this.runtime.renderer?.size?.pixelRatio || window.devicePixelRatio || 1;
    const threshold = 34;
    let best = null;
    for (const entry of this.previewRecords || []) {
      const projected = this.runtime.camera?.projectToPixel?.(entry.position);
      if (!projected || projected.visible === false) continue;
      const x = rect.left + projected.x / pixelRatio;
      const y = rect.top + projected.y / pixelRatio;
      const distance = Math.hypot(event.clientX - x, event.clientY - y);
      if (distance <= threshold && (!best || distance < best.distance)) {
        best = { ...entry, distance, screen: [x, y] };
      }
    }
    return best;
  }

  captureHeroFrame(options = {}) {
    this.runtime.renderOnce(options.time);
    return this.canvas;
  }

  getState() {
    return {
      document: this.document,
      visualSpace: this.visualSpace,
      views: this.views.slice(),
      descriptors: this.descriptors.slice(),
      runtime: this.runtime.getState(),
      diagnostics: this.getDiagnostics(),
    };
  }
}

function describeMetricVisualContext(options = {}) {
  const labels = [];
  for (const key of ["context", "contextKind", "surfaceContext", "pageContext", "evidenceContext"]) {
    if (typeof options[key] === "string") labels.push(options[key]);
  }
  if (typeof options.gallery === "string") labels.push(options.gallery);
  if (typeof options.public === "string") labels.push(options.public);
  if (typeof options.publicGallery === "string") labels.push(options.publicGallery);
  const dataset = options.target?.dataset || options.root?.dataset || null;
  if (dataset) {
    for (const key of ["context", "visualContext", "metricVisualContext", "gallery", "publicGallery"]) {
      if (typeof dataset[key] === "string") labels.push(dataset[key]);
    }
  }

  const explicitPublicGallery = options.publicGallery === true
    || datasetBoolean(dataset?.publicGallery)
    || datasetBoolean(dataset?.metricPublicGallery);
  const pairedPublicGallery = options.public === true && options.gallery === true;
  const label = labels.join(" ");
  const publicGalleryLike = explicitPublicGallery
    || pairedPublicGallery
    || /\b(public[-_\s]*(gallery|hero|site)|gallery[-_\s]*public|docs[-_\s]*site|project[-_\s]*site)\b/i.test(label);

  return {
    publicGalleryLike,
    label: label || (explicitPublicGallery || pairedPublicGallery ? "public-gallery" : null),
  };
}

function createMetricVisualDocumentWarnings(document, context = {}) {
  if (document?.provenance?.synthetic !== true || !context.publicGalleryLike) return [];
  return [{
    code: "synthetic_evidence_in_public_gallery_context",
    severity: "warning",
    message: "Synthetic METRIC visual evidence was loaded in a public-gallery-like context. Public gallery pages should use native exported evidence.",
    loadedSchema: document?.schema || null,
    recordCount: countVisualRecords(document),
    context: context.label || "public-gallery",
    generator: document?.provenance?.generator || null,
  }];
}

function inferMetricVisualViewKind(views) {
  const kinds = toMetricVisualArray(views).map((view) => view?.kind).filter(Boolean);
  if (kinds.length === 0) return null;
  const unique = Array.from(new Set(kinds));
  return unique.length === 1 ? unique[0] : unique.join("+");
}

function countVisualRecords(document) {
  return Array.isArray(document?.records) ? document.records.length : null;
}

function countRuntimeLayers(runtime) {
  return Array.isArray(runtime?.layers) ? runtime.layers.length : null;
}

function safeRuntimeState(runtime) {
  try {
    return runtime?.getState?.() || {};
  } catch {
    return {};
  }
}

function datasetBoolean(value) {
  return value === true || value === "true" || value === "1" || value === "";
}

function cloneMetricVisualDiagnostics(diagnostics) {
  return {
    ...diagnostics,
    warnings: cloneDiagnosticEntries(diagnostics?.warnings),
    commands: cloneDiagnosticEntries(diagnostics?.commands),
  };
}

function cloneDiagnosticEntries(entries) {
  return Array.isArray(entries) ? entries.map((entry) => ({ ...entry })) : [];
}

function conditionMonitoringMotion() {
  return {
    mode: "studio-drift",
    loop: true,
    durationMs: 26000,
    focus: {
      enabled: true,
      yAmplitude: 0.008,
      radiusAmplitude: 0.014,
    },
    camera: {
      enabled: true,
      yawAmplitude: 0.018,
      pitchAmplitude: 0.006,
      radiusAmplitude: 0.01,
      respectInteraction: true,
    },
    postprocess: {
      enabled: true,
      frameAmountAmplitude: 0.008,
      focusLiftAmplitude: 0.003,
      floorSheenAmplitude: 0.014,
      stageGlowAmplitude: 0.012,
      subjectIsolationAmplitude: 0.01,
    },
  };
}

function conditionMonitoringCamera() {
  return {
    fov: 35,
    target: [-0.35, -0.1, 0],
    yaw: -0.52,
    pitch: 0.25,
    radius: 8.2,
  };
}

async function resolveMetricVisualInput(input, options = {}) {
  if (typeof input === "string") return loadVisualDocument(input, options.data || {});
  if (input && typeof input === "object") return normalizeVisualInput(input, options.data || {});
  throw new Error("createMetricVisual() requires evidence, document, url or input.");
}

function buildRecordPreview(surface, input = {}) {
  const recordId = input.recordId || input.focusTarget?.recordId;
  if (!recordId) return null;
  const record = surface.visualSpace.getRecord(recordId);
  if (!record) return null;
  const payload = record.payload || {};
  const fields = [
    { label: "id", value: record.id },
    { label: "type", value: record.record_type },
    { label: "dataset", value: record.dataset_id },
  ];
  const featureFields = payload.features && typeof payload.features === "object"
    ? Object.entries(payload.features).slice(0, 5).map(([label, value]) => ({ label, value }))
    : [];
  const preview = {
    title: record.label || record.id,
    subtitle: record.record_type || "record",
    fields: fields.concat(featureFields),
    sections: [],
  };
  if (Array.isArray(payload.series)) {
    preview.series = [{ label: "series", values: payload.series, color: "rgba(32, 118, 132, 0.86)" }];
  } else if (Array.isArray(payload.values) && payload.values.every((value) => Number.isFinite(Number(value)))) {
    preview.series = [{ label: "values", values: payload.values, color: "rgba(32, 118, 132, 0.86)" }];
  } else if (payload.values !== undefined) {
    preview.sections.push({
      title: "payload",
      fields: [{ label: "value", value: summarizePreviewValue(payload.values) }],
    });
  }
  return preview;
}

function collectPreviewRecords(out, view) {
  if (!view) return out;
  if (view.recordIds && view.positions) {
    for (const recordId of view.recordIds) {
      const position = getPosition(view.positions, recordId);
      if (position) out.push({ recordId, position });
    }
  }
  if (view.space) collectPreviewRecords(out, view.space);
  if (view.fittedStates?.length) {
    const state = view.fittedStates[view.activeStep || 0] || view.fittedStates[0];
    if (state?.positions) {
      for (const recordId of view.recordIds || []) {
        const position = getPosition(state.positions, recordId);
        if (position) out.push({ recordId, position });
      }
    }
  }
  return out;
}

function summarizePreviewValue(value) {
  if (Array.isArray(value)) return `${value.length} values`;
  if (value && typeof value === "object") return Object.keys(value).slice(0, 4).join(", ");
  return value;
}

function resolveMetricVisualCanvas(options = {}) {
  if (options.canvas instanceof HTMLCanvasElement) return options.canvas;
  const target = options.target || document.body;
  const canvas = document.createElement("canvas");
  canvas.className = options.canvasClass || "metric-visual-canvas";
  canvas.setAttribute("aria-label", options.ariaLabel || "METRIC visual");
  Object.assign(canvas.style, {
    position: options.canvasPosition || "absolute",
    inset: "0",
    width: "100%",
    height: "100%",
    display: "block",
    touchAction: "none",
  });
  if (target && target.appendChild) target.appendChild(canvas);
  return canvas;
}

function normalizeMetricViewOptions(document, options = {}) {
  const next = { ...options };
  if (next.coordinate && !next.coordinateId) next.coordinateId = next.coordinate;
  if (next.colorBy && !next.colorProperty && !next.scalarProperty) {
    const property = findVisualItem(document, "properties", next.colorBy);
    if (property?.value_type === "scalar" || property?.value_type === "rank") next.scalarProperty = property.id;
    else next.colorProperty = property?.id || next.colorBy;
  }
  if (next.labels && next.labels !== true && !next.labelProperty) next.labelProperty = next.labels;
  if (next.pointSize !== undefined && next.size === undefined) next.size = next.pointSize;
  return next;
}

function normalizeRelationOptions(document, options = {}) {
  const next = { ...options };
  if (next.relation && !next.relationId) next.relationId = next.relation;
  if (!next.relationId && document?.relations?.length) next.relationId = document.relations[0].id;
  return next;
}

function normalizeGraphOptions(document, options = {}) {
  return normalizeMetricViewOptions(document, normalizeRelationOptions(document, options));
}

function normalizePropertyOptions(document, options = {}) {
  const next = normalizeMetricViewOptions(document, options);
  if (next.property && !next.propertyId) next.propertyId = next.property;
  if (next.colorBy && !next.propertyId) next.propertyId = next.scalarProperty || next.colorBy;
  if (!next.propertyId && next.scalarProperty) next.propertyId = next.scalarProperty;
  return next;
}

function normalizeMappingOptions(document, options = {}) {
  const next = normalizeMetricViewOptions(document, options);
  if (next.residual && !next.residualProperty) next.residualProperty = next.residual;
  if (next.residualProperty && !next.scalarProperty) next.scalarProperty = next.residualProperty;
  if (next.morphDurationMs !== undefined && next.durationMs === undefined) next.durationMs = next.morphDurationMs;
  return next;
}

function propertyFieldDescriptors(document, options = {}, fittedSpace = null) {
  const propertyId = options.propertyId || options.property || options.groundField;
  if (!propertyId) return [];
  try {
    const field = DenseFieldView.fromVisualSpace(document, {
      ...options,
      propertyId,
      coordinateId: options.coordinateId || fittedSpace?.coordinateId,
      alpha: options.alpha ?? 0.34,
      radius: options.radius ?? 0.2,
      material: {
        alpha: options.alpha ?? 0.34,
        contour: 0.14,
        glow: 0.1,
        depthWrite: false,
        ...(options.material || {}),
      },
    });
    const descriptors = field.toLayerDescriptors();
    for (const descriptor of descriptors) {
      descriptor.order = options.order ?? -2;
      descriptor.metadata = { ...descriptor.metadata, role: "property-field", propertyId };
    }
    return descriptors;
  } catch {
    return [];
  }
}

function recordTrajectoryDescriptor(space, options = {}) {
  if (!space?.recordIds?.length || !space.positions) return null;
  const ids = space.recordIds.slice();
  const pathCount = Math.max(1, Math.floor(Number(options.pathCount) || 1));
  const segmentSize = Math.max(2, Math.ceil(ids.length / pathCount));
  const paths = [];
  for (let start = 0; start < ids.length; start += segmentSize) {
    const group = ids.slice(start, start + segmentSize);
    const points = [];
    for (const id of group) {
      const position = getPosition(space.positions, id);
      if (position) points.push(position);
    }
    if (points.length >= 2) {
      paths.push({
        id: `${options.id || space.id}:path-${paths.length}`,
        points,
        color: options.color || [0.14, 0.36, 0.46, options.alpha ?? 0.78],
        width: options.width ?? 4,
        metadata: { evidenceType: "record-order-trajectory", sourceView: space.id },
      });
    }
  }
  if (!paths.length) return null;
  const descriptor = createTubeRibbonPathLayerDescriptor({ paths }, {
    id: options.id || `${space.id}:trajectory`,
    order: options.order ?? 30,
    alpha: options.alpha ?? 0.78,
    defaultWidth: options.width ?? 4,
    colorMix: 1,
  });
  descriptor.metadata = { ...descriptor.metadata, role: "trajectory/path" };
  return descriptor;
}

function residualVectorDescriptor(mapping, options = {}) {
  const space = mapping?.space;
  if (!space?.targetPositions) return null;
  const ids = space.recordIds || [];
  const sourcePosition = new Float32Array(ids.length * 3);
  const targetPosition = new Float32Array(ids.length * 3);
  const color = new Float32Array(ids.length * 4);
  let count = 0;
  for (const id of ids) {
    const source = getPosition(space.positions, id);
    const target = getPosition(space.targetPositions, id);
    if (!source || !target) continue;
    sourcePosition.set(source, count * 3);
    targetPosition.set(target, count * 3);
    color.set(options.color || [0.8, 0.22, 0.12, 0.42], count * 4);
    count += 1;
  }
  if (!count) return null;
  return new VisualLayer({
    id: options.residualVectorId || `${mapping.id}:residual-vectors`,
    kind: "residual/error-vectors",
    primitive: "RelationEdgeLayer",
    order: 36,
    source: { viewId: mapping.id, viewKind: "mapping", role: "residual/error" },
    channels: {
      sourcePosition: createChannel(sourcePosition.subarray(0, count * 3), 3, "source-position"),
      targetPosition: createChannel(targetPosition.subarray(0, count * 3), 3, "target-position"),
      color: createChannel(color.subarray(0, count * 4), 4, "rgba"),
    },
    geometry: { width: options.width ?? 1 },
    material: { alpha: options.alpha ?? 0.62, transparent: true, depthWrite: false },
    metadata: { role: "residual/error", edgeCount: count },
  }).toDescriptor();
}

function pairedRecordBridgeDescriptor(left, right, options = {}) {
  const ids = (left.recordIds || []).filter((id) => getPosition(left.positions, id) && getPosition(right.positions, id));
  if (!ids.length) return null;
  const sourcePosition = new Float32Array(ids.length * 3);
  const targetPosition = new Float32Array(ids.length * 3);
  const color = new Float32Array(ids.length * 4);
  for (let index = 0; index < ids.length; index += 1) {
    sourcePosition.set(getPosition(left.positions, ids[index]), index * 3);
    targetPosition.set(getPosition(right.positions, ids[index]), index * 3);
    color.set(options.bridgeColor || [0.28, 0.43, 0.62, 0.18], index * 4);
  }
  return new VisualLayer({
    id: options.bridgeId || "cross-space:dependence-bridge",
    kind: "paired-space-dependence",
    primitive: "RelationEdgeLayer",
    order: 18,
    source: { viewKind: "paired-space", role: "dependence bridge" },
    channels: {
      recordId: createStringChannel(ids, "record-id"),
      sourcePosition: createChannel(sourcePosition, 3, "source-position"),
      targetPosition: createChannel(targetPosition, 3, "target-position"),
      color: createChannel(color, 4, "rgba"),
    },
    geometry: { width: options.width ?? 1 },
    material: { alpha: options.alpha ?? 0.58, transparent: true, depthWrite: false },
    metadata: { role: "dependence bridge", edgeCount: ids.length, linkedBrushing: true },
  }).toDescriptor();
}

function sharedGroundDescriptor(id, options = {}) {
  return new VisualLayer({
    id,
    kind: "ground-plane",
    primitive: "GroundPlaneLayer",
    order: -20,
    source: { role: "shared-stage" },
    channels: {},
    geometry: {
      width: options.width ?? 4.4,
      depth: options.depth ?? 3.2,
      y: options.y ?? -0.58,
      gridScale: options.gridScale ?? 8,
    },
    material: {
      alpha: options.alpha ?? 0.62,
      gridAlpha: options.gridAlpha ?? 0.28,
      axisAlpha: options.axisAlpha ?? 0.2,
    },
    metadata: { role: "shared-ground" },
  }).toDescriptor();
}

function offsetPositionMap(map, offset) {
  for (const [id, position] of map.entries()) {
    map.set(id, [
      (Number(position[0]) || 0) + offset[0],
      (Number(position[1]) || 0) + offset[1],
      (Number(position[2]) || 0) + offset[2],
    ]);
  }
}

function getPosition(map, id) {
  return map?.get?.(id) || map?.get?.(String(id)) || null;
}

function findVisualItem(document, collection, id) {
  if (id == null || id === true || id === false) return null;
  return (document?.[collection] || []).find((item) => String(item.id) === String(id) || String(item.name) === String(id)) || null;
}

function toMetricVisualArray(value) {
  if (value == null) return [];
  return Array.isArray(value) ? value : [value];
}

/*
 * METRIC Visual
 *
 * Dependency-free browser views for metric.evidence.v1 artifacts exported by
 * native METRIC C++ examples. The 3D scene uses raw WebGL and METRIC-owned
 * view contracts; no external renderer is required.
 */

export async function loadMetricEvidence(url) {
  const response = await fetch(url);
  if (!response.ok) {
    throw new Error(`Unable to load METRIC evidence from ${url}: ${response.status}`);
  }
  const evidence = await response.json();
  assertMetricEvidence(evidence);
  return evidence;
}

export function assertMetricEvidence(evidence) {
  if (!evidence || evidence.schema !== "metric.evidence.v1") {
    throw new Error("Unsupported METRIC evidence schema");
  }
  for (const key of ["records", "spaces", "pair_values", "queries", "winners", "coordinates", "diagnostics"]) {
    if (!Array.isArray(evidence[key])) {
      throw new Error(`METRIC evidence is missing array field: ${key}`);
    }
  }
}

export class MetricSelection {
  constructor() {
    this.state = {};
    this.listeners = new Set();
  }

  subscribe(listener) {
    this.listeners.add(listener);
    listener(this.state);
    return () => this.listeners.delete(listener);
  }

  set(next) {
    this.state = { ...this.state, ...next };
    for (const listener of this.listeners) {
      listener(this.state);
    }
  }
}

export class MetricScene3D {
  constructor(canvas, selection, options = {}) {
    this.canvas = canvas;
    this.selection = selection;
    this.options = {
      pointSize: 8,
      background: [0.955, 0.965, 0.955, 1],
      autoRotate: true,
      projection: "isometric",
      initialYaw: -Math.PI / 4,
      initialPitch: Math.atan(1 / Math.sqrt(2)),
      ground: true,
      groundY: -1.08,
      groundSize: 2.45,
      groundGridDivisions: 14,
      morph: 1,
      morphLoop: false,
      morphSpeed: 0.00032,
      miniature: true,
      miniatureFocusY: 0.58,
      miniatureFocusWidth: 0.2,
      miniatureBlurPixels: 6.5,
      miniatureSaturation: 1.18,
      miniatureVignette: 0.22,
      onMorphChange: undefined,
      ...options,
    };
    this.points = [];
    this.morph = clamp(Number(this.options.morph), 0, 1);
    this.morphLoop = Boolean(this.options.morphLoop);
    this.morphDirection = -1;
    this.yaw = this.options.initialYaw;
    this.pitch = this.options.initialPitch;
    this.distance = 2.65;
    this.dragging = false;
    this.lastMouse = [0, 0];
    this.selectedRecordId = undefined;
    this.projected = [];
    this.gl = canvas.getContext("webgl", { antialias: true, depth: true });
    if (!this.gl) {
      throw new Error("WebGL is not available");
    }
    this.program = createPointProgram(this.gl);
    this.projectionProgram = createProjectionProgram(this.gl);
    this.groundProgram = createGroundProgram(this.gl);
    this.compositeProgram = createMiniatureCompositeProgram(this.gl);
    this.quadPositionBuffer = this.gl.createBuffer();
    this.quadUvBuffer = this.gl.createBuffer();
    this.groundBuffer = this.gl.createBuffer();
    this.groundPlaneVertexCount = 0;
    this.groundGridVertexCount = 0;
    this.sceneTarget = undefined;
    this.positionBuffer = this.gl.createBuffer();
    this.colorBuffer = this.gl.createBuffer();
    this.selectedPositionBuffer = this.gl.createBuffer();
    this.selectedColorBuffer = this.gl.createBuffer();
    this.shadowPositionBuffer = this.gl.createBuffer();
    this.shadowColorBuffer = this.gl.createBuffer();
    this.bindEvents();
    this.uploadQuad();
    this.uploadGround();
    this.selection.subscribe((state) => {
      this.selectedRecordId = state.recordId;
      this.draw();
    });
    requestAnimationFrame((time) => this.frame(time));
  }

  setPoints(points) {
    this.points = normalizePoints(points);
    this.upload();
    this.draw();
  }

  setMorph(value) {
    const next = clamp(Number(value), 0, 1);
    if (Math.abs(next - this.morph) < 0.0001) return;
    this.morph = next;
    this.upload();
    this.draw();
    if (typeof this.options.onMorphChange === "function") {
      this.options.onMorphChange(this.morph);
    }
  }

  setMorphLoop(enabled) {
    this.morphLoop = Boolean(enabled);
    return this.morphLoop;
  }

  bindEvents() {
    this.canvas.addEventListener("pointerdown", (event) => {
      this.dragging = true;
      this.lastMouse = [event.clientX, event.clientY];
      this.canvas.setPointerCapture(event.pointerId);
    });
    this.canvas.addEventListener("pointermove", (event) => {
      if (!this.dragging) return;
      const dx = event.clientX - this.lastMouse[0];
      const dy = event.clientY - this.lastMouse[1];
      this.yaw += dx * 0.008;
      this.pitch = clamp(this.pitch + dy * 0.006, -1.35, 1.35);
      this.lastMouse = [event.clientX, event.clientY];
      this.draw();
    });
    this.canvas.addEventListener("pointerup", () => {
      this.dragging = false;
    });
    this.canvas.addEventListener("wheel", (event) => {
      event.preventDefault();
      this.distance = clamp(this.distance + event.deltaY * 0.003, 1.1, 8);
      this.draw();
    }, { passive: false });
    this.canvas.addEventListener("click", (event) => {
      const picked = this.pick(event);
      if (picked) {
        this.selection.set({ recordId: picked.recordId, domain: picked.domain });
      }
    });
  }

  upload() {
    const gl = this.gl;
    const positions = [];
    const colors = [];
    for (const point of this.points) {
      point.displayPosition = interpolatedPosition(point, this.morph);
      positions.push(point.displayPosition[0], point.displayPosition[1], point.displayPosition[2]);
      colors.push(point.color[0], point.color[1], point.color[2]);
    }
    gl.bindBuffer(gl.ARRAY_BUFFER, this.positionBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(positions), gl.STATIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.colorBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(colors), gl.STATIC_DRAW);
  }

  uploadQuad() {
    const gl = this.gl;
    gl.bindBuffer(gl.ARRAY_BUFFER, this.quadPositionBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([
      -1, -1,
      3, -1,
      -1, 3,
      -1, 3,
    ]), gl.STATIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.quadUvBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([
      0, 0,
      2, 0,
      0, 2,
      0, 2,
    ]), gl.STATIC_DRAW);
  }

  uploadGround() {
    const gl = this.gl;
    const y = this.options.groundY;
    const size = this.options.groundSize;
    const divisions = Math.max(2, Math.floor(this.options.groundGridDivisions));
    const planeColor = [0.78, 0.84, 0.79, 0.56];
    const gridColor = [0.28, 0.38, 0.34, 0.18];
    const axisColor = [0.18, 0.28, 0.26, 0.32];
    const vertices = [];
    const push = (position, color) => vertices.push(position[0], position[1], position[2], color[0], color[1], color[2], color[3]);
    const corners = [
      [-size, y, -size],
      [size, y, -size],
      [-size, y, size],
      [-size, y, size],
      [size, y, -size],
      [size, y, size],
    ];
    for (const corner of corners) push(corner, planeColor);
    this.groundPlaneVertexCount = corners.length;

    for (let index = -divisions; index <= divisions; index++) {
      const p = (index / divisions) * size;
      const color = index === 0 ? axisColor : gridColor;
      push([-size, y + 0.002, p], color);
      push([size, y + 0.002, p], color);
      push([p, y + 0.002, -size], color);
      push([p, y + 0.002, size], color);
    }
    this.groundGridVertexCount = (divisions * 2 + 1) * 4;
    gl.bindBuffer(gl.ARRAY_BUFFER, this.groundBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);
  }

  frame(time) {
    const delta = this.lastFrameTime ? time - this.lastFrameTime : 16;
    let needsDraw = false;
    if (this.options.autoRotate && !this.dragging) {
      this.yaw += 0.00025 * delta;
      needsDraw = true;
    }
    if (this.morphLoop && this.points.some((point) => point.position2d)) {
      this.morph += this.morphDirection * this.options.morphSpeed * delta;
      if (this.morph <= 0) {
        this.morph = 0;
        this.morphDirection = 1;
      } else if (this.morph >= 1) {
        this.morph = 1;
        this.morphDirection = -1;
      }
      this.upload();
      needsDraw = true;
      if (typeof this.options.onMorphChange === "function") {
        this.options.onMorphChange(this.morph);
      }
    }
    if (needsDraw) this.draw();
    this.lastFrameTime = time;
    requestAnimationFrame((next) => this.frame(next));
  }

  draw() {
    const gl = this.gl;
    resizeCanvas(this.canvas);
    if (this.options.miniature) {
      this.ensureSceneTarget();
      gl.bindFramebuffer(gl.FRAMEBUFFER, this.sceneTarget.framebuffer);
      this.drawScenePass();
      gl.bindFramebuffer(gl.FRAMEBUFFER, null);
      this.drawMiniatureCompositePass();
    } else {
      gl.bindFramebuffer(gl.FRAMEBUFFER, null);
      this.drawScenePass();
    }
  }

  drawScenePass() {
    const gl = this.gl;
    gl.viewport(0, 0, this.canvas.width, this.canvas.height);
    gl.clearColor(...this.options.background);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.enable(gl.DEPTH_TEST);

    const matrix = this.viewProjectionMatrix();
    if (this.options.ground) {
      this.drawGround(matrix);
      this.drawGroundProjection(matrix);
    }

    gl.useProgram(this.program.program);
    enableAttributes(gl, [this.program.attributes.position, this.program.attributes.color]);
    gl.uniformMatrix4fv(this.program.uniforms.matrix, false, matrix);
    gl.uniform1f(this.program.uniforms.pointSize, this.options.pointSize * (window.devicePixelRatio || 1));
    bindAttribute(gl, this.program.attributes.position, this.positionBuffer, 3);
    bindAttribute(gl, this.program.attributes.color, this.colorBuffer, 3);
    if (this.points.length > 0) {
      gl.drawArrays(gl.POINTS, 0, this.points.length);
    }

    this.projected = this.points.map((point) => ({
      point,
      screen: project(point.displayPosition || point.position3d, matrix, this.canvas.width, this.canvas.height),
    }));

    const selected = this.points.find((point) => point.recordId === this.selectedRecordId);
    if (selected) {
      gl.uniform1f(this.program.uniforms.pointSize, this.options.pointSize * 1.8 * (window.devicePixelRatio || 1));
      enableAttributes(gl, [this.program.attributes.position, this.program.attributes.color]);
      gl.bindBuffer(gl.ARRAY_BUFFER, this.selectedPositionBuffer);
      gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(selected.displayPosition || selected.position3d), gl.DYNAMIC_DRAW);
      bindAttribute(gl, this.program.attributes.position, this.selectedPositionBuffer, 3);
      gl.bindBuffer(gl.ARRAY_BUFFER, this.selectedColorBuffer);
      gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([0.02, 0.08, 0.15]), gl.DYNAMIC_DRAW);
      bindAttribute(gl, this.program.attributes.color, this.selectedColorBuffer, 3);
      gl.disable(gl.DEPTH_TEST);
      gl.drawArrays(gl.POINTS, 0, 1);
      gl.enable(gl.DEPTH_TEST);
    }
  }

  drawGround(matrix) {
    const gl = this.gl;
    gl.useProgram(this.groundProgram.program);
    gl.uniformMatrix4fv(this.groundProgram.uniforms.matrix, false, matrix);
    bindInterleavedGround(gl, this.groundProgram, this.groundBuffer);
    gl.enable(gl.BLEND);
    gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
    gl.drawArrays(gl.TRIANGLES, 0, this.groundPlaneVertexCount);
    gl.drawArrays(gl.LINES, this.groundPlaneVertexCount, this.groundGridVertexCount);
    gl.disable(gl.BLEND);
  }

  drawGroundProjection(matrix) {
    const gl = this.gl;
    if (!this.points.length) return;
    const data = [];
    for (const point of this.points) {
      const base = point.position2d || point.displayPosition || point.position3d;
      const height = Math.max(0, (point.displayPosition || point.position3d)[1] - this.options.groundY);
      const color = point.color || [0.2, 0.28, 0.25];
      data.push(base[0], this.options.groundY + 0.012, base[2], color[0], color[1], color[2], clamp(0.34 + height * 0.12, 0.32, 0.62));
    }
    gl.useProgram(this.projectionProgram.program);
    gl.uniformMatrix4fv(this.projectionProgram.uniforms.matrix, false, matrix);
    gl.uniform1f(this.projectionProgram.uniforms.pointSize, this.options.pointSize * 2.4 * (window.devicePixelRatio || 1));
    gl.bindBuffer(gl.ARRAY_BUFFER, this.shadowPositionBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(data), gl.DYNAMIC_DRAW);
    bindInterleavedProjection(gl, this.projectionProgram, this.shadowPositionBuffer);
    gl.enable(gl.BLEND);
    gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
    gl.drawArrays(gl.POINTS, 0, this.points.length);
    gl.disable(gl.BLEND);
  }

  drawMiniatureCompositePass() {
    const gl = this.gl;
    gl.viewport(0, 0, this.canvas.width, this.canvas.height);
    gl.disable(gl.DEPTH_TEST);
    gl.clearColor(...this.options.background);
    gl.clear(gl.COLOR_BUFFER_BIT);
    gl.useProgram(this.compositeProgram.program);
    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, this.sceneTarget.texture);
    gl.uniform1i(this.compositeProgram.uniforms.scene, 0);
    gl.uniform2f(this.compositeProgram.uniforms.resolution, this.canvas.width, this.canvas.height);
    gl.uniform1f(this.compositeProgram.uniforms.focusY, this.options.miniatureFocusY);
    gl.uniform1f(this.compositeProgram.uniforms.focusWidth, this.options.miniatureFocusWidth);
    gl.uniform1f(this.compositeProgram.uniforms.blurPixels, this.options.miniatureBlurPixels * (window.devicePixelRatio || 1));
    gl.uniform1f(this.compositeProgram.uniforms.saturation, this.options.miniatureSaturation);
    gl.uniform1f(this.compositeProgram.uniforms.vignette, this.options.miniatureVignette);
    enableAttributes(gl, [this.compositeProgram.attributes.position, this.compositeProgram.attributes.uv]);
    bindAttribute(gl, this.compositeProgram.attributes.position, this.quadPositionBuffer, 2);
    bindAttribute(gl, this.compositeProgram.attributes.uv, this.quadUvBuffer, 2);
    gl.drawArrays(gl.TRIANGLES, 0, 3);
    gl.enable(gl.DEPTH_TEST);
  }

  ensureSceneTarget() {
    const gl = this.gl;
    const width = this.canvas.width;
    const height = this.canvas.height;
    if (this.sceneTarget && this.sceneTarget.width === width && this.sceneTarget.height === height) return;
    if (this.sceneTarget) {
      gl.deleteFramebuffer(this.sceneTarget.framebuffer);
      gl.deleteTexture(this.sceneTarget.texture);
      gl.deleteRenderbuffer(this.sceneTarget.depth);
    }
    const texture = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, width, height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);

    const depth = gl.createRenderbuffer();
    gl.bindRenderbuffer(gl.RENDERBUFFER, depth);
    gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, width, height);

    const framebuffer = gl.createFramebuffer();
    gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffer);
    gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, texture, 0);
    gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.RENDERBUFFER, depth);
    if (gl.checkFramebufferStatus(gl.FRAMEBUFFER) !== gl.FRAMEBUFFER_COMPLETE) {
      throw new Error("Unable to create METRIC miniature render target");
    }
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
    this.sceneTarget = { framebuffer, texture, depth, width, height };
  }

  pick(event) {
    const rect = this.canvas.getBoundingClientRect();
    const x = (event.clientX - rect.left) * (this.canvas.width / rect.width);
    const y = (event.clientY - rect.top) * (this.canvas.height / rect.height);
    let best;
    let bestDistance = 18 * (window.devicePixelRatio || 1);
    for (const projected of this.projected) {
      if (!projected.screen.visible) continue;
      const dx = projected.screen.x - x;
      const dy = projected.screen.y - y;
      const distance = Math.hypot(dx, dy);
      if (distance < bestDistance) {
        bestDistance = distance;
        best = projected.point;
      }
    }
    return best;
  }

  viewProjectionMatrix() {
    const aspect = this.canvas.width / Math.max(1, this.canvas.height);
    const projection = this.options.projection === "isometric"
      ? orthographic(this.distance * aspect, this.distance, -100, 100)
      : perspective(Math.PI / 4, aspect, 0.05, 100);
    const eye = [
      Math.cos(this.pitch) * Math.sin(this.yaw) * this.distance,
      Math.sin(this.pitch) * this.distance,
      Math.cos(this.pitch) * Math.cos(this.yaw) * this.distance,
    ];
    const view = lookAt(eye, [0, 0, 0], [0, 1, 0]);
    return multiply(projection, view);
  }
}

export class MetricRecordGallery {
  constructor(canvas, selection) {
    this.canvas = canvas;
    this.selection = selection;
    this.records = [];
    this.selectedRecordId = undefined;
    canvas.addEventListener("click", (event) => this.click(event));
    selection.subscribe((state) => {
      this.selectedRecordId = state.recordId;
      this.draw();
    });
  }

  setRecords(records) {
    this.records = records.slice(0, 28);
    this.draw();
  }

  draw() {
    const ctx = prepare2D(this.canvas);
    const w = this.canvas.width;
    const h = this.canvas.height;
    const values = this.records.flatMap((record) => record.values);
    const range = extent(values);
    const row = Math.max(22, (h - 46) / Math.max(1, this.records.length));
    ctx.fillStyle = "#172033";
    ctx.font = "600 17px system-ui, sans-serif";
    ctx.fillText("Record gallery", 28, 26);
    this.records.forEach((record, index) => {
      const y = 42 + index * row;
      ctx.fillStyle = record.id === this.selectedRecordId ? "#fff7ed" : index % 2 ? "#f8fafc" : "#ffffff";
      ctx.fillRect(20, y - 3, w - 40, row - 2);
      ctx.fillStyle = colorForLabel(record.label);
      ctx.fillRect(24, y + 4, 5, row - 11);
      ctx.fillStyle = "#475569";
      ctx.font = "11px system-ui, sans-serif";
      ctx.fillText(record.label, 38, y + 12);
      drawCurve(ctx, record.values, { x: 126, y: y + 3, w: w - 148, h: row - 10 }, range, colorForLabel(record.label), 1.3);
    });
  }

  click(event) {
    const rect = this.canvas.getBoundingClientRect();
    const y = (event.clientY - rect.top) * (this.canvas.height / rect.height);
    const row = Math.max(22, (this.canvas.height - 46) / Math.max(1, this.records.length));
    const index = Math.floor((y - 42) / row);
    if (this.records[index]) {
      this.selection.set({ recordId: this.records[index].id, domain: this.records[index].domain });
    }
  }
}

export class MetricHeatmap {
  constructor(canvas, selection, options) {
    this.canvas = canvas;
    this.selection = selection;
    this.options = options;
    this.records = [];
    this.values = [];
    canvas.addEventListener("click", (event) => this.click(event));
    selection.subscribe((state) => {
      this.selectedPair = state.pair;
      this.draw();
    });
  }

  setData(records, values) {
    this.records = records;
    this.values = values;
    this.draw();
  }

  draw() {
    const ctx = prepare2D(this.canvas);
    const w = this.canvas.width;
    const h = this.canvas.height;
    const n = this.records.length;
    const margin = 44;
    const size = Math.min(w - margin * 1.35, h - margin * 1.45);
    const cell = n ? size / n : 0;
    const values = this.values.map((entry) => entry.value);
    const range = extent(values);
    ctx.fillStyle = "#172033";
    ctx.font = "600 16px system-ui, sans-serif";
    ctx.fillText(this.options.title, margin, 26);
    for (const entry of this.values) {
      ctx.fillStyle = heatColor(entry.value, range);
      ctx.fillRect(margin + entry.column * cell, margin + entry.row * cell, Math.ceil(cell), Math.ceil(cell));
    }
    ctx.strokeStyle = "#cbd5e1";
    ctx.strokeRect(margin, margin, size, size);
    if (this.selectedPair && this.selectedPair.space === this.options.space) {
      ctx.strokeStyle = "#0f172a";
      ctx.lineWidth = 2;
      ctx.strokeRect(margin + this.selectedPair.column * cell, margin + this.selectedPair.row * cell, cell, cell);
    }
  }

  click(event) {
    const rect = this.canvas.getBoundingClientRect();
    const x = (event.clientX - rect.left) * (this.canvas.width / rect.width);
    const y = (event.clientY - rect.top) * (this.canvas.height / rect.height);
    const margin = 44;
    const n = this.records.length;
    const size = Math.min(this.canvas.width - margin * 1.35, this.canvas.height - margin * 1.45);
    const cell = n ? size / n : 0;
    const column = Math.floor((x - margin) / cell);
    const row = Math.floor((y - margin) / cell);
    if (row >= 0 && row < n && column >= 0 && column < n) {
      this.selection.set({
        pair: { domain: this.records[row].domain, space: this.options.space, row, column },
        recordId: this.records[row].id,
      });
    }
  }
}

export class MetricQueryInspector {
  constructor(canvas, selection) {
    this.canvas = canvas;
    this.selection = selection;
    this.evidence = undefined;
    this.domain = "";
    canvas.addEventListener("click", () => this.nextQuery());
    selection.subscribe((state) => {
      if (state.query) {
        this.domain = state.query.domain;
        this.queryId = state.query.id;
      }
      this.draw();
    });
  }

  setEvidence(evidence, domain) {
    this.evidence = evidence;
    this.domain = domain;
    const first = evidence.queries.find((query) => query.domain === domain);
    if (first) this.selection.set({ query: { domain, id: first.id } });
    this.draw();
  }

  draw() {
    const ctx = prepare2D(this.canvas);
    if (!this.evidence) return;
    const query = this.evidence.queries.find((entry) => entry.domain === this.domain && entry.id === this.queryId);
    if (!query) return;
    const metricWinner = winnerFor(this.evidence, this.domain, query.id, "metric_space");
    const baselineWinner = winnerFor(this.evidence, this.domain, query.id, "padded_vector_baseline");
    const metricRecord = metricWinner && recordById(this.evidence, this.domain, metricWinner.winner_id);
    const baselineRecord = baselineWinner && recordById(this.evidence, this.domain, baselineWinner.winner_id);
    const allValues = [query.values, metricRecord?.values || [], baselineRecord?.values || []].flat();
    const range = extent(allValues);
    const chart = { x: 34, y: 76, w: this.canvas.width - 68, h: this.canvas.height - 142 };
    ctx.fillStyle = "#172033";
    ctx.font = "600 17px system-ui, sans-serif";
    ctx.fillText("Query inspector", 28, 26);
    ctx.fillStyle = "#475569";
    ctx.font = "12px system-ui, sans-serif";
    ctx.fillText(query.id, 28, 46);
    ctx.fillText(`expected: ${query.expected_label}`, 28, 62);
    if (metricRecord) drawCurve(ctx, metricRecord.values, chart, range, "#16a34a", 2);
    if (baselineRecord) drawCurve(ctx, baselineRecord.values, chart, range, "#dc2626", 2);
    drawCurve(ctx, query.values, chart, range, "#0f172a", 2, [6, 5]);
    const bottom = this.canvas.height - 44;
    ctx.fillStyle = "#166534";
    ctx.fillText(`metric winner: ${metricWinner?.winner_label || "n/a"} at ${format(metricWinner?.value)}`, 28, bottom);
    ctx.fillStyle = "#991b1b";
    ctx.fillText(`baseline winner: ${baselineWinner?.winner_label || "n/a"} at ${format(baselineWinner?.value)}`, 28, bottom + 18);
  }

  nextQuery() {
    if (!this.evidence) return;
    const queries = this.evidence.queries.filter((query) => query.domain === this.domain);
    const index = queries.findIndex((query) => query.id === this.queryId);
    const next = queries[(index + 1) % queries.length];
    if (next) this.selection.set({ query: { domain: this.domain, id: next.id } });
  }
}

export class MetricProcessCurveApp {
  constructor(elements) {
    this.elements = elements;
    this.selection = new MetricSelection();
    this.scene = new MetricScene3D(elements.scene, this.selection, {
      morphLoop: true,
      onMorphChange: (value) => {
        if (elements.morphSlider) elements.morphSlider.value = value.toFixed(3);
      },
    });
    this.gallery = new MetricRecordGallery(elements.gallery, this.selection);
    this.query = new MetricQueryInspector(elements.query, this.selection);
    this.metricHeatmap = new MetricHeatmap(elements.metricHeatmap, this.selection, {
      title: "finite metric-space pair values",
      space: "metric_space",
    });
    this.baselineHeatmap = new MetricHeatmap(elements.baselineHeatmap, this.selection, {
      title: "padded-vector baseline pair values",
      space: "padded_vector_baseline",
    });
    elements.domainSelect.addEventListener("change", () => this.setDomain(elements.domainSelect.value));
    if (elements.morphSlider) {
      elements.morphSlider.addEventListener("input", () => {
        this.scene.setMorphLoop(false);
        this.updateMorphToggle();
        this.scene.setMorph(elements.morphSlider.value);
      });
    }
    if (elements.morphToggle) {
      elements.morphToggle.addEventListener("click", () => {
        this.scene.setMorphLoop(!this.scene.morphLoop);
        this.updateMorphToggle();
      });
      this.updateMorphToggle();
    }
  }

  load(evidence) {
    this.evidence = evidence;
    const domains = [...new Set(evidence.records.map((record) => record.domain))];
    this.elements.domainSelect.replaceChildren(...domains.map((domain) => {
      const option = document.createElement("option");
      option.value = domain;
      option.textContent = domain.replaceAll("_", " ");
      return option;
    }));
    this.setDomain(domains[0]);
  }

  setDomain(domain) {
    this.domain = domain;
    const records = this.evidence.records.filter((record) => record.domain === domain);
    const metricPairs = this.evidence.pair_values.filter((entry) => entry.domain === domain && entry.space === "metric_space");
    const baselinePairs = this.evidence.pair_values.filter((entry) => entry.domain === domain && entry.space === "padded_vector_baseline");
    const morphTargets = new Map(this.evidence.coordinates
      .filter((entry) => entry.domain === domain && entry.space === "metric_space_landmark2")
      .map((entry) => [entry.record_id, entry.values]));
    const coordinates = this.evidence.coordinates
      .filter((entry) => entry.domain === domain && entry.space === "metric_space_landmark3")
      .map((entry) => ({
        domain,
        recordId: entry.record_id,
        label: entry.label,
        position: entry.values,
        morphTarget: morphTargets.get(entry.record_id),
        color: rgbForLabel(entry.label),
      }));
    this.scene.setPoints(coordinates);
    this.gallery.setRecords(records);
    this.metricHeatmap.setData(records, metricPairs);
    this.baselineHeatmap.setData(records, baselinePairs);
    this.query.setEvidence(this.evidence, domain);
  }

  updateMorphToggle() {
    if (this.elements.morphToggle) {
      this.elements.morphToggle.textContent = this.scene.morphLoop ? "Pause morph" : "Animate morph";
    }
  }
}

function createPointProgram(gl) {
  const vertex = `
    attribute vec3 aPosition;
    attribute vec3 aColor;
    uniform mat4 uMatrix;
    uniform float uPointSize;
    varying vec3 vColor;
    void main() {
      gl_Position = uMatrix * vec4(aPosition, 1.0);
      gl_PointSize = uPointSize;
      vColor = aColor;
    }
  `;
  const fragment = `
    precision mediump float;
    varying vec3 vColor;
    void main() {
      vec2 p = gl_PointCoord - vec2(0.5);
      if (dot(p, p) > 0.25) discard;
      gl_FragColor = vec4(vColor, 1.0);
    }
  `;
  const program = gl.createProgram();
  gl.attachShader(program, compileShader(gl, gl.VERTEX_SHADER, vertex));
  gl.attachShader(program, compileShader(gl, gl.FRAGMENT_SHADER, fragment));
  gl.linkProgram(program);
  if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
    throw new Error(gl.getProgramInfoLog(program));
  }
  return {
    program,
    attributes: {
      position: gl.getAttribLocation(program, "aPosition"),
      color: gl.getAttribLocation(program, "aColor"),
    },
    uniforms: {
      matrix: gl.getUniformLocation(program, "uMatrix"),
      pointSize: gl.getUniformLocation(program, "uPointSize"),
    },
  };
}

function createGroundProgram(gl) {
  const vertex = `
    attribute vec3 aPosition;
    attribute vec4 aColor;
    uniform mat4 uMatrix;
    varying vec4 vColor;
    void main() {
      gl_Position = uMatrix * vec4(aPosition, 1.0);
      vColor = aColor;
    }
  `;
  const fragment = `
    precision mediump float;
    varying vec4 vColor;
    void main() {
      gl_FragColor = vColor;
    }
  `;
  const program = gl.createProgram();
  gl.attachShader(program, compileShader(gl, gl.VERTEX_SHADER, vertex));
  gl.attachShader(program, compileShader(gl, gl.FRAGMENT_SHADER, fragment));
  gl.linkProgram(program);
  if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
    throw new Error(gl.getProgramInfoLog(program));
  }
  return {
    program,
    attributes: {
      position: gl.getAttribLocation(program, "aPosition"),
      color: gl.getAttribLocation(program, "aColor"),
    },
    uniforms: {
      matrix: gl.getUniformLocation(program, "uMatrix"),
    },
  };
}

function createProjectionProgram(gl) {
  const vertex = `
    attribute vec3 aPosition;
    attribute vec4 aColor;
    uniform mat4 uMatrix;
    uniform float uPointSize;
    varying vec4 vColor;
    void main() {
      gl_Position = uMatrix * vec4(aPosition, 1.0);
      gl_PointSize = uPointSize;
      vColor = aColor;
    }
  `;
  const fragment = `
    precision mediump float;
    varying vec4 vColor;
    void main() {
      vec2 p = gl_PointCoord - vec2(0.5);
      float r = dot(p, p) * 4.0;
      if (r > 1.0) discard;
      float alpha = smoothstep(1.0, 0.0, r) * vColor.a;
      vec3 color = mix(vColor.rgb * 0.58, vColor.rgb, smoothstep(0.85, 0.0, r));
      gl_FragColor = vec4(color, alpha);
    }
  `;
  const program = gl.createProgram();
  gl.attachShader(program, compileShader(gl, gl.VERTEX_SHADER, vertex));
  gl.attachShader(program, compileShader(gl, gl.FRAGMENT_SHADER, fragment));
  gl.linkProgram(program);
  if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
    throw new Error(gl.getProgramInfoLog(program));
  }
  return {
    program,
    attributes: {
      position: gl.getAttribLocation(program, "aPosition"),
      color: gl.getAttribLocation(program, "aColor"),
    },
    uniforms: {
      matrix: gl.getUniformLocation(program, "uMatrix"),
      pointSize: gl.getUniformLocation(program, "uPointSize"),
    },
  };
}

function createMiniatureCompositeProgram(gl) {
  const vertex = `
    attribute vec2 aPosition;
    attribute vec2 aUv;
    varying vec2 vUv;
    void main() {
      vUv = aUv;
      gl_Position = vec4(aPosition, 0.0, 1.0);
    }
  `;
  const fragment = `
    precision mediump float;
    uniform sampler2D uScene;
    uniform vec2 uResolution;
    uniform float uFocusY;
    uniform float uFocusWidth;
    uniform float uBlurPixels;
    uniform float uSaturation;
    uniform float uVignette;
    varying vec2 vUv;

    vec3 saturateColor(vec3 color, float amount) {
      float luma = dot(color, vec3(0.299, 0.587, 0.114));
      return mix(vec3(luma), color, amount);
    }

    vec3 sampleBlur(vec2 uv, float radius) {
      vec2 texel = vec2(radius) / uResolution;
      vec3 color = texture2D(uScene, uv).rgb * 0.2;
      color += texture2D(uScene, uv + vec2(texel.x, 0.0)).rgb * 0.12;
      color += texture2D(uScene, uv - vec2(texel.x, 0.0)).rgb * 0.12;
      color += texture2D(uScene, uv + vec2(0.0, texel.y)).rgb * 0.12;
      color += texture2D(uScene, uv - vec2(0.0, texel.y)).rgb * 0.12;
      color += texture2D(uScene, uv + vec2(texel.x, texel.y)).rgb * 0.08;
      color += texture2D(uScene, uv + vec2(-texel.x, texel.y)).rgb * 0.08;
      color += texture2D(uScene, uv + vec2(texel.x, -texel.y)).rgb * 0.08;
      color += texture2D(uScene, uv + vec2(-texel.x, -texel.y)).rgb * 0.08;
      return color;
    }

    void main() {
      vec3 sharp = texture2D(uScene, vUv).rgb;
      float focusDistance = abs(vUv.y - uFocusY);
      float blurMask = smoothstep(uFocusWidth * 0.5, 0.48, focusDistance);
      vec3 blurred = sampleBlur(vUv, uBlurPixels * blurMask);
      vec3 color = mix(sharp, blurred, blurMask);
      color = saturateColor(color, uSaturation);
      color = mix(color, vec3(0.985, 0.98, 0.93), 0.08);
      float radial = distance(vUv, vec2(0.5, 0.54));
      color *= 1.0 - smoothstep(0.42, 0.82, radial) * uVignette;
      gl_FragColor = vec4(color, 1.0);
    }
  `;
  const program = gl.createProgram();
  gl.attachShader(program, compileShader(gl, gl.VERTEX_SHADER, vertex));
  gl.attachShader(program, compileShader(gl, gl.FRAGMENT_SHADER, fragment));
  gl.linkProgram(program);
  if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
    throw new Error(gl.getProgramInfoLog(program));
  }
  return {
    program,
    attributes: {
      position: gl.getAttribLocation(program, "aPosition"),
      uv: gl.getAttribLocation(program, "aUv"),
    },
    uniforms: {
      scene: gl.getUniformLocation(program, "uScene"),
      resolution: gl.getUniformLocation(program, "uResolution"),
      focusY: gl.getUniformLocation(program, "uFocusY"),
      focusWidth: gl.getUniformLocation(program, "uFocusWidth"),
      blurPixels: gl.getUniformLocation(program, "uBlurPixels"),
      saturation: gl.getUniformLocation(program, "uSaturation"),
      vignette: gl.getUniformLocation(program, "uVignette"),
    },
  };
}

function compileShader(gl, type, source) {
  const shader = gl.createShader(type);
  gl.shaderSource(shader, source);
  gl.compileShader(shader);
  if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
    throw new Error(gl.getShaderInfoLog(shader));
  }
  return shader;
}

function bindAttribute(gl, location, buffer, size) {
  if (location < 0) return;
  gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
  gl.enableVertexAttribArray(location);
  gl.vertexAttribPointer(location, size, gl.FLOAT, false, 0, 0);
}

function enableAttributes(gl, locations) {
  const active = new Set(locations.filter((location) => location >= 0));
  const count = gl.getParameter(gl.MAX_VERTEX_ATTRIBS);
  for (let index = 0; index < count; index++) {
    if (active.has(index)) {
      gl.enableVertexAttribArray(index);
    } else {
      gl.disableVertexAttribArray(index);
    }
  }
}

function bindInterleavedGround(gl, program, buffer) {
  const stride = 7 * 4;
  gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
  enableAttributes(gl, [program.attributes.position, program.attributes.color]);
  if (program.attributes.position >= 0) {
    gl.vertexAttribPointer(program.attributes.position, 3, gl.FLOAT, false, stride, 0);
  }
  if (program.attributes.color >= 0) {
    gl.vertexAttribPointer(program.attributes.color, 4, gl.FLOAT, false, stride, 3 * 4);
  }
}

function bindInterleavedProjection(gl, program, buffer) {
  const stride = 7 * 4;
  gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
  enableAttributes(gl, [program.attributes.position, program.attributes.color]);
  if (program.attributes.position >= 0) {
    gl.vertexAttribPointer(program.attributes.position, 3, gl.FLOAT, false, stride, 0);
  }
  if (program.attributes.color >= 0) {
    gl.vertexAttribPointer(program.attributes.color, 4, gl.FLOAT, false, stride, 3 * 4);
  }
}

function bindInterleavedQuad(gl, program, buffer) {
  const stride = 4 * 4;
  gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
  enableAttributes(gl, [program.attributes.position, program.attributes.uv]);
  if (program.attributes.position >= 0) {
    gl.vertexAttribPointer(program.attributes.position, 2, gl.FLOAT, false, stride, 0);
  }
  if (program.attributes.uv >= 0) {
    gl.vertexAttribPointer(program.attributes.uv, 2, gl.FLOAT, false, stride, 2 * 4);
  }
}

function normalizePoints(points) {
  if (!points.length) return [];
  const vectors = points.flatMap((point) => {
    const source = vector3(point.position);
    const target = point.morphTarget ? vector3(point.morphTarget) : undefined;
    return target ? [source, target] : [source];
  });
  const axes = [0, 1, 2].map((axis) => extent(vectors.map((vector) => vector[axis] || 0)));
  return points.map((point) => ({
    ...point,
    position3d: [0, 1, 2].map((axis) => normalize(vector3(point.position)[axis] || 0, axes[axis])),
    position2d: point.morphTarget
      ? [0, 1, 2].map((axis) => normalize(vector3(point.morphTarget)[axis] || 0, axes[axis]))
      : undefined,
    color: point.color || rgbForLabel(point.label),
  }));
}

function interpolatedPosition(point, morph) {
  const source = point.position3d;
  const target = point.position2d || point.position3d;
  return [
    target[0] + (source[0] - target[0]) * morph,
    target[1] + (source[1] - target[1]) * morph,
    target[2] + (source[2] - target[2]) * morph,
  ];
}

function vector3(values) {
  return [Number(values?.[0]) || 0, Number(values?.[1]) || 0, Number(values?.[2]) || 0];
}

function resizeCanvas(canvas) {
  const ratio = window.devicePixelRatio || 1;
  const width = Math.max(1, Math.floor(canvas.clientWidth * ratio));
  const height = Math.max(1, Math.floor(canvas.clientHeight * ratio));
  if (canvas.width !== width || canvas.height !== height) {
    canvas.width = width;
    canvas.height = height;
  }
}

function prepare2D(canvas) {
  resizeCanvas(canvas);
  const ctx = canvas.getContext("2d");
  ctx.setTransform(1, 0, 0, 1, 0, 0);
  ctx.fillStyle = "#f8fafc";
  ctx.fillRect(0, 0, canvas.width, canvas.height);
  return ctx;
}

function drawCurve(ctx, values, box, range, stroke, width, dash = []) {
  ctx.save();
  ctx.strokeStyle = stroke;
  ctx.lineWidth = width;
  ctx.setLineDash(dash);
  ctx.beginPath();
  values.forEach((value, index) => {
    const x = box.x + box.w * (index / Math.max(1, values.length - 1));
    const y = box.y + box.h - ((value - range.min) / (range.max - range.min)) * box.h;
    if (index === 0) ctx.moveTo(x, y);
    else ctx.lineTo(x, y);
  });
  ctx.stroke();
  ctx.restore();
}

function extent(values) {
  let min = Infinity;
  let max = -Infinity;
  for (const value of values) {
    if (Number.isFinite(value)) {
      min = Math.min(min, value);
      max = Math.max(max, value);
    }
  }
  if (!Number.isFinite(min) || !Number.isFinite(max)) return { min: 0, max: 1 };
  if (min === max) return { min: min - 1, max: max + 1 };
  return { min, max };
}

function normalize(value, range) {
  return ((value - range.min) / (range.max - range.min) - 0.5) * 1.65;
}

function colorForLabel(label) {
  switch (label) {
    case "normal": return "#2563eb";
    case "normal_mid": return "#3b82f6";
    case "pre_anomaly": return "#16a34a";
    case "anomaly":
    case "anomaly_start": return "#dc2626";
    case "anomaly_mid": return "#ea580c";
    case "recovery": return "#7c3aed";
    default: return "#64748b";
  }
}

function rgbForLabel(label) {
  const hex = colorForLabel(label).slice(1);
  return [0, 2, 4].map((index) => parseInt(hex.slice(index, index + 2), 16) / 255);
}

function heatColor(value, range) {
  const t = Math.max(0, Math.min(1, (value - range.min) / (range.max - range.min)));
  const shade = Math.round(236 - 186 * t);
  return `rgb(${shade}, ${Math.round(244 - 145 * t)}, ${Math.round(248 - 108 * t)})`;
}

function winnerFor(evidence, domain, queryId, space) {
  return evidence.winners.find((winner) => winner.domain === domain && winner.query_id === queryId && winner.space === space);
}

function recordById(evidence, domain, id) {
  return evidence.records.find((record) => record.domain === domain && record.id === id);
}

function format(value) {
  return Number.isFinite(value) ? Number(value).toFixed(3) : "n/a";
}

function perspective(fovy, aspect, near, far) {
  const f = 1 / Math.tan(fovy / 2);
  const nf = 1 / (near - far);
  return new Float32Array([
    f / aspect, 0, 0, 0,
    0, f, 0, 0,
    0, 0, (far + near) * nf, -1,
    0, 0, 2 * far * near * nf, 0,
  ]);
}

function orthographic(halfWidth, halfHeight, near, far) {
  const lr = 1 / (halfWidth * 2);
  const bt = 1 / (halfHeight * 2);
  const nf = 1 / (near - far);
  return new Float32Array([
    2 * lr, 0, 0, 0,
    0, 2 * bt, 0, 0,
    0, 0, 2 * nf, 0,
    0, 0, (far + near) * nf, 1,
  ]);
}

function lookAt(eye, center, up) {
  const z = normalize3(subtract(eye, center));
  const x = normalize3(cross(up, z));
  const y = cross(z, x);
  return new Float32Array([
    x[0], y[0], z[0], 0,
    x[1], y[1], z[1], 0,
    x[2], y[2], z[2], 0,
    -dot(x, eye), -dot(y, eye), -dot(z, eye), 1,
  ]);
}

function multiply(a, b) {
  const out = new Float32Array(16);
  for (let row = 0; row < 4; row++) {
    for (let col = 0; col < 4; col++) {
      out[col * 4 + row] =
        a[0 * 4 + row] * b[col * 4 + 0] +
        a[1 * 4 + row] * b[col * 4 + 1] +
        a[2 * 4 + row] * b[col * 4 + 2] +
        a[3 * 4 + row] * b[col * 4 + 3];
    }
  }
  return out;
}

function project(position, matrix, width, height) {
  const x = position[0], y = position[1], z = position[2];
  const clip = [
    matrix[0] * x + matrix[4] * y + matrix[8] * z + matrix[12],
    matrix[1] * x + matrix[5] * y + matrix[9] * z + matrix[13],
    matrix[2] * x + matrix[6] * y + matrix[10] * z + matrix[14],
    matrix[3] * x + matrix[7] * y + matrix[11] * z + matrix[15],
  ];
  if (clip[3] <= 0) return { x: 0, y: 0, visible: false };
  const nx = clip[0] / clip[3];
  const ny = clip[1] / clip[3];
  return { x: (nx * 0.5 + 0.5) * width, y: (-ny * 0.5 + 0.5) * height, visible: true };
}

function subtract(a, b) {
  return [a[0] - b[0], a[1] - b[1], a[2] - b[2]];
}

function cross(a, b) {
  return [a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]];
}

function dot(a, b) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

function normalize3(v) {
  const length = Math.hypot(v[0], v[1], v[2]) || 1;
  return [v[0] / length, v[1] / length, v[2] / length];
}

function clamp(value, min, max) {
  return Math.max(min, Math.min(max, value));
}
