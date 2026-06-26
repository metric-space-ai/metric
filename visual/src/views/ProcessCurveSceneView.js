import {
  createMiniatureAnimationPreset,
  createMiniatureSceneBundle,
  createMiniatureStagePreset,
} from "../style/miniature/index.js";
import { BaseView } from "./BaseView.js";
import { DenseFieldView } from "./DenseFieldView.js";
import { GroundProjectionView } from "./GroundProjectionView.js";
import { MorphView } from "./MorphView.js";
import { NeighborhoodGraphView } from "./NeighborhoodGraphView.js";
import { PropertyFieldView } from "./PropertyFieldView.js";
import { RelationMatrixView } from "./RelationMatrixView.js";
import { TrajectoryPathView } from "./TrajectoryPathView.js";
import { createChannel } from "./view-utils.js";

export class ProcessCurveSceneView extends BaseView {
  constructor(options = {}) {
    const inputs = options.inputs || {};
    const sourceCoordinate = options.sourceCoordinate || inputs.sourceCoordinate || null;
    const targetCoordinate = options.targetCoordinate || inputs.targetCoordinate || null;
    const labelProperty = options.labelProperty || inputs.labelProperty || null;
    const relation = options.relation || null;
    const graph = options.graph || null;
    const datasetId = options.datasetId ?? inputs.datasetId ?? targetCoordinate?.dataset_id ?? null;
    const sourceCoordinateId = options.sourceCoordinateId ?? sourceCoordinate?.id ?? targetCoordinate?.id ?? null;
    const targetCoordinateId = options.targetCoordinateId ?? targetCoordinate?.id ?? options.coordinateId ?? null;
    const propertyId = options.propertyId ?? options.labelPropertyId ?? labelProperty?.id ?? null;
    const relationId = options.relationId ?? relation?.id ?? null;
    const graphId = options.graphId ?? graph?.id ?? null;
    const metadata = {
      ...(options.metadata || {}),
      viewClass: "ProcessCurveSceneView",
      visualGrammar: "process-curves",
      algorithmicComputation: false,
      datasetId,
      coordinateId: targetCoordinateId,
      sourceCoordinateId,
      targetCoordinateId,
      propertyId,
      labelPropertyId: propertyId,
      relationId,
      graphId,
      relationIds: {
        primary: relationId,
        graph: graphId,
      },
    };
    super({
      ...options,
      kind: "process-curves",
      datasetId,
      coordinateId: targetCoordinateId,
      propertyId,
      relationId,
      metadata,
    });
    this.document = options.document || null;
    this.inputs = {
      ...inputs,
      datasetId,
      sourceCoordinate,
      targetCoordinate,
      labelProperty,
      records: options.records || inputs.records || [],
    };
    this.recordIds = (options.recordIds || this.inputs.records.map((record) => record.id)).map(String);
    this.positions = options.positions || processCurvePreviewPositions(targetCoordinate, this.recordIds);
    this.sourceCoordinateId = sourceCoordinateId;
    this.targetCoordinateId = targetCoordinateId;
    this.labelPropertyId = propertyId;
    this.relation = relation;
    this.graph = graph;
    this.graphId = graphId;
    this.includeNeighborhood = options.includeNeighborhood !== false && options.neighborhood !== false;
    this.includeMatrix = options.includeMatrix === true || options.matrix === true;
    this.options = {
      ...options,
      datasetId,
      sourceCoordinateId,
      targetCoordinateId,
      labelPropertyId: propertyId,
      relationId,
      graphId,
    };
  }

  static fromVisualSpace(document, options = {}) {
    const inputs = resolveProcessCurveSceneInputs(document, options);
    const relation = resolveProcessCurveRelation(document, inputs.datasetId, options);
    const graph = resolveProcessCurveGraph(document, inputs.datasetId, relation, options);
    const matrixRequested = options.includeMatrix === true || options.matrix === true;
    const graphRequested = options.includeNeighborhood === true || options.neighborhood === true;
    if (matrixRequested && !relation) {
      throw new Error(`Process-curve scene requires a relation for matrix support in ${inputs.datasetId}.`);
    }
    if (graphRequested && !relation && !graph) {
      throw new Error(`Process-curve scene requires relation or graph evidence for neighborhood support in ${inputs.datasetId}.`);
    }
    return new ProcessCurveSceneView({
      ...options,
      document,
      inputs,
      records: inputs.records,
      datasetId: inputs.datasetId,
      sourceCoordinate: inputs.sourceCoordinate,
      targetCoordinate: inputs.targetCoordinate,
      labelProperty: inputs.labelProperty,
      sourceCoordinateId: inputs.sourceCoordinate?.id,
      targetCoordinateId: inputs.targetCoordinate?.id,
      coordinateId: inputs.targetCoordinate?.id,
      propertyId: inputs.labelProperty?.id,
      labelPropertyId: inputs.labelProperty?.id,
      relation,
      relationId: relation?.id ?? null,
      graph,
      graphId: graph?.id ?? null,
    });
  }

  toLayerDescriptors() {
    if (!this.document) throw new Error("ProcessCurveSceneView requires a metric.visual document.");
    const descriptors = createProcessCurveLayerDescriptors(this.document, this.inputs, this.options)
      .map((descriptor) => this.withProcessCurveMetadata(descriptor));
    descriptors.push(...this.neighborhoodDescriptors());
    descriptors.push(...this.matrixDescriptors());
    return descriptors.filter(Boolean);
  }

