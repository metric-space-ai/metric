import { BaseView } from "./BaseView.js";
import { VisualLayer } from "./VisualLayer.js";
import {
  colorChannelFrom,
  createChannel,
  createStringChannel,
  descriptorSource,
  extractCoordinatePositions,
  extractPropertyValues,
  flattenCategories,
  flattenValues,
  flattenVectors,
  inferRecordIds,
  recordsFor,
  resolveCollectionItem,
} from "./view-utils.js";

export class ClusterView extends BaseView {
  constructor(options = {}) {
    super({ ...options, kind: "cluster" });
    this.recordIds = inferRecordIds(options);
    this.positions = options.positions || options.coordinates || new Map();
    this.clusterValues = options.clusterValues || options.categories || options.values || new Map();
    this.confidenceValues = options.confidenceValues || options.scalarValues;
    this.sizeValues = options.sizeValues;
    this.selectionValues = options.selectionValues;
    this.focusValues = options.focusValues;
    this.alpha = Number.isFinite(Number(options.alpha)) ? Number(options.alpha) : 0.94;
    this.size = Number.isFinite(Number(options.size)) ? Number(options.size) : 1;
    this.shape = options.shape || "disc";
    this.footprints = options.footprints;
    this.material = {
      lighting: "data-preserving",
      diffuse: "category-palette",
      ambient: 0.58,
      pointLight: 0.34,
      alphaMode: this.alpha < 1 ? "blend" : "opaque",
      ...(options.material || {}),
    };
  }

  static fromVisualSpace(document, options = {}) {
    const coordinate = resolveCollectionItem(document, "coordinates", options.coordinate || options.coordinateId || options.coordinate_id);
    const clusterProperty = resolveCollectionItem(document, "properties", options.clusterProperty || options.clusterPropertyId || options.property || options.propertyId);
    const confidenceProperty = resolveCollectionItem(document, "properties", options.confidenceProperty || options.confidencePropertyId);
    const records = recordsFor(document, {
      ...options,
      datasetId: options.datasetId ?? clusterProperty?.dataset_id ?? coordinate?.dataset_id,
    });
    const positions = extractCoordinatePositions(coordinate, { records, recordIds: options.recordIds });

    return new ClusterView({
      ...options,
      records,
      recordIds: positions.ids,
      positions: positions.positions,
      clusterValues: clusterProperty ? extractPropertyValues(clusterProperty, { records, recordIds: positions.ids }) : options.clusterValues,
      confidenceValues: confidenceProperty ? extractPropertyValues(confidenceProperty, { records, recordIds: positions.ids }) : options.confidenceValues,
      datasetId: options.datasetId ?? clusterProperty?.dataset_id ?? clusterProperty?.datasetId ?? coordinate?.dataset_id,
      spaceId: options.spaceId ?? coordinate?.space_id ?? coordinate?.spaceId,
      coordinateId: coordinate?.id,
      propertyId: clusterProperty?.id,
    });
  }

  toLayerDescriptors() {
    const category = flattenCategories(this.clusterValues, this.recordIds);
    const confidence = this.confidenceValues
      ? flattenValues(this.confidenceValues, this.recordIds, 0)
      : new Float32Array(this.recordIds.length);
    const size = this.sizeValues
      ? flattenValues(this.sizeValues, this.recordIds, this.size)
      : new Float32Array(this.recordIds.length).fill(this.size);
    const selection = this.selectionValues
      ? flattenValues(this.selectionValues, this.recordIds, 0)
      : new Float32Array(this.recordIds.length);
    const focus = this.focusValues
      ? flattenValues(this.focusValues, this.recordIds, 0)
      : new Float32Array(this.recordIds.length);
    const alpha = new Float32Array(this.recordIds.length).fill(this.alpha);

    const descriptors = [new VisualLayer({
      id: `${this.id}:clusters`,
      kind: "cluster",
      primitive: "InstancedGlyphLayer",
      source: descriptorSource(this),
      channels: {
        recordId: createStringChannel(this.recordIds, "record-id"),
        position: createChannel(flattenVectors(this.positions, this.recordIds, 3), 3, "position"),
        color: colorChannelFrom({
          ids: this.recordIds,
          colorValues: this.clusterValues,
          categoryValues: this.clusterValues,
          alpha: this.alpha,
        }),
        category: createChannel(category.data, 1, "category", { categories: category.categories }),
        scalar: createChannel(confidence, 1, "cluster-confidence"),
        size: createChannel(size, 1, "glyph-size"),
        alpha: createChannel(alpha, 1, "alpha"),
        selection: createChannel(selection, 1, "selection-state"),
        focusWeight: createChannel(focus, 1, "focus-weight"),
      },
      geometry: {
        shape: this.shape,
        instanceCount: this.recordIds.length,
      },
      material: this.material,
      animation: {
        mode: "attention",
        channels: ["selection", "focusWeight"],
      },
      picking: {
        mode: "record-id",
        channel: "recordId",
      },
      metadata: {
        ...this.metadata,
        clusters: category.categories,
        recordCount: this.recordIds.length,
      },
    }).toDescriptor()];

    if (this.footprints) {
      descriptors.push(new VisualLayer({
        id: `${this.id}:footprints`,
        kind: "cluster-footprint",
        primitive: "GroundProjectionLayer",
        source: descriptorSource(this, { footprintSource: "exported" }),
        channels: {
          category: createChannel(category.data, 1, "category", { categories: category.categories }),
        },
        geometry: {
          mode: "cluster-footprint",
          footprints: this.footprints,
          derivation: "exported",
        },
        material: {
          lighting: "projection",
          diffuse: "category-palette",
          alphaMode: "blend",
        },
        animation: { mode: "none" },
        metadata: {
          footprintCount: Array.isArray(this.footprints) ? this.footprints.length : undefined,
        },
      }).toDescriptor());
    }

    return descriptors;
  }
}

export function createClusterView(options) {
  return new ClusterView(options);
}

