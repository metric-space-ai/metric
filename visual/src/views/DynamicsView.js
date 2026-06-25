import { BaseView } from "./BaseView.js";
import { PointCloudView } from "./PointCloudView.js";
import { createTrajectoryBundleLayerDescriptor } from "../curves/index.js";
import {
  createTimelineAnimationDescriptor,
  createTimelineControlDescriptor,
  createTimelineModel,
  sampleTimelineState,
} from "../timeline/index.js";
import { applyPositionFit, computePositionFit } from "./scene-fit.js";
import {
  extractCoordinatePositions,
  extractPropertyValues,
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
    const timelineModel = timeline
      ? createTimelineModel(document, {
        timelineId: timeline.id,
        loop: options.loop ?? true,
        playbackRate: options.playbackRate,
        easing: options.easing,
      })
      : null;
    const steps = timelineModel?.steps?.length
      ? timelineModel.steps
      : (Array.isArray(timeline?.steps) ? timeline.steps : []);
    const timelineSampling = timelineModel ? resolveTimelineSamplingOptions(options) : {};
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

    let recordIds = options.recordIds || [];
    const stepStates = steps.map((step) => {
      const sourceStep = step.source || step;
      const coordinateId = step.coordinateId ?? sourceStep.coordinate_id ?? sourceStep.coordinateId;
      const propertyId = step.propertyId ?? sourceStep.property_id ?? sourceStep.propertyId;
      const coordinate = resolveCollectionItem(document, "coordinates", coordinateId);
      const property = resolveCollectionItem(document, "properties", propertyId);
      const positions = extractCoordinatePositions(coordinate, { records, recordIds: recordIds.length ? recordIds : undefined });
      if (!recordIds.length) recordIds = positions.ids;
      return {
        name: step.label || sourceStep.name || coordinate?.name || coordinate?.id,
        stepId: sourceStep.id || null,
        stepIndex: step.index ?? sourceStep.index ?? sourceStep.step_index ?? sourceStep.step ?? null,
        time: step.time ?? sourceStep.time ?? sourceStep.t ?? null,
        coordinateId: coordinate?.id,
        propertyId: property?.id,
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
    };
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
      coordinateIds: this.fittedStates.map((state) => state.coordinateId).filter(Boolean),
      algorithmicComputation: false,
    };
    return descriptor;
  }

  setActiveStep(step) {
    this.activeStep = clampInt(step, 0, Math.max(0, this.fittedStates.length - 1));
    return this;
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