  neighborhoodDescriptors() {
    if (!this.includeNeighborhood || !this.relationId || !this.targetCoordinateId) return [];
    const graph = NeighborhoodGraphView.fromVisualSpace(this.document, {
      coordinateId: this.targetCoordinateId,
      relationId: this.relationId,
      graphId: this.graph?.id || this.graphId || undefined,
      colorProperty: this.labelPropertyId,
      topK: this.options.topK ?? 4,
      size: this.options.neighborhoodPointSize ?? 1,
      targetRadius: this.options.neighborhoodTargetRadius ?? 1.6,
      groundY: this.options.neighborhoodGroundY ?? 0,
      fit: this.options.neighborhoodFit,
    });
    return graph.toLayerDescriptors().map((descriptor) => this.withProcessCurveMetadata({
      ...descriptor,
      id: `${descriptor.id || "process-curve-neighborhood"}:support`,
      order: Math.max(Number(descriptor.order ?? 0), 36),
      metadata: {
        ...(descriptor.metadata || {}),
        role: "process-curve-neighborhood-support",
      },
    }));
  }

  matrixDescriptors() {
    if (!this.includeMatrix || !this.relationId) return [];
    const matrix = RelationMatrixView.fromVisualSpace(this.document, {
      relationId: this.relationId,
      rect: this.options.matrixRect || [0.61, 0.25, 0.35, 0.50],
      palette: this.options.palette || "metric",
      symmetric: this.options.symmetric ?? true,
      missingAlpha: this.options.matrixMissingAlpha ?? 0,
      materialAlpha: this.options.matrixMaterialAlpha ?? 0.96,
    });
    return matrix.toLayerDescriptors().map((descriptor) => this.withProcessCurveMetadata({
      ...descriptor,
      order: Math.max(Number(descriptor.order ?? 0), 220),
      metadata: {
        ...(descriptor.metadata || {}),
        role: "process-curve-relation-matrix-support",
      },
    }));
  }

  withProcessCurveMetadata(descriptor) {
    if (!descriptor) return descriptor;
    const resolved = this.resolvedMetadata();
    return {
      ...descriptor,
      source: {
        ...(descriptor.source || {}),
        processCurveViewId: this.id,
        processCurveViewKind: this.kind,
        processCurveViewClass: "ProcessCurveSceneView",
        datasetId: descriptor.source?.datasetId ?? this.datasetId,
        coordinateId: descriptor.source?.coordinateId ?? this.targetCoordinateId,
        propertyId: descriptor.source?.propertyId ?? this.labelPropertyId,
        relationId: descriptor.source?.relationId ?? this.relationId ?? null,
        graphId: descriptor.source?.graphId ?? this.graphId ?? null,
      },
      metadata: {
        ...(descriptor.metadata || {}),
        visualGrammar: "process-curves",
        algorithmicComputation: false,
        processCurveView: resolved,
      },
    };
  }

  resolvedMetadata() {
    return {
      viewClass: "ProcessCurveSceneView",
      visualGrammar: "process-curves",
      algorithmicComputation: false,
      datasetId: this.datasetId,
      coordinateId: this.targetCoordinateId,
      sourceCoordinateId: this.sourceCoordinateId,
      targetCoordinateId: this.targetCoordinateId,
      propertyId: this.labelPropertyId,
      labelPropertyId: this.labelPropertyId,
      relationId: this.relationId ?? null,
      graphId: this.graphId ?? null,
      relationIds: {
        primary: this.relationId ?? null,
        graph: this.graphId ?? null,
      },
    };
  }
}

export function resolveProcessCurveSceneInputs(document, options = {}) {
  const datasetId = options.datasetId || firstDatasetId(document);
  const coordinates = document?.coordinates || [];
  const properties = document?.properties || [];
  const coordinateName = (entry) => `${entry?.id || ""} ${entry?.name || ""}`.toLowerCase();
  const coordinateKey = (entry) => coordinateName(entry).replace(/[^a-z0-9]/g, "");
  const matchesCoordinateName = (entry, name) => {
    const needle = String(name || "").toLowerCase();
    const compactNeedle = needle.replace(/[^a-z0-9]/g, "");
    return coordinateName(entry).includes(needle) || coordinateKey(entry).includes(compactNeedle);
  };
  let sourceCoordinate = resolveByIdOrPredicate(
    coordinates,
    options.sourceCoordinateId || options.sourceCoordinate,
    (entry) => entry.dataset_id === datasetId && matchesCoordinateName(entry, options.sourceCoordinateName || "landmark2"),
  );
  let targetCoordinate = resolveByIdOrPredicate(
    coordinates,
    options.targetCoordinateId || options.targetCoordinate || options.coordinateId || options.coordinate,
    (entry) => entry.dataset_id === datasetId && matchesCoordinateName(entry, options.targetCoordinateName || "landmark3"),
  );
  const labelProperty = resolveByIdOrPredicate(
    properties,
    options.labelPropertyId || options.propertyId || options.labelProperty,
    (entry) => {
      if (entry.dataset_id !== datasetId) return false;
      const name = `${entry.id || ""} ${entry.name || ""}`.toLowerCase();
      return String(entry.id || "").endsWith(":record_label") || /(^|[-_: ])(role|label|state|regime)([-_: ]|$)/.test(name);
    },
  );
  if (!targetCoordinate) {
    targetCoordinate = coordinates.find((entry) => (
      entry.dataset_id === datasetId && Number(entry.dimension) === 3
    )) || coordinates.find((entry) => entry.dataset_id === datasetId);
  }
  if (!sourceCoordinate) sourceCoordinate = targetCoordinate;
  if (!datasetId) throw new Error("Process-curve scene requires a dataset id.");
  if (!targetCoordinate) throw new Error(`Process-curve scene could not find target coordinate for ${datasetId}.`);
  if (!labelProperty) throw new Error(`Process-curve scene could not find label property for ${datasetId}.`);
  return {
    datasetId,
    sourceCoordinate,
    targetCoordinate,
    labelProperty,
    records: (document?.records || []).filter((record) => record.dataset_id === datasetId),
  };
}

