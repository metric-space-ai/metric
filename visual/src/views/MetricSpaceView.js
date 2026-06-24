import { BaseView } from "./BaseView.js";
import { PointCloudView } from "./PointCloudView.js";
import { VisualLayer } from "./VisualLayer.js";
import { applyPositionFit, computePositionFit } from "./scene-fit.js";
import {
  colorChannelFrom,
  createChannel,
  createStringChannel,
  descriptorSource,
  extractCoordinatePositions,
  extractPropertyValues,
  flattenValues,
  flattenVectors,
  inferRecordIds,
  recordsFor,
  resolveCollectionItem,
} from "./view-utils.js";

// Crisp miniature material: opaque sphere-shaded points with a specular
// highlight and a dark edge, so each record reads as a sharp little model ball.
// This is the canonical dense record-cloud look. It is shared by metric-space
// point-cloud views, but it is not the universal grammar for all METRIC heroes.
// Translucent discs produced soft fog and are not used.
const CRISP_POINT_MATERIAL = Object.freeze({
  pointPixelScale: 9,
  minPointSize: 2.5,
  maxPointSize: 30,
  alphaMode: "blend",
  transparent: true,
  alpha: 0.95,
  ambient: 0.5,
  pointLight: 0.5,
  sphereShade: 0.78,
  gloss: 0.42,
  specular: 0.5,
  specularPower: 36,
  edgeShade: 0.5,
  saturation: 1.16,
  focusBoost: 0.28,
});

/**
 * MetricSpaceView is the dense coordinate-state grammar for a finite metric
 * space: a ground plane, a semantic ground projection, region labels and an
 * instanced point cloud, with an optional source->target morph.
 *
 * It is one semantic view in the engine, not the engine itself. Relation
 * matrices, graphs, property fields, typed glyph scenes, dynamics and mapping
 * residuals use their own semantic views while sharing the same runtime and
 * layer descriptor pipeline.
 *
 * It is a semantic view: it never computes metric values. It only reads an
 * exported coordinate state plus optional color/scalar/label properties.
 */
export class MetricSpaceView extends BaseView {
  constructor(options = {}) {
    super({ ...options, kind: "metric-space" });
    this.recordIds = inferRecordIds(options);

    this.fitEnabled = options.fit !== false;
    this.groundY = Number.isFinite(Number(options.groundY)) ? Number(options.groundY) : 0;
    this.targetRadius = Number.isFinite(Number(options.targetRadius)) ? Number(options.targetRadius) : 1.6;

    const rawPositions = options.positions || options.coordinates || new Map();
    const rawTargetPositions = options.targetPositions || options.toPositions || null;
    this.fit = this.fitEnabled
      ? computePositionFit(rawPositions, this.recordIds, {
        targetRadius: this.targetRadius,
        groundY: this.groundY,
        groundAlign: options.groundAlign !== false,
      })
      : null;
    this.positions = this.fit ? applyPositionFit(rawPositions, this.recordIds, this.fit.transform) : rawPositions;
    this.targetPositions = rawTargetPositions
      ? (this.fit ? applyPositionFit(rawTargetPositions, this.recordIds, this.fit.transform) : rawTargetPositions)
      : null;

    this.colors = options.colors;
    this.colorValues = options.colorValues || options.categoryValues;
    this.scalarValues = options.scalarValues;
    this.sizeValues = options.sizeValues;
    this.size = Number.isFinite(Number(options.size)) ? Number(options.size) : 1;
    this.alpha = Number.isFinite(Number(options.alpha)) ? Number(options.alpha) : 0.95;
    this.shape = options.shape || "sphere";
    this.pointMaterial = { ...CRISP_POINT_MATERIAL, ...(options.pointMaterial || {}) };

    // Region labels (categorical), placed at per-category centroids.
    this.labelValues = options.labelValues || null;
    this.labelColor = options.labelColor || [0.13, 0.17, 0.19, 1];
    this.labelFontSize = Number.isFinite(Number(options.labelFontSize)) ? Number(options.labelFontSize) : 28;
    this.labelLift = Number.isFinite(Number(options.labelLift)) ? Number(options.labelLift) : 0.5;
    this.labelOffsetRadius = Number.isFinite(Number(options.labelOffsetRadius)) ? Number(options.labelOffsetRadius) : 0;
    this.labelMap = options.labelMap || null;
    this.showLabels = options.labels !== false && Boolean(this.labelValues);

    this.showGround = options.ground !== false;
    this.showGroundProjection = options.groundProjection !== false;
    this.groundScale = Number.isFinite(Number(options.groundScale)) ? Number(options.groundScale) : 2.2;
    this.groundGeometry = options.groundGeometry || {};
    this.groundMaterial = options.groundMaterial || {};
    this.groundProjectionAlpha = Number.isFinite(Number(options.groundProjectionAlpha))
      ? Number(options.groundProjectionAlpha)
      : 0.42;
    this.groundProjectionMaterial = options.groundProjectionMaterial || {};
    this.morphProgress = Number.isFinite(Number(options.progress)) ? Number(options.progress) : 0;
    this.morphLoop = Boolean(options.loop ?? Boolean(this.targetPositions));
    this.durationMs = Number.isFinite(Number(options.durationMs)) ? Number(options.durationMs) : 5200;
    this.sourceCoordinateId = options.sourceCoordinateId || this.coordinateId;
    this.targetCoordinateId = options.targetCoordinateId || null;
  }

