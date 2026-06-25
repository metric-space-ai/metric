import { BaseView } from "./BaseView.js";
import { PointCloudView } from "./PointCloudView.js";
import { VisualLayer } from "./VisualLayer.js";
import { createTrajectoryBundleLayerDescriptor } from "../curves/index.js";
import {
  createTimelineAnimationDescriptor,
  createTimelineControlDescriptor,
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
    const datasetId = options.datasetId ?? timeline?.dataset_id;
    const records = recordsFor(document, { ...options, datasetId });
    const explicitFieldPropertyRef = options.fieldPropertyId
      || options.propertyField
      || options.groundField
      || options.scalarProperty
      || options.scalarPropertyId
      || null;

    let recordIds = options.recordIds || [];
    const stepStates = steps.map((step) => {
      const sourceStep = step.source || step;
      const coordinateId = step.coordinateId ?? sourceStep.coordinate_id ?? sourceStep.coordinateId;
      const propertyId = step.propertyId ?? sourceStep.property_id ?? sourceStep.propertyId;
      const coordinate = resolveCollectionItem(document, "coordinates", coordinateId);
      const fieldPropertyId = propertyId || explicitFieldPropertyRef;
      const property = resolveCollectionItem(document, "properties", fieldPropertyId);
      const positions = extractCoordinatePositions(coordinate, { records, recordIds: recordIds.length ? recordIds : undefined });
      if (!recordIds.length) recordIds = positions.ids;
      return {
        name: step.label || sourceStep.name || coordinate?.name || coordinate?.id,
        stepId: sourceStep.id || null,
        stepIndex: step.index ?? sourceStep.index ?? sourceStep.step_index ?? sourceStep.step ?? null,
        time: step.time ?? sourceStep.time ?? sourceStep.t ?? null,
        coordinateId: coordinate?.id,
        propertyId: property?.id,
        fieldPropertySource: propertyId ? "timeline-step-property" : (property ? "selected-property" : null),
        positions: positions.positions,
        scalarValues: property ? extractPropertyValues(property, { records, recordIds: positions.ids }) : null,
      };
    });

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
      timelineAnimation: timelineModel
        ? createTimelineAnimationDescriptor(timelineModel, {
          mode: "timeline-coordinate-morph",
          loop: options.loop ?? true,
          direction: options.direction || "alternate",
          durationMs: options.timelineDurationMs ?? options.durationMs,
          stepDurationMs: options.stepDurationMs ?? 360,
          minDurationMs: options.minDurationMs ?? 6800,
          maxDurationMs: options.maxDurationMs ?? 18000,
        })
        : null,
      metadata: {
        ...(options.metadata || {}),
        timelineId: timeline?.id,
        stepCount: stepStates.length,
        coordinateIds: stepStates.map((state) => state.coordinateId).filter(Boolean),
        timelineControl,
        timelineState,
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
    const active = this.fittedStates[this.activeStep];
    if (active) {
      const target = this.motionTargetPositions();
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
        shape: "sphere",
        animation: target ? this.stateAnimationDescriptor() : { mode: "none" },
        metadata: {
          ...this.metadata,
          ...this.timelineDescriptorMetadata(),
          step: this.activeStep,
          activeCoordinateId: active.coordinateId,
          sampledCoordinateId: this.timelineState?.activeCoordinateId ?? active.coordinateId,
          activePropertyId: active.propertyId ?? this.timelineState?.activePropertyId ?? null,
          activeFieldPropertyId: this.activeFieldPropertyId(active),
          timelineFieldState: this.timelineFieldState,
          targetCoordinateId: target ? this.fittedStates[this.motionTargetStep]?.coordinateId : null,
        },
      });
      descriptors.push(...point.toLayerDescriptors());
    }
    return descriptors;
  }

  timelineDescriptorMetadata() {
    return {
      timelineId: this.timelineId,
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
    const fieldStateSource = active.scalarValues ? active.fieldPropertySource || "selected-property" : "timeline-step-property";

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
        algorithmicComputation: false,
      },
    }).toDescriptor();
  }

  trajectoryDescriptor() {
    const paths = [];
    for (let recordIndex = 0; recordIndex < this.recordIds.length; recordIndex += 1) {
      const recordId = this.recordIds[recordIndex];
      const points = [];
      for (let stepIndex = 0; stepIndex < this.fittedStates.length; stepIndex += 1) {
        const state = this.fittedStates[stepIndex];
        const position = positionFor(state.positions, recordId);
        if (!position) continue;
        points.push({
          x: position[0],
          y: position[1],
          z: position[2],
          time: Number.isFinite(Number(state.time)) ? Number(state.time) : stepIndex,
          color: timelinePathColor(stepIndex, this.fittedStates.length, this.pathAlpha),
          width: this.pathWidth,
        });
      }
      if (points.length >= 2) {
        paths.push({
          id: `${this.id}:trajectory:${recordId}`,
          points,
          metadata: {
            recordId,
            evidenceType: "diffusion-trajectory",
            timelineId: this.timelineId,
          },
        });
      }
    }
    if (!paths.length) return null;
    const descriptor = createTrajectoryBundleLayerDescriptor({ paths }, {
      id: `${this.id}:trajectory`,
      order: 28,
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
    });
    descriptor.kind = "trajectory";
    descriptor.source = {
      ...descriptor.source,
      viewId: this.id,
      viewKind: "dynamics-trajectory",
      timelineId: this.timelineId,
    };
    descriptor.metadata = {
      ...descriptor.metadata,
      ...this.metadata,
      ...this.timelineDescriptorMetadata(),
      role: "trajectory",
      evidenceRole: "trajectory/path",
      stateHistoryRole: "timeline-state-history",
      pathCount: paths.length,
      stepCount: this.fittedStates.length,
      timelineId: this.timelineId,
      timelineFieldState: this.timelineFieldState,
      coordinateIds: this.fittedStates.map((state) => state.coordinateId).filter(Boolean),
      algorithmicComputation: false,
    };
    return descriptor;
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
    if (this.timelineControl) animation.control = this.timelineControl;
    if (this.timelineState) animation.state = this.timelineState;
    if (this.motionProgress != null) {
      animation.loop = false;
      animation.progress = this.motionProgress;
    }
    return animation;
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

function positionFor(map, id) {
  return map?.get?.(id) || map?.get?.(String(id)) || null;
}

function timelinePathColor(stepIndex, stepCount, alpha) {
  const t = stepCount <= 1 ? 0 : stepIndex / (stepCount - 1);
  const low = [0.12, 0.25, 0.42];
  const mid = [0.12, 0.54, 0.52];
  const high = [0.92, 0.54, 0.22];
  const color = t < 0.58
    ? mixColor(low, mid, t / 0.58)
    : mixColor(mid, high, (t - 0.58) / 0.42);
  return [color[0], color[1], color[2], alpha];
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
