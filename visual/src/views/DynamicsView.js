import { BaseView } from "./BaseView.js";
import { normalizePathEvidence } from "../curves/index.js";
import { PointCloudView } from "./PointCloudView.js";
import {
  VISUAL_LAYER_HIERARCHY_BANDS,
  TrajectoryPathView,
  createVisualLayerHierarchy,
} from "./TrajectoryPathView.js";
import { VisualLayer } from "./VisualLayer.js";
import {
  createTimelineAnimationDescriptor,
  createTimelineControlDescriptor,
  createTimelineEvidenceDescriptor,
  createTimelineModel,
  sampleTimelineState,
} from "../timeline/index.js";
import { applyPositionFit, computePositionFit } from "./scene-fit.js";
import {
  colorChannelFrom,
  createChannel,
  createStringChannel,
  extractCoordinatePositions,
  extractPropertyValues,
  flattenValues,
  flattenVectors,
  inferScalarDomain,
  recordsFor,
  resolveCollectionItem,
} from "./view-utils.js";

/**
 * DynamicsView renders dynamics over a finite metric space: record states that
 * move between exported coordinate states (diffusion, reverse flow, denoise,
 * resampling). It draws the active state as a point cloud and the full
 * trajectory of each record across timeline steps as faded path segments. It
 * interpolates only between already-exported states.
 */
export class DynamicsView extends BaseView {
  constructor(options = {}) {
    super({ ...options, kind: "dynamics" });
    this.recordIds = options.recordIds || [];
    this.stepStates = options.stepStates || []; // [{ name, positions: Map, scalarValues?: Map }]
    this.timelineId = options.timelineId || options.timeline_id || options.metadata?.timelineId || null;
    this.timelineState = options.timelineState || null;
    this.timelineControl = options.timelineControl || null;
    this.timelineFieldState = options.timelineFieldState || selectTimelineFieldState(this.timelineState, options);
    this.nativeEvidence = options.nativeEvidence || options.metadata?.nativeEvidence || null;
    this.trajectoryPathEvidence = options.trajectoryPathEvidence || options.pathEvidence || options.metadata?.trajectoryPathEvidence || null;
    this.trajectoryPaths = normalizeDynamicsPathRecords(
      options.trajectoryPaths || options.paths || options.trajectories || this.trajectoryPathEvidence?.paths || [],
      this.trajectoryPathEvidence,
    );
    const sampledStep = Number.isFinite(Number(this.timelineState?.activeStepOrder))
      ? Number(this.timelineState.activeStepOrder)
      : 0;
    this.activeStep = clampInt(options.activeStep ?? options.stateStep ?? sampledStep, 0, Math.max(0, this.stepStates.length - 1));
    this.groundY = Number.isFinite(Number(options.groundY)) ? Number(options.groundY) : 0;
    this.targetRadius = Number.isFinite(Number(options.targetRadius)) ? Number(options.targetRadius) : 1.6;
    this.size = Number.isFinite(Number(options.size)) ? Number(options.size) : 1;
    this.showTrajectory = options.trajectory !== false;
    this.colorValues = options.colorValues;
    this.timelineAnimation = options.timelineAnimation || null;
    this.timelineEvidence = options.timelineEvidence || null;
    this.motionTargetStep = clampInt(
      options.motionTargetStep ?? options.targetStep ?? Math.max(0, this.stepStates.length - 1),
      0,
      Math.max(0, this.stepStates.length - 1),
    );
    this.motionProgress = Number.isFinite(Number(options.progress ?? options.flowProgress))
      ? Math.max(0, Math.min(1, Number(options.progress ?? options.flowProgress)))
      : null;
    this.pathWidth = Number.isFinite(Number(options.pathWidth)) ? Number(options.pathWidth) : 3.2;
    this.pathAlpha = Number.isFinite(Number(options.pathAlpha)) ? Number(options.pathAlpha) : 0.66;
    this.pathColorValues = options.pathColorValues || options.trajectoryPathColorValues || null;
    this.pathWidthValues = options.pathWidthValues || options.trajectoryPathWidthValues || null;
    this.pathColorPropertyId = options.pathColorPropertyId
      || options.pathColorProperty
      || options.trajectoryPathColorPropertyId
      || options.trajectoryPathColorProperty
      || null;
    this.pathWidthPropertyId = options.pathWidthPropertyId
      || options.pathWidthProperty
      || options.trajectoryPathWidthPropertyId
      || options.trajectoryPathWidthProperty
      || this.pathColorPropertyId
      || null;
    this.pathColorRamp = options.pathColorRamp || options.trajectoryPathColorRamp || "residual";
    this.pathColorDomain = options.pathColorDomain || options.trajectoryPathColorDomain || scalarDomainForValues(this.pathColorValues, this.recordIds);
    this.pathWidthDomain = options.pathWidthDomain || options.trajectoryPathWidthDomain || scalarDomainForValues(this.pathWidthValues || this.pathColorValues, this.recordIds);
    this.pathWidthRange = normalizeNumberRange(options.pathWidthRange || options.trajectoryPathWidthRange, [
      Math.max(1, this.pathWidth * 0.56),
      Math.max(1, this.pathWidth * 1.32),
    ]);
    this.trajectoryPathLimit = Math.max(0, Math.floor(Number(
      options.trajectoryPathLimit
        ?? options.pathLimit
        ?? options.trajectoryLimit
        ?? 0,
    ) || 0));
    this.trajectoryPathSelection = options.trajectoryPathSelection
      || options.pathSelection
      || (this.trajectoryPathLimit > 0 ? "deterministic-record-stride" : "all-exported-records");
    this.timelineContextEnabled = options.timelineContext === true
      || options.stateHistory === true
      || options.timelineStateHistory === true
      || options.preview === "state-history";
    this.timelineContextStops = normalizeTimelineContextStops(
      options.timelineContextStops || options.stateHistoryStops || options.timelineStateStops || this.timelineControl?.samples,
      this.stepStates.length,
    );
    this.timelineContextAlpha = Number.isFinite(Number(options.timelineContextAlpha))
      ? Number(options.timelineContextAlpha)
      : 0.2;
    this.timelineContextPointSize = Number.isFinite(Number(options.timelineContextPointSize))
      ? Number(options.timelineContextPointSize)
      : Math.max(0.7, this.size * 0.68);
    this.pointMaterial = {
      pointPixelScale: 10,
      minPointSize: 2,
      maxPointSize: 32,
      alphaMode: "blend",
      transparent: true,
      ambient: 0.48,
      pointLight: 0.54,
      sphereShade: 0.82,
      gloss: 0.44,
      specular: 0.52,
      specularPower: 38,
      edgeShade: 0.46,
      saturation: 1.12,
      ...(options.pointMaterial || {}),
    };
    this.timelineContextMaterial = {
      pointPixelScale: 8,
      minPointSize: 1.4,
      maxPointSize: 18,
      alphaMode: "blend",
      transparent: true,
      ambient: 0.58,
      pointLight: 0.32,
      sphereShade: 0.58,
      edgeShade: 0.38,
      saturation: 0.9,
      depthWrite: false,
      ...(options.timelineContextMaterial || options.stateHistoryMaterial || {}),
    };
    this.fieldEnabled = options.field !== false
      && options.propertyField !== false
      && options.groundField !== false
      && Boolean(
        options.field === true
        || options.propertyField
        || options.groundField
        || options.scalarProperty
        || this.timelineFieldState
        || this.stepStates.some((state) => state?.scalarValues),
      );
    this.fieldMode = options.fieldMode || options.fieldGrammar || "ground";
    this.fieldAlpha = Number.isFinite(Number(options.fieldAlpha ?? options.alpha))
      ? Number(options.fieldAlpha ?? options.alpha)
      : 0.3;
    this.fieldRadius = Number.isFinite(Number(options.fieldRadius ?? options.radius))
      ? Number(options.fieldRadius ?? options.radius)
      : 0.22;
    this.fieldLift = Number.isFinite(Number(options.fieldLift)) ? Number(options.fieldLift) : 0.24;
    this.fieldMaterial = options.fieldMaterial || options.material || {};
    this.fieldPropertyId = options.fieldPropertyId
      || options.propertyField
      || options.groundField
      || options.scalarProperty
      || options.scalarPropertyId
      || null;

    // Shared fit across all steps so trajectories stay registered.
    const combined = combinePositions(this.stepStates, this.recordIds);
    this.fit = options.fit === false
      ? null
      : computePositionFit(combined.source, combined.ids, { targetRadius: this.targetRadius, groundY: this.groundY });
    this.fittedStates = this.stepStates.map((state) => ({
      ...state,
      positions: this.fit ? applyPositionFit(state.positions, this.recordIds, this.fit.transform) : state.positions,
    }));
  }

