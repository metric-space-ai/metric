/*
 * Channel encoders for METRIC Visual layers.
 */

import { createColorScale, createOrdinalColorScale, extent, clamp01 } from "./color-scales.js";

export const CHANNELS = [
  "position",
  "color",
  "height",
  "size",
  "alpha",
  "category",
  "focus",
  "phase",
];

export function encodeVisualChannels(visualSpace, spec = {}) {
  const records = resolveRecords(visualSpace, spec);
  return {
    records,
    position: encodePositionChannel(visualSpace, channelSpec(spec, "position", records)),
    color: encodeColorChannel(visualSpace, channelSpec(spec, "color", records)),
    height: encodeHeightChannel(visualSpace, channelSpec(spec, "height", records)),
    size: encodeSizeChannel(visualSpace, channelSpec(spec, "size", records)),
    alpha: encodeAlphaChannel(visualSpace, channelSpec(spec, "alpha", records)),
    category: encodeCategoryChannel(visualSpace, channelSpec(spec, "category", records)),
    focus: encodeFocusChannel(visualSpace, channelSpec(spec, "focus", records)),
    phase: encodePhaseChannel(visualSpace, channelSpec(spec, "phase", records)),
  };
}

export function encodePositionChannel(visualSpace, spec = {}) {
  const records = spec.records || resolveRecords(visualSpace, spec);
  const coordinateId = spec.coordinate_id || spec.coordinateId || firstCoordinateId(visualSpace, spec);
  const targetCoordinateId = spec.target_coordinate_id || spec.targetCoordinateId;
  const state = coordinateId ? coordinateStateFor(visualSpace, coordinateId, spec.coordinate) : coordinateStateFromCoordinate(spec.coordinate);
  const targetState = targetCoordinateId ? coordinateStateFor(visualSpace, targetCoordinateId, spec.targetCoordinate) : coordinateStateFromCoordinate(spec.targetCoordinate);
  const array = new Float32Array(records.length * 3);
  const targetArray = targetState ? new Float32Array(records.length * 3) : undefined;

  for (let index = 0; index < records.length; index += 1) {
    const record = records[index];
    writeVec3(array, index, positionForRecord(state, record.id, spec.raw));
    if (targetArray) writeVec3(targetArray, index, positionForRecord(targetState, record.id, spec.raw));
  }

  return {
    channel: "position",
    itemSize: 3,
    coordinateId,
    targetCoordinateId,
    array,
    targetArray,
    state,
    targetState,
  };
}

export function encodeColorChannel(visualSpace, spec = {}) {
  const records = spec.records || resolveRecords(visualSpace, spec);
  const values = resolveRecordValues(visualSpace, records, spec.source || spec);
  const domain = spec.domain || (values.every(isFiniteNumberLike) ? extent(values) : undefined);
  const categories = domain ? undefined : uniqueValues(values);
  const scale = spec.scale === "categorical" || !domain
    ? createOrdinalColorScale({ categories, colors: spec.colors, unknownColor: spec.unknownColor })
    : createColorScale({ ...spec, domain });
  const array = new Float32Array(records.length * 4);

  for (let index = 0; index < records.length; index += 1) {
    const color = spec.constant || spec.color ? createColorScale(spec)(values[index]) : scale(values[index]);
    writeVec4(array, index, color);
  }

  return {
    channel: "color",
    itemSize: 4,
    array,
    values,
    scale,
    domain,
    categories,
  };
}

export function encodeHeightChannel(visualSpace, spec = {}) {
  return encodeNumericChannel(visualSpace, "height", {
    range: [0, 1],
    defaultValue: 0,
    ...spec,
  });
}

export function encodeSizeChannel(visualSpace, spec = {}) {
  return encodeNumericChannel(visualSpace, "size", {
    range: [1, 1],
    defaultValue: 1,
    ...spec,
  });
}

export function encodeAlphaChannel(visualSpace, spec = {}) {
  return encodeNumericChannel(visualSpace, "alpha", {
    range: [1, 1],
    defaultValue: 1,
    clamp: true,
    ...spec,
  });
}

export function encodeCategoryChannel(visualSpace, spec = {}) {
  const records = spec.records || resolveRecords(visualSpace, spec);
  const values = resolveRecordValues(visualSpace, records, spec.source || spec);
  const categories = uniqueValues(values);
  const categoryIndex = new Map(categories.map((category, index) => [String(category), index]));
  const array = new Float32Array(records.length);

  for (let index = 0; index < records.length; index += 1) {
    const value = values[index];
    array[index] = categoryIndex.get(String(value)) ?? -1;
  }

  return {
    channel: "category",
    itemSize: 1,
    array,
    values,
    categories,
    categoryIndex,
  };
}