export function createProcessCurveMiniatureStage(options = {}) {
  const materials = options.materials || {};
  const grounding = options.grounding || {};
  const postprocess = options.postprocess || {};
  return createMiniatureStagePreset({
    look: options.look,
    ...((options.variant || !options.look) ? { variant: options.variant || "white" } : {}),
    camera: {
      fov: 36,
      position: [0, -3.25, 1.72],
      target: [0, -0.1, 0.02],
      ...(options.camera || {}),
    },
    focus: {
      axis: "horizontal",
      yRatio: 0.48,
      radiusRatio: 0.32,
      focusBandRatio: 0.07,
      blurRadius: 5.4,
      blurCurve: 1.42,
      ...(options.focus || {}),
    },
    materials: {
      ...materials,
      mesh: {
        ambient: 0.82,
        pointLight: 0.92,
        specular: 0.2,
        ambientColor: [0.96, 0.97, 0.92],
        ...(materials.mesh || {}),
      },
    },
    grounding: {
      ...grounding,
      groundY: finiteNumber(options.groundY, -0.56),
      ground: {
        gridScale: 6,
        alpha: 0.42,
        fade: 0.9,
        gridAlpha: 0.28,
        axisAlpha: 0.24,
        stageTilt: -0.12,
        stageScale: [0.94, 0.76],
        sheen: 0.06,
        matte: 0.82,
        contactShade: 0.16,
        horizonFade: 0.12,
        ...(grounding.ground || {}),
      },
      projection: {
        alpha: 0.52,
        softness: 0.8,
        footprintStretch: 2.05,
        footprintSkew: -0.3,
        footprintCore: 0.16,
        footprintFalloff: 1.14,
        colorMix: 0.68,
        shadowDensity: 1.26,
        coreDensity: 0.42,
        edgeTint: 0.54,
        shadowDirection: [-0.44, 0.9],
        shadowTail: 0.6,
        shadowTailStrength: 0.3,
        contactHardness: 0.34,
        surfaceLift: 0.02,
        pointPixelScale: 18,
        ...(grounding.projection || {}),
      },
      contact: {
        alpha: 0.46,
        pointPixelScale: 26,
        radiusScale: 1.34,
        heightScale: 0.32,
        footprintStretch: 2.22,
        footprintSkew: -0.36,
        colorMix: 0.52,
        shadowDensity: 1.48,
        coreDensity: 0.52,
        edgeTint: 0.46,
        shadowDirection: [-0.44, 0.9],
        shadowTail: 0.68,
        shadowTailStrength: 0.38,
        contactHardness: 0.44,
        surfaceLift: 0.018,
        ...(grounding.contact || {}),
      },
    },
    postprocess: {
      ...postprocess,
      postFx: {
        grade: {
          enabled: true,
          contrast: 1.04,
          saturation: 0.96,
          highlightCompression: 0.12,
        },
        vignette: {
          enabled: true,
          amount: 0.08,
          radius: 0.84,
          softness: 0.72,
          color: [0.86, 0.88, 0.84],
        },
        bloom: {
          enabled: true,
          intensity: 0.1,
          threshold: 0.94,
          knee: 0.16,
          radius: 1.0,
          iterations: 3,
          downsample: 2,
          saturation: 0.72,
          warmth: 0.08,
          clamp: 0.78,
        },
        filmGrain: { enabled: false },
        ...(postprocess.postFx || {}),
      },
    },
    ...(options.stage || {}),
  });
}

function createProcessCurveMiniatureLayerDescriptors(document, options = {}) {
  const inputs = resolveProcessCurveSceneInputs(document, options);
  return {
    inputs,
    descriptors: createProcessCurveLayerDescriptors(document, inputs, options),
  };
}

