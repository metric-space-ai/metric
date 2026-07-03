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

export class GroundProjectionView extends BaseView {
  constructor(options = {}) {
    super({ ...options, kind: "ground-projection" });
    this.recordIds = inferRecordIds(options);
    this.positions = options.positions || options.coordinates || new Map();
    this.projectionPositions = options.projectionPositions || options.targetPositions;
    this.colors = options.colors;
    this.colorValues = options.colorValues || options.categoryValues;
    this.scalarValues = options.scalarValues;
    this.alphaValues = options.alphaValues;
    this.sizeValues = options.sizeValues;
    this.groundY = Number.isFinite(Number(options.groundY)) ? Number(options.groundY) : 0;
    this.projectionDimension = Number(options.projectionDimension) || 3;
    this.size = Number.isFinite(Number(options.size)) ? Number(options.size) : 1;
    this.alpha = Number.isFinite(Number(options.alpha)) ? Number(options.alpha) : 0.42;
    this.plane = options.plane || "xz";
    this.mode = options.mode || "semantic-shadow";
    this.material = {
      lighting: "projection",
      diffuse: "instance-color",
      alphaMode: "blend",
      ...(options.material || {}),
    };
  }

  static fromVisualSpace(document, options = {}) {
    const coordinate = resolveCollectionItem(document, "coordinates", options.coordinate || options.coordinateId || options.coordinate_id);
    const projectionCoordinate = resolveCollectionItem(document, "coordinates", options.projectionCoordinate || options.projectionCoordinateId);
    const property = resolveCollectionItem(document, "properties", options.property || options.propertyId || options.property_id);
    const records = recordsFor(document, {
      ...options,
      datasetId: options.datasetId ?? coordinate?.dataset_id ?? coordinate?.datasetId,
    });
    const positions = extractCoordinatePositions(coordinate, { records, recordIds: options.recordIds });
    const projectionPositions = projectionCoordinate
      ? extractCoordinatePositions(projectionCoordinate, { records, recordIds: positions.ids }).positions
      : undefined;

    return new GroundProjectionView({
      ...options,
      records,
      recordIds: positions.ids,
      positions: positions.positions,
      projectionPositions,
      colorValues: property ? extractPropertyValues(property, { records, recordIds: positions.ids }) : options.colorValues,
      scalarValues: property ? extractPropertyValues(property, { records, recordIds: positions.ids }) : options.scalarValues,
      datasetId: options.datasetId ?? coordinate?.dataset_id ?? coordinate?.datasetId,
      spaceId: options.spaceId ?? coordinate?.space_id ?? coordinate?.spaceId,
      coordinateId: coordinate?.id,
      propertyId: property?.id,
      projectionDimension: Number(projectionCoordinate?.dimension) || options.projectionDimension || 3,
    });
  }

  toLayerDescriptors() {
    const sourcePositions = flattenVectors(this.positions, this.recordIds, 3);
    const projectedPositions = this.projectionPositions
      ? projectToGround(flattenVectors(this.projectionPositions, this.recordIds, 3), this.groundY, this.projectionDimension)
      : projectToGround(sourcePositions, this.groundY, 3);
    const alpha = this.alphaValues
      ? flattenValues(this.alphaValues, this.recordIds, this.alpha)
      : new Float32Array(this.recordIds.length).fill(this.alpha);
    const size = this.sizeValues
      ? flattenValues(this.sizeValues, this.recordIds, this.size)
      : new Float32Array(this.recordIds.length).fill(this.size);
    const scalar = this.scalarValues
      ? flattenValues(this.scalarValues, this.recordIds, 0)
      : new Float32Array(this.recordIds.length);
    const category = flattenCategories(this.colorValues || this.recordIds, this.recordIds);

    return [new VisualLayer({
      id: `${this.id}:projection`,
      kind: "ground-projection",
      primitive: "GroundProjectionLayer",
      source: descriptorSource(this),
      channels: {
        recordId: createStringChannel(this.recordIds, "record-id"),
        sourcePosition: createChannel(sourcePositions, 3, "source-position"),
        position: createChannel(projectedPositions, 3, "projected-position"),
        color: colorChannelFrom({
          ids: this.recordIds,
          colors: this.colors,
          colorValues: this.colorValues,
          scalarValues: this.scalarValues,
          alpha: this.alpha,
        }),
        size: createChannel(size, 1, "projection-size"),
        alpha: createChannel(alpha, 1, "alpha"),
        category: createChannel(category.data, 1, "category", { categories: category.categories }),
        scalar: createChannel(scalar, 1, "scalar"),
      },
      geometry: {
        mode: this.mode,
        plane: this.plane,
        groundY: this.groundY,
        projectionSource: this.projectionPositions ? "coordinate-state" : "axis-drop",
      },
      material: this.material,
      animation: { mode: "none" },
      picking: {
        mode: "record-id",
        channel: "recordId",
      },
      metadata: {
        ...this.metadata,
        recordCount: this.recordIds.length,
      },
    }).toDescriptor()];
  }
}

export function createGroundProjectionView(options) {
  return new GroundProjectionView(options);
}

function projectToGround(sourcePositions, groundY, sourceDimension) {
  const projected = new Float32Array(sourcePositions.length);
  for (let index = 0; index < sourcePositions.length; index += 3) {
    projected[index] = sourcePositions[index];
    projected[index + 1] = groundY;
    projected[index + 2] = sourceDimension === 2 ? sourcePositions[index + 1] : sourcePositions[index + 2];
  }
  return projected;
}
