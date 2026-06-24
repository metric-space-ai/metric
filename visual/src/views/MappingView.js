import { BaseView } from "./BaseView.js";
import { MetricSpaceView, defaultCoordinateId } from "./MetricSpaceView.js";
import {
  extractCoordinatePositions,
  extractPropertyValues,
  recordsFor,
  resolveCollectionItem,
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
    const space = resolveCollectionItem(document, "spaces", options.space || options.spaceId);
    const sourceRef = options.sourceCoordinate ?? options.sourceCoordinateId ?? defaultCoordinateId(document, space, { dimension: 2 });
    const targetRef = options.targetCoordinate ?? options.targetCoordinateId ?? defaultCoordinateId(document, space, { dimension: 3 });
    const sourceCoordinate = resolveCollectionItem(document, "coordinates", sourceRef);
    const targetCoordinate = resolveCollectionItem(document, "coordinates", targetRef);
    const residualProperty = resolveCollectionItem(document, "properties", options.residualProperty || options.residualPropertyId || options.scalarProperty);
    const colorProperty = resolveCollectionItem(document, "properties", options.colorProperty || options.colorPropertyId);

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
      datasetId,
      spaceId: options.spaceId ?? space?.id,
      coordinateId: sourceCoordinate?.id,
      sourceCoordinateId: sourceCoordinate?.id,
      targetCoordinateId: targetCoordinate?.id,
      propertyId: residualProperty?.id,
    });
  }

  toLayerDescriptors() {
    return this.space.toLayerDescriptors();
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