  static fromVisualSpace(document, options = {}) {
    const space = resolveCollectionItem(document, "spaces", options.space || options.spaceId || options.space_id);
    const coordinateRef = options.coordinate
      ?? options.coordinateId
      ?? options.coordinate_id
      ?? defaultCoordinateId(document, space, { dimension: 3 });
    const coordinate = resolveCollectionItem(document, "coordinates", coordinateRef);
    const targetCoordinate = resolveCollectionItem(document, "coordinates", options.targetCoordinate || options.targetCoordinateId);

    const datasetId = options.datasetId ?? coordinate?.dataset_id ?? coordinate?.datasetId ?? space?.dataset_id;
    const records = recordsFor(document, { ...options, datasetId });
    const positions = extractCoordinatePositions(coordinate, { records, recordIds: options.recordIds || space?.record_ids });
    const targetPositions = targetCoordinate
      ? extractCoordinatePositions(targetCoordinate, { records, recordIds: positions.ids }).positions
      : null;

    const colorProperty = resolveCollectionItem(document, "properties", options.colorProperty || options.colorPropertyId);
    const scalarProperty = resolveCollectionItem(document, "properties", options.scalarProperty || options.scalarPropertyId);
    const sizeProperty = resolveCollectionItem(document, "properties", options.sizeProperty || options.sizePropertyId);
    const labelProperty = resolveCollectionItem(document, "properties", options.labelProperty || options.labelPropertyId);

    return new MetricSpaceView({
      ...options,
      records,
      recordIds: positions.ids,
      positions: positions.positions,
      targetPositions,
      datasetId,
      spaceId: options.spaceId ?? space?.id ?? coordinate?.space_id,
      coordinateId: coordinate?.id,
      sourceCoordinateId: coordinate?.id,
      targetCoordinateId: targetCoordinate?.id ?? null,
      propertyId: scalarProperty?.id ?? colorProperty?.id,
      colorValues: colorProperty ? extractPropertyValues(colorProperty, { records, recordIds: positions.ids }) : options.colorValues,
      scalarValues: scalarProperty ? extractPropertyValues(scalarProperty, { records, recordIds: positions.ids }) : options.scalarValues,
      sizeValues: sizeProperty ? extractPropertyValues(sizeProperty, { records, recordIds: positions.ids }) : options.sizeValues,
      labelValues: labelProperty ? extractPropertyValues(labelProperty, { records, recordIds: positions.ids }) : options.labelValues,
    });
  }

