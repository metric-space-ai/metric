import { BaseView } from "./BaseView.js";
import { MetricSpaceView, defaultCoordinateId } from "./MetricSpaceView.js";
import {
  extractCoordinatePositions,
  extractPropertyValues,
  recordsFor,
  resolveCollectionItem,
} from "./view-utils.js";

/**
 * SpacePropertiesView overlays an exported per-record scalar property (entropy,
 * density, anomaly, outlier score, residual, ...) on the metric space. It draws
 * the points with a scalar color ramp, projects a semantic density field onto
 * the ground, and exposes a property summary (histogram + ranked records) for
 * non-GPU panels. All values come from the exported property.
 */
export class SpacePropertiesView extends BaseView {
  constructor(options = {}) {
    super({ ...options, kind: "space-properties" });
    this.scalarValues = options.scalarValues || new Map();
    this.recordIds = options.recordIds || Array.from(this.scalarValues.keys?.() || []);
    this.histogramBins = Number.isFinite(Number(options.histogramBins)) ? Number(options.histogramBins) : 12;
    this.topCount = Number.isFinite(Number(options.topCount)) ? Number(options.topCount) : 8;
    this.space = new MetricSpaceView({
      ...options,
      id: `${this.id}:space`,
      scalarValues: this.scalarValues,
      colorValues: undefined,
      ground: options.ground !== false,
      groundProjection: options.groundProjection !== false,
    });
  }

  static fromVisualSpace(document, options = {}) {
    const property = resolveCollectionItem(document, "properties", options.property || options.propertyId || options.property_id)
      || firstScalarProperty(document);
    const space = resolveCollectionItem(document, "spaces", options.space || options.spaceId);
    const coordinateRef = options.coordinate ?? options.coordinateId ?? defaultCoordinateId(document, space, { dimension: 3 });
    const coordinate = resolveCollectionItem(document, "coordinates", coordinateRef);
    const datasetId = options.datasetId ?? property?.dataset_id ?? coordinate?.dataset_id ?? space?.dataset_id;
    const records = recordsFor(document, { ...options, datasetId });
    const positions = extractCoordinatePositions(coordinate, { records, recordIds: options.recordIds || space?.record_ids });
    const scalarValues = property ? extractPropertyValues(property, { records, recordIds: positions.ids }) : new Map();

    return new SpacePropertiesView({
      ...options,
      records,
      recordIds: positions.ids,
      positions: positions.positions,
      scalarValues,
      datasetId,
      spaceId: options.spaceId ?? space?.id ?? coordinate?.space_id,
      coordinateId: coordinate?.id,
      propertyId: property?.id,
      name: options.name || property?.name || property?.id,
    });
  }

  toLayerDescriptors() {
    return this.space.toLayerDescriptors();
  }

  /**
   * Summarize the property for panels: domain, histogram and ranked records.
   * Pure data reduction over already-exported values.
   */
  summary() {
    const entries = this.recordIds
      .map((id) => ({ recordId: id, value: Number(this.scalarValues.get?.(id) ?? this.scalarValues.get?.(String(id))) }))
      .filter((entry) => Number.isFinite(entry.value));
    if (!entries.length) {
      return { count: 0, domain: { min: 0, max: 0 }, histogram: [], top: [], bottom: [] };
    }
    let min = Infinity;
    let max = -Infinity;
    for (const entry of entries) {
      min = Math.min(min, entry.value);
      max = Math.max(max, entry.value);
    }
    const span = max - min || 1;
    const bins = Array.from({ length: this.histogramBins }, () => 0);
    for (const entry of entries) {
      const bin = Math.min(this.histogramBins - 1, Math.floor(((entry.value - min) / span) * this.histogramBins));
      bins[bin] += 1;
    }
    const sorted = entries.slice().sort((a, b) => b.value - a.value);
    return {
      count: entries.length,
      domain: { min, max },
      histogram: bins.map((count, index) => ({
        index,
        count,
        from: min + (index / this.histogramBins) * span,
        to: min + ((index + 1) / this.histogramBins) * span,
      })),
      top: sorted.slice(0, this.topCount),
      bottom: sorted.slice(-this.topCount).reverse(),
    };
  }
}

export function createSpacePropertiesView(options) {
  return new SpacePropertiesView(options);
}

function firstScalarProperty(document) {
  const properties = Array.isArray(document?.properties) ? document.properties : [];
  return properties.find((property) => property.value_type === "scalar") || properties[0] || null;
}
