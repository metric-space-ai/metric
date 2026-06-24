import { BaseView } from "./BaseView.js";
import { PointCloudView } from "./PointCloudView.js";
import { VisualLayer } from "./VisualLayer.js";
import { applyPositionFit, computePositionFit } from "./scene-fit.js";
import {
  createChannel,
  descriptorSource,
  extractCoordinatePositions,
  extractPropertyValues,
  flattenVectors,
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
    this.activeStep = clampInt(options.activeStep, 0, Math.max(0, this.stepStates.length - 1));
    this.groundY = Number.isFinite(Number(options.groundY)) ? Number(options.groundY) : 0;
    this.targetRadius = Number.isFinite(Number(options.targetRadius)) ? Number(options.targetRadius) : 1.6;
    this.size = Number.isFinite(Number(options.size)) ? Number(options.size) : 1;
    this.showTrajectory = options.trajectory !== false;
    this.colorValues = options.colorValues;

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
    const steps = Array.isArray(timeline?.steps) ? timeline.steps : [];
    const datasetId = options.datasetId ?? timeline?.dataset_id;
    const records = recordsFor(document, { ...options, datasetId });

    let recordIds = options.recordIds || [];
    const stepStates = steps.map((step) => {
      const coordinate = resolveCollectionItem(document, "coordinates", step.coordinate_id ?? step.coordinateId);
      const property = resolveCollectionItem(document, "properties", step.property_id ?? step.propertyId);
      const positions = extractCoordinatePositions(coordinate, { records, recordIds: recordIds.length ? recordIds : undefined });
      if (!recordIds.length) recordIds = positions.ids;
      return {
        name: step.name || coordinate?.name || coordinate?.id,
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
      datasetId,
      spaceId: options.spaceId ?? timeline?.space_id,
      name: options.name || timeline?.name,
      metadata: { ...(options.metadata || {}), timelineId: timeline?.id, stepCount: stepStates.length },
    });
  }

  toLayerDescriptors() {
    const descriptors = [];
    if (this.showTrajectory && this.fittedStates.length > 1) {
      descriptors.push(this.trajectoryDescriptor());
    }
    const active = this.fittedStates[this.activeStep];
    if (active) {
      const point = new PointCloudView({
        id: `${this.id}:state`,
        datasetId: this.datasetId,
        spaceId: this.spaceId,
        records: this.records,
        recordIds: this.recordIds,
        positions: active.positions,
        scalarValues: active.scalarValues || undefined,
        colorValues: this.colorValues,
        size: this.size,
        shape: "sphere",
        metadata: { ...this.metadata, step: this.activeStep },
      });
      descriptors.push(...point.toLayerDescriptors());
    }
    return descriptors;
  }

  trajectoryDescriptor() {
    const segments = [];
    for (let stepIndex = 0; stepIndex < this.fittedStates.length - 1; stepIndex += 1) {
      const from = flattenVectors(this.fittedStates[stepIndex].positions, this.recordIds, 3);
      const to = flattenVectors(this.fittedStates[stepIndex + 1].positions, this.recordIds, 3);
      const alpha = 0.18 + 0.5 * (stepIndex / Math.max(1, this.fittedStates.length - 1));
      for (let record = 0; record < this.recordIds.length; record += 1) {
        const offset = record * 3;
        segments.push({
          source: [from[offset], from[offset + 1], from[offset + 2]],
          target: [to[offset], to[offset + 1], to[offset + 2]],
          alpha,
        });
      }
    }
    const sourcePosition = new Float32Array(segments.length * 3);
    const targetPosition = new Float32Array(segments.length * 3);
    const color = new Float32Array(segments.length * 4);
    for (let index = 0; index < segments.length; index += 1) {
      sourcePosition.set(segments[index].source, index * 3);
      targetPosition.set(segments[index].target, index * 3);
      color.set([0.32, 0.56, 0.66, segments[index].alpha], index * 4);
    }
    return new VisualLayer({
      id: `${this.id}:trajectory`,
      kind: "trajectory",
      primitive: "RelationEdgeLayer",
      order: 5,
      source: descriptorSource(this, { viewKind: "dynamics-trajectory" }),
      channels: {
        sourcePosition: createChannel(sourcePosition, 3, "source-position"),
        targetPosition: createChannel(targetPosition, 3, "target-position"),
        color: createChannel(color, 4, "rgba"),
      },
      geometry: { width: 1 },
      material: { alpha: 1, transparent: true, depthWrite: false },
      metadata: { ...this.metadata, segmentCount: segments.length, role: "trajectory" },
    }).toDescriptor();
  }

  setActiveStep(step) {
    this.activeStep = clampInt(step, 0, Math.max(0, this.fittedStates.length - 1));
    return this;
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

function clampInt(value, min, max) {
  const number = Math.floor(Number(value));
  if (!Number.isFinite(number)) return min;
  return Math.max(min, Math.min(max, number));
}
