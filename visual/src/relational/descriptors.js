import { buildRelationMatrixTextureData } from "./matrix-texture.js";
import { buildGraphEdgeChannels, buildRelationNeighborhoodGraph } from "./neighborhood-graph.js";

/**
 * Create a layer descriptor for relation matrix rendering.
 *
 * @param {object|Array} source
 * @param {object} [options]
 * @returns {object}
 */
export function createRelationMatrixLayerDescriptor(source, options = {}) {
  const texture = source?.kind === "relation-matrix-texture-data"
    ? source
    : buildRelationMatrixTextureData(source, options);

  return {
    id: options.id || "relation-matrix",
    kind: "RelationMatrixLayer",
    primitive: "RelationMatrixLayer",
    order: options.order ?? 0,
    visible: options.visible !== false,
    source: {
      texture,
      recordIds: texture.recordIds,
    },
    geometry: {
      rect: options.rect || [0, 0, 1, 1],
      preserveAspect: options.preserveAspect !== false,
    },
    material: {
      alpha: options.alpha ?? 1,
      missingAlpha: options.missingAlpha ?? 0,
      background: options.background || [0.02, 0.025, 0.03, 1],
      blockLineAlpha: options.blockLineAlpha ?? 0.22,
      blockLineColor: options.blockLineColor || [1, 1, 1, 1],
    },
    metadata: {
      relationVisualization: "matrix",
      diagnostics: texture.diagnostics,
      matrix: texture.matrix,
      relationId: options.relationId || source?.id || null,
    },
  };
}

/**
 * Create a graph-edge layer descriptor from relation neighborhood evidence.
 *
 * @param {object|Array} source
 * @param {object} [options]
 * @returns {object}
 */
export function createRelationGraphEdgeLayerDescriptor(source, options = {}) {
  const graph = source?.kind === "sparse-neighborhood-graph" || source?.kind === "native-neighborhood-graph"
    ? source
    : buildRelationNeighborhoodGraph(source, options);

  return {
    id: options.id || "relation-neighborhood-edges",
    kind: "RelationEdgeLayer",
    primitive: "RelationEdgeLayer",
    order: options.order ?? 10,
    visible: options.visible !== false,
    source: {
      graph,
      recordIds: graph.recordIds,
      relationId: graph.relationId || options.relationId || source?.id || null,
      graphId: graph.id || options.graphId || options.graph?.id || null,
      nativeGraph: graph.native === true,
    },
    channels: options.channels || buildGraphEdgeChannels(graph, options),
    geometry: {
      width: options.width ?? 1,
    },
    material: {
      alpha: options.alpha ?? 0.72,
      depthWrite: options.depthWrite === true,
      transparent: true,
    },
    metadata: {
      relationVisualization: "neighborhood-graph",
      graph,
      diagnostics: graph.diagnostics,
    },
  };
}

/**
 * Create descriptors and metadata for focusing a selected record.
 *
 * The descriptor carries row/column relation values for matrix overlays and an
 * optional edge descriptor for spatial focus lines. A renderer can consume the
 * metadata directly or register a dedicated focus layer later.
 *
 * @param {object|Array} source Relation source or dense matrix.
 * @param {string|number|object} selectedRecord
 * @param {object} [options]
 * @returns {object}
 */
export function createSelectedRecordFocusDescriptor(source, selectedRecord, options = {}) {
  const texture = source?.kind === "relation-matrix-texture-data"
    ? source
    : buildRelationMatrixTextureData(source, options);
  const matrix = texture.matrix;
  const selectedId = resolveSelectedId(selectedRecord);
  const selectedIndex = matrix.recordIds.indexOf(selectedId);
  const row = [];
  const column = [];

  if (selectedIndex >= 0) {
    for (let index = 0; index < matrix.size; index += 1) {
      const rowOffset = selectedIndex * matrix.size + index;
      const columnOffset = index * matrix.size + selectedIndex;
      row.push({
        source: selectedId,
        target: matrix.recordIds[index],
        value: matrix.values[rowOffset],
        present: matrix.present[rowOffset] === 1,
      });
      column.push({
        source: matrix.recordIds[index],
        target: selectedId,
        value: matrix.values[columnOffset],
        present: matrix.present[columnOffset] === 1,
      });
    }
  }

  const graph = buildRelationNeighborhoodGraph(source, {
    ...options,
    mode: options.focusMode || "topK",
    topK: options.topK || 16,
  });
  const focusEdges = graph.edges.filter((edge) => edge.source === selectedId || edge.target === selectedId);

  return {
    id: options.id || `relation-focus-${selectedId || "none"}`,
    kind: "RelationFocusDescriptor",
    primitive: "RelationFocusDescriptor",
    visible: options.visible !== false,
    source: {
      selectedId,
      selectedIndex,
      row,
      column,
      focusEdges,
      recordIds: matrix.recordIds,
    },
    material: {
      color: options.color || [1, 0.82, 0.28, 1],
      alpha: options.alpha ?? 0.9,
    },
    metadata: {
      relationVisualization: "selected-record-focus",
      selectedId,
      selectedIndex,
      rowCount: row.filter((cell) => cell.present).length,
      columnCount: column.filter((cell) => cell.present).length,
      edgeCount: focusEdges.length,
    },
  };
}

function resolveSelectedId(value) {
  if (value == null) return "";
  if (typeof value === "string" || typeof value === "number") return String(value);
  return String(value.id ?? value.recordId ?? value.key ?? "");
}