function createProcessCurveLayerDescriptors(document, inputs, options = {}) {
  const groundY = finiteNumber(options.groundY, options.stage?.grounding?.groundY, -0.56);
  const morph = createProcessCurveMorphDescriptor(document, inputs, options);
  const projection = createProcessCurveProjectionDescriptor(document, inputs, { ...options, groundY: groundY + 0.04 });
  const skyline = shouldIncludeProcessCurveSkyline(options)
    ? createProcessCurveSkylineDescriptor(document, inputs, { ...options, groundY })
    : null;
  const field = createProcessCurvePropertyFieldDescriptor(document, inputs, { ...options, groundY: groundY + 0.03 })
    || createProcessCurveStateFieldDescriptor(document, inputs, { ...options, groundY: groundY + 0.03 });
  const track = createProcessCurveTrackDescriptor(document, inputs, { ...options, groundY: groundY + 0.06 });
  const labels = createProcessCurveLabelDescriptor(document, inputs, { ...options, groundY });
  return [field, projection, track, skyline, morph, labels].filter(Boolean);
}

function shouldIncludeProcessCurveSkyline(options = {}) {
  return options.includeRecordSkyline !== false
    && options.recordSkyline !== false
    && options.curveEnergySkyline !== false;
}

export function createProcessCurveMiniatureSceneBundle(document, options = {}) {
  const stage = options.stage || createProcessCurveMiniatureStage(options);
  const { inputs, descriptors } = createProcessCurveMiniatureLayerDescriptors(document, {
    ...options,
    stage,
    groundY: stage.grounding?.groundY,
  });
  const bundle = createMiniatureSceneBundle(descriptors, {
    stage,
    fit: {
      targetSpan: 1.95,
      ...(options.fit || {}),
    },
    ground: {
      groundY: stage.grounding?.groundY,
      padding: 0.48,
      ...(options.ground || {}),
    },
    motion: options.motion ?? false,
    contacts: options.contacts,
  });
  return { inputs, stage, descriptors, bundle };
}

function createProcessCurveMorphDescriptor(document, inputs, options) {
  const loop = options.morphLoop === true;
  const hasProgress = Number.isFinite(Number(options.morphProgress));
  const progress = hasProgress ? Math.max(0, Math.min(1, Number(options.morphProgress))) : 1;
  const animation = createMiniatureAnimationPreset("coordinate-morph", {
    loop,
    progress,
    direction: "alternate",
    durationMs: finiteNumber(options.morphDurationMs, 4800),
    easing: "smoothstep",
  });
  if (loop && !hasProgress) delete animation.progress;
  const view = MorphView.fromVisualSpace(document, {
    datasetId: inputs.datasetId,
    sourceCoordinateId: inputs.sourceCoordinate.id,
    targetCoordinateId: inputs.targetCoordinate.id,
    colorPropertyId: inputs.labelProperty.id,
    size: finiteNumber(options.pointSize, 1.7),
    alpha: finiteNumber(options.pointAlpha, 0.72),
    shape: "disc",
    material: {
      pointPixelScale: 12,
      alphaMode: "blend",
      transparent: true,
      alpha: 0.72,
      focusBoost: 0.1,
      phasePulse: 0.04,
      ...(options.pointMaterial || {}),
    },
    animation,
  });
  return view.toLayerDescriptors()[0];
}

function createProcessCurveProjectionDescriptor(document, inputs, options) {
  const view = GroundProjectionView.fromVisualSpace(document, {
    datasetId: inputs.datasetId,
    coordinateId: inputs.targetCoordinate.id,
    propertyId: inputs.labelProperty.id,
    groundY: finiteNumber(options.groundY, -0.52),
    size: finiteNumber(options.projectionSize, 2.65),
    alpha: finiteNumber(options.projectionAlpha, 0.34),
    material: {
      transparent: true,
      ...(options.projectionMaterial || {}),
    },
  });
  const descriptor = view.toLayerDescriptors()[0];
  descriptor.order = -28;
  return descriptor;
}

function createProcessCurvePropertyFieldDescriptor(document, inputs, options) {
  const property = resolveProcessCurveScalarProperty(document, inputs.datasetId, options);
  if (!property) return null;
  const view = PropertyFieldView.fromVisualSpace(document, {
    datasetId: inputs.datasetId,
    coordinateId: inputs.targetCoordinate.id,
    propertyId: property.id,
    groundY: finiteNumber(options.groundY, -0.53),
    alpha: finiteNumber(options.propertyFieldAlpha, options.fieldAlpha, 0.34),
    radius: finiteNumber(options.propertyFieldRadius, options.fieldRadius, 0.22),
    fieldMode: options.propertyFieldMode || options.fieldMode || "ground",
    order: -22,
    material: {
      contour: 0.14,
      glow: 0.08,
      depthWrite: false,
      ...(options.propertyFieldMaterial || options.fieldMaterial || {}),
    },
    metadata: {
      role: "semantic-ground-field",
      visualGrammar: "process-curves",
      processCurveField: {
        source: "PropertyFieldView",
        propertyId: property.id,
      },
    },
  });
  const descriptor = view.toLayerDescriptors()[0];
  if (!descriptor) return null;
  descriptor.order = -22;
  descriptor.source = {
    ...(descriptor.source || {}),
    processCurveViewClass: "ProcessCurveSceneView",
    processCurveFieldPropertyId: property.id,
  };
  descriptor.metadata = {
    ...(descriptor.metadata || {}),
    role: "property-field",
    processCurveField: {
      source: "PropertyFieldView",
      propertyId: property.id,
      propertyName: property.name || property.id,
    },
    visualGrammar: "process-curves",
  };
  return descriptor;
}

