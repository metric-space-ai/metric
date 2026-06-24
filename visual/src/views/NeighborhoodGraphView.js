import { BaseView } from "./BaseView.js";
import { PointCloudView } from "./PointCloudView.js";
import { createRelationGraphEdgeLayerDescriptor } from "../relational/descriptors.js";
import { applyPositionFit, computePositionFit } from "./scene-fit.js";
import {
  extractCoordinatePositions,
  extractPropertyValues,
  inferRecordIds,
  recordsFor,
  resolveCollectionItem,
} from "./view-utils.js";
import { defaultCoordinateId } from "./MetricSpaceView.js";

/**
 * NeighborhoodGraphView renders a sparse nearest-neighbor graph over the metric
 * space: nodes placed at the exported coordinate state and edges selected from
 * the exported relation values (top-k / threshold / quantile). It selects
 * existing pair evidence; it does not compute neighbors or shortest paths.
 */
export class NeighborhoodGraphView extends BaseView {
  constructor(options = {}) {
    super({ ...options, kind: "neighborhood-graph" });
    this.recordIds = inferRecordIds(options);
    this.relation = options.relation || null;
    this.graph = options.graph || null;
    this.relationKind = options.relationKind || "distance";
    this.mode = options.mode || "topK";
    this.topK = Number.isFinite(Number(options.topK)) ? Number(options.topK) : 6;
    this.directed = options.directed == null ? undefined : Boolean(options.directed);
    this.fitEnabled = options.fit !== false;
    this.groundY = Number.isFinite(Number(options.groundY)) ? Number(options.groundY) : 0;
    this.targetRadius = Number.isFinite(Number(options.targetRadius)) ? Number(options.targetRadius) : 1.6;
    this.showNodes = options.nodes !== false;
    this.size = Number.isFinite(Number(options.size)) ? Number(options.size) : 1;
    this.colors = options.colors;
    this.colorValues = options.colorValues;
    this.scalarValues = options.scalarValues;

    const rawPositions = options.positions || new Map();
    this.fit = this.fitEnabled
      ? computePositionFit(rawPositions, this.recordIds, { targetRadius: this.targetRadius, groundY: this.groundY })
      : null;
    this.positions = this.fit ? applyPositionFit(rawPositions, this.recordIds, this.fit.transform) : rawPositions;
  }

  static fromVisualSpace(document, options = {}) {
    const graphRef = options.graph || options.graphId;
    let graph = resolveCollectionItem(document, "graphs", graphRef, {
      required: graphRef != null,
      label: "graph",
    });
    const relationRef = options.relation || options.relationId || graph?.edge_relation_id;
    const relation = resolveCollectionItem(document, "relations", relationRef, {
      required: relationRef != null,
      label: "relation",
    })
      || firstRelation(document);
    if (!graph) graph = firstGraphForRelation(document, relation?.id);
    const spaceRef = options.space || options.spaceId || relation?.space_id;
    const space = resolveCollectionItem(document, "spaces", spaceRef, {
      required: (options.space || options.spaceId) != null,
      label: "space",
    });
    const explicitCoordinateRef = options.coordinate ?? options.coordinateId;
    const coordinateRef = options.coordinate
      ?? options.coordinateId
      ?? defaultCoordinateId(document, space, { dimension: 3 });
    const coordinate = resolveCollectionItem(document, "coordinates", coordinateRef, {
      required: coordinateRef != null,
      label: explicitCoordinateRef != null ? "coordinate" : "default coordinate",
    });
    const datasetId = options.datasetId ?? relation?.dataset_id ?? coordinate?.dataset_id ?? space?.dataset_id;
    const graphRecordIds = graph?.node_record_ids || graph?.nodeRecordIds;
    const relationRecordIds = relation?.record_ids || relation?.recordIds;
    const viewRecordIds = options.recordIds || relationRecordIds || graphRecordIds || space?.record_ids;
    const records = recordsFor(document, { ...options, datasetId });
    const positions = extractCoordinatePositions(coordinate, {
      records,
      recordIds: viewRecordIds,
    });
    const colorPropertyRef = options.colorProperty || options.colorPropertyId;
    const scalarPropertyRef = options.scalarProperty || options.scalarPropertyId;
    const colorProperty = resolveCollectionItem(document, "properties", colorPropertyRef, {
      required: colorPropertyRef != null,
      label: "color property",
    });
    const scalarProperty = resolveCollectionItem(document, "properties", scalarPropertyRef, {
      required: scalarPropertyRef != null,
      label: "scalar property",
    });

    return new NeighborhoodGraphView({
      ...options,
      records,
      recordIds: positions.ids,
      relation,
      graph,
      graphId: graph?.id,
      relationId: relation?.id,
      relationKind: options.relationKind || (relation?.relation_type === "similarity" ? "similarity" : "distance"),
      datasetId,
      spaceId: options.spaceId ?? space?.id,
      coordinateId: coordinate?.id,
      positions: positions.positions,
      colorValues: colorProperty ? extractPropertyValues(colorProperty, { records, recordIds: positions.ids }) : options.colorValues,
      scalarValues: scalarProperty ? extractPropertyValues(scalarProperty, { records, recordIds: positions.ids }) : options.scalarValues,
    });
  }

  relationSource() {
    if (!this.relation) return { record_ids: this.recordIds, values: [] };
    return this.relation;
  }

  edgeOptions() {
    return {
      id: `${this.id}:edges`,
      recordIds: this.recordIds,
      positions: this.positions,
      mode: this.mode,
      topK: this.topK,
      directed: this.directed,
      relationKind: this.relationKind,
      graph: this.graph,
      graphId: this.graph?.id,
      relationId: this.relationId || this.relation?.id,
      order: 10,
    };
  }

  toLayerDescriptors() {
    const descriptors = [];
    if (this.showNodes) {
      const point = new PointCloudView({
        id: `${this.id}:nodes`,
        datasetId: this.datasetId,
        spaceId: this.spaceId,
        coordinateId: this.coordinateId,
        records: this.records,
        recordIds: this.recordIds,
        positions: this.positions,
        colors: this.colors,
        colorValues: this.colorValues,
        scalarValues: this.scalarValues,
        size: this.size,
        shape: "sphere",
        metadata: this.metadata,
      });
      descriptors.push(...point.toLayerDescriptors());
    }
    const edges = createRelationGraphEdgeLayerDescriptor(this.relationSource(), this.edgeOptions());
    edges.source = {
      ...edges.source,
      viewId: this.id,
      viewKind: this.kind,
      relationId: this.relationId,
      graphId: this.graph?.id || edges.source?.graphId || null,
    };
    descriptors.push(edges);
    return descriptors;
  }
}

export function createNeighborhoodGraphView(options) {
  return new NeighborhoodGraphView(options);
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
