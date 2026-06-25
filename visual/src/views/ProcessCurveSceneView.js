import { createTubeRibbonPathLayerDescriptor } from "../curves/index.js";
import {
  createMiniatureAnimationPreset,
  createMiniatureSceneBundle,
  createMiniatureStagePreset,
} from "../style/miniature/index.js";
import { DenseFieldView } from "./DenseFieldView.js";
import { GroundProjectionView } from "./GroundProjectionView.js";
import { MorphView } from "./MorphView.js";
import { createChannel } from "./view-utils.js";

export function resolveProcessCurveSceneInputs(document, options = {}) {
  const datasetId = options.datasetId || firstDatasetId(document);
  const coordinates = document?.coordinates || [];
  const properties = document?.properties || [];
  const coordinateName = (entry) => String(entry?.name || entry?.id || "").toLowerCase();
  let sourceCoordinate = resolveByIdOrPredicate(
    coordinates,
    options.sourceCoordinateId || options.sourceCoordinate,
    (entry) => entry.dataset_id === datasetId && coordinateName(entry).includes(options.sourceCoordinateName || "landmark2"),
  );
  const targetCoordinate = resolveByIdOrPredicate(
    coordinates,
    options.targetCoordinateId || options.targetCoordinate,
    (entry) => entry.dataset_id === datasetId && coordinateName(entry).includes(options.targetCoordinateName || "landmark3"),
  );
  const labelProperty = resolveByIdOrPredicate(
    properties,
    options.labelPropertyId || options.propertyId || options.labelProperty,
    (entry) => entry.dataset_id === datasetId && String(entry.id || "").endsWith(":record_label"),
  );
  if (!sourceCoordinate) sourceCoordinate = targetCoordinate;
  if (!datasetId) throw new Error("Process-curve scene requires a dataset id.");
  if (!targetCoordinate) throw new Error(`Process-curve scene could not find target coordinate for ${datasetId}.`);
  if (!labelProperty) throw new Error(`Process-curve scene could not find label property for ${datasetId}.`);
  return {
    datasetId,
    sourceCoordinate,
    targetCoordinate,
    labelProperty,
    records: (document?.records || []).filter((record) => record.dataset_id === datasetId),
  };
}

export function createProcessCurveMiniatureStage(options = {}) {
  const materials = options.materials || {};
  const grounding = options.grounding || {};
  const postprocess = options.postprocess || {};
  return createMiniatureStagePreset({
    look: options.look,
    ...((options.variant || !options.look) ? { variant: options.variant || "white" } : {}),
    camera: {
      fov: 36,
      position: [0, -3.25, 1.72],
      target: [0, -0.1, 0.02],
      ...(options.camera || {}),
    },
    focus: {
      axis: "horizontal",
      yRatio: 0.48,
      radiusRatio: 0.32,
      focusBandRatio: 0.07,
      blurRadius: 5.4,
      blurCurve: 1.42,
      ...(options.focus || {}),
    },
    materials: {
      ...materials,
      mesh: {
        ambient: 0.82,
        pointLight: 0.92,
        specular: 0.2,
        ambientColor: [0.96, 0.97, 0.92],
        ...(materials.mesh || {}),
      },
    },
    grounding: {
      ...grounding,
      groundY: finiteNumber(options.groundY, -0.56),
      ground: {
        gridScale: 6,
        alpha: 0.42,
        fade: 0.9,
        gridAlpha: 0.28,
        axisAlpha: 0.24,
        stageTilt: -0.12,
        stageScale: [0.94, 0.76],
        sheen: 0.06,
        matte: 0.82,
        contactShade: 0.16,
        horizonFade: 0.12,
        ...(grounding.ground || {}),
      },
      projection: {
        alpha: 0.52,
        softness: 0.8,
        footprintStretch: 2.05,
        footprintSkew: -0.3,
        footprintCore: 0.16,
        footprintFalloff: 1.14,
        colorMix: 0.68,
        shadowDensity: 1.26,
        coreDensity: 0.42,
        edgeTint: 0.54,
        shadowDirection: [-0.44, 0.9],
        shadowTail: 0.6,
        shadowTailStrength: 0.3,
        contactHardness: 0.34,
        surfaceLift: 0.02,
        pointPixelScale: 18,
        ...(grounding.projection || {}),
      },
      contact: {
        alpha: 0.46,
        pointPixelScale: 26,
        radiusScale: 1.34,
        heightScale: 0.32,
        footprintStretch: 2.22,
        footprintSkew: -0.36,
        colorMix: 0.52,
        shadowDensity: 1.48,
        coreDensity: 0.52,
        edgeTint: 0.46,
        shadowDirection: [-0.44, 0.9],
        shadowTail: 0.68,
        shadowTailStrength: 0.38,
        contactHardness: 0.44,
        surfaceLift: 0.018,
        ...(grounding.contact || {}),
      },
    },
    postprocess: {
      ...postprocess,
      postFx: {
        grade: {
          enabled: true,
          contrast: 1.04,
          saturation: 0.96,
          highlightCompression: 0.12,
        },
        vignette: {
          enabled: true,
          amount: 0.08,
          radius: 0.84,
          softness: 0.72,
          color: [0.86, 0.88, 0.84],
        },
        bloom: {
          enabled: true,
          intensity: 0.1,
          threshold: 0.94,
          knee: 0.16,
          radius: 1.0,
          iterations: 3,
          downsample: 2,
          saturation: 0.72,
          warmth: 0.08,
          clamp: 0.78,
        },
        filmGrain: { enabled: false },
        ...(postprocess.postFx || {}),
      },
    },
    ...(options.stage || {}),
  });
}

