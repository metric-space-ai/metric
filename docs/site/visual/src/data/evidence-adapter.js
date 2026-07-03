/*
 * Compatibility loader for current metric.evidence.v1 artifacts.
 *
 * The adapter only reshapes exported evidence into metric.visual.v1. It does
 * not compute metric values, embeddings, winners, law checks, or diagnostics.
 */

import { VISUAL_SCHEMA, createEmptyVisualDocument, assertVisualDocument, isVisualDocument } from "./schema.js";
import { VisualSpace } from "./visual-space.js";

export const EVIDENCE_SCHEMA = "metric.evidence.v1";

export async function loadVisualDocument(url, options = {}) {
  const response = await fetch(url);
  if (!response.ok) {
    throw new Error(`Unable to load METRIC visual document from ${url}: ${response.status}`);
  }
  const document = await response.json();
  return normalizeVisualInput(document, options);
}

export async function loadEvidenceAsVisualDocument(url, options = {}) {
  const response = await fetch(url);
  if (!response.ok) {
    throw new Error(`Unable to load METRIC evidence from ${url}: ${response.status}`);
  }
  const evidence = await response.json();
  return adaptMetricEvidenceV1(evidence, options);
}

export async function loadVisualSpace(url, options = {}) {
  const document = await loadVisualDocument(url, options);
  return new VisualSpace(document, options.space || {});
}

export function normalizeVisualInput(input, options = {}) {
  if (isVisualDocument(input)) {
    if (options.validate !== false) assertVisualDocument(input, options.validation || {});
    return input;
  }
  if (input && input.schema === EVIDENCE_SCHEMA) {
    return adaptMetricEvidenceV1(input, options);
  }
  throw new Error("Unsupported METRIC visual input schema");
}

export function assertMetricEvidenceV1(evidence) {
  const result = validateMetricEvidenceV1(evidence);
  if (!result.ok) {
    const first = result.errors[0];
    throw new Error(`METRIC evidence validation failed at ${first.path}: ${first.message}`);
  }
  return evidence;
}

export function validateMetricEvidenceV1(evidence) {
  const errors = [];
  if (!evidence || typeof evidence !== "object") {
    errors.push({ path: "$", message: "Evidence must be an object" });
    return { ok: false, errors };
  }
  if (evidence.schema !== EVIDENCE_SCHEMA) {
    errors.push({ path: "$.schema", message: `Expected schema ${EVIDENCE_SCHEMA}` });
  }
  for (const key of ["records", "spaces", "pair_values", "queries", "winners", "coordinates", "diagnostics"]) {
    if (!Array.isArray(evidence[key])) {
      errors.push({ path: `$.${key}`, message: `Expected array field ${key}` });
    }
  }
  return { ok: errors.length === 0, errors };
}

