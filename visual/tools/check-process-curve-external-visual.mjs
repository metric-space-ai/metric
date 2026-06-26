#!/usr/bin/env node

import { readFile } from "node:fs/promises";
import { resolve } from "node:path";
import { validateVisualDocument, VisualSpace } from "../src/data/index.js";
import { createMetricVisualDocumentDiagnostics } from "../src/metric-visual.js";

const root = resolve(new URL("../..", import.meta.url).pathname);
const documentPath = resolve(root, "docs/examples/assets/process-curve-external/metric.visual.json");
const pagePath = resolve(root, "visual/examples/process-curve-external-hero/index.html");

function fail(message, details = {}) {
  const error = new Error(message);
  error.details = details;
  throw error;
}

function requireEqual(actual, expected, message) {
  if (actual !== expected) fail(message, { actual, expected });
}

function requireAtLeast(actual, expected, message) {
  if (!(actual >= expected)) fail(message, { actual, expected });
}

function requireTruthy(value, message, details = {}) {
  if (!value) fail(message, details);
}

function propertyById(document_, id) {
  return document_.properties.find((property) => property.id === id);
}

function relationById(document_, id) {
  return document_.relations.find((relation) => relation.id === id);
}

function valuesByRecordId(property) {
  return new Map((property?.values || []).map((entry) => [entry.record_id, entry.value]));
}

const [documentText, pageText] = await Promise.all([
  readFile(documentPath, "utf8"),
  readFile(pagePath, "utf8"),
]);
const document_ = JSON.parse(documentText);
const validation = validateVisualDocument(document_);
if (!validation.ok) fail("metric.visual document validation failed", { errors: validation.errors });
VisualSpace.fromDocument(document_);

const diagnostics = createMetricVisualDocumentDiagnostics(document_, { publicGallery: true });
requireEqual(document_.schema, "metric.visual.v1", "wrong schema");
requireTruthy(diagnostics.evidenceNative, "document is not classified as native evidence", diagnostics);
requireEqual(document_.provenance?.synthetic, false, "document must not be marked synthetic");
requireEqual(document_.provenance?.writer, "examples/engine/process_curve_external_visual_export.cpp", "wrong native writer");

const sourceRecords = document_.records.filter((record) => record.record_type === "process_curve_source");
const queryRecords = document_.records.filter((record) => record.record_type === "process_curve_query");
requireAtLeast(sourceRecords.length, 500, "source relation must contain hero-scale real process windows");
requireAtLeast(queryRecords.length, 16, "query evidence must preserve at least the original contrast-query scale");
for (const record of sourceRecords) {
  requireEqual(record.payload?.kind, "time_series", "source record payload must be a time series");
  requireEqual(record.payload?.series?.length, 36, "source record must preserve original 36-sample snippet");
  requireTruthy(record.payload?.source_file, "source record missing original source file");
}
for (const record of queryRecords) {
  requireEqual(record.payload?.kind, "time_series", "query record payload must be a time series");
  requireEqual(record.payload?.series?.length, 18, "query record must preserve downsampled 18-sample snippet");
  requireTruthy(record.payload?.expected_label, "query record missing expected label");
}

const relation = relationById(document_, "process-curve-external-aligned-metric");
requireTruthy(relation, "missing aligned process-curve metric relation");
requireEqual(relation.storage, "dense_matrix", "aligned metric relation must be dense for matrix rendering");
requireEqual(relation.record_ids.length, sourceRecords.length, "relation must cover source records");
requireEqual(relation.values.length, sourceRecords.length * sourceRecords.length, "relation pair count mismatch");
requireTruthy(relation.metadata?.operator === "AlignedCurveDistance", "relation does not identify aligned metric operator");

const coordinate = document_.coordinates.find((entry) => entry.id === "process-curve-external-landmark-3d");
requireTruthy(coordinate, "missing C++ exported landmark coordinates");
requireEqual(coordinate.dimension, 3, "landmark coordinate must be 3D");
requireEqual(coordinate.record_positions.length, sourceRecords.length, "coordinate must cover source records");
requireTruthy(coordinate.metadata?.metric_derived === true, "coordinate metadata must mark metric-derived layout");

const graph = document_.graphs.find((entry) => entry.id === "process-curve-external-knn");
requireTruthy(graph, "missing process-curve external graph");
requireEqual(graph.edges.length, sourceRecords.length * 4, "unexpected k-nearest edge count");
requireEqual(graph.edge_relation_id, relation.id, "graph must use the aligned metric relation");

const expectedRole = valuesByRecordId(propertyById(document_, "query-expected-role"));
const metricWinner = valuesByRecordId(propertyById(document_, "query-metric-winner-role"));
const vectorWinner = valuesByRecordId(propertyById(document_, "query-vector-baseline-winner-role"));
const metricCorrect = valuesByRecordId(propertyById(document_, "query-metric-correct"));
const vectorMismatch = valuesByRecordId(propertyById(document_, "query-vector-baseline-mismatch"));
const metricMargin = valuesByRecordId(propertyById(document_, "query-metric-margin"));
requireEqual(expectedRole.size, queryRecords.length, "query expected-role property count mismatch");
requireEqual(metricWinner.size, queryRecords.length, "metric winner property count mismatch");
requireEqual(vectorWinner.size, queryRecords.length, "baseline winner property count mismatch");
for (const query of queryRecords) {
  requireEqual(metricWinner.get(query.id), expectedRole.get(query.id), "metric winner role must match expected role");
  requireEqual(metricCorrect.get(query.id), 1, "metric correctness property must be 1");
  requireEqual(vectorMismatch.get(query.id), 1, "baseline mismatch property must be 1");
  requireAtLeast(metricMargin.get(query.id), 1, "metric margin must be positive");
}

const view = document_.views.find((entry) => entry.id === "process-curve-external-relation-neighborhood-view");
requireTruthy(view, "missing reusable semantic view definition");
requireEqual(view.command, "showRelationMatrixNeighborhood", "native view hint must preserve relation-neighborhood evidence");

requireTruthy(pageText.includes("createMetricVisual"), "example page must create a Visual Engine surface");
requireTruthy(pageText.includes("showProcessCurves"), "example page must call the process-curve Visual Engine command");
requireTruthy(!pageText.includes(".showRelationMatrixNeighborhood("), "example page must not use the generic relation-matrix command as its primary process-curve view");
requireTruthy(pageText.includes("docs/examples/assets/process-curve-external/metric.visual.json"), "example page must load the native metric.visual asset");
for (const forbidden of ["createElementNS", "<svg", ".getContext(", "new Path2D", "arc(", "fillRect("]) {
  requireTruthy(!pageText.includes(forbidden), `example page contains forbidden page-local renderer token: ${forbidden}`);
}

console.log(JSON.stringify({
  ok: true,
  document: documentPath,
  page: pagePath,
  sourceRecords: sourceRecords.length,
  queryRecords: queryRecords.length,
  relations: document_.relations.length,
  relationPairs: relation.values.length,
  coordinates: coordinate.record_positions.length,
  graphEdges: graph.edges.length,
  viewHintCommand: view.command,
  pageCommand: "showProcessCurves",
}, null, 2));
