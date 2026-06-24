import { BaseView } from "./BaseView.js";
import { MetricSpaceView, defaultCoordinateId } from "./MetricSpaceView.js";
import { VisualLayer } from "./VisualLayer.js";
import {
  colorChannelFrom,
  createChannel,
  createStringChannel,
  descriptorSource,
  extractCoordinatePositions,
  extractPropertyValues,
  flattenValues,
  flattenVectors,
  inferScalarDomain,
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
    this.positions = options.positions || options.coordinates || new Map();
    this.histogramBins = Number.isFinite(Number(options.histogramBins)) ? Number(options.histogramBins) : 12;
    this.topCount = Number.isFinite(Number(options.topCount)) ? Number(options.topCount) : 8;
    this.propertyName = options.propertyName || options.name || options.propertyId || this.propertyId;
    this.field = options.field !== false && options.propertyField !== false;
    this.fieldMode = options.fieldMode || options.fieldGrammar || "ground";
    this.fieldAlpha = Number.isFinite(Number(options.fieldAlpha ?? options.alpha)) ? Number(options.fieldAlpha ?? options.alpha) : 0.34;
    this.fieldRadius = Number.isFinite(Number(options.fieldRadius ?? options.radius)) ? Number(options.fieldRadius ?? options.radius) : 0.22;
    this.fieldLift = Number.isFinite(Number(options.fieldLift)) ? Number(options.fieldLift) : 0.32;
    this.fieldMaterial = options.fieldMaterial || options.material || {};
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
      propertyName: property?.name,
      name: options.name || property?.name || property?.id,
    });
  }

  toLayerDescriptors() {
    const descriptors = this.space.toLayerDescriptors();
    const field = this.field ? this.fieldDescriptor() : null;
    if (field) descriptors.splice(this.space.showGround ? 1 : 0, 0, field);
    return descriptors;
  }

  fieldDescriptor() {
    if (!this.recordIds.length || !this.scalarValues) return null;
    const rawScalar = flattenValues(this.scalarValues, this.recordIds, 0);
    const domain = inferScalarDomain(Array.from(rawScalar));
    const positions = flattenVectors(this.positions, this.recordIds, 3);
    const fieldPositions = propertyFieldPositions(positions, rawScalar, domain, {
      mode: this.fieldMode,
      groundY: this.space.groundY,
      lift: this.fieldLift,
    });
    const radius = new Float32Array(this.recordIds.length).fill(this.fieldRadius);
    const alpha = new Float32Array(this.recordIds.length).fill(this.fieldAlpha);
    const selection = new Float32Array(this.recordIds.length);
    const semantic = propertyFieldSemantic(this.propertyId, this.propertyName);

    return new VisualLayer({
      id: `${this.id}:property-field`,
      kind: "property-field",
      primitive: "HeatFieldLayer",
      order: -3,
      source: descriptorSource(this, {
        propertyField: true,
        scalarSource: "exported-property",
      }),
      channels: {
        recordId: createStringChannel(this.recordIds, "record-id"),
        sourcePosition: createChannel(positions, 3, "source-position"),
        position: createChannel(fieldPositions, 3, "property-field-position"),
        scalar: createChannel(rawScalar, 1, "property-scalar", { domain }),
        color: colorChannelFrom({
          ids: this.recordIds,
          scalarValues: this.scalarValues,
          alpha: this.fieldAlpha,
        }),
        radius: createChannel(radius, 1, "influence-radius"),
        alpha: createChannel(alpha, 1, "alpha"),
        selection: createChannel(selection, 1, "selection-state"),
      },
      geometry: {
        mode: this.fieldMode === "lifted" ? "lifted-property-field" : "ground-property-field",
        plane: "xz",
        sampleCount: this.recordIds.length,
        interpolation: "renderer-defined",
        radius: this.fieldRadius,
      },
      material: {
        lighting: "field",
        diffuse: "scalar-ramp",
        ramp: semantic,
        alphaMode: "blend",
        alpha: this.fieldAlpha,
        contour: 0.14,
        glow: 0.1,
        depthWrite: false,
        ...(this.fieldMaterial || {}),
      },
      animation: { mode: "none" },
      picking: {
        mode: "record-id",
        channel: "recordId",
      },
      metadata: {
        ...this.metadata,
        role: "property-field",
        propertyId: this.propertyId,
        propertyName: this.propertyName,
        propertySemantic: semantic,
        scalarDomain: domain,
        recordCount: this.recordIds.length,
        fieldMode: this.fieldMode === "lifted" ? "lifted" : "ground",
        source: "exported-scalar-property",
      },
    }).toDescriptor();
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

function propertyFieldPositions(sourcePositions, scalarValues, domain, options = {}) {
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

function propertyFieldSemantic(propertyId, propertyName) {
  const text = `${propertyId || ""} ${propertyName || ""}`.toLowerCase();
  if (/density/.test(text)) return "density";
  if (/anomaly|outlier|noise|fault/.test(text)) return "anomaly";
  if (/entropy|uncertainty/.test(text)) return "entropy";
  if (/residual|error/.test(text)) return "residual";
  return "scalar";
}
