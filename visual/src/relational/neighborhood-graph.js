import {
  collectRecordIds,
  indexRecordIds,
  pairSourceId,
  pairTargetId,
  pairValue,
  relationPairs,
} from "./relation-source.js";
import { createQuantileScaler, summarizeFiniteValues } from "./scaling.js";

/**
 * Build sparse neighborhood graph descriptors from exported relation values.
 *
 * This selects existing pair evidence for visualization. It does not compute
 * shortest paths, clustering, embeddings, or any derived metric algorithm.
 *
 * @param {object|Array} source
 * @param {object} [options]
 * @param {"topK"|"threshold"|"quantile"} [options.mode="topK"]
 * @param {number} [options.topK=8]
 * @param {number} [options.maxRelation] Include values <= maxRelation.
 * @param {number} [options.minRelation] Include values >= minRelation.
 * @param {number} [options.quantile=0.15] For distance-like values, lower quantile is nearer.
 * @param {"distance"|"similarity"} [options.relationKind="distance"]
 * @param {boolean} [options.directed=false]
 * @param {object|Map|Array} [options.positions] Optional id->vec3 positions.
 * @returns {object}
 */
export function buildSparseNeighborhoodGraph(source, options = {}) {
  const recordIds = collectRecordIds(source, options);
  const indexById = indexRecordIds(recordIds);
  const relationKind = options.relationKind || "distance";
  const directed = options.directed === true;
  const candidates = collectCandidates(source, { ...options, recordIds, indexById, directed });
  const selected = selectCandidates(candidates, options, relationKind);
  const nodes = buildNodes(recordIds, options.positions);
  const edges = selected.map((edge, index) => ({
    id: edge.id || `${edge.source}->${edge.target}:${index}`,
    source: edge.source,
    target: edge.target,
    sourceIndex: edge.sourceIndex,
    targetIndex: edge.targetIndex,
    value: edge.value,
    directed,
    pair: edge.pair,
  }));

  return {
    kind: "sparse-neighborhood-graph",
    mode: options.mode || "topK",
    relationKind,
    directed,
    recordIds,
    nodes,
    edges,
    edgeCount: edges.length,
    candidateCount: candidates.length,
    valueSummary: summarizeFiniteValues(candidates.map((edge) => edge.value)),
    channels: buildGraphChannels(nodes, edges, options),
  };
}

/**
 * Convert graph edges and optional positions into channel arrays compatible
 * with the existing RelationEdgeLayer primitive.
 *
 * @param {object} graph Output of buildSparseNeighborhoodGraph.
 * @param {object} [options]
 * @returns {object}
 */
export function buildGraphEdgeChannels(graph, options = {}) {
  const edges = graph.edges || [];
  const nodesById = new Map((graph.nodes || []).map((node) => [node.id, node]));
  const sourcePosition = new Float32Array(edges.length * 3);
  const targetPosition = new Float32Array(edges.length * 3);
  const value = new Float32Array(edges.length);

  for (let index = 0; index < edges.length; index += 1) {
    const edge = edges[index];
    const source = nodesById.get(edge.source);
    const target = nodesById.get(edge.target);
    writeVec3(sourcePosition, index * 3, source?.position || fallbackNodePosition(edge.sourceIndex, graph.recordIds.length));
    writeVec3(targetPosition, index * 3, target?.position || fallbackNodePosition(edge.targetIndex, graph.recordIds.length));
    value[index] = edge.value;
  }

  return {
    sourcePosition: { data: sourcePosition, size: 3 },
    targetPosition: { data: targetPosition, size: 3 },
    relationValue: { data: value, size: 1 },
    color: options.color || defaultEdgeColor(edges.length, options.alpha ?? 0.42),
  };
}

