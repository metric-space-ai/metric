export const LAYER_DESCRIPTOR_SCHEMA = "metric.visual.layer_descriptor.v1";
export const VIEW_DESCRIPTOR_SCHEMA = "metric.visual.view_descriptor.v1";

export const DEFAULT_PALETTE = [
  "#295f98",
  "#d1495b",
  "#2a9d8f",
  "#f4a261",
  "#6d597a",
  "#577590",
  "#bc6c25",
  "#386641",
  "#8d99ae",
  "#9d4edd",
  "#457b9d",
  "#e76f51",
];

export function makeId(prefix = "view") {
  const random = Math.random().toString(36).slice(2, 9);
  return `${prefix}-${random}`;
}

export function ensureArray(value) {
  if (Array.isArray(value)) return value;
  if (value == null) return [];
  return [value];
}

export function isObject(value) {
  return value !== null && typeof value === "object" && !Array.isArray(value);
}

export function vector(values, itemSize = 3, fallback = 0) {
  const source = Array.isArray(values) || ArrayBuffer.isView(values) ? values : [];
  const out = new Array(itemSize);
  for (let index = 0; index < itemSize; index += 1) {
    const next = Number(source[index]);
    out[index] = Number.isFinite(next) ? next : fallback;
  }
  return out;
}

export function scaleVector(values, scale = [1, 1, 1]) {
  const next = vector(values, 3);
  return [
    next[0] * (Number(scale[0]) || 1),
    next[1] * (Number(scale[1]) || 1),
    next[2] * (Number(scale[2]) || 1),
  ];
}

export function flattenVectors(source, ids, itemSize = 3, options = {}) {
  const scale = options.scale || [1, 1, 1];
  const fallback = options.fallback ?? 0;
  const data = new Float32Array(ids.length * itemSize);
  for (let index = 0; index < ids.length; index += 1) {
    const values = valueForId(source, ids[index], index);
    const next = itemSize === 3
      ? scaleVector(values, scale)
      : vector(values, itemSize, fallback);
    for (let axis = 0; axis < itemSize; axis += 1) {
      data[index * itemSize + axis] = Number.isFinite(next[axis]) ? next[axis] : fallback;
    }
  }
  return data;
}

export function flattenValues(source, ids, fallback = 0) {
  const data = new Float32Array(ids.length);
  for (let index = 0; index < ids.length; index += 1) {
    const value = Number(valueForId(source, ids[index], index));
    data[index] = Number.isFinite(value) ? value : fallback;
  }
  return data;
}

export function flattenCategories(source, ids) {
  const categories = [];
  const categoryIds = new Map();
  const data = new Float32Array(ids.length);
  for (let index = 0; index < ids.length; index += 1) {
    const raw = valueForId(source, ids[index], index);
    const key = raw == null ? "" : String(raw);
    if (!categoryIds.has(key)) {
      categoryIds.set(key, categoryIds.size);
      categories.push({ id: key, index: categoryIds.get(key), label: key });
    }
    data[index] = categoryIds.get(key);
  }
  return { data, categories };
}

export function valueForId(source, id, index) {
  if (source == null) return undefined;
  if (source instanceof Map) return source.has(id) ? source.get(id) : source.get(String(id));
  if (ArrayBuffer.isView(source)) return source[index];
  if (Array.isArray(source)) {
    const direct = source[index];
    if (isObject(direct)) {
      const directId = getRecordId(direct);
      if (directId == null || String(directId) === String(id)) {
        return direct.value ?? direct.values ?? direct.position ?? direct.coordinates ?? direct.color ?? direct.category;
      }
      for (const entry of source) {
        if (isObject(entry) && String(getRecordId(entry)) === String(id)) {
          return entry.value ?? entry.values ?? entry.position ?? entry.coordinates ?? entry.color ?? entry.category;
        }
      }
      return undefined;
    }
    if (Array.isArray(direct) && direct.length >= 2 && String(direct[0]) === String(id)) {
      return direct.length === 2 ? direct[1] : direct.slice(1);
    }
    return direct;
  }
  if (isObject(source)) {
    if (Object.prototype.hasOwnProperty.call(source, id)) return source[id];
    if (Object.prototype.hasOwnProperty.call(source, String(id))) return source[String(id)];
  }
  return undefined;
}

export function recordIdsFromRecords(records) {
  return ensureArray(records)
    .map((record, index) => getRecordId(record) ?? String(index))
    .map(String);
}

export function getRecordId(record) {
  if (record == null) return undefined;
  if (typeof record === "string" || typeof record === "number") return record;
  return record.id ?? record.record_id ?? record.recordId;
}

