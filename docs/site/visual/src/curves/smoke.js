import {
  buildRibbonPathBuffers,
  buildTubePathGeometry,
  createGroundProjectedPathLayerDescriptor,
  createPolylinePathLayerDescriptor,
  createTrajectoryBundleLayerDescriptor,
  createTubeRibbonPathLayerDescriptor,
  normalizePathEvidence,
} from "./index.js";

const evidence = {
  trajectories: [
    {
      id: "solver-a",
      points: [
        { x: 0, y: 0, z: 0, time: 0, width: 2, color: [0.12, 0.48, 0.68, 1] },
        { x: 1, y: 0.2, z: 0.1, time: 1, width: 4, color: [0.22, 0.62, 0.56, 0.9] },
        { x: 2, y: 0.8, z: 0.4, time: 2, width: 3, color: [0.92, 0.58, 0.24, 0.85] },
      ],
    },
    {
      id: "solver-b",
      positions: new Float32Array([0, 0.4, 0, 0.7, 0.9, 0.2, 1.8, 1.1, 0.6]),
      times: new Float32Array([0, 1, 2]),
      width: 2.5,
    },
  ],
};

const normalized = normalizePathEvidence(evidence, { resample: true, maxSegmentLength: 0.35 });
const ribbonBuffers = buildRibbonPathBuffers(normalized);
const tubeGeometry = buildTubePathGeometry(normalized, { radius: 0.04, radialSegments: 6 });
const descriptors = [
  createPolylinePathLayerDescriptor(normalized),
  createTubeRibbonPathLayerDescriptor(normalized),
  createTubeRibbonPathLayerDescriptor(normalized, { mode: "tube" }),
  createTrajectoryBundleLayerDescriptor(evidence),
  createGroundProjectedPathLayerDescriptor(evidence, { plane: "xz", groundY: 0 }),
];

if (normalized.paths.length !== 2) throw new Error("Expected two normalized paths.");
if (ribbonBuffers.vertexCount <= 0) throw new Error("Expected ribbon vertices.");
if (tubeGeometry.indexCount <= 0) throw new Error("Expected tube indices.");
if (tubeGeometry.distances.length !== tubeGeometry.vertexCount) throw new Error("Expected tube path distances.");
if (!descriptors.some((descriptor) => descriptor.primitive === "CurveTubeMeshLayer")) {
  throw new Error("Expected tube descriptor to use CurveTubeMeshLayer.");
}
if (!descriptors.every((descriptor) => descriptor.metadata?.algorithmicComputation === false)) {
  throw new Error("Curve descriptors must not claim algorithmic computation.");
}

console.log("curve smoke ok", {
  paths: normalized.paths.length,
  ribbonVertices: ribbonBuffers.vertexCount,
  tubeVertices: tubeGeometry.vertexCount,
  descriptors: descriptors.map((descriptor) => descriptor.kind),
});
