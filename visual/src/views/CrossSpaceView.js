import { BaseView } from "./BaseView.js";
import { MetricSpaceView } from "./MetricSpaceView.js";
import { VisualLayer } from "./VisualLayer.js";
import {
  createChannel,
  createStringChannel,
  resolveCollectionItem,
} from "./view-utils.js";

const PAIRED_SPACE_VIEW_SCHEMA = "metric.visual.paired_space_view.v1";

/**
 * CrossSpaceView is the paired-space semantic grammar: two exported metric
 * spaces rendered side by side, linked by explicit paired-record evidence.
 *
 * It does not compute dependence, MGC, distances, pairings or coordinates. It
 * only reads exported record ids, coordinate positions, properties and native
 * dependence diagnostics, then exposes a descriptor contract that runtime
 * picking can interpret as paired evidence instead of unrelated point clouds.
 */
export class CrossSpaceView extends BaseView {
  constructor(options = {}) {
    super({ ...options, kind: "paired-space" });
    this.contractSchema = PAIRED_SPACE_VIEW_SCHEMA;
    this.left = options.left || null;
    this.right = options.right || null;
    this.leftRole = options.leftRole || "source-space";
    this.rightRole = options.rightRole || "target-space";
    this.leftOffset = vector3(options.leftOffset, [-1.18, 0, 0]);
    this.rightOffset = vector3(options.rightOffset, [1.18, 0, 0]);
    this.groundY = finiteNumber(options.groundY, -0.58);
    this.ground = options.ground !== false;
    this.pairSetId = options.pairSetId || `${this.id}:linked-pairs`;
    this.dependencePropertyId = options.dependencePropertyId || options.scalarProperty || null;
    this.bridgeRelationId = options.bridgeRelationId || options.bridgeRelation || null;
    this.bridgeRelationName = options.bridgeRelationName || null;
    this.bridgeRelationType = options.bridgeRelationType || null;
    this.bridgeGraphId = options.bridgeGraphId || options.graphId || this.pairSetId;
    this.globalDependence = options.globalDependence || null;
    this.pairRecords = normalizePairRecords(options.pairRecords || options.pairs, this.left, this.right);
    if (!this.pairRecords.length) this.pairRecords = sharedRecordIdPairs(this.left, this.right);
    this.metadata = {
      ...this.metadata,
      primaryGrammar: "paired-space",
      contract: this.semanticContract(),
    };
    this.applyPairMetadata();
  }

  static fromVisualSpace(document, options = {}) {
    const pairedView = findCrossSpaceViewDeclaration(document, options.viewId || options.view);
    const leftCoordinateId = options.leftCoordinateId
      || options.coordinateA
      || options.coordinate_a
      || options.sourceCoordinateId
      || pairedView?.leftCoordinateId
      || pairedView?.left_coordinate_id
      || "space-a-3d";
    const rightCoordinateId = options.rightCoordinateId
      || options.coordinateB
      || options.coordinate_b
      || options.targetCoordinateId
      || pairedView?.rightCoordinateId
      || pairedView?.right_coordinate_id
      || "space-b-3d";
    const scalarProperty = options.scalarProperty
      || options.dependenceProperty
      || options.dependencePropertyId
      || pairedView?.propertyId
      || pairedView?.property_id
      || "local-dependence";

    const leftCoordinate = resolveCollectionItem(document, "coordinates", leftCoordinateId, {
      required: leftCoordinateId != null,
      label: "left cross-space coordinate",
    });
    const rightCoordinate = resolveCollectionItem(document, "coordinates", rightCoordinateId, {
      required: rightCoordinateId != null,
      label: "right cross-space coordinate",
    });
    const leftSpaceId = options.leftSpaceId || options.spaceA || pairedView?.leftSpaceId || pairedView?.left_space_id || leftCoordinate?.space_id || leftCoordinate?.spaceId;
    const rightSpaceId = options.rightSpaceId || options.spaceB || pairedView?.rightSpaceId || pairedView?.right_space_id || rightCoordinate?.space_id || rightCoordinate?.spaceId;
    const bridgeRelationId = options.bridgeRelationId
      || options.bridgeRelation
      || pairedView?.bridgeRelationId
      || pairedView?.bridge_relation_id
      || null;
    const bridgeGraphId = options.bridgeGraphId
      || options.bridgeGraph
      || pairedView?.bridgeGraphId
      || pairedView?.bridge_graph_id
      || null;
    const bridgeRelation = bridgeRelationId
      ? resolveCollectionItem(document, "relations", bridgeRelationId, {
        required: false,
        label: "cross-space bridge relation",
      })
      : null;

    const left = MetricSpaceView.fromVisualSpace(document, {
      ...options,
      id: options.leftViewId || `${options.id || "cross-space"}:left`,
      name: options.leftName || "Space A",
      spaceId: leftSpaceId,
      coordinateId: leftCoordinateId,
      scalarProperty,
      ground: false,
      groundProjection: false,
      labels: false,
      targetRadius: options.targetRadius ?? 1.05,
      metadata: {
        ...(options.leftMetadata || {}),
        role: "paired-space-side",
        pairedSpaceRole: options.leftRole || "source-space",
      },
    });
    const right = MetricSpaceView.fromVisualSpace(document, {
      ...options,
      id: options.rightViewId || `${options.id || "cross-space"}:right`,
      name: options.rightName || "Space B",
      spaceId: rightSpaceId,
      coordinateId: rightCoordinateId,
      scalarProperty,
      ground: false,
      groundProjection: false,
      labels: false,
      targetRadius: options.targetRadius ?? 1.05,
      metadata: {
        ...(options.rightMetadata || {}),
        role: "paired-space-side",
        pairedSpaceRole: options.rightRole || "target-space",
      },
    });

    left.positions = translatePositionMap(left.positions, left.recordIds, vector3(options.leftOffset, [-1.18, 0, 0]));
    right.positions = translatePositionMap(right.positions, right.recordIds, vector3(options.rightOffset, [1.18, 0, 0]));

    return new CrossSpaceView({
      ...options,
      id: options.id || "cross-space",
      name: options.name || "Cross-space dependence",
      datasetId: options.datasetId || left.datasetId || right.datasetId,
      left,
      right,
      dependencePropertyId: scalarProperty,
      bridgeRelationId,
      bridgeGraphId,
      bridgeRelationName: bridgeRelation?.name || null,
      bridgeRelationType: bridgeRelation?.relation_type || bridgeRelation?.relationType || null,
      globalDependence: options.globalDependence
        || findCrossSpaceDependenceDiagnostic(document, left.spaceId, right.spaceId, pairedView?.diagnosticId || pairedView?.diagnostic_id),
    });
  }

