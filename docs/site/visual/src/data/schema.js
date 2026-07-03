/*
 * metric.visual.v1 validation skeleton.
 *
 * This module validates the shape and references of METRIC visual documents.
 * It deliberately does not validate metric laws or algorithm-specific output.
 */

export const VISUAL_SCHEMA = "metric.visual.v1";

export const VISUAL_TOP_LEVEL_ARRAYS = [
  "datasets",
  "records",
  "relations",
  "spaces",
  "properties",
  "graphs",
  "coordinates",
  "timelines",
  "events",
  "views",
  "diagnostics",
];

const REQUIRED_FIELDS = {
  datasets: ["id", "title", "description", "source", "license"],
  records: ["id", "dataset_id", "record_type", "label", "payload"],
  relations: ["id", "dataset_id", "name", "relation_type", "value_type", "record_ids", "storage", "values"],
  spaces: ["id", "dataset_id", "record_ids", "primary_relation_id", "space_type", "metadata"],
  properties: ["id", "target_type", "value_type", "values"],
  graphs: ["id", "dataset_id", "node_record_ids", "edge_relation_id", "graph_type", "edges"],
  coordinates: ["id", "dataset_id", "space_id", "name", "dimension", "record_positions"],
  timelines: ["id", "dataset_id", "name", "steps"],
};

export class VisualValidationError extends Error {
  constructor(result) {
    super(formatValidationMessage(result));
    this.name = "VisualValidationError";
    this.result = result;
    this.errors = result.errors;
    this.warnings = result.warnings;
  }
}

export function validateVisualDocument(document, options = {}) {
  const issues = [];
  const ctx = createContext(issues);

  if (!isPlainObject(document)) {
    ctx.error("$", "document_type", "Visual document must be an object");
    return toResult(issues);
  }

  if (document.schema !== VISUAL_SCHEMA) {
    ctx.error("$.schema", "schema", `Expected schema ${VISUAL_SCHEMA}`);
  }

  for (const key of VISUAL_TOP_LEVEL_ARRAYS) {
    if (!Array.isArray(document[key])) {
      const level = options.allowMissingOptionalArrays && key !== "datasets" && key !== "records" ? "warning" : "error";
      ctx[level](`$.${key}`, "top_level_array", `Expected top-level array field: ${key}`);
    }
  }

  if (issues.some((issue) => issue.level === "error")) {
    return toResult(issues);
  }

  validateRequiredFields(document, ctx);
  validateUniqueIds(document, ctx);
  validateReferences(document, ctx);
  return toResult(issues);
}

export function assertVisualDocument(document, options = {}) {
  const result = validateVisualDocument(document, options);
  if (!result.ok) {
    throw new VisualValidationError(result);
  }
  return document;
}

export function isVisualDocument(document) {
  return Boolean(document && document.schema === VISUAL_SCHEMA);
}

export function createEmptyVisualDocument(overrides = {}) {
  const document = {
    schema: VISUAL_SCHEMA,
    provenance: {},
  };
  for (const key of VISUAL_TOP_LEVEL_ARRAYS) {
    document[key] = [];
  }
  return { ...document, ...overrides };
}

function validateRequiredFields(document, ctx) {
  for (const [collection, fields] of Object.entries(REQUIRED_FIELDS)) {
    const items = document[collection] || [];
    for (let index = 0; index < items.length; index += 1) {
      const item = items[index];
      if (!isPlainObject(item)) {
        ctx.error(`$.${collection}[${index}]`, "item_type", `${collection} item must be an object`);
        continue;
      }
      for (const field of fields) {
        if (item[field] === undefined || item[field] === null) {
          ctx.error(`$.${collection}[${index}].${field}`, "required_field", `Missing required field: ${field}`);
        }
      }
    }
  }
}

function validateUniqueIds(document, ctx) {
  for (const key of ["datasets", "records", "relations", "spaces", "properties", "graphs", "coordinates", "timelines", "views"]) {
    const seen = new Map();
    const items = document[key] || [];
    for (let index = 0; index < items.length; index += 1) {
      const id = items[index] && items[index].id;
      if (id === undefined || id === null || id === "") continue;
      if (seen.has(id)) {
        ctx.error(`$.${key}[${index}].id`, "duplicate_id", `Duplicate ${key} id: ${id}`);
      } else {
        seen.set(id, index);
      }
    }
  }
}

