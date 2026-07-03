/*
 * Indexes for normalized metric.visual.v1 documents.
 */

export function buildVisualIndexes(document) {
  const indexes = {
    datasetsById: mapById(document.datasets),
    recordsById: mapById(document.records),
    relationsById: mapById(document.relations),
    spacesById: mapById(document.spaces),
    propertiesById: mapById(document.properties),
    graphsById: mapById(document.graphs),
    coordinatesById: mapById(document.coordinates),
    timelinesById: mapById(document.timelines),
    viewsById: mapById(document.views),
    recordsByDatasetId: groupBy(document.records, (record) => record.dataset_id),
    relationsByDatasetId: groupBy(document.relations, (relation) => relation.dataset_id),
    spacesByDatasetId: groupBy(document.spaces, (space) => space.dataset_id),
    propertiesByDatasetId: groupBy(document.properties, (property) => property.dataset_id || ""),
    coordinatesByDatasetId: groupBy(document.coordinates, (coordinate) => coordinate.dataset_id),
    coordinatesBySpaceId: groupBy(document.coordinates, (coordinate) => coordinate.space_id),
    timelinesByDatasetId: groupBy(document.timelines, (timeline) => timeline.dataset_id),
    viewsByDatasetId: groupBy(document.views, (view) => view.dataset_id || ""),
    relationIndexesById: new Map(),
    propertyIndexesById: new Map(),
  };

  for (const relation of document.relations) {
    indexes.relationIndexesById.set(relation.id, buildRelationIndex(relation));
  }
  for (const property of document.properties) {
    indexes.propertyIndexesById.set(property.id, buildPropertyIndex(property));
  }

  return indexes;
}

export function mapById(items = []) {
  const map = new Map();
  for (const item of items) {
    if (item && item.id !== undefined && item.id !== null) {
      map.set(item.id, item);
    }
  }
  return map;
}

export function groupBy(items = [], keyFn) {
  const map = new Map();
  for (const item of items) {
    const key = keyFn(item);
    if (!map.has(key)) map.set(key, []);
    map.get(key).push(item);
  }
  return map;
}

export function buildRelationIndex(relation) {
  const recordIndexById = new Map();
  const valuesByRecordId = new Map();
  const valueByRecordPair = new Map();
  const entries = [];

  relation.record_ids.forEach((recordId, index) => {
    recordIndexById.set(recordId, index);
    valuesByRecordId.set(recordId, []);
  });

  for (const entry of relationValueEntries(relation)) {
    entries.push(entry);
    if (!valueByRecordPair.has(entry.row_id)) {
      valueByRecordPair.set(entry.row_id, new Map());
    }
    valueByRecordPair.get(entry.row_id).set(entry.column_id, entry);
    if (!valuesByRecordId.has(entry.row_id)) valuesByRecordId.set(entry.row_id, []);
    if (!valuesByRecordId.has(entry.column_id)) valuesByRecordId.set(entry.column_id, []);
    valuesByRecordId.get(entry.row_id).push(entry);
    if (entry.column_id !== entry.row_id) valuesByRecordId.get(entry.column_id).push(entry);
  }

  return {
    relation,
    entries,
    recordIndexById,
    valuesByRecordId,
    valueByRecordPair,
    valueFor(rowId, columnId) {
      return valueByRecordPair.get(rowId)?.get(columnId);
    },
  };
}

export function relationValueEntries(relation) {
  if (!Array.isArray(relation.values)) return [];

  if (relation.values.length > 0 && Array.isArray(relation.values[0])) {
    return denseMatrixEntries(relation, relation.values);
  }

  const recordCount = relation.record_ids.length;
  if (relation.values.length === recordCount * recordCount && relation.values.every((value) => typeof value !== "object")) {
    return flatDenseEntries(relation, relation.values);
  }

  const entries = [];
  for (let index = 0; index < relation.values.length; index += 1) {
    const value = relation.values[index];
    if (value && typeof value === "object" && !Array.isArray(value)) {
      entries.push(normalizeRelationEntry(relation, value, index));
    }
  }
  return entries;
}