  semanticContract() {
    const pairCount = this.pairRecords.length;
    return {
      schema: this.contractSchema,
      pairSetId: this.pairSetId,
      pairCount,
      pairModel: "exported-paired-record-ids",
      primaryGrammar: "paired-space",
      linkedSelection: {
        kind: "paired-space-linked-selection",
        respondsTo: ["record", "pair"],
        recordSelectionInterpretation: "record id identifies one paired observation across both spaces",
        pairSelectionInterpretation: "rowId/sourceId is the left-space record and columnId/targetId is the right-space paired record",
        pairIdChannel: "recordId",
        sourceRecordChannel: "sourceId",
        targetRecordChannel: "targetId",
      },
      spaces: {
        left: this.spaceContract(this.left, this.leftRole),
        right: this.spaceContract(this.right, this.rightRole),
      },
      dependence: {
        propertyId: this.dependencePropertyId,
        global: this.globalDependence,
        bridgeRelationId: this.bridgeRelationId,
        bridgeGraphId: this.bridgeGraphId,
      },
    };
  }

  spaceContract(space, role) {
    return {
      role,
      viewId: space?.id || null,
      spaceId: space?.spaceId || null,
      coordinateId: space?.coordinateId || null,
      recordCount: Array.isArray(space?.recordIds) ? space.recordIds.length : 0,
    };
  }

  applyPairMetadata() {
    const contract = this.semanticContract();
    decoratePairedSpace(this.left, {
      contract,
      pairSetId: this.pairSetId,
      role: this.leftRole,
      pairedRole: this.rightRole,
      pairedSpaceId: this.right?.spaceId || null,
      pairedCoordinateId: this.right?.coordinateId || null,
    });
    decoratePairedSpace(this.right, {
      contract,
      pairSetId: this.pairSetId,
      role: this.rightRole,
      pairedRole: this.leftRole,
      pairedSpaceId: this.left?.spaceId || null,
      pairedCoordinateId: this.left?.coordinateId || null,
    });
  }

  toViewDescriptor() {
    return {
      ...super.toViewDescriptor(),
      kind: this.kind,
      metadata: { ...this.metadata },
      pairedSpaces: {
        left: this.spaceContract(this.left, this.leftRole),
        right: this.spaceContract(this.right, this.rightRole),
      },
      pairSetId: this.pairSetId,
      pairCount: this.pairRecords.length,
    };
  }