  static fromVisualSpace(document, options = {}) {
    const timeline = resolveCollectionItem(document, "timelines", options.timeline || options.timelineId)
      || firstTimeline(document);
    const timelinePropertySampling = resolveTimelinePropertySamplingOptions(options);
    const timelineModel = timeline
      ? createTimelineModel(document, {
        timelineId: timeline.id,
        loop: options.loop ?? true,
        playbackRate: options.playbackRate,
        easing: options.easing,
        ...timelinePropertySampling,
      })
      : null;
    const steps = timelineModel?.steps?.length
      ? timelineModel.steps
      : (Array.isArray(timeline?.steps) ? timeline.steps : []);
    const timelineSampling = timelineModel ? { ...resolveTimelineSamplingOptions(options), ...timelinePropertySampling } : {};
    const timelineState = timelineModel ? sampleTimelineState(timelineModel, timelineSampling) : null;
    const timelineControl = timelineModel
      ? createTimelineControlDescriptor(timelineModel, {
        ...timelineSampling,
        loop: options.loop ?? true,
        direction: options.direction || "alternate",
      })
      : null;
    const timelineAnimation = timelineModel
      ? createTimelineAnimationDescriptor(timelineModel, {
        mode: "timeline-coordinate-morph",
        loop: options.loop ?? true,
        direction: options.direction || "alternate",
        durationMs: options.timelineDurationMs ?? options.durationMs,
        stepDurationMs: options.stepDurationMs ?? 360,
        minDurationMs: options.minDurationMs ?? 6800,
        maxDurationMs: options.maxDurationMs ?? 18000,
      })
      : null;
    const timelineEvidence = timelineModel
      ? createTimelineEvidenceDescriptor(timelineModel, {
        ...timelineSampling,
        loop: options.loop ?? true,
        direction: options.direction || "alternate",
        state: timelineState,
        control: timelineControl,
        animation: timelineAnimation,
      })
      : null;
    const datasetId = options.datasetId ?? timeline?.dataset_id;
    const records = recordsFor(document, { ...options, datasetId });
    const explicitFieldPropertyRef = firstPropertyRef(
      options.fieldPropertyId,
      options.propertyField,
      options.groundField,
      options.scalarProperty,
      options.scalarPropertyId,
    );

    let recordIds = options.recordIds || [];
    const stepStates = steps.map((step) => {
      const sourceStep = step.source || step;
      const coordinateId = step.coordinateId ?? sourceStep.coordinate_id ?? sourceStep.coordinateId;
      const propertyId = step.propertyId ?? sourceStep.property_id ?? sourceStep.propertyId;
      const coordinate = resolveCollectionItem(document, "coordinates", coordinateId);
      const fieldPropertyId = propertyId || explicitFieldPropertyRef;
      const property = resolveCollectionItem(document, "properties", fieldPropertyId);
      const propertyTargetsRecords = isRecordTargetProperty(property);
      const positions = extractCoordinatePositions(coordinate, { records, recordIds: recordIds.length ? recordIds : undefined });
      if (!recordIds.length) recordIds = positions.ids;
      return {
        name: step.label || sourceStep.name || coordinate?.name || coordinate?.id,
        stepId: sourceStep.id || null,
        stepIndex: step.index ?? sourceStep.index ?? sourceStep.step_index ?? sourceStep.step ?? null,
        time: step.time ?? sourceStep.time ?? sourceStep.t ?? null,
        coordinateId: coordinate?.id,
        propertyId: property?.id,
        fieldPropertySource: property
          ? (propertyTargetsRecords ? (propertyId ? "timeline-step-record-property" : "selected-property") : "timeline-step-property")
          : null,
        fieldPropertyTargetType: property?.target_type ?? property?.targetType ?? null,
        positions: positions.positions,
        scalarValues: propertyTargetsRecords ? extractPropertyValues(property, { records, recordIds: positions.ids }) : null,
      };
    });
    const trajectoryPathEvidence = resolveDynamicsTrajectoryPathEvidence(document, timeline, options, datasetId);
    const nativeEvidence = createDynamicsTrajectoryEvidenceReference(document, timeline, stepStates, trajectoryPathEvidence);
    const pathColorPropertyRef = firstPropertyRef(
      options.pathColorPropertyId,
      options.pathColorProperty,
      options.trajectoryPathColorPropertyId,
      options.trajectoryPathColorProperty,
    );
    const pathWidthPropertyRef = firstPropertyRef(
      options.pathWidthPropertyId,
      options.pathWidthProperty,
      options.trajectoryPathWidthPropertyId,
      options.trajectoryPathWidthProperty,
    );
    const pathColorProperty = resolveCollectionItem(document, "properties", pathColorPropertyRef, {
      required: pathColorPropertyRef != null,
      label: "trajectory path color property",
    });
    const pathWidthProperty = resolveCollectionItem(document, "properties", pathWidthPropertyRef, {
      required: pathWidthPropertyRef != null,
      label: "trajectory path width property",
    });
    const pathColorValues = isRecordTargetProperty(pathColorProperty)
      ? extractPropertyValues(pathColorProperty, { records, recordIds })
      : options.pathColorValues || options.trajectoryPathColorValues || null;
    const pathWidthValues = isRecordTargetProperty(pathWidthProperty)
      ? extractPropertyValues(pathWidthProperty, { records, recordIds })
      : options.pathWidthValues || options.trajectoryPathWidthValues || null;

    return new DynamicsView({
      ...options,
      records,
      recordIds,
      stepStates,
      activeStep: options.activeStep ?? timelineState?.activeStepOrder,
      datasetId,
      spaceId: options.spaceId ?? timeline?.space_id,
      name: options.name || timeline?.name,
      timelineId: timeline?.id,
      timelineState,
      timelineControl,
      timelineFieldState: selectTimelineFieldState(timelineState, options),
      fieldPropertyId: explicitFieldPropertyRef,
      timelineAnimation,
      timelineEvidence,
      trajectoryPathEvidence,
      trajectoryPaths: trajectoryPathEvidence?.paths || options.trajectoryPaths || options.paths || options.trajectories,
      pathColorValues,
      pathWidthValues,
      pathColorPropertyId: pathColorProperty?.id || options.pathColorPropertyId || options.pathColorProperty || null,
      pathWidthPropertyId: pathWidthProperty?.id || options.pathWidthPropertyId || options.pathWidthProperty || null,
      nativeEvidence,
      metadata: {
        ...(options.metadata || {}),
        timelineId: timeline?.id,
        stepCount: stepStates.length,
        coordinateIds: stepStates.map((state) => state.coordinateId).filter(Boolean),
        timelineControl,
        timelineState,
        timelineEvidence,
        trajectoryPathEvidence,
        trajectoryPathEncoding: {
          colorPropertyId: pathColorProperty?.id || null,
          widthPropertyId: pathWidthProperty?.id || null,
          colorSource: pathColorProperty ? "exported-record-property" : null,
          widthSource: pathWidthProperty ? "exported-record-property" : null,
          algorithmicComputation: false,
        },
        nativeEvidence,
      },
    });
  }