function validateReferences(document, ctx) {
  const datasetIds = idSet(document.datasets);
  const recordIds = idSet(document.records);
  const relationIds = idSet(document.relations);
  const spaceIds = idSet(document.spaces);
  const propertyIds = idSet(document.properties);
  const coordinateIds = idSet(document.coordinates);
  const graphIds = idSet(document.graphs);
  const relationsById = mapById(document.relations);

  for (let index = 0; index < document.records.length; index += 1) {
    const record = document.records[index];
    expectId(datasetIds, record.dataset_id, `$.records[${index}].dataset_id`, "dataset_ref", ctx);
  }

  for (let index = 0; index < document.relations.length; index += 1) {
    const relation = document.relations[index];
    expectId(datasetIds, relation.dataset_id, `$.relations[${index}].dataset_id`, "dataset_ref", ctx);
    if (!Array.isArray(relation.record_ids)) {
      ctx.error(`$.relations[${index}].record_ids`, "record_ids_type", "Relation record_ids must be an array");
    } else {
      relation.record_ids.forEach((recordId, recordIndex) => {
        expectId(recordIds, recordId, `$.relations[${index}].record_ids[${recordIndex}]`, "record_ref", ctx);
      });
    }
    validateRelationValues(relation, index, recordIds, ctx);
  }

  for (let index = 0; index < document.spaces.length; index += 1) {
    const space = document.spaces[index];
    expectId(datasetIds, space.dataset_id, `$.spaces[${index}].dataset_id`, "dataset_ref", ctx);
    expectId(relationIds, space.primary_relation_id, `$.spaces[${index}].primary_relation_id`, "relation_ref", ctx);
    if (!Array.isArray(space.record_ids)) {
      ctx.error(`$.spaces[${index}].record_ids`, "record_ids_type", "Space record_ids must be an array");
    } else {
      space.record_ids.forEach((recordId, recordIndex) => {
        expectId(recordIds, recordId, `$.spaces[${index}].record_ids[${recordIndex}]`, "record_ref", ctx);
      });
    }
  }

  for (let index = 0; index < document.properties.length; index += 1) {
    const property = document.properties[index];
    if (property.dataset_id !== undefined) {
      expectId(datasetIds, property.dataset_id, `$.properties[${index}].dataset_id`, "dataset_ref", ctx);
    } else {
      ctx.warning(`$.properties[${index}].dataset_id`, "dataset_ref_missing", "Property has no dataset_id");
    }
    validatePropertyTargets(property, index, { recordIds, relationIds, spaceIds, graphIds }, ctx);
  }

  for (let index = 0; index < document.graphs.length; index += 1) {
    const graph = document.graphs[index];
    expectId(datasetIds, graph.dataset_id, `$.graphs[${index}].dataset_id`, "dataset_ref", ctx);
    expectId(relationIds, graph.edge_relation_id, `$.graphs[${index}].edge_relation_id`, "relation_ref", ctx);
    if (Array.isArray(graph.node_record_ids)) {
      graph.node_record_ids.forEach((recordId, recordIndex) => {
        expectId(recordIds, recordId, `$.graphs[${index}].node_record_ids[${recordIndex}]`, "record_ref", ctx);
      });
    }
    validateGraphEdges(graph, index, recordIds, relationsById.get(graph.edge_relation_id), ctx);
  }

  for (let index = 0; index < document.coordinates.length; index += 1) {
    const coordinate = document.coordinates[index];
    expectId(datasetIds, coordinate.dataset_id, `$.coordinates[${index}].dataset_id`, "dataset_ref", ctx);
    expectId(spaceIds, coordinate.space_id, `$.coordinates[${index}].space_id`, "space_ref", ctx);
    if (!Number.isFinite(Number(coordinate.dimension)) || Number(coordinate.dimension) <= 0) {
      ctx.error(`$.coordinates[${index}].dimension`, "coordinate_dimension", "Coordinate dimension must be a positive number");
    }
    if (!Array.isArray(coordinate.record_positions)) {
      ctx.error(`$.coordinates[${index}].record_positions`, "record_positions_type", "Coordinate record_positions must be an array");
    } else {
      coordinate.record_positions.forEach((entry, entryIndex) => {
        const recordId = entry && (entry.record_id ?? entry.id);
        expectId(recordIds, recordId, `$.coordinates[${index}].record_positions[${entryIndex}].record_id`, "record_ref", ctx);
        validateCoordinatePosition(entry, index, entryIndex, Number(coordinate.dimension), ctx);
      });
    }
  }

  for (let index = 0; index < document.timelines.length; index += 1) {
    const timeline = document.timelines[index];
    expectId(datasetIds, timeline.dataset_id, `$.timelines[${index}].dataset_id`, "dataset_ref", ctx);
    if (!Array.isArray(timeline.steps)) {
      ctx.error(`$.timelines[${index}].steps`, "timeline_steps_type", "Timeline steps must be an array");
      continue;
    }
    timeline.steps.forEach((step, stepIndex) => {
      if (step.coordinate_id !== undefined) {
        expectId(coordinateIds, step.coordinate_id, `$.timelines[${index}].steps[${stepIndex}].coordinate_id`, "coordinate_ref", ctx);
      }
      if (step.property_id !== undefined) {
        expectId(propertyIds, step.property_id, `$.timelines[${index}].steps[${stepIndex}].property_id`, "property_ref", ctx);
      }
      if (step.relation_id !== undefined) {
        expectId(relationIds, step.relation_id, `$.timelines[${index}].steps[${stepIndex}].relation_id`, "relation_ref", ctx);
      }
      if (step.graph_id !== undefined) {
        expectId(graphIds, step.graph_id, `$.timelines[${index}].steps[${stepIndex}].graph_id`, "graph_ref", ctx);
      }
    });
  }
}