export function encodeFocusChannel(visualSpace, spec = {}) {
  const records = spec.records || resolveRecords(visualSpace, spec);
  const selected = new Set(spec.selectedRecordIds || spec.selected_record_ids || []);
  const hasSelection = selected.size > 0;
  const values = hasSelection ? records.map((record) => selected.has(record.id) ? 1 : 0) : resolveRecordValues(visualSpace, records, spec.source || spec);
  const array = new Float32Array(records.length);

  for (let index = 0; index < records.length; index += 1) {
    array[index] = clamp01(Number(values[index] ?? 0));
  }

  return {
    channel: "focus",
    itemSize: 1,
    array,
    values,
  };
}

export function encodePhaseChannel(visualSpace, spec = {}) {
  const records = spec.records || resolveRecords(visualSpace, spec);
  const values = spec.source || spec.property_id || spec.propertyId
    ? resolveRecordValues(visualSpace, records, spec.source || spec)
    : records.map((record, index) => spec.mode === "index" ? index / Math.max(records.length - 1, 1) : hashToUnit(record.id));
  const domain = spec.domain || extent(values);
  const array = new Float32Array(records.length);

  for (let index = 0; index < records.length; index += 1) {
    const number = Number(values[index]);
    array[index] = Number.isFinite(number) ? clamp01((number - domain[0]) / (domain[1] - domain[0])) : 0;
  }

  return {
    channel: "phase",
    itemSize: 1,
    array,
    values,
    domain,
  };
}

export function encodeNumericChannel(visualSpace, channel, spec = {}) {
  const records = spec.records || resolveRecords(visualSpace, spec);
  const source = spec.source || spec;
  const constant = constantNumericValue(source);
  if (constant !== undefined) {
    const array = new Float32Array(records.length);
    array.fill(channel === "alpha" ? clamp01(constant) : constant);
    return {
      channel,
      itemSize: 1,
      array,
      values: records.map(() => constant),
      domain: [constant, constant],
      range: [constant, constant],
    };
  }

  const values = resolveRecordValues(visualSpace, records, source);
  const fallback = Number.isFinite(Number(spec.defaultValue)) ? Number(spec.defaultValue) : 0;
  const domain = spec.domain || extent(values);
  const range = spec.range || [0, 1];
  const array = new Float32Array(records.length);

  for (let index = 0; index < records.length; index += 1) {
    const value = Number(values[index]);
    if (!Number.isFinite(value)) {
      array[index] = fallback;
      continue;
    }
    const t = (value - domain[0]) / (domain[1] - domain[0]);
    const mapped = range[0] + (range[1] - range[0]) * (spec.clamp === false ? t : clamp01(t));
    array[index] = channel === "alpha" ? clamp01(mapped) : mapped;
  }

  return {
    channel,
    itemSize: 1,
    array,
    values,
    domain,
    range,
  };
}

function channelSpec(root, channel, records) {
  return {
    dataset_id: root.dataset_id,
    datasetId: root.datasetId,
    space_id: root.space_id,
    spaceId: root.spaceId,
    record_ids: root.record_ids,
    recordIds: root.recordIds,
    ...(root[channel] || {}),
    records,
  };
}

export function resolveRecords(visualSpace, spec = {}) {
  if (Array.isArray(spec.records)) return spec.records;
  if (!visualSpace || typeof visualSpace !== "object") return [];
  if (Array.isArray(spec.record_ids) || Array.isArray(spec.recordIds)) {
    const ids = spec.record_ids || spec.recordIds;
    if (typeof visualSpace.getRecord === "function") {
      return ids.map((id) => visualSpace.getRecord(id)).filter(Boolean);
    }
    const byId = new Map((visualSpace.records || []).map((record) => [String(record.id), record]));
    return ids.map((id) => byId.get(String(id))).filter(Boolean);
  }
  if (spec.dataset_id || spec.datasetId) {
    const datasetId = spec.dataset_id || spec.datasetId;
    if (typeof visualSpace.recordsForDataset === "function") return visualSpace.recordsForDataset(datasetId);
    return (visualSpace.records || []).filter((record) => String(record.dataset_id ?? record.datasetId) === String(datasetId));
  }
  if (spec.space_id || spec.spaceId) {
    const spaceId = spec.space_id || spec.spaceId;
    const space = visualSpace.indexes?.spacesById?.get(spaceId) || (visualSpace.spaces || []).find((entry) => String(entry.id) === String(spaceId));
    const ids = space?.record_ids || space?.recordIds || [];
    if (typeof visualSpace.getRecord === "function") return ids.map((id) => visualSpace.getRecord(id)).filter(Boolean);
    const byId = new Map((visualSpace.records || []).map((record) => [String(record.id), record]));
    return ids.map((id) => byId.get(String(id))).filter(Boolean);
  }
  return Array.isArray(visualSpace.records) ? visualSpace.records : [];
}

