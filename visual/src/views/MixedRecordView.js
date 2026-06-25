import { BaseView } from "./BaseView.js";
import { MetricSpaceView } from "./MetricSpaceView.js";
import { VisualLayer } from "./VisualLayer.js";
import { createRelationGraphEdgeLayerDescriptor } from "../relational/descriptors.js";
import {
  RECORD_GLYPH_GRAMMAR_SCHEMA,
  createRecordGlyphGrammar,
} from "../glyphs/index.js";
import {
  colorChannelFrom,
  createChannel,
  createStringChannel,
  descriptorSource,
  extractPropertyValues,
  flattenCategories,
  flattenValues,
  flattenVectors,
  resolveCollectionItem,
  valueForId,
} from "./view-utils.js";

const MIXED_RECORD_GLYPH_MATERIAL = Object.freeze({
  pointPixelScale: 15,
  minPointSize: 4,
  maxPointSize: 56,
  alphaMode: "blend",
  transparent: true,
  alpha: 0.96,
  glyphGrammar: "typed-record-glyphs",
  diffuse: "record-payload-marks",
  glyphStroke: 0.72,
  glyphInk: [0.08, 0.12, 0.14],
  focusBoost: 0.42,
});

/**
 * MixedRecordView is the typed glyph grammar for heterogeneous METRIC records.
 *
 * It reads exported coordinates, record payload metadata and relation evidence,
 * then emits primary typed-glyph descriptors plus relation-edge context. It
 * does not compute METRIC distances or record embeddings.
 */
export class MixedRecordView extends BaseView {
  constructor(options = {}) {
    super({ ...options, kind: "mixed-records" });
    this.space = options.space || null;
    this.recordIds = (options.recordIds || this.space?.recordIds || []).map(String);
    this.records = options.records || this.space?.records || [];
    this.positions = options.positions || this.space?.positions || new Map();
    this.targetPositions = options.targetPositions || this.space?.targetPositions || null;
    this.colorValues = options.colorValues || this.space?.colorValues || null;
    this.scalarValues = options.scalarValues || this.space?.scalarValues || null;
    this.sizeValues = options.sizeValues || this.space?.sizeValues || null;
    this.edgeTypeValues = options.edgeTypeValues || this.colorValues || null;
    this.glyphBy = options.glyphBy || options.colorProperty || options.colorPropertyId || "record_type";
    this.edgeTypeProperty = options.edgeTypeProperty || options.crossTypeProperty || this.glyphBy;
    this.size = Number.isFinite(Number(options.size)) ? Number(options.size) : 1.15;
    this.alpha = Number.isFinite(Number(options.alpha)) ? Number(options.alpha) : 0.96;
    this.glyphLabelLift = Number.isFinite(Number(options.glyphLabelLift)) ? Number(options.glyphLabelLift) : 0.18;
    this.material = { ...MIXED_RECORD_GLYPH_MATERIAL, ...(options.material || options.glyphMaterial || {}) };
    this.recordGlyphGrammar = options.recordGlyphGrammar || createRecordGlyphGrammar(this.records, this.recordIds);

    this.relation = options.relation || null;
    this.graph = options.graph || null;
    this.showRelationEdges = options.relationEdges !== false && options.edges !== false;
    this.crossTypeRelations = options.crossTypeRelations !== false;
    this.relationKind = options.relationKind || (this.relation?.relation_type === "similarity" ? "similarity" : "distance");
    this.mode = options.mode || "topK";
    this.topK = Number.isFinite(Number(options.topK)) ? Number(options.topK) : 4;
    this.directed = options.directed == null ? undefined : Boolean(options.directed);
    this.edgeAlpha = Number.isFinite(Number(options.edgeAlpha)) ? Number(options.edgeAlpha) : 0.38;
    this.edgeWidth = Number.isFinite(Number(options.edgeWidth)) ? Number(options.edgeWidth) : 1;
    this.edgeOrder = Number.isFinite(Number(options.edgeOrder)) ? Number(options.edgeOrder) : 12;
  }

