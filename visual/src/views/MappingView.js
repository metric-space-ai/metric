import { BaseView } from "./BaseView.js";
import { MetricSpaceView, defaultCoordinateId } from "./MetricSpaceView.js";
import { VisualLayer } from "./VisualLayer.js";
import {
  createChannel,
  createStringChannel,
  extractCoordinatePositions,
  extractPropertyValues,
  inferScalarDomain,
  recordsFor,
  resolveCollectionItem,
  scalarColor,
  valueForId,
} from "./view-utils.js";

export const DEFAULT_MAPPING_MOTION_TIMING = Object.freeze({
  profile: "source-hold-quick-transition-target-hold",
  sourceHoldMs: 1600,
  transitionMs: 720,
  targetHoldMs: 1700,
  resetHoldMs: 360,
});

/**
 * MappingView treats a mapping as a transformation between two exported
 * coordinate states (source -> target, e.g. 2D -> 3D, or PHATE/AE/PCFA target).
 * It animates a uniform morph between the states and colors records by an
 * exported preservation/residual property so distortion is visible. It does not
 * fit a mapping; it only visualizes the exported source/target/residual.
 */
export class MappingView extends BaseView {
  constructor(options = {}) {
    super({ ...options, kind: "mapping" });
    this.residualValues = options.residualValues || options.scalarValues || null;
    this.recordIds = options.recordIds || [];
    this.sourceCoordinateId = options.sourceCoordinateId || options.coordinateId || null;
    this.targetCoordinateId = options.targetCoordinateId || null;
    this.showResidualVectors = options.residualVectors !== false;
    this.residualVectorColor = options.residualVectorColor || options.vectorColor || null;
    this.residualVectorAlpha = Number.isFinite(Number(options.residualVectorAlpha))
      ? Number(options.residualVectorAlpha)
      : 0.58;
    this.residualVectorWidth = Number.isFinite(Number(options.residualVectorWidth))
      ? Number(options.residualVectorWidth)
      : 1;
    this.motionTiming = normalizeMappingMotionTiming(options.motionTiming || options.mappingMotionTiming || options.morphTiming, options);
    const hasNamedResidualProperty = hasNonEmptyId(this.propertyId) && this.residualValues;
    this.space = new MetricSpaceView({
      ...options,
      id: `${this.id}:space`,
      scalarValues: hasNamedResidualProperty ? this.residualValues : undefined,
      colorValues: hasNamedResidualProperty ? undefined : options.colorValues,
      ground: options.ground !== false,
      groundProjection: options.groundProjection !== false,
      loop: options.loop ?? true,
      durationMs: this.motionTiming.totalMs,
    });
  }

  static fromVisualSpace(document, options = {}) {
    const spaceRef = options.space || options.spaceId;
    const space = resolveCollectionItem(document, "spaces", spaceRef, {
      required: spaceRef != null,
      label: "space",
    });
    const explicitSourceRef = options.sourceCoordinate ?? options.sourceCoordinateId;
    const explicitTargetRef = options.targetCoordinate ?? options.targetCoordinateId;
    const sourceRef = explicitSourceRef ?? defaultCoordinateId(document, space, { dimension: 2 });
    const targetRef = explicitTargetRef ?? defaultCoordinateId(document, space, { dimension: 3 });
    const sourceCoordinate = resolveCollectionItem(document, "coordinates", sourceRef, {
      required: sourceRef != null,
      label: explicitSourceRef != null ? "source coordinate" : "default source coordinate",
    });
    const targetCoordinate = resolveCollectionItem(document, "coordinates", targetRef, {
      required: targetRef != null,
      label: explicitTargetRef != null ? "target coordinate" : "default target coordinate",
    });
    const residualPropertyRef = options.residualProperty || options.residualPropertyId || options.scalarProperty;
    const colorPropertyRef = options.colorProperty || options.colorPropertyId;
    const labelPropertyRef = options.labelProperty || options.labelPropertyId || (typeof options.labels === "string" ? options.labels : undefined);
    const residualProperty = resolveCollectionItem(document, "properties", residualPropertyRef, {
      required: residualPropertyRef != null,
      label: "residual property",
    });
    const colorProperty = resolveCollectionItem(document, "properties", colorPropertyRef, {
      required: colorPropertyRef != null,
      label: "color property",
    });
    const labelProperty = resolveCollectionItem(
      document,
      "properties",
      labelPropertyRef,
      {
        required: labelPropertyRef != null,
        label: "label property",
      },
    );

    const datasetId = options.datasetId ?? sourceCoordinate?.dataset_id ?? space?.dataset_id;
    const records = recordsFor(document, { ...options, datasetId });
    const source = extractCoordinatePositions(sourceCoordinate, { records, recordIds: options.recordIds || space?.record_ids });
    const target = extractCoordinatePositions(targetCoordinate, { records, recordIds: source.ids });
    const residualValues = residualProperty ? extractPropertyValues(residualProperty, { records, recordIds: source.ids }) : null;

    return new MappingView({
      ...options,
      records,
      recordIds: source.ids,
      positions: source.positions,
      targetPositions: target.positions,
      residualValues,
      colorValues: colorProperty ? extractPropertyValues(colorProperty, { records, recordIds: source.ids }) : options.colorValues,
      labelValues: labelProperty ? extractPropertyValues(labelProperty, { records, recordIds: source.ids }) : options.labelValues,
      datasetId,
      spaceId: options.spaceId ?? space?.id,
      coordinateId: sourceCoordinate?.id,
      sourceCoordinateId: sourceCoordinate?.id,
      targetCoordinateId: targetCoordinate?.id,
      propertyId: residualProperty?.id,
    });
  }