function createProcessCurveStateFieldDescriptor(document, inputs, options) {
  const records = inputs.records;
  const positionsByRecord = new Map((inputs.targetCoordinate.record_positions || []).map((entry) => [entry.record_id, entry.position || []]));
  const labelByRecord = new Map((inputs.labelProperty.values || []).map((entry) => [entry.record_id, entry.value]));
  const positions = new Map();
  const values = new Map();
  const recordIds = [];
  const groundY = finiteNumber(options.groundY, -0.53);

  for (const record of records) {
    const position = positionsByRecord.get(record.id);
    if (!position) continue;
    recordIds.push(record.id);
    positions.set(record.id, [
      Number(position[0]) || 0,
      groundY,
      Number(position[2] ?? position[1]) || 0,
    ]);
    values.set(record.id, labelValue(labelByRecord.get(record.id) ?? record.label));
  }

  const view = new DenseFieldView({
    id: `field:${inputs.datasetId}:state-density`,
    name: "state-density-field",
    datasetId: inputs.datasetId,
    coordinateId: inputs.targetCoordinate.id,
    propertyId: inputs.labelProperty.id,
    recordIds,
    positions,
    values,
    radius: finiteNumber(options.fieldRadius, 1.85),
    alpha: finiteNumber(options.fieldAlpha, 0.54),
    material: {
      alpha: 0.72,
      pointPixelScale: 54,
      maxPointSize: 440,
      contour: 0.22,
      glow: 0.2,
      depthWrite: false,
      ...(options.fieldMaterial || {}),
    },
    metadata: {
      role: "semantic-ground-field",
      visualizes: "record-state intensity over metric coordinates",
    },
  });
  const descriptor = view.toLayerDescriptors()[0];
  descriptor.order = -22;
  return descriptor;
}

function createProcessCurveTrackDescriptor(document, inputs, options) {
  const graphTrack = createProcessCurveGraphTrackDescriptor(document, inputs, options);
  if (graphTrack) return graphTrack;

  const records = inputs.records
    .slice()
    .sort((a, b) => Number(a.payload?.source_index ?? 0) - Number(b.payload?.source_index ?? 0));
  const positions = new Map((inputs.targetCoordinate.record_positions || []).map((entry) => [entry.record_id, entry.position || []]));
  const labels = new Map((inputs.labelProperty.values || []).map((entry) => [entry.record_id, entry.value]));
  const categories = new Map();
  const pathsByLabel = new Map();
  const trackRecordIds = [];
  const groundY = finiteNumber(options.groundY, -0.5);

  for (const record of records) {
    const position = positions.get(record.id);
    if (!position) continue;
    const label = labels.get(record.id) ?? record.label ?? record.id;
    if (!categories.has(label)) categories.set(label, categories.size);
    if (!pathsByLabel.has(label)) {
      pathsByLabel.set(label, {
        id: `track:${inputs.datasetId}:${label}`,
        label,
        color: trackColor(categories.get(label)),
        points: [],
        recordIds: [],
      });
    }
    trackRecordIds.push(record.id);
    pathsByLabel.get(label).recordIds.push(record.id);
    pathsByLabel.get(label).points.push([
      Number(position[0]) || 0,
      groundY,
      Number(position[2] ?? position[1]) || 0,
    ]);
  }

  const view = new TrajectoryPathView({
    id: `track:${inputs.datasetId}:record-order`,
    kind: "record-track",
    descriptorKind: "record-track",
    sourceViewKind: "record-track",
    datasetId: inputs.datasetId,
    coordinateId: inputs.targetCoordinate.id,
    propertyId: inputs.labelProperty.id,
    recordIds: trackRecordIds,
    paths: Array.from(pathsByLabel.values()),
    pathSource: "record-order-layout",
    order: -8,
    mode: options.trackMode || "ribbon",
    width: finiteNumber(options.trackWidth, 3.15),
    alpha: finiteNumber(options.trackAlpha, 0.72),
    descriptorFactory: "tube-ribbon",
    nativeEvidence: {
      schema: "metric.visual.trajectory_path_evidence_ref.v1",
      source: "record-order-layout",
      documentSchema: document?.schema || null,
      provenance: document?.provenance ? {
        writer: document.provenance.writer || null,
        runtime: document.provenance.runtime || null,
        source_example: document.provenance.source_example || null,
        synthetic: document.provenance.synthetic === true,
      } : null,
      datasetId: inputs.datasetId,
      spaceId: inputs.targetCoordinate.space_id || null,
      coordinateId: inputs.targetCoordinate.id,
      relationId: null,
      graphId: null,
      timelineId: null,
    },
    curveOptions: {
      radius: finiteNumber(options.trackRadius, 0.018),
      radialSegments: Math.max(3, Math.floor(finiteNumber(options.trackRadialSegments, 8))),
      useWidthsAsRadius: options.trackUseWidthsAsRadius === true,
      emission: finiteNumber(options.trackEmission, 0.14),
      rimLight: finiteNumber(options.trackRimLight, 0.36),
      coreGlow: finiteNumber(options.trackCoreGlow, 0.2),
      flowStrength: finiteNumber(options.trackFlowStrength, 0.1),
      depthWrite: options.trackMode === "tube" ? true : false,
      resample: false,
    },
    metadata: {
      visualGrammar: "process-curves",
    },
  });
  return view.toLayerDescriptors()[0];
}

