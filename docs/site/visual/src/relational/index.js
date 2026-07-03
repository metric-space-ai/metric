export {
  collectRecordIds,
  finiteRelationValues,
  indexRecordIds,
  pairSourceId,
  pairTargetId,
  pairValue,
  recordId,
  relationPairs,
  uniqueIds,
} from "./relation-source.js";

export {
  clamp01,
  computeQuantileBreakpoints,
  createMinMaxScaler,
  createQuantileScaler,
  resolveRelationScaler,
  scaleValuesToUnit,
  summarizeFiniteValues,
} from "./scaling.js";

export {
  checkAsymmetricRelations,
  checkSymmetricRelations,
  createRelationMatrixDiagnostics,
  diagnoseRelationSymmetry,
  RELATION_MATRIX_READABILITY_DIAGNOSTICS_SCHEMA,
} from "./diagnostics.js";

export {
  buildDenseRelationMatrix,
  buildRelationMatrixTextureData,
  resolvePalette,
  samplePalette,
  summarizeFiniteMatrix,
} from "./matrix-texture.js";

export {
  createRelationMatrixReadabilityProfile,
  summarizeMatrixTiles,
} from "./matrix-readability.js";

export {
  buildGraphEdgeChannels,
  buildNativeNeighborhoodGraph,
  buildRelationNeighborhoodGraph,
  buildSparseNeighborhoodGraph,
} from "./neighborhood-graph.js";

export {
  applyRelationEdgeLegibilityDescriptor,
  createRelationEdgeLegibilityProfile,
  RELATION_EDGE_LEGIBILITY_SCHEMA,
} from "./edge-legibility.js";

export {
  createRelationMatrixPicker,
  pickRelationMatrixCell,
} from "./matrix-picking.js";

export {
  createRelationGraphEdgeLayerDescriptor,
  createRelationMatrixLayerDescriptor,
  createSelectedRecordFocusDescriptor,
} from "./descriptors.js";

export {
  MATRIX_FRAGMENT_SHADER,
  MATRIX_VERTEX_SHADER,
  RelationMatrixLayer,
} from "./RelationMatrixLayer.js";
