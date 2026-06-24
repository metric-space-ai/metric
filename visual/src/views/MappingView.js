import { BaseView } from "./BaseView.js";
import { MetricSpaceView, defaultCoordinateId } from "./MetricSpaceView.js";
import { VisualLayer } from "./VisualLayer.js";
import {
  createChannel,
  extractCoordinatePositions,
  extractPropertyValues,
  inferScalarDomain,
  recordsFor,
  resolveCollectionItem,
  scalarColor,
  valueForId,
} from "./view-utils.js";

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
    this.space = new MetricSpaceView({
      ...options,
      id: `${this.id}:space`,
      scalarValues: this.residualValues || undefined,
      colorValues: this.residualValues ? undefined : options.colorValues,
      ground: options.ground !== false,
      groundProjection: options.groundProjection !== false,
      loop: options.loop ?? true,
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
    const descriptors = this.space.toLayerDescriptors();
    const residual = this.residualVectorDescriptor();
    if (residual) descriptors.push(residual);
    return descriptors;
  }

  residualVectorDescriptor() {
    if (!this.showResidualVectors || !this.space?.targetPositions) return null;
    const ids = this.recordIds || this.space.recordIds || [];
    const sourcePosition = new Float32Array(ids.length * 3);
    const targetPosition = new Float32Array(ids.length * 3);
    const color = new Float32Array(ids.length * 4);
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
      count += 1;
    }
    if (!count) return null;
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
        sourcePosition: createChannel(sourcePosition.subarray(0, count * 3), 3, "source-position"),
        targetPosition: createChannel(targetPosition.subarray(0, count * 3), 3, "target-position"),
        color: createChannel(color.subarray(0, count * 4), 4, "rgba"),
      },
      geometry: { width: this.residualVectorWidth },
      material: { alpha: 1, transparent: true, depthWrite: false },
      metadata: {
        ...this.metadata,
        role: "residual/error",
        evidenceRole: "mapping-residual-vectors",
        recordCount: count,
        sourceCoordinateId: this.sourceCoordinateId,
        targetCoordinateId: this.targetCoordinateId,
        residualPropertyId: this.propertyId,
        algorithmicComputation: false,
      },
    }).toDescriptor();
  }

  /**
   * Summarize residual/preservation evidence for panels. Pure reduction over
   * the exported residual property.
   */
  preservationSummary() {
    if (!this.residualValues) return { count: 0, meanResidual: 0, maxResidual: 0 };
    const values = this.recordIds
      .map((id) => Number(this.residualValues.get?.(id) ?? this.residualValues.get?.(String(id))))
      .filter((value) => Number.isFinite(value));
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
}

export function createMappingView(options) {
  return new MappingView(options);
}

function positionFor(map, id) {
  return map?.get?.(id) || map?.get?.(String(id)) || null;
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