export function createProcessCurveMiniatureLayerDescriptors(document, options = {}) {
  const inputs = resolveProcessCurveSceneInputs(document, options);
  const groundY = finiteNumber(options.groundY, options.stage?.grounding?.groundY, -0.56);
  const morph = createProcessCurveMorphDescriptor(document, inputs, options);
  const projection = createProcessCurveProjectionDescriptor(document, inputs, { ...options, groundY: groundY + 0.04 });
  const skyline = createProcessCurveSkylineDescriptor(document, inputs, { ...options, groundY });
  const field = createProcessCurveStateFieldDescriptor(document, inputs, { ...options, groundY: groundY + 0.03 });
  const track = createProcessCurveTrackDescriptor(document, inputs, { ...options, groundY: groundY + 0.06 });
  return {
    inputs,
    descriptors: [field, projection, track, skyline, morph].filter(Boolean),
  };
}

export function createProcessCurveMiniatureSceneBundle(document, options = {}) {
  const stage = options.stage || createProcessCurveMiniatureStage(options);
  const { inputs, descriptors } = createProcessCurveMiniatureLayerDescriptors(document, {
    ...options,
    stage,
    groundY: stage.grounding?.groundY,
  });
  const bundle = createMiniatureSceneBundle(descriptors, {
    stage,
    fit: {
      targetSpan: 1.95,
      ...(options.fit || {}),
    },
    ground: {
      groundY: stage.grounding?.groundY,
      padding: 0.48,
      ...(options.ground || {}),
    },
    motion: options.motion ?? false,
    contacts: options.contacts,
  });
  return { inputs, stage, descriptors, bundle };
}