function createProcessCurveGraphTrackDescriptor(document, inputs, options) {
  const requestedGraphTrack = options.useGraphTrajectory === true
    || options.preferGraphTrajectory === true
    || options.graphId != null
    || options.pathGraphId != null
    || options.relationId != null
    || options.pathRelationId != null
    || options.transitionRelationId != null;
  if (!requestedGraphTrack) return null;
  const relation = resolveProcessCurveRelation(document, inputs.datasetId, options);
  const graph = resolveProcessCurveGraph(document, inputs.datasetId, relation, options);
  if (!graph && !relation) return null;
  const view = TrajectoryPathView.fromVisualSpace(document, {
    datasetId: inputs.datasetId,
    coordinateId: inputs.targetCoordinate.id,
    relationId: relation?.id,
    graphId: graph?.id,
    preferGraph: true,
    pathSource: graph ? "graph" : "transition",
    id: `track:${inputs.datasetId}:graph-trajectory`,
    kind: "record-track",
    descriptorKind: "record-track",
    sourceViewKind: "record-track",
    mode: options.trackMode || "ribbon",
    width: finiteNumber(options.trackWidth, 3.15),
    alpha: finiteNumber(options.trackAlpha, 0.72),
    color: options.trackColor || [0.12, 0.36, 0.42, finiteNumber(options.trackAlpha, 0.72)],
    order: -8,
    edgeMode: options.trackEdgeMode || "auto",
    curveOptions: {
      radius: finiteNumber(options.trackRadius, 0.018),
      radialSegments: Math.max(3, Math.floor(finiteNumber(options.trackRadialSegments, 8))),
      useWidthsAsRadius: options.trackUseWidthsAsRadius === true,
      emission: finiteNumber(options.trackEmission, 0.14),
      rimLight: finiteNumber(options.trackRimLight, 0.36),
      coreGlow: finiteNumber(options.trackCoreGlow, 0.2),
      flowStrength: finiteNumber(options.trackFlowStrength, 0.1),
      depthWrite: options.trackMode === "tube" ? true : false,
      resample: false,
      ...(options.trackCurveOptions || {}),
    },
    metadata: {
      visualGrammar: "process-curves",
    },
  });
  return view.toLayerDescriptors()[0];
}

function createProcessCurveLabelDescriptor(document, inputs, options) {
  if (options.labels === false || options.showLabels === false) return null;
  const positionsByRecord = new Map((inputs.targetCoordinate.record_positions || []).map((entry) => [entry.record_id, entry.position || []]));
  const labelsByRecord = new Map((inputs.labelProperty.values || []).map((entry) => [entry.record_id, entry.value]));
  const groups = new Map();
  for (const record of inputs.records) {
    const position = positionsByRecord.get(record.id);
    if (!position) continue;
    const label = labelsByRecord.get(record.id) ?? record.label;
    const key = String(label || "").trim();
    if (!key) continue;
    if (!groups.has(key)) groups.set(key, { count: 0, x: 0, y: 0, z: 0 });
    const group = groups.get(key);
    group.count += 1;
    group.x += Number(position[0]) || 0;
    group.y += Number(position[1]) || 0;
    group.z += Number(position[2] ?? position[1]) || 0;
  }
  const labelEntries = Array.from(groups.entries())
    .filter(([, group]) => group.count > 0)
    .sort(([a], [b]) => String(a).localeCompare(String(b)));
  if (!labelEntries.length) return null;
  const labelLift = finiteNumber(options.labelLift, 0.42);
  const fontSize = Math.max(14, Math.floor(finiteNumber(options.labelFontSize, 22)));
  const color = options.labelColor || [0.12, 0.16, 0.18, 0.92];
  const labels = labelEntries.map(([category, group], index) => {
    const offset = labelOffset(index, labelEntries.length, finiteNumber(options.labelOffsetRadius, 0.14));
    return {
      id: `label:${inputs.datasetId}:${category}`,
      text: labelText(category, options.labelMap),
      position: [
        group.x / group.count + offset[0],
        group.y / group.count + labelLift,
        group.z / group.count + offset[1],
      ],
      color,
      background: options.labelBackground || [1, 1, 1, 0.72],
      border: options.labelBorder || [0.1, 0.14, 0.16, 0.12],
      size: finiteNumber(options.labelSize, 0.2),
    };
  });
  return {
    schema: "metric.visual.layer_descriptor.v1",
    id: `labels:${inputs.datasetId}:regimes`,
    kind: "labels",
    primitive: "BillboardLabelLayer",
    visible: true,
    order: 62,
    labels,
    geometry: { fontSize },
    material: { fontSize },
    source: {
      viewKind: "process-curves",
      viewClass: "ProcessCurveSceneView",
      datasetId: inputs.datasetId,
      coordinateId: inputs.targetCoordinate.id,
      propertyId: inputs.labelProperty.id,
    },
    metadata: {
      role: "region-labels",
      visualGrammar: "process-curves",
      viewClass: "ProcessCurveSceneView",
      labelCount: labels.length,
      propertyId: inputs.labelProperty.id,
      algorithmicComputation: false,
    },
  };
}