export function inferRecordIds(options = {}) {
  if (options.recordIds) return options.recordIds.map(String);
  const fromRecords = recordIdsFromRecords(options.records);
  if (fromRecords.length) return fromRecords;
  const fromSource = idsFromSource(options.positions || options.coordinates || options.values || options.colors);
  if (fromSource.length) return fromSource;
  const count = options.count || sourceLength(options.positions || options.coordinates || options.values || options.colors);
  return Array.from({ length: count }, (_, index) => String(index));
}

export function idsFromSource(source) {
  if (!Array.isArray(source)) return [];
  const ids = [];
  for (const entry of source) {
    if (isObject(entry)) {
      const id = getRecordId(entry);
      if (id != null) ids.push(String(id));
    } else if (Array.isArray(entry) && typeof entry[0] === "string") {
      ids.push(String(entry[0]));
    }
  }
  return ids;
}

export function sourceLength(source) {
  if (source == null) return 0;
  if (ArrayBuffer.isView(source) || Array.isArray(source)) return source.length;
  if (source instanceof Map) return source.size;
  if (isObject(source)) return Object.keys(source).length;
  return 0;
}

export function createChannel(data, itemSize = 1, semantic = "scalar", options = {}) {
  const array = ArrayBuffer.isView(data) ? data : new Float32Array(ensureArray(data));
  return {
    semantic,
    itemSize,
    count: itemSize > 0 ? Math.floor(array.length / itemSize) : 0,
    array,
    ...options,
  };
}

export function createStringChannel(values, semantic, options = {}) {
  return {
    semantic,
    itemSize: 1,
    count: values.length,
    array: values.map((value) => value == null ? "" : String(value)),
    ...options,
  };
}

export function colorChannelFrom(options = {}) {
  const ids = options.ids || inferRecordIds(options);
  const explicitColors = options.colors;
  const scalarValues = options.scalarValues;
  const categoryValues = options.categoryValues || options.colorValues;
  const scalarDomain = options.scalarDomain || inferScalarDomain(ids.map((id, index) => valueForId(scalarValues, id, index)));
  const data = new Float32Array(ids.length * 4);
  for (let index = 0; index < ids.length; index += 1) {
    const id = ids[index];
    const explicit = valueForId(explicitColors, id, index);
    const scalar = valueForId(scalarValues, id, index);
    const category = valueForId(categoryValues, id, index);
    const color = explicit != null
      ? parseColor(explicit, options.fallbackColor)
      : Number.isFinite(Number(scalar))
        ? scalarColor(Number(scalar), scalarDomain, options.alpha)
        : categoryColor(category ?? id, options.palette, options.alpha);
    data.set(color, index * 4);
  }
  return createChannel(data, 4, "rgba", {
    colorSpace: "srgb",
    source: explicitColors ? "explicit" : scalarValues ? "scalar-ramp" : "category-palette",
  });
}

export function parseColor(value, fallback = [0.25, 0.34, 0.42, 1]) {
  if (Array.isArray(value) || ArrayBuffer.isView(value)) {
    const values = Array.from(value);
    const max = Math.max(...values.slice(0, 3).map((entry) => Number(entry) || 0));
    const divisor = max > 1 ? 255 : 1;
    return [
      clamp01((Number(values[0]) || 0) / divisor),
      clamp01((Number(values[1]) || 0) / divisor),
      clamp01((Number(values[2]) || 0) / divisor),
      clamp01(values[3] == null ? 1 : Number(values[3])),
    ];
  }
  if (typeof value !== "string") return fallback.slice();
  const text = value.trim();
  if (!text.startsWith("#")) return categoryColor(text);
  const hex = text.slice(1);
  if (![3, 4, 6, 8].includes(hex.length)) return fallback.slice();
  const expanded = hex.length <= 4
    ? hex.split("").map((char) => `${char}${char}`).join("")
    : hex;
  const r = parseInt(expanded.slice(0, 2), 16);
  const g = parseInt(expanded.slice(2, 4), 16);
  const b = parseInt(expanded.slice(4, 6), 16);
  const a = expanded.length >= 8 ? parseInt(expanded.slice(6, 8), 16) : 255;
  if ([r, g, b, a].some((entry) => Number.isNaN(entry))) return fallback.slice();
  return [r / 255, g / 255, b / 255, a / 255];
}

export function categoryColor(value, palette = DEFAULT_PALETTE, alpha = 1) {
  const colors = palette.length ? palette : DEFAULT_PALETTE;
  const color = parseColor(colors[hashString(String(value ?? "")) % colors.length]);
  color[3] = clamp01(alpha);
  return color;
}