  toLayerDescriptors() {
    const descriptors = [];
    if (this.ground) descriptors.push(this.sharedGroundDescriptor());
    descriptors.push(...this.sideDescriptors(this.left, this.leftRole));
    descriptors.push(...this.sideDescriptors(this.right, this.rightRole));
    const bridge = this.bridgeDescriptor();
    if (bridge) descriptors.push(bridge);
    return descriptors;
  }

  sideDescriptors(space, role) {
    if (!space) return [];
    return space.toLayerDescriptors().map((descriptor) => ({
      ...descriptor,
      source: {
        ...(descriptor.source || {}),
        parentViewId: this.id,
        parentViewKind: this.kind,
        pairedSpaceRole: role,
        pairSetId: this.pairSetId,
      },
      metadata: {
        ...(descriptor.metadata || {}),
        pairedSpace: this.spaceContract(space, role),
        pairedSpaceContract: this.semanticContract(),
        selectionModel: {
          ...(descriptor.metadata?.selectionModel || {}),
          kind: "paired-space-side-records",
          pairSetId: this.pairSetId,
          pairedSpaceRole: role,
          respondsTo: ["record"],
          linkedPairInterpretation: "selected record id addresses a paired observation in the companion space",
        },
      },
    }));
  }

  sharedGroundDescriptor() {
    return new VisualLayer({
      id: `${this.id}:ground`,
      kind: "ground-plane",
      primitive: "GroundPlaneLayer",
      order: -20,
      source: { viewId: this.id, viewKind: this.kind, role: "shared-paired-space-stage" },
      channels: {},
      geometry: {
        width: 5.2,
        depth: 3.2,
        y: this.groundY,
        gridScale: 8,
      },
      material: {
        alpha: 0.62,
        gridAlpha: 0.28,
        axisAlpha: 0.2,
      },
      metadata: {
        role: "shared-ground",
        pairedSpaceContract: this.semanticContract(),
      },
    }).toDescriptor();
  }

  bridgeDescriptor() {
    const pairs = this.validPairs();
    if (!pairs.length) return null;
    const sourcePosition = new Float32Array(pairs.length * 3);
    const targetPosition = new Float32Array(pairs.length * 3);
    const color = new Float32Array(pairs.length * 4);
    const values = new Float32Array(pairs.length);
    const edgeIds = [];
    const sourceIds = [];
    const targetIds = [];
    const pairIds = [];
    const graphEdges = [];

    for (let index = 0; index < pairs.length; index += 1) {
      const pair = pairs[index];
      const source = getPosition(this.left.positions, pair.sourceRecordId);
      const target = getPosition(this.right.positions, pair.targetRecordId);
      const value = dependenceValue(this.left, this.right, pair);
      sourcePosition.set(source, index * 3);
      targetPosition.set(target, index * 3);
      color.set(pair.color || [0.28, 0.43, 0.62, 0.18], index * 4);
      values[index] = Number.isFinite(Number(value)) ? Number(value) : 0;
      const edgeId = `${this.pairSetId}:${pair.pairId}`;
      edgeIds.push(edgeId);
      sourceIds.push(pair.sourceRecordId);
      targetIds.push(pair.targetRecordId);
      pairIds.push(pair.pairId);
      graphEdges.push({
        id: edgeId,
        pairId: pair.pairId,
        source: pair.sourceRecordId,
        target: pair.targetRecordId,
        rowId: pair.sourceRecordId,
        columnId: pair.targetRecordId,
        sourceSpaceId: this.left?.spaceId || null,
        targetSpaceId: this.right?.spaceId || null,
        sourceCoordinateId: this.left?.coordinateId || null,
        targetCoordinateId: this.right?.coordinateId || null,
        value: Number.isFinite(Number(value)) ? Number(value) : null,
        properties: pair.properties || pairProperties(this.dependencePropertyId, value),
        present: true,
      });
    }

    const graph = {
      kind: "paired-space-linked-pairs",
      id: this.bridgeGraphId || this.pairSetId,
      native: true,
      relationId: this.bridgeRelationId,
      relationName: this.bridgeRelationName,
      relationType: this.bridgeRelationType,
      edge_relation_id: this.bridgeRelationId,
      recordIds: pairs.map((pair) => pair.sourceRecordId),
      edges: graphEdges,
      diagnostics: {
        pairCount: pairs.length,
        pairModel: "exported-paired-record-ids",
        dependencePropertyId: this.dependencePropertyId,
        bridgeRelationId: this.bridgeRelationId,
        bridgeGraphId: this.bridgeGraphId,
      },
    };

    return new VisualLayer({
      id: `${this.id}:dependence-bridge`,
      kind: "paired-space-dependence",
      primitive: "RelationEdgeLayer",
      order: 18,
      source: {
        viewId: this.id,
        viewKind: this.kind,
        role: "dependence bridge",
        graph,
        graphId: this.bridgeGraphId || this.pairSetId,
        relationId: this.bridgeRelationId,
        pairSetId: this.pairSetId,
      },
      channels: {
        recordId: createStringChannel(pairIds, "paired-record-id"),
        edgeId: createStringChannel(edgeIds, "edge-id"),
        sourceId: createStringChannel(sourceIds, "source-record-id"),
        targetId: createStringChannel(targetIds, "target-record-id"),
        rowId: createStringChannel(sourceIds, "row-record-id"),
        columnId: createStringChannel(targetIds, "column-record-id"),
        sourcePosition: createChannel(sourcePosition, 3, "source-position"),
        targetPosition: createChannel(targetPosition, 3, "target-position"),
        relationValue: createChannel(values, 1, this.dependencePropertyId || "paired-evidence-value", {
          propertyId: this.dependencePropertyId,
        }),
        color: createChannel(color, 4, "rgba"),
      },
      geometry: { width: 1 },
      material: { alpha: 0.58, transparent: true, depthWrite: false },
      metadata: {
        role: "dependence bridge",
        primaryGrammar: "paired-space",
        edgeCount: pairs.length,
        linkedBrushing: true,
        relationId: this.bridgeRelationId,
        relationName: this.bridgeRelationName,
        relationType: this.bridgeRelationType,
        graph,
        pairedSpaceContract: this.semanticContract(),
        selectionModel: {
          kind: "paired-space-linked-selection",
          pairSetId: this.pairSetId,
          pairSource: "exported-paired-record-ids",
          pairIdChannel: "recordId",
          sourceRecordChannel: "sourceId",
          targetRecordChannel: "targetId",
          rowRole: this.leftRole,
          columnRole: this.rightRole,
          respondsTo: ["record", "pair"],
          selectedFeatures: ["source-space-record", "target-space-record", "dependence-bridge"],
        },
      },
    }).toDescriptor();
  }