function createProcessCurveSkylineDescriptor(document, inputs, options) {
  const records = inputs.records
    .slice()
    .sort((a, b) => Number(a.payload?.source_index ?? 0) - Number(b.payload?.source_index ?? 0));
  const positionsByRecord = new Map((inputs.targetCoordinate.record_positions || []).map((entry) => [entry.record_id, entry.position || []]));
  const labelByRecord = new Map((inputs.labelProperty.values || []).map((entry) => [entry.record_id, entry.value]));
  const entries = [];

  for (const record of records) {
    const position = positionsByRecord.get(record.id);
    const values = curveValues(record);
    if (!position || values.length < 2) continue;
    entries.push({
      id: record.id,
      label: labelByRecord.get(record.id) ?? record.label,
      position,
      energy: curveEnergy(values),
      amplitude: curveAmplitude(values),
    });
  }

  const energyRange = extent(entries.map((entry) => entry.energy));
  const amplitudeRange = extent(entries.map((entry) => entry.amplitude));
  const positions = new Float32Array(entries.length * 3);
  const sizes = new Float32Array(entries.length * 2);
  const heights = new Float32Array(entries.length);
  const colors = new Float32Array(entries.length * 4);
  const phases = new Float32Array(entries.length);
  const focus = new Float32Array(entries.length);
  const groundY = finiteNumber(options.groundY, -0.56);

  for (let index = 0; index < entries.length; index += 1) {
    const entry = entries[index];
    const x = Number(entry.position[0]) || 0;
    const z = Number(entry.position[2] ?? entry.position[1]) || 0;
    const energy = normalize(entry.energy, energyRange);
    const amplitude = normalize(entry.amplitude, amplitudeRange);
    const height = 0.09 + energy * 0.44 + amplitude * 0.22;
    const footprint = 0.038 + Math.sqrt(Math.max(0, amplitude)) * 0.045;
    const color = stateColor(entry.label, 0.96);

    positions.set([x, groundY, z], index * 3);
    sizes.set([footprint, footprint * 1.15], index * 2);
    heights[index] = height;
    colors.set(color, index * 4);
    phases[index] = (index % 11) / 11;
    focus[index] = String(entry.label || "").toLowerCase().includes("anomaly") ? 1 : 0.22;
  }

  return {
    schema: "metric.visual.layer_descriptor.v1",
    id: `skyline:${inputs.datasetId}:curve-energy`,
    kind: "record-skyline",
    primitive: "InstancedBoxLayer",
    visible: true,
    order: -4,
    channels: {
      position: createChannel(positions, 3, "record-position"),
      size: createChannel(sizes, 2, "record-footprint"),
      height: createChannel(heights, 1, "curve-energy-height"),
      color: createChannel(colors, 4, "state-color"),
      animationPhase: createChannel(phases, 1, "animation-phase"),
      focusWeight: createChannel(focus, 1, "state-focus"),
    },
    geometry: {
      anchor: "base",
      instanceCount: entries.length,
    },
    material: {
      family: "mesh",
      lighting: "phong-like",
      alpha: 1,
      alphaMode: "opaque",
      depthWrite: true,
    },
    animation: {
      mode: "attention",
    },
    picking: {
      mode: "record-id",
    },
    metadata: {
      role: "miniature-record-volume",
      visualizes: "record curve energy as grounded miniature volume",
      source: "process-curve C++ evidence",
    },
  };
}

function firstDatasetId(document) {
  const dataset = document?.datasets?.[0]?.id;
  if (dataset) return dataset;
  return document?.records?.find((record) => record.dataset_id)?.dataset_id || null;
}

function resolveByIdOrPredicate(items, id, predicate) {
  if (id && typeof id === "object") return id;
  if (id) {
    const byId = items.find((entry) => entry.id === id || entry.name === id);
    if (byId) return byId;
  }
  return items.find(predicate);
}

function resolveProcessCurveRelation(document, datasetId, options = {}) {
  const relationRef = options.relationId || options.relation;
  const relations = Array.isArray(document?.relations) ? document.relations : [];
  if (relationRef && typeof relationRef === "object") return relationRef;
  if (relationRef) {
    const relation = relations.find((entry) => String(entry.id ?? entry.name) === String(relationRef));
    if (!relation) throw new Error(`Unknown process-curve relation reference: ${String(relationRef)}`);
    return relation;
  }
  return relations.find((entry) => (
    matchesDataset(entry, datasetId) &&
    /process|curve|aligned|metric/i.test(`${entry.id || ""} ${entry.name || ""} ${entry.relation_type || ""}`)
  )) || relations.find((entry) => matchesDataset(entry, datasetId)) || null;
}

function resolveProcessCurveGraph(document, datasetId, relation, options = {}) {
  const graphRef = options.graphId || options.graph;
  const graphs = Array.isArray(document?.graphs) ? document.graphs : [];
  if (graphRef && typeof graphRef === "object") return graphRef;
  if (graphRef) {
    const graph = graphs.find((entry) => String(entry.id ?? entry.name) === String(graphRef));
    if (!graph) throw new Error(`Unknown process-curve graph reference: ${String(graphRef)}`);
    return graph;
  }
  const relationId = relation?.id;
  return graphs.find((entry) => (
    relationId != null &&
    String(entry.edge_relation_id ?? entry.edgeRelationId ?? entry.relation_id ?? entry.relationId) === String(relationId)
  )) || graphs.find((entry) => matchesDataset(entry, datasetId)) || null;
}

