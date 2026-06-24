#!/usr/bin/env node
/*
 * Hero evidence generator (revised plan, work orders W0-W8).
 *
 * Emits one metric.visual.v1 evidence document per hero work order, colocated
 * with its example, at the densities the plan's "Evidence Density Rules" require.
 * The generator only authors already-computed synthetic evidence — it never runs
 * a METRIC algorithm. Deterministic (seeded PRNG, no Date/random globals).
 *
 * Run: node visual/tools/build-hero-evidence.mjs
 */

import { mkdir, writeFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const HERE = dirname(fileURLToPath(import.meta.url));
const EXAMPLES = resolve(HERE, "..", "examples");
const SCHEMA = "metric.visual.v1";

function mulberry32(seed) {
  let a = seed >>> 0;
  return () => { a |= 0; a = (a + 0x6d2b79f5) | 0; let t = Math.imul(a ^ (a >>> 15), 1 | a); t = (t + Math.imul(t ^ (t >>> 7), 61 | t)) ^ t; return ((t ^ (t >>> 14)) >>> 0) / 4294967296; };
}
const round = (v, d = 4) => { const f = 10 ** d; return Math.round(Number(v) * f) / f; };
const euclidean = (a, b) => { let s = 0; for (let i = 0; i < a.length; i += 1) { const d = a[i] - b[i]; s += d * d; } return Math.sqrt(s); };
const smoothstep = (t) => { const x = Math.min(1, Math.max(0, t)); return x * x * (3 - 2 * x); };

const REGIMES = [
  { id: "healthy", color: [0.20, 0.54, 0.78] },
  { id: "drift", color: [0.75, 0.77, 0.24] },
  { id: "fault", color: [0.88, 0.28, 0.31] },
  { id: "recovery", color: [0.46, 0.34, 0.72] },
];

function knnEdges(positions, ids, k) {
  const edges = [];
  for (let i = 0; i < positions.length; i += 1) {
    const neighbors = [];
    for (let j = 0; j < positions.length; j += 1) { if (i !== j) neighbors.push({ j, d: euclidean(positions[i], positions[j]) }); }
    neighbors.sort((a, b) => a.d - b.d);
    for (const n of neighbors.slice(0, k)) edges.push({ row_id: ids[i], column_id: ids[n.j], value: round(n.d) });
  }
  return edges;
}

/**
 * Shared dense "process sculpture" geometry: runs that degrade (healthy -> drift
 * -> fault) and recover, tracing swirling arms that rise into a fault ridge.
 */
function buildProcessSculpture(seed, runs, stepsPerRun) {
  const random = mulberry32(seed);
  const positions3d = [];
  const positions2d = [];
  const regimeByRecord = [];
  const severityByRecord = [];
  const series = [];
  const regimeFor = (t) => (t < 0.32 ? 0 : t < 0.58 ? 1 : t < 0.82 ? 2 : 3);

  for (let run = 0; run < runs; run += 1) {
    const angle0 = (run / runs) * Math.PI * 2;
    const swirl = 1.0 + (run % 3) * 0.45;
    const lift = 0.85 + (run % 4) * 0.12;
    for (let step = 0; step < stepsPerRun; step += 1) {
      const t = step / (stepsPerRun - 1);
      const severity = Math.max(0, Math.sin(Math.min(1, t / 0.78) * Math.PI * 0.92)) * (t < 0.82 ? 1 : 0.42 + (1 - t) * 1.2);
      const radius = 0.45 + severity * 1.35 + random() * 0.06;
      const height = severity * 1.9 * lift + random() * 0.05;
      const angle = angle0 + t * 1.5 * swirl + (random() - 0.5) * 0.12;
      const x = round(Math.cos(angle) * radius + (random() - 0.5) * 0.08);
      const z = round(Math.sin(angle) * radius + (random() - 0.5) * 0.08);
      const y = round(height);
      positions3d.push([x, y, z]);
      positions2d.push([x, z]);
      regimeByRecord.push(REGIMES[regimeFor(t)]);
      severityByRecord.push(severity);
      const amp = 0.18 + severity * 0.7;
      const phase = run * 0.6 + t * Math.PI * 3;
      series.push(Array.from({ length: 24 }, (_, k) => round(0.5 + amp * Math.sin(phase + (k / 23) * Math.PI * 4) * 0.5 + (random() - 0.5) * (0.05 + severity * 0.18))));
    }
  }
  return { random, positions3d, positions2d, regimeByRecord, severityByRecord, series, count: positions3d.length };
}

function processRecords(prefix, datasetId, sculpture) {
  return sculpture.positions3d.map((_, index) => ({
    id: `${prefix}-${String(index).padStart(4, "0")}`,
    dataset_id: datasetId,
    record_type: "time_series_window",
    label: `run · ${sculpture.regimeByRecord[index].id}`,
    payload: { kind: "time_series", regime: sculpture.regimeByRecord[index].id, sample_rate_hz: 256, series: sculpture.series[index], features: { severity: round(sculpture.severityByRecord[index]) } },
  }));
}

// ---------------------------------------------------------------------------
// W0: schema-contract fixtures
// ---------------------------------------------------------------------------
function buildMinimalFixture() {
  const datasetId = "minimal";
  const random = mulberry32(0x10a1);
  const records = Array.from({ length: 6 }, (_, i) => ({ id: `r${i}`, dataset_id: datasetId, record_type: "vector", label: `obs ${i}`, payload: { kind: "vector", values: [round(random()), round(random()), round(random())] } }));
  const positions = records.map((r) => r.payload.values);
  const ids = records.map((r) => r.id);
  return {
    schema: SCHEMA, provenance: { generator: "build-hero-evidence.mjs", synthetic: true },
    datasets: [{ id: datasetId, title: "Minimal", description: "Minimal valid metric.visual.v1 document.", source: "synthetic", license: "CC0-1.0" }],
    records,
    relations: [{ id: "m", dataset_id: datasetId, name: "metric", relation_type: "metric", value_type: "scalar", record_ids: ids, storage: "sparse_edge_list", values: knnEdges(positions, ids, 3), metadata: {} }],
    spaces: [{ id: "s", dataset_id: datasetId, record_ids: ids, primary_relation_id: "m", space_type: "finite_metric_space", metadata: {} }],
    properties: [], graphs: [],
    coordinates: [{ id: "c3", dataset_id: datasetId, space_id: "s", name: "3D", dimension: 3, record_positions: ids.map((id, i) => ({ record_id: id, position: positions[i] })) }],
    timelines: [], events: [], views: [], diagnostics: [],
  };
}

function buildRelationContractFixture() {
  const datasetId = "relation-contract";
  const random = mulberry32(0x52b1);
  const count = 160;
  const positions = Array.from({ length: count }, () => [round(random() * 2 - 1), round(random()), round(random() * 2 - 1)]);
  const records = positions.map((p, i) => ({ id: `rc-${String(i).padStart(3, "0")}`, dataset_id: datasetId, record_type: "vector", label: `signal ${i}`, payload: { kind: "vector", values: p } }));
  const ids = records.map((r) => r.id);
  const dense = [];
  for (let i = 0; i < count; i += 1) for (let j = i + 1; j < count; j += 1) dense.push({ row_id: ids[i], column_id: ids[j], value: round(euclidean(positions[i], positions[j])) });
  return {
    schema: SCHEMA, provenance: { generator: "build-hero-evidence.mjs", synthetic: true },
    datasets: [{ id: datasetId, title: "Relation contract", description: `Dense pairwise metric over ${count} signals.`, source: "synthetic", license: "CC0-1.0" }],
    records,
    relations: [{ id: "dense-metric", dataset_id: datasetId, name: "dense metric", relation_type: "metric", value_type: "scalar", record_ids: ids, storage: "symmetric_dense_matrix", values: dense, metadata: { symmetric: true } }],
    spaces: [{ id: "s", dataset_id: datasetId, record_ids: ids, primary_relation_id: "dense-metric", space_type: "finite_metric_space", metadata: {} }],
    properties: [], graphs: [], coordinates: [], timelines: [], events: [], views: [], diagnostics: [],
  };
}

// ---------------------------------------------------------------------------
// W4: condition monitoring (>=500 windows)
// ---------------------------------------------------------------------------
function buildConditionEvidence() {
  const datasetId = "condition";
  const s = buildProcessSculpture(0x0fde12, 16, 96);
  const records = processRecords("cd", datasetId, s);
  const ids = records.map((r) => r.id);
  const prop = (fn) => ids.map((id, i) => ({ record_id: id, value: round(fn(i)) }));
  return {
    schema: SCHEMA, provenance: { generator: "build-hero-evidence.mjs", synthetic: true, note: "Dense condition-monitoring metric space." },
    datasets: [{ id: datasetId, title: "Condition monitoring", description: `${records.length} windowed records across 16 degrading/recovering runs.`, source: "synthetic", license: "CC0-1.0" }],
    records,
    relations: [{ id: "condition-metric", dataset_id: datasetId, name: "feature metric", relation_type: "metric", value_type: "scalar", record_ids: ids, storage: "sparse_edge_list", values: knnEdges(s.positions3d, ids, 6), metadata: { neighbors: 6 } }],
    spaces: [{ id: "condition-space", dataset_id: datasetId, record_ids: ids, primary_relation_id: "condition-metric", space_type: "finite_metric_space", metadata: { regimes: REGIMES.map((r) => r.id) } }],
    properties: [
      { id: "anomaly", dataset_id: datasetId, target_type: "record", value_type: "scalar", name: "anomaly score", values: prop((i) => Math.min(1, s.severityByRecord[i] + s.random() * 0.06)) },
      { id: "entropy", dataset_id: datasetId, target_type: "record", value_type: "scalar", name: "local entropy", values: prop((i) => 0.35 + s.severityByRecord[i] * 0.5 + s.random() * 0.08) },
      { id: "density", dataset_id: datasetId, target_type: "record", value_type: "scalar", name: "local density", values: prop((i) => 0.92 - s.severityByRecord[i] * 0.55 + s.random() * 0.06) },
      { id: "cluster", dataset_id: datasetId, target_type: "record", value_type: "categorical", name: "regime", values: ids.map((id, i) => ({ record_id: id, value: s.regimeByRecord[i].id })) },
    ],
    graphs: [],
    coordinates: [
      { id: "condition-3d", dataset_id: datasetId, space_id: "condition-space", name: "metric-space lift (3D)", dimension: 3, record_positions: ids.map((id, i) => ({ record_id: id, position: s.positions3d[i] })) },
      { id: "condition-2d", dataset_id: datasetId, space_id: "condition-space", name: "flat projection (2D)", dimension: 2, record_positions: ids.map((id, i) => ({ record_id: id, position: s.positions2d[i] })) },
    ],
    timelines: [], events: [],
    views: [{ id: "condition", kind: "metric-space", name: "Condition metric space", spaceId: "condition-space", coordinateId: "condition-3d", propertyId: "anomaly" }],
    diagnostics: [],
  };
}

// ---------------------------------------------------------------------------
// W6: mapping/dimensionality (>=2000, source+target+residual, >=3 families)
// ---------------------------------------------------------------------------
function buildMappingEvidence() {
  const datasetId = "mapping";
  const s = buildProcessSculpture(0x6a01, 24, 96); // 2304 records
  const records = processRecords("mp", datasetId, s);
  const ids = records.map((r) => r.id);
  const residual = ids.map((id, i) => ({ record_id: id, value: round(Math.min(1, s.positions3d[i][1] / 2.2 + s.random() * 0.05)) }));
  return {
    schema: SCHEMA, provenance: { generator: "build-hero-evidence.mjs", synthetic: true },
    datasets: [{ id: datasetId, title: "Mapping & dimensionality", description: `${records.length} records with 2D source and 3D target states.`, source: "synthetic", license: "CC0-1.0" }],
    records,
    relations: [{ id: "mapping-metric", dataset_id: datasetId, name: "metric", relation_type: "metric", value_type: "scalar", record_ids: ids, storage: "sparse_edge_list", values: knnEdges(s.positions3d, ids, 6), metadata: {} }],
    spaces: [{ id: "mapping-space", dataset_id: datasetId, record_ids: ids, primary_relation_id: "mapping-metric", space_type: "finite_metric_space", metadata: {} }],
    properties: [
      { id: "map-residual", dataset_id: datasetId, target_type: "record", value_type: "scalar", name: "mapping residual", values: residual },
      { id: "cluster", dataset_id: datasetId, target_type: "record", value_type: "categorical", name: "family", values: ids.map((id, i) => ({ record_id: id, value: s.regimeByRecord[i].id })) },
    ],
    graphs: [],
    coordinates: [
      { id: "source-2d", dataset_id: datasetId, space_id: "mapping-space", name: "source projection (2D)", dimension: 2, record_positions: ids.map((id, i) => ({ record_id: id, position: s.positions2d[i] })) },
      { id: "target-3d", dataset_id: datasetId, space_id: "mapping-space", name: "target metric space (3D)", dimension: 3, record_positions: ids.map((id, i) => ({ record_id: id, position: s.positions3d[i] })) },
    ],
    timelines: [], events: [],
    views: [{ id: "mapping", kind: "metric-space", name: "Mapping", spaceId: "mapping-space", coordinateId: "target-3d", propertyId: "map-residual" }],
    diagnostics: [],
  };
}

// ---------------------------------------------------------------------------
// W7: cross-space dependency (>=1000 paired, two geometries, dependence)
// ---------------------------------------------------------------------------
function buildCrossSpaceEvidence() {
  const datasetId = "paired";
  const random = mulberry32(0x71c3a9);
  const count = 1100;
  const CLUSTERS = 9;
  const distorted = new Set([2, 5, 7]);
  const centroids = Array.from({ length: CLUSTERS }, (_, c) => { const a = (c / CLUSTERS) * Math.PI * 2; const r = 0.7 + (c % 3) * 0.5; return [Math.cos(a) * r, 0.35 + (c % 4) * 0.34, Math.sin(a) * r]; });
  const records = []; const posA = []; const posB = []; const dependence = [];
  for (let i = 0; i < count; i += 1) {
    const cluster = i % CLUSTERS; const c = centroids[cluster];
    const a = [round(c[0] + (random() - 0.5) * 0.5), round(Math.max(0, c[1] + (random() - 0.5) * 0.4)), round(c[2] + (random() - 0.5) * 0.5)];
    const distort = distorted.has(cluster) ? 0.85 : (random() < 0.08 ? 0.7 : 0.1);
    const b = [round(a[0] + (random() - 0.5) * distort * 1.6), round(Math.max(0, a[1] + (random() - 0.5) * distort)), round(a[2] + (random() - 0.5) * distort * 1.6)];
    posA.push(a); posB.push(b);
    records.push({ id: `pr-${String(i).padStart(4, "0")}`, dataset_id: datasetId, record_type: "vector", label: `obs ${i} · cluster ${cluster}`, payload: { kind: "vector", values: a, cluster } });
    dependence.push({ record_id: `pr-${String(i).padStart(4, "0")}`, value: round(1 - distort) });
  }
  const ids = records.map((r) => r.id);
  const global = round(dependence.reduce((s, e) => s + e.value, 0) / count);
  return {
    schema: SCHEMA, provenance: { generator: "build-hero-evidence.mjs", synthetic: true, global_dependence: global },
    datasets: [{ id: datasetId, title: "Cross-space dependency", description: `${count} paired observations in two metric spaces.`, source: "synthetic", license: "CC0-1.0" }],
    records,
    relations: [
      { id: "relation-a", dataset_id: datasetId, name: "space A metric", relation_type: "metric", value_type: "scalar", record_ids: ids, storage: "sparse_edge_list", values: knnEdges(posA, ids, 5), metadata: {} },
      { id: "relation-b", dataset_id: datasetId, name: "space B metric", relation_type: "metric", value_type: "scalar", record_ids: ids, storage: "sparse_edge_list", values: knnEdges(posB, ids, 5), metadata: {} },
    ],
    spaces: [
      { id: "space-a", dataset_id: datasetId, record_ids: ids, primary_relation_id: "relation-a", space_type: "paired_space", metadata: { pair: "B", global_dependence: global } },
      { id: "space-b", dataset_id: datasetId, record_ids: ids, primary_relation_id: "relation-b", space_type: "paired_space", metadata: { pair: "A", global_dependence: global } },
    ],
    properties: [{ id: "local-dependence", dataset_id: datasetId, target_type: "record", value_type: "scalar", name: "local dependence", values: dependence }],
    graphs: [],
    coordinates: [
      { id: "space-a-3d", dataset_id: datasetId, space_id: "space-a", name: "space A coordinates", dimension: 3, record_positions: ids.map((id, i) => ({ record_id: id, position: posA[i] })) },
      { id: "space-b-3d", dataset_id: datasetId, space_id: "space-b", name: "space B coordinates", dimension: 3, record_positions: ids.map((id, i) => ({ record_id: id, position: posB[i] })) },
    ],
    timelines: [], events: [],
    views: [],
    diagnostics: [{ id: "global-dependence", kind: "dependence-statistic", value: global, note: "synthetic global dependence between space A and B" }],
  };
}

// ---------------------------------------------------------------------------
// W8: dynamics/noise (>=500 records, >=12 timeline states, trajectories)
// ---------------------------------------------------------------------------
function buildDynamicsEvidence() {
  const datasetId = "dynamics";
  const random = mulberry32(0x0dd1);
  const count = 640;
  const STEPS = 14;
  const CLUSTERS = 6;
  const centroids = Array.from({ length: CLUSTERS }, (_, c) => { const a = (c / CLUSTERS) * Math.PI * 2; const r = 0.5 + (c % 3) * 0.35; return [Math.cos(a) * r, 0.25 + (c % 3) * 0.4, Math.sin(a) * r]; });
  const clusterByRecord = [];
  const noisy = [];
  const records = [];
  for (let i = 0; i < count; i += 1) {
    const cluster = i % CLUSTERS; clusterByRecord.push(cluster);
    noisy.push([(random() - 0.5) * 2.6, Math.max(0, random() * 1.9), (random() - 0.5) * 2.6]);
    records.push({ id: `dy-${String(i).padStart(4, "0")}`, dataset_id: datasetId, record_type: "vector", label: `state · cluster ${cluster}`, payload: { kind: "vector", values: centroids[cluster], cluster } });
  }
  const ids = records.map((r) => r.id);
  // States: noisy -> representative over STEPS keyframes (reverse flow).
  const states = [];
  for (let step = 0; step < STEPS; step += 1) {
    const t = step / (STEPS - 1);
    const eased = smoothstep(t) * 0.9;
    states.push(ids.map((id, i) => { const s = noisy[i]; const r = centroids[clusterByRecord[i]]; return { record_id: id, position: [round(s[0] + (r[0] - s[0]) * eased), round(s[1] + (r[1] - s[1]) * eased), round(s[2] + (r[2] - s[2]) * eased)] }; }));
  }
  const cost = ids.map((id, i) => ({ record_id: id, value: round(euclidean(noisy[i], centroids[clusterByRecord[i]]) / 4) }));
  const coordinates = states.map((positions, step) => ({ id: `state-${String(step).padStart(2, "0")}`, dataset_id: datasetId, space_id: "dynamics-space", name: `reverse-flow state ${step}`, dimension: 3, record_positions: positions }));
  return {
    schema: SCHEMA, provenance: { generator: "build-hero-evidence.mjs", synthetic: true },
    datasets: [{ id: datasetId, title: "Dynamics & noise", description: `${count} record states over ${STEPS} reverse-flow keyframes.`, source: "synthetic", license: "CC0-1.0" }],
    records,
    relations: [{ id: "transition", dataset_id: datasetId, name: "transition metric", relation_type: "transition", value_type: "scalar", record_ids: ids, storage: "sparse_edge_list", values: knnEdges(centroids.map((c, i) => centroids[i % CLUSTERS]), ids, 4), metadata: {} }],
    spaces: [{ id: "dynamics-space", dataset_id: datasetId, record_ids: ids, primary_relation_id: "transition", space_type: "finite_metric_space", metadata: { keyframes: STEPS } }],
    properties: [
      { id: "transition-cost", dataset_id: datasetId, target_type: "record", value_type: "scalar", name: "transition cost", values: cost },
      { id: "cluster", dataset_id: datasetId, target_type: "record", value_type: "categorical", name: "representative", values: ids.map((id, i) => ({ record_id: id, value: `rep ${clusterByRecord[i]}` })) },
    ],
    graphs: [],
    coordinates,
    timelines: [{ id: "reverse-flow", dataset_id: datasetId, name: "reverse flow to representatives", steps: coordinates.map((c, step) => ({ name: c.name, coordinate_id: c.id, property_id: "transition-cost", t: step / (STEPS - 1) })) }],
    events: [],
    views: [],
    diagnostics: [],
  };
}

// ---------------------------------------------------------------------------
// W5: mixed records (>=2000 across >=4 types, families, kNN, previews)
// ---------------------------------------------------------------------------
const FAMILIES = [
  { id: "text", record_type: "string" },
  { id: "vector", record_type: "vector" },
  { id: "curve", record_type: "time_series_window" },
  { id: "histogram", record_type: "histogram" },
  { id: "image", record_type: "image" },
];
function mixedPayload(familyId, index, random) {
  if (familyId === "text") return { kind: "string", text: `event ${index}: ${["soft-limit", "warning", "trip", "reset"][index % 4]} at sensor ${index % 9}` };
  if (familyId === "vector") return { kind: "vector", values: Array.from({ length: 5 }, () => round(random())), names: ["rms", "peak", "crest", "kurtosis", "skew"] };
  if (familyId === "curve") return { kind: "time_series", unit: "g", sample_rate_hz: 256, series: Array.from({ length: 32 }, (_, k) => round(Math.sin(k / 5 + index) * 0.5 + 0.5)) };
  if (familyId === "histogram") return { kind: "histogram", bins: Array.from({ length: 8 }, () => Math.floor(random() * 14)), edges: Array.from({ length: 9 }, (_, e) => round(e / 8)) };
  return { kind: "image_ref", href: `./assets/mixed-${index}.png`, width: 192, height: 96, alt: `record ${index}` };
}
function buildMixedEvidence() {
  const datasetId = "mixed";
  const random = mulberry32(0x3a17c4);
  const perFamily = 420; // 2100 across 5 types
  const records = []; const positions = []; const familyByRecord = []; const features = [];
  FAMILIES.forEach((family, fi) => {
    const a = (fi / FAMILIES.length) * Math.PI * 2; const center = [Math.cos(a) * 1.25, 0.4 + fi * 0.14, Math.sin(a) * 1.25];
    for (let m = 0; m < perFamily; m += 1) {
      const index = records.length; familyByRecord.push(family.id);
      const p = [round(center[0] + (random() - 0.5) * 0.78), round(Math.max(0, center[1] + (random() - 0.5) * 0.42)), round(center[2] + (random() - 0.5) * 0.78)];
      positions.push(p); features.push([p[0], p[1], p[2], fi * 0.25 + (random() - 0.5) * 0.4]);
      records.push({ id: `mx-${String(index).padStart(4, "0")}`, dataset_id: datasetId, record_type: family.record_type, label: `${family.id} ${m}`, payload: mixedPayload(family.id, index, random) });
    }
  });
  const ids = records.map((r) => r.id);
  return {
    schema: SCHEMA, provenance: { generator: "build-hero-evidence.mjs", synthetic: true },
    datasets: [{ id: datasetId, title: "Mixed-record metric space", description: `${records.length} heterogeneous records across ${FAMILIES.length} types.`, source: "synthetic", license: "CC0-1.0" }],
    records,
    relations: [{ id: "cross-metric", dataset_id: datasetId, name: "cross-type metric", relation_type: "metric", value_type: "scalar", record_ids: ids, storage: "sparse_edge_list", values: knnEdges(features, ids, 5), metadata: { neighbors: 5 } }],
    spaces: [{ id: "mixed-space", dataset_id: datasetId, record_ids: ids, primary_relation_id: "cross-metric", space_type: "finite_metric_space", metadata: { families: FAMILIES.map((f) => f.id) } }],
    properties: [{ id: "family", dataset_id: datasetId, target_type: "record", value_type: "categorical", name: "record family", values: ids.map((id, i) => ({ record_id: id, value: familyByRecord[i] })) }],
    graphs: [{ id: "mixed-knn", dataset_id: datasetId, node_record_ids: ids, edge_relation_id: "cross-metric", graph_type: "k-nearest graph", edges: knnEdges(features, ids, 5).map((p) => ({ source: p.row_id, target: p.column_id, value: p.value })) }],
    coordinates: [{ id: "layout-3d", dataset_id: datasetId, space_id: "mixed-space", name: "family layout (3D)", dimension: 3, record_positions: ids.map((id, i) => ({ record_id: id, position: positions[i] })) }],
    timelines: [], events: [],
    views: [{ id: "mixed", kind: "metric-space", name: "Mixed record space", spaceId: "mixed-space", coordinateId: "layout-3d", propertyId: "family" }],
    diagnostics: [],
  };
}

// ---------------------------------------------------------------------------
// W3: relation matrix + neighborhood (>=128 records, dense matrix + graph)
// ---------------------------------------------------------------------------
function buildRelationNeighborhoodEvidence() {
  const datasetId = "metric-catalog";
  const random = mulberry32(0x510bb1);
  const count = 144;
  const CLUSTERS = 6;
  const centroids = Array.from({ length: CLUSTERS }, (_, c) => { const a = (c / CLUSTERS) * Math.PI * 2; const r = 0.8 + (c % 3) * 0.45; return [Math.cos(a) * r, 0.3 + (c % 4) * 0.36, Math.sin(a) * r]; });
  // Records are ordered CONTIGUOUSLY by cluster (group 0 first, then group 1,
  // ...). That makes the pairwise matrix read as a clean block-diagonal — dark
  // within-cluster blocks on the diagonal — instead of interleaved moiré noise.
  const perCluster = Math.ceil(count / CLUSTERS);
  const records = []; const positions = []; const clusterByRecord = [];
  for (let i = 0; i < count; i += 1) { const cluster = Math.min(CLUSTERS - 1, Math.floor(i / perCluster)); const c = centroids[cluster]; const p = [round(c[0] + (random() - 0.5) * 0.46), round(Math.max(0, c[1] + (random() - 0.5) * 0.34)), round(c[2] + (random() - 0.5) * 0.46)]; positions.push(p); clusterByRecord.push(cluster); records.push({ id: `m-${String(i).padStart(3, "0")}`, dataset_id: datasetId, record_type: "vector", label: `signal ${i} · group ${cluster}`, payload: { kind: "vector", values: p, group: cluster } }); }
  const ids = records.map((r) => r.id);
  const dense = [];
  for (let i = 0; i < count; i += 1) for (let j = i + 1; j < count; j += 1) dense.push({ row_id: ids[i], column_id: ids[j], value: round(euclidean(positions[i], positions[j])) });
  const edges = knnEdges(positions, ids, 5).map((p) => ({ source: p.row_id, target: p.column_id, value: p.value }));
  return {
    schema: SCHEMA, provenance: { generator: "build-hero-evidence.mjs", synthetic: true },
    datasets: [{ id: datasetId, title: "Metric catalog", description: `Dense pairwise metric over ${count} clustered signals.`, source: "synthetic", license: "CC0-1.0" }],
    records,
    relations: [{ id: "catalog-metric", dataset_id: datasetId, name: "dense metric", relation_type: "metric", value_type: "scalar", record_ids: ids, storage: "symmetric_dense_matrix", values: dense, metadata: { symmetric: true, law_check: { triangle: "exported", symmetry: "symmetric" } } }],
    spaces: [{ id: "catalog-space", dataset_id: datasetId, record_ids: ids, primary_relation_id: "catalog-metric", space_type: "finite_metric_space", metadata: { clusters: CLUSTERS } }],
    properties: [{ id: "group", dataset_id: datasetId, target_type: "record", value_type: "categorical", name: "signal group", values: ids.map((id, i) => ({ record_id: id, value: `group ${clusterByRecord[i]}` })) }],
    graphs: [{ id: "catalog-knn", dataset_id: datasetId, node_record_ids: ids, edge_relation_id: "catalog-metric", graph_type: "k-nearest graph", edges }],
    coordinates: [{ id: "catalog-3d", dataset_id: datasetId, space_id: "catalog-space", name: "metric layout (3D)", dimension: 3, record_positions: ids.map((id, i) => ({ record_id: id, position: positions[i] })) }],
    timelines: [], events: [],
    views: [{ id: "matrix", kind: "relation-matrix", name: "Dense metric matrix", relationId: "catalog-metric" }],
    diagnostics: [],
  };
}

// ---------------------------------------------------------------------------
// W2: record preview contract records (mixed payload families)
// ---------------------------------------------------------------------------
function buildPreviewRecords() {
  const datasetId = "preview";
  const random = mulberry32(0x9e1d);
  const records = [
    { id: "rp-text", dataset_id: datasetId, record_type: "string", label: "log line", payload: { kind: "string", text: "bearing temperature exceeded soft limit at 14:02:11" } },
    { id: "rp-vector", dataset_id: datasetId, record_type: "vector", label: "feature vector", payload: { kind: "vector", values: [0.21, 0.88, 0.34, 0.57, 0.12], names: ["rms", "peak", "crest", "kurtosis", "skew"] } },
    { id: "rp-series", dataset_id: datasetId, record_type: "time_series_window", label: "vibration window", payload: { kind: "time_series", unit: "g", sample_rate_hz: 512, series: Array.from({ length: 48 }, (_, k) => round(Math.sin(k / 6) * 0.5 + 0.5)) } },
    { id: "rp-histogram", dataset_id: datasetId, record_type: "histogram", label: "amplitude histogram", payload: { kind: "histogram", bins: [2, 5, 9, 14, 11, 6, 3, 1], edges: [0, 0.125, 0.25, 0.375, 0.5, 0.625, 0.75, 0.875, 1] } },
    { id: "rp-image", dataset_id: datasetId, record_type: "image", label: "spectrogram reference", payload: { kind: "image_ref", href: "./assets/spectrogram.png", width: 256, height: 128, alt: "spectrogram" } },
    { id: "rp-table", dataset_id: datasetId, record_type: "composed", label: "table-like record", payload: { kind: "composed", parts: [{ label: "asset", kind: "string", text: "pump-07" }, { label: "trend", kind: "vector", values: [0.4, 0.42, 0.51, 0.69] }] } },
  ];
  return {
    schema: SCHEMA, provenance: { generator: "build-hero-evidence.mjs", synthetic: true },
    datasets: [{ id: datasetId, title: "Record preview families", description: "One record per supported preview payload family.", source: "synthetic", license: "CC0-1.0" }],
    records, relations: [], spaces: [], properties: [], graphs: [], coordinates: [], timelines: [], events: [], views: [], diagnostics: [],
  };
}

const OUTPUTS = [
  ["fixtures/metric-visual-minimal.json", buildMinimalFixture],
  ["fixtures/metric-visual-relation.json", buildRelationContractFixture],
  ["condition-monitoring-hero/evidence.json", buildConditionEvidence],
  ["mapping-dimensionality-hero/evidence.json", buildMappingEvidence],
  ["cross-space-dependency-hero/evidence.json", buildCrossSpaceEvidence],
  ["dynamics-noise-hero/evidence.json", buildDynamicsEvidence],
  ["mixed-record-hero/evidence.json", buildMixedEvidence],
  ["relation-matrix-neighborhood/evidence.json", buildRelationNeighborhoodEvidence],
  ["record-preview-contract/records.json", buildPreviewRecords],
];

async function main() {
  const written = [];
  for (const [rel, build] of OUTPUTS) {
    const path = resolve(EXAMPLES, rel);
    await mkdir(dirname(path), { recursive: true });
    const doc = build();
    await writeFile(path, `${JSON.stringify(doc)}\n`, "utf8");
    written.push({ path: rel, records: doc.records?.length ?? 0 });
  }
  console.log(JSON.stringify({ ok: true, written }, null, 2));
}

main().catch((error) => { console.error(error); process.exitCode = 1; });