  validPairs() {
    return this.pairRecords.filter((pair) => (
      getPosition(this.left?.positions, pair.sourceRecordId)
      && getPosition(this.right?.positions, pair.targetRecordId)
    ));
  }
}

export function createCrossSpaceView(options) {
  return new CrossSpaceView(options);
}

function decoratePairedSpace(space, metadata = {}) {
  if (!space) return;
  space.metadata = {
    ...(space.metadata || {}),
    primaryGrammar: "paired-space",
    pairedSpace: {
      role: metadata.role,
      pairedRole: metadata.pairedRole,
      pairSetId: metadata.pairSetId,
      pairedSpaceId: metadata.pairedSpaceId,
      pairedCoordinateId: metadata.pairedCoordinateId,
    },
    pairedSpaceContract: metadata.contract,
    selectionModel: {
      kind: "paired-space-side-records",
      pairSetId: metadata.pairSetId,
      pairedSpaceRole: metadata.role,
      respondsTo: ["record"],
      linkedPairInterpretation: "selected record id addresses a paired observation in the companion space",
    },
  };
}

function normalizePairRecords(records, left, right) {
  if (!Array.isArray(records)) return [];
  const out = [];
  for (let index = 0; index < records.length; index += 1) {
    const pair = normalizePairRecord(records[index], index);
    if (pair) out.push(pair);
  }
  return out.filter((pair) => (
    !left?.recordIds || left.recordIds.includes(pair.sourceRecordId)
  ) && (
    !right?.recordIds || right.recordIds.includes(pair.targetRecordId)
  ));
}

function normalizePairRecord(record, index) {
  if (Array.isArray(record)) {
    const source = record[0];
    const target = record[1] ?? record[0];
    if (source == null || target == null) return null;
    return {
      pairId: String(record[2] ?? source),
      sourceRecordId: String(source),
      targetRecordId: String(target),
    };
  }
  if (!record || typeof record !== "object") return null;
  const source = record.sourceRecordId
    ?? record.source_record_id
    ?? record.sourceId
    ?? record.source_id
    ?? record.rowId
    ?? record.row_id
    ?? record.recordId
    ?? record.record_id
    ?? record.id;
  const target = record.targetRecordId
    ?? record.target_record_id
    ?? record.targetId
    ?? record.target_id
    ?? record.columnId
    ?? record.column_id
    ?? record.pairedRecordId
    ?? record.paired_record_id
    ?? source;
  if (source == null || target == null) return null;
  return {
    pairId: String(record.pairId ?? record.pair_id ?? record.id ?? source ?? index),
    sourceRecordId: String(source),
    targetRecordId: String(target),
    value: record.value,
    color: record.color,
    properties: record.properties || record.pairProperties || record.pair_properties || null,
  };
}