export function adaptMetricEvidenceV1(evidence, options = {}) {
  assertMetricEvidenceV1(evidence);

  const domains = sortedUnique(evidence.records.map((record) => record.domain));
  const recordsByDomain = groupBy(evidence.records, (record) => record.domain);
  const pairGroups = groupBy(evidence.pair_values, (entry) => `${entry.domain}\u0000${entry.space}`);
  const coordinateGroups = groupBy(evidence.coordinates, (entry) => `${entry.domain}\u0000${entry.space}`);
  const relationIdFor = (domain, space) => `relation:${domain}:${space}`;
  const visual = createEmptyVisualDocument({
    schema: VISUAL_SCHEMA,
    provenance: {
      ...(evidence.provenance || {}),
      source_schema: EVIDENCE_SCHEMA,
      adapter: "metric.visual.data.evidence-adapter",
    },
  });

  visual.datasets = domains.map((domain) => datasetFromDomain(domain, evidence));

  visual.records = evidence.records
    .slice()
    .sort(compareEvidenceRecord)
    .map((record) => ({
      id: record.id,
      dataset_id: record.domain,
      record_type: record.record_type || "record",
      label: record.label ?? record.id,
      payload: {
        family: Array.isArray(record.values) ? "time_series" : "opaque",
        values: Array.isArray(record.values) ? record.values.slice() : record.values,
        source_index: record.index,
        source_domain: record.domain,
      },
      metadata: pickRest(record, ["domain", "index", "id", "label", "record_type", "values"]),
    }));

  visual.relations = Array.from(pairGroups.entries()).map(([key, values]) => {
    const [domain, space] = key.split("\u0000");
    const recordIds = recordIdsForPairGroup(recordsByDomain.get(domain) || [], values);
    return {
      id: relationIdFor(domain, space),
      dataset_id: domain,
      name: space,
      relation_type: relationTypeForSpace(space),
      value_type: "scalar",
      record_ids: recordIds,
      storage: {
        type: "dense_matrix",
        row_field: "row_id",
        column_field: "column_id",
        source_space: space,
      },
      values: values
        .slice()
        .sort(comparePairValue)
        .map((entry) => ({
          row: entry.row,
          column: entry.column,
          row_id: entry.row_id,
          column_id: entry.column_id,
          value: entry.value,
        })),
      metadata: { source_space: space },
    };
  });

  const existingSpaceIds = new Set();
  visual.spaces = evidence.spaces.map((space) => {
    existingSpaceIds.add(space.id);
    const records = recordsByDomain.get(space.domain) || [];
    return {
      id: space.id,
      dataset_id: space.domain,
      record_ids: records.slice().sort(compareEvidenceRecord).map((record) => record.id),
      primary_relation_id: relationIdFor(space.domain, space.representation),
      space_type: spaceTypeForRepresentation(space.representation),
      metadata: {
        title: space.title,
        representation: space.representation,
        metric: space.metric,
        record_count: space.record_count,
        source_space: space,
      },
    };
  });

  for (const relation of visual.relations) {
    const spaceId = `${relation.dataset_id}:${relation.name}`;
    if (!existingSpaceIds.has(spaceId)) {
      existingSpaceIds.add(spaceId);
      visual.spaces.push({
        id: spaceId,
        dataset_id: relation.dataset_id,
        record_ids: relation.record_ids.slice(),
        primary_relation_id: relation.id,
        space_type: spaceTypeForRepresentation(relation.name),
        metadata: {
          representation: relation.name,
          generated_from_relation: relation.id,
        },
      });
    }
  }

  visual.properties = domains.flatMap((domain) => {
    const records = (recordsByDomain.get(domain) || []).slice().sort(compareEvidenceRecord);
    return [
      {
        id: `property:${domain}:record_label`,
        dataset_id: domain,
        name: "record label",
        target_type: "record",
        value_type: "categorical",
        values: records.map((record) => ({
          record_id: record.id,
          value: record.label,
          index: record.index,
        })),
        metadata: { source_field: "records.label" },
      },
      {
        id: `property:${domain}:record_index`,
        dataset_id: domain,
        name: "record index",
        target_type: "record",
        value_type: "rank",
        values: records.map((record) => ({
          record_id: record.id,
          value: record.index,
        })),
        metadata: { source_field: "records.index" },
      },
    ];
  });

  visual.coordinates = Array.from(coordinateGroups.entries()).map(([key, values]) => {
    const [domain, space] = key.split("\u0000");
    const sorted = values.slice().sort(compareCoordinateValue);
    const dimension = Math.max(1, ...sorted.map((entry) => Array.isArray(entry.values) ? entry.values.length : 0));
    return {
      id: `coordinate:${domain}:${space}`,
      dataset_id: domain,
      space_id: parentSpaceIdForCoordinate(domain, space, existingSpaceIds),
      name: space,
      dimension,
      record_positions: sorted.map((entry) => ({
        record_id: entry.record_id,
        record_index: entry.record_index,
        label: entry.label,
        position: Array.isArray(entry.values) ? entry.values.slice() : [],
      })),
      metadata: { source_space: space },
    };
  });

  visual.timelines = buildTimelines(domains, visual.coordinates);
  visual.events = buildEvents(evidence);
  visual.views = buildViews(domains, visual.relations, visual.coordinates, visual.timelines);
  visual.diagnostics = evidence.diagnostics.map((diagnostic, index) => ({
    id: `diagnostic:${diagnostic.domain ?? "global"}:${index}`,
    dataset_id: diagnostic.domain,
    diagnostic_type: "metric.evidence.v1",
    payload: { ...diagnostic },
  }));

  if (options.validate !== false) {
    assertVisualDocument(visual, options.validation || {});
  }

  return visual;
}

function datasetFromDomain(domain, evidence) {
  const domainSpaces = evidence.spaces.filter((space) => space.domain === domain);
  const firstSpace = domainSpaces[0];
  return {
    id: domain,
    title: firstSpace?.title || domain.replaceAll("_", " "),
    description: "",
    source: evidence.provenance?.source || "",
    license: evidence.provenance?.license || "",
    metadata: {
      source_domain: domain,
      spaces: domainSpaces.map((space) => space.id),
    },
  };
}

function recordIdsForPairGroup(domainRecords, values) {
  const byRow = new Map();
  for (const entry of values) {
    if (entry.row_id !== undefined && Number.isFinite(Number(entry.row))) {
      byRow.set(Number(entry.row), entry.row_id);
    }
  }
  if (byRow.size > 0) {
    return Array.from(byRow.entries())
      .sort((a, b) => a[0] - b[0])
      .map((entry) => entry[1]);
  }
  return domainRecords.slice().sort(compareEvidenceRecord).map((record) => record.id);
}

