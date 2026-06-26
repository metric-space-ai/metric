import { buildRelationMatrixTextureData } from "./matrix-texture.js";
import { createRelationMatrixReadabilityProfile } from "./matrix-readability.js";
import { createRelationMatrixDiagnostics } from "./diagnostics.js";
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
  const readability = createRelationMatrixReadabilityProfile(texture.matrix, options);
  const denseCellSmoothing = readability.lod.denseCellSmoothing;
  const relationId = options.relationId || source?.id || source?.relation_id || texture.relationId || texture.matrix?.relationId || null;
  const relationName = options.relationName || source?.name || source?.label || texture.relationName || texture.matrix?.relationName || null;
  const readabilityDiagnostics = createRelationMatrixDiagnostics({
    matrix: texture.matrix,
    readability,
    relationId,
    relationName,
  });

  return {
    id: options.id || "relation-matrix",
    kind: "RelationMatrixLayer",
    primitive: "RelationMatrixLayer",
    order: options.order ?? 0,
    visible: options.visible !== false,
    source: {
      texture,
      recordIds: texture.recordIds,
      relationId,
      relationName,
    },
    geometry: {
      rect: options.rect || [0, 0, 1, 1],
      preserveAspect: options.preserveAspect !== false,
    },
    material: {
      alpha: resolveMaterialAlpha(options),
      missingAlpha: options.missingAlpha ?? 0,
      background: options.background || [0.02, 0.025, 0.03, 1],
      smoothingCellPixels: denseCellSmoothing.smoothingCellPixels,
      smoothingStrength: denseCellSmoothing.smoothingStrength,
      lodSmoothingStrength: denseCellSmoothing.lodSmoothingStrength,
      lodSmoothingStartCellPixels: denseCellSmoothing.startCellPixels,
      lodSmoothingFullCellPixels: denseCellSmoothing.fullCellPixels,
      selectionAlpha: options.selectionAlpha ?? 0.54,
      selectionRowAlpha: options.selectionRowAlpha ?? options.selectionAlpha ?? 0.54,
      selectionColumnAlpha: options.selectionColumnAlpha ?? options.selectionAlpha ?? 0.54,
      selectionCellAlpha: options.selectionCellAlpha ?? 0.9,
      selectionOutlineAlpha: options.selectionOutlineAlpha ?? 1,
      selectionOutlinePixels: options.selectionOutlinePixels ?? 2.2,
      selectionColor: options.selectionColor || [1, 0.86, 0.42, 1],
      selectionRowColor: options.selectionRowColor || options.selectionColor || [1, 0.86, 0.42, 1],
      selectionColumnColor: options.selectionColumnColor || [0.42, 0.66, 1, 1],
      selectionCellColor: options.selectionCellColor || options.selectionColor || [1, 0.92, 0.56, 1],
      focusBackdropAlpha: options.focusBackdropAlpha ?? 0.38,
      focusBlockAlpha: options.focusBlockAlpha ?? 0.14,
      blockBandAlpha: options.blockBandAlpha ?? 0.11,
      blockBandColor: options.blockBandColor || [1, 0.95, 0.72, 1],
      blockLineAlpha: options.blockLineAlpha ?? 0.64,
      blockLineColor: options.blockLineColor || [1, 1, 1, 1],
      blockLineWidthCells: options.blockLineWidthCells ?? 1.15,
      outerBorderAlpha: options.outerBorderAlpha ?? 0.7,
      tileSize: readability.tiles.tileSize,
      tileBoundaryAlpha: options.tileBoundaryAlpha ?? 0.2,
      tileBoundaryWidthCells: options.tileBoundaryWidthCells ?? 0.5,
      tileBoundaryColor: options.tileBoundaryColor || [0.72, 0.84, 1, 1],
    },
    picking: {
      mode: "semantic-matrix-picker",
      source: "dense-relation-matrix",
      preservesNativePairIdentity: true,
      domFallback: false,
      svgFallback: false,
    },
    metadata: {
      relationVisualization: "matrix",
      diagnostics: {
        ...texture.diagnostics,
        blockCount: readabilityDiagnostics.blockCount,
        tileCount: readabilityDiagnostics.tileCount,
        tileSummarySource: readabilityDiagnostics.tileSummarySource,
        selected: readabilityDiagnostics.selected,
        matrixReadability: readabilityDiagnostics,
      },
      matrix: texture.matrix,
      readability,
      readabilityDiagnostics,
      relationId,
      relationName,
      selectionModel: {
        relationId,
        recordIds: texture.recordIds,
        picker: "semantic-matrix-picker",
        preservesNativePairIdentity: true,
        respondsTo: ["record", "pair"],
        selectedFeatures: ["row", "column", "cell"],
        selectedFeatureSemantics: readability.selection,
      },
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
      diagnostics: {
        ...graph.diagnostics,
        relationId: graph.relationId || options.relationId || source?.id || null,
        edgeCount: graph.edgeCount,
        graphEdgeCount: graph.edgeCount,
      },
      selectionModel: {
        relationId: graph.relationId || options.relationId || source?.id || null,
        graphId: graph.id || options.graphId || options.graph?.id || null,
        pairSource: graph.native === true ? "native-graph-evidence" : "relation-pair-evidence",
        recordIds: graph.recordIds,
        respondsTo: ["record", "pair"],
      },
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

function resolveMaterialAlpha(options = {}) {
  const explicit = Number(options.materialAlpha ?? options.opacity);
  if (Number.isFinite(explicit)) return explicit;
  const alpha = Number(options.alpha);
  return Number.isFinite(alpha) && alpha >= 0 && alpha <= 1 ? alpha : 1;
}