function validateRelationValues(relation, relationIndex, recordIds, ctx) {
  const path = `$.relations[${relationIndex}]`;
  if (!Array.isArray(relation.values)) {
    ctx.error(`${path}.values`, "relation_values_type", "Relation values must be an array");
    return;
  }
  const relationRecordIds = Array.isArray(relation.record_ids) ? relation.record_ids : [];
  const relationRecordSet = new Set(relationRecordIds);
  const storage = String(relation.storage || "");
  if (storage === "dense_matrix" || storage === "symmetric_dense_matrix") {
    validateDenseRelationValues(relation, relationIndex, ctx);
  }
  for (let valueIndex = 0; valueIndex < relation.values.length; valueIndex += 1) {
    const entry = relation.values[valueIndex];
    if (!isPlainObject(entry)) continue;
    const valuePath = `${path}.values[${valueIndex}]`;
    const rowIndex = numberIndex(entry.row ?? entry.i);
    const columnIndex = numberIndex(entry.column ?? entry.j);
    const rowId = entry.row_id ?? entry.source_id ?? entry.source ?? entry.from_id ?? entry.from
      ?? (rowIndex != null ? relationRecordIds[rowIndex] : undefined);
    const columnId = entry.column_id ?? entry.target_id ?? entry.target ?? entry.to_id ?? entry.to
      ?? (columnIndex != null ? relationRecordIds[columnIndex] : undefined);
    if (rowIndex != null && (rowIndex < 0 || rowIndex >= relationRecordIds.length)) {
      ctx.error(`${valuePath}.row`, "relation_value_index", `Relation row index out of range: ${rowIndex}`);
    }
    if (columnIndex != null && (columnIndex < 0 || columnIndex >= relationRecordIds.length)) {
      ctx.error(`${valuePath}.column`, "relation_value_index", `Relation column index out of range: ${columnIndex}`);
    }
    expectRelationEndpoint(recordIds, relationRecordSet, rowId, `${valuePath}.row_id`, ctx);
    expectRelationEndpoint(recordIds, relationRecordSet, columnId, `${valuePath}.column_id`, ctx);
    if (entry.value === undefined) {
      ctx.error(`${valuePath}.value`, "relation_value_missing", "Relation value entry must include a value");
    }
  }
}