function collectCandidates(source, options) {
  const out = [];
  const seenUndirected = new Set();
  for (const pair of relationPairs(source)) {
    const sourceId = pairSourceId(pair, options.recordIds);
    const targetId = pairTargetId(pair, options.recordIds);
    const sourceIndex = options.indexById.get(sourceId);
    const targetIndex = options.indexById.get(targetId);
    const value = pairValue(pair, options.valueKey);
    if (sourceIndex == null || targetIndex == null || sourceIndex === targetIndex || !Number.isFinite(value)) continue;

    if (!options.directed) {
      const key = sourceIndex < targetIndex ? `${sourceIndex}:${targetIndex}` : `${targetIndex}:${sourceIndex}`;
      if (seenUndirected.has(key)) continue;
      seenUndirected.add(key);
    }

    out.push({
      source: sourceId,
      target: targetId,
      sourceIndex,
      targetIndex,
      value,
      pair,
    });
  }
  return out;
}

function selectCandidates(candidates, options, relationKind) {
  const mode = options.mode || "topK";
  if (mode === "threshold") return filterByThreshold(candidates, options);
  if (mode === "quantile") return filterByQuantile(candidates, options, relationKind);
  return filterTopK(candidates, options.topK || 8, relationKind, options.directed === true);
}

function filterTopK(candidates, topK, relationKind, directed) {
  const bySource = new Map();
  const selected = new Map();
  for (const edge of candidates) {
    if (!bySource.has(edge.source)) bySource.set(edge.source, []);
    bySource.get(edge.source).push(edge);
    if (!directed) {
      if (!bySource.has(edge.target)) bySource.set(edge.target, []);
      bySource.get(edge.target).push(edge);
    }
  }
  for (const edges of bySource.values()) {
    edges.sort((a, b) => relationKind === "similarity" ? b.value - a.value : a.value - b.value);
    for (const edge of edges.slice(0, Math.max(0, Math.floor(topK)))) {
      selected.set(`${edge.source}\u0000${edge.target}`, edge);
    }
  }
  return Array.from(selected.values());
}

function filterByThreshold(candidates, options) {
  const hasMin = Number.isFinite(options.minRelation);
  const hasMax = Number.isFinite(options.maxRelation);
  return candidates.filter((edge) => {
    if (hasMin && edge.value < options.minRelation) return false;
    if (hasMax && edge.value > options.maxRelation) return false;
    return true;
  });
}

function filterByQuantile(candidates, options, relationKind) {
  const quantile = Math.max(0, Math.min(1, Number(options.quantile ?? 0.15)));
  const values = candidates.map((edge) => edge.value);
  const scaler = createQuantileScaler(values, { buckets: Math.max(16, values.length || 16) });
  return candidates.filter((edge) => {
    const rank = scaler.scale(edge.value);
    return relationKind === "similarity" ? rank >= 1 - quantile : rank <= quantile;
  });
}

function buildNodes(recordIds, positions) {
  return recordIds.map((id, index) => ({
    id,
    index,
    position: readPosition(positions, id, index) || fallbackNodePosition(index, recordIds.length),
  }));
}

function buildGraphChannels(nodes, edges, options) {
  return buildGraphEdgeChannels({ nodes, edges, recordIds: nodes.map((node) => node.id) }, options);
}

function readPosition(positions, id, index) {
  if (!positions) return null;
  if (positions instanceof Map) return normalizeVec3(positions.get(id) || positions.get(index));
  if (Array.isArray(positions)) return normalizeVec3(positions[index]);
  return normalizeVec3(positions[id] || positions[index]);
}

function normalizeVec3(value) {
  if (!value) return null;
  return [
    Number(value[0] ?? value.x) || 0,
    Number(value[1] ?? value.y) || 0,
    Number(value[2] ?? value.z) || 0,
  ];
}

function fallbackNodePosition(index, count) {
  const angle = count > 0 ? (index / count) * Math.PI * 2 : 0;
  return [Math.cos(angle), Math.sin(angle), 0];
}

function writeVec3(target, offset, value) {
  target[offset] = Number(value[0]) || 0;
  target[offset + 1] = Number(value[1]) || 0;
  target[offset + 2] = Number(value[2]) || 0;
}

function defaultEdgeColor(count, alpha) {
  const data = new Float32Array(count * 4);
  for (let index = 0; index < count; index += 1) {
    const offset = index * 4;
    data[offset] = 0.17;
    data[offset + 1] = 0.34;
    data[offset + 2] = 0.42;
    data[offset + 3] = Number(alpha);
  }
  return { data, size: 4 };
}