  toLayerDescriptors() {
    const descriptors = [];
    if (this.showTrajectory && this.fittedStates.length > 1) {
      descriptors.push(this.trajectoryDescriptor());
    }
    const field = this.fieldDescriptor();
    if (field) descriptors.push(field);
    descriptors.push(...this.timelineContextDescriptors());
    const active = this.fittedStates[this.activeStep];
    if (active) {
      const target = this.motionTargetPositions();
      const currentStateHierarchy = createVisualLayerHierarchy({
        band: VISUAL_LAYER_HIERARCHY_BANDS.currentState,
        role: "current-timeline-state",
        viewClass: "DynamicsView",
        order: 0,
        drawsAbove: [
          VISUAL_LAYER_HIERARCHY_BANDS.supportField,
          VISUAL_LAYER_HIERARCHY_BANDS.stateHistoryContext,
          VISUAL_LAYER_HIERARCHY_BANDS.trajectoryPath,
        ],
        drawsBelow: [VISUAL_LAYER_HIERARCHY_BANDS.sceneLabels],
        depthPolicy: {
          depthTest: this.pointMaterial?.depthTest ?? null,
          depthWrite: this.pointMaterial?.depthWrite ?? null,
          depthBias: this.pointMaterial?.depthBias ?? null,
        },
        purpose: "keep the active exported timeline state above fields, history context and paths",
      });
      const point = new PointCloudView({
        id: `${this.id}:state`,
        datasetId: this.datasetId,
        spaceId: this.spaceId,
        records: this.records,
        recordIds: this.recordIds,
        positions: active.positions,
        targetPositions: target || undefined,
        scalarValues: active.scalarValues || undefined,
        colorValues: this.colorValues,
        size: this.size,
        alpha: 0.96,
        shape: "sphere",
        material: this.pointMaterial,
        animation: target ? this.stateAnimationDescriptor() : { mode: "none" },
        metadata: {
          ...this.metadata,
          ...this.timelineDescriptorMetadata(),
          role: "current-timeline-state",
          evidenceRole: "timeline-current-state",
          stateHistoryRole: "current-exported-timeline-state",
          visualComposition: "dynamics-current-state-over-trajectory-and-field",
          step: this.activeStep,
          activeCoordinateId: active.coordinateId,
          sampledCoordinateId: this.timelineState?.activeCoordinateId ?? active.coordinateId,
          activePropertyId: active.propertyId ?? this.timelineState?.activePropertyId ?? null,
          activeFieldPropertyId: this.activeFieldPropertyId(active),
          timelineFieldState: this.timelineFieldState,
          targetCoordinateId: target ? this.fittedStates[this.motionTargetStep]?.coordinateId : null,
          visualPriority: currentStateHierarchy,
          visualHierarchy: currentStateHierarchy,
          semanticRenderPriority: currentStateHierarchy.sortPriority,
        },
      });
      descriptors.push(...point.toLayerDescriptors());
    }
    return descriptors;
  }

