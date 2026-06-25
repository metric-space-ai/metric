import {
  createTrajectoryBundleLayerDescriptor,
  createTubeRibbonPathLayerDescriptor,
} from "../curves/index.js";
import { BaseView } from "./BaseView.js";
import { defaultCoordinateId } from "./MetricSpaceView.js";
import { applyPositionFit, computePositionFit } from "./scene-fit.js";
import {
  extractCoordinatePositions,
  inferRecordIds,
  recordsFor,
  resolveCollectionItem,
} from "./view-utils.js";

/**
 * TrajectoryPathView is the reusable trajectory/path grammar for METRIC Visual.
 * It translates exported record order, explicit record-id paths, graph/transition
 * edges, or exported timeline coordinate states into CurveRibbon/CurveTube
 * descriptors. It does not solve, smooth, cluster, or infer metric structure.
 */
export class TrajectoryPathView extends BaseView {
  constructor(options = {}) {
    super({ ...options, kind: options.kind || "trajectory-path" });
    this.recordIds = normalizeIds(options.recordIds || inferRecordIds(options));
    this.positions = options.positions || new Map();
    this.stepStates = Array.isArray(options.stepStates) ? options.stepStates : [];
    this.paths = normalizeExplicitPaths(options.paths || options.trajectories || options.routes || []);
    this.pathRecordIds = options.pathRecordIds || options.recordPaths || options.pathIds || null;
    this.graph = options.graph || null;
    this.relation = options.relation || null;
    this.pathSource = options.pathSource || options.sourceMode || null;
    this.edgeMode = options.edgeMode || "auto";
    this.fitEnabled = options.fit !== false;
    this.groundY = finiteNumber(options.groundY, 0);
    this.targetRadius = finiteNumber(options.targetRadius, 1.6);
    this.mode = options.mode || options.trackMode || "ribbon";
    this.descriptorFactory = options.descriptorFactory || (this.mode === "tube" ? "tube-ribbon" : "trajectory-bundle");
    this.descriptorKind = options.descriptorKind || options.layerKind || this.kind;
    this.sourceViewKind = options.sourceViewKind || this.kind;
    this.order = finiteNumber(options.order, 30);
    this.width = finiteNumber(options.width, options.pathWidth, options.trackWidth, 4);
    this.alpha = finiteNumber(options.alpha, options.pathAlpha, options.trackAlpha, 0.78);
    this.color = options.color || options.pathColor || options.defaultColor || [0.14, 0.36, 0.46, this.alpha];
    this.pathCount = Math.max(1, Math.floor(finiteNumber(options.pathCount, 1)));
    this.defaultWidth = finiteNumber(options.defaultWidth, this.width);
    this.colorMode = options.colorMode || (this.stepStates.length > 1 ? "timeline" : "constant");
    this.evidenceRole = options.evidenceRole || "trajectory/path";
    this.motionGrammar = options.motionGrammar || null;
    this.nativeEvidence = options.nativeEvidence || createNativeEvidenceReference(options.document, {
      source: this.pathSource,
      datasetId: this.datasetId,
      spaceId: this.spaceId,
      coordinateId: this.coordinateId,
      relationId: this.relationId || this.relation?.id,
      graphId: this.graph?.id,
      timelineId: options.timelineId || options.metadata?.timelineId,
    });
    this.curveOptions = {
      ...(options.curveOptions || {}),
    };
    copyDefined(this.curveOptions, options, [
      "ambient",
      "pointLight",
      "emission",
      "rimLight",
      "coreGlow",
      "edgeFeather",
      "tubeShade",
      "flowStrength",
      "flowScale",
      "flowSpeed",
      "saturation",
      "depthShade",
      "shadowTint",
      "highlightColor",
      "radius",
      "radialSegments",
      "useWidthsAsRadius",
      "depthWrite",
      "depthTest",
      "colorMix",
      "resample",
      "maxSegmentLength",
      "samples",
    ]);

    if (this.fitEnabled && this.positions && !this.stepStates.length && !this.paths.length) {
      this.fit = computePositionFit(this.positions, this.recordIds, {
        targetRadius: this.targetRadius,
        groundY: this.groundY,
      });
      this.positions = applyPositionFit(this.positions, this.recordIds, this.fit.transform);
    } else {
      this.fit = null;
    }
  }