export function buildPropertyIndex(property) {
  const entries = Array.isArray(property.values) ? property.values : [];
  const valueByTargetKey = new Map();
  const valuesByRecordId = new Map();
  const valuesByRelationId = new Map();
  const valuesBySpaceId = new Map();

  for (let index = 0; index < entries.length; index += 1) {
    const entry = normalizePropertyEntry(property, entries[index], index);
    if (!entry) continue;
    valueByTargetKey.set(entry.target_key, entry);
    if (entry.record_id !== undefined) pushToMap(valuesByRecordId, entry.record_id, entry);
    if (entry.relation_id !== undefined) pushToMap(valuesByRelationId, entry.relation_id, entry);
    if (entry.space_id !== undefined) pushToMap(valuesBySpaceId, entry.space_id, entry);
  }

  return {
    property,
    valueByTargetKey,
    valuesByRecordId,
    valuesByRelationId,
    valuesBySpaceId,
    valueFor(target) {
      return valueByTargetKey.get(propertyTargetKey(property.target_type, target));
    },
  };
}

export function propertyTargetKey(targetType, target) {
  if (target === undefined || target === null) return `${targetType}:`;
  if (typeof target === "string") return `${targetType}:${target}`;
  if (targetType === "record") return `${targetType}:${target.record_id ?? target.target_id ?? target.id ?? ""}`;
  if (targetType === "pair") {
    const relationId = target.relation_id ?? "";
    const rowId = target.row_id ?? target.source_id ?? target.a ?? "";
    const columnId = target.column_id ?? target.target_id ?? target.b ?? "";
    return `${targetType}:${relationId}:${rowId}:${columnId}`;
  }
  if (targetType === "relation") return `${targetType}:${target.relation_id ?? target.target_id ?? target.id ?? ""}`;
  if (targetType === "space") return `${targetType}:${target.space_id ?? target.target_id ?? target.id ?? ""}`;
  if (targetType === "graph_edge") return `${targetType}:${target.graph_id ?? ""}:${target.edge_id ?? target.id ?? ""}`;
  if (targetType === "timeline_step") return `${targetType}:${target.timeline_id ?? ""}:${target.step_id ?? target.index ?? ""}`;
  return `${targetType}:${target.target_id ?? target.id ?? ""}`;
}

function denseMatrixEntries(relation, matrix) {
  const entries = [];
  for (let row = 0; row < matrix.length; row += 1) {
    const rowValues = matrix[row] || [];
    for (let column = 0; column < rowValues.length; column += 1) {
      entries.push({
        row,
        column,
        row_id: relation.record_ids[row],
        column_id: relation.record_ids[column],
        value: rowValues[column],
      });
    }
  }
  return entries;
}

function flatDenseEntries(relation, values) {
  const entries = [];
  const width = relation.record_ids.length;
  for (let index = 0; index < values.length; index += 1) {
    const row = Math.floor(index / width);
    const column = index % width;
    entries.push({
      row,
      column,
      row_id: relation.record_ids[row],
      column_id: relation.record_ids[column],
      value: values[index],
    });
  }
  return entries;
}

function normalizeRelationEntry(relation, entry, fallbackIndex) {
  const row = Number.isFinite(Number(entry.row)) ? Number(entry.row) : fallbackIndex;
  const column = Number.isFinite(Number(entry.column)) ? Number(entry.column) : fallbackIndex;
  const rowId = entry.row_id ?? entry.source_id ?? entry.from_id ?? relation.record_ids[row];
  const columnId = entry.column_id ?? entry.target_id ?? entry.to_id ?? relation.record_ids[column];
  return {
    ...entry,
    row,
    column,
    row_id: rowId,
    column_id: columnId,
    value: entry.value,
  };
}

function normalizePropertyEntry(property, entry, index) {
  if (entry === undefined || entry === null) return undefined;
  const objectEntry = typeof entry === "object" && !Array.isArray(entry) ? entry : { index, value: entry };
  const normalized = { ...objectEntry };

  if (property.target_type === "record") {
    normalized.record_id = normalized.record_id ?? normalized.target_id ?? normalized.id;
  } else if (property.target_type === "pair") {
    normalized.relation_id = normalized.relation_id ?? property.relation_id;
    normalized.row_id = normalized.row_id ?? normalized.source_id ?? normalized.a;
    normalized.column_id = normalized.column_id ?? normalized.target_id ?? normalized.b;
  } else if (property.target_type === "relation") {
    normalized.relation_id = normalized.relation_id ?? normalized.target_id ?? property.relation_id;
  } else if (property.target_type === "space") {
    normalized.space_id = normalized.space_id ?? normalized.target_id ?? property.space_id;
  }

  normalized.value = normalized.value ?? normalized.label ?? normalized.category;
  normalized.target_key = propertyTargetKey(property.target_type, normalized);
  return normalized;
}

function pushToMap(map, key, value) {
  if (!map.has(key)) map.set(key, []);
  map.get(key).push(value);
}
