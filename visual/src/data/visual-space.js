/*
 * VisualSpace normalizes metric.visual.v1 documents for render layers.
 */

import { VISUAL_TOP_LEVEL_ARRAYS, assertVisualDocument, createEmptyVisualDocument } from "./schema.js";
import { buildVisualIndexes } from "./indexes.js";

export class VisualSpace {
  constructor(document, options = {}) {
    const normalizedDocument = normalizeVisualDocument(document);
    if (options.validate !== false) {
      assertVisualDocument(normalizedDocument, options.validation || {});
    }

    this.document = normalizedDocument;
    for (const key of VISUAL_TOP_LEVEL_ARRAYS) {
      this[key] = normalizedDocument[key];
    }
    this.provenance = normalizedDocument.provenance || {};
    this.indexes = buildVisualIndexes(normalizedDocument);
    this.coordinateStatesById = new Map();

    for (const coordinate of this.coordinates) {
      this.coordinateStatesById.set(coordinate.id, normalizeCoordinateState(coordinate, this.indexes, options.coordinates || {}));
    }
  }

  static fromDocument(document, options = {}) {
    return new VisualSpace(document, options);
  }

  getDataset(id) {
    return this.indexes.datasetsById.get(id);
  }

  getRecord(id) {
    return this.indexes.recordsById.get(id);
  }

  recordsForDataset(datasetId) {
    return this.indexes.recordsByDatasetId.get(datasetId) || [];
  }

  getRelation(id) {
    return this.indexes.relationsById.get(id);
  }

  relationIndex(id) {
    return this.indexes.relationIndexesById.get(id);
  }

  relationValue(relationId, rowRecordId, columnRecordId) {
    return this.relationIndex(relationId)?.valueFor(rowRecordId, columnRecordId);
  }

  getProperty(id) {
    return this.indexes.propertiesById.get(id);
  }

  propertyIndex(id) {
    return this.indexes.propertyIndexesById.get(id);
  }

  propertyValue(propertyId, target) {
    return this.propertyIndex(propertyId)?.valueFor(target);
  }

  getCoordinateState(id) {
    return this.coordinateStatesById.get(id);
  }

  coordinateStatesForSpace(spaceId) {
    return (this.indexes.coordinatesBySpaceId.get(spaceId) || []).map((coordinate) => this.getCoordinateState(coordinate.id));
  }

  coordinatesForRecord(recordId) {
    const states = [];
    for (const state of this.coordinateStatesById.values()) {
      const position = state.positionByRecordId.get(recordId);
      if (position) states.push({ state, position });
    }
    return states;
  }
}

export function normalizeVisualDocument(document) {
  const base = createEmptyVisualDocument();
  const normalized = {
    ...base,
    ...document,
    provenance: { ...(document?.provenance || {}) },
  };

  for (const key of VISUAL_TOP_LEVEL_ARRAYS) {
    normalized[key] = Array.isArray(document?.[key]) ? document[key].map((item) => ({ ...item })) : [];
  }

  return normalized;
}

export function normalizeCoordinateState(coordinate, indexes, options = {}) {
  const dimension = Math.max(1, Number(coordinate.dimension) || inferredCoordinateDimension(coordinate.record_positions));
  const sourcePositions = [];

  for (const entry of coordinate.record_positions || []) {
    const values = entry.position ?? entry.values ?? entry.coordinates ?? [];
    const raw = Array.from(values, (value) => numberOrZero(value));
    const position = toPosition3(raw, dimension, options);
    sourcePositions.push({
      ...entry,
      record_id: entry.record_id ?? entry.id,
      raw,
      position,
    });
  }

  const bounds = boundsForPositions(sourcePositions.map((entry) => entry.position));
  const normalizedBounds = normalizeBounds(bounds, options);
  const positionByRecordId = new Map();
  const rawPositionByRecordId = new Map();
  const normalizedPositions = sourcePositions.map((entry) => {
    const normalized = normalizePosition(entry.position, normalizedBounds);
    const normalizedEntry = {
      ...entry,
      normalized,
    };
    positionByRecordId.set(entry.record_id, normalized);
    rawPositionByRecordId.set(entry.record_id, entry.position);
    return normalizedEntry;
  });

  return {
    ...coordinate,
    dimension,
    sourcePositions,
    normalizedPositions,
    bounds,
    normalization: normalizedBounds,
    positionByRecordId,
    rawPositionByRecordId,
    recordIds: sourcePositions.map((entry) => entry.record_id),
  };
}

export function toPosition3(values, dimension = values.length, options = {}) {
  const axis = options.twoDimensionalAxis || "xz";
  const x = numberOrZero(values[0]);
  const y = numberOrZero(values[1]);
  const z = numberOrZero(values[2]);

  if (dimension <= 1) return [x, 0, 0];
  if (dimension === 2) {
    if (axis === "xy") return [x, y, 0];
    return [x, 0, y];
  }
  return [x, y, z];
}

export function boundsForPositions(positions) {
  const min = [Infinity, Infinity, Infinity];
  const max = [-Infinity, -Infinity, -Infinity];

  for (const position of positions) {
    for (let axis = 0; axis < 3; axis += 1) {
      const value = numberOrZero(position[axis]);
      min[axis] = Math.min(min[axis], value);
      max[axis] = Math.max(max[axis], value);
    }
  }

  for (let axis = 0; axis < 3; axis += 1) {
    if (!Number.isFinite(min[axis]) || !Number.isFinite(max[axis])) {
      min[axis] = 0;
      max[axis] = 0;
    }
  }

  return {
    min,
    max,
    center: [
      (min[0] + max[0]) / 2,
      (min[1] + max[1]) / 2,
      (min[2] + max[2]) / 2,
    ],
    span: [
      max[0] - min[0],
      max[1] - min[1],
      max[2] - min[2],
    ],
  };
}

export function normalizeBounds(bounds, options = {}) {
  const targetRadius = Number.isFinite(Number(options.targetRadius)) ? Number(options.targetRadius) : 1;
  const maxSpan = Math.max(bounds.span[0], bounds.span[1], bounds.span[2], 1);
  return {
    center: bounds.center,
    scale: (maxSpan / 2) / targetRadius,
    targetRadius,
  };
}

export function normalizePosition(position, normalization) {
  const scale = normalization.scale || 1;
  return [
    (numberOrZero(position[0]) - normalization.center[0]) / scale,
    (numberOrZero(position[1]) - normalization.center[1]) / scale,
    (numberOrZero(position[2]) - normalization.center[2]) / scale,
  ];
}

function inferredCoordinateDimension(recordPositions = []) {
  let dimension = 0;
  for (const entry of recordPositions) {
    const values = entry.position ?? entry.values ?? entry.coordinates ?? [];
    if (Array.isArray(values)) dimension = Math.max(dimension, values.length);
  }
  return dimension || 3;
}

function numberOrZero(value) {
  const number = Number(value);
  return Number.isFinite(number) ? number : 0;
}
