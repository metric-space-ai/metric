import {
  buildGroundProjectedPathBuffers,
  buildPolylinePathBuffers,
  buildRibbonPathBuffers,
  buildTubePathGeometry,
} from "./tessellation.js";

/**
 * Create a descriptor for thin polyline path rendering.
 *
 * @param {object|Array|ArrayBufferView} evidence Exported coordinate/time/path evidence.
 * @param {object} [options]
 * @returns {object}
 */
export function createPolylinePathLayerDescriptor(evidence, options = {}) {
  const buffers = buildPolylinePathBuffers(evidence, options);
  return baseCurveDescriptor("CurvePolylineLayer", buffers, options, {
    curveVisualization: "polyline-path",
    drawMode: "lines",
  });
}

/**
 * Create a descriptor for thick camera-facing ribbons or CPU tube geometry.
 *
 * Use `mode: "tube"` for indexed world-space tube geometry rendered by
 * `CurveTubeMeshLayer`. The default `mode: "ribbon"` targets
 * `CurveRibbonLayer`.
 *
 * @param {object|Array|ArrayBufferView} evidence Exported coordinate/time/path evidence.
 * @param {object} [options]
 * @param {"ribbon"|"tube"} [options.mode="ribbon"]
 * @returns {object}
 */
export function createTubeRibbonPathLayerDescriptor(evidence, options = {}) {
  if (options.mode === "tube") {
    const geometry = buildTubePathGeometry(evidence, options);
    return {
      id: options.id || "curve-tube-paths",
      kind: options.kind || "CurveTubeMeshLayer",
      primitive: options.primitive || "CurveTubeMeshLayer",
      order: options.order ?? 30,
      visible: options.visible !== false,
      source: {
        geometry,
        evidence: geometry.evidence,
      },
      channels: {
        position: { array: geometry.positions, itemSize: 3, count: geometry.vertexCount },
        normal: { array: geometry.normals, itemSize: 3, count: geometry.vertexCount },
        color: { array: geometry.colors, itemSize: 4, count: geometry.vertexCount },
        distance: { array: geometry.distances, itemSize: 1, count: geometry.vertexCount },
        pathIndex: { array: geometry.pathIndices, itemSize: 1, count: geometry.vertexCount },
      },
      geometry: {
        mode: "tube",
        indices: geometry.indices,
        radialSegments: geometry.radialSegments,
        indexCount: geometry.indexCount,
        ranges: geometry.ranges,
      },
      material: curveMaterial(options, { alpha: 0.9, depthWrite: true }),
      metadata: curveMetadata(geometry, options, {
        curveVisualization: "tube-path",
        drawMode: "triangles-indexed",
      }),
      bounds: geometry.bounds,
    };
  }

  const buffers = buildRibbonPathBuffers(evidence, options);
  return baseCurveDescriptor("CurveRibbonLayer", buffers, options, {
    curveVisualization: "ribbon-path",
    drawMode: "triangles",
    widthUnits: "screen-pixels",
  });
}

/**
 * Create a descriptor for trajectory bundles, route sets, solver histories, or
 * nearest-neighbor route families.
 *
 * @param {object|Array|ArrayBufferView} evidence
 * @param {object} [options]
 * @returns {object}
 */
export function createTrajectoryBundleLayerDescriptor(evidence, options = {}) {
  const buffers = options.mode === "polyline"
    ? buildPolylinePathBuffers(evidence, options)
    : buildRibbonPathBuffers(evidence, options);
  return baseCurveDescriptor(options.mode === "polyline" ? "CurvePolylineLayer" : "CurveRibbonLayer", buffers, {
    ...options,
    id: options.id || "curve-trajectory-bundle",
    order: options.order ?? 35,
  }, {
    curveVisualization: "trajectory-bundle",
    evidenceRole: options.evidenceRole || "trajectory-family",
    drawMode: options.mode === "polyline" ? "lines" : "triangles",
    widthUnits: options.mode === "polyline" ? "implementation-defined" : "screen-pixels",
  });
}

/**
 * Create a descriptor for ground-projected paths.
 *
 * @param {object|Array|ArrayBufferView} evidence
 * @param {object} [options]
 * @returns {object}
 */
export function createGroundProjectedPathLayerDescriptor(evidence, options = {}) {
  const buffers = buildGroundProjectedPathBuffers(evidence, options);
  const isRibbon = options.mode === "ribbon";
  return baseCurveDescriptor(isRibbon ? "CurveRibbonLayer" : "CurvePolylineLayer", buffers, {
    ...options,
    id: options.id || "curve-ground-projected-paths",
    order: options.order ?? 20,
  }, {
    curveVisualization: "ground-projected-path",
    drawMode: isRibbon ? "triangles" : "lines",
    projectedPlane: options.plane || (Number.isFinite(Number(options.groundZ)) ? "xy" : "xz"),
    widthUnits: isRibbon ? "screen-pixels" : "implementation-defined",
  });
}

function baseCurveDescriptor(kind, buffers, options, metadata) {
  return {
    id: options.id || defaultDescriptorId(metadata.curveVisualization),
    kind: options.kind || kind,
    primitive: options.primitive || kind,
    order: options.order ?? 30,
    visible: options.visible !== false,
    source: {
      evidence: buffers.evidence,
      segmentCount: buffers.segmentCount,
      vertexCount: buffers.vertexCount,
    },
    channels: buffers.channels,
    geometry: {
      mode: metadata.drawMode,
      widthUnits: metadata.widthUnits,
      segmentCount: buffers.segmentCount,
      vertexCount: buffers.vertexCount,
      joins: options.joins || "overlap",
      caps: options.caps || "butt",
    },
    material: curveMaterial(options, { alpha: 0.9, depthWrite: false }),
    metadata: curveMetadata(buffers, options, metadata),
    bounds: buffers.bounds,
  };
}

function curveMaterial(options, defaults) {
  const material = {
    alpha: options.alpha ?? defaults.alpha,
    transparent: options.transparent !== false,
    depthWrite: options.depthWrite ?? defaults.depthWrite,
    depthTest: options.depthTest !== false,
    widthScale: options.widthScale ?? 1,
    colorMix: options.colorMix ?? 1,
  };
  copyDefined(material, options, [
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
  ]);
  return material;
}

function copyDefined(target, source, keys) {
  for (const key of keys) {
    if (source[key] !== undefined) target[key] = source[key];
  }
}

function curveMetadata(buffers, options, metadata) {
  const diagnostics = buffers.evidence?.diagnostics || buffers.evidence?.metadata?.diagnostics || {};
  return {
    subsystem: "metric.visual.curves",
    schema: "metric.visual.curve_layer_descriptor.v1",
    ...metadata,
    evidenceTypes: [
      "time-series-curve",
      "transformation-path",
      "diffusion-trajectory",
      "morph-trace",
      "nearest-neighbor-route",
      "solver-history",
    ],
    algorithmicComputation: false,
    visualTessellationOnly: true,
    resampling: {
      enabled: Boolean(diagnostics.resampled || options.resample || options.maxSegmentLength || options.samples),
      evidenceMutation: false,
      maxSegmentLength: options.maxSegmentLength ?? null,
      samples: options.samples ?? null,
    },
    diagnostics,
  };
}

function defaultDescriptorId(name) {
  return `curve-${String(name || "paths").replace(/[^a-z0-9]+/gi, "-").toLowerCase()}`;
}
