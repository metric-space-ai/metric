import { loadVisualDocument, normalizeVisualInput, VisualSpace } from "./data/index.js";
import { createLayerFromDescriptor } from "./layers/index.js";
import { MetricVisualRuntime } from "./runtime/index.js";
import { createMiniatureHeroRuntimeOptions } from "./style/miniature/index.js";
import { RecordPreviewPanel, TimelineControlWidget, findTimelineControlDescriptor } from "./interaction/index.js";
import { createRelationMatrixPicker } from "./relational/index.js";
import {
  CrossSpaceView,
  DynamicsView,
  MappingView,
  mappingMotionProgressAt,
  normalizeMappingMotionTiming,
  MetricSpaceView,
  MixedRecordView,
  NeighborhoodGraphView,
  PropertyFieldView,
  ProcessCurveSceneView,
  RelationMatrixView,
  SolverTraceView,
  SpacePropertiesView,
} from "./views/index.js";

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

export async function showConditionMonitoring(evidence, options = {}) {
  const visual = await createMetricVisual({ ...options, evidence });
  return visual.showConditionMonitoring(options);
}

export async function showProcessCurves(evidence, options = {}) {
  const visual = await createMetricVisual({ ...options, evidence });
  return visual.showProcessCurves(options);
}

export async function showMixedRecords(evidence, options = {}) {
  const visual = await createMetricVisual({ ...options, evidence });
  return visual.showMixedRecords(options);
}

export async function showCrossSpace(evidence, options = {}) {
  const visual = await createMetricVisual({ ...options, evidence });
  return visual.showCrossSpace(options);
}

export async function showRelationMatrixNeighborhood(evidence, options = {}) {
  const visual = await createMetricVisual({ ...options, evidence });
  return visual.showRelationMatrixNeighborhood(options);
}

export async function showSolverTrace(evidence, options = {}) {
  const visual = await createMetricVisual({ ...options, evidence });
  return visual.showSolverTrace(options);
}

export async function showPreview(evidence, options = {}) {
  const visual = await createMetricVisual({ ...options, evidence });
  return visual.showPreview(options);
}

export function captureHeroFrame(visual, options = {}) {
  if (!visual || typeof visual.captureHeroFrame !== "function") {
    throw new TypeError("captureHeroFrame expects a MetricVisualSurface-compatible object.");
  }
  return visual.captureHeroFrame(options);
}

