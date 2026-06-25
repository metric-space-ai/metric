#!/usr/bin/env node

import assert from "node:assert/strict";
import { readFileSync } from "node:fs";
import { VisualSpace } from "../src/data/index.js";
import {
  buildMetricPairPreview,
  buildMetricRecordPreview,
} from "../src/interaction/record-preview.js";

function loadFixture(name) {
  const document = JSON.parse(readFileSync(new URL(`../../docs/examples/assets/${name}/metric.visual.json`, import.meta.url), "utf8"));
  return {
    document,
    visualSpace: VisualSpace.fromDocument(document),
  };
}

function field(fields, label) {
  return fields?.find((entry) => entry.label === label);
}

function section(preview, title) {
  return preview.sections?.find((entry) => entry.title === title);
}

function approx(actual, expected, epsilon = 1e-9) {
  assert.ok(Math.abs(Number(actual) - expected) <= epsilon, `${actual} ~= ${expected}`);
}

const mixed = loadFixture("mixed-records");
const mixedPreview = buildMetricRecordPreview({ recordId: "fleet-00" }, mixed);
assert.equal(mixedPreview.kind, "record");
assert.equal(mixedPreview.source, "metric.visual.v1");
assert.equal(mixedPreview.title, "valveflow severity 0");
assert.equal(mixedPreview.payloadKind, "composed");
assert.equal(field(mixedPreview.record.properties, "fault family").value, "valveflow");
assert.equal(field(mixedPreview.record.properties, "farthest-first representative").value, true);
assert.equal(section(mixedPreview, "component: code").fields.find((entry) => entry.label === "text").value, "valveflow");
assert.equal(section(mixedPreview, "component: spectrum").series[0].values.length, 16);
assert.equal(section(mixedPreview, "component: curve").series[0].values.length, 12);
assert.equal(field(section(mixedPreview, "component: vitals").fields, "severity_small").value, 0);

const cross = loadFixture("cross-space-dependency");
const crossRecordPreview = buildMetricRecordPreview({ recordId: "obs-00" }, cross);
assert.equal(crossRecordPreview.payloadKind, "composed");
assert.equal(crossRecordPreview.linkedViews.length, 2);
assert.deepEqual(crossRecordPreview.linkedViews.map((link) => link.coordinateId).sort(), [
  "event-log-landmark-3d",
  "process-curve-landmark-3d",
]);
assert.equal(field(crossRecordPreview.record.properties, "local distance-profile alignment").value, 0.986754245804);
assert.equal(section(crossRecordPreview, "component: event_log").fields.find((entry) => entry.label === "text").value, "ShHchHchHojjohHchHchHchHchHchHE");
assert.equal(section(crossRecordPreview, "component: process_curve").series[0].values.length, 32);

const crossPairPreview = buildMetricPairPreview({
  relationId: "event-log-edit-distance",
  rowId: "obs-00",
  columnId: "obs-01",
}, cross);
assert.equal(crossPairPreview.kind, "pair");
assert.equal(crossPairPreview.pair.relationId, "event-log-edit-distance");
assert.equal(crossPairPreview.pair.rowId, "obs-00");
assert.equal(crossPairPreview.pair.columnId, "obs-01");
assert.equal(crossPairPreview.pair.value, 6);
approx(field(crossPairPreview.pair.properties, "pair distance-profile z-product contribution").value, 0.705432747484);
assert.equal(crossPairPreview.records.row.payloadKind, "composed");
assert.equal(crossPairPreview.records.column.payloadKind, "composed");
assert.equal(field(crossPairPreview.records.row.properties, "local dependence contribution").value, 1.19270988143);
assert.equal(field(section(crossPairPreview, "row record").fields, "label").value, "paired observation 0");
assert.equal(field(section(crossPairPreview, "column record").fields, "label").value, "paired observation 1");

const relationIndependentPairPreview = buildMetricPairPreview({
  relationId: null,
  rowId: "obs-00",
  columnId: "obs-01",
}, cross);
assert.equal(relationIndependentPairPreview.pair.relationId, null);
assert.equal(relationIndependentPairPreview.pair.present, false);
approx(field(relationIndependentPairPreview.pair.properties, "pair distance-profile z-product contribution").value, 0.705432747484);
assert.equal(relationIndependentPairPreview.records.row.payloadKind, "composed");

const relationMatrix = loadFixture("relation-matrix");
const reversePairPreview = buildMetricPairPreview({
  relationId: "process-curve-aligned-metric",
  rowId: "pc-001",
  columnId: "pc-000",
}, relationMatrix);
assert.equal(reversePairPreview.kind, "pair");
assert.equal(reversePairPreview.pair.reversedLookup, true);
assert.equal(reversePairPreview.pair.value, 2.34478);
assert.equal(field(reversePairPreview.fields, "value").value, "2.3448");
assert.equal(field(reversePairPreview.records.row.properties, "process family").value, "normal reference");
assert.equal(field(reversePairPreview.records.column.properties, "matrix order index").value, 0);

console.log("Native record and pair preview payload contract passed.");