function createProcessCurveMorphDescriptor(document, inputs, options) {
  const loop = options.morphLoop === true;
  const hasProgress = Number.isFinite(Number(options.morphProgress));
  const progress = hasProgress ? Math.max(0, Math.min(1, Number(options.morphProgress))) : 1;
  const animation = createMiniatureAnimationPreset("coordinate-morph", {
    loop,
    progress,
    direction: "alternate",
    durationMs: finiteNumber(options.morphDurationMs, 4800),
    easing: "smoothstep",
  });
  if (loop && !hasProgress) delete animation.progress;
  const view = MorphView.fromVisualSpace(document, {
    datasetId: inputs.datasetId,
    sourceCoordinateId: inputs.sourceCoordinate.id,
    targetCoordinateId: inputs.targetCoordinate.id,
    colorPropertyId: inputs.labelProperty.id,
    size: finiteNumber(options.pointSize, 1.7),
    alpha: finiteNumber(options.pointAlpha, 0.72),
    shape: "disc",
    material: {
      pointPixelScale: 12,
      alphaMode: "blend",
      transparent: true,
      alpha: 0.72,
      focusBoost: 0.1,
      phasePulse: 0.04,
      ...(options.pointMaterial || {}),
    },
    animation,
  });
  return view.toLayerDescriptors()[0];
}

function createProcessCurveProjectionDescriptor(document, inputs, options) {
  const view = GroundProjectionView.fromVisualSpace(document, {
    datasetId: inputs.datasetId,
    coordinateId: inputs.targetCoordinate.id,
    propertyId: inputs.labelProperty.id,
    groundY: finiteNumber(options.groundY, -0.52),
    size: finiteNumber(options.projectionSize, 2.65),
    alpha: finiteNumber(options.projectionAlpha, 0.34),
    material: {
      transparent: true,
      ...(options.projectionMaterial || {}),
    },
  });
  const descriptor = view.toLayerDescriptors()[0];
  descriptor.order = -28;
  return descriptor;
}

function createProcessCurveStateFieldDescriptor(document, inputs, options) {
  const records = inputs.records;
  const positionsByRecord = new Map((inputs.targetCoordinate.record_positions || []).map((entry) => [entry.record_id, entry.position || []]));
  const labelByRecord = new Map((inputs.labelProperty.values || []).map((entry) => [entry.record_id, entry.value]));
  const positions = new Map();
  const values = new Map();
  const recordIds = [];
  const groundY = finiteNumber(options.groundY, -0.53);

  for (const record of records) {
    const position = positionsByRecord.get(record.id);
    if (!position) continue;
    recordIds.push(record.id);
    positions.set(record.id, [
      Number(position[0]) || 0,
      groundY,
      Number(position[2] ?? position[1]) || 0,
    ]);
    values.set(record.id, labelValue(labelByRecord.get(record.id) ?? record.label));
  }

  const view = new DenseFieldView({
    id: `field:${inputs.datasetId}:state-density`,
    name: "state-density-field",
    datasetId: inputs.datasetId,
    coordinateId: inputs.targetCoordinate.id,
    propertyId: inputs.labelProperty.id,
    recordIds,
    positions,
    values,
    radius: finiteNumber(options.fieldRadius, 1.85),
    alpha: finiteNumber(options.fieldAlpha, 0.54),
    material: {
      alpha: 0.72,
      pointPixelScale: 54,
      maxPointSize: 440,
      contour: 0.22,
      glow: 0.2,
      depthWrite: false,
      ...(options.fieldMaterial || {}),
    },
    metadata: {
      role: "semantic-ground-field",
      visualizes: "record-state intensity over metric coordinates",
    },
  });
  const descriptor = view.toLayerDescriptors()[0];
  descriptor.order = -22;
  return descriptor;
}