  timelineDescriptorMetadata() {
    return {
      timelineId: this.timelineId,
      timelineEvidenceSchema: this.timelineEvidence?.schema || null,
      timelineEvidence: this.timelineEvidence,
      timelineStateSchema: this.timelineState?.schema || null,
      timelineControlSchema: this.timelineControl?.schema || null,
      timelineState: this.timelineState,
      timelineControl: this.timelineControl,
      timelineSamples: this.timelineControl?.samples || [],
      timelineFieldState: this.timelineFieldState,
    };
  }

  fieldDescriptor() {
    if (!this.fieldEnabled || !this.recordIds.length) return null;
    const active = this.fittedStates[this.activeStep];
    if (!active?.positions) return null;
    const ids = this.recordIds;
    const sourcePositions = flattenVectors(active.positions, ids, 3);
    const timelineScalarValue = Number(this.timelineFieldState?.value);
    const timelineScalar = Number.isFinite(timelineScalarValue)
      ? timelineScalarValue
      : 0;
    const scalarValues = active.scalarValues || scalarMapFromValue(ids, timelineScalar);
    const rawScalar = flattenValues(scalarValues, ids, timelineScalar);
    const domain = active.scalarValues
      ? inferScalarDomain(Array.from(rawScalar))
      : (this.timelineFieldState?.domain || inferScalarDomain(Array.from(rawScalar)));
    const fieldPositions = timelineFieldPositions(sourcePositions, rawScalar, domain, {
      mode: this.fieldMode,
      groundY: this.groundY,
      lift: this.fieldLift,
    });
    const timelineScalarChannel = new Float32Array(ids.length).fill(timelineScalar);
    const radius = new Float32Array(ids.length).fill(this.fieldRadius);
    const alpha = new Float32Array(ids.length).fill(this.fieldAlpha);
    const selection = new Float32Array(ids.length);
    const fieldStateSource = active.scalarValues ? active.fieldPropertySource || "selected-property" : "timeline-step-property";
    const fieldEvidence = this.dynamicsFieldEvidence(active, fieldStateSource, domain);
    const visualHierarchy = createVisualLayerHierarchy({
      band: VISUAL_LAYER_HIERARCHY_BANDS.supportField,
      role: "property-field",
      viewClass: "DynamicsView",
      order: -3,
      drawsBelow: [
        VISUAL_LAYER_HIERARCHY_BANDS.stateHistoryContext,
        VISUAL_LAYER_HIERARCHY_BANDS.trajectoryPath,
        VISUAL_LAYER_HIERARCHY_BANDS.currentState,
        VISUAL_LAYER_HIERARCHY_BANDS.sceneLabels,
      ],
      depthPolicy: {
        depthTest: this.fieldMaterial?.depthTest ?? null,
        depthWrite: this.fieldMaterial?.depthWrite ?? false,
        depthBias: this.fieldMaterial?.depthBias ?? null,
      },
      purpose: "keep exported propagation or uncertainty fields as support below dynamics paths and current state",
    });

    return new VisualLayer({
      id: `${this.id}:timeline-field`,
      kind: "timeline-property-field",
      primitive: "HeatFieldLayer",
      order: -3,
      source: {
        viewId: this.id,
        viewKind: "dynamics",
        role: "timeline-ground-field",
        timelineId: this.timelineId,
        coordinateId: active.coordinateId,
        propertyId: this.activeFieldPropertyId(active),
        scalarSource: fieldStateSource,
        nativeEvidence: fieldEvidence,
      },
      channels: {
        recordId: createStringChannel(ids, "record-id"),
        sourcePosition: createChannel(sourcePositions, 3, "source-position"),
        position: createChannel(fieldPositions, 3, "timeline-field-position"),
        scalar: createChannel(rawScalar, 1, "property-scalar", { domain }),
        timelineScalar: createChannel(timelineScalarChannel, 1, "timeline-step-scalar", {
          domain: this.timelineFieldState?.domain || { min: timelineScalar, max: timelineScalar },
        }),
        color: colorChannelFrom({
          ids,
          scalarValues,
          alpha: this.fieldAlpha,
          scalarDomain: domain,
        }),
        radius: createChannel(radius, 1, "influence-radius"),
        alpha: createChannel(alpha, 1, "alpha"),
        selection: createChannel(selection, 1, "selection-state"),
      },
      geometry: {
        mode: this.fieldMode === "lifted" ? "lifted-timeline-field" : "ground-timeline-field",
        plane: "xz",
        groundY: this.groundY,
        sampleCount: ids.length,
        interpolation: "renderer-defined",
        radius: this.fieldRadius,
      },
      material: {
        lighting: "field",
        diffuse: "scalar-ramp",
        alphaMode: "blend",
        alpha: this.fieldAlpha,
        contour: 0.12,
        glow: 0.08,
        depthWrite: false,
        ...(this.fieldMaterial || {}),
      },
      animation: {
        mode: "timeline-field-state",
        clock: this.timelineAnimation?.clock || "render-loop",
        timelineId: this.timelineId,
        evidence: this.timelineEvidence || undefined,
        control: this.timelineControl || undefined,
        state: this.timelineState || undefined,
      },
      picking: {
        mode: "record-id",
        channel: "recordId",
      },
      metadata: {
        ...this.metadata,
        ...this.timelineDescriptorMetadata(),
        role: "property-field",
        evidenceRole: "timeline-ground-field",
        fieldStateRole: "changing-ground-field-state",
        activeStep: this.activeStep,
        activeCoordinateId: active.coordinateId,
        activePropertyId: active.propertyId ?? this.timelineState?.activePropertyId ?? null,
        activeFieldPropertyId: this.activeFieldPropertyId(active),
        timelineFieldState: this.timelineFieldState,
        scalarDomain: domain,
        recordCount: ids.length,
        fieldMode: this.fieldMode === "lifted" ? "lifted" : "ground",
        fieldStateSource,
        nativeEvidence: fieldEvidence,
        previewLinked: true,
        selectionLinked: true,
        supportEvidenceRole: "propagation-or-uncertainty-field",
        fieldComposition: active.scalarValues
          ? "exported-record-property-shaped-by-exported-timeline-state"
          : "exported-timeline-step-scalar-field-state",
        visualPriority: visualHierarchy,
        visualHierarchy,
        semanticRenderPriority: visualHierarchy.sortPriority,
        algorithmicComputation: false,
      },
    }).toDescriptor();
  }

