import { PointCloudView } from "./PointCloudView.js";
import {
  extractCoordinatePositions,
  extractPropertyValues,
  recordsFor,
  resolveCollectionItem,
} from "./view-utils.js";

export class MorphView extends PointCloudView {
  constructor(options = {}) {
    super({
      ...options,
      targetPositions: options.targetPositions || options.toPositions,
      animation: {
        mode: "coordinate-morph",
        channel: "targetPosition",
        progress: Number.isFinite(Number(options.progress)) ? Number(options.progress) : 0,
        durationMs: Number.isFinite(Number(options.durationMs)) ? Number(options.durationMs) : 1200,
        easing: options.easing || "smoothstep",
        loop: Boolean(options.loop),
        direction: options.direction || "alternate",
        ...(options.animation || {}),
      },
    });
    this.kind = "morph";
    this.sourceCoordinateId = options.sourceCoordinateId || options.fromCoordinateId || this.coordinateId;
    this.targetCoordinateId = options.targetCoordinateId || options.toCoordinateId;
  }

  static fromVisualSpace(document, options = {}) {
    const sourceCoordinate = resolveCollectionItem(document, "coordinates", options.sourceCoordinate || options.fromCoordinate || options.sourceCoordinateId);
    const targetCoordinate = resolveCollectionItem(document, "coordinates", options.targetCoordinate || options.toCoordinate || options.targetCoordinateId);
    const colorProperty = resolveCollectionItem(document, "properties", options.colorProperty || options.colorPropertyId);
    const scalarProperty = resolveCollectionItem(document, "properties", options.scalarProperty || options.scalarPropertyId);
    const records = recordsFor(document, {
      ...options,
      datasetId: options.datasetId ?? sourceCoordinate?.dataset_id ?? sourceCoordinate?.datasetId,
    });
    const source = extractCoordinatePositions(sourceCoordinate, { records, recordIds: options.recordIds });
    const target = extractCoordinatePositions(targetCoordinate, { records, recordIds: source.ids });

    return new MorphView({
      ...options,
      records,
      recordIds: source.ids,
      positions: source.positions,
      targetPositions: target.positions,
      datasetId: options.datasetId ?? sourceCoordinate?.dataset_id ?? sourceCoordinate?.datasetId,
      spaceId: options.spaceId ?? sourceCoordinate?.space_id ?? sourceCoordinate?.spaceId,
      coordinateId: sourceCoordinate?.id,
      sourceCoordinateId: sourceCoordinate?.id,
      targetCoordinateId: targetCoordinate?.id,
      propertyId: scalarProperty?.id ?? colorProperty?.id,
      colorValues: colorProperty ? extractPropertyValues(colorProperty, { records, recordIds: source.ids }) : options.colorValues,
      scalarValues: scalarProperty ? extractPropertyValues(scalarProperty, { records, recordIds: source.ids }) : options.scalarValues,
    });
  }

  toLayerDescriptors() {
    const descriptors = super.toLayerDescriptors();
    return descriptors.map((descriptor) => ({
      ...descriptor,
      id: descriptor.id.replace(":points", ":morph"),
      kind: "morph",
      source: {
        ...descriptor.source,
        viewKind: "morph",
        sourceCoordinateId: this.sourceCoordinateId,
        targetCoordinateId: this.targetCoordinateId,
      },
      geometry: {
        ...descriptor.geometry,
        shape: this.shape || "point",
      },
      animation: {
        mode: "coordinate-morph",
        channel: "targetPosition",
        requiresChannels: ["position", "targetPosition"],
        ...this.animation,
      },
    }));
  }
}

export function createMorphView(options) {
  return new MorphView(options);
}