  toLayerDescriptors() {
    const mappingEvidence = this.mappingMotionEvidenceDescriptor();
    const descriptors = this.space.toLayerDescriptors().map((rawDescriptor) => {
      const descriptor = this.withMappingMotionTiming(rawDescriptor);
      if (descriptor.primitive !== "InstancedPointLayer" && descriptor.primitive !== "InstancedGlyphLayer") return descriptor;
      return {
        ...descriptor,
        source: {
          ...descriptor.source,
          viewKind: "mapping",
          sourceCoordinateId: this.sourceCoordinateId,
          targetCoordinateId: this.targetCoordinateId,
          residualPropertyId: this.propertyId,
        },
        metadata: {
          ...descriptor.metadata,
          role: "mapping-coordinate-morph",
          evidenceRole: "mapping-source-target-morph",
          motionGrammar: "mapping-coordinate-morph",
          mappingEvidence,
          mappingMotionTiming: this.motionTiming,
          sourceCoordinateId: this.sourceCoordinateId,
          targetCoordinateId: this.targetCoordinateId,
          residualPropertyId: this.propertyId,
          algorithmicComputation: false,
        },
      };
    });
    const residual = this.residualVectorDescriptor();
    if (residual) descriptors.push(residual);
    return descriptors;
  }

  withMappingMotionTiming(descriptor) {
    const animation = descriptor.animation || { mode: "none" };
    const isMorph = String(animation.mode || "").includes("morph") || animation.channel === "targetPosition";
    return {
      ...descriptor,
      animation: isMorph
        ? {
          ...animation,
          durationMs: this.motionTiming.totalMs,
          timingProfile: this.motionTiming.profile,
          timingPhases: this.motionTiming.phases,
        }
        : animation,
      metadata: {
        ...descriptor.metadata,
        mappingMotionTiming: this.motionTiming,
      },
    };
  }