function createProcessCurveTrackDescriptor(document, inputs, options) {
  const records = inputs.records
    .slice()
    .sort((a, b) => Number(a.payload?.source_index ?? 0) - Number(b.payload?.source_index ?? 0));
  const positions = new Map((inputs.targetCoordinate.record_positions || []).map((entry) => [entry.record_id, entry.position || []]));
  const labels = new Map((inputs.labelProperty.values || []).map((entry) => [entry.record_id, entry.value]));
  const categories = new Map();
  const pathsByLabel = new Map();
  const groundY = finiteNumber(options.groundY, -0.5);

  for (const record of records) {
    const position = positions.get(record.id);
    if (!position) continue;
    const label = labels.get(record.id) ?? record.label ?? record.id;
    if (!categories.has(label)) categories.set(label, categories.size);
    if (!pathsByLabel.has(label)) {
      pathsByLabel.set(label, {
        id: `track:${inputs.datasetId}:${label}`,
        label,
        color: trackColor(categories.get(label)),
        points: [],
      });
    }
    pathsByLabel.get(label).points.push([
      Number(position[0]) || 0,
      groundY,
      Number(position[2] ?? position[1]) || 0,
    ]);
  }

  return createTubeRibbonPathLayerDescriptor(Array.from(pathsByLabel.values()), {
    id: `track:${inputs.datasetId}:record-order`,
    kind: "record-track",
    order: -8,
    mode: options.trackMode || "ribbon",
    radius: finiteNumber(options.trackRadius, 0.018),
    radialSegments: Math.max(3, Math.floor(finiteNumber(options.trackRadialSegments, 8))),
    useWidthsAsRadius: options.trackUseWidthsAsRadius === true,
    width: finiteNumber(options.trackWidth, 3.15),
    alpha: finiteNumber(options.trackAlpha, 0.72),
    emission: finiteNumber(options.trackEmission, 0.14),
    rimLight: finiteNumber(options.trackRimLight, 0.36),
    coreGlow: finiteNumber(options.trackCoreGlow, 0.2),
    flowStrength: finiteNumber(options.trackFlowStrength, 0.1),
    depthWrite: options.trackMode === "tube" ? true : false,
    resample: false,
  });
}

function createProcessCurveSkylineDescriptor(document, inputs, options) {
  const records = inputs.records
    .slice()
    .sort((a, b) => Number(a.payload?.source_index ?? 0) - Number(b.payload?.source_index ?? 0));
  const positionsByRecord = new Map((inputs.targetCoordinate.record_positions || []).map((entry) => [entry.record_id, entry.position || []]));
  const labelByRecord = new Map((inputs.labelProperty.values || []).map((entry) => [entry.record_id, entry.value]));
  const entries = [];

  for (const record of records) {
    const position = positionsByRecord.get(record.id);
    const values = curveValues(record);
    if (!position || values.length < 2) continue;
    entries.push({
      id: record.id,
      label: labelByRecord.get(record.id) ?? record.label,
      position,
      energy: curveEnergy(values),
      amplitude: curveAmplitude(values),
    });
  }

  const energyRange = extent(entries.map((entry) => entry.energy));
  const amplitudeRange = extent(entries.map((entry) => entry.amplitude));
  const positions = new Float32Array(entries.length * 3);
  const sizes = new Float32Array(entries.length * 2);
  const heights = new Float32Array(entries.length);
  const colors = new Float32Array(entries.length * 4);
  const phases = new Float32Array(entries.length);
  const focus = new Float32Array(entries.length);
  const groundY = finiteNumber(options.groundY, -0.56);

  for (let index = 0; index < entries.length; index += 1) {
    const entry = entries[index];
    const x = Number(entry.position[0]) || 0;
    const z = Number(entry.position[2] ?? entry.position[1]) || 0;
    const energy = normalize(entry.energy, energyRange);
    const amplitude = normalize(entry.amplitude, amplitudeRange);
    const height = 0.09 + energy * 0.44 + amplitude * 0.22;
    const footprint = 0.038 + Math.sqrt(Math.max(0, amplitude)) * 0.045;
    const color = stateColor(entry.label, 0.96);

    positions.set([x, groundY, z], index * 3);
    sizes.set([footprint, footprint * 1.15], index * 2);
    heights[index] = height;
    colors.set(color, index * 4);
    phases[index] = (index % 11) / 11;
    focus[index] = String(entry.label || "").toLowerCase().includes("anomaly") ? 1 : 0.22;
  }

  return {
    schema: "metric.visual.layer_descriptor.v1",
    id: `skyline:${inputs.datasetId}:curve-energy`,
    kind: "record-skyline",
    primitive: "InstancedBoxLayer",
    visible: true,
    order: -4,
    channels: {
      position: createChannel(positions, 3, "record-position"),
      size: createChannel(sizes, 2, "record-footprint"),
      height: createChannel(heights, 1, "curve-energy-height"),
      color: createChannel(colors, 4, "state-color"),
      animationPhase: createChannel(phases, 1, "animation-phase"),
      focusWeight: createChannel(focus, 1, "state-focus"),
    },
    geometry: {
      anchor: "base",
      instanceCount: entries.length,
    },
    material: {
      family: "mesh",
      lighting: "phong-like",
      alpha: 1,
      alphaMode: "opaque",
      depthWrite: true,
    },
    animation: {
      mode: "attention",
    },
    picking: {
      mode: "record-id",
    },
    metadata: {
      role: "miniature-record-volume",
      visualizes: "record curve energy as grounded miniature volume",
      source: "process-curve C++ evidence",
    },
  };
}

