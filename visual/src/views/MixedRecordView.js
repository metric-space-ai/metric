import { BaseView } from "./BaseView.js";
import { MetricSpaceView } from "./MetricSpaceView.js";
import { VisualLayer } from "./VisualLayer.js";
import { createRelationGraphEdgeLayerDescriptor } from "../relational/descriptors.js";
import {
  RECORD_GLYPH_GRAMMAR_SCHEMA,
  createRecordGlyphGrammar,
} from "../glyphs/index.js";
import {
  createChannel,
  createStringChannel,
  descriptorSource,
  extractPropertyValues,
  flattenCategories,
  flattenValues,
  flattenVectors,
  parseColor,
  resolveCollectionItem,
  valueForId,
} from "./view-utils.js";

const MIXED_RECORD_TYPE_PALETTE = Object.freeze([
  "#2f86b7",
  "#d97924",
  "#6f58b5",
  "#169f8b",
  "#cf4a57",
  "#7d9342",
]);

const MIXED_RECORD_TYPE_LABELS = Object.freeze({
  text_code_record: "Text code",
  histogram_spectrum_record: "Histogram spectrum",
  process_curve_record: "Process curve",
  numeric_vitals_record: "Numeric vitals",
});

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

const MIXED_RECORD_GROUND_PROJECTION_MATERIAL = Object.freeze({
  lighting: "projection",
  diffuse: "mixed-record-type-shadow",
  colorMix: 0.92,
  shadowDensity: 0.72,
  coreDensity: 0.2,
  softness: 0.68,
  shadowTail: 0.52,
  shadowTailStrength: 0.22,
  edgeTint: 0.38,
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
    this.glyphBy = options.glyphBy || options.colorProperty || options.colorPropertyId || "record_type";
    this.edgeTypeProperty = options.edgeTypeProperty || options.crossTypeProperty || this.glyphBy;
    this.size = numberOption(options.size, options.pointSize, 1.15);
    this.alpha = Number.isFinite(Number(options.alpha)) ? Number(options.alpha) : 0.96;
    this.glyphLabelLift = Number.isFinite(Number(options.glyphLabelLift)) ? Number(options.glyphLabelLift) : 0.18;
    this.material = { ...MIXED_RECORD_GLYPH_MATERIAL, ...(options.material || options.glyphMaterial || {}) };
    this.recordGlyphGrammar = options.recordGlyphGrammar || createRecordGlyphGrammar(this.records, this.recordIds);
    this.recordTypeValues = options.recordTypeValues || createRecordTypeValueMap(this.records, this.recordIds, this.recordGlyphGrammar);
    this.typePalette = normalizePalette(options.typePalette || options.palette || MIXED_RECORD_TYPE_PALETTE);
    this.typeLabelMap = options.typeLabelMap || options.labelMap || MIXED_RECORD_TYPE_LABELS;
    this.glyphColorValues = options.glyphColorValues || this.colorValues || this.recordTypeValues || this.recordGlyphGrammar.channels.glyphFamily.array;
    this.projectionColorValues = options.projectionColorValues || this.glyphColorValues;
    this.edgeTypeValues = options.edgeTypeValues || this.recordTypeValues || this.colorValues || null;
    this.groundProjectionSize = numberOption(options.groundProjectionSize, this.size * 0.62, 0.72);

    this.relation = options.relation || null;
    this.graph = options.graph || null;
    this.showRelationEdges = options.relationEdges !== false && options.edges !== false;
    this.crossTypeRelations = options.crossTypeRelations !== false;
    this.relationKind = options.relationKind || (this.relation?.relation_type === "similarity" ? "similarity" : "distance");
    this.mode = options.mode || "topK";
    this.topK = Number.isFinite(Number(options.topK)) ? Number(options.topK) : 4;
    this.directed = options.directed == null ? undefined : Boolean(options.directed);
    this.edgeAlpha = Number.isFinite(Number(options.edgeAlpha)) ? Number(options.edgeAlpha) : 0.26;
    this.edgeWidth = Number.isFinite(Number(options.edgeWidth)) ? Number(options.edgeWidth) : 0.78;
    this.edgeOrder = Number.isFinite(Number(options.edgeOrder)) ? Number(options.edgeOrder) : 12;
  }

  static fromVisualSpace(document, options = {}) {
    const recordTypeProperty = resolveMixedRecordTypeProperty(document, options);
    const viewOptions = normalizeMixedRecordSpaceOptions(options, recordTypeProperty);
    const space = MetricSpaceView.fromVisualSpace(document, {
      ...viewOptions,
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
    const edgeTypePropertyRef = viewOptions.edgeTypeProperty
      || viewOptions.crossTypeProperty
      || viewOptions.glyphBy
      || viewOptions.colorProperty
      || viewOptions.colorPropertyId
      || viewOptions.recordType
      || viewOptions.labelProperty
      || viewOptions.labelPropertyId
      || recordTypeProperty?.id;
    const edgeTypeProperty = resolveCollectionItem(document, "properties", edgeTypePropertyRef, {
      required: viewOptions.edgeTypeProperty != null || viewOptions.crossTypeProperty != null,
      label: "mixed-record edge type property",
    });

    return new MixedRecordView({
      ...viewOptions,
      id: viewOptions.id || "mixed-records",
      space,
      records: space.records,
      recordIds: space.recordIds,
      positions: space.positions,
      targetPositions: space.targetPositions,
      datasetId: viewOptions.datasetId ?? space.datasetId,
      spaceId: viewOptions.spaceId ?? space.spaceId,
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
      recordTypeValues: recordTypeProperty
        ? extractPropertyValues(recordTypeProperty, { records: space.records, recordIds: space.recordIds })
        : undefined,
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
      descriptors.push(this.withMixedRecordSource(this.mixedGroundProjectionDescriptor(this.space.groundProjectionDescriptor()), "ground-projection"));
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
    const colorValues = this.glyphColorValues || grammar.channels.glyphFamily.array;
    const category = flattenCategories(colorValues, this.recordIds);
    const channels = {
      recordId: createStringChannel(this.recordIds, "record-id"),
      position: createChannel(flattenVectors(this.positions, this.recordIds, 3), 3, "position"),
      color: createMixedRecordColorChannel(colorValues, this.recordIds, this.typePalette, this.alpha, this.typeLabelMap),
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
        typeColorEncoding: categoryColorMetadata(category, this.typePalette, this.typeLabelMap),
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
    const edgeVisualChannels = this.createRelationEdgeVisualChannels(descriptor.metadata?.graph || graph);
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
      channels: {
        ...descriptor.channels,
        ...edgeVisualChannels.channels,
      },
      geometry: {
        ...descriptor.geometry,
        width: this.edgeWidth,
        semantic: "native-cross-type-relation-structure",
      },
      material: {
        ...descriptor.material,
        alpha: this.edgeAlpha,
        emphasisStrength: 0.34,
      },
      metadata: {
        ...descriptor.metadata,
        role: "cross-type-relation-edges",
        primaryGrammar: "cross-type-relation-edges",
        edgeTypeProperty: this.edgeTypeProperty,
        crossTypeRelations: this.crossTypeRelations,
        nativeEvidence: {
          relationId: this.relationId || this.relation?.id || null,
          graphId: graph?.id || this.graph?.id || null,
          sourceGraphId: this.graph?.id || null,
          sourceEdgeCount: Array.isArray(this.graph?.edges) ? this.graph.edges.length : null,
          relationValueSource: "native-relation-or-native-graph-edge-value",
        },
        relationStructureEncoding: edgeVisualChannels.metadata,
      },
    };
  }

  mixedGroundProjectionDescriptor(descriptor) {
    const ids = this.recordIds;
    const category = flattenCategories(this.projectionColorValues || this.glyphColorValues, ids);
    return {
      ...descriptor,
      channels: {
        ...(descriptor.channels || {}),
        color: createMixedRecordColorChannel(this.projectionColorValues || this.glyphColorValues, ids, this.typePalette, this.space?.groundProjectionAlpha ?? 0.42, this.typeLabelMap),
        size: createChannel(new Float32Array(ids.length).fill(this.groundProjectionSize), 1, "projection-size", { units: "scene" }),
        category: createChannel(category.data, 1, "record-type-projection-category", { categories: category.categories }),
      },
      geometry: {
        ...(descriptor.geometry || {}),
        size: this.groundProjectionSize,
        semantic: "typed-record-ground-projection",
      },
      material: {
        ...MIXED_RECORD_GROUND_PROJECTION_MATERIAL,
        ...(descriptor.material || {}),
      },
      metadata: {
        ...(descriptor.metadata || {}),
        role: "ground-projection",
        primaryGrammar: "typed-record-ground-projection",
        typeColorEncoding: categoryColorMetadata(category, this.typePalette, this.typeLabelMap),
      },
    };
  }

  createRelationEdgeVisualChannels(graph) {
    const edges = Array.isArray(graph?.edges) ? graph.edges : [];
    const colorData = new Float32Array(edges.length * 4);
    const emphasis = new Float32Array(edges.length);
    const typePairs = new Array(edges.length);
    const valueDomain = finiteDomain(edges.map((edge) => Number(edge?.value)));
    const typeColorLookup = categoryColorLookup(this.edgeTypeValues || this.recordTypeValues || this.glyphColorValues, this.recordIds, this.typePalette);

    for (let index = 0; index < edges.length; index += 1) {
      const edge = edges[index] || {};
      const sourceId = endpointId(edge, "source");
      const targetId = endpointId(edge, "target");
      const sourceType = this.edgeTypeForRecordId(sourceId, index);
      const targetType = this.edgeTypeForRecordId(targetId, index);
      const sourceColor = this.colorForType(sourceType, typeColorLookup);
      const targetColor = this.colorForType(targetType, typeColorLookup);
      const edgeColor = mixRgba(sourceColor, targetColor, 0.46);
      colorData[index * 4] = edgeColor[0] * 0.84;
      colorData[index * 4 + 1] = edgeColor[1] * 0.84;
      colorData[index * 4 + 2] = edgeColor[2] * 0.84;
      colorData[index * 4 + 3] = this.edgeAlpha;
      emphasis[index] = relationValueStrength(edge?.value, valueDomain, this.relationKind);
      typePairs[index] = `${sourceType || "unknown"} -> ${targetType || "unknown"}`;
    }

    const uniqueTypePairs = Array.from(new Set(typePairs)).sort();
    return {
      channels: {
        color: createChannel(colorData, 4, "rgba", {
          colorSpace: "srgb",
          source: "mixed-record-native-edge-type-pair",
        }),
        edgeEmphasis: createChannel(emphasis, 1, "native-relation-value-emphasis", {
          relationKind: this.relationKind,
          domain: valueDomain,
        }),
        edgeTypePair: createStringChannel(typePairs, "record-type-pair"),
      },
      metadata: {
        edgeCount: edges.length,
        typePairCount: uniqueTypePairs.length,
        typePairs: uniqueTypePairs,
        color: "source-target-record-type-blend",
        emphasis: "native-relation-value-rank",
      },
    };
  }

  colorForType(value, lookup = null) {
    const colorLookup = lookup || categoryColorLookup(this.edgeTypeValues || this.recordTypeValues || this.glyphColorValues, this.recordIds, this.typePalette);
    const key = value == null ? "" : String(value);
    return colorLookup.get(key) || this.typePalette[0] || [0.25, 0.34, 0.42, 1];
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
        ...(role === "type-labels"
          ? {
            labelAnchorMode: "mixed-record-type-centroids",
            typeLabelMap: this.typeLabelMap,
          }
          : {}),
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

function normalizeMixedRecordSpaceOptions(options, recordTypeProperty) {
  const typePropertyId = recordTypeProperty?.id || "record_type";
  const next = {
    ...options,
    labelMap: options.labelMap || MIXED_RECORD_TYPE_LABELS,
    labelFontSize: numberOption(options.labelFontSize, 22),
    labelLift: numberOption(options.labelLift, 0.52),
    labelOffsetRadius: numberOption(options.labelOffsetRadius, 0.56),
    groundProjectionAlpha: numberOption(options.groundProjectionAlpha, 0.36),
    groundProjectionMaterial: {
      ...MIXED_RECORD_GROUND_PROJECTION_MATERIAL,
      ...(options.groundProjectionMaterial || {}),
    },
  };
  if (next.pointSize !== undefined && next.size === undefined) next.size = next.pointSize;
  if (next.labels && next.labels !== true && !next.labelProperty && !next.labelPropertyId) {
    next.labelProperty = next.labels;
  }
  if (!next.glyphBy && !next.colorProperty && !next.colorPropertyId) {
    next.glyphBy = typePropertyId;
    next.colorProperty = typePropertyId;
  }
  if (next.labels === true && !next.labelProperty && !next.labelPropertyId) {
    next.labelProperty = typePropertyId;
  }
  if (next.labels == null && !next.labelProperty && !next.labelPropertyId) {
    next.labelProperty = typePropertyId;
  }
  return next;
}

function resolveMixedRecordTypeProperty(document, options = {}) {
  for (const ref of [
    options.recordTypeProperty,
    options.recordTypePropertyId,
    "record_type",
    "type",
    options.crossTypeProperty,
    options.edgeTypeProperty,
    options.glyphBy,
  ]) {
    const property = resolveCollectionItem(document, "properties", ref, {
      required: false,
      label: "mixed-record type property",
    });
    if (property) return property;
  }
  return null;
}

function createRecordTypeValueMap(records, recordIds, grammar) {
  const out = new Map();
  for (let index = 0; index < recordIds.length; index += 1) {
    const id = recordIds[index];
    const record = records.find((entry) => String(entry?.id ?? entry?.record_id ?? entry?.recordId) === String(id));
    const value = record?.record_type
      ?? record?.recordType
      ?? record?.type
      ?? grammar?.channels?.recordType?.array?.[index]
      ?? grammar?.channels?.glyphFamily?.array?.[index]
      ?? "";
    out.set(String(id), String(value));
  }
  return out;
}

function createMixedRecordColorChannel(values, ids, palette, alpha, labelMap = MIXED_RECORD_TYPE_LABELS) {
  const category = flattenCategories(values, ids);
  const colors = normalizePalette(palette);
  const data = new Float32Array(ids.length * 4);
  for (let index = 0; index < ids.length; index += 1) {
    const color = colors[Math.round(category.data[index]) % colors.length] || colors[0];
    data[index * 4] = color[0];
    data[index * 4 + 1] = color[1];
    data[index * 4 + 2] = color[2];
    data[index * 4 + 3] = clamp01(alpha);
  }
  return createChannel(data, 4, "rgba", {
    colorSpace: "srgb",
    source: "mixed-record-category-order-palette",
    categories: categoryColorMetadata(category, colors, labelMap),
  });
}

function categoryColorMetadata(category, palette, labelMap = {}) {
  const colors = normalizePalette(palette);
  return (category.categories || []).map((entry) => {
    const color = colors[entry.index % colors.length] || colors[0];
    return {
      ...entry,
      label: labelMap[entry.id] || entry.label || entry.id,
      color,
    };
  });
}

function categoryColorLookup(values, ids, palette) {
  const category = flattenCategories(values, ids);
  const colors = normalizePalette(palette);
  const out = new Map();
  for (const entry of category.categories || []) {
    out.set(String(entry.id), colors[entry.index % colors.length] || colors[0]);
  }
  return out;
}

function normalizePalette(palette) {
  const colors = (Array.isArray(palette) ? palette : MIXED_RECORD_TYPE_PALETTE)
    .map((entry) => parseColor(entry, [0.25, 0.34, 0.42, 1]));
  return colors.length ? colors : [[0.25, 0.34, 0.42, 1]];
}

function finiteDomain(values) {
  let min = Infinity;
  let max = -Infinity;
  for (const value of values) {
    if (!Number.isFinite(value)) continue;
    min = Math.min(min, value);
    max = Math.max(max, value);
  }
  if (!Number.isFinite(min) || !Number.isFinite(max)) return { min: 0, max: 1 };
  return { min, max };
}

function relationValueStrength(value, domain, relationKind) {
  const number = Number(value);
  if (!Number.isFinite(number) || domain.max === domain.min) return 0.18;
  const t = clamp01((number - domain.min) / (domain.max - domain.min));
  const near = relationKind === "similarity" ? t : 1 - t;
  return 0.08 + near * 0.34;
}

function mixRgba(a, b, amount = 0.5) {
  const t = clamp01(amount);
  return [
    a[0] + (b[0] - a[0]) * t,
    a[1] + (b[1] - a[1]) * t,
    a[2] + (b[2] - a[2]) * t,
    a[3] + (b[3] - a[3]) * t,
  ];
}

function numberOption(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number)) return number;
  }
  return 0;
}

function clamp01(value) {
  return Math.max(0, Math.min(1, Number.isFinite(Number(value)) ? Number(value) : 0));
}

function endpointId(edge, side) {
  if (!edge || typeof edge !== "object") return null;
  if (side === "source") {
    return edge.source ?? edge.source_id ?? edge.sourceId ?? edge.row_id ?? edge.rowId ?? edge.a ?? edge.i ?? null;
  }
  return edge.target ?? edge.target_id ?? edge.targetId ?? edge.column_id ?? edge.columnId ?? edge.b ?? edge.j ?? null;
}