function relationTypeForSpace(space) {
  if (space === "metric_space") return "metric";
  if (space.includes("baseline") || space.includes("padded_vector")) return "baseline_metric";
  if (space.includes("similarity")) return "similarity";
  if (space.includes("transition")) return "transition";
  return "distance_like";
}

function spaceTypeForRepresentation(representation) {
  if (representation === "metric_space") return "finite_metric_space";
  if (representation.includes("baseline") || representation.includes("vector")) return "baseline_vector_space";
  if (representation.includes("graph")) return "graph_space";
  if (representation.includes("landmark") || representation.includes("coordinate")) return "derived_coordinate_space";
  return "custom_space";
}

function parentSpaceIdForCoordinate(domain, coordinateSpace, existingSpaceIds) {
  const candidates = [
    `${domain}:metric_space`,
    `${domain}:${coordinateSpace}`,
  ];
  for (const candidate of candidates) {
    if (existingSpaceIds.has(candidate)) return candidate;
  }
  return candidates[0];
}

function buildTimelines(domains, coordinates) {
  const timelines = [];
  for (const domain of domains) {
    const domainCoordinates = coordinates.filter((coordinate) => coordinate.dataset_id === domain);
    const landmark2 = domainCoordinates.find((coordinate) => coordinate.name.includes("landmark2"));
    const landmark3 = domainCoordinates.find((coordinate) => coordinate.name.includes("landmark3"));
    if (landmark2 && landmark3) {
      timelines.push({
        id: `timeline:${domain}:landmark2-landmark3`,
        dataset_id: domain,
        name: "landmark 2D/3D morph",
        timeline_type: "coordinate_morph",
        steps: [
          { index: 0, time: 0, coordinate_id: landmark2.id },
          { index: 1, time: 1, coordinate_id: landmark3.id },
        ],
        metadata: {
          source: EVIDENCE_SCHEMA,
          note: "References exported coordinate states only",
        },
      });
    }
  }
  return timelines;
}

function buildEvents(evidence) {
  const queryEvents = evidence.queries.map((query) => ({
    id: `event:${query.domain}:query:${query.id}`,
    dataset_id: query.domain,
    event_type: "query",
    payload: { ...query },
  }));
  const winnerEvents = evidence.winners.map((winner, index) => ({
    id: `event:${winner.domain}:winner:${winner.query_id}:${winner.space}:${index}`,
    dataset_id: winner.domain,
    event_type: "query_winner",
    payload: { ...winner },
  }));
  return queryEvents.concat(winnerEvents);
}

function buildViews(domains, relations, coordinates, timelines) {
  const views = [];
  for (const domain of domains) {
    const coordinate = coordinates.find((entry) => entry.dataset_id === domain && entry.name.includes("landmark3"))
      || coordinates.find((entry) => entry.dataset_id === domain);
    if (coordinate) {
      views.push({
        id: `view:${domain}:record_cloud`,
        dataset_id: domain,
        view_type: "record_cloud",
        coordinate_id: coordinate.id,
        channels: {
          position: { coordinate_id: coordinate.id },
          color: { property_id: `property:${domain}:record_label`, scale: "categorical" },
          category: { property_id: `property:${domain}:record_label` },
        },
      });
    }
    for (const relation of relations.filter((entry) => entry.dataset_id === domain)) {
      views.push({
        id: `view:${domain}:${relation.name}:matrix`,
        dataset_id: domain,
        view_type: "relation_matrix",
        relation_id: relation.id,
      });
    }
    const timeline = timelines.find((entry) => entry.dataset_id === domain);
    if (timeline) {
      views.push({
        id: `view:${domain}:morph`,
        dataset_id: domain,
        view_type: "morph",
        timeline_id: timeline.id,
      });
    }
  }
  return views;
}

function compareEvidenceRecord(a, b) {
  return numericCompare(a.index, b.index) || String(a.id).localeCompare(String(b.id));
}

function comparePairValue(a, b) {
  return numericCompare(a.row, b.row) || numericCompare(a.column, b.column);
}

function compareCoordinateValue(a, b) {
  return numericCompare(a.record_index, b.record_index) || String(a.record_id).localeCompare(String(b.record_id));
}

function numericCompare(a, b) {
  const na = Number(a);
  const nb = Number(b);
  if (Number.isFinite(na) && Number.isFinite(nb)) return na - nb;
  return 0;
}

function sortedUnique(values) {
  return Array.from(new Set(values.filter((value) => value !== undefined && value !== null))).sort();
}

function groupBy(items, keyFn) {
  const map = new Map();
  for (const item of items) {
    const key = keyFn(item);
    if (!map.has(key)) map.set(key, []);
    map.get(key).push(item);
  }
  return map;
}

function pickRest(object, excluded) {
  const excludedSet = new Set(excluded);
  const rest = {};
  for (const [key, value] of Object.entries(object)) {
    if (!excludedSet.has(key)) rest[key] = value;
  }
  return rest;
}