  static fromVisualSpace(document, options = {}) {
    const spaceRef = options.space || options.spaceId || options.space_id;
    const space = resolveCollectionItem(document, "spaces", spaceRef, {
      required: spaceRef != null,
      label: "space",
    });
    const coordinateRef = options.coordinate
      ?? options.coordinateId
      ?? options.coordinate_id
      ?? defaultCoordinateId(document, space, { dimension: 3 });
    const coordinate = resolveCollectionItem(document, "coordinates", coordinateRef, {
      required: coordinateRef != null,
      label: options.coordinate || options.coordinateId ? "coordinate" : "default coordinate",
    });
    const datasetId = options.datasetId ?? coordinate?.dataset_id ?? space?.dataset_id;
    const records = recordsFor(document, { ...options, datasetId });
    const positions = extractCoordinatePositions(coordinate, {
      records,
      recordIds: options.recordIds || space?.record_ids,
    });
    const graph = resolveTrajectoryGraph(document, options, datasetId);
    const relation = resolveTrajectoryRelation(document, options, datasetId, graph);
    return new TrajectoryPathView({
      ...options,
      document,
      records,
      recordIds: positions.ids,
      positions: positions.positions,
      graph,
      relation,
      graphId: graph?.id,
      relationId: relation?.id,
      datasetId,
      spaceId: options.spaceId ?? space?.id ?? coordinate?.space_id,
      coordinateId: coordinate?.id,
      nativeEvidence: createNativeEvidenceReference(document, {
        source: options.pathSource,
        datasetId,
        spaceId: options.spaceId ?? space?.id ?? coordinate?.space_id,
        coordinateId: coordinate?.id,
        relationId: relation?.id,
        graphId: graph?.id,
        timelineId: options.timelineId,
      }),
    });
  }

  static fromMetricSpaceView(space, options = {}) {
    const graph = options.graph || resolveTrajectoryGraph(options.document, options, space?.datasetId);
    const relation = options.relation || resolveTrajectoryRelation(options.document, options, space?.datasetId, graph);
    return new TrajectoryPathView({
      ...options,
      records: space?.records || [],
      recordIds: space?.recordIds || [],
      positions: space?.positions || new Map(),
      graph,
      relation,
      graphId: graph?.id,
      relationId: relation?.id,
      datasetId: space?.datasetId,
      spaceId: space?.spaceId,
      coordinateId: space?.coordinateId,
      fit: false,
      nativeEvidence: createNativeEvidenceReference(options.document, {
        source: options.pathSource,
        datasetId: space?.datasetId,
        spaceId: space?.spaceId,
        coordinateId: space?.coordinateId,
        relationId: relation?.id,
        graphId: graph?.id,
        timelineId: options.timelineId,
      }),
    });
  }

