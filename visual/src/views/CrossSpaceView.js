import { BaseView } from "./BaseView.js";
import { MetricSpaceView } from "./MetricSpaceView.js";
import { VisualLayer } from "./VisualLayer.js";
import {
  createChannel,
  createStringChannel,
  resolveCollectionItem,
} from "./view-utils.js";
import {
  pairSourceId,
  pairTargetId,
  pairValue,
  relationPairs,
} from "../relational/relation-source.js";

const PAIRED_SPACE_VIEW_SCHEMA = "metric.visual.paired_space_view.v1";
const DEFAULT_LEFT_OFFSET = Object.freeze([-1.42, 0, 0]);
const DEFAULT_RIGHT_OFFSET = Object.freeze([1.42, 0, 0]);
const DEFAULT_SOURCE_COLOR = Object.freeze([0.06, 0.45, 0.51, 1]);
const DEFAULT_TARGET_COLOR = Object.freeze([0.78, 0.43, 0.17, 1]);

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
    this.leftOffset = vector3(options.leftOffset, DEFAULT_LEFT_OFFSET);
    this.rightOffset = vector3(options.rightOffset, DEFAULT_RIGHT_OFFSET);
    this.groundY = finiteNumber(options.groundY, -0.58);
    this.ground = options.ground !== false;
    this.spaceLabels = options.spaceLabels !== false;
    this.leftLabel = options.leftLabel || options.leftName || "source metric space";
    this.rightLabel = options.rightLabel || options.rightName || "target metric space";
    this.leftSubLabel = options.leftSubLabel || options.leftSubtitle || this.left?.spaceId || "";
    this.rightSubLabel = options.rightSubLabel || options.rightSubtitle || this.right?.spaceId || "";
    this.leftColor = color4(options.leftColor, DEFAULT_SOURCE_COLOR);
    this.rightColor = color4(options.rightColor, DEFAULT_TARGET_COLOR);
    this.pairSetId = options.pairSetId || `${this.id}:linked-pairs`;
    this.dependencePropertyId = options.dependencePropertyId || options.scalarProperty || null;
    this.bridgeRelationId = options.bridgeRelationId || options.bridgeRelation || null;
    this.bridgeRelationName = options.bridgeRelationName || null;
    this.bridgeRelationType = options.bridgeRelationType || null;
    this.bridgeGraphId = options.bridgeGraphId || options.graphId || this.pairSetId;
    this.bridgeRelation = options.bridgeRelation || null;
    this.bridgeRecords = normalizeBridgeRecords(options.bridgeRecords || options.bridgePairs || options.bridges, this.left, this.right);
    if (!this.bridgeRecords.length) this.bridgeRecords = normalizeBridgeRelationRecords(this.bridgeRelation, this.left, this.right);
    this.bridgeMaxCount = positiveIntegerOrNull(options.bridgeMaxCount || options.bridgeLimit || options.maxBridgeCount);
    this.bridgeAlphaRange = range2(options.bridgeAlphaRange || options.bridgeAlpha, [0.12, 0.46]);
    this.bridgeValueRange = options.bridgeValueRange || null;
    this.bridgeColorLow = color4(options.bridgeColorLow, [0.18, 0.43, 0.58, 1]);
    this.bridgeColorHigh = color4(options.bridgeColorHigh, [0.86, 0.34, 0.21, 1]);
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
      pointMaterial: {
        pointPixelScale: 8.4,
        minPointSize: 2.3,
        maxPointSize: 26,
        alpha: 0.9,
        focusBoost: 0.34,
        ...(options.pointMaterial || {}),
        ...(options.leftPointMaterial || {}),
      },
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
      pointMaterial: {
        pointPixelScale: 8.4,
        minPointSize: 2.3,
        maxPointSize: 26,
        alpha: 0.9,
        focusBoost: 0.34,
        ...(options.pointMaterial || {}),
        ...(options.rightPointMaterial || {}),
      },
      metadata: {
        ...(options.rightMetadata || {}),
        role: "paired-space-side",
        pairedSpaceRole: options.rightRole || "target-space",
      },
    });

    left.positions = translatePositionMap(left.positions, left.recordIds, vector3(options.leftOffset, DEFAULT_LEFT_OFFSET));
    right.positions = translatePositionMap(right.positions, right.recordIds, vector3(options.rightOffset, DEFAULT_RIGHT_OFFSET));

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
      bridgeRelation,
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
      visibleBridgeCount: this.visibleBridgeRecords().length,
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
        bridgeEvidence: this.bridgeEvidenceSummary(),
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
      visibleBridgeCount: this.visibleBridgeRecords().length,
    };
  }

  toLayerDescriptors() {
    const descriptors = [];
    if (this.ground) descriptors.push(this.sharedGroundDescriptor());
    descriptors.push(...this.sideDescriptors(this.left, this.leftRole));
    descriptors.push(...this.sideDescriptors(this.right, this.rightRole));
    if (this.spaceLabels) {
      const labels = this.spaceLabelDescriptor();
      if (labels) descriptors.push(labels);
    }
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
        width: Math.max(5.4, Math.abs(this.rightOffset[0] - this.leftOffset[0]) + 2.6),
        depth: 3.35,
        y: this.groundY,
        gridScale: 8,
      },
      material: {
        alpha: 0.62,
        gridAlpha: 0.28,
        axisAlpha: 0.2,
        baseColor: [0.86, 0.9, 0.87],
        gridColor: [0.32, 0.44, 0.43],
        axisXColor: [0.16, 0.47, 0.5],
        axisZColor: [0.58, 0.45, 0.28],
      },
      metadata: {
        role: "shared-ground",
        pairedSpaceContract: this.semanticContract(),
      },
    }).toDescriptor();
  }

  spaceLabelDescriptor() {
    const labels = [
      {
        id: "left-space-anchor",
        text: this.leftLabel,
        subtext: this.leftSubLabel,
        position: [
          this.leftOffset[0],
          this.groundY + 0.64,
          -1.18,
        ],
        color: this.leftColor,
        background: [0.96, 0.99, 0.98, 0.86],
        border: [this.leftColor[0], this.leftColor[1], this.leftColor[2], 0.32],
        size: 0.19,
      },
      {
        id: "right-space-anchor",
        text: this.rightLabel,
        subtext: this.rightSubLabel,
        position: [
          this.rightOffset[0],
          this.groundY + 0.64,
          -1.18,
        ],
        color: this.rightColor,
        background: [0.99, 0.97, 0.94, 0.86],
        border: [this.rightColor[0], this.rightColor[1], this.rightColor[2], 0.32],
        size: 0.19,
      },
    ].filter((label) => label.text);

    if (!labels.length) return null;
    return new VisualLayer({
      id: `${this.id}:space-labels`,
      kind: "paired-space-anchors",
      primitive: "BillboardLabelLayer",
      order: 28,
      source: {
        viewId: this.id,
        viewKind: this.kind,
        role: "paired-space-label-anchors",
        labels,
      },
      channels: {},
      geometry: {
        fontSize: 26,
        textureScale: 2,
      },
      material: {
        alpha: 0.9,
        fontSize: 26,
        fontWeight: "760",
        paddingX: 14,
        paddingY: 8,
        maxAtlasRowWidth: 1536,
      },
      metadata: {
        role: "paired-space-label-anchors",
        labelCount: labels.length,
        pairedSpaceContract: this.semanticContract(),
        anchors: labels.map((label) => ({
          id: label.id,
          text: label.text,
          subtext: label.subtext,
          position: label.position,
        })),
      },
    }).toDescriptor();
  }

  bridgeDescriptor() {
    const pairs = this.visibleBridgeRecords();
    if (!pairs.length) return null;
    const sourcePosition = new Float32Array(pairs.length * 3);
    const targetPosition = new Float32Array(pairs.length * 3);
    const color = new Float32Array(pairs.length * 4);
    const values = new Float32Array(pairs.length);
    const edgeEmphasis = new Float32Array(pairs.length);
    const edgeIds = pairs.map((pair) => `${this.pairSetId}:${pair.pairId}`);
    const pairIds = this.pairRecords.map((pair) => pair.pairId);
    const sourceIds = this.pairRecords.map((pair) => pair.sourceRecordId);
    const targetIds = this.pairRecords.map((pair) => pair.targetRecordId);
    const graphEdges = this.graphBridgeEdges();
    const renderedValueRange = numericRange(pairs.map((pair) => dependenceValue(this.left, this.right, pair)), this.bridgeValueRange);

    for (let index = 0; index < pairs.length; index += 1) {
      const pair = pairs[index];
      const source = getPosition(this.left.positions, pair.sourceRecordId);
      const target = getPosition(this.right.positions, pair.targetRecordId);
      const value = dependenceValue(this.left, this.right, pair);
      const t = normalizeValue(value, renderedValueRange);
      const rgba = pair.color || bridgeColor(t, {
        low: this.bridgeColorLow,
        high: this.bridgeColorHigh,
        alphaRange: this.bridgeAlphaRange,
      });
      sourcePosition.set(source, index * 3);
      targetPosition.set(target, index * 3);
      color.set(rgba, index * 4);
      values[index] = Number.isFinite(Number(value)) ? Number(value) : 0;
      edgeEmphasis[index] = 0.22 + t * 0.56;
    }

    const bridgeEvidence = this.bridgeEvidenceSummary({
      renderedCount: pairs.length,
      graphEdgeCount: graphEdges.length,
      valueRange: renderedValueRange,
    });
    const graph = {
      kind: "paired-space-linked-pairs",
      id: this.bridgeGraphId || this.pairSetId,
      native: true,
      relationId: this.bridgeRelationId,
      relationName: this.bridgeRelationName,
      relationType: this.bridgeRelationType,
      edge_relation_id: this.bridgeRelationId,
      recordIds: this.pairRecords.map((pair) => pair.sourceRecordId),
      edges: graphEdges,
      edgeCount: graphEdges.length,
      visibleEdgeIds: edgeIds,
      diagnostics: {
        pairCount: this.pairRecords.length,
        pairModel: "exported-paired-record-ids",
        dependencePropertyId: this.dependencePropertyId,
        bridgeRelationId: this.bridgeRelationId,
        bridgeGraphId: this.bridgeGraphId,
        bridgeEvidence,
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
        edgeEmphasis: createChannel(edgeEmphasis, 1, "bridge-emphasis"),
        color: createChannel(color, 4, "rgba"),
      },
      geometry: {
        width: 1,
        pickWidth: 7,
      },
      material: {
        alpha: 0.82,
        transparent: true,
        depthWrite: false,
        emphasisStrength: 0.5,
      },
      metadata: {
        role: "dependence bridge",
        primaryGrammar: "paired-space",
        edgeCount: graphEdges.length,
        visibleEdgeCount: pairs.length,
        bridgeEvidence,
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

  validBridgeRecords() {
    const candidates = this.bridgeRecords.length ? this.bridgeRecords : this.validPairs();
    return candidates.filter((pair) => (
      getPosition(this.left?.positions, pair.sourceRecordId)
      && getPosition(this.right?.positions, pair.targetRecordId)
    ));
  }

  visibleBridgeRecords() {
    const records = this.validBridgeRecords();
    if (!this.bridgeMaxCount || records.length <= this.bridgeMaxCount) return records;
    return records.slice(0, this.bridgeMaxCount);
  }

  graphBridgeEdges() {
    const explicitByPair = new Map(this.bridgeRecords.map((pair) => [pairKey(pair.sourceRecordId, pair.targetRecordId), pair]));
    return this.validPairs().map((pair, index) => {
      const explicit = explicitByPair.get(pairKey(pair.sourceRecordId, pair.targetRecordId));
      const value = dependenceValue(this.left, this.right, explicit || pair);
      return {
        id: `${this.pairSetId}:${pair.pairId}`,
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
        properties: explicit?.properties || pair.properties || pairProperties(this.dependencePropertyId, value),
        present: Boolean(explicit || !this.bridgeRecords.length),
        visibleBridge: Boolean(explicit || !this.bridgeRecords.length),
        sourceIndex: index,
        targetIndex: index,
      };
    });
  }

  bridgeEvidenceSummary(overrides = {}) {
    const candidateCount = this.validPairs().length;
    const exportedRelationCount = this.bridgeRecords.length;
    const validBridgeCount = this.validBridgeRecords().length;
    const renderedCount = overrides.renderedCount ?? this.visibleBridgeRecords().length;
    const capped = Boolean(this.bridgeMaxCount && validBridgeCount > this.bridgeMaxCount);
    return {
      schema: "metric.visual.cross_space_bridge_sampling.v1",
      source: exportedRelationCount ? "exported-relation-values" : "paired-record-fallback",
      relationId: this.bridgeRelationId,
      graphId: this.bridgeGraphId,
      candidatePairCount: candidateCount,
      exportedBridgeCount: exportedRelationCount || candidateCount,
      validBridgeCount,
      renderedBridgeCount: renderedCount,
      graphEdgeCount: overrides.graphEdgeCount ?? candidateCount,
      visibleBridgeStrategy: exportedRelationCount
        ? (capped ? "first-exported-relation-values-limit" : "exported-relation-values")
        : "all-paired-records",
      bridgeLimit: this.bridgeMaxCount,
      capped,
      valueRange: overrides.valueRange || numericRange(this.validBridgeRecords().map((pair) => dependenceValue(this.left, this.right, pair)), this.bridgeValueRange),
      selectionGraph: "full-paired-record-ids",
      nativeEvidenceOnly: true,
      syntheticEvidence: false,
    };
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

function normalizeBridgeRecords(records, left, right) {
  if (!Array.isArray(records)) return [];
  const out = [];
  for (let index = 0; index < records.length; index += 1) {
    const pair = normalizePairRecord(records[index], index);
    if (pair) out.push(pair);
  }
  return filterBridgeRecords(out, left, right);
}

function normalizeBridgeRelationRecords(relation, left, right) {
  if (!relation) return [];
  const recordIds = (relation.record_ids || relation.recordIds || []).map(String);
  const pairs = [];
  const values = relationPairs(relation);
  for (let index = 0; index < values.length; index += 1) {
    const value = values[index];
    const source = pairSourceId(value, recordIds);
    const target = pairTargetId(value, recordIds) || source;
    if (!source || !target) continue;
    const exportedValue = pairValue(value);
    pairs.push({
      pairId: String(value?.pairId ?? value?.pair_id ?? value?.id ?? source),
      sourceRecordId: String(source),
      targetRecordId: String(target),
      value: Number.isFinite(exportedValue) ? exportedValue : value?.value,
      properties: pairPropertiesFromRelationValue(value),
    });
  }
  return filterBridgeRecords(pairs, left, right);
}

function filterBridgeRecords(records, left, right) {
  const leftIds = new Set((left?.recordIds || []).map(String));
  const rightIds = new Set((right?.recordIds || []).map(String));
  return records.filter((pair) => (
    (!leftIds.size || leftIds.has(String(pair.sourceRecordId)))
    && (!rightIds.size || rightIds.has(String(pair.targetRecordId)))
  ));
}

function pairPropertiesFromRelationValue(value) {
  if (!value || typeof value !== "object" || Array.isArray(value)) return null;
  if (Array.isArray(value.properties)) return value.properties;
  return null;
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

function pairKey(sourceId, targetId) {
  return `${String(sourceId)}\u0000${String(targetId)}`;
}

function numericRange(values, explicitRange = null) {
  if (Array.isArray(explicitRange) && explicitRange.length >= 2) {
    const min = Number(explicitRange[0]);
    const max = Number(explicitRange[1]);
    if (Number.isFinite(min) && Number.isFinite(max) && max > min) return [min, max];
  }
  let min = Infinity;
  let max = -Infinity;
  for (const value of values || []) {
    const number = Number(value);
    if (!Number.isFinite(number)) continue;
    min = Math.min(min, number);
    max = Math.max(max, number);
  }
  if (!Number.isFinite(min) || !Number.isFinite(max)) return [0, 1];
  if (min === max) return [min - 0.5, max + 0.5];
  return [min, max];
}

function normalizeValue(value, range) {
  const number = Number(value);
  if (!Number.isFinite(number)) return 0;
  const min = Number(range?.[0]);
  const max = Number(range?.[1]);
  if (!Number.isFinite(min) || !Number.isFinite(max) || max <= min) return 0;
  return Math.max(0, Math.min(1, (number - min) / (max - min)));
}

function bridgeColor(t, options = {}) {
  const low = options.low || [0.18, 0.43, 0.58, 1];
  const high = options.high || [0.86, 0.34, 0.21, 1];
  const alphaRange = options.alphaRange || [0.12, 0.46];
  return [
    mix(low[0], high[0], t),
    mix(low[1], high[1], t),
    mix(low[2], high[2], t),
    mix(alphaRange[0], alphaRange[1], t),
  ];
}

function mix(a, b, t) {
  return (Number(a) || 0) * (1 - t) + (Number(b) || 0) * t;
}

function color4(value, fallback) {
  const source = Array.isArray(value) || ArrayBuffer.isView(value) ? value : fallback;
  return [
    finiteNumber(source[0], fallback[0]),
    finiteNumber(source[1], fallback[1]),
    finiteNumber(source[2], fallback[2]),
    finiteNumber(source[3], fallback[3]),
  ];
}

function range2(value, fallback) {
  const source = Array.isArray(value) || ArrayBuffer.isView(value) ? value : fallback;
  return [
    finiteNumber(source[0], fallback[0]),
    finiteNumber(source[1], fallback[1]),
  ];
}

function positiveIntegerOrNull(value) {
  const number = Math.floor(Number(value));
  return Number.isFinite(number) && number > 0 ? number : null;
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