function validateDenseRelationValues(relation, relationIndex, ctx) {
  const recordCount = Array.isArray(relation.record_ids) ? relation.record_ids.length : 0;
  const values = relation.values || [];
  const path = `$.relations[${relationIndex}].values`;
  if (!recordCount) return;
  if (values.length === 0) {
    ctx.error(path, "relation_dense_shape", "Dense relation values must not be empty");
    return;
  }
  if (Array.isArray(values[0])) {
    if (values.length !== recordCount) {
      ctx.error(path, "relation_dense_shape", `Dense matrix row count must equal record_ids length (${recordCount})`);
    }
    values.forEach((row, rowIndex) => {
      if (!Array.isArray(row)) {
        ctx.error(`${path}[${rowIndex}]`, "relation_dense_shape", "Dense matrix row must be an array");
      } else if (row.length !== recordCount) {
        ctx.error(`${path}[${rowIndex}]`, "relation_dense_shape", `Dense matrix row length must equal record_ids length (${recordCount})`);
      }
    });
    return;
  }
  if (values.every((value) => typeof value !== "object")) {
    const expected = recordCount * recordCount;
    if (values.length !== expected) {
      ctx.error(path, "relation_dense_shape", `Flat dense matrix length must equal record_ids length squared (${expected})`);
    }
  }
}

function validatePropertyTargets(property, propertyIndex, refs, ctx) {
  if (!Array.isArray(property.values)) return;
  for (let valueIndex = 0; valueIndex < property.values.length; valueIndex += 1) {
    const entry = property.values[valueIndex];
    if (!isPlainObject(entry)) continue;
    const path = `$.properties[${propertyIndex}].values[${valueIndex}]`;
    if (property.target_type === "record") {
      expectId(refs.recordIds, entry.record_id ?? entry.target_id ?? entry.id, `${path}.record_id`, "record_ref", ctx);
    } else if (property.target_type === "pair") {
      const relationId = entry.relation_id ?? property.relation_id;
      if (relationId !== undefined && relationId !== null && relationId !== "") {
        expectId(refs.relationIds, relationId, `${path}.relation_id`, "relation_ref", ctx);
      }
      expectId(refs.recordIds, entry.row_id ?? entry.source_id ?? entry.a, `${path}.row_id`, "record_ref", ctx);
      expectId(refs.recordIds, entry.column_id ?? entry.target_id ?? entry.b, `${path}.column_id`, "record_ref", ctx);
    } else if (property.target_type === "relation") {
      expectId(refs.relationIds, entry.relation_id ?? entry.target_id ?? property.relation_id, `${path}.relation_id`, "relation_ref", ctx);
    } else if (property.target_type === "space") {
      expectId(refs.spaceIds, entry.space_id ?? entry.target_id ?? property.space_id, `${path}.space_id`, "space_ref", ctx);
    } else if (property.target_type === "graph_edge" && property.graph_id !== undefined) {
      expectId(refs.graphIds, property.graph_id, `$.properties[${propertyIndex}].graph_id`, "graph_ref", ctx);
    }
  }
}

function validateGraphEdges(graph, graphIndex, recordIds, relation, ctx) {
  const path = `$.graphs[${graphIndex}]`;
  if (!Array.isArray(graph.edges)) {
    ctx.error(`${path}.edges`, "graph_edges_type", "Graph edges must be an array");
    return;
  }
  const graphRecordIds = Array.isArray(graph.node_record_ids) ? graph.node_record_ids.map(String) : [];
  const graphRecordSet = new Set(graphRecordIds);
  const relationRecordIds = Array.isArray(relation?.record_ids) ? relation.record_ids.map(String) : [];
  const relationRecordSet = new Set(relationRecordIds);
  for (let edgeIndex = 0; edgeIndex < graph.edges.length; edgeIndex += 1) {
    const edge = graph.edges[edgeIndex];
    const edgePath = `${path}.edges[${edgeIndex}]`;
    if (!isPlainObject(edge) && !Array.isArray(edge)) {
      ctx.error(edgePath, "graph_edge_type", "Graph edge must be an object or endpoint tuple");
      continue;
    }
    const source = graphEndpointId(edge, "source", graphRecordIds);
    const target = graphEndpointId(edge, "target", graphRecordIds);
    expectGraphEndpoint(recordIds, graphRecordSet, relationRecordSet, source, `${edgePath}.source`, ctx);
    expectGraphEndpoint(recordIds, graphRecordSet, relationRecordSet, target, `${edgePath}.target`, ctx);
  }
}