  toLayerDescriptors() {
    const evidence = this.buildPathEvidence();
    if (!evidence.paths.length) return [];

    const nativeEvidence = {
      ...(this.nativeEvidence || {}),
      source: this.nativeEvidence?.source || evidence.source,
    };
    const descriptorOptions = this.descriptorOptions();
    const descriptor = this.descriptorFactory === "tube-ribbon" || this.mode === "tube"
      ? createTubeRibbonPathLayerDescriptor({ paths: evidence.paths }, descriptorOptions)
      : createTrajectoryBundleLayerDescriptor({ paths: evidence.paths }, descriptorOptions);

    descriptor.kind = this.descriptorKind;
    descriptor.source = {
      ...(descriptor.source || {}),
      viewId: this.id,
      viewKind: this.sourceViewKind,
      viewClass: "TrajectoryPathView",
      datasetId: this.datasetId,
      spaceId: this.spaceId,
      coordinateId: this.coordinateId,
      relationId: this.relationId || this.relation?.id || null,
      graphId: this.graph?.id || null,
      nativeEvidence,
    };
    descriptor.metadata = {
      ...(descriptor.metadata || {}),
      ...this.metadata,
      role: "trajectory/path",
      evidenceRole: this.evidenceRole,
      viewClass: "TrajectoryPathView",
      viewKind: this.sourceViewKind,
      pathSource: evidence.source,
      nativeEvidence,
      recordCount: evidence.recordCount,
      pathCount: evidence.paths.length,
      requestedPathCount: this.pathCount,
      graphId: this.graph?.id || null,
      relationId: this.relationId || this.relation?.id || null,
      coordinateId: this.coordinateId || null,
      coordinateIds: this.metadata?.coordinateIds || evidence.coordinateIds || undefined,
      timelineId: this.metadata?.timelineId || this.nativeEvidence?.timelineId || null,
      motionGrammar: this.motionGrammar || this.metadata?.motionGrammar,
      algorithmicComputation: false,
    };
    return [descriptor];
  }

  descriptorOptions() {
    return {
      id: this.id,
      kind: this.descriptorKind,
      order: this.order,
      mode: this.mode,
      alpha: this.alpha,
      defaultWidth: this.defaultWidth,
      width: this.width,
      defaultColor: this.color,
      evidenceRole: this.evidenceRole,
      ...this.curveOptions,
    };
  }

  buildPathEvidence() {
    if (this.paths.length) {
      const recordCount = countPathRecords(this.paths) || this.recordIds.length;
      return {
        source: this.pathSource || "explicit-paths",
        paths: this.paths.map((path, index) => this.normalizePath(path, index, "explicit-path")),
        recordCount,
      };
    }

    if (this.stepStates.length > 1) {
      const paths = this.pathsFromStepStates();
      return {
        source: this.pathSource || "exported-timeline-states",
        paths,
        recordCount: this.recordIds.length,
        coordinateIds: this.stepStates.map((state) => state.coordinateId).filter(Boolean),
      };
    }

    const pathIdSequences = normalizePathIdSequences(this.pathRecordIds);
    if (pathIdSequences.length) {
      const paths = this.pathsFromRecordIdSequences(pathIdSequences, "explicit-record-ids");
      return {
        source: this.pathSource || "explicit-record-ids",
        paths,
        recordCount: countUnique(pathIdSequences.flat()),
      };
    }

    if (this.graph || this.relation) {
      const graphPaths = this.pathsFromGraphOrRelation();
      if (graphPaths.length) {
        return {
          source: this.pathSource || (this.graph ? "exported-graph-transition" : "exported-transition-relation"),
          paths: graphPaths,
          recordCount: countUnique(graphPaths.flatMap((path) => path.metadata?.recordIds || [])),
        };
      }
    }

    const recordOrder = this.pathsFromRecordOrder();
    return {
      source: this.pathSource || "exported-record-order",
      paths: recordOrder,
      recordCount: this.recordIds.length,
    };
  }

  normalizePath(path, index, evidenceType) {
    const recordIds = normalizeIds(path.recordIds || path.record_ids || path.ids || []);
    if (recordIds.length && !hasPointData(path)) {
      return this.pathFromRecordIds(recordIds, {
        id: path.id || `${this.id}:path-${index}`,
        color: path.color,
        width: path.width,
        metadata: path.metadata,
        evidenceType,
      });
    }
    return {
      ...path,
      id: path.id || `${this.id}:path-${index}`,
      color: path.color || this.color,
      width: finiteNumber(path.width, this.width),
      metadata: {
        ...(path.metadata || {}),
        evidenceType: path.evidenceType || path.type || evidenceType,
        sourceView: this.id,
        recordIds,
      },
    };
  }