export function scalarColor(value, domain, alpha = 1) {
  const min = Number.isFinite(domain?.min) ? domain.min : 0;
  const max = Number.isFinite(domain?.max) ? domain.max : 1;
  const t = max === min ? 0.5 : clamp01((value - min) / (max - min));
  const low = [0.12, 0.22, 0.36];
  const mid = [0.19, 0.57, 0.55];
  const high = [0.94, 0.62, 0.28];
  const color = t < 0.5
    ? mixColor(low, mid, t * 2)
    : mixColor(mid, high, (t - 0.5) * 2);
  return [color[0], color[1], color[2], clamp01(alpha)];
}

export function inferScalarDomain(values) {
  let min = Infinity;
  let max = -Infinity;
  for (const raw of values || []) {
    const value = Number(raw);
    if (!Number.isFinite(value)) continue;
    min = Math.min(min, value);
    max = Math.max(max, value);
  }
  if (!Number.isFinite(min) || !Number.isFinite(max)) return { min: 0, max: 1 };
  return { min, max };
}

export function clamp01(value) {
  return Math.min(1, Math.max(0, Number.isFinite(value) ? value : 0));
}

export function mixColor(a, b, t) {
  return [
    a[0] + (b[0] - a[0]) * t,
    a[1] + (b[1] - a[1]) * t,
    a[2] + (b[2] - a[2]) * t,
  ];
}

export function hashString(value) {
  let hash = 2166136261;
  for (let index = 0; index < value.length; index += 1) {
    hash ^= value.charCodeAt(index);
    hash = Math.imul(hash, 16777619);
  }
  return hash >>> 0;
}

export function resolveCollectionItem(document, collectionName, ref, options = {}) {
  if (!document || !collectionName || ref == null) {
    if (options.required) {
      throw new Error(`Missing required ${options.label || collectionName} reference.`);
    }
    return undefined;
  }
  if (isObject(ref)) return ref;
  const collection = ensureArray(document[collectionName]);
  const item = collection.find((entry) => String(entry.id ?? entry.name) === String(ref));
  if (!item && options.required) {
    throw new Error(`Unknown ${options.label || collectionName} reference: ${String(ref)}`);
  }
  return item;
}

export function recordsFor(document, options = {}) {
  if (options.records) return ensureArray(options.records);
  const records = ensureArray(document?.records);
  const recordIds = new Set(ensureArray(options.recordIds).map(String));
  if (recordIds.size) {
    return records.filter((record) => recordIds.has(String(getRecordId(record))));
  }
  const datasetId = options.datasetId ?? options.dataset_id;
  if (datasetId != null) {
    return records.filter((record) => String(record.dataset_id ?? record.datasetId) === String(datasetId));
  }
  return records;
}

export function extractCoordinatePositions(coordinate, options = {}) {
  const source = coordinate?.record_positions ?? coordinate?.positions ?? coordinate?.values ?? coordinate;
  const ids = options.recordIds?.map(String) || idsFromSource(source);
  const fallbackIds = ids.length ? ids : recordIdsFromRecords(options.records);
  const out = new Map();

  if (Array.isArray(source)) {
    source.forEach((entry, index) => {
      if (isObject(entry)) {
        const id = getRecordId(entry) ?? fallbackIds[index] ?? String(index);
        const position = entry.position ?? entry.coordinates ?? entry.values ?? entry.value;
        out.set(String(id), vector(position, 3));
      } else if (Array.isArray(entry) && typeof entry[0] === "string") {
        out.set(String(entry[0]), vector(entry.length === 2 && Array.isArray(entry[1]) ? entry[1] : entry.slice(1), 3));
      } else {
        const id = fallbackIds[index] ?? String(index);
        out.set(String(id), vector(entry, 3));
      }
    });
  } else if (source instanceof Map) {
    for (const [id, position] of source.entries()) out.set(String(id), vector(position, 3));
  } else if (isObject(source)) {
    for (const [id, position] of Object.entries(source)) out.set(String(id), vector(position, 3));
  }

  return {
    ids: fallbackIds.length ? fallbackIds : Array.from(out.keys()),
    positions: out,
  };
}

export function extractPropertyValues(property, options = {}) {
  const source = property?.values
    ?? property?.record_values
    ?? property?.assignments
    ?? property?.categories
    ?? property;
  const ids = options.recordIds?.map(String) || recordIdsFromRecords(options.records);
  const out = new Map();

  if (Array.isArray(source)) {
    source.forEach((entry, index) => {
      if (isObject(entry)) {
        const id = getRecordId(entry) ?? entry.target_id ?? ids[index] ?? String(index);
        out.set(String(id), entry.value ?? entry.scalar ?? entry.category ?? entry.label ?? entry.values);
      } else if (Array.isArray(entry) && typeof entry[0] === "string") {
        out.set(String(entry[0]), entry.length === 2 ? entry[1] : entry.slice(1));
      } else {
        const id = ids[index] ?? String(index);
        out.set(String(id), entry);
      }
    });
  } else if (source instanceof Map) {
    for (const [id, value] of source.entries()) out.set(String(id), value);
  } else if (isObject(source)) {
    for (const [id, value] of Object.entries(source)) out.set(String(id), value);
  }

  return out;
}