function validateCoordinatePosition(entry, coordinateIndex, entryIndex, dimension, ctx) {
  const path = `$.coordinates[${coordinateIndex}].record_positions[${entryIndex}]`;
  const values = entry && (entry.position ?? entry.values ?? entry.coordinates);
  if (!Array.isArray(values)) {
    ctx.error(`${path}.position`, "coordinate_position_type", "Coordinate position must be an array");
    return;
  }
  if (Number.isFinite(dimension) && dimension > 0 && values.length < dimension) {
    ctx.error(`${path}.position`, "coordinate_position_dimension", `Coordinate position length must be at least dimension (${dimension})`);
  }
  values.forEach((value, valueIndex) => {
    if (!Number.isFinite(Number(value))) {
      ctx.error(`${path}.position[${valueIndex}]`, "coordinate_position_value", "Coordinate position entries must be numeric");
    }
  });
}

function expectId(ids, id, path, code, ctx) {
  if (id === undefined || id === null || id === "") {
    ctx.error(path, code, "Missing referenced id");
    return;
  }
  if (!ids.has(id)) {
    ctx.error(path, code, `Unknown referenced id: ${id}`);
  }
}

function expectRelationEndpoint(recordIds, relationRecordIds, id, path, ctx) {
  expectId(recordIds, id, path, "record_ref", ctx);
  if (id !== undefined && id !== null && id !== "" && !relationRecordIds.has(id)) {
    ctx.error(path, "relation_record_ref", `Relation value endpoint is not listed in relation.record_ids: ${id}`);
  }
}

function expectGraphEndpoint(recordIds, graphRecordIds, relationRecordIds, id, path, ctx) {
  expectId(recordIds, id, path, "record_ref", ctx);
  if (id !== undefined && id !== null && id !== "" && !graphRecordIds.has(String(id))) {
    ctx.error(path, "graph_node_ref", `Graph edge endpoint is not listed in graph.node_record_ids: ${id}`);
  }
  if (
    id !== undefined
    && id !== null
    && id !== ""
    && relationRecordIds.size > 0
    && !relationRecordIds.has(String(id))
  ) {
    ctx.error(path, "graph_relation_record_ref", `Graph edge endpoint is not listed in edge relation record_ids: ${id}`);
  }
}

function graphEndpointId(edge, side, recordIds) {
  if (Array.isArray(edge)) {
    const index = side === "source" ? 0 : 1;
    const value = edge[index];
    return Number.isInteger(Number(value)) && recordIds[Number(value)] != null
      ? recordIds[Number(value)]
      : value;
  }
  if (!edge || typeof edge !== "object") return null;
  const value = side === "source"
    ? edge.source_id ?? edge.sourceId ?? edge.source ?? edge.from_id ?? edge.from ?? edge.row_id ?? edge.rowId ?? edge.a ?? edge.i
    : edge.target_id ?? edge.targetId ?? edge.target ?? edge.to_id ?? edge.to ?? edge.column_id ?? edge.columnId ?? edge.b ?? edge.j;
  return Number.isInteger(Number(value)) && recordIds[Number(value)] != null
    ? recordIds[Number(value)]
    : value;
}

function numberIndex(value) {
  if (!Number.isInteger(Number(value))) return null;
  return Number(value);
}

function mapById(items = []) {
  const out = new Map();
  for (const item of items || []) {
    if (item?.id !== undefined && item?.id !== null && item.id !== "") out.set(item.id, item);
  }
  return out;
}

function idSet(items = []) {
  return new Set(items.map((item) => item && item.id).filter((id) => id !== undefined && id !== null && id !== ""));
}

function createContext(issues) {
  const add = (level, path, code, message) => issues.push({ level, path, code, message });
  return {
    error: (path, code, message) => add("error", path, code, message),
    warning: (path, code, message) => add("warning", path, code, message),
  };
}

function toResult(issues) {
  const errors = issues.filter((issue) => issue.level === "error");
  const warnings = issues.filter((issue) => issue.level === "warning");
  return {
    ok: errors.length === 0,
    errors,
    warnings,
    issues,
  };
}

function formatValidationMessage(result) {
  const first = result.errors[0];
  if (!first) return "METRIC visual document validation failed";
  return `METRIC visual document validation failed at ${first.path}: ${first.message}`;
}

function isPlainObject(value) {
  return Boolean(value) && typeof value === "object" && !Array.isArray(value);
}
