#!/usr/bin/env node

import { existsSync, readFileSync } from "node:fs";
import { join } from "node:path";

const root = new URL("../..", import.meta.url).pathname;

const heroRules = [
  {
    id: "condition-monitoring-hero",
    required: [
      ["property field", /SpacePropertiesView|DenseFieldView|HeatFieldLayer|showSpaceProperties|groundField|density|entropy/i],
      ["trajectory/path", /CurveRibbonLayer|CurveTubeMeshLayer|DynamicsView|trajectory|path|anomaly/i],
      ["record preview", /RecordPreviewPanel|preview|time[-_ ]?series/i],
    ],
  },
  {
    id: "mixed-record-hero",
    required: [
      ["typed glyphs", /InstancedGlyphLayer|glyph|recordType|typed/i],
      ["cross-type relations", /RelationEdgeLayer|NeighborhoodGraphView|showNeighborhoodGraph|edge|cross[-_ ]?type/i],
      ["typed preview", /RecordPreviewPanel|preview|text|image|curve|table/i],
    ],
  },
  {
    id: "cross-space-dependency-hero",
    required: [
      ["paired spaces", /paired|pair|spaceA|spaceB|cross[-_ ]?space|showCrossSpace/i],
      ["dependence evidence", /dependence|correlation|MGC|localContribution|bridge|contribution/i],
      ["linked brushing", /selection|linked|brush|pairedHighlight/i],
    ],
  },
  {
    id: "mapping-dimensionality-hero",
    required: [
      ["morph", /MappingView|showMapping|morph|sourceCoordinate|targetCoordinate/i],
      ["residual/error", /residual|error|distortion|preservation|RelationEdgeLayer|vector/i],
      ["multi-state capture", /source|transition|target|hold/i],
    ],
  },
  {
    id: "dynamics-noise-hero",
    required: [
      ["dynamics", /DynamicsView|showDynamics|trajectory|stateHistory|timeline/i],
      ["propagation field", /propagation|field|HeatFieldLayer|DenseFieldView|uncertainty/i],
      ["record history preview", /RecordPreviewPanel|preview|history|state/i],
    ],
  },
  {
    id: "relation-matrix-neighborhood",
    required: [
      ["relation matrix", /RelationMatrixView|RelationMatrixLayer|showRelationMatrix|matrix/i],
      ["neighborhood graph", /NeighborhoodGraphView|RelationEdgeLayer|showNeighborhoodGraph|graph|edge/i],
      ["pair preview", /pair|relation value|RecordPreviewPanel|preview/i],
    ],
  },
];

const failures = [];

for (const rule of heroRules) {
  const page = join(root, "visual/examples", rule.id, "index.html");
  if (!existsSync(page)) {
    failures.push(`${rule.id}: missing example page`);
    continue;
  }
  const text = readFileSync(page, "utf8");
  const hasPointOnlyHelper = /createMetricSpaceHeroScene|MetricSpaceHeroScene/.test(text);
  if (hasPointOnlyHelper) {
    failures.push(`${rule.id}: uses forbidden generic metric-space hero helper`);
  }
  for (const [label, pattern] of rule.required) {
    if (!pattern.test(text)) {
      failures.push(`${rule.id}: missing required visual grammar evidence: ${label}`);
    }
  }
}

if (failures.length > 0) {
  console.error("Hero visual grammar check failed:");
  for (const failure of failures) console.error(`- ${failure}`);
  process.exit(1);
}

console.log("Hero visual grammar check passed.");