export function extractRelationCells(relation, options = {}) {
  const values = relation?.values ?? relation?.matrix ?? relation;
  const rowIds = ensureArray(options.rowIds || relation?.row_record_ids || relation?.record_ids || relation?.recordIds)
    .map(String);
  const columnIds = ensureArray(options.columnIds || relation?.column_record_ids || relation?.record_ids || relation?.recordIds)
    .map(String);
  const cells = [];

  if (Array.isArray(values) && Array.isArray(values[0])) {
    values.forEach((row, rowIndex) => {
      row.forEach((value, columnIndex) => {
        cells.push({
          row: rowIndex,
          column: columnIndex,
          rowId: rowIds[rowIndex] ?? String(rowIndex),
          columnId: columnIds[columnIndex] ?? String(columnIndex),
          value,
        });
      });
    });
  } else if (Array.isArray(values) && values.every((entry) => isObject(entry))) {
    for (const entry of values) {
      const rowId = entry.row_record_id ?? entry.rowRecordId ?? entry.source_record_id ?? entry.source ?? entry.a ?? entry.i;
      const columnId = entry.column_record_id ?? entry.columnRecordId ?? entry.target_record_id ?? entry.target ?? entry.b ?? entry.j;
      const row = numberOrIndex(entry.row ?? entry.i, rowIds, rowId);
      const column = numberOrIndex(entry.column ?? entry.j, columnIds, columnId);
      cells.push({
        row,
        column,
        rowId: rowId == null ? String(row) : String(rowId),
        columnId: columnId == null ? String(column) : String(columnId),
        value: entry.value ?? entry.weight ?? entry.distance ?? entry.score,
      });
    }
  } else if (Array.isArray(values)) {
    const columns = Math.max(1, columnIds.length || Math.round(Math.sqrt(values.length)));
    values.forEach((value, index) => {
      const row = Math.floor(index / columns);
      const column = index % columns;
      cells.push({
        row,
        column,
        rowId: rowIds[row] ?? String(row),
        columnId: columnIds[column] ?? String(column),
        value,
      });
    });
  } else if (isObject(values)) {
    for (const [rowId, rowValues] of Object.entries(values)) {
      if (isObject(rowValues)) {
        for (const [columnId, value] of Object.entries(rowValues)) {
          cells.push({
            row: numberOrIndex(undefined, rowIds, rowId),
            column: numberOrIndex(undefined, columnIds, columnId),
            rowId,
            columnId,
            value,
          });
        }
      }
    }
  }

  const derivedRowIds = rowIds.length ? rowIds : unique(cells.map((cell) => cell.rowId));
  const derivedColumnIds = columnIds.length ? columnIds : unique(cells.map((cell) => cell.columnId));
  if (!rowIds.length || !columnIds.length) {
    for (const cell of cells) {
      if (!rowIds.length) cell.row = Math.max(0, derivedRowIds.findIndex((entry) => String(entry) === String(cell.rowId)));
      if (!columnIds.length) cell.column = Math.max(0, derivedColumnIds.findIndex((entry) => String(entry) === String(cell.columnId)));
    }
  }

  const maxRow = cells.reduce((max, cell) => Math.max(max, cell.row), -1);
  const maxColumn = cells.reduce((max, cell) => Math.max(max, cell.column), -1);
  return {
    rowIds: derivedRowIds.length ? derivedRowIds : Array.from({ length: maxRow + 1 }, (_, index) => String(index)),
    columnIds: derivedColumnIds.length ? derivedColumnIds : Array.from({ length: maxColumn + 1 }, (_, index) => String(index)),
    cells,
  };
}

export function unique(values) {
  const seen = new Set();
  const out = [];
  for (const value of values) {
    const key = String(value);
    if (seen.has(key)) continue;
    seen.add(key);
    out.push(key);
  }
  return out;
}

export function numberOrIndex(value, ids, id) {
  const number = Number(value);
  if (Number.isInteger(number) && number >= 0) return number;
  const index = ids.findIndex((entry) => String(entry) === String(id));
  return index >= 0 ? index : 0;
}

export function descriptorSource(view, extra = {}) {
  return {
    viewId: view.id,
    viewKind: view.kind,
    datasetId: view.datasetId,
    spaceId: view.spaceId,
    coordinateId: view.coordinateId,
    propertyId: view.propertyId,
    relationId: view.relationId,
    ...extra,
  };
}