function resolveProcessCurveScalarProperty(document, datasetId, options = {}) {
  const propertyRef = options.propertyFieldId
    || options.fieldPropertyId
    || options.scalarPropertyId
    || options.scalarProperty
    || (typeof options.propertyField === "string" ? options.propertyField : null)
    || (typeof options.groundField === "string" ? options.groundField : null)
    || null;
  const properties = Array.isArray(document?.properties) ? document.properties : [];
  const isScalar = (entry) => {
    const valueType = entry?.value_type ?? entry?.valueType;
    return valueType === "scalar" || valueType === "rank" || valueType === "number";
  };
  if (propertyRef) {
    const property = properties.find((entry) => (
      String(entry.id ?? entry.name) === String(propertyRef) ||
      String(entry.name ?? entry.id) === String(propertyRef)
    ));
    if (!property || !isScalar(property)) return null;
    return property;
  }
  const preferred = ["local-density", "density", "metric-anomaly-severity", "anomaly", "entropy"];
  for (const needle of preferred) {
    const property = properties.find((entry) => (
      matchesDataset(entry, datasetId) &&
      isScalar(entry) &&
      `${entry.id || ""} ${entry.name || ""}`.toLowerCase().includes(needle)
    ));
    if (property) return property;
  }
  return properties.find((entry) => matchesDataset(entry, datasetId) && isScalar(entry)) || null;
}

function matchesDataset(entry, datasetId) {
  if (datasetId == null) return true;
  const entryDataset = entry?.dataset_id ?? entry?.datasetId;
  return entryDataset == null || String(entryDataset) === String(datasetId);
}

function processCurvePreviewPositions(coordinate, recordIds = []) {
  const positionsByRecord = new Map((coordinate?.record_positions || []).map((entry) => [String(entry.record_id), entry.position || []]));
  const positions = new Map();
  for (const recordId of recordIds) {
    const position = positionsByRecord.get(String(recordId));
    if (!position) continue;
    positions.set(String(recordId), [
      Number(position[0]) || 0,
      Number(position[1]) || 0,
      Number(position[2] ?? position[1]) || 0,
    ]);
  }
  return positions;
}

function trackColor(index) {
  const colors = [
    [0.12, 0.44, 0.66, 0.72],
    [0.82, 0.42, 0.14, 0.72],
    [0.16, 0.54, 0.48, 0.68],
    [0.52, 0.42, 0.62, 0.68],
  ];
  return colors[Math.abs(index) % colors.length];
}

function labelText(category, labelMap) {
  if (labelMap && typeof labelMap === "object" && Object.prototype.hasOwnProperty.call(labelMap, category)) {
    return String(labelMap[category]);
  }
  return String(category);
}

function labelOffset(index, count, radius) {
  if (!radius || count <= 1) return [0, 0];
  const angle = (index / Math.max(1, count)) * Math.PI * 2;
  return [Math.cos(angle) * radius, Math.sin(angle) * radius];
}

function labelValue(label) {
  const normalized = String(label || "").toLowerCase();
  if (normalized.includes("anomaly")) return 0.96;
  if (normalized.includes("recovery")) return 0.68;
  if (normalized.includes("pre")) return 0.48;
  return 0.16;
}

function curveEnergy(values) {
  let total = 0;
  for (let index = 1; index < values.length; index += 1) {
    const delta = Number(values[index]) - Number(values[index - 1]);
    if (Number.isFinite(delta)) total += delta * delta;
  }
  return Math.sqrt(total / Math.max(1, values.length - 1));
}

function curveValues(record) {
  if (Array.isArray(record?.payload?.values)) return record.payload.values;
  if (Array.isArray(record?.payload?.series)) return record.payload.series;
  if (Array.isArray(record?.values)) return record.values;
  return [];
}

function curveAmplitude(values) {
  let min = Infinity;
  let max = -Infinity;
  for (const value of values) {
    const number = Number(value);
    if (!Number.isFinite(number)) continue;
    min = Math.min(min, number);
    max = Math.max(max, number);
  }
  return Number.isFinite(min) && Number.isFinite(max) ? max - min : 0;
}

function extent(values) {
  let min = Infinity;
  let max = -Infinity;
  for (const value of values) {
    const number = Number(value);
    if (!Number.isFinite(number)) continue;
    min = Math.min(min, number);
    max = Math.max(max, number);
  }
  return Number.isFinite(min) && Number.isFinite(max) ? [min, max] : [0, 1];
}

function normalize(value, range) {
  const span = Math.max(1e-9, Number(range?.[1]) - Number(range?.[0]));
  return Math.max(0, Math.min(1, (Number(value) - Number(range?.[0] || 0)) / span));
}

function stateColor(label, alpha = 1) {
  const normalized = String(label || "").toLowerCase();
  if (normalized.includes("anomaly")) return [0.85, 0.38, 0.14, alpha];
  if (normalized.includes("recovery")) return [0.42, 0.36, 0.65, alpha];
  if (normalized.includes("pre")) return [0.22, 0.58, 0.46, alpha];
  return [0.18, 0.48, 0.67, alpha];
}

function finiteNumber(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number)) return number;
  }
  return 0;
}