export function createMetricVisualDocumentDiagnostics(document, options = {}) {
  const context = describeMetricVisualContext(options);
  const warnings = createMetricVisualDocumentWarnings(document, context);
  const evidence = classifyMetricVisualEvidence(document);
  return {
    schema: METRIC_VISUAL_DOCUMENT_DIAGNOSTICS_SCHEMA,
    evidenceSchema: document?.schema || null,
    loadedSchema: document?.schema || null,
    recordCount: countVisualRecords(document),
    datasetCount: countVisualCollection(document, "datasets"),
    spaceCount: countVisualCollection(document, "spaces"),
    coordinateCount: countVisualCollection(document, "coordinates"),
    relationCount: countVisualCollection(document, "relations"),
    propertyCount: countVisualCollection(document, "properties"),
    graphCount: countVisualCollection(document, "graphs"),
    timelineCount: countVisualCollection(document, "timelines"),
    declaredViewCount: countVisualCollection(document, "views"),
    evidenceKind: evidence.kind,
    evidenceNative: evidence.native,
    evidenceDocumentedReference: evidence.documentedReference,
    evidenceSyntheticFixture: evidence.syntheticFixture,
    evidenceKindSignals: evidence.signals.slice(),
    evidenceProvenance: { ...evidence.provenance },
    evidenceReport: createMetricVisualEvidenceReport(document, evidence),
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
    this.timelineControlWidget = null;
    this.lastDynamicsOptions = null;
    this.applyingTimelineControl = false;
    this.previewSubscriptions = [];
    this.previewPairPickers = [];
    this.mappingMotionSubscription = null;
    this.mappingMotionState = null;
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
    if (normalized === "process-curves" || normalized === "process-curve" || normalized === "external-process-curves") return this.showProcessCurves(options);
    if (normalized === "mixed-records") return this.showMixedRecords(options);
    if (normalized === "cross-space") return this.showCrossSpace(options);
    if (normalized === "relation-matrix-neighborhood") return this.showRelationMatrixNeighborhood(options);
    if (normalized === "solver-trace" || normalized === "solver") return this.showSolverTrace(options);
    if (normalized === "preview" || normalized === "record-preview" || normalized === "pair-preview") return this.showPreview(options);
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
    if (options.viewKind && options.viewKind !== "dynamics") {
      this.disableTimelineControl();
    }
    if (options.viewKind && options.viewKind !== "mapping") {
      this.disableMappingMotion();
    }
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
    const normalized = normalizeMetricViewOptions(this.document, options);
    const view = MetricSpaceView.fromVisualSpace(this.document, normalized);
    this.views = [view];
    const descriptors = view.toLayerDescriptors();
    if (normalized.groundField || normalized.propertyField) {
      const field = PropertyFieldView.fromMetricSpaceView(this.document, view, normalizePropertyFieldViewOptions(normalized));
      this.views.push(field);
      descriptors.splice(1, 0, ...field.toLayerDescriptors());
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
    const view = SpacePropertiesView.fromVisualSpace(this.document, { ...normalized, field: false, propertyField: false });
    const field = PropertyFieldView.fromMetricSpaceView(this.document, view.space, normalized);
    this.views = [view, field];
    const descriptors = view.toLayerDescriptors();
    descriptors.splice(1, 0, ...field.toLayerDescriptors());
    this.setLayerDescriptors(descriptors, { source: "showSpaceProperties", viewKind: "space-properties" });
    return this.configurePreview(options);
  }

  showMapping(options = {}) {
    const view = MappingView.fromVisualSpace(this.document, normalizeMappingOptions(this.document, options));
    this.views = [view];
    const descriptors = view.toLayerDescriptors();
    this.setLayerDescriptors(descriptors, { source: "showMapping", viewKind: "mapping" });
    this.configureMappingMotion(view, options);
    return this.configurePreview(options);
  }

  showDynamics(options = {}) {
    const normalized = normalizeMetricViewOptions(this.document, options);
    const fieldEnabled = normalized.propertyField || normalized.groundField;
    const view = DynamicsView.fromVisualSpace(this.document, fieldEnabled
      ? { ...normalized, field: false, propertyField: false, groundField: false }
      : normalized);
    this.lastDynamicsOptions = { ...normalized };
    this.views = [view];
    const descriptors = view.toLayerDescriptors();
    if (fieldEnabled) {
      const active = view.fittedStates?.[view.activeStep];
      const field = PropertyFieldView.fromMetricSpaceView(this.document, {
        recordIds: view.recordIds,
        positions: active?.positions,
        datasetId: view.datasetId,
        spaceId: view.spaceId,
        coordinateId: active?.coordinateId,
        groundY: view.groundY,
      }, normalizePropertyFieldViewOptions({
        ...normalized,
        coordinateId: active?.coordinateId,
      }));
      descriptors.unshift(...field.toLayerDescriptors());
      this.views.push(field);
    }
    this.setLayerDescriptors(descriptors, { source: "showDynamics", viewKind: "dynamics" });
    this.configureTimelineControl(view, normalized);
    return this.configurePreview(options);
  }

  showConditionMonitoring(options = {}) {
    const view = ProcessCurveSceneView.fromVisualSpace(this.document, {
      ...options,
      targetCoordinateId: options.targetCoordinateId || options.coordinateId || options.coordinate || "condition-3d",
      sourceCoordinateId: options.sourceCoordinateId || options.sourceCoordinate,
      labelPropertyId: options.labelPropertyId || options.labelProperty || options.labels || "cluster",
      relationId: options.relationId || options.pathRelationId || options.transitionRelationId,
      graphId: options.graphId || options.pathGraphId,
      groundField: options.groundField || options.propertyField || options.scalarProperty || options.colorBy || "anomaly",
      includeNeighborhood: options.includeNeighborhood ?? false,
      includeMatrix: options.includeMatrix ?? false,
      useGraphTrajectory: options.useGraphTrajectory ?? true,
      trackMode: options.trackMode || "ribbon",
      trackWidth: options.trackWidth ?? options.pathWidth ?? 3.4,
      trackAlpha: options.trackAlpha ?? options.pathAlpha ?? 0.68,
      trackColor: options.trackColor || options.pathColor || [0.12, 0.36, 0.42, 0.68],
      pointSize: options.pointSize ?? 1.42,
      pointAlpha: options.pointAlpha ?? 0.78,
      fieldAlpha: options.fieldAlpha ?? 0.2,
      fieldRadius: options.fieldRadius ?? 0.16,
      labelFontSize: options.labelFontSize ?? 28,
      labelLift: options.labelLift ?? 0.46,
      labelOffsetRadius: options.labelOffsetRadius ?? 0.16,
      labelMap: {
        healthy: "normal regime",
        normal: "normal regime",
        drift: "drift regime",
        fault: "fault regime",
        recovery: "recovery",
        ...(options.labelMap || {}),
      },
      metadata: {
        ...(options.metadata || {}),
        visualGrammar: "condition-monitoring",
        command: "showConditionMonitoring",
      },
    });
    this.views = [view];
    const descriptors = view.toLayerDescriptors();
    this.setLayerDescriptors(descriptors, { source: "showConditionMonitoring", viewKind: "condition-monitoring" });
    if (options.camera !== false) {
      this.setCamera(options.camera || conditionMonitoringCamera());
    }
    this.setMotion(options.motion ?? conditionMonitoringMotion(), { start: options.startMotion !== false });
    return this.configurePreview({ ...options, preview: options.preview ?? "time-series" });
  }

  showProcessCurves(options = {}) {
    const normalized = {
      targetCoordinateId: options.targetCoordinateId || options.coordinateId || options.coordinate,
      sourceCoordinateId: options.sourceCoordinateId || options.sourceCoordinate,
      labelPropertyId: options.labelPropertyId || options.labelProperty || options.colorProperty || options.labels || "process-role",
      pointSize: options.pointSize ?? 1.55,
      pointAlpha: options.pointAlpha ?? 0.76,
      trackMode: options.trackMode || "tube",
      trackAlpha: options.trackAlpha ?? 0.68,
      morphProgress: options.morphProgress ?? 1,
      ...options,
    };
    const view = ProcessCurveSceneView.fromVisualSpace(this.document, normalized);
    this.views = [view];
    const descriptors = view.toLayerDescriptors();
    this.setLayerDescriptors(descriptors, { source: "showProcessCurves", viewKind: "process-curves" });
    if (options.camera !== false) {
      this.setCamera(options.camera || processCurveCamera());
    }
    this.setMotion(options.motion ?? "studio-drift", { start: options.startMotion !== false });
    return this.configurePreview({ ...options, preview: options.preview ?? "record" });
  }

  showMixedRecords(options = {}) {
    const normalized = normalizeMetricViewOptions(this.document, {
      colorProperty: options.colorProperty || options.glyphBy || options.recordType || "family",
      labelProperty: options.labelProperty || options.labels || "family",
      pointSize: options.pointSize ?? 1.15,
      relationId: options.relationId || options.relation,
      crossTypeRelations: options.crossTypeRelations ?? true,
      topK: options.topK ?? 4,
      ...options,
    });
    const view = MixedRecordView.fromVisualSpace(this.document, normalized);
    this.views = [view];
    this.setViews([view], { source: "showMixedRecords", viewKind: "mixed-records" });
    return this.configurePreview({ ...options, preview: options.preview ?? "record" });
  }

  showCrossSpace(options = {}) {
    const view = CrossSpaceView.fromVisualSpace(this.document, {
      id: options.id || "cross-space",
      leftCoordinateId: options.leftCoordinateId || options.coordinateA || "space-a-3d",
      rightCoordinateId: options.rightCoordinateId || options.coordinateB || "space-b-3d",
      scalarProperty: options.scalarProperty || options.dependenceProperty || "local-dependence",
      groundY: this.setup.stage?.grounding?.groundY ?? -0.58,
      targetRadius: options.targetRadius ?? 1.05,
      ...options,
    });
    this.views = [view];
    const descriptors = view.toLayerDescriptors();
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

  showSolverTrace(options = {}) {
    const view = SolverTraceView.fromVisualSpace(this.document, options);
    this.views = [view];
    this.setViews([view], { source: "showSolverTrace", viewKind: "solver-trace" });
    return this.configurePreview(options);
  }

  showPreview(options = {}) {
    const mode = options.mode || options.preview || "record";
    if (!this.descriptors.length) {
      const pairMode = String(mode).toLowerCase().includes("pair") || options.relationId || options.relation;
      if (pairMode && this.document?.relations?.length) {
        this.showRelationMatrix({ ...options, preview: false });
      } else {
        this.showMetricSpace({ ...options, preview: false });
      }
    }
    this.enableRecordPreview({ mode, ...(options.previewOptions || {}) });
    this.recordCommandDiagnostics({
      command: "showPreview",
      viewKind: options.viewKind || `${String(mode).replace(/\s+/g, "-")}-preview`,
    });
    return this;
  }

  recordCommandDiagnostics(options = {}) {
    const runtimeState = safeRuntimeState(this.runtime);
    const evidence = classifyMetricVisualEvidence(this.document);
    const command = options.command || "metric-visual-command";
    const entry = {
      schema: METRIC_VISUAL_COMMAND_DIAGNOSTICS_SCHEMA,
      command,
      selectedCommand: command,
      evidenceSchema: this.documentDiagnostics.evidenceSchema,
      loadedSchema: this.documentDiagnostics.loadedSchema,
      recordCount: this.documentDiagnostics.recordCount,
      datasetCount: this.documentDiagnostics.datasetCount,
      spaceCount: this.documentDiagnostics.spaceCount,
      coordinateCount: this.documentDiagnostics.coordinateCount,
      relationCount: this.documentDiagnostics.relationCount,
      propertyCount: this.documentDiagnostics.propertyCount,
      graphCount: this.documentDiagnostics.graphCount,
      timelineCount: this.documentDiagnostics.timelineCount,
      declaredViewCount: this.documentDiagnostics.declaredViewCount,
      evidenceKind: evidence.kind,
      evidenceNative: evidence.native,
      evidenceDocumentedReference: evidence.documentedReference,
      evidenceSyntheticFixture: evidence.syntheticFixture,
      evidenceKindSignals: evidence.signals.slice(),
      evidenceProvenance: { ...evidence.provenance },
      evidenceReport: cloneDiagnosticValue(this.documentDiagnostics.evidenceReport),
      selectedViewKind: options.viewKind ?? inferMetricVisualViewKind(this.views),
      descriptorCount: this.descriptors.length,
      layerDescriptorCount: this.descriptors.length,
      runtimeLayerCount: runtimeState.layerInstanceCount ?? countRuntimeLayers(this.runtime),
      runtimeLayerState: runtimeState.layerState ? { ...runtimeState.layerState } : null,
      descriptorKinds: describeLayerDescriptors(this.descriptors),
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
      evidenceSchema: this.documentDiagnostics.evidenceSchema,
      loadedSchema: this.documentDiagnostics.loadedSchema,
      recordCount: this.documentDiagnostics.recordCount,
      datasetCount: this.documentDiagnostics.datasetCount,
      spaceCount: this.documentDiagnostics.spaceCount,
      coordinateCount: this.documentDiagnostics.coordinateCount,
      relationCount: this.documentDiagnostics.relationCount,
      propertyCount: this.documentDiagnostics.propertyCount,
      graphCount: this.documentDiagnostics.graphCount,
      timelineCount: this.documentDiagnostics.timelineCount,
      declaredViewCount: this.documentDiagnostics.declaredViewCount,
      evidenceKind: this.documentDiagnostics.evidenceKind,
      evidenceNative: this.documentDiagnostics.evidenceNative,
      evidenceDocumentedReference: this.documentDiagnostics.evidenceDocumentedReference,
      evidenceSyntheticFixture: this.documentDiagnostics.evidenceSyntheticFixture,
      evidenceKindSignals: this.documentDiagnostics.evidenceKindSignals.slice(),
      evidenceProvenance: { ...this.documentDiagnostics.evidenceProvenance },
      evidenceReport: cloneDiagnosticValue(this.documentDiagnostics.evidenceReport),
      selectedViewKind: current?.selectedViewKind ?? inferMetricVisualViewKind(this.views),
      selectedCommand: current?.selectedCommand ?? current?.command ?? null,
      descriptorCount: current?.descriptorCount ?? this.descriptors.length,
      layerDescriptorCount: current?.layerDescriptorCount ?? this.descriptors.length,
      runtimeLayerCount: current?.runtimeLayerCount ?? runtimeState.layerInstanceCount ?? countRuntimeLayers(this.runtime),
      descriptorKinds: cloneDiagnosticValue(current?.descriptorKinds ?? describeLayerDescriptors(this.descriptors)),
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

  configureTimelineControl(view, options = {}) {
    const widgetOptions = normalizeTimelineControlWidgetOptions(options);
    if (widgetOptions.enabled === false) {
      return this.disableTimelineControl();
    }
    const descriptor = widgetOptions.descriptor || view?.timelineControl || findTimelineControlDescriptor(this.descriptors);
    const canAttachDom = typeof document !== "undefined"
      && typeof window !== "undefined"
      && this.canvas;
    if (!descriptor || !canAttachDom) return this;
    const root = widgetOptions.root
      || options.timelineControlRoot
      || options.controlsRoot
      || this.canvas.parentElement
      || document.body;
    const onTimelineChange = (state) => this.applyTimelineControlState(state);
    if (!this.timelineControlWidget) {
      this.timelineControlWidget = new TimelineControlWidget({
        root,
        runtime: this.runtime,
        descriptor,
        ...widgetOptions,
        onTimelineChange,
      });
    } else {
      this.timelineControlWidget
        .setRuntime(this.runtime)
        .setOnTimelineChange(onTimelineChange)
        .setRoot(root)
        .setDescriptor(descriptor);
    }
    return this;
  }

  configureMappingMotion(view, options = {}) {
    this.disableMappingMotion();
    const timing = normalizeMappingMotionTiming(
      options.motionTiming || options.mappingMotionTiming || options.morphTiming || view?.motionTiming,
      options,
    );
    const staticProgress = Number(options.progress ?? options.morphProgress);
    if (options.loop === false || Number.isFinite(staticProgress)) {
      const progress = Number.isFinite(staticProgress) ? staticProgress : 0;
      this.applyMappingMotionProgress(progress, {
        phase: progress >= 0.95 ? "target-hold" : progress <= 0.05 ? "source-hold" : "quick-transition",
        timing,
        emit: true,
        render: true,
      });
      return this;
    }

    const startedAt = this.runtime?.renderer?.loop?.time?.elapsed ?? 0;
    const update = (context = {}) => {
      const elapsed = Math.max(0, Number(context?.time?.elapsed ?? 0) - startedAt);
      const frame = mappingMotionProgressAt(elapsed, timing);
      this.applyMappingMotionProgress(frame.progress, {
        ...frame,
        timing,
        emit: true,
        render: false,
      });
    };
    if (this.runtime?.renderer && typeof this.runtime.renderer.on === "function") {
      this.mappingMotionSubscription = this.runtime.renderer.on("beforeRender", update);
      this.mappingMotionState = {
        schema: "metric.visual.mapping_motion_state.v1",
        mode: "runtime-before-render",
        timing,
        progress: 0,
        phase: "source-hold",
      };
      this.runtime.start?.();
      this.runtime.requestRender?.();
    }
    return this;
  }

  applyMappingMotionProgress(progress, options = {}) {
    const next = Math.max(0, Math.min(1, Number(progress) || 0));
    for (const layer of this.runtime?.layers || []) {
      const animation = layer?.animation;
      if (!animation) continue;
      if (String(animation.mode || "").includes("morph") || animation.channel === "targetPosition") {
        animation.progress = next;
      }
    }
    for (const descriptor of this.descriptors || []) {
      const animation = descriptor?.animation;
      if (!animation) continue;
      if (String(animation.mode || "").includes("morph") || animation.channel === "targetPosition") {
        animation.progress = next;
      }
    }
    this.mappingMotionState = {
      schema: "metric.visual.mapping_motion_state.v1",
      mode: options.mode || "runtime-before-render",
      progress: next,
      phase: options.phase || null,
      elapsedMs: Number.isFinite(Number(options.elapsedMs)) ? Number(options.elapsedMs) : null,
      timing: options.timing || null,
    };
    if (options.emit !== false) {
      this.runtime?.emit?.("mappingmotionchange", {
        runtime: this.runtime,
        surface: this,
        state: this.mappingMotionState,
      });
    }
    if (options.render === true) this.runtime?.requestRender?.();
    return this;
  }

  disableMappingMotion() {
    if (this.mappingMotionSubscription) {
      this.mappingMotionSubscription();
      this.mappingMotionSubscription = null;
    }
    this.mappingMotionState = null;
    return this;
  }

  applyTimelineControlState(state = {}) {
    if (this.applyingTimelineControl) return this;
    const normalized = Number(state.normalized ?? state.value);
    if (!Number.isFinite(normalized)) return this;
    const base = this.lastDynamicsOptions || {};
    this.applyingTimelineControl = true;
    try {
      this.showDynamics({
        ...base,
        timelineProgress: normalized,
        timelinePosition: normalized,
      });
    } finally {
      this.applyingTimelineControl = false;
    }
    this.runtime?.emit?.("timelinecontrolchange", {
      runtime: this.runtime,
      surface: this,
      state,
      descriptor: state.descriptor || state.control || null,
    });
    return this;
  }

  disableTimelineControl() {
    this.timelineControlWidget?.dispose?.();
    this.timelineControlWidget = null;
    return this;
  }

  enableRecordPreview(options = {}) {
    const canAttachDom = typeof document !== "undefined"
      && typeof window !== "undefined"
      && this.canvas
      && typeof this.canvas.addEventListener === "function";
    if (!this.previewPanel) {
      if (canAttachDom) {
        this.previewPanel = new RecordPreviewPanel({
          root: options.root || document.body,
          resolver: (input) => buildRecordPreview(this, input),
          ...(options.panel || {}),
        });
      }
    } else if (options.panel?.resolver) {
      this.previewPanel.setResolver(options.panel.resolver);
    }
    if (canAttachDom && !this.previewSubscriptions.length) {
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
        if (selection?.pair) {
          this.previewPanel?.show({
            pair: selection.pair,
            x: window.innerWidth * 0.58,
            y: window.innerHeight * 0.34,
          });
          return;
        }
        if (!selection?.recordId) return;
        this.previewPanel?.show({
          recordId: selection.recordId,
          focusTarget: { recordId: selection.recordId },
          x: window.innerWidth * 0.58,
          y: window.innerHeight * 0.34,
        });
      }));
      this.previewPointerMoveBound ||= ((event) => this.handlePreviewPointerMove(event));
      this.previewPointerClickBound ||= ((event) => this.handlePreviewPointerClick(event));
      this.hideRecordPreviewBound ||= (() => this.previewPanel?.hide());
      this.canvas.addEventListener("pointermove", this.previewPointerMoveBound);
      this.canvas.addEventListener("click", this.previewPointerClickBound);
      this.canvas.addEventListener("pointerleave", this.hideRecordPreviewBound);
    }
    this.previewMode = options.mode || "record";
    return this;
  }

  disableRecordPreview() {
    for (const unsubscribe of this.previewSubscriptions.splice(0)) unsubscribe?.();
    if (this.previewPointerMoveBound) this.canvas.removeEventListener("pointermove", this.previewPointerMoveBound);
    if (this.previewPointerClickBound) this.canvas.removeEventListener("click", this.previewPointerClickBound);
    if (this.hideRecordPreviewBound) this.canvas.removeEventListener("pointerleave", this.hideRecordPreviewBound);
    this.previewPanel?.hide();
    return this;
  }

  updatePreviewIndex() {
    this.previewRecords = [];
    this.previewPairPickers = [];
    for (const view of this.views || []) {
      collectPreviewRecords(this.previewRecords, view);
    }
    for (const descriptor of this.descriptors || []) {
      if (descriptor?.primitive !== "RelationMatrixLayer") continue;
      try {
        const picker = createRelationMatrixPicker(descriptor, { canvas: this.canvas });
        picker.relationId = descriptor.source?.relationId || descriptor.metadata?.relationId || descriptor.relationId || null;
        this.previewPairPickers.push(picker);
      } catch {
        // Non-matrix descriptors or incomplete development descriptors are
        // ignored here; renderer creation still reports hard layer errors.
      }
    }
    return this;
  }

  handlePreviewPointerMove(event) {
    if (!this.previewPanel) return;
    if (String(this.previewMode || "").toLowerCase().includes("pair")) {
      const pairHit = this.pickPreviewPair(event);
      if (pairHit) {
        this.previewPanel.show({
          pair: pairHit,
          x: event.clientX,
          y: event.clientY,
        });
        return;
      }
    }
    if (!this.previewRecords?.length) {
      this.previewPanel.hide();
      return;
    }
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

  handlePreviewPointerClick(event) {
    if (String(this.previewMode || "").toLowerCase().includes("pair")) {
      this.inspectRuntimeAtEvent(event, {
        mode: "click",
        select: true,
        source: "metric-visual-pair-preview",
      });
    }
  }

  inspectRuntimeAtEvent(event, options = {}) {
    if (!this.runtime || !this.canvas) return null;
    const rect = this.canvas.getBoundingClientRect();
    const pixelRatio = this.runtime.renderer?.size?.pixelRatio || globalThis.devicePixelRatio || 1;
    const x = event.clientX - rect.left;
    const y = event.clientY - rect.top;
    return this.runtime.inspectAt({}, {
      ...options,
      rawPointer: {
        x,
        y,
        css: { x, y },
        pixel: { x: x * pixelRatio, y: y * pixelRatio },
        inside: x >= 0 && y >= 0 && x <= rect.width && y <= rect.height,
      },
      sourceEvent: event,
    });
  }

  pickPreviewPair(event) {
    for (const picker of this.previewPairPickers || []) {
      const cell = picker.cellAtClientPoint(event.clientX, event.clientY);
      if (!cell) continue;
      return {
        ...cell,
        relationId: picker.relationId,
      };
    }
    return null;
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

export function classifyMetricVisualEvidence(document) {
  const provenance = document?.provenance || {};
  const signals = [];
  const syntheticFixture = provenance.synthetic === true || provenance.synthetic_fixture === true;
  if (provenance.synthetic === true) signals.push("provenance.synthetic");
  if (provenance.synthetic_fixture === true) signals.push("provenance.synthetic_fixture");
  const explicitNativeExport = isExplicitNativeMetricVisualExport(provenance);
  const native = !syntheticFixture && explicitNativeExport;
  if (native) {
    signals.push(provenance.native_export === true ? "provenance.native_export" : "provenance.nativeExport");
  }
  const documentedReference = !syntheticFixture && !native && (
    provenance.documented_reference === true
    || provenance.reference === true
    || Boolean(provenance.reference_url || provenance.documentation || provenance.source_document)
  );
  if (documentedReference) {
    if (provenance.documented_reference === true) signals.push("provenance.documented_reference");
    if (provenance.reference === true) signals.push("provenance.reference");
    if (provenance.reference_url) signals.push("provenance.reference_url");
    if (provenance.documentation) signals.push("provenance.documentation");
    if (provenance.source_document) signals.push("provenance.source_document");
  }
  const kind = syntheticFixture
    ? "synthetic_fixture"
    : native
      ? "native"
      : documentedReference
        ? "documented_reference"
        : "unknown";
  if (!signals.length) signals.push("no_evidence_kind_signal");
  return {
    kind,
    native,
    documentedReference,
    syntheticFixture,
    signals,
    provenance: summarizeMetricVisualProvenance(provenance),
  };
}

function createMetricVisualEvidenceReport(document, evidence = classifyMetricVisualEvidence(document)) {
  return {
    schema: "metric.visual.public_evidence_report.v1",
    evidenceSchema: document?.schema || null,
    kind: evidence.kind,
    native: evidence.native,
    documentedReference: evidence.documentedReference,
    syntheticFixture: evidence.syntheticFixture,
    signals: evidence.signals.slice(),
    provenance: { ...evidence.provenance },
    recordCount: countVisualRecords(document),
    datasetCount: countVisualCollection(document, "datasets"),
    spaceCount: countVisualCollection(document, "spaces"),
    coordinateCount: countVisualCollection(document, "coordinates"),
    relationCount: countVisualCollection(document, "relations"),
    propertyCount: countVisualCollection(document, "properties"),
    graphCount: countVisualCollection(document, "graphs"),
    timelineCount: countVisualCollection(document, "timelines"),
    declaredViewCount: countVisualCollection(document, "views"),
    coordinateDimensions: summarizeVisualCoordinates(document),
    relationStorage: summarizeVisualRelations(document),
    propertyTargets: summarizeVisualProperties(document),
    declaredViewKinds: summarizeVisualViews(document),
  };
}

function summarizeMetricVisualProvenance(provenance = {}) {
  return {
    generator: provenance.generator || provenance.writer || null,
    runtime: provenance.runtime || provenance.computation || null,
    source: provenance.source || provenance.source_example || provenance.source_document || null,
    status: provenance.status || null,
    nativeExport: isExplicitNativeMetricVisualExport(provenance),
    synthetic: provenance.synthetic === true || provenance.synthetic_fixture === true,
    syntheticJs: provenance.synthetic_js === true,
    publicHeroReady: provenance.public_hero_ready === true,
    nativeChecksPass: provenance.native_checks_pass === true,
  };
}

export function isExplicitNativeMetricVisualExport(provenance = {}) {
  return provenance.native_export === true || provenance.nativeExport === true;
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

function countVisualCollection(document, key) {
  return Array.isArray(document?.[key]) ? document[key].length : 0;
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
  return cloneDiagnosticValue(diagnostics);
}

function cloneDiagnosticEntries(entries) {
  return Array.isArray(entries) ? entries.map((entry) => cloneDiagnosticValue(entry)) : [];
}

function cloneDiagnosticValue(value) {
  if (Array.isArray(value)) return value.map((entry) => cloneDiagnosticValue(entry));
  if (!value || typeof value !== "object") return value;
  return Object.fromEntries(Object.entries(value).map(([key, entry]) => [key, cloneDiagnosticValue(entry)]));
}

function describeLayerDescriptors(descriptors = []) {
  const primitiveCounts = new Map();
  const viewKindCounts = new Map();
  for (const descriptor of descriptors || []) {
    const primitive = descriptor?.primitive || descriptor?.kind || "unknown";
    const viewKind = descriptor?.source?.viewKind || descriptor?.metadata?.viewKind || descriptor?.kind || "unknown";
    primitiveCounts.set(primitive, (primitiveCounts.get(primitive) || 0) + 1);
    viewKindCounts.set(viewKind, (viewKindCounts.get(viewKind) || 0) + 1);
  }
  return {
    primitives: Object.fromEntries(primitiveCounts),
    viewKinds: Object.fromEntries(viewKindCounts),
  };
}

function summarizeVisualCoordinates(document) {
  const counts = new Map();
  for (const coordinate of document?.coordinates || []) {
    const dimension = Number.isFinite(Number(coordinate?.dimension)) ? String(Number(coordinate.dimension)) : "unknown";
    counts.set(dimension, (counts.get(dimension) || 0) + 1);
  }
  return Object.fromEntries(counts);
}

function summarizeVisualRelations(document) {
  const counts = new Map();
  for (const relation of document?.relations || []) {
    const key = relation?.storage || relation?.relation_type || "unknown";
    counts.set(key, (counts.get(key) || 0) + 1);
  }
  return Object.fromEntries(counts);
}

function summarizeVisualProperties(document) {
  const counts = new Map();
  for (const property of document?.properties || []) {
    const key = `${property?.target_type || "record"}:${property?.value_type || "unknown"}`;
    counts.set(key, (counts.get(key) || 0) + 1);
  }
  return Object.fromEntries(counts);
}

function summarizeVisualViews(document) {
  const counts = new Map();
  for (const view of document?.views || []) {
    const key = view?.kind || "unknown";
    counts.set(key, (counts.get(key) || 0) + 1);
  }
  return Object.fromEntries(counts);
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

function processCurveCamera() {
  return {
    fov: 34,
    target: [0.02, -0.08, 0.04],
    yaw: -0.46,
    pitch: 0.34,
    radius: 7.35,
  };
}

function firstVisualRelationId(document) {
  return document?.relations?.find((relation) => relation?.relation_type === "metric")?.id
    || document?.relations?.[0]?.id
    || null;
}

async function resolveMetricVisualInput(input, options = {}) {
  if (typeof input === "string") return loadVisualDocument(input, options.data || {});
  if (input && typeof input === "object") return normalizeVisualInput(input, options.data || {});
  throw new Error("createMetricVisual() requires evidence, document, url or input.");
}

function buildRecordPreview(surface, input = {}) {
  if (input.pair || input.pairId || input.rowId || input.row_id) return buildPairPreview(surface, input);
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

function buildPairPreview(surface, input = {}) {
  const pair = input.pair || input.focusTarget?.pair || input;
  const rowId = pair.rowId ?? pair.row_id ?? pair.sourceId ?? pair.source_id;
  const columnId = pair.columnId ?? pair.column_id ?? pair.targetId ?? pair.target_id;
  if (rowId == null || columnId == null) return null;

  const relationId = pair.relationId ?? pair.relation_id ?? firstRelationId(surface.document);
  const relation = relationId ? surface.visualSpace.getRelation(relationId) : null;
  const entry = pair.value !== undefined
    ? pair
    : relationId
      ? surface.visualSpace.relationValue(relationId, rowId, columnId)
      : null;
  const rowRecord = surface.visualSpace.getRecord(rowId);
  const columnRecord = surface.visualSpace.getRecord(columnId);
  const value = entry?.value ?? pair.value;
  const present = pair.present ?? Boolean(entry);

  const fields = [
    { label: "relation", value: relation?.name || relation?.id || relationId || "relation" },
    { label: "row", value: labelForRecord(rowRecord, rowId) },
    { label: "column", value: labelForRecord(columnRecord, columnId) },
    { label: "value", value: present && Number.isFinite(Number(value)) ? Number(value).toFixed(4) : (present ? value : "no direct pair") },
  ];

  for (const property of surface.document.properties || []) {
    if (property.target_type !== "pair") continue;
    const propertyValue = surface.visualSpace.propertyValue(property.id, {
      relation_id: relationId,
      row_id: rowId,
      column_id: columnId,
    });
    if (propertyValue) fields.push({ label: property.name || property.id, value: propertyValue.value });
    if (fields.length >= 8) break;
  }

  return {
    title: `${rowId} ↔ ${columnId}`,
    subtitle: relation?.relation_type || "pair relation",
    fields,
  };
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

function labelForRecord(record, fallback) {
  return record?.label || record?.id || fallback;
}

function firstRelationId(document) {
  return Array.isArray(document?.relations) && document.relations.length ? document.relations[0].id : null;
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

function normalizeTimelineControlWidgetOptions(options = {}) {
  const raw = options.timelineControlWidget ?? options.timelineWidget ?? options.timelineControl;
  if (raw === false) return { enabled: false };
  if (raw && typeof raw === "object" && raw.schema === "metric.visual.timeline_control.v1") {
    return { enabled: true, descriptor: raw };
  }
  if (raw && typeof raw === "object") return { enabled: true, ...raw };
  return { enabled: true };
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

function normalizePropertyFieldViewOptions(options = {}) {
  const next = { ...options };
  const explicitPropertyId = typeof options.propertyField === "string"
    ? options.propertyField
    : typeof options.groundField === "string"
      ? options.groundField
      : null;
  if (explicitPropertyId) next.propertyId = explicitPropertyId;
  else if (next.propertyId === true || next.propertyId === false) delete next.propertyId;
  return next;
}

function normalizeMappingOptions(document, options = {}) {
  const next = normalizeMetricViewOptions(document, options);
  if (next.residual && !next.residualProperty) next.residualProperty = next.residual;
  if (next.residualProperty && !next.scalarProperty) next.scalarProperty = next.residualProperty;
  if (next.morphDurationMs !== undefined && next.durationMs === undefined) next.durationMs = next.morphDurationMs;
  return next;
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