function sharedRecordIdPairs(left, right) {
  const rightIds = new Set((right?.recordIds || []).map(String));
  const pairs = [];
  for (const id of left?.recordIds || []) {
    const key = String(id);
    if (!rightIds.has(key)) continue;
    pairs.push({
      pairId: key,
      sourceRecordId: key,
      targetRecordId: key,
    });
  }
  return pairs;
}

function translatePositionMap(source, ids = [], offset = [0, 0, 0]) {
  const out = new Map();
  for (const id of ids || []) {
    const position = getPosition(source, id);
    if (!position) continue;
    out.set(String(id), [
      (Number(position[0]) || 0) + offset[0],
      (Number(position[1]) || 0) + offset[1],
      (Number(position[2]) || 0) + offset[2],
    ]);
  }
  return out;
}

function getPosition(map, id) {
  return map?.get?.(id) || map?.get?.(String(id)) || null;
}

function dependenceValue(left, right, pair) {
  const explicit = pair.value;
  if (Number.isFinite(Number(explicit))) return Number(explicit);
  const source = left?.scalarValues?.get?.(pair.sourceRecordId)
    ?? left?.scalarValues?.get?.(String(pair.sourceRecordId));
  if (Number.isFinite(Number(source))) return Number(source);
  const target = right?.scalarValues?.get?.(pair.targetRecordId)
    ?? right?.scalarValues?.get?.(String(pair.targetRecordId));
  return Number.isFinite(Number(target)) ? Number(target) : null;
}

function pairProperties(propertyId, value) {
  if (!propertyId || !Number.isFinite(Number(value))) return null;
  return [{ id: propertyId, label: propertyId, value: Number(value) }];
}

function findCrossSpaceViewDeclaration(document, viewId = null) {
  const views = Array.isArray(document?.views) ? document.views : [];
  if (viewId != null) {
    const direct = views.find((view) => String(view?.id ?? view?.name) === String(viewId));
    if (direct) return direct;
  }
  return views.find((view) => {
    const kind = String(view?.kind || view?.view_kind || "").toLowerCase();
    return kind === "paired-space" || kind === "cross-space" || kind === "cross_space";
  }) || null;
}

function findCrossSpaceDependenceDiagnostic(document, leftSpaceId, rightSpaceId, diagnosticId = null) {
  const diagnostics = Array.isArray(document?.diagnostics) ? document.diagnostics : [];
  if (diagnosticId != null) {
    const direct = diagnostics.find((diagnostic) => String(diagnostic?.id ?? diagnostic?.name) === String(diagnosticId));
    if (direct) return normalizeCrossSpaceDependenceDiagnostic(direct);
  }
  for (const diagnostic of diagnostics) {
    if (diagnostic?.kind !== "cross_space_dependence") continue;
    const spaceIds = (diagnostic.space_ids || diagnostic.spaceIds || []).map(String);
    if (leftSpaceId && rightSpaceId && spaceIds.length) {
      const hasBoth = spaceIds.includes(String(leftSpaceId)) && spaceIds.includes(String(rightSpaceId));
      if (!hasBoth) continue;
    }
    return normalizeCrossSpaceDependenceDiagnostic(diagnostic);
  }
  return null;
}

function normalizeCrossSpaceDependenceDiagnostic(diagnostic) {
  if (!diagnostic) return null;
  return {
    id: diagnostic.id || null,
    kind: diagnostic.kind,
    statistic: finiteOrNull(diagnostic.compare_statistic ?? diagnostic.compareStatistic ?? diagnostic.values?.mgc_statistic ?? diagnostic.report?.statistic),
    alpha: finiteOrNull(diagnostic.alpha),
    report: diagnostic.report || null,
    relationIds: diagnostic.relation_ids || diagnostic.relationIds || null,
  };
}

function vector3(value, fallback) {
  const source = Array.isArray(value) || ArrayBuffer.isView(value) ? value : fallback;
  return [
    finiteNumber(source[0], fallback[0]),
    finiteNumber(source[1], fallback[1]),
    finiteNumber(source[2], fallback[2]),
  ];
}

function finiteNumber(value, fallback) {
  const number = Number(value);
  return Number.isFinite(number) ? number : fallback;
}

function finiteOrNull(value) {
  const number = Number(value);
  return Number.isFinite(number) ? number : null;
}
