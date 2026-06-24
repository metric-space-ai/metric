#!/usr/bin/env node
/*
 * Deterministic generator for metric.visual.v1 fixtures.
 *
 * These fixtures stand in for evidence that METRIC C++ exports. The generator
 * only authors synthetic data: it does not implement a METRIC algorithm that
 * the runtime is supposed to import. Distances, entropy and density here are
 * plain synthetic numbers attached to records, exactly as a C++ exporter would
 * attach already-computed evidence.
 *
 * Run: node visual/tools/build-fixtures.mjs
 * Output: visual/examples/fixtures/*.visual.json
 */

import { mkdir, writeFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const HERE = dirname(fileURLToPath(import.meta.url));
const FIXTURE_DIR = resolve(HERE, "..", "examples", "fixtures");

const SCHEMA = "metric.visual.v1";

// Four operating regimes for the synthetic condition-monitoring space.
const REGIMES = [
  { id: "healthy", label: "healthy", color: [0.20, 0.54, 0.78], base: 0.10 },
  { id: "drift", label: "drift", color: [0.75, 0.77, 0.24], base: 0.32 },
  { id: "fault", label: "fault", color: [0.88, 0.28, 0.31], base: 0.74 },
  { id: "recovery", label: "recovery", color: [0.46, 0.34, 0.72], base: 0.44 },
];

function mulberry32(seed) {
  let a = seed >>> 0;
  return function next() {
    a |= 0;
    a = (a + 0x6d2b79f5) | 0;
    let t = Math.imul(a ^ (a >>> 15), 1 | a);
    t = (t + Math.imul(t ^ (t >>> 7), 61 | t)) ^ t;
    return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
  };
}

function round(value, digits = 4) {
  const factor = 10 ** digits;
  return Math.round(Number(value) * factor) / factor;
}

function euclidean(a, b) {
  let sum = 0;
  for (let index = 0; index < a.length; index += 1) {
    const delta = a[index] - b[index];
    sum += delta * delta;
  }
  return Math.sqrt(sum);
}

/**
 * Build the canonical sensor-condition metric space. Records carry mixed
 * preview payloads, the metric relation is exported as a sparse kNN edge list,
 * and several coordinate states allow morph/mapping inspection.
 */
function buildMetricSpaceFixture() {
  const random = mulberry32(0x5e1f01);
  const datasetId = "sensor-condition";
  const recordCount = 24;
  const records = [];
  const featureVectors = [];
  const regimeByRecord = [];

  for (let index = 0; index < recordCount; index += 1) {
    const regime = REGIMES[index % REGIMES.length];
    regimeByRecord.push(regime);
    const phase = (index / recordCount) * Math.PI * 2;
    const amplitude = regime.base + 0.18 * random();
    const window = Array.from({ length: 32 }, (_, step) => {
      const t = step / 31;
      const carrier = Math.sin(phase + t * Math.PI * 4);
      const noise = (random() - 0.5) * (0.12 + regime.base * 0.4);
      return round(0.5 + amplitude * carrier * 0.5 + noise, 4);
    });
    const rms = round(Math.sqrt(window.reduce((sum, value) => sum + value * value, 0) / window.length), 4);
    const peak = round(Math.max(...window), 4);
    const crest = round(peak / Math.max(rms, 1e-3), 4);
    const features = [rms, peak, crest, round(amplitude, 4)];
    featureVectors.push(features);

    records.push({
      id: `rec-${String(index).padStart(2, "0")}`,
      dataset_id: datasetId,
      record_type: "time_series_window",
      label: `${regime.label} ${Math.floor(index / REGIMES.length)}`,
      payload: {
        kind: "time_series",
        regime: regime.id,
        unit: "normalized amplitude",
        sample_rate_hz: 256,
        series: window,
        features: { rms, peak, crest_factor: crest, amplitude: round(amplitude, 4) },
        summary: `Window ${index} sampled in the ${regime.label} regime`,
      },
    });
  }

  // Coordinate states. The 3D state is the metric-space lift; the 2D state is a
  // flat projection used for the source of the 2D->3D morph and mapping views.
  const positions3d = [];
  const positions2d = [];
  for (let index = 0; index < recordCount; index += 1) {
    const regime = regimeByRecord[index];
    const ring = REGIMES.indexOf(regime);
    const angle = (index / recordCount) * Math.PI * 2 + ring * 0.4;
    const radius = 0.6 + ring * 0.28 + random() * 0.12;
    const height = regime.base * 1.6 + random() * 0.2;
    positions3d.push([
      round(Math.cos(angle) * radius, 4),
      round(height, 4),
      round(Math.sin(angle) * radius, 4),
    ]);
    positions2d.push([
      round(Math.cos(angle) * radius, 4),
      round(Math.sin(angle) * radius, 4),
    ]);
  }

  // Sparse kNN metric relation (k=6) built from feature-vector distances. This
  // is synthetic exported evidence, not a runtime metric computation.
  const k = 6;
  const edges = [];
  const relationValues = [];
  for (let i = 0; i < recordCount; i += 1) {
    const neighbors = [];
    for (let j = 0; j < recordCount; j += 1) {
      if (i === j) continue;
      neighbors.push({ j, d: euclidean(featureVectors[i], featureVectors[j]) });
    }
    neighbors.sort((a, b) => a.d - b.d);
    for (const neighbor of neighbors.slice(0, k)) {
      relationValues.push({
        row_id: records[i].id,
        column_id: records[neighbor.j].id,
        value: round(neighbor.d, 4),
      });
      edges.push({
        source: records[i].id,
        target: records[neighbor.j].id,
        value: round(neighbor.d, 4),
      });
    }
  }

  const recordIds = records.map((record) => record.id);

  // Reverse-diffusion trajectory: records start spread out and noisy, then flow
  // back toward their regime representative over several exported states. This
  // is synthetic exported dynamics evidence, not a runtime simulation.
  const DIFFUSION_STEPS = 5;
  const representatives = new Map();
  for (const regime of REGIMES) {
    const members = positions3d.filter((_, index) => regimeByRecord[index].id === regime.id);
    const centroid = members
      .reduce((acc, point) => [acc[0] + point[0], acc[1] + point[1], acc[2] + point[2]], [0, 0, 0])
      .map((value) => value / Math.max(1, members.length));
    representatives.set(regime.id, centroid);
  }
  const noisyStart = positions3d.map((point) => [
    point[0] + (random() - 0.5) * 0.7,
    Math.max(0, point[1] + (random() - 0.5) * 0.4),
    point[2] + (random() - 0.5) * 0.7,
  ]);
  // Records flow toward (but never fully collapse onto) their representative, so
  // every exported state stays populated and visually distinct.
  const COLLAPSE = 0.82;
  const diffusionCoordinates = [];
  for (let step = 0; step < DIFFUSION_STEPS; step += 1) {
    const t = step / (DIFFUSION_STEPS - 1);
    const eased = t * t * (3 - 2 * t) * COLLAPSE;
    diffusionCoordinates.push({
      id: `diffusion-${step}`,
      dataset_id: datasetId,
      space_id: "sensor-space",
      name: `reverse diffusion step ${step}`,
      dimension: 3,
      record_positions: recordIds.map((id, index) => {
        const start = noisyStart[index];
        const rep = representatives.get(regimeByRecord[index].id);
        return {
          record_id: id,
          position: [
            round(start[0] + (rep[0] - start[0]) * eased, 4),
            round(start[1] + (rep[1] - start[1]) * eased, 4),
            round(start[2] + (rep[2] - start[2]) * eased, 4),
          ],
        };
      }),
    });
  }

  // Per-record properties exported alongside the space.
  const entropy = recordIds.map((id, index) => ({
    record_id: id,
    value: round(0.4 + regimeByRecord[index].base * 0.5 + random() * 0.1, 4),
  }));
  const density = recordIds.map((id, index) => ({
    record_id: id,
    value: round(0.9 - regimeByRecord[index].base * 0.6 + random() * 0.08, 4),
  }));
  const anomaly = recordIds.map((id, index) => ({
    record_id: id,
    value: round(regimeByRecord[index].base + random() * 0.12, 4),
  }));
  const cluster = recordIds.map((id, index) => ({
    record_id: id,
    value: regimeByRecord[index].id,
  }));
  const residual = recordIds.map((id, index) => ({
    record_id: id,
    value: round(Math.abs(positions3d[index][1] - positions2d[index][1]) * 0.3 + random() * 0.05, 4),
  }));

  return {
    schema: SCHEMA,
    provenance: {
      generator: "visual/tools/build-fixtures.mjs",
      synthetic: true,
      note: "Synthetic stand-in for METRIC C++ metric.visual.v1 export.",
    },
    datasets: [
      {
        id: datasetId,
        title: "Sensor condition monitoring",
        description: "Windowed vibration records across healthy, drift, fault and recovery regimes.",
        source: "synthetic",
        license: "CC0-1.0",
      },
    ],
    records,
    relations: [
      {
        id: "sensor-metric",
        dataset_id: datasetId,
        name: "feature-space metric",
        relation_type: "metric",
        value_type: "scalar",
        record_ids: recordIds,
        storage: "sparse_edge_list",
        values: relationValues,
        metadata: {
          symmetric: false,
          neighbors: k,
          law_check: { triangle: "exported", symmetry: "directed_knn" },
        },
      },
    ],
    spaces: [
      {
        id: "sensor-space",
        dataset_id: datasetId,
        record_ids: recordIds,
        primary_relation_id: "sensor-metric",
        space_type: "finite_metric_space",
        metadata: { dimension_hint: 3, regimes: REGIMES.map((regime) => regime.id) },
      },
    ],
    properties: [
      { id: "entropy", dataset_id: datasetId, target_type: "record", value_type: "scalar", name: "local entropy", values: entropy },
      { id: "density", dataset_id: datasetId, target_type: "record", value_type: "scalar", name: "local density", values: density },
      { id: "anomaly", dataset_id: datasetId, target_type: "record", value_type: "scalar", name: "anomaly score", values: anomaly },
      { id: "cluster", dataset_id: datasetId, target_type: "record", value_type: "categorical", name: "regime cluster", values: cluster },
      { id: "map-residual", dataset_id: datasetId, target_type: "record", value_type: "scalar", name: "mapping residual", values: residual },
    ],
    graphs: [
      {
        id: "sensor-knn",
        dataset_id: datasetId,
        node_record_ids: recordIds,
        edge_relation_id: "sensor-metric",
        graph_type: "k-nearest graph",
        edges,
      },
    ],
    coordinates: [
      {
        id: "landmark-3d",
        dataset_id: datasetId,
        space_id: "sensor-space",
        name: "metric-space lift (3D)",
        dimension: 3,
        record_positions: recordIds.map((id, index) => ({ record_id: id, position: positions3d[index] })),
      },
      {
        id: "landmark-2d",
        dataset_id: datasetId,
        space_id: "sensor-space",
        name: "flat projection (2D)",
        dimension: 2,
        record_positions: recordIds.map((id, index) => ({ record_id: id, position: positions2d[index] })),
      },
      ...diffusionCoordinates,
    ],
    timelines: [
      {
        id: "condition-morph",
        dataset_id: datasetId,
        name: "2D projection to 3D metric space",
        steps: [
          { name: "flat projection", coordinate_id: "landmark-2d", property_id: "density", t: 0 },
          { name: "metric-space lift", coordinate_id: "landmark-3d", property_id: "entropy", t: 1 },
        ],
      },
      {
        id: "reverse-diffusion",
        dataset_id: datasetId,
        name: "reverse diffusion to representatives",
        steps: diffusionCoordinates.map((coordinate, step) => ({
          name: coordinate.name,
          coordinate_id: coordinate.id,
          property_id: "anomaly",
          t: step / (diffusionCoordinates.length - 1),
        })),
      },
    ],
    events: [],
    views: [
      {
        id: "space-overview",
        kind: "metric-space",
        name: "Condition metric space",
        spaceId: "sensor-space",
        coordinateId: "landmark-3d",
        propertyId: "anomaly",
      },
      {
        id: "relation-overview",
        kind: "relation-matrix",
        name: "Pairwise metric matrix",
        relationId: "sensor-metric",
      },
    ],
    diagnostics: [],
  };
}

/**
 * A compact dense relation-matrix fixture. The relation is exported as a dense
 * symmetric matrix so the matrix view can be exercised independently.
 */
function buildRelationMatrixFixture() {
  const random = mulberry32(0x510bb1);
  const datasetId = "metric-catalog";
  const count = 120;
  const CLUSTERS = 6;
  const centroids = Array.from({ length: CLUSTERS }, (_, c) => {
    const angle = (c / CLUSTERS) * Math.PI * 2;
    const radius = 0.8 + (c % 3) * 0.45;
    return [Math.cos(angle) * radius, 0.3 + (c % 4) * 0.36, Math.sin(angle) * radius];
  });
  const records = [];
  const positions = [];
  const clusterByRecord = [];
  for (let index = 0; index < count; index += 1) {
    const cluster = index % CLUSTERS;
    const c = centroids[cluster];
    const position = [
      round(c[0] + (random() - 0.5) * 0.46, 4),
      round(Math.max(0, c[1] + (random() - 0.5) * 0.34), 4),
      round(c[2] + (random() - 0.5) * 0.46, 4),
    ];
    positions.push(position);
    clusterByRecord.push(cluster);
    records.push({
      id: `m-${String(index).padStart(3, "0")}`,
      dataset_id: datasetId,
      record_type: "vector",
      label: `signal ${index} · group ${cluster}`,
      payload: { kind: "vector", values: position, group: cluster },
    });
  }

  const recordIds = records.map((record) => record.id);
  // Dense symmetric pairwise metric (upper triangle).
  const values = [];
  for (let i = 0; i < count; i += 1) {
    for (let j = i + 1; j < count; j += 1) {
      values.push({ row_id: records[i].id, column_id: records[j].id, value: round(euclidean(positions[i], positions[j]), 4) });
    }
  }
  // Sparse kNN for the neighborhood graph.
  const k = 5;
  const edges = [];
  for (let i = 0; i < count; i += 1) {
    const neighbors = [];
    for (let j = 0; j < count; j += 1) { if (i !== j) neighbors.push({ j, d: euclidean(positions[i], positions[j]) }); }
    neighbors.sort((a, b) => a.d - b.d);
    for (const n of neighbors.slice(0, k)) edges.push({ source: records[i].id, target: records[n.j].id, value: round(n.d, 4) });
  }

  return {
    schema: SCHEMA,
    provenance: { generator: "visual/tools/build-fixtures.mjs", synthetic: true },
    datasets: [
      { id: datasetId, title: "Metric catalog", description: `Dense pairwise metric over ${count} clustered signals.`, source: "synthetic", license: "CC0-1.0" },
    ],
    records,
    relations: [
      {
        id: "catalog-metric",
        dataset_id: datasetId,
        name: "dense metric",
        relation_type: "metric",
        value_type: "scalar",
        record_ids: recordIds,
        storage: "symmetric_dense_matrix",
        values,
        metadata: { symmetric: true, law_check: { triangle: "exported", symmetry: "symmetric" } },
      },
    ],
    spaces: [
      { id: "catalog-space", dataset_id: datasetId, record_ids: recordIds, primary_relation_id: "catalog-metric", space_type: "finite_metric_space", metadata: { clusters: CLUSTERS } },
    ],
    properties: [
      { id: "group", dataset_id: datasetId, target_type: "record", value_type: "categorical", name: "signal group", values: recordIds.map((id, index) => ({ record_id: id, value: `group ${clusterByRecord[index]}` })) },
    ],
    graphs: [
      { id: "catalog-knn", dataset_id: datasetId, node_record_ids: recordIds, edge_relation_id: "catalog-metric", graph_type: "k-nearest graph", edges },
    ],
    coordinates: [
      { id: "catalog-3d", dataset_id: datasetId, space_id: "catalog-space", name: "metric layout (3D)", dimension: 3, record_positions: recordIds.map((id, index) => ({ record_id: id, position: positions[index] })) },
    ],
    timelines: [],
    events: [],
    views: [{ id: "matrix", kind: "relation-matrix", name: "Dense metric matrix", relationId: "catalog-metric" }],
    diagnostics: [],
  };
}

/**
 * Heterogeneous record fixture for the preview resolver. Each record uses a
 * different payload family so the preview path can be checked across types.
 */
function buildRecordPreviewFixture() {
  const datasetId = "mixed-records";
  const records = [
    {
      id: "rp-text",
      dataset_id: datasetId,
      record_type: "string",
      label: "log line",
      payload: { kind: "string", text: "bearing temperature exceeded soft limit at 14:02:11" },
    },
    {
      id: "rp-vector",
      dataset_id: datasetId,
      record_type: "vector",
      label: "feature vector",
      payload: { kind: "vector", values: [0.21, 0.88, 0.34, 0.57, 0.12], names: ["rms", "peak", "crest", "kurtosis", "skew"] },
    },
    {
      id: "rp-series",
      dataset_id: datasetId,
      record_type: "time_series_window",
      label: "vibration window",
      payload: {
        kind: "time_series",
        unit: "g",
        sample_rate_hz: 512,
        series: Array.from({ length: 48 }, (_, step) => round(Math.sin(step / 6) * 0.5 + 0.5, 4)),
      },
    },
    {
      id: "rp-histogram",
      dataset_id: datasetId,
      record_type: "histogram",
      label: "amplitude histogram",
      payload: { kind: "histogram", bins: [2, 5, 9, 14, 11, 6, 3, 1], edges: [0, 0.125, 0.25, 0.375, 0.5, 0.625, 0.75, 0.875, 1] },
    },
    {
      id: "rp-image",
      dataset_id: datasetId,
      record_type: "image",
      label: "spectrogram reference",
      payload: { kind: "image_ref", href: "./assets/spectrogram-rp-image.png", width: 256, height: 128, alt: "spectrogram" },
    },
    {
      id: "rp-composed",
      dataset_id: datasetId,
      record_type: "composed",
      label: "composed record",
      payload: {
        kind: "composed",
        parts: [
          { label: "asset", kind: "string", text: "pump-07" },
          { label: "trend", kind: "vector", values: [0.4, 0.42, 0.51, 0.69] },
        ],
      },
    },
  ];

  return {
    schema: SCHEMA,
    provenance: { generator: "visual/tools/build-fixtures.mjs", synthetic: true },
    datasets: [
      { id: datasetId, title: "Mixed record families", description: "One record per supported preview payload family.", source: "synthetic", license: "CC0-1.0" },
    ],
    records,
    relations: [],
    spaces: [],
    properties: [],
    graphs: [],
    coordinates: [],
    timelines: [],
    events: [],
    views: [],
    diagnostics: [],
  };
}

const FAMILIES = [
  { id: "text", record_type: "string", color: [0.20, 0.54, 0.78] },
  { id: "vector", record_type: "vector", color: [0.10, 0.67, 0.60] },
  { id: "curve", record_type: "time_series_window", color: [0.95, 0.55, 0.18] },
  { id: "histogram", record_type: "histogram", color: [0.46, 0.34, 0.72] },
  { id: "image", record_type: "image", color: [0.88, 0.28, 0.31] },
];

function mixedPayload(family, index, random) {
  if (family.id === "text") {
    return { kind: "string", text: `event ${index}: ${["soft-limit", "warning", "trip", "reset"][index % 4]} at sensor ${index % 7}` };
  }
  if (family.id === "vector") {
    return { kind: "vector", values: Array.from({ length: 5 }, () => round(random(), 4)), names: ["rms", "peak", "crest", "kurtosis", "skew"] };
  }
  if (family.id === "curve") {
    return { kind: "time_series", unit: "g", sample_rate_hz: 256, series: Array.from({ length: 32 }, (_, step) => round(Math.sin(step / 5 + index) * 0.5 + 0.5, 4)) };
  }
  if (family.id === "histogram") {
    return { kind: "histogram", bins: Array.from({ length: 8 }, () => Math.floor(random() * 14)), edges: Array.from({ length: 9 }, (_, edge) => round(edge / 8, 4)) };
  }
  return { kind: "image_ref", href: `./assets/mixed-${index}.png`, width: 192, height: 96, alt: `record ${index}` };
}

/**
 * Mixed-record metric space: heterogeneous record types compared in one space,
 * with type-appropriate preview payloads, family labels and cross-type nearest
 * links. Distances are synthetic exported evidence over a feature embedding.
 */
function buildMixedSpaceFixture() {
  const random = mulberry32(0x3a17c4);
  const datasetId = "mixed-space";
  const perFamily = 120;
  const records = [];
  const positions = [];
  const features = [];
  const familyByRecord = [];

  FAMILIES.forEach((family, familyIndex) => {
    const angle = (familyIndex / FAMILIES.length) * Math.PI * 2;
    const center = [Math.cos(angle) * 1.1, 0.4 + familyIndex * 0.12, Math.sin(angle) * 1.1];
    for (let member = 0; member < perFamily; member += 1) {
      const index = records.length;
      familyByRecord.push(family);
      const jitter = [
        (random() - 0.5) * 0.7,
        (random() - 0.5) * 0.4,
        (random() - 0.5) * 0.7,
      ];
      const position = [round(center[0] + jitter[0], 4), round(Math.max(0, center[1] + jitter[1]), 4), round(center[2] + jitter[2], 4)];
      positions.push(position);
      // A feature embedding that mostly separates families but keeps some
      // cross-type neighbors, so cross-type nearest links are interesting.
      features.push([position[0], position[1], position[2], familyIndex * 0.25 + (random() - 0.5) * 0.4]);
      records.push({
        id: `mx-${String(index).padStart(2, "0")}`,
        dataset_id: datasetId,
        record_type: family.record_type,
        label: `${family.id} ${member}`,
        payload: mixedPayload(family, index, random),
      });
    }
  });

  const recordIds = records.map((record) => record.id);
  const k = 5;
  const relationValues = [];
  for (let i = 0; i < records.length; i += 1) {
    const neighbors = [];
    for (let j = 0; j < records.length; j += 1) {
      if (i === j) continue;
      neighbors.push({ j, d: euclidean(features[i], features[j]) });
    }
    neighbors.sort((a, b) => a.d - b.d);
    for (const neighbor of neighbors.slice(0, k)) {
      relationValues.push({ row_id: recordIds[i], column_id: recordIds[neighbor.j], value: round(neighbor.d, 4) });
    }
  }

  return {
    schema: SCHEMA,
    provenance: { generator: "visual/tools/build-fixtures.mjs", synthetic: true },
    datasets: [{ id: datasetId, title: "Mixed-record metric space", description: "Heterogeneous record types in one finite metric space.", source: "synthetic", license: "CC0-1.0" }],
    records,
    relations: [{
      id: "cross-metric",
      dataset_id: datasetId,
      name: "cross-type metric",
      relation_type: "metric",
      value_type: "scalar",
      record_ids: recordIds,
      storage: "sparse_edge_list",
      values: relationValues,
      metadata: { neighbors: k, law_check: { triangle: "exported", symmetry: "directed_knn" } },
    }],
    spaces: [{ id: "mixed-space-obj", dataset_id: datasetId, record_ids: recordIds, primary_relation_id: "cross-metric", space_type: "finite_metric_space", metadata: { families: FAMILIES.map((f) => f.id) } }],
    properties: [{
      id: "family",
      dataset_id: datasetId,
      target_type: "record",
      value_type: "categorical",
      name: "record family",
      values: recordIds.map((id, index) => ({ record_id: id, value: familyByRecord[index].id })),
    }],
    graphs: [{ id: "mixed-knn", dataset_id: datasetId, node_record_ids: recordIds, edge_relation_id: "cross-metric", graph_type: "k-nearest graph", edges: relationValues.map((pair) => ({ source: pair.row_id, target: pair.column_id, value: pair.value })) }],
    coordinates: [{
      id: "layout-3d",
      dataset_id: datasetId,
      space_id: "mixed-space-obj",
      name: "family layout (3D)",
      dimension: 3,
      record_positions: recordIds.map((id, index) => ({ record_id: id, position: positions[index] })),
    }],
    timelines: [],
    events: [],
    views: [{ id: "mixed", kind: "metric-space", name: "Mixed record space", spaceId: "mixed-space-obj", coordinateId: "layout-3d", propertyId: "family" }],
    diagnostics: [],
  };
}

/**
 * Paired metric spaces over the same records: two different coordinate states
 * (A and B) plus an exported local-dependence score per record and a global
 * dependence statistic. Brushing one space highlights paired records in the
 * other. Dependence is synthetic exported evidence, not recomputed in JS.
 */
function buildPairedSpacesFixture() {
  const random = mulberry32(0x71c3a9);
  const datasetId = "paired";
  const count = 640;
  const records = Array.from({ length: count }, (_, index) => ({
    id: `pr-${String(index).padStart(2, "0")}`,
    dataset_id: datasetId,
    record_type: "vector",
    label: `obs ${index}`,
    payload: { kind: "vector", values: Array.from({ length: 4 }, () => round(random(), 4)) },
  }));
  const recordIds = records.map((record) => record.id);

  // Several 3D clusters. Space B keeps most clusters coherent but distorts a
  // couple of them (and a noisy fraction), so local dependence varies spatially.
  const CLUSTERS = 7;
  const centroids = Array.from({ length: CLUSTERS }, (_, c) => {
    const angle = (c / CLUSTERS) * Math.PI * 2;
    const radius = 0.7 + (c % 3) * 0.5;
    return [Math.cos(angle) * radius, 0.35 + (c % 4) * 0.34, Math.sin(angle) * radius];
  });
  const distortedClusters = new Set([2, 5]);
  const posA = [];
  const posB = [];
  const dependence = [];
  for (let index = 0; index < count; index += 1) {
    const cluster = index % CLUSTERS;
    const c = centroids[cluster];
    const a = [
      round(c[0] + (random() - 0.5) * 0.5, 4),
      round(Math.max(0, c[1] + (random() - 0.5) * 0.4), 4),
      round(c[2] + (random() - 0.5) * 0.5, 4),
    ];
    const distort = distortedClusters.has(cluster) ? 0.85 : (random() < 0.08 ? 0.7 : 0.1);
    const b = [
      round(a[0] + (random() - 0.5) * distort * 1.6, 4),
      round(Math.max(0, a[1] + (random() - 0.5) * distort), 4),
      round(a[2] + (random() - 0.5) * distort * 1.6, 4),
    ];
    posA.push(a);
    posB.push(b);
    dependence.push({ record_id: recordIds[index], value: round(1 - distort, 4) });
  }

  const knn = (positions) => {
    const values = [];
    for (let i = 0; i < count; i += 1) {
      const neighbors = [];
      for (let j = 0; j < count; j += 1) {
        if (i === j) continue;
        neighbors.push({ j, d: euclidean(positions[i], positions[j]) });
      }
      neighbors.sort((x, y) => x.d - y.d);
      for (const neighbor of neighbors.slice(0, 5)) values.push({ row_id: recordIds[i], column_id: recordIds[neighbor.j], value: round(neighbor.d, 4) });
    }
    return values;
  };

  const globalDependence = round(dependence.reduce((sum, entry) => sum + entry.value, 0) / count, 4);

  return {
    schema: SCHEMA,
    provenance: { generator: "visual/tools/build-fixtures.mjs", synthetic: true, global_dependence: globalDependence },
    datasets: [{ id: datasetId, title: "Paired metric spaces", description: "Two metric spaces over the same paired observations.", source: "synthetic", license: "CC0-1.0" }],
    records,
    relations: [
      { id: "relation-a", dataset_id: datasetId, name: "space A metric", relation_type: "metric", value_type: "scalar", record_ids: recordIds, storage: "sparse_edge_list", values: knn(posA), metadata: {} },
      { id: "relation-b", dataset_id: datasetId, name: "space B metric", relation_type: "metric", value_type: "scalar", record_ids: recordIds, storage: "sparse_edge_list", values: knn(posB), metadata: {} },
    ],
    spaces: [
      { id: "space-a", dataset_id: datasetId, record_ids: recordIds, primary_relation_id: "relation-a", space_type: "paired_space", metadata: { pair: "B", global_dependence: globalDependence } },
      { id: "space-b", dataset_id: datasetId, record_ids: recordIds, primary_relation_id: "relation-b", space_type: "paired_space", metadata: { pair: "A", global_dependence: globalDependence } },
    ],
    properties: [{ id: "local-dependence", dataset_id: datasetId, target_type: "record", value_type: "scalar", name: "local dependence", values: dependence }],
    graphs: [],
    coordinates: [
      { id: "space-a-3d", dataset_id: datasetId, space_id: "space-a", name: "space A coordinates", dimension: 3, record_positions: recordIds.map((id, index) => ({ record_id: id, position: posA[index] })) },
      { id: "space-b-3d", dataset_id: datasetId, space_id: "space-b", name: "space B coordinates", dimension: 3, record_positions: recordIds.map((id, index) => ({ record_id: id, position: posB[index] })) },
    ],
    timelines: [],
    events: [],
    views: [
      { id: "paired-a", kind: "metric-space", name: "Space A", spaceId: "space-a", coordinateId: "space-a-3d", propertyId: "local-dependence" },
      { id: "paired-b", kind: "metric-space", name: "Space B", spaceId: "space-b", coordinateId: "space-b-3d", propertyId: "local-dependence" },
    ],
    diagnostics: [{ id: "global-dependence", kind: "dependence-statistic", value: globalDependence, note: "synthetic global dependence between space A and B" }],
  };
}

/**
 * Dense condition-monitoring metric space. Many process runs degrade
 * (healthy -> drift -> fault) and recover, tracing swirling arms that rise into
 * a fault ridge — a real 3D sculpture, not a handful of dots. ~1500 records with
 * exported entropy/density/anomaly and a sparse kNN metric relation.
 */
function buildConditionDenseFixture() {
  const random = mulberry32(0x0fde12);
  const datasetId = "condition-dense";
  const RUNS = 16;
  const STEPS = 96;
  const records = [];
  const positions3d = [];
  const positions2d = [];
  const features = [];
  const regimeByRecord = [];
  const severityByRecord = [];

  const regimeFor = (t) => (t < 0.32 ? REGIMES[0] : t < 0.58 ? REGIMES[1] : t < 0.82 ? REGIMES[2] : REGIMES[3]);

  for (let run = 0; run < RUNS; run += 1) {
    const angle0 = (run / RUNS) * Math.PI * 2;
    const swirl = 1.0 + (run % 3) * 0.45;
    const runLift = 0.85 + (run % 4) * 0.12;
    for (let step = 0; step < STEPS; step += 1) {
      const index = records.length;
      const t = step / (STEPS - 1);
      // Severity ramps up into the fault ridge near t~0.74 then recovers.
      const severity = Math.max(0, Math.sin(Math.min(1, t / 0.78) * Math.PI * 0.92)) * (t < 0.82 ? 1 : 0.42 + (1 - t) * 1.2);
      const regime = regimeFor(t);
      const radius = 0.45 + severity * 1.35 + random() * 0.06;
      const height = severity * 1.9 * runLift + random() * 0.05;
      const angle = angle0 + t * 1.5 * swirl + (random() - 0.5) * 0.12;
      const x = round(Math.cos(angle) * radius + (random() - 0.5) * 0.08, 4);
      const z = round(Math.sin(angle) * radius + (random() - 0.5) * 0.08, 4);
      const y = round(height, 4);
      positions3d.push([x, y, z]);
      positions2d.push([x, z]);
      features.push([x, y, z, severity]);
      regimeByRecord.push(regime);
      severityByRecord.push(severity);

      const amp = 0.18 + severity * 0.7;
      const phase = run * 0.6 + t * Math.PI * 3;
      const series = Array.from({ length: 24 }, (_, k) => {
        const u = k / 23;
        return round(0.5 + amp * Math.sin(phase + u * Math.PI * 4) * 0.5 + (random() - 0.5) * (0.05 + severity * 0.18), 4);
      });
      records.push({
        id: `cd-${String(index).padStart(4, "0")}`,
        dataset_id: datasetId,
        record_type: "time_series_window",
        label: `run ${run} · ${regime.label}`,
        payload: { kind: "time_series", regime: regime.id, run, step, sample_rate_hz: 256, series, features: { severity: round(severity, 4) } },
      });
    }
  }

  const recordIds = records.map((record) => record.id);
  const k = 6;
  const relationValues = [];
  for (let i = 0; i < records.length; i += 1) {
    const neighbors = [];
    for (let j = 0; j < records.length; j += 1) {
      if (i === j) continue;
      neighbors.push({ j, d: euclidean(features[i], features[j]) });
    }
    neighbors.sort((a, b) => a.d - b.d);
    for (const neighbor of neighbors.slice(0, k)) {
      relationValues.push({ row_id: recordIds[i], column_id: recordIds[neighbor.j], value: round(neighbor.d, 4) });
    }
  }

  const anomaly = recordIds.map((id, index) => ({ record_id: id, value: round(Math.min(1, severityByRecord[index] + random() * 0.06), 4) }));
  const entropy = recordIds.map((id, index) => ({ record_id: id, value: round(0.35 + severityByRecord[index] * 0.5 + random() * 0.08, 4) }));
  const density = recordIds.map((id, index) => ({ record_id: id, value: round(0.92 - severityByRecord[index] * 0.55 + random() * 0.06, 4) }));
  const cluster = recordIds.map((id, index) => ({ record_id: id, value: regimeByRecord[index].id }));
  // Mapping residual: flattening 3D -> 2D loses the most where the fault ridge
  // rises, so residual tracks lift height.
  const mapResidual = recordIds.map((id, index) => ({ record_id: id, value: round(Math.min(1, positions3d[index][1] / 2.2 + random() * 0.05), 4) }));

  // Reverse-diffusion states: records start noisy and flow back to their regime
  // representative over several exported states (dense flowing cloud).
  const DIFFUSION_STEPS = 5;
  const reps = new Map();
  for (const regime of REGIMES) {
    const members = positions3d.filter((_, i) => regimeByRecord[i].id === regime.id);
    const c = members.reduce((acc, p) => [acc[0] + p[0], acc[1] + p[1], acc[2] + p[2]], [0, 0, 0]).map((v) => v / Math.max(1, members.length));
    reps.set(regime.id, c);
  }
  const noisy = positions3d.map((p) => [p[0] + (random() - 0.5) * 1.1, Math.max(0, p[1] + (random() - 0.5) * 0.7), p[2] + (random() - 0.5) * 1.1]);
  const diffusion = [];
  for (let step = 0; step < DIFFUSION_STEPS; step += 1) {
    const t = step / (DIFFUSION_STEPS - 1);
    const eased = t * t * (3 - 2 * t) * 0.86;
    diffusion.push({
      id: `cd-diffusion-${step}`,
      dataset_id: datasetId,
      space_id: "condition-space",
      name: `reverse diffusion step ${step}`,
      dimension: 3,
      record_positions: recordIds.map((id, index) => {
        const s = noisy[index];
        const r = reps.get(regimeByRecord[index].id);
        return { record_id: id, position: [round(s[0] + (r[0] - s[0]) * eased, 4), round(s[1] + (r[1] - s[1]) * eased, 4), round(s[2] + (r[2] - s[2]) * eased, 4)] };
      }),
    });
  }

  return {
    schema: SCHEMA,
    provenance: { generator: "visual/tools/build-fixtures.mjs", synthetic: true, note: "Dense synthetic condition-monitoring metric space." },
    datasets: [{ id: datasetId, title: "Condition monitoring (dense)", description: `${records.length} windowed records across ${RUNS} degrading/recovering process runs.`, source: "synthetic", license: "CC0-1.0" }],
    records,
    relations: [{ id: "condition-metric", dataset_id: datasetId, name: "feature metric", relation_type: "metric", value_type: "scalar", record_ids: recordIds, storage: "sparse_edge_list", values: relationValues, metadata: { neighbors: k, symmetric: false } }],
    spaces: [{ id: "condition-space", dataset_id: datasetId, record_ids: recordIds, primary_relation_id: "condition-metric", space_type: "finite_metric_space", metadata: { runs: RUNS, regimes: REGIMES.map((r) => r.id) } }],
    properties: [
      { id: "anomaly", dataset_id: datasetId, target_type: "record", value_type: "scalar", name: "anomaly score", values: anomaly },
      { id: "entropy", dataset_id: datasetId, target_type: "record", value_type: "scalar", name: "local entropy", values: entropy },
      { id: "density", dataset_id: datasetId, target_type: "record", value_type: "scalar", name: "local density", values: density },
      { id: "cluster", dataset_id: datasetId, target_type: "record", value_type: "categorical", name: "regime", values: cluster },
      { id: "map-residual", dataset_id: datasetId, target_type: "record", value_type: "scalar", name: "mapping residual", values: mapResidual },
    ],
    graphs: [],
    coordinates: [
      { id: "condition-3d", dataset_id: datasetId, space_id: "condition-space", name: "metric-space lift (3D)", dimension: 3, record_positions: recordIds.map((id, index) => ({ record_id: id, position: positions3d[index] })) },
      { id: "condition-2d", dataset_id: datasetId, space_id: "condition-space", name: "flat projection (2D)", dimension: 2, record_positions: recordIds.map((id, index) => ({ record_id: id, position: positions2d[index] })) },
      ...diffusion,
    ],
    timelines: [
      { id: "reverse-diffusion", dataset_id: datasetId, name: "reverse diffusion to representatives", steps: diffusion.map((coord, step) => ({ name: coord.name, coordinate_id: coord.id, property_id: "anomaly", t: step / (diffusion.length - 1) })) },
    ],
    events: [],
    views: [{ id: "condition", kind: "metric-space", name: "Condition metric space", spaceId: "condition-space", coordinateId: "condition-3d", propertyId: "anomaly" }],
    diagnostics: [],
  };
}

// Invalid fixtures for validator negative tests.
function buildInvalidMissingRecordId() {
  const doc = buildRelationMatrixFixture();
  delete doc.records[3].id; // remove a required record id
  return doc;
}

function buildInvalidRelationRef() {
  const doc = buildRelationMatrixFixture();
  doc.relations[0].record_ids.push("does-not-exist"); // broken relation reference
  return doc;
}

const FIXTURES = [
  ["metric-space.visual.json", buildMetricSpaceFixture],
  ["relation-matrix.visual.json", buildRelationMatrixFixture],
  ["record-preview.visual.json", buildRecordPreviewFixture],
  ["condition-dense.visual.json", buildConditionDenseFixture],
  ["mixed-space.visual.json", buildMixedSpaceFixture],
  ["paired-spaces.visual.json", buildPairedSpacesFixture],
  ["invalid-missing-record-id.visual.json", buildInvalidMissingRecordId],
  ["invalid-relation-ref.visual.json", buildInvalidRelationRef],
];

async function main() {
  await mkdir(FIXTURE_DIR, { recursive: true });
  const written = [];
  for (const [name, build] of FIXTURES) {
    const document = build();
    const path = resolve(FIXTURE_DIR, name);
    await writeFile(path, `${JSON.stringify(document, null, 2)}\n`, "utf8");
    written.push(name);
  }
  console.log(JSON.stringify({ ok: true, dir: FIXTURE_DIR, written }, null, 2));
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
