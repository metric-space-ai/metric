/**
 * Utilities for reading finite relation-space evidence without recomputing it.
 *
 * A relation source is expected to expose metric relation values that were
 * exported by METRIC evidence code. These helpers normalize record identifiers
 * and pair endpoints only; they do not derive distances, embeddings, topology,
 * or graph algorithms from raw records.
 *
 * Supported pair shapes are intentionally small and explicit:
 * - { source, target, value }
 * - { sourceId, targetId, value }
 * - { from, to, value }
 * - { i, j, value } or { row, column, value } with recordIds supplied
 * - metric.visual.v1 relation values: { row_id, column_id, row, column, value }
 * - [source, target, value]
 */

const DEFAULT_VALUE_KEYS = ["value", "distance", "relation", "weight", "score"];

/**
 * Return a stable string identifier for a record or endpoint value.
 *
 * @param {*} value Record object, scalar id, or endpoint descriptor.
 * @returns {string}
 */
export function recordId(value) {
  if (value == null) return "";
  if (typeof value === "string") return value;
  if (typeof value === "number" || typeof value === "boolean") return String(value);
  if (typeof value.id === "string" || typeof value.id === "number") return String(value.id);
  if (typeof value.recordId === "string" || typeof value.recordId === "number") return String(value.recordId);
  if (typeof value.key === "string" || typeof value.key === "number") return String(value.key);
  return String(value);
}

/**
 * Collect ordered record ids from explicit records, recordIds, or pair evidence.
 *
 * @param {object} source Relation source.
 * @param {object} [options]
 * @param {Array<string|number|object>} [options.recordIds]
 * @param {Array<object>} [options.records]
 * @returns {string[]}
 */
export function collectRecordIds(source = {}, options = {}) {
  const explicitIds = options.recordIds || source.recordIds || source.record_ids || source.ids;
  if (Array.isArray(explicitIds)) return uniqueIds(explicitIds.map(recordId).filter(Boolean));

  const records = options.records || source.records || source.nodes;
  if (Array.isArray(records)) return uniqueIds(records.map(recordId).filter(Boolean));

  const ids = [];
  const seen = new Set();
  for (const pair of relationPairs(source)) {
    const sourceId = pairSourceId(pair, null);
    const targetId = pairTargetId(pair, null);
    if (sourceId && !seen.has(sourceId)) {
      seen.add(sourceId);
      ids.push(sourceId);
    }
    if (targetId && !seen.has(targetId)) {
      seen.add(targetId);
      ids.push(targetId);
    }
  }
  return ids;
}

/**
 * Build a lookup from record id to matrix index.
 *
 * @param {Array<string|number|object>} ids
 * @returns {Map<string, number>}
 */
export function indexRecordIds(ids) {
  const index = new Map();
  ids.forEach((id, offset) => {
    const key = recordId(id);
    if (key && !index.has(key)) index.set(key, offset);
  });
  return index;
}

/**
 * Return relation pairs from a source without changing their meaning.
 *
 * @param {object|Array} source Relation source or raw pair array.
 * @returns {Array}
 */
export function relationPairs(source = {}) {
  if (Array.isArray(source)) return source;
  if (Array.isArray(source.pairs)) return source.pairs;
  if (Array.isArray(source.relations)) return source.relations;
  if (Array.isArray(source.edges)) return source.edges;
  if (Array.isArray(source.values)) return source.values;
  return [];
}

/**
 * Resolve a pair source id. Numeric endpoints are interpreted through recordIds
 * when supplied.
 *
 * @param {object|Array} pair
 * @param {string[]} [recordIds]
 * @returns {string}
 */
export function pairSourceId(pair, recordIds) {
  if (Array.isArray(pair)) return endpointId(pair[0], recordIds);
  return endpointId(
    firstDefined(
      pair.sourceId,
      pair.source_id,
      pair.source,
      pair.fromId,
      pair.from_id,
      pair.from,
      pair.rowId,
      pair.row_id,
      pair.aId,
      pair.a_id,
      pair.a,
      pair.i,
      pair.row,
    ),
    recordIds,
  );
}

/**
 * Resolve a pair target id. Numeric endpoints are interpreted through recordIds
 * when supplied.
 *
 * @param {object|Array} pair
 * @param {string[]} [recordIds]
 * @returns {string}
 */
export function pairTargetId(pair, recordIds) {
  if (Array.isArray(pair)) return endpointId(pair[1], recordIds);
  return endpointId(
    firstDefined(
      pair.targetId,
      pair.target_id,
      pair.target,
      pair.toId,
      pair.to_id,
      pair.to,
      pair.columnId,
      pair.column_id,
      pair.colId,
      pair.col_id,
      pair.bId,
      pair.b_id,
      pair.b,
      pair.j,
      pair.column,
      pair.col,
    ),
    recordIds,
  );
}

/**
 * Read an exported relation value from a pair.
 *
 * @param {object|Array} pair
 * @param {string|string[]} [valueKey="value"]
 * @returns {number}
 */
export function pairValue(pair, valueKey = "value") {
  const keys = Array.isArray(valueKey) ? valueKey : [valueKey, ...DEFAULT_VALUE_KEYS];
  if (Array.isArray(pair)) return finiteOrNaN(pair[2]);
  for (const key of keys) {
    if (pair && pair[key] != null) return finiteOrNaN(pair[key]);
  }
  return Number.NaN;
}

/**
 * Return finite exported relation values from a source.
 *
 * @param {object|Array} source
 * @param {object} [options]
 * @param {string|string[]} [options.valueKey]
 * @param {boolean} [options.includeDiagonal=false]
 * @returns {Float64Array}
 */
export function finiteRelationValues(source, options = {}) {
  const values = [];
  for (const pair of relationPairs(source)) {
    const sourceId = pairSourceId(pair, options.recordIds);
    const targetId = pairTargetId(pair, options.recordIds);
    if (!options.includeDiagonal && sourceId && sourceId === targetId) continue;
    const value = pairValue(pair, options.valueKey);
    if (Number.isFinite(value)) values.push(value);
  }
  return Float64Array.from(values);
}

export function uniqueIds(ids) {
  const out = [];
  const seen = new Set();
  for (const id of ids) {
    const key = recordId(id);
    if (!key || seen.has(key)) continue;
    seen.add(key);
    out.push(key);
  }
  return out;
}

function endpointId(value, recordIds) {
  if (recordIds && Number.isInteger(value) && value >= 0 && value < recordIds.length) {
    return recordId(recordIds[value]);
  }
  return recordId(value);
}

function firstDefined(...values) {
  for (const value of values) {
    if (value != null) return value;
  }
  return undefined;
}

function finiteOrNaN(value) {
  const number = Number(value);
  return Number.isFinite(number) ? number : Number.NaN;
}