  static fromVisualSpace(document, options = {}) {
    const space = MetricSpaceView.fromVisualSpace(document, {
      ...options,
      recordGlyphs: false,
    });
    const relationRef = options.relation || options.relationId || options.relation_id;
    const relation = resolveCollectionItem(document, "relations", relationRef, {
      required: relationRef != null,
      label: "relation",
    }) || firstRelation(document);
    const graphRef = options.graph || options.graphId || options.graph_id;
    const graph = resolveCollectionItem(document, "graphs", graphRef, {
      required: graphRef != null,
      label: "graph",
    }) || firstGraphForRelation(document, relation?.id);
    const edgeTypePropertyRef = options.edgeTypeProperty
      || options.crossTypeProperty
      || options.glyphBy
      || options.colorProperty
      || options.colorPropertyId
      || options.recordType
      || options.labelProperty
      || options.labelPropertyId;
    const edgeTypeProperty = resolveCollectionItem(document, "properties", edgeTypePropertyRef, {
      required: options.edgeTypeProperty != null || options.crossTypeProperty != null,
      label: "mixed-record edge type property",
    });

    return new MixedRecordView({
      ...options,
      id: options.id || "mixed-records",
      space,
      records: space.records,
      recordIds: space.recordIds,
      positions: space.positions,
      targetPositions: space.targetPositions,
      datasetId: options.datasetId ?? space.datasetId,
      spaceId: options.spaceId ?? space.spaceId,
      coordinateId: space.coordinateId,
      sourceCoordinateId: space.sourceCoordinateId,
      targetCoordinateId: space.targetCoordinateId,
      propertyId: space.propertyId,
      relation,
      graph,
      graphId: graph?.id,
      relationId: relation?.id,
      colorValues: space.colorValues,
      scalarValues: space.scalarValues,
      sizeValues: space.sizeValues,
      edgeTypeValues: edgeTypeProperty
        ? extractPropertyValues(edgeTypeProperty, { records: space.records, recordIds: space.recordIds })
        : space.colorValues,
    });
  }

  toLayerDescriptors() {
    const descriptors = [];
    if (this.space?.showGround && typeof this.space.groundDescriptor === "function") {
      descriptors.push(this.withMixedRecordSource(this.space.groundDescriptor(), "stage"));
    }
    if (this.space?.showGroundProjection && typeof this.space.groundProjectionDescriptor === "function") {
      descriptors.push(this.withMixedRecordSource(this.space.groundProjectionDescriptor(), "ground-projection"));
    }
    if (this.space?.showLabels && typeof this.space.labelDescriptor === "function") {
      const labels = this.space.labelDescriptor();
      if (labels) descriptors.push(this.withMixedRecordSource(labels, "type-labels"));
    }
    descriptors.push(this.typedGlyphDescriptor());
    const edges = this.relationEdgeDescriptor();
    if (edges) descriptors.push(edges);
    return descriptors;
  }