function firstDatasetId(document) {
  const dataset = document?.datasets?.[0]?.id;
  if (dataset) return dataset;
  return document?.records?.find((record) => record.dataset_id)?.dataset_id || null;
}

function resolveByIdOrPredicate(items, id, predicate) {
  if (id && typeof id === "object") return id;
  if (id) {
    const byId = items.find((entry) => entry.id === id || entry.name === id);
    if (byId) return byId;
  }
  return items.find(predicate);
}

function trackColor(index) {
  const colors = [
    [0.12, 0.44, 0.66, 0.72],
    [0.82, 0.42, 0.14, 0.72],
    [0.16, 0.54, 0.48, 0.68],
    [0.52, 0.42, 0.62, 0.68],
  ];
  return colors[Math.abs(index) % colors.length];
}

function labelValue(label) {
  const normalized = String(label || "").toLowerCase();
  if (normalized.includes("anomaly")) return 0.96;
  if (normalized.includes("recovery")) return 0.68;
  if (normalized.includes("pre")) return 0.48;
  return 0.16;
}

function curveEnergy(values) {
  let total = 0;
  for (let index = 1; index < values.length; index += 1) {
    const delta = Number(values[index]) - Number(values[index - 1]);
    if (Number.isFinite(delta)) total += delta * delta;
  }
  return Math.sqrt(total / Math.max(1, values.length - 1));
}

function curveValues(record) {
  if (Array.isArray(record?.payload?.values)) return record.payload.values;
  if (Array.isArray(record?.payload?.series)) return record.payload.series;
  if (Array.isArray(record?.values)) return record.values;
  return [];
}

function curveAmplitude(values) {
  let min = Infinity;
  let max = -Infinity;
  for (const value of values) {
    const number = Number(value);
    if (!Number.isFinite(number)) continue;
    min = Math.min(min, number);
    max = Math.max(max, number);
  }
  return Number.isFinite(min) && Number.isFinite(max) ? max - min : 0;
}

function extent(values) {
  let min = Infinity;
  let max = -Infinity;
  for (const value of values) {
    const number = Number(value);
    if (!Number.isFinite(number)) continue;
    min = Math.min(min, number);
    max = Math.max(max, number);
  }
  return Number.isFinite(min) && Number.isFinite(max) ? [min, max] : [0, 1];
}

function normalize(value, range) {
  const span = Math.max(1e-9, Number(range?.[1]) - Number(range?.[0]));
  return Math.max(0, Math.min(1, (Number(value) - Number(range?.[0] || 0)) / span));
}

function stateColor(label, alpha = 1) {
  const normalized = String(label || "").toLowerCase();
  if (normalized.includes("anomaly")) return [0.85, 0.38, 0.14, alpha];
  if (normalized.includes("recovery")) return [0.42, 0.36, 0.65, alpha];
  if (normalized.includes("pre")) return [0.22, 0.58, 0.46, alpha];
  return [0.18, 0.48, 0.67, alpha];
}

function finiteNumber(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number)) return number;
  }
  return 0;
}