  trajectoryDescriptor() {
    const trajectoryPaths = this.fittedTrajectoryPaths();
    const selection = this.trajectorySelectionFor(trajectoryPaths);
    const selectedPaths = trajectoryPaths.length
      ? selectByIndex(trajectoryPaths, selection.indices)
      : [];
    const trajectorySource = trajectoryPaths.length
      ? this.trajectoryPathEvidence?.source || "exported-trajectory-path-evidence"
      : "exported-timeline-states";
    const [descriptor] = new TrajectoryPathView({
      id: `${this.id}:trajectory`,
      kind: "dynamics-trajectory",
      descriptorKind: "trajectory",
      sourceViewKind: "dynamics-trajectory",
      datasetId: this.datasetId,
      spaceId: this.spaceId,
      recordIds: trajectoryPaths.length ? this.recordIds : selection.recordIds,
      stepStates: this.fittedStates,
      paths: selectedPaths,
      pathSource: trajectorySource,
      width: this.pathWidth,
      alpha: this.pathAlpha,
      colorValues: this.pathColorValues,
      widthValues: this.pathWidthValues,
      pathColorPropertyId: this.pathColorPropertyId,
      pathWidthPropertyId: this.pathWidthPropertyId,
      pathColorRamp: this.pathColorRamp,
      pathColorDomain: this.pathColorDomain,
      pathWidthDomain: this.pathWidthDomain,
      pathWidthRange: this.pathWidthRange,
      colorMode: this.pathColorValues ? "property" : "timeline",
      order: 28,
      motionGrammar: "timeline-trajectory-state-history",
      nativeEvidence: this.nativeEvidence || {
        schema: "metric.visual.trajectory_path_evidence_ref.v1",
        source: "exported-timeline-states",
        documentSchema: null,
        provenance: null,
        datasetId: this.datasetId || null,
        spaceId: this.spaceId || null,
        coordinateId: null,
        relationId: null,
        graphId: null,
        timelineId: this.timelineId || null,
      },
      curveOptions: {
        alpha: 1,
        defaultWidth: this.pathWidth,
        flowStrength: 0.16,
        flowScale: 3.1,
        flowSpeed: 0.28,
        ambient: 0.42,
        pointLight: 0.54,
        coreGlow: 0.18,
        rimLight: 0.22,
        saturation: 1.08,
        depthWrite: false,
      },
      metadata: {
        ...this.metadata,
        ...this.timelineDescriptorMetadata(),
        stateHistoryRole: "timeline-state-history",
        motionGrammar: "timeline-trajectory-state-history",
        stepCount: this.fittedStates.length,
        timelineId: this.timelineId,
        timelineFieldState: this.timelineFieldState,
        coordinateIds: this.fittedStates.map((state) => state.coordinateId).filter(Boolean),
        trajectoryPathEvidence: this.trajectoryPathEvidence,
        trajectorySelection: selection.metadata,
        nativeTrajectoryCandidateCount: selection.candidateCount,
        selectedTrajectoryCount: selection.selectedCount,
        pathColorPropertyId: this.pathColorPropertyId,
        pathWidthPropertyId: this.pathWidthPropertyId,
        pathColorDomain: this.pathColorDomain,
        pathWidthDomain: this.pathWidthDomain,
        algorithmicComputation: false,
      },
    }).toLayerDescriptors();
    return descriptor;
  }

  timelineContextDescriptors() {
    if (!this.timelineContextEnabled || this.fittedStates.length < 2 || !this.recordIds.length) return [];
    const descriptors = [];
    const seen = new Set();
    for (let index = 0; index < this.timelineContextStops.length; index += 1) {
      const stop = this.timelineContextStops[index];
      const stepOrder = clampInt(stop.stepOrder, 0, Math.max(0, this.fittedStates.length - 1));
      if (stepOrder === this.activeStep || seen.has(stepOrder)) continue;
      const state = this.fittedStates[stepOrder];
      if (!state?.positions) continue;
      seen.add(stepOrder);
      const relation = stepOrder < this.activeStep ? "past" : "future";
      const distance = Math.abs(stepOrder - this.activeStep) / Math.max(1, this.fittedStates.length - 1);
      const alpha = Math.max(0.07, this.timelineContextAlpha * (relation === "past" ? 0.92 : 0.72) * (1 - distance * 0.38));
      const color = relation === "past"
        ? [0.1, 0.28, 0.43, alpha]
        : [0.75, 0.47, 0.18, alpha];
      const order = relation === "past" ? 5 : 4;
      const visualHierarchy = createVisualLayerHierarchy({
        band: VISUAL_LAYER_HIERARCHY_BANDS.stateHistoryContext,
        role: "timeline-state-history-context",
        viewClass: "DynamicsView",
        order,
        drawsAbove: [VISUAL_LAYER_HIERARCHY_BANDS.supportField],
        drawsBelow: [
          VISUAL_LAYER_HIERARCHY_BANDS.trajectoryPath,
          VISUAL_LAYER_HIERARCHY_BANDS.currentState,
          VISUAL_LAYER_HIERARCHY_BANDS.sceneLabels,
        ],
        depthPolicy: {
          depthTest: this.timelineContextMaterial?.depthTest ?? null,
          depthWrite: this.timelineContextMaterial?.depthWrite ?? false,
          depthBias: this.timelineContextMaterial?.depthBias ?? null,
        },
        purpose: "keep faded state-history context visible above fields but below primary paths and current state",
      });
      const point = new PointCloudView({
        id: `${this.id}:timeline-context:${stepOrder}`,
        datasetId: this.datasetId,
        spaceId: this.spaceId,
        coordinateId: state.coordinateId,
        records: this.records,
        recordIds: this.recordIds,
        positions: state.positions,
        colors: constantColorMap(this.recordIds, color),
        size: this.timelineContextPointSize,
        alpha,
        shape: "sphere",
        material: this.timelineContextMaterial,
        animation: { mode: "none" },
        metadata: {
          ...this.metadata,
          ...this.timelineDescriptorMetadata(),
          role: "timeline-state-history-context",
          evidenceRole: "timeline-state-history",
          stateHistoryRole: relation,
          timelineContextRole: relation,
          activeStep: this.activeStep,
          contextStep: stepOrder,
          contextCoordinateId: state.coordinateId,
          contextLabel: state.name || stop.label || null,
          contextNormalized: stop.normalized,
          recordCount: this.recordIds.length,
          nativeEvidence: this.nativeEvidence,
          visualPriority: visualHierarchy,
          visualHierarchy,
          semanticRenderPriority: visualHierarchy.sortPriority,
          algorithmicComputation: false,
        },
      }).toLayerDescriptors()[0];
      point.order = order;
      descriptors.push(point);
    }
    return descriptors;
  }