  pathsFromStepStates() {
    const paths = [];
    for (const recordId of this.recordIds) {
      const points = [];
      for (let stepIndex = 0; stepIndex < this.stepStates.length; stepIndex += 1) {
        const state = this.stepStates[stepIndex];
        const position = positionFor(state?.positions, recordId);
        if (!position) continue;
        points.push({
          x: Number(position[0]) || 0,
          y: Number(position[1]) || 0,
          z: Number(position[2]) || 0,
          time: Number.isFinite(Number(state.time)) ? Number(state.time) : stepIndex,
          color: this.colorMode === "timeline"
            ? timelinePathColor(stepIndex, this.stepStates.length, this.alpha)
            : this.color,
          width: this.width,
        });
      }
      if (points.length >= 2) {
        paths.push({
          id: `${this.id}:record:${recordId}`,
          points,
          metadata: {
            recordId,
            recordIds: [recordId],
            evidenceType: "exported-timeline-state-history",
            timelineId: this.metadata?.timelineId || this.nativeEvidence?.timelineId,
          },
        });
      }
    }
    return paths;
  }

  pathsFromRecordIdSequences(sequences, evidenceType) {
    return sequences
      .map((ids, index) => this.pathFromRecordIds(ids, {
        id: `${this.id}:path-${index}`,
        evidenceType,
      }))
      .filter(Boolean);
  }

  pathsFromGraphOrRelation() {
    const edges = this.graph
      ? extractGraphEdges(this.graph)
      : extractRelationEdges(this.relation);
    const sequences = edgeRecordSequences(edges, this.edgeMode);
    return this.pathsFromRecordIdSequences(sequences, this.graph ? "exported-graph-transition" : "exported-transition-relation");
  }

  pathsFromRecordOrder() {
    const ids = this.recordIds.slice();
    const segmentSize = Math.max(2, Math.ceil(ids.length / this.pathCount));
    const paths = [];
    for (let start = 0; start < ids.length; start += segmentSize) {
      const group = ids.slice(start, start + segmentSize);
      const path = this.pathFromRecordIds(group, {
        id: `${this.id}:path-${paths.length}`,
        evidenceType: "exported-record-order",
      });
      if (path) paths.push(path);
    }
    return paths;
  }

  pathFromRecordIds(ids, options = {}) {
    const points = [];
    const recordIds = normalizeIds(ids);
    for (let index = 0; index < recordIds.length; index += 1) {
      const position = positionFor(this.positions, recordIds[index]);
      if (!position) continue;
      points.push({
        x: Number(position[0]) || 0,
        y: Number(position[1]) || 0,
        z: Number(position[2]) || 0,
        time: index,
        color: options.color || this.color,
        width: finiteNumber(options.width, this.width),
      });
    }
    if (points.length < 2) return null;
    return {
      id: options.id || `${this.id}:path`,
      points,
      color: options.color || this.color,
      width: finiteNumber(options.width, this.width),
      metadata: {
        ...(options.metadata || {}),
        evidenceType: options.evidenceType || "trajectory/path",
        sourceView: this.id,
        recordIds,
      },
    };
  }
}

export function createTrajectoryPathView(options) {
  return new TrajectoryPathView(options);
}

function resolveTrajectoryGraph(document, options = {}, datasetId = null) {
  const graphRef = options.graph || options.graphId || options.pathGraph || options.pathGraphId;
  if (graphRef != null) {
    return resolveCollectionItem(document, "graphs", graphRef, {
      required: true,
      label: "trajectory graph",
    });
  }
  if (options.preferGraph !== true && options.pathSource !== "graph" && options.pathSource !== "transition") return null;
  const graphs = Array.isArray(document?.graphs) ? document.graphs : [];
  return graphs.find((graph) => (
    (datasetId == null || String(graph.dataset_id ?? graph.datasetId) === String(datasetId)) &&
    /trajectory|transition|path/i.test(`${graph.id || ""} ${graph.name || ""} ${graph.graph_type || ""}`)
  )) || null;
}