export function resolveRecordValues(visualSpace, records, source = {}) {
  if (typeof source === "function") {
    return records.map((record, index) => source(record, index, visualSpace));
  }
  if (Array.isArray(source)) return source.slice(0, records.length);
  if (source.constant !== undefined) return records.map(() => source.constant);
  if (source.value !== undefined && source.property_id === undefined && source.propertyId === undefined) {
    return records.map(() => source.value);
  }
  if (source.values && typeof source.values === "object") {
    return records.map((record) => source.values[record.id]);
  }
  if (source.map instanceof Map) {
    return records.map((record) => source.map.get(record.id));
  }

  const propertyId = source.property_id || source.propertyId;
  if (propertyId) {
    return records.map((record) => {
      const entry = typeof visualSpace.propertyValue === "function"
        ? visualSpace.propertyValue(propertyId, { record_id: record.id })
        : undefined;
      return entry?.value;
    });
  }

  const field = source.field || source.record_field || source.recordField || "label";
  return records.map((record) => getPath(record, field));
}

function firstCoordinateId(visualSpace, spec) {
  if (!visualSpace || typeof visualSpace !== "object") return undefined;
  if (spec.space_id || spec.spaceId) {
    const spaceId = spec.space_id || spec.spaceId;
    if (typeof visualSpace.coordinateStatesForSpace === "function") {
      const states = visualSpace.coordinateStatesForSpace(spaceId);
      return states[0]?.id;
    }
    return (visualSpace.coordinates || []).find((coordinate) => String(coordinate.space_id ?? coordinate.spaceId) === String(spaceId))?.id;
  }
  if (spec.dataset_id || spec.datasetId) {
    return (visualSpace.coordinates || []).find((coordinate) => coordinate.dataset_id === (spec.dataset_id || spec.datasetId))?.id;
  }
  return (visualSpace.coordinates || [])[0]?.id;
}

function coordinateStateFor(visualSpace, coordinateId, fallbackCoordinate) {
  if (visualSpace && typeof visualSpace.getCoordinateState === "function") {
    const state = visualSpace.getCoordinateState(coordinateId);
    if (state) return state;
  }
  const coordinate = fallbackCoordinate
    || (visualSpace?.coordinates || []).find((entry) => String(entry.id) === String(coordinateId));
  return coordinateStateFromCoordinate(coordinate);
}

function coordinateStateFromCoordinate(coordinate) {
  if (!coordinate) return undefined;
  if (coordinate.positionByRecordId instanceof Map) return coordinate;
  const entries = coordinate.record_positions || coordinate.positions || [];
  const positionByRecordId = new Map();
  const rawPositionByRecordId = new Map();
  for (const entry of entries) {
    const recordId = entry.record_id ?? entry.recordId ?? entry.id;
    if (recordId == null) continue;
    const raw = entry.position ?? entry.values ?? entry.coordinates ?? [0, 0, 0];
    const position = [Number(raw[0]) || 0, Number(raw[1]) || 0, Number(raw[2]) || 0];
    positionByRecordId.set(recordId, position);
    rawPositionByRecordId.set(recordId, position);
  }
  return {
    ...coordinate,
    positionByRecordId,
    rawPositionByRecordId,
  };
}

function positionForRecord(state, recordId, raw = false) {
  if (!state) return [0, 0, 0];
  const map = raw ? state.rawPositionByRecordId : state.positionByRecordId;
  return map.get(recordId) || [0, 0, 0];
}

function writeVec3(array, index, value) {
  array[index * 3] = Number(value[0]) || 0;
  array[index * 3 + 1] = Number(value[1]) || 0;
  array[index * 3 + 2] = Number(value[2]) || 0;
}

function writeVec4(array, index, value) {
  array[index * 4] = Number(value[0]) || 0;
  array[index * 4 + 1] = Number(value[1]) || 0;
  array[index * 4 + 2] = Number(value[2]) || 0;
  array[index * 4 + 3] = value[3] === undefined ? 1 : Number(value[3]) || 0;
}

function uniqueValues(values) {
  return Array.from(new Set(values.map((value) => String(value ?? ""))));
}

function isFiniteNumberLike(value) {
  return Number.isFinite(Number(value));
}

function getPath(object, path) {
  return String(path).split(".").reduce((value, key) => value?.[key], object);
}

function hashToUnit(value) {
  const text = String(value);
  let hash = 2166136261;
  for (let index = 0; index < text.length; index += 1) {
    hash ^= text.charCodeAt(index);
    hash = Math.imul(hash, 16777619);
  }
  return (hash >>> 0) / 4294967295;
}

function constantNumericValue(source) {
  if (source.constant !== undefined) {
    const number = Number(source.constant);
    return Number.isFinite(number) ? number : undefined;
  }
  const hasReference = source.property_id !== undefined
    || source.propertyId !== undefined
    || source.field !== undefined
    || source.record_field !== undefined
    || source.recordField !== undefined
    || source.values !== undefined
    || source.map !== undefined;
  if (!hasReference && source.value !== undefined) {
    const number = Number(source.value);
    return Number.isFinite(number) ? number : undefined;
  }
  return undefined;
}
