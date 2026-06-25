#!/usr/bin/env node
/*
 * Schema fixture smoke test.
 *
 * Validates every metric.visual.v1 fixture through the real data API:
 * - valid fixtures must pass validation and build a VisualSpace
 * - invalid fixtures must fail with the expected error code
 *
 * Run: node visual/tools/check-schema-fixtures.mjs
 */

import { readFile, readdir } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import { validateVisualDocument, VisualSpace } from "../src/data/index.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const FIXTURE_DIR = resolve(HERE, "..", "examples", "fixtures");

// Invalid fixtures and the error code each is expected to surface.
const EXPECTED_INVALID = {
  "invalid-missing-record-id.visual.json": "required_field",
  "invalid-relation-ref.visual.json": "record_ref",
};

async function loadFixture(name) {
  const path = resolve(FIXTURE_DIR, name);
  return JSON.parse(await readFile(path, "utf8"));
}

async function main() {
  const files = (await readdir(FIXTURE_DIR)).filter((name) => name.endsWith(".visual.json")).sort();
  const results = [];
  let failures = 0;

  for (const name of files) {
    const document = await loadFixture(name);
    const result = validateVisualDocument(document);
    const isInvalidFixture = name.startsWith("invalid-");

    if (isInvalidFixture) {
      const expectedCode = EXPECTED_INVALID[name];
      const codes = result.errors.map((error) => error.code);
      const ok = !result.ok && (!expectedCode || codes.includes(expectedCode));
      if (!ok) failures += 1;
      results.push({
        name,
        kind: "invalid",
        ok,
        expectedCode,
        sawCodes: codes.slice(0, 5),
        errorCount: result.errors.length,
      });
      continue;
    }

    let spaceOk = false;
    let spaceError = null;
    try {
      const space = VisualSpace.fromDocument(document);
      spaceOk = Array.isArray(space.records) && space.records.length > 0;
    } catch (error) {
      spaceError = error.message;
    }
    const ok = result.ok && spaceOk;
    if (!ok) failures += 1;
    results.push({
      name,
      kind: "valid",
      ok,
      errorCount: result.errors.length,
      warningCount: result.warnings.length,
      firstError: result.errors[0] || null,
      spaceError,
    });
  }

  const negativeCases = buildInMemoryNegativeCases(await loadFixture("metric-space.visual.json"));
  for (const testCase of negativeCases) {
    const result = validateVisualDocument(testCase.document);
    const codes = result.errors.map((error) => error.code);
    const ok = !result.ok && codes.includes(testCase.expectedCode);
    if (!ok) failures += 1;
    results.push({
      name: testCase.name,
      kind: "invalid-in-memory",
      ok,
      expectedCode: testCase.expectedCode,
      sawCodes: codes.slice(0, 8),
      errorCount: result.errors.length,
    });
  }

  const ok = failures === 0 && files.length > 0;
  console.log(JSON.stringify({ ok, fixtureDir: FIXTURE_DIR, count: files.length, failures, results }, null, 2));
  if (!ok) process.exitCode = 1;
}

function buildInMemoryNegativeCases(baseDocument) {
  const firstRecord = baseDocument.records[0]?.id;
  const secondRecord = baseDocument.records[1]?.id;
  const firstRelation = baseDocument.relations[0];
  const outsider = {
    ...baseDocument.records[0],
    id: "schema-outsider-record",
    label: "schema outsider record",
  };
  return [
    {
      name: "invalid-dense-relation-length",
      expectedCode: "relation_dense_shape",
      document: mutate(baseDocument, (document) => {
        document.relations[0] = {
          ...firstRelation,
          storage: "dense_matrix",
          values: [0, 1, 1],
        };
      }),
    },
    {
      name: "invalid-relation-value-endpoint-not-in-relation-records",
      expectedCode: "relation_record_ref",
      document: mutate(baseDocument, (document) => {
        document.records.push(outsider);
        document.relations[0].values.push({
          row_id: outsider.id,
          column_id: firstRecord,
          value: 0.42,
        });
      }),
    },
    {
      name: "invalid-pair-property-endpoint",
      expectedCode: "record_ref",
      document: mutate(baseDocument, (document) => {
        document.properties.push({
          id: "broken-pair-property",
          dataset_id: document.datasets[0].id,
          target_type: "pair",
          value_type: "scalar",
          values: [{
            relation_id: firstRelation.id,
            row_id: firstRecord,
            column_id: "missing-record",
            value: 1,
          }],
        });
      }),
    },
    {
      name: "invalid-coordinate-position-dimension",
      expectedCode: "coordinate_position_dimension",
      document: mutate(baseDocument, (document) => {
        document.coordinates[0].dimension = 3;
        document.coordinates[0].record_positions[0].position = [1, 2];
      }),
    },
    {
      name: "invalid-coordinate-position-value",
      expectedCode: "coordinate_position_value",
      document: mutate(baseDocument, (document) => {
        document.coordinates[0].record_positions[0].position = [1, "not-a-number", 3];
      }),
    },
    {
      name: "invalid-graph-edge-endpoint-not-in-graph-nodes",
      expectedCode: "graph_node_ref",
      document: mutate(baseDocument, (document) => {
        document.records.push(outsider);
        if (!document.graphs?.length) {
          document.graphs = [{
            id: "schema-test-graph",
            dataset_id: document.datasets[0].id,
            node_record_ids: [firstRecord, secondRecord],
            edge_relation_id: firstRelation.id,
            graph_type: "schema-test",
            edges: [],
          }];
        }
        document.graphs[0].edges.push({
          source: firstRecord,
          target: outsider.id,
          value: 1,
        });
      }),
    },
    {
      name: "invalid-timeline-graph-reference",
      expectedCode: "graph_ref",
      document: mutate(baseDocument, (document) => {
        document.timelines.push({
          id: "broken-timeline-graph-ref",
          dataset_id: document.datasets[0].id,
          name: "broken timeline graph ref",
          steps: [{
            id: "broken-step",
            graph_id: "missing-graph",
          }],
        });
      }),
    },
  ].filter((testCase) => firstRecord && secondRecord && firstRelation && testCase.document);
}

function mutate(document, fn) {
  const copy = JSON.parse(JSON.stringify(document));
  fn(copy);
  return copy;
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