  residualVectorDescriptor() {
    if (!this.showResidualVectors || !this.space?.targetPositions || !this.hasResidualEvidence()) return null;
    const ids = this.recordIds || this.space.recordIds || [];
    const sourcePosition = new Float32Array(ids.length * 3);
    const targetPosition = new Float32Array(ids.length * 3);
    const color = new Float32Array(ids.length * 4);
    const residualMagnitude = new Float32Array(ids.length);
    const residualRecordIds = [];
    const finiteResiduals = ids
      .map((id, index) => Number(valueForId(this.residualValues, id, index)))
      .filter((value) => Number.isFinite(value));
    const domain = inferScalarDomain(finiteResiduals);
    let count = 0;
    for (let index = 0; index < ids.length; index += 1) {
      const id = ids[index];
      const source = positionFor(this.space.positions, id);
      const target = positionFor(this.space.targetPositions, id);
      if (!source || !target) continue;
      sourcePosition.set(source, count * 3);
      targetPosition.set(target, count * 3);
      const residual = Number(valueForId(this.residualValues, id, index));
      const nextColor = this.residualVectorColor
        ? normalizeRgba(this.residualVectorColor, this.residualVectorAlpha)
        : Number.isFinite(residual)
          ? scalarColor(residual, domain, this.residualVectorAlpha)
          : [0.8, 0.22, 0.12, this.residualVectorAlpha];
      color.set(nextColor, count * 4);
      residualMagnitude[count] = Number.isFinite(residual) ? residual : 0;
      residualRecordIds.push(String(id));
      count += 1;
    }
    if (!count) return null;
    const mappingEvidence = this.mappingMotionEvidenceDescriptor({ residualVectorCount: count });
    return new VisualLayer({
      id: `${this.id}:residual-vectors`,
      kind: "residual/error-vectors",
      primitive: "RelationEdgeLayer",
      order: 36,
      source: {
        viewId: this.id,
        viewKind: "mapping",
        role: "residual/error",
        sourceCoordinateId: this.sourceCoordinateId,
        targetCoordinateId: this.targetCoordinateId,
        propertyId: this.propertyId,
      },
      channels: {
        recordId: createStringChannel(residualRecordIds, "record-id"),
        sourcePosition: createChannel(sourcePosition.subarray(0, count * 3), 3, "source-position"),
        targetPosition: createChannel(targetPosition.subarray(0, count * 3), 3, "target-position"),
        residual: createChannel(residualMagnitude.subarray(0, count), 1, "residual-magnitude", { domain }),
        color: createChannel(color.subarray(0, count * 4), 4, "rgba"),
      },
      geometry: { width: this.residualVectorWidth },
      material: { alpha: 1, transparent: true, depthWrite: false },
      metadata: {
        ...this.metadata,
        role: "residual/error",
        evidenceRole: "mapping-residual-vectors",
        diagnosticLayer: true,
        motionGrammar: "mapping-residual-error-scene",
        mappingEvidence,
        mappingMotionTiming: this.motionTiming,
        recordCount: count,
        sourceCoordinateId: this.sourceCoordinateId,
        targetCoordinateId: this.targetCoordinateId,
        residualPropertyId: this.propertyId,
        residualDomain: domain,
        algorithmicComputation: false,
      },
    }).toDescriptor();
  }

  mappingMotionEvidenceDescriptor(extra = {}) {
    const residualLayer = this.showResidualVectors && this.hasResidualEvidence()
      ? `${this.id}:residual-vectors`
      : null;
    return {
      schema: "metric.visual.mapping_motion_evidence.v1",
      source: "exported-coordinate-and-property-evidence",
      viewId: this.id,
      datasetId: this.datasetId,
      spaceId: this.spaceId,
      recordCount: this.recordIds.length,
      sourceCoordinateId: this.sourceCoordinateId,
      targetCoordinateId: this.targetCoordinateId,
      residualPropertyId: this.propertyId,
      motionContract: {
        mode: "coordinate-morph",
        channel: "targetPosition",
        residualLayer,
        timingProfile: this.motionTiming.profile,
        timingPhases: this.motionTiming.phases,
        controlledBy: "descriptor-animation",
      },
      motionTiming: this.motionTiming,
      preservationSummary: this.preservationSummary(),
      algorithmicComputation: false,
      ...extra,
    };
  }

  /**
   * Summarize residual/preservation evidence for panels. Pure reduction over
   * the exported residual property.
   */
  preservationSummary() {
    const values = this.residualEvidenceValues();
    if (!values.length) return { count: 0, meanResidual: 0, maxResidual: 0 };
    const sum = values.reduce((total, value) => total + value, 0);
    return {
      count: values.length,
      meanResidual: sum / values.length,
      maxResidual: Math.max(...values),
      sourceCoordinateId: this.sourceCoordinateId,
      targetCoordinateId: this.targetCoordinateId,
    };
  }

  hasResidualEvidence() {
    return this.residualEvidenceValues().length > 0;
  }

  residualEvidenceValues() {
    if (!hasNonEmptyId(this.propertyId) || !this.residualValues) return [];
    return (this.recordIds || [])
      .map((id, index) => Number(valueForId(this.residualValues, id, index)))
      .filter((value) => Number.isFinite(value));
  }
}

export function createMappingView(options) {
  return new MappingView(options);
}