  typedGlyphDescriptor() {
    const count = this.recordIds.length;
    const grammar = this.recordGlyphGrammar;
    const sizeData = this.sizeValues
      ? flattenValues(this.sizeValues, this.recordIds, this.size)
      : new Float32Array(count).fill(this.size);
    const alphaData = new Float32Array(count).fill(this.alpha);
    const scalarData = this.scalarValues
      ? flattenValues(this.scalarValues, this.recordIds, 0)
      : new Float32Array(count);
    const category = flattenCategories(this.colorValues || grammar.channels.glyphFamily.array, this.recordIds);
    const channels = {
      recordId: createStringChannel(this.recordIds, "record-id"),
      position: createChannel(flattenVectors(this.positions, this.recordIds, 3), 3, "position"),
      color: colorChannelFrom({
        ids: this.recordIds,
        colorValues: this.colorValues || grammar.channels.glyphFamily.array,
        categoryValues: this.colorValues || grammar.channels.glyphFamily.array,
        scalarValues: this.scalarValues,
        alpha: this.alpha,
      }),
      size: createChannel(sizeData, 1, "typed-glyph-size", { units: "scene" }),
      alpha: createChannel(alphaData, 1, "alpha"),
      scalar: createChannel(scalarData, 1, "scalar"),
      category: createChannel(category.data, 1, "glyph-category", { categories: category.categories }),
      ...grammar.channels,
      labelAnchor: this.createGlyphLabelAnchorChannel(this.positions),
    };
    const animation = this.targetPositions
      ? {
        mode: "coordinate-morph",
        channel: "targetPosition",
        requiresChannels: ["position", "targetPosition"],
        durationMs: this.space?.durationMs,
        easing: "smoothstep",
        loop: this.space?.morphLoop,
        direction: "alternate",
      }
      : { mode: "none" };
    if (this.targetPositions) {
      channels.targetPosition = createChannel(flattenVectors(this.targetPositions, this.recordIds, 3), 3, "target-position");
      channels.targetLabelAnchor = this.createGlyphLabelAnchorChannel(this.targetPositions);
      if (this.space?.morphLoop === false && Number.isFinite(Number(this.space?.morphProgress))) {
        animation.progress = this.space.morphProgress;
      }
    }

    return new VisualLayer({
      id: `${this.id}:typed-glyphs`,
      kind: "typed-glyph-scene",
      primitive: "InstancedGlyphLayer",
      order: 8,
      source: descriptorSource(this, {
        glyphBy: this.glyphBy,
        sourceCoordinateId: this.space?.sourceCoordinateId,
        targetCoordinateId: this.space?.targetCoordinateId,
      }),
      channels,
      geometry: {
        grammar: RECORD_GLYPH_GRAMMAR_SCHEMA,
        glyphFamilies: grammar.families,
        payloadKinds: grammar.payloadKinds,
        labelAnchors: true,
        instanceCount: count,
        semantic: "typed-record-glyphs",
      },
      material: this.material,
      animation,
      picking: {
        mode: "record-id",
        channel: "recordId",
      },
      metadata: {
        ...this.metadata,
        role: "typed-glyphs",
        primaryGrammar: "typed-record-glyphs",
        glyphBy: this.glyphBy,
        recordCount: count,
        recordGlyphGrammar: {
          schema: grammar.schema,
          families: grammar.families,
          payloadKinds: grammar.payloadKinds,
          recordCount: grammar.recordCount,
          labelAnchors: true,
        },
        typedGlyphSemantics: {
          recordIdentity: "recordId",
          glyphType: "glyphType",
          glyphFamily: "glyphFamily",
          recordType: "recordType",
          payloadKind: "payloadKind",
          payloadFeatures: "glyphFeature",
          payloadComplexity: "payloadComplexity",
          label: "labelText",
        },
      },
    }).toDescriptor();
  }

  relationEdgeDescriptor() {
    if (!this.showRelationEdges || !this.relation) return null;
    const graph = this.crossTypeRelations ? this.crossTypeGraph(this.graph) : this.graph;
    const relation = graph ? this.relation : this.crossTypeRelation(this.relation);
    const descriptor = createRelationGraphEdgeLayerDescriptor(relation, {
      id: `${this.id}:cross-type-relations`,
      recordIds: this.recordIds,
      positions: this.positions,
      relation: this.relation,
      relationId: this.relationId || this.relation?.id,
      graph,
      graphId: graph?.id || this.graph?.id,
      relationKind: this.relationKind,
      mode: this.mode,
      topK: this.topK,
      directed: this.directed,
      order: this.edgeOrder,
      width: this.edgeWidth,
      alpha: this.edgeAlpha,
    });
    return {
      ...descriptor,
      kind: "cross-type-relation-edges",
      source: {
        ...descriptor.source,
        viewId: this.id,
        viewKind: this.kind,
        coordinateId: this.coordinateId,
        edgeTypeProperty: this.edgeTypeProperty,
      },
      geometry: {
        ...descriptor.geometry,
        width: this.edgeWidth,
      },
      metadata: {
        ...descriptor.metadata,
        role: "cross-type-relation-edges",
        primaryGrammar: "cross-type-relation-edges",
        edgeTypeProperty: this.edgeTypeProperty,
        crossTypeRelations: this.crossTypeRelations,
      },
    };
  }