function resolveTrajectoryRelation(document, options = {}, datasetId = null, graph = null) {
  const relationRef = options.relation || options.relationId || options.pathRelation || options.pathRelationId || graph?.edge_relation_id;
  if (relationRef != null) {
    return resolveCollectionItem(document, "relations", relationRef, {
      required: false,
      label: "trajectory relation",
    });
  }
  if (options.pathSource !== "transition") return null;
  const relations = Array.isArray(document?.relations) ? document.relations : [];
  return relations.find((relation) => (
    (datasetId == null || String(relation.dataset_id ?? relation.datasetId) === String(datasetId)) &&
    /transition|trajectory|path/i.test(`${relation.id || ""} ${relation.name || ""} ${relation.relation_type || ""}`)
  )) || null;
}

function normalizeExplicitPaths(paths) {
  if (!paths) return [];
  if (Array.isArray(paths)) return paths;
  if (paths.paths && Array.isArray(paths.paths)) return paths.paths;
  return [paths];
}

function normalizePathIdSequences(value) {
  if (!value) return [];
  if (Array.isArray(value)) {
    if (!value.length) return [];
    if (Array.isArray(value[0])) return value.map((entry) => normalizeIds(entry)).filter((entry) => entry.length >= 2);
    if (typeof value[0] === "object") {
      return value
        .map((entry) => normalizeIds(entry.recordIds || entry.record_ids || entry.ids || entry.path || []))
        .filter((entry) => entry.length >= 2);
    }
    return [normalizeIds(value)].filter((entry) => entry.length >= 2);
  }
  if (typeof value === "object") {
    return Object.values(value)
      .map((entry) => normalizeIds(entry))
      .filter((entry) => entry.length >= 2);
  }
  return [];
}

function extractGraphEdges(graph) {
  return (Array.isArray(graph?.edges) ? graph.edges : [])
    .map((edge, index) => normalizeEdge(edge, index))
    .filter(Boolean);
}

function extractRelationEdges(relation) {
  const values = relation?.values || relation?.matrix || [];
  const ids = normalizeIds(relation?.record_ids || relation?.recordIds || []);
  const edges = [];
  if (Array.isArray(values) && values.length && Array.isArray(values[0])) {
    for (let row = 0; row < values.length; row += 1) {
      for (let column = 0; column < values[row].length; column += 1) {
        const value = Number(values[row][column]);
        if (!Number.isFinite(value) || value <= 0 || row === column) continue;
        edges.push({ sourceId: ids[row] || String(row), targetId: ids[column] || String(column), value, index: edges.length });
      }
    }
    return edges;
  }
  if (Array.isArray(values)) {
    return values
      .map((entry, index) => normalizeEdge(entry, index))
      .filter(Boolean);
  }
  return edges;
}

function normalizeEdge(edge, index) {
  if (!edge) return null;
  const sourceId = edge.source_id ?? edge.sourceId ?? edge.source_record_id ?? edge.sourceRecordId ?? edge.row_id ?? edge.rowId ?? edge.source ?? edge.from;
  const targetId = edge.target_id ?? edge.targetId ?? edge.target_record_id ?? edge.targetRecordId ?? edge.column_id ?? edge.columnId ?? edge.target ?? edge.to;
  if (sourceId == null || targetId == null || String(sourceId) === String(targetId)) return null;
  return {
    sourceId: String(sourceId),
    targetId: String(targetId),
    value: edge.value ?? edge.weight ?? edge.probability ?? edge.affinity ?? null,
    index,
  };
}