export function normalizeMappingMotionTiming(timing = {}, options = {}) {
  const sourceHoldMs = positiveNumber(timing.sourceHoldMs ?? options.sourceHoldMs, DEFAULT_MAPPING_MOTION_TIMING.sourceHoldMs);
  const transitionMs = positiveNumber(timing.transitionMs ?? options.transitionMs, DEFAULT_MAPPING_MOTION_TIMING.transitionMs);
  const targetHoldMs = positiveNumber(timing.targetHoldMs ?? options.targetHoldMs, DEFAULT_MAPPING_MOTION_TIMING.targetHoldMs);
  const resetHoldMs = positiveNumber(timing.resetHoldMs ?? options.resetHoldMs, DEFAULT_MAPPING_MOTION_TIMING.resetHoldMs);
  const explicitTotal = positiveNumber(timing.totalMs ?? options.totalMs ?? options.durationMs, 0);
  const totalMs = explicitTotal > 0
    ? explicitTotal
    : sourceHoldMs + transitionMs + targetHoldMs + resetHoldMs;
  const normalized = {
    profile: timing.profile || DEFAULT_MAPPING_MOTION_TIMING.profile,
    sourceHoldMs,
    transitionMs,
    targetHoldMs,
    resetHoldMs,
    totalMs,
  };
  normalized.phases = mappingMotionPhases(normalized);
  normalized.transitionShare = transitionMs / Math.max(1, totalMs);
  return normalized;
}

export function mappingMotionProgressAt(elapsedMs, timing = DEFAULT_MAPPING_MOTION_TIMING) {
  const normalized = timing?.phases ? timing : normalizeMappingMotionTiming(timing);
  const totalMs = Math.max(1, Number(normalized.totalMs) || 1);
  const phaseMs = ((Number(elapsedMs) || 0) % totalMs + totalMs) % totalMs;
  const sourceEnd = normalized.phases.sourceHold.endMs;
  const transitionEnd = normalized.phases.quickTransition.endMs;
  const targetEnd = normalized.phases.targetHold.endMs;
  if (phaseMs <= sourceEnd) return { progress: 0, phase: "source-hold", elapsedMs: phaseMs };
  if (phaseMs <= transitionEnd) {
    const local = (phaseMs - sourceEnd) / Math.max(1, normalized.transitionMs);
    return { progress: smoothstep(clamp01(local)), phase: "quick-transition", elapsedMs: phaseMs };
  }
  if (phaseMs <= targetEnd) return { progress: 1, phase: "target-hold", elapsedMs: phaseMs };
  return { progress: 0, phase: "source-reset-hold", elapsedMs: phaseMs };
}

function positionFor(map, id) {
  return map?.get?.(id) || map?.get?.(String(id)) || null;
}

function mappingMotionPhases(timing) {
  const sourceEnd = timing.sourceHoldMs;
  const transitionEnd = sourceEnd + timing.transitionMs;
  const targetEnd = transitionEnd + timing.targetHoldMs;
  const resetEnd = Math.max(timing.totalMs, targetEnd + timing.resetHoldMs);
  return {
    sourceHold: { startMs: 0, endMs: sourceEnd, progress: 0 },
    quickTransition: { startMs: sourceEnd, endMs: transitionEnd, progress: [0, 1] },
    targetHold: { startMs: transitionEnd, endMs: targetEnd, progress: 1 },
    resetHold: { startMs: targetEnd, endMs: resetEnd, progress: 0 },
  };
}

function positiveNumber(value, fallback) {
  const number = Number(value);
  return Number.isFinite(number) && number > 0 ? number : fallback;
}

function hasNonEmptyId(value) {
  return value != null && String(value).trim().length > 0;
}

function smoothstep(value) {
  const t = clamp01(value);
  return t * t * (3 - 2 * t);
}

function normalizeRgba(color, fallbackAlpha) {
  if (!(Array.isArray(color) || ArrayBuffer.isView(color))) return [0.8, 0.22, 0.12, fallbackAlpha];
  const source = Array.from(color);
  const divisor = Math.max(Number(source[0]) || 0, Number(source[1]) || 0, Number(source[2]) || 0) > 1 ? 255 : 1;
  return [
    clamp01((Number(source[0]) || 0) / divisor),
    clamp01((Number(source[1]) || 0) / divisor),
    clamp01((Number(source[2]) || 0) / divisor),
    clamp01(source[3] == null ? fallbackAlpha : Number(source[3])),
  ];
}

function clamp01(value) {
  return Math.max(0, Math.min(1, Number.isFinite(value) ? value : 0));
}