  setActiveStep(step) {
    this.activeStep = clampInt(step, 0, Math.max(0, this.fittedStates.length - 1));
    return this;
  }

  activeFieldPropertyId(active = this.fittedStates[this.activeStep]) {
    return active?.propertyId
      ?? this.fieldPropertyId
      ?? this.timelineFieldState?.propertyId
      ?? this.timelineState?.activePropertyId
      ?? null;
  }

  motionTargetPositions() {
    if (!this.fittedStates.length || this.motionTargetStep === this.activeStep) return null;
    return this.fittedStates[this.motionTargetStep]?.positions || null;
  }

  stateAnimationDescriptor() {
    const base = this.timelineAnimation || {
      schema: "metric.visual.timeline_animation.v1",
      mode: "timeline-coordinate-morph",
      clock: "render-loop",
      timelineId: this.timelineId,
      durationMs: 6800,
      loop: true,
      direction: "alternate",
      easing: "smoothstep",
      stepCount: this.fittedStates.length,
    };
    const animation = {
      ...base,
      channel: "targetPosition",
      requiresChannels: ["position", "targetPosition"],
    };
    if (this.timelineEvidence) animation.evidence = this.timelineEvidence;
    if (this.timelineControl) animation.control = this.timelineControl;
    if (this.timelineState) animation.state = this.timelineState;
    if (this.motionProgress != null) {
      animation.loop = false;
      animation.progress = this.motionProgress;
    }
    return animation;
  }

  fittedTrajectoryPaths() {
    if (!this.trajectoryPaths.length) return [];
    const normalized = normalizePathEvidence(this.trajectoryPaths, {
      defaultWidth: this.pathWidth,
      defaultColor: [0.14, 0.36, 0.46, this.pathAlpha],
    }).paths;
    if (!this.fit?.transform) return normalized;
    return normalized.map((path) => {
      const points = new Float32Array(path.points);
      for (let offset = 0; offset < points.length; offset += 3) {
        const fitted = this.fit.transform([points[offset], points[offset + 1], points[offset + 2]]);
        points[offset] = fitted[0];
        points[offset + 1] = fitted[1];
        points[offset + 2] = fitted[2];
      }
      return {
        ...path,
        points,
        metadata: {
          ...path.metadata,
          visualFit: {
            source: "DynamicsView.shared-coordinate-fit",
            evidenceMutation: false,
          },
        },
      };
    });
  }

  trajectorySelectionFor(trajectoryPaths = []) {
    const candidateCount = trajectoryPaths.length || this.recordIds.length;
    const limit = this.trajectoryPathLimit > 0
      ? Math.min(this.trajectoryPathLimit, candidateCount)
      : candidateCount;
    const indices = evenlySpacedIndices(candidateCount, limit);
    const recordIds = trajectoryPaths.length
      ? this.recordIds
      : selectByIndex(this.recordIds, indices);
    return {
      indices,
      recordIds,
      candidateCount,
      selectedCount: indices.length,
      metadata: {
        schema: "metric.visual.dynamics_trajectory_selection.v1",
        strategy: this.trajectoryPathSelection,
        candidateCount,
        selectedCount: indices.length,
        limit: this.trajectoryPathLimit || null,
        source: trajectoryPaths.length ? "exported-trajectory-paths" : "exported-timeline-states",
        evidenceMutation: false,
        algorithmicComputation: false,
      },
    };
  }

  dynamicsFieldEvidence(active, fieldStateSource, domain) {
    return {
      schema: "metric.visual.dynamics_field_evidence_ref.v1",
      source: active?.scalarValues ? "exported-record-property" : "exported-timeline-step-property",
      timelineId: this.timelineId || null,
      datasetId: this.datasetId || null,
      spaceId: this.spaceId || null,
      coordinateId: active?.coordinateId || null,
      propertyId: this.activeFieldPropertyId(active),
      timelinePropertyId: this.timelineFieldState?.propertyId || null,
      fieldStateSource,
      timelineStateSchema: this.timelineState?.schema || null,
      timelineEvidenceSchema: this.timelineEvidence?.schema || null,
      scalarDomain: domain,
      recordCount: this.recordIds.length,
      provenance: this.nativeEvidence?.provenance || null,
      algorithmicComputation: false,
    };
  }
}

export function createDynamicsView(options) {
  return new DynamicsView(options);
}

function combinePositions(stepStates, recordIds) {
  const source = new Map();
  const ids = recordIds.length ? recordIds.slice() : [];
  for (let stepIndex = 0; stepIndex < stepStates.length; stepIndex += 1) {
    const positions = stepStates[stepIndex].positions;
    const stepIds = recordIds.length ? recordIds : Array.from(positions.keys?.() || []);
    for (const id of stepIds) {
      const value = positions.get?.(id) ?? positions.get?.(String(id));
      if (value) source.set(`${id}@${stepIndex}`, value);
      if (!recordIds.length && !ids.includes(String(id))) ids.push(String(id));
    }
  }
  return { source, ids: Array.from(source.keys()) };
}

function firstTimeline(document) {
  const timelines = Array.isArray(document?.timelines) ? document.timelines : [];
  return timelines[0] || null;
}

function createDynamicsTrajectoryEvidenceReference(document, timeline, stepStates = [], trajectoryPathEvidence = null) {
  return {
    schema: "metric.visual.trajectory_path_evidence_ref.v1",
    source: trajectoryPathEvidence?.source || "exported-timeline-states",
    pathEvidenceId: trajectoryPathEvidence?.id || null,
    pathEvidenceCollection: trajectoryPathEvidence?.collection || null,
    pathCount: trajectoryPathEvidence?.pathCount ?? null,
    documentSchema: document?.schema || null,
    provenance: document?.provenance ? {
      writer: document.provenance.writer || null,
      runtime: document.provenance.runtime || null,
      source_example: document.provenance.source_example || null,
      native_export: document.provenance.native_export === true || document.provenance.nativeExport === true,
      synthetic: document.provenance.synthetic === true,
      synthetic_js: document.provenance.synthetic_js === true,
    } : null,
    datasetId: timeline?.dataset_id || null,
    spaceId: timeline?.space_id || null,
    coordinateId: null,
    coordinateIds: stepStates.map((state) => state.coordinateId).filter(Boolean),
    relationId: null,
    graphId: null,
    timelineId: timeline?.id || null,
  };
}

