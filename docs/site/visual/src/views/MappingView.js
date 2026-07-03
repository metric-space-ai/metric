import { BaseView } from "./BaseView.js";
import { MetricSpaceView, defaultCoordinateId } from "./MetricSpaceView.js";
import { VisualLayer } from "./VisualLayer.js";
import {
  createTimelineAnimationDescriptor,
  createTimelineControlDescriptor,
  createTimelineEvidenceDescriptor,
  createTimelineModel,
} from "../timeline/index.js";
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
import { applyRelationEdgeLegibilityDescriptor } from "../relational/edge-legibility.js";

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
    this.timelineId = options.timelineId || options.timeline_id || options.metadata?.timelineId || null;
    this.timelineEvidence = options.timelineEvidence || options.mappingTimelineEvidence || options.metadata?.timelineEvidence || null;
    this.timelineAnimation = options.timelineAnimation || options.metadata?.timelineAnimation || null;
    this.timelineControl = options.timelineControl || options.metadata?.timelineControl || null;
    this.timelineCoordinateIds = options.timelineCoordinateIds || options.metadata?.timelineCoordinateIds || [];
    this.showResidualVectors = options.residualVectors !== false;
    this.residualVectorColor = options.residualVectorColor || options.vectorColor || null;
    this.residualVectorAlpha = Number.isFinite(Number(options.residualVectorAlpha))
      ? Number(options.residualVectorAlpha)
      : 0.58;
    this.residualVectorWidth = Number.isFinite(Number(options.residualVectorWidth))
      ? Number(options.residualVectorWidth)
      : 1;
    this.residualVectorOrder = Number.isFinite(Number(options.residualVectorOrder))
      ? Number(options.residualVectorOrder)
      : 36;
    this.residualVectorLimit = positiveInteger(
      options.residualVectorLimit ?? options.residualVectorSampleLimit ?? options.residualSampleLimit,
      Infinity,
    );
    this.residualVectorBucketCount = positiveInteger(options.residualVectorBucketCount ?? options.residualBuckets, 5);
    this.residualVectorSectorCount = positiveInteger(options.residualVectorSectorCount ?? options.residualSectors, 16);
    this.residualVectorRadialBuckets = positiveInteger(options.residualVectorRadialBuckets ?? options.residualRadialBuckets, 3);
    this.residualVectorLengthScale = positiveNumber(options.residualVectorLengthScale ?? options.vectorLengthScale, 1);
    this.residualVectorMaxLength = positiveFiniteNumberOrNull(options.residualVectorMaxLength ?? options.vectorMaxLength);
    this.residualVectorMinLength = positiveNumber(options.residualVectorMinLength ?? options.vectorMinLength, 0);
    this.residualVectorScaleByMagnitude = options.residualVectorScaleByMagnitude === true
      || options.scaleResidualVectorsByMagnitude === true;
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
    const explicitTimelineRef = options.timeline ?? options.timelineId ?? options.timeline_id;
    const explicitSourceRef = options.sourceCoordinate ?? options.sourceCoordinateId;
    const explicitTargetRef = options.targetCoordinate ?? options.targetCoordinateId;
    const shouldUseTimeline = explicitTimelineRef != null || (explicitSourceRef == null && explicitTargetRef == null);
    const timeline = shouldUseTimeline
      ? resolveMappingTimeline(document, explicitTimelineRef, {
        required: explicitTimelineRef != null,
        datasetId: options.datasetId ?? options.dataset_id ?? space?.dataset_id,
      })
      : null;
    const timelineModel = timeline
      ? createTimelineModel(document, {
        timelineId: timeline.id,
        loop: options.loop ?? true,
        playbackRate: options.playbackRate,
        easing: options.easing,
      })
      : null;
    const timelineSteps = timelineModel?.steps || [];
    const firstTimelineStep = timelineSteps[0] || null;
    const lastTimelineStep = timelineSteps[timelineSteps.length - 1] || null;
    const sourceRef = explicitSourceRef ?? firstTimelineStep?.coordinateId ?? defaultCoordinateId(document, space, { dimension: 2 });
    const targetRef = explicitTargetRef ?? lastTimelineStep?.coordinateId ?? defaultCoordinateId(document, space, { dimension: 3 });
    const sourceCoordinate = resolveCollectionItem(document, "coordinates", sourceRef, {
      required: sourceRef != null,
      label: explicitSourceRef != null ? "source coordinate" : "default source coordinate",
    });
    const targetCoordinate = resolveCollectionItem(document, "coordinates", targetRef, {
      required: targetRef != null,
      label: explicitTargetRef != null ? "target coordinate" : "default target coordinate",
    });
    const residualPropertyRef = options.residualProperty
      || options.residualPropertyId
      || options.scalarProperty
      || inferTimelineResidualPropertyRef(document, timelineSteps);
    const colorPropertyRef = options.colorProperty || options.colorPropertyId;
    const labelPropertyRef = options.labelProperty
      || options.labelPropertyId
      || (typeof options.labels === "string" ? options.labels : undefined)
      || inferTimelineLabelPropertyRef(document, timelineSteps);
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
    const timelineAnimation = timelineModel
      ? createTimelineAnimationDescriptor(timelineModel, {
        mode: "mapping-coordinate-morph",
        loop: options.loop ?? true,
        direction: options.direction || "alternate",
        durationMs: options.timelineDurationMs ?? options.durationMs ?? options.totalMs ?? options.morphDurationMs,
        minDurationMs: 5200,
        maxDurationMs: 18000,
      })
      : null;
    const timelineControl = timelineModel
      ? createTimelineControlDescriptor(timelineModel, {
        loop: options.loop ?? true,
        direction: options.direction || "alternate",
        label: options.timelineLabel || timeline?.name || "Mapping timeline",
      })
      : null;
    const timelineEvidence = timelineModel
      ? createTimelineEvidenceDescriptor(timelineModel, {
        loop: options.loop ?? true,
        direction: options.direction || "alternate",
        animation: timelineAnimation,
        control: timelineControl,
      })
      : null;

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
      timelineId: timeline?.id ?? options.timelineId,
      timelineCoordinateIds: timelineSteps.map((step) => step.coordinateId).filter(Boolean),
      timelineAnimation,
      timelineControl,
      timelineEvidence,
      metadata: {
        ...(options.metadata || {}),
        timelineId: timeline?.id ?? options.timelineId ?? null,
        timelineCoordinateIds: timelineSteps.map((step) => step.coordinateId).filter(Boolean),
        timelineEvidence,
        timelineAnimation,
        timelineControl,
      },
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
          timelineId: this.timelineId,
          timelineEvidence: this.timelineEvidence,
          timelineAnimation: this.timelineAnimation,
          timelineControl: this.timelineControl,
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
    const candidates = [];
    let count = 0;
    for (let index = 0; index < ids.length; index += 1) {
      const id = ids[index];
      const source = positionFor(this.space.positions, id);
      const target = positionFor(this.space.targetPositions, id);
      if (!source || !target) continue;
      const residual = Number(valueForId(this.residualValues, id, index));
      if (!Number.isFinite(residual)) continue;
      const length = distance3(source, target);
      if (!Number.isFinite(length)) continue;
      candidates.push({
        id: String(id),
        index,
        source,
        target,
        residual,
        length,
      });
    }
    if (!candidates.length) return null;
    const finiteResiduals = candidates.map((candidate) => candidate.residual);
    const domain = inferScalarDomain(finiteResiduals);
    const selected = selectResidualVectorRepresentatives(candidates, {
      limit: this.residualVectorLimit,
      bucketCount: this.residualVectorBucketCount,
      sectorCount: this.residualVectorSectorCount,
      radialBuckets: this.residualVectorRadialBuckets,
      domain,
    });
    const sourcePosition = new Float32Array(selected.length * 3);
    const targetPosition = new Float32Array(selected.length * 3);
    const color = new Float32Array(selected.length * 4);
    const residualMagnitude = new Float32Array(selected.length);
    const endpointEmphasis = new Float32Array(selected.length);
    const residualRecordIds = [];
    for (const candidate of selected) {
      const normalizedResidual = normalizeScalar(candidate.residual, domain);
      const vectorAlpha = residualAlphaFor(normalizedResidual, this.residualVectorAlpha);
      sourcePosition.set(candidate.source, count * 3);
      targetPosition.set(this.residualVectorEndpoint(candidate, normalizedResidual), count * 3);
      const nextColor = this.residualVectorColor
        ? normalizeRgba(this.residualVectorColor, vectorAlpha)
        : scalarColor(candidate.residual, domain, vectorAlpha);
      color.set(nextColor, count * 4);
      residualMagnitude[count] = candidate.residual;
      endpointEmphasis[count] = 0.18 + Math.sqrt(clamp01(normalizedResidual)) * 0.82;
      residualRecordIds.push(candidate.id);
      count += 1;
    }
    if (!count) return null;
    const residualSelection = residualSelectionMetadata({
      candidateCount: candidates.length,
      selectedCount: count,
      limit: this.residualVectorLimit,
      bucketCount: this.residualVectorBucketCount,
      sectorCount: this.residualVectorSectorCount,
      radialBuckets: this.residualVectorRadialBuckets,
    });
    const residualVectorLength = {
      scale: this.residualVectorLengthScale,
      maxLength: this.residualVectorMaxLength,
      minLength: this.residualVectorMinLength,
      scaleByMagnitude: this.residualVectorScaleByMagnitude,
    };
    const mappingEvidence = this.mappingMotionEvidenceDescriptor({
      residualVectorCount: count,
      nativeResidualRecordCount: candidates.length,
      residualSelection,
      residualVectorLength,
    });
    const descriptor = new VisualLayer({
      id: `${this.id}:residual-vectors`,
      kind: "residual/error-vectors",
      primitive: "RelationEdgeLayer",
      order: this.residualVectorOrder,
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
        endpointEmphasis: createChannel(endpointEmphasis.subarray(0, count), 1, "residual-endpoint-emphasis", {
          domain: [0, 1],
          source: "native-residual-magnitude",
        }),
        color: createChannel(color.subarray(0, count * 4), 4, "rgba"),
      },
      geometry: {
        width: this.residualVectorWidth,
        representativeResiduals: residualSelection.strategy,
        lengthClamp: this.residualVectorMaxLength,
      },
      material: { alpha: 1, transparent: true, depthWrite: false },
      metadata: {
        ...this.metadata,
        role: "residual/error",
        evidenceRole: "mapping-residual-vectors",
        diagnosticLayer: true,
        motionGrammar: "mapping-residual-error-scene",
        mappingEvidence,
        mappingMotionTiming: this.motionTiming,
        timelineId: this.timelineId,
        timelineEvidence: this.timelineEvidence,
        timelineAnimation: this.timelineAnimation,
        timelineControl: this.timelineControl,
        recordCount: count,
        sourceCoordinateId: this.sourceCoordinateId,
        targetCoordinateId: this.targetCoordinateId,
        residualPropertyId: this.propertyId,
        residualDomain: domain,
        nativeResidualRecordCount: candidates.length,
        residualSelection,
        residualVectorLength,
        algorithmicComputation: false,
      },
    }).toDescriptor();

    const legible = applyRelationEdgeLegibilityDescriptor(descriptor, {
      role: "residual/error",
      edgeCount: count,
      sourceEdgeCount: candidates.length,
      laneOffsetScale: 0,
      laneModulo: 1,
    });
    const edgeLegibility = enrichResidualLegibilityProfile(legible.metadata?.edgeLegibility, {
      rankAlphaBoost: 0.34,
      rank: {
        strategy: "native-residual-magnitude-representative-rank",
        source: "native-residual-magnitude",
        channel: "endpointEmphasis",
        direction: "high-value-important",
      },
      laneBundle: {
        strategy: residualSelection.strategy,
        bundleKey: "residualBucket:angularSector:radialBucket",
      },
      endpointEmphasis: {
        strategy: "residual-endpoint-magnitude",
        source: "residual-vector-endpoints",
        channel: "endpointEmphasis",
      },
      sampling: {
        strategy: residualSelection.strategy,
        sourceEdgeCount: candidates.length,
        renderedEdgeCount: count,
        sampled: candidates.length > count,
        preservesSelectionGraph: false,
      },
    });
    return {
      ...legible,
      geometry: {
        ...(legible.geometry || {}),
        edgeBundleKey: edgeLegibility.laneBundle.bundleKey,
        edgeBundleStrategy: edgeLegibility.laneBundle.strategy,
      },
      material: {
        ...(legible.material || {}),
        edgeRankAlphaBoost: edgeLegibility.rankAlphaBoost,
      },
      metadata: {
        ...(legible.metadata || {}),
        edgeLegibility,
      },
    };
  }

  residualVectorEndpoint(candidate, normalizedResidual) {
    const scaleByMagnitude = this.residualVectorScaleByMagnitude
      ? Math.max(0.16, Math.sqrt(clamp01(normalizedResidual)))
      : 1;
    const scaledLength = candidate.length * this.residualVectorLengthScale * scaleByMagnitude;
    const maxLength = this.residualVectorMaxLength;
    const minLength = Math.max(0, this.residualVectorMinLength);
    const clampedLength = maxLength == null
      ? Math.max(minLength, scaledLength)
      : Math.max(minLength, Math.min(maxLength, scaledLength));
    if (!(candidate.length > 0) || !Number.isFinite(clampedLength)) return candidate.target;
    const ratio = clampedLength / candidate.length;
    return [
      candidate.source[0] + (candidate.target[0] - candidate.source[0]) * ratio,
      candidate.source[1] + (candidate.target[1] - candidate.source[1]) * ratio,
      candidate.source[2] + (candidate.target[2] - candidate.source[2]) * ratio,
    ];
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
      timelineId: this.timelineId,
      timelineCoordinateIds: this.timelineCoordinateIds,
      timelineEvidenceSchema: this.timelineEvidence?.schema || null,
      motionContract: {
        mode: "coordinate-morph",
        channel: "targetPosition",
        residualLayer,
        timingProfile: this.motionTiming.profile,
        timingPhases: this.motionTiming.phases,
        controlledBy: "descriptor-animation",
        timelineId: this.timelineId,
        timelineSchema: this.timelineEvidence?.schema || null,
      },
      timelineEvidence: this.timelineEvidence,
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

function positiveInteger(value, fallback) {
  if (value === Infinity) return Infinity;
  const number = Number(value);
  return Number.isFinite(number) && number > 0 ? Math.floor(number) : fallback;
}

function positiveFiniteNumberOrNull(value) {
  const number = Number(value);
  return Number.isFinite(number) && number > 0 ? number : null;
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

function selectResidualVectorRepresentatives(candidates, options = {}) {
  const limit = options.limit === Infinity ? Infinity : positiveInteger(options.limit, Infinity);
  if (limit === Infinity || candidates.length <= limit) return candidates;
  const bucketCount = positiveInteger(options.bucketCount, 5);
  const sectorCount = positiveInteger(options.sectorCount, 16);
  const radialBuckets = positiveInteger(options.radialBuckets, 3);
  const centroid = sourceCentroid(candidates);
  const maxRadius = maxSourceRadius(candidates, centroid);
  const maxLength = maxCandidateLength(candidates);
  const groups = new Map();

  for (const candidate of candidates) {
    const residualT = normalizeScalar(candidate.residual, options.domain);
    const residualBucket = Math.min(bucketCount - 1, Math.floor(residualT * bucketCount));
    const dx = candidate.source[0] - centroid[0];
    const dz = candidate.source[2] - centroid[1];
    const angle = Math.atan2(dz, dx) + Math.PI;
    const sector = Math.min(sectorCount - 1, Math.floor((angle / (Math.PI * 2)) * sectorCount));
    const radiusT = clamp01(distance2(dx, dz) / maxRadius);
    const radialBucket = Math.min(radialBuckets - 1, Math.floor(radiusT * radialBuckets));
    const key = `${residualBucket}:${sector}:${radialBucket}`;
    const score = residualT + clamp01(candidate.length / maxLength) * 0.18;
    const previous = groups.get(key);
    if (!previous || score > previous.score || (score === previous.score && candidate.id < previous.candidate.id)) {
      groups.set(key, { candidate, score, residualBucket });
    }
  }

  const byBucket = new Map();
  for (const entry of groups.values()) {
    if (!byBucket.has(entry.residualBucket)) byBucket.set(entry.residualBucket, []);
    byBucket.get(entry.residualBucket).push(entry);
  }
  for (const entries of byBucket.values()) {
    entries.sort((a, b) => b.score - a.score || a.candidate.id.localeCompare(b.candidate.id));
  }

  const selected = [];
  const selectedIds = new Set();
  const bucketOrder = Array.from({ length: bucketCount }, (_, index) => bucketCount - index - 1);
  while (selected.length < limit) {
    let advanced = false;
    for (const bucket of bucketOrder) {
      const entries = byBucket.get(bucket);
      const entry = entries?.shift();
      if (!entry) continue;
      selected.push(entry.candidate);
      selectedIds.add(entry.candidate.id);
      advanced = true;
      if (selected.length >= limit) break;
    }
    if (!advanced) break;
  }

  if (selected.length < limit) {
    const fallback = candidates
      .filter((candidate) => !selectedIds.has(candidate.id))
      .map((candidate) => ({
        candidate,
        score: normalizeScalar(candidate.residual, options.domain) + clamp01(candidate.length / maxLength) * 0.18,
      }))
      .sort((a, b) => b.score - a.score || a.candidate.id.localeCompare(b.candidate.id));
    for (const entry of fallback) {
      selected.push(entry.candidate);
      if (selected.length >= limit) break;
    }
  }

  return selected.sort((a, b) => a.index - b.index);
}

function residualSelectionMetadata(options = {}) {
  const selectedCount = Number(options.selectedCount) || 0;
  const candidateCount = Number(options.candidateCount) || 0;
  const limit = options.limit === Infinity ? null : Number(options.limit) || null;
  return {
    strategy: selectedCount < candidateCount ? "representative-residual-buckets" : "complete-native-residuals",
    candidateCount,
    selectedCount,
    limit,
    residualBucketCount: options.bucketCount,
    angularSectorCount: options.sectorCount,
    radialBucketCount: options.radialBuckets,
  };
}

function sourceCentroid(candidates) {
  if (!candidates.length) return [0, 0];
  const sum = candidates.reduce((acc, candidate) => {
    acc[0] += candidate.source[0];
    acc[1] += candidate.source[2];
    return acc;
  }, [0, 0]);
  return [sum[0] / candidates.length, sum[1] / candidates.length];
}

function maxSourceRadius(candidates, centroid) {
  let maxRadius = 1;
  for (const candidate of candidates) {
    const radius = distance2(candidate.source[0] - centroid[0], candidate.source[2] - centroid[1]);
    if (radius > maxRadius) maxRadius = radius;
  }
  return maxRadius;
}

function maxCandidateLength(candidates) {
  let maxLength = 1;
  for (const candidate of candidates) {
    if (Number.isFinite(candidate.length) && candidate.length > maxLength) maxLength = candidate.length;
  }
  return maxLength;
}

function normalizeScalar(value, domain = {}) {
  const min = Number.isFinite(domain.min) ? domain.min : 0;
  const max = Number.isFinite(domain.max) ? domain.max : 1;
  if (max === min) return 0.5;
  return clamp01((Number(value) - min) / (max - min));
}

function residualAlphaFor(normalizedResidual, maxAlpha) {
  const alpha = Number.isFinite(Number(maxAlpha)) ? Number(maxAlpha) : 1;
  return clamp01(alpha * (0.28 + 0.72 * Math.sqrt(clamp01(normalizedResidual))));
}

function distance3(a, b) {
  return Math.hypot(
    Number(b?.[0]) - Number(a?.[0]),
    Number(b?.[1]) - Number(a?.[1]),
    Number(b?.[2]) - Number(a?.[2]),
  );
}

function distance2(x, y) {
  return Math.hypot(Number(x) || 0, Number(y) || 0);
}

function resolveMappingTimeline(document, ref, options = {}) {
  if (ref != null) {
    return resolveCollectionItem(document, "timelines", ref, {
      required: options.required,
      label: "mapping timeline",
    });
  }
  const timelines = Array.isArray(document?.timelines) ? document.timelines : [];
  const datasetId = options.datasetId;
  return timelines.find((timeline) => (
    (datasetId == null || String(timeline.dataset_id ?? timeline.datasetId) === String(datasetId))
    && /mapping|morph|coordinate|dimensional/i.test(`${timeline.id || ""} ${timeline.name || ""}`)
  )) || null;
}

function inferTimelineResidualPropertyRef(document, steps = []) {
  for (let index = steps.length - 1; index >= 0; index -= 1) {
    const propertyId = steps[index]?.propertyId;
    const property = resolveCollectionItem(document, "properties", propertyId);
    if (property && isRecordScalarProperty(property) && propertyLooksResidual(property)) return property.id;
  }
  return null;
}

function inferTimelineLabelPropertyRef(document, steps = []) {
  for (const step of steps) {
    const property = resolveCollectionItem(document, "properties", step?.propertyId);
    if (property && isRecordLabelProperty(property)) return property.id;
  }
  return null;
}

function isRecordScalarProperty(property) {
  const target = String(property?.target_type ?? property?.targetType ?? property?.target ?? "record").replace(/-/g, "_");
  const valueType = String(property?.value_type ?? property?.valueType ?? "").toLowerCase();
  return target === "record" && (!valueType || /scalar|number|float|double/.test(valueType));
}

function isRecordLabelProperty(property) {
  const target = String(property?.target_type ?? property?.targetType ?? property?.target ?? "record").replace(/-/g, "_");
  const valueType = String(property?.value_type ?? property?.valueType ?? "").toLowerCase();
  if (target !== "record") return false;
  if (/categorical|category|label|string|enum/.test(valueType)) return true;
  return /label|class|family|category|cluster|type/.test(`${property?.id || ""} ${property?.name || ""}`);
}

function propertyLooksResidual(property) {
  return /residual|error|distortion|preservation|loss|mse|objective/.test(`${property?.id || ""} ${property?.name || ""}`.toLowerCase());
}

function clamp01(value) {
  return Math.max(0, Math.min(1, Number.isFinite(value) ? value : 0));
}

function enrichResidualLegibilityProfile(profile = {}, options = {}) {
  const rank = {
    ...(options.rank || {}),
    deterministic: true,
    tieBreak: "stable-edge-id-then-index",
    usedFor: ["alpha-emphasis", "selection-legibility"],
  };
  const laneBundle = {
    ...(options.laneBundle || {}),
    laneStrategy: profile.laneStrategy || "none",
    laneCount: profile.laneStrategy === "none" ? 1 : Math.max(1, Number(profile.laneModulo) || 1),
    laneModulo: Math.max(1, Number(profile.laneModulo) || 1),
    laneOffsetScale: Number(profile.laneOffsetScale) || 0,
    deterministic: true,
    preservesTopology: true,
    geometryOnly: true,
  };
  return {
    ...profile,
    rankAlphaBoost: Number.isFinite(Number(options.rankAlphaBoost)) ? Number(options.rankAlphaBoost) : 0,
    alphaTransfer: "density-scale-with-rank-boost",
    rankSource: rank.source || profile.rankSource,
    rank,
    laneBundle,
    endpointEmphasis: {
      ...(options.endpointEmphasis || {}),
      affectsPicking: false,
      preservesEndpointIdentity: true,
    },
    sampling: {
      ...(options.sampling || {}),
      deterministic: true,
      syntheticEdges: false,
    },
  };
}
