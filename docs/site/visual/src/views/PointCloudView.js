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

export class PointCloudView extends BaseView {
  constructor(options = {}) {
    super({ ...options, kind: "point-cloud" });
    this.recordIds = inferRecordIds(options);
    this.positions = options.positions || options.coordinates || new Map();
    this.targetPositions = options.targetPositions || options.animationTargets;
    this.colors = options.colors;
    this.colorValues = options.colorValues || options.categoryValues;
    this.scalarValues = options.scalarValues;
    this.sizeValues = options.sizeValues;
    this.heightValues = options.heightValues;
    this.alphaValues = options.alphaValues;
    this.selectionValues = options.selectionValues;
    this.focusValues = options.focusValues;
    this.animationPhaseValues = options.animationPhaseValues;
    this.shape = options.shape || "point";
    this.size = Number.isFinite(Number(options.size)) ? Number(options.size) : 1;
    this.alpha = Number.isFinite(Number(options.alpha)) ? Number(options.alpha) : 1;
    this.scale = options.scale || [1, 1, 1];
    this.material = {
      lighting: "data-preserving",
      diffuse: "instance-color",
      ambient: 0.62,
      pointLight: 0.38,
      alphaMode: this.alpha < 1 ? "blend" : "opaque",
      ...(options.material || {}),
    };
    this.animation = options.animation || {
      mode: this.targetPositions ? "morph-target" : "none",
      channel: this.targetPositions ? "targetPosition" : undefined,
    };
  }

  static fromVisualSpace(document, options = {}) {
    const coordinate = resolveCollectionItem(document, "coordinates", options.coordinate || options.coordinateId || options.coordinate_id);
    const targetCoordinate = resolveCollectionItem(document, "coordinates", options.targetCoordinate || options.targetCoordinateId);
    const records = recordsFor(document, {
      ...options,
      datasetId: options.datasetId ?? coordinate?.dataset_id ?? coordinate?.datasetId,
    });
    const positions = extractCoordinatePositions(coordinate, { records, recordIds: options.recordIds });
    const targetPositions = targetCoordinate
      ? extractCoordinatePositions(targetCoordinate, { records, recordIds: positions.ids }).positions
      : undefined;
    const colorProperty = resolveCollectionItem(document, "properties", options.colorProperty || options.colorPropertyId);
    const scalarProperty = resolveCollectionItem(document, "properties", options.scalarProperty || options.scalarPropertyId);
    const sizeProperty = resolveCollectionItem(document, "properties", options.sizeProperty || options.sizePropertyId);
    const focusProperty = resolveCollectionItem(document, "properties", options.focusProperty || options.focusPropertyId);

    return new PointCloudView({
      ...options,
      records,
      recordIds: positions.ids,
      positions: positions.positions,
      targetPositions,
      datasetId: options.datasetId ?? coordinate?.dataset_id ?? coordinate?.datasetId,
      spaceId: options.spaceId ?? coordinate?.space_id ?? coordinate?.spaceId,
      coordinateId: coordinate?.id,
      propertyId: scalarProperty?.id ?? colorProperty?.id,
      colorValues: colorProperty ? extractPropertyValues(colorProperty, { records, recordIds: positions.ids }) : options.colorValues,
      scalarValues: scalarProperty ? extractPropertyValues(scalarProperty, { records, recordIds: positions.ids }) : options.scalarValues,
      sizeValues: sizeProperty ? extractPropertyValues(sizeProperty, { records, recordIds: positions.ids }) : options.sizeValues,
      focusValues: focusProperty ? extractPropertyValues(focusProperty, { records, recordIds: positions.ids }) : options.focusValues,
    });
  }

  toLayerDescriptors() {
    const count = this.recordIds.length;
    const sizeData = this.sizeValues
      ? flattenValues(this.sizeValues, this.recordIds, this.size)
      : new Float32Array(count).fill(this.size);
    const alphaData = this.alphaValues
      ? flattenValues(this.alphaValues, this.recordIds, this.alpha)
      : new Float32Array(count).fill(this.alpha);
    const heightData = this.heightValues
      ? flattenValues(this.heightValues, this.recordIds, 0)
      : new Float32Array(count);
    const scalarData = this.scalarValues
      ? flattenValues(this.scalarValues, this.recordIds, 0)
      : new Float32Array(count);
    const selectionData = this.selectionValues
      ? flattenValues(this.selectionValues, this.recordIds, 0)
      : new Float32Array(count);
    const focusData = this.focusValues
      ? flattenValues(this.focusValues, this.recordIds, 0)
      : new Float32Array(count);
    const phaseData = this.animationPhaseValues
      ? flattenValues(this.animationPhaseValues, this.recordIds, 0)
      : new Float32Array(count);
    const category = flattenCategories(this.colorValues || this.recordIds, this.recordIds);

    const channels = {
      recordId: createStringChannel(this.recordIds, "record-id"),
      position: createChannel(flattenVectors(this.positions, this.recordIds, 3, { scale: this.scale }), 3, "position"),
      color: colorChannelFrom({
        ids: this.recordIds,
        colors: this.colors,
        colorValues: this.colorValues,
        categoryValues: this.colorValues,
        scalarValues: this.scalarValues,
        alpha: this.alpha,
      }),
      size: createChannel(sizeData, 1, "size", { units: "scene" }),
      height: createChannel(heightData, 1, "height", { units: "scene" }),
      alpha: createChannel(alphaData, 1, "alpha"),
      category: createChannel(category.data, 1, "category", { categories: category.categories }),
      scalar: createChannel(scalarData, 1, "scalar"),
      selection: createChannel(selectionData, 1, "selection-state"),
      focusWeight: createChannel(focusData, 1, "focus-weight"),
      animationPhase: createChannel(phaseData, 1, "animation-phase"),
    };

    if (this.targetPositions) {
      channels.targetPosition = createChannel(
        flattenVectors(this.targetPositions, this.recordIds, 3, { scale: this.scale }),
        3,
        "target-position",
      );
    }

    return [new VisualLayer({
      id: `${this.id}:points`,
      kind: "point-cloud",
      primitive: "InstancedPointLayer",
      source: descriptorSource(this),
      channels,
      geometry: {
        shape: this.shape,
        instanceCount: count,
      },
      material: this.material,
      animation: this.animation,
      picking: {
        mode: "record-id",
        channel: "recordId",
      },
      metadata: {
        ...this.metadata,
        recordCount: count,
      },
    }).toDescriptor()];
  }
}

export function createPointCloudView(options) {
  return new PointCloudView(options);
}