  crossTypeGraph(graph) {
    if (!graph || !Array.isArray(graph.edges) || !this.crossTypeRelations) return graph;
    const edges = graph.edges.filter((edge, index) => this.isCrossTypeEdge(edge, index));
    if (!edges.length) return graph;
    return {
      ...graph,
      id: graph.id ? `${graph.id}:cross-type` : `${this.id}:cross-type-graph`,
      edges,
      metadata: {
        ...(graph.metadata || {}),
        filtered_by: "cross-type-record-semantics",
        source_graph_id: graph.id || null,
        source_edge_count: graph.edges.length,
        edge_type_property: this.edgeTypeProperty,
      },
    };
  }

  crossTypeRelation(relation) {
    if (!relation || !Array.isArray(relation.values) || !this.crossTypeRelations) return relation;
    const values = relation.values.filter((edge, index) => this.isCrossTypeEdge(edge, index));
    if (!values.length) return relation;
    return {
      ...relation,
      id: relation.id,
      values,
      metadata: {
        ...(relation.metadata || {}),
        filtered_by: "cross-type-record-semantics",
        source_pair_count: relation.values.length,
        edge_type_property: this.edgeTypeProperty,
      },
    };
  }

  isCrossTypeEdge(edge, index) {
    const sourceId = endpointId(edge, "source") ?? this.recordIds[Number(edge?.row ?? edge?.i) || 0];
    const targetId = endpointId(edge, "target") ?? this.recordIds[Number(edge?.column ?? edge?.j) || 0];
    if (sourceId == null || targetId == null || String(sourceId) === String(targetId)) return false;
    const sourceType = this.edgeTypeForRecordId(sourceId, index);
    const targetType = this.edgeTypeForRecordId(targetId, index);
    return sourceType !== "" && targetType !== "" && sourceType !== targetType;
  }

  edgeTypeForRecordId(id) {
    const recordIndex = this.recordIds.findIndex((entry) => String(entry) === String(id));
    const explicit = recordIndex >= 0 ? valueForId(this.edgeTypeValues, id, recordIndex) : undefined;
    if (explicit != null && explicit !== "") return String(explicit);
    const family = recordIndex >= 0 ? this.recordGlyphGrammar.channels.glyphFamily.array[recordIndex] : null;
    if (family != null && family !== "") return String(family);
    const record = this.records.find((entry) => String(entry?.id ?? entry?.record_id ?? entry?.recordId) === String(id));
    return String(record?.record_type ?? record?.recordType ?? record?.type ?? "");
  }

  createGlyphLabelAnchorChannel(positions) {
    const anchors = flattenVectors(positions, this.recordIds, 3);
    for (let index = 0; index < this.recordIds.length; index += 1) {
      anchors[index * 3 + 1] += this.glyphLabelLift;
    }
    return createChannel(anchors, 3, "record-label-anchor", {
      grammar: RECORD_GLYPH_GRAMMAR_SCHEMA,
    });
  }

  withMixedRecordSource(descriptor, role) {
    return {
      ...descriptor,
      source: {
        ...(descriptor.source || {}),
        viewId: this.id,
        viewKind: this.kind,
        coordinateId: this.coordinateId,
        relationId: this.relationId,
      },
      metadata: {
        ...(descriptor.metadata || {}),
        role,
        semanticView: "MixedRecordView",
      },
    };
  }
}

export function createMixedRecordView(options) {
  return new MixedRecordView(options);
}

function firstRelation(document) {
  const relations = Array.isArray(document?.relations) ? document.relations : [];
  return relations[0] || null;
}

function firstGraphForRelation(document, relationId) {
  const graphs = Array.isArray(document?.graphs) ? document.graphs : [];
  if (relationId != null) {
    const match = graphs.find((graph) => String(graph.edge_relation_id ?? graph.edgeRelationId) === String(relationId));
    if (match) return match;
  }
  return graphs[0] || null;
}

function endpointId(edge, side) {
  if (!edge || typeof edge !== "object") return null;
  if (side === "source") {
    return edge.source ?? edge.source_id ?? edge.sourceId ?? edge.row_id ?? edge.rowId ?? edge.a ?? edge.i ?? null;
  }
  return edge.target ?? edge.target_id ?? edge.targetId ?? edge.column_id ?? edge.columnId ?? edge.b ?? edge.j ?? null;
}
