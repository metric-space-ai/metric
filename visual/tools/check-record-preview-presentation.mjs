#!/usr/bin/env node

import assert from "node:assert/strict";
import { VisualSpace } from "../src/data/index.js";
import {
  PREVIEW_PRESENTATION_DIAGNOSTICS_SCHEMA,
  buildMetricPairPreview,
  buildMetricRecordPreview,
} from "../src/interaction/record-preview.js";

const datasetId = "preview-presentation";
const longText = "pressure sensor emitted a long diagnostic payload ".repeat(8);
const document = {
  schema: "metric.visual.v1",
  datasets: [{ id: datasetId, title: "Preview presentation payloads" }],
  records: [
    {
      id: "text",
      dataset_id: datasetId,
      record_type: "text",
      label: "text payload",
      payload: { kind: "text", value: longText },
    },
    {
      id: "curve",
      dataset_id: datasetId,
      record_type: "process_curve",
      label: "process curve",
      payload: {
        kind: "process_curve",
        unit: "bar",
        sample_rate_hz: 10,
        series: Array.from({ length: 24 }, (_, index) => Math.sin(index / 3)),
      },
    },
    {
      id: "histogram",
      dataset_id: datasetId,
      record_type: "distribution",
      label: "amplitude distribution",
      payload: { kind: "distribution", bins: [1, 4, 7, 4, 1], edges: [0, 1, 2, 3, 4, 5] },
    },
    {
      id: "image",
      dataset_id: datasetId,
      record_type: "image",
      label: "compact pixel image",
      payload: {
        kind: "image",
        width: 3,
        height: 2,
        pixels: [0, 0.2, 0.4, 0.6, 0.8, 1],
        label: "six-pixel image",
      },
    },
    {
      id: "composed",
      dataset_id: datasetId,
      record_type: "mixed",
      label: "composed evidence",
      payload: {
        kind: "composed",
        fields: {
          code: { kind: "text", value: "pump-07" },
          spectrum: { kind: "histogram", bins: [2, 5, 3] },
        },
      },
    },
    {
      id: "properties",
      dataset_id: datasetId,
      record_type: "properties",
      label: "scalar categorical vector properties",
      payload: {
        kind: "properties",
        value: 42,
        category: "stable",
        features: { confidence: 0.91, vector: [1, 2, 3] },
      },
    },
    {
      id: "vector",
      dataset_id: datasetId,
      record_type: "vector",
      label: "named vector",
      payload: {
        kind: "vector",
        values: [0.2, 0.4, 0.6],
        names: ["rms", "peak", "crest"],
      },
    },
  ],
  relations: [{
    id: "preview-distance",
    dataset_id: datasetId,
    name: "Preview exported distance",
    relation_type: "distance",
    storage: "symmetric_upper",
    record_ids: ["text", "curve"],
    metadata: { symmetric: true },
    values: [{ row: 0, column: 1, value: 0.75 }],
  }],
  properties: [{
    id: "pair-source",
    dataset_id: datasetId,
    target_type: "pair",
    name: "source",
    values: [{ relation_id: "preview-distance", row_id: "text", column_id: "curve", value: "exported symmetric" }],
  }],
  spaces: [],
  graphs: [],
  coordinates: [],
  timelines: [],
  views: [],
  diagnostics: [],
};

const context = {
  document,
  visualSpace: VisualSpace.fromDocument(document, { validate: false }),
};

function field(fields, label) {
  return fields?.find((entry) => entry.label === label);
}

function section(preview, title) {
  return preview.sections?.find((entry) => entry.title === title);
}

function record(recordId) {
  return buildMetricRecordPreview({ recordId }, context);
}

const text = record("text");
assert.equal(text.diagnostics.schema, PREVIEW_PRESENTATION_DIAGNOSTICS_SCHEMA);
assert.equal(text.diagnostics.payloadRenderer, "text");
assert.equal(text.diagnostics.truncated, true);
assert.equal(field(section(text, "payload").fields, "text").value.endsWith("..."), true);

const curve = record("curve");
assert.equal(curve.diagnostics.payloadRenderer, "time-series");
assert.equal(curve.diagnostics.summarized, true);
assert.equal(curve.series[0].values.length, 24);
assert.equal(field(section(curve, "payload").fields, "sample rate").value, "10 Hz");

const histogram = record("histogram");
assert.equal(histogram.diagnostics.payloadRenderer, "histogram");
assert.equal(histogram.diagnostics.summarized, true);
assert.equal(section(histogram, "payload").series[0].values.length, 5);

const image = record("image");
assert.equal(image.diagnostics.payloadRenderer, "compact-image");
assert.equal(section(image, "payload").image.kind, "compact-array");
assert.equal(section(image, "payload").image.sourceWidth, 3);
assert.equal(section(image, "payload").image.sourceHeight, 2);
assert.equal(section(image, "payload").image.values.length, 6);

const composed = record("composed");
assert.equal(composed.diagnostics.payloadRenderer, "composed");
assert.equal(field(section(composed, "component: code").fields, "text").value, "pump-07");
assert.equal(section(composed, "component: spectrum").series[0].values.length, 3);

const properties = record("properties");
assert.equal(properties.diagnostics.payloadRenderer, "properties");
assert.equal(field(section(properties, "payload").fields, "value").value, 42);
assert.equal(field(section(properties, "payload").fields, "category").value, "stable");
assert.equal(field(section(properties, "payload").fields, "vector").value, "3 values");

const vector = record("vector");
assert.equal(vector.diagnostics.payloadRenderer, "vector");
assert.equal(field(section(vector, "payload").fields, "rms").value, 0.2);
assert.equal(field(section(vector, "payload").fields, "crest").value, 0.6);

const pair = buildMetricPairPreview({
  relationId: "preview-distance",
  rowId: "curve",
  columnId: "text",
}, context);
assert.equal(pair.diagnostics.schema, PREVIEW_PRESENTATION_DIAGNOSTICS_SCHEMA);
assert.equal(pair.diagnostics.payloadRenderer, "pair-relation");
assert.equal(pair.diagnostics.relation.reversedLookup, true);
assert.equal(pair.diagnostics.fields.rowIdentity, true);
assert.equal(pair.diagnostics.fields.columnIdentity, true);
assert.equal(pair.diagnostics.fields.pairProperties, 1);
assert.equal(pair.pair.value, 0.75);
assert.equal(field(pair.pair.properties, "source").value, "exported symmetric");

console.log("Record preview presentation diagnostics contract passed.");