  pointView() {
    return new PointCloudView({
      id: `${this.id}:points`,
      name: this.name,
      datasetId: this.datasetId,
      spaceId: this.spaceId,
      coordinateId: this.coordinateId,
      propertyId: this.propertyId,
      records: this.records,
      recordIds: this.recordIds,
      positions: this.positions,
      targetPositions: this.targetPositions || undefined,
      colors: this.colors,
      colorValues: this.colorValues,
      scalarValues: this.scalarValues,
      sizeValues: this.sizeValues,
      size: this.size,
      alpha: this.alpha,
      shape: this.shape,
      material: this.pointMaterial,
      metadata: this.metadata,
    });
  }

  toLayerDescriptors() {
    const descriptors = [];
    if (this.showGround) descriptors.push(this.groundDescriptor());
    if (this.showGroundProjection) descriptors.push(this.groundProjectionDescriptor());
    if (this.showLabels) {
      const labels = this.labelDescriptor();
      if (labels) descriptors.push(labels);
    }

    const point = this.pointView().toLayerDescriptors().map((descriptor) => {
      if (!this.targetPositions) return descriptor;
      // When looping, omit `progress` so InstancedPointLayer advances the morph
      // from the render-loop clock. A finite `progress` pins it to a static frame.
      const animation = {
        mode: "coordinate-morph",
        channel: "targetPosition",
        requiresChannels: ["position", "targetPosition"],
        durationMs: this.durationMs,
        easing: "smoothstep",
        loop: this.morphLoop,
        direction: "alternate",
      };
      if (!this.morphLoop) animation.progress = this.morphProgress;
      return {
        ...descriptor,
        animation,
        source: {
          ...descriptor.source,
          sourceCoordinateId: this.sourceCoordinateId,
          targetCoordinateId: this.targetCoordinateId,
        },
      };
    });
    descriptors.push(...point);
    return descriptors;
  }

  groundDescriptor() {
    const half = this.targetRadius * this.groundScale;
    return new VisualLayer({
      id: `${this.id}:ground`,
      kind: "ground-plane",
      primitive: "GroundPlaneLayer",
      order: -10,
      source: descriptorSource(this),
      channels: {},
      geometry: {
        width: half * 2,
        depth: half * 2,
        y: this.groundY,
        gridScale: 12,
        ...(this.groundGeometry || {}),
      },
      material: {
        alpha: 0.5,
        gridAlpha: 0.14,
        axisAlpha: 0.08,
        baseColor: [0.84, 0.87, 0.82],
        gridColor: [0.34, 0.41, 0.39],
        axisXColor: [0.36, 0.43, 0.43],
        axisZColor: [0.42, 0.38, 0.32],
        ...(this.groundMaterial || {}),
      },
      metadata: { ...this.metadata, role: "stage" },
    }).toDescriptor();
  }

  groundProjectionDescriptor() {
    const ids = this.recordIds;
    const sourcePositions = flattenVectors(this.positions, ids, 3);
    const projected = new Float32Array(sourcePositions.length);
    for (let index = 0; index < sourcePositions.length; index += 3) {
      projected[index] = sourcePositions[index];
      projected[index + 1] = this.groundY;
      projected[index + 2] = sourcePositions[index + 2];
    }
    const scalar = this.scalarValues ? flattenValues(this.scalarValues, ids, 0) : new Float32Array(ids.length);
    return new VisualLayer({
      id: `${this.id}:projection`,
      kind: "ground-projection",
      primitive: "GroundProjectionLayer",
      order: -5,
      source: descriptorSource(this),
      channels: {
        recordId: createStringChannel(ids, "record-id"),
        sourcePosition: createChannel(sourcePositions, 3, "source-position"),
        position: createChannel(projected, 3, "projected-position"),
        color: colorChannelFrom({
          ids,
          colors: this.colors,
          colorValues: this.colorValues,
          scalarValues: this.scalarValues,
          alpha: 0.5,
        }),
        scalar: createChannel(scalar, 1, "scalar"),
        alpha: createChannel(new Float32Array(ids.length).fill(this.groundProjectionAlpha), 1, "alpha"),
      },
      geometry: { mode: "semantic-shadow", plane: "xz", groundY: this.groundY },
      material: {
        lighting: "projection",
        alphaMode: "blend",
        ...(this.groundProjectionMaterial || {}),
      },
      picking: { mode: "record-id", channel: "recordId" },
      metadata: { ...this.metadata, recordCount: ids.length, role: "ground-projection" },
    }).toDescriptor();
  }