function resolveDynamicsTrajectoryPathEvidence(document, timeline, options = {}, datasetId = null) {
  const explicit = options.trajectoryPathEvidence
    || options.pathEvidence
    || options.trajectoryPaths
    || options.paths
    || options.trajectories
    || options.routes;
  if (explicit) {
    return createTrajectoryPathEvidence("provided-trajectory-path-evidence", explicit, {
      id: explicit.id,
      collection: "options",
      timelineId: timeline?.id || null,
      datasetId,
    });
  }

  for (const collection of ["trajectory_path_evidence", "trajectoryPaths", "path_evidence", "pathEvidence", "trajectories", "paths", "routes", "curves"]) {
    const value = document?.[collection];
    const entries = Array.isArray(value) ? value : (value ? [value] : []);
    if (!entries.length) continue;

    const matchedPaths = [];
    let evidenceId = null;
    for (const entry of entries) {
      if (!dynamicsPathEvidenceMatches(entry, timeline, datasetId)) continue;
      if (evidenceId == null) evidenceId = entry?.id ?? entry?.name ?? null;
      matchedPaths.push(...extractDynamicsPathRecords(entry));
    }
    if (matchedPaths.length) {
      return createTrajectoryPathEvidence(`exported-${collection}`, matchedPaths, {
        id: evidenceId,
        collection,
        timelineId: timeline?.id || null,
        datasetId,
      });
    }
  }
  return null;
}

function createTrajectoryPathEvidence(source, value, options = {}) {
  const paths = normalizeDynamicsPathRecords(extractDynamicsPathRecords(value), {
    id: options.id || null,
    source,
    timelineId: options.timelineId || null,
    datasetId: options.datasetId || null,
  });
  if (!paths.length) return null;
  return {
    schema: "metric.visual.trajectory_path_evidence_ref.v1",
    id: options.id || null,
    source,
    collection: options.collection || null,
    timelineId: options.timelineId || null,
    datasetId: options.datasetId || null,
    pathCount: paths.length,
    paths,
    algorithmicComputation: false,
  };
}

function extractDynamicsPathRecords(value) {
  if (!value) return [];
  if (Array.isArray(value)) return value;
  if (Array.isArray(value.paths)) return value.paths;
  if (Array.isArray(value.trajectories)) return value.trajectories;
  if (Array.isArray(value.routes)) return value.routes;
  if (Array.isArray(value.curves)) return value.curves;
  if (hasPathGeometry(value) || value.record_id != null || value.recordId != null || value.recordIds || value.record_ids) return [value];
  return [];
}

function normalizeDynamicsPathRecords(paths, evidence = null) {
  if (!Array.isArray(paths)) return [];
  return paths.map((path, index) => {
    if (Array.isArray(path) || ArrayBuffer.isView(path)) return path;
    if (!path || typeof path !== "object") return path;
    const recordIds = normalizePathRecordIds(path);
    return {
      ...path,
      id: path.id || `${evidence?.id || "trajectory"}:path-${index}`,
      recordIds: recordIds.length ? recordIds : path.recordIds,
      metadata: {
        ...(path.metadata || {}),
        evidenceType: path.evidenceType || path.type || "exported-dynamics-trajectory",
        pathEvidenceId: evidence?.id || null,
        pathEvidenceSource: evidence?.source || null,
        timelineId: path.timelineId || path.timeline_id || evidence?.timelineId || path.metadata?.timelineId || null,
        recordIds: recordIds.length ? recordIds : path.metadata?.recordIds,
      },
    };
  });
}

function normalizePathRecordIds(path) {
  const ids = path.recordIds || path.record_ids || path.ids || path.metadata?.recordIds;
  if (Array.isArray(ids)) return ids.filter((id) => id != null).map(String);
  const single = path.record_id ?? path.recordId ?? path.metadata?.recordId;
  return single == null ? [] : [String(single)];
}

function dynamicsPathEvidenceMatches(entry, timeline, datasetId = null) {
  const timelineId = timeline?.id;
  const entryTimelineIds = [
    entry?.timeline_id,
    entry?.timelineId,
    entry?.source_timeline_id,
    entry?.sourceTimelineId,
    entry?.metadata?.timelineId,
    entry?.metadata?.timeline_id,
  ].filter((value) => value != null).map(String);
  if (timelineId != null && entryTimelineIds.length && !entryTimelineIds.includes(String(timelineId))) return false;

  const entryDatasetId = entry?.dataset_id ?? entry?.datasetId ?? entry?.metadata?.datasetId ?? entry?.metadata?.dataset_id;
  if (datasetId != null && entryDatasetId != null && String(entryDatasetId) !== String(datasetId)) return false;

  if (entryTimelineIds.length) return true;
  const text = `${entry?.id || ""} ${entry?.name || ""} ${entry?.kind || ""} ${entry?.type || ""} ${entry?.metadata?.evidenceType || ""}`;
  return /trajectory|path|history|timeline|dynamics|diffusion|reverse|flow/i.test(text);
}

function hasPathGeometry(value) {
  return Boolean(value?.points || value?.positions || value?.coordinates || value?.coords || value?.path);
}

function resolveTimelineSamplingOptions(options = {}) {
  const out = {};
  const normalized = options.timelinePosition
    ?? options.timelineProgress
    ?? options.normalizedTimelineTime
    ?? options.normalizedTimelinePosition;
  if (normalized != null) out.normalized = normalized;
  const time = options.timelineTime ?? options.timelineCurrentTime;
  if (time != null) out.time = time;
  return out;
}

function resolveTimelinePropertySamplingOptions(options = {}) {
  const out = {};
  const propertyId = options.timelinePropertyId
    ?? options.timelineProperty
    ?? options.timelineFieldPropertyId
    ?? options.timelineFieldProperty
    ?? options.fieldTimelinePropertyId
    ?? options.fieldTimelineProperty;
  if (propertyId != null) out.timelinePropertyId = propertyId;
  return out;
}