function edgeRecordSequences(edges, edgeMode = "auto") {
  if (!edges.length) return [];
  if (edgeMode === "segments") return edges.map((edge) => [edge.sourceId, edge.targetId]);

  const outgoing = new Map();
  const incoming = new Map();
  const nodes = new Set();
  for (const edge of edges) {
    nodes.add(edge.sourceId);
    nodes.add(edge.targetId);
    if (!outgoing.has(edge.sourceId)) outgoing.set(edge.sourceId, []);
    outgoing.get(edge.sourceId).push(edge);
    incoming.set(edge.targetId, (incoming.get(edge.targetId) || 0) + 1);
    incoming.set(edge.sourceId, incoming.get(edge.sourceId) || 0);
  }

  const branchy = Array.from(nodes).some((node) => (outgoing.get(node)?.length || 0) > 1 || (incoming.get(node) || 0) > 1);
  if (branchy && edgeMode !== "chain") return edges.map((edge) => [edge.sourceId, edge.targetId]);

  const visited = new Set();
  const starts = Array.from(nodes)
    .filter((node) => (incoming.get(node) || 0) === 0 && (outgoing.get(node)?.length || 0) > 0);
  if (!starts.length) starts.push(edges[0].sourceId);

  const sequences = [];
  for (const start of starts) {
    const sequence = [start];
    let current = start;
    while (outgoing.get(current)?.length) {
      const next = outgoing.get(current).find((edge) => !visited.has(edgeKey(edge)));
      if (!next) break;
      visited.add(edgeKey(next));
      sequence.push(next.targetId);
      current = next.targetId;
      if (sequence.length > edges.length + 1) break;
    }
    if (sequence.length >= 2) sequences.push(sequence);
  }

  for (const edge of edges) {
    if (!visited.has(edgeKey(edge))) sequences.push([edge.sourceId, edge.targetId]);
  }
  return sequences;
}

function edgeKey(edge) {
  return `${edge.sourceId}->${edge.targetId}@${edge.index}`;
}

function positionFor(map, id) {
  return map?.get?.(id) || map?.get?.(String(id)) || null;
}

function hasPointData(path) {
  return Boolean(path?.points || path?.positions || path?.coordinates || path?.coords || path?.path);
}

function normalizeIds(ids) {
  if (!Array.isArray(ids)) return [];
  return ids.filter((id) => id != null).map(String);
}

function countPathRecords(paths) {
  const ids = [];
  for (const path of paths) {
    ids.push(...normalizeIds(path.recordIds || path.record_ids || path.ids || path.metadata?.recordIds || []));
  }
  return countUnique(ids);
}

function countUnique(values) {
  return new Set(normalizeIds(values)).size;
}

function timelinePathColor(stepIndex, stepCount, alpha) {
  const t = stepCount <= 1 ? 0 : stepIndex / (stepCount - 1);
  const low = [0.12, 0.25, 0.42];
  const mid = [0.12, 0.54, 0.52];
  const high = [0.92, 0.54, 0.22];
  const color = t < 0.58
    ? mixColor(low, mid, t / 0.58)
    : mixColor(mid, high, (t - 0.58) / 0.42);
  return [color[0], color[1], color[2], alpha];
}

function mixColor(a, b, t) {
  const x = Math.max(0, Math.min(1, Number.isFinite(t) ? t : 0));
  return [
    a[0] + (b[0] - a[0]) * x,
    a[1] + (b[1] - a[1]) * x,
    a[2] + (b[2] - a[2]) * x,
  ];
}

function createNativeEvidenceReference(document, options = {}) {
  const provenance = document?.provenance
    ? {
      writer: document.provenance.writer || null,
      runtime: document.provenance.runtime || null,
      source_example: document.provenance.source_example || null,
      native_export: document.provenance.native_export === true || document.provenance.nativeExport === true,
      synthetic: document.provenance.synthetic === true,
      synthetic_js: document.provenance.synthetic_js === true,
    }
    : null;
  return {
    schema: "metric.visual.trajectory_path_evidence_ref.v1",
    source: options.source || null,
    documentSchema: document?.schema || null,
    provenance,
    datasetId: options.datasetId || null,
    spaceId: options.spaceId || null,
    coordinateId: options.coordinateId || null,
    relationId: options.relationId || null,
    graphId: options.graphId || null,
    timelineId: options.timelineId || null,
  };
}

function copyDefined(target, source, keys) {
  for (const key of keys) {
    if (source[key] !== undefined) target[key] = source[key];
  }
}

function finiteNumber(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number)) return number;
  }
  return 0;
}