  /**
   * Region labels at per-category centroids of the (fitted) coordinate state.
   * Labels morph with the points when a target state is present.
   */
  labelDescriptor() {
    const ids = this.recordIds;
    const source = flattenVectors(this.positions, ids, 3);
    const target = this.targetPositions ? flattenVectors(this.targetPositions, ids, 3) : source;
    const groups = new Map();
    for (let index = 0; index < ids.length; index += 1) {
      const value = this.labelValues.get?.(ids[index]) ?? this.labelValues.get?.(String(ids[index]));
      const category = value == null ? "" : String(value);
      if (!category) continue;
      if (!groups.has(category)) groups.set(category, { s: [0, 0, 0], t: [0, 0, 0], n: 0 });
      const group = groups.get(category);
      const offset = index * 3;
      group.s[0] += source[offset]; group.s[1] += source[offset + 1]; group.s[2] += source[offset + 2];
      group.t[0] += target[offset]; group.t[1] += target[offset + 1]; group.t[2] += target[offset + 2];
      group.n += 1;
    }
    const labels = [];
    const entries = Array.from(groups.entries());
    for (let labelIndex = 0; labelIndex < entries.length; labelIndex += 1) {
      const [category, group] = entries[labelIndex];
      if (!group.n) continue;
      const offset = labelOffsetForIndex(labelIndex, entries.length, this.labelOffsetRadius);
      labels.push({
        text: labelTextForCategory(category, this.labelMap),
        subtext: `${group.n.toLocaleString("en-US")} records`,
        position: [
          group.s[0] / group.n + offset[0],
          group.s[1] / group.n + this.labelLift,
          group.s[2] / group.n + offset[1],
        ],
        targetPosition: [
          group.t[0] / group.n + offset[0],
          group.t[1] / group.n + this.labelLift,
          group.t[2] / group.n + offset[1],
        ],
        color: this.labelColor,
      });
    }
    if (!labels.length) return null;
    return {
      schema: "metric.visual.layer_descriptor.v1",
      id: `${this.id}:labels`,
      kind: "labels",
      primitive: "BillboardLabelLayer",
      order: 20,
      labels,
      geometry: { fontSize: this.labelFontSize },
      material: { fontSize: this.labelFontSize },
      animation: this.targetPositions
        ? { mode: "coordinate-morph", loop: this.morphLoop, durationMs: this.durationMs, direction: "alternate" }
        : { mode: "none" },
      metadata: { ...this.metadata, labelCount: labels.length, role: "region-labels" },
    };
  }
}

export function createMetricSpaceView(options) {
  return new MetricSpaceView(options);
}

function labelTextForCategory(category, labelMap) {
  if (!labelMap) return category;
  if (labelMap instanceof Map) {
    const mapped = labelMap.get(category) ?? labelMap.get(String(category));
    return mapped == null ? category : String(mapped);
  }
  if (typeof labelMap === "object" && Object.prototype.hasOwnProperty.call(labelMap, category)) {
    return String(labelMap[category]);
  }
  return category;
}

function labelOffsetForIndex(index, count, radius) {
  const safeRadius = Number.isFinite(Number(radius)) ? Number(radius) : 0;
  if (!(safeRadius > 0) || count <= 1) return [0, 0];
  const angle = -Math.PI * 0.5 + (index / Math.max(1, count)) * Math.PI * 2;
  return [Math.cos(angle) * safeRadius, Math.sin(angle) * safeRadius];
}

export function defaultCoordinateId(document, space, options = {}) {
  const coordinates = Array.isArray(document?.coordinates) ? document.coordinates : [];
  const wantedDimension = options.dimension;
  const scoped = space?.id
    ? coordinates.filter((coordinate) => String(coordinate.space_id ?? coordinate.spaceId) === String(space.id))
    : coordinates;
  const pool = scoped.length ? scoped : coordinates;
  const byDimension = wantedDimension != null
    ? pool.find((coordinate) => Number(coordinate.dimension) === Number(wantedDimension))
    : null;
  return (byDimension || pool[0])?.id;
}