function selectTimelineFieldState(timelineState, options = {}) {
  const samples = Array.isArray(timelineState?.activePropertySamples)
    ? timelineState.activePropertySamples.slice()
    : [];
  if (!samples.length) return null;
  const preferred = [
    options.timelineFieldPropertyId,
    options.timelineFieldProperty,
    options.fieldTimelinePropertyId,
    options.fieldTimelineProperty,
    options.timelinePropertyId,
    options.timelineProperty,
  ].filter((value) => value != null).map(String);
  if (preferred.length) {
    const preferredSet = new Set(preferred);
    const explicit = samples.find((sample) => preferredSet.has(String(sample.propertyId)));
    if (explicit) return explicit;
  }
  samples.sort((a, b) => timelineFieldPriority(a) - timelineFieldPriority(b)
    || String(a.propertyId || "").localeCompare(String(b.propertyId || "")));
  return samples[0] || null;
}

function timelineFieldPriority(sample) {
  const text = `${sample?.propertyId || ""} ${sample?.propertyName || ""}`.toLowerCase();
  if (/mse|error|residual|loss|reconstruction|noise/.test(text)) return 0;
  if (/density|entropy|uncertainty|anomaly|outlier/.test(text)) return 1;
  if (/energy|dirichlet|objective|cost/.test(text)) return 2;
  return 3;
}

function scalarMapFromValue(ids, value) {
  const out = new Map();
  const scalar = Number.isFinite(Number(value)) ? Number(value) : 0;
  for (const id of ids) out.set(String(id), scalar);
  return out;
}

function timelineFieldPositions(sourcePositions, scalarValues, domain, options = {}) {
  const positions = new Float32Array(sourcePositions);
  const mode = options.mode === "lifted" ? "lifted" : "ground";
  const groundY = Number.isFinite(Number(options.groundY)) ? Number(options.groundY) : 0;
  const lift = Number.isFinite(Number(options.lift)) ? Number(options.lift) : 0;
  for (let index = 0; index < scalarValues.length; index += 1) {
    const offset = index * 3;
    const t = normalizeScalar(scalarValues[index], domain);
    positions[offset + 1] = mode === "lifted" ? sourcePositions[offset + 1] + t * lift : groundY;
  }
  return positions;
}

function normalizeScalar(value, domain) {
  const min = Number.isFinite(Number(domain?.min)) ? Number(domain.min) : 0;
  const max = Number.isFinite(Number(domain?.max)) ? Number(domain.max) : 1;
  const span = Math.max(0.000001, max - min);
  return Math.max(0, Math.min(1, (Number(value) - min) / span));
}

function scalarDomainForValues(values, ids = []) {
  if (!values) return undefined;
  return inferScalarDomain(Array.from(flattenValues(values, ids, 0)));
}

function normalizeNumberRange(value, fallback) {
  const source = Array.isArray(value) || ArrayBuffer.isView(value) ? Array.from(value) : [];
  const min = Number(source[0]);
  const max = Number(source[1]);
  if (Number.isFinite(min) && Number.isFinite(max) && max >= min) return [min, max];
  return fallback;
}

function normalizeTimelineContextStops(value, stepCount) {
  const source = Array.isArray(value) && value.length ? value : [0, 0.5, 1];
  const stops = [];
  const seen = new Set();
  const maxStep = Math.max(0, stepCount - 1);
  for (let index = 0; index < source.length; index += 1) {
    const entry = source[index];
    const normalized = typeof entry === "object" && entry
      ? Number(entry.normalized ?? entry.value ?? entry.timelineProgress ?? entry.progress)
      : Number(entry);
    const explicitStepOrder = typeof entry === "object" && entry
      ? Number(entry.activeStepOrder ?? entry.stepOrder ?? entry.order ?? entry.index)
      : NaN;
    const safeNormalized = Number.isFinite(normalized)
      ? Math.max(0, Math.min(1, normalized))
      : Number.isFinite(explicitStepOrder) && maxStep > 0
        ? Math.max(0, Math.min(1, explicitStepOrder / maxStep))
        : index / Math.max(1, source.length - 1);
    const stepOrder = Number.isFinite(explicitStepOrder)
      ? clampInt(explicitStepOrder, 0, maxStep)
      : clampInt(Math.round(safeNormalized * maxStep), 0, maxStep);
    if (seen.has(stepOrder)) continue;
    seen.add(stepOrder);
    stops.push({
      normalized: safeNormalized,
      stepOrder,
      label: typeof entry === "object" && entry ? entry.activeLabel || entry.label || null : null,
    });
  }
  return stops;
}

function constantColorMap(ids, color) {
  const out = new Map();
  for (const id of ids || []) out.set(String(id), color);
  return out;
}

function evenlySpacedIndices(count, limit) {
  const total = Math.max(0, Math.floor(Number(count) || 0));
  const wanted = Math.max(0, Math.min(total, Math.floor(Number(limit) || 0)));
  if (!total || !wanted) return [];
  if (wanted >= total) return Array.from({ length: total }, (_, index) => index);
  if (wanted === 1) return [0];
  const out = [];
  const seen = new Set();
  for (let index = 0; index < wanted; index += 1) {
    const next = Math.round((index * (total - 1)) / (wanted - 1));
    if (!seen.has(next)) {
      seen.add(next);
      out.push(next);
    }
  }
  for (let index = 0; out.length < wanted && index < total; index += 1) {
    if (seen.has(index)) continue;
    seen.add(index);
    out.push(index);
  }
  out.sort((a, b) => a - b);
  return out;
}

function selectByIndex(values, indices) {
  return indices.map((index) => values[index]).filter((value) => value != null);
}

function firstPropertyRef(...values) {
  for (const value of values) {
    if (value == null || value === true || value === false) continue;
    return value;
  }
  return null;
}

function isRecordTargetProperty(property) {
  if (!property) return false;
  const target = property.target_type ?? property.targetType ?? "record";
  return target == null || target === "record" || target === "records";
}

function mixColor(a, b, t) {
  const x = Math.max(0, Math.min(1, Number.isFinite(t) ? t : 0));
  return [
    a[0] + (b[0] - a[0]) * x,
    a[1] + (b[1] - a[1]) * x,
    a[2] + (b[2] - a[2]) * x,
  ];
}

function clampInt(value, min, max) {
  const number = Math.floor(Number(value));
  if (!Number.isFinite(number)) return min;
  return Math.max(min, Math.min(max, number));
}
