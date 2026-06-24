#!/usr/bin/env node
import fs from "node:fs/promises";
import path from "node:path";
import { fileURLToPath } from "node:url";

import {
  adaptMetricEvidenceV1,
  createMiniatureLookSceneAtlas,
  createMiniaturePhotographicRigContract,
  createMiniatureRigReport,
  createMiniatureStyleMotionAtlas,
  createProcessCurveMiniatureLayerDescriptors,
  miniatureRigReportDataset,
} from "../src/index.js";

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "../..");

async function main() {
  const galleryHtml = await fs.readFile(path.join(root, "visual/examples/miniature-look-gallery/index.html"), "utf8");
  assertGalleryHtmlContract(galleryHtml);

  const evidence = JSON.parse(await fs.readFile(path.join(root, "docs/examples/assets/process-curve-external/evidence.json"), "utf8"));
  const document = adaptMetricEvidenceV1(evidence);
  const sceneInput = createProcessCurveMiniatureLayerDescriptors(document, {
    datasetId: "power_demand",
    morphLoop: true,
    pointSize: 1.72,
    projectionSize: 2.7,
    trackMode: "tube",
    trackRadius: 0.012,
    trackRadialSegments: 8,
    trackAlpha: 0.9,
  });
  const atlas = createMiniatureLookSceneAtlas(sceneInput.descriptors, {
    contacts: { includeSamples: false },
    fit: { targetSpan: 2.58 },
    metadata: {
      verifier: "visual/tools/verify-miniature-look-atlas.mjs",
      datasetId: "power_demand",
    },
  });
  const motionAtlas = createMiniatureStyleMotionAtlas();

  assert(atlas.summary.entryCount >= 6, "look scene atlas must cover multiple reference looks", atlas.summary);
  assert(atlas.summary.readyCount === atlas.summary.entryCount, "all look scene atlas entries must be style-ready", atlas.summary);
  assert(motionAtlas.summary.entryCount >= 4, "style motion atlas must expose multiple motion modes", motionAtlas.summary);
  assert(motionAtlas.summary.focusCount > 0, "style motion atlas must include focus motion", motionAtlas.summary);
  assert(motionAtlas.summary.cameraCount > 0, "style motion atlas must include camera motion", motionAtlas.summary);
  assert(motionAtlas.summary.postprocessCount > 0, "style motion atlas must include postprocess motion", motionAtlas.summary);

  const entries = atlas.entries.map((entry) => verifyLookEntry(entry));
  const result = {
    status: "ok",
    schema: "metric.visual.miniature_look_atlas_verification.v1",
    lookCount: entries.length,
    readyCount: entries.filter((entry) => entry.ready).length,
    looks: entries,
    motionSummary: motionAtlas.summary,
  };
  console.log(JSON.stringify(result, null, 2));
}

function verifyLookEntry(entry) {
  const styleMotion = {
    enabled: true,
    motion: entry.styleMotion,
    hasRuntime: true,
    hasBeforeRenderHook: true,
  };
  const rigContract = createMiniaturePhotographicRigContract({
    stage: entry.stage,
    bundle: entry.bundle,
    descriptors: entry.bundle?.layers || [],
    styleMotion,
  });
  const report = createMiniatureRigReport({
    stage: entry.stage,
    bundle: entry.bundle,
    descriptors: entry.bundle?.layers || [],
    rigContract,
    styleMotion,
  });
  const dataset = miniatureRigReportDataset(report);

  assert(rigContract.ok, `${entry.look}: photographic rig contract must be ready`, rigContract.missing);
  assert(report.ready, `${entry.look}: rig report must be ready`, report.missing);
  assert(report.camera.projection === "perspective", `${entry.look}: perspective camera required`, report.camera);
  assert(report.camera.coordinateSystem === "metric-xz-ground", `${entry.look}: metric x/z ground camera required`, report.camera);
  assert(report.camera.isOblique === true, `${entry.look}: oblique miniature camera required`, report.camera);
  assert(report.light.lightCount >= 1, `${entry.look}: scene light rig required`, report.light);
  assert(report.ground.hasGroundLayer === true, `${entry.look}: ground layer required`, report.ground);
  assert(report.ground.hasProjectionLayer === true, `${entry.look}: projection/contact layer required`, report.ground);
  assert(report.materials.styledDescriptorCount === report.materials.descriptorCount, `${entry.look}: all descriptors must be miniature-styled`, report.materials);
  assert(report.materials.litDescriptorCount > 0, `${entry.look}: lit material response required`, report.materials);
  assert(report.motion.enabled === true, `${entry.look}: style or descriptor motion required`, report.motion);
  assert(report.motion.mode && report.motion.mode !== "none", `${entry.look}: named motion mode required`, report.motion);
  assert(report.morphRoom.morphLayerCount >= 1, `${entry.look}: same-room morph layer required`, report.morphRoom);
  assert(Number.isFinite(Number(dataset.metricCameraFov)), `${entry.look}: dataset camera FOV required`, dataset);
  assert(dataset.metricMaterialFamilies.includes("sample"), `${entry.look}: dataset material families required`, dataset);
  assert(dataset.metricMotionMode === report.motion.mode, `${entry.look}: dataset motion mode must match report`, dataset);
  assert(dataset.metricMiniatureRigReady === "true", `${entry.look}: dataset ready flag required`, dataset);

  return {
    look: entry.look,
    ready: report.ready,
    variant: entry.variant,
    camera: {
      fov: report.camera.fov,
      coordinateSystem: report.camera.coordinateSystem,
      oblique: report.camera.isOblique,
    },
    light: {
      mode: report.light.mode,
      count: report.light.lightCount,
      points: report.light.pointCount,
    },
    ground: {
      groundY: report.ground.groundY,
      projection: report.ground.hasProjectionLayer,
      contactAlpha: report.ground.contactAlpha,
    },
    materials: {
      families: report.materials.families,
      styled: report.materials.styledDescriptorCount,
      lit: report.materials.litDescriptorCount,
    },
    motion: {
      mode: report.motion.mode,
      domains: report.motion.activeDomains,
    },
    morphRoom: {
      layers: report.morphRoom.morphLayerCount,
      flatY: report.morphRoom.sameRoomMorph?.flatY ?? null,
    },
  };
}

function assertGalleryHtmlContract(html) {
  assert(html.includes("attachMiniatureRigReporter"), "look gallery must attach per-panel miniature rig reporters");
  assert(html.includes("target: panel.root"), "look gallery rig reports must be panel-scoped");
  assert(!html.includes("metric-show-ui"), "look gallery must not require debug UI to publish rig reports");
}

function assert(condition, message, details) {
  if (condition) return;
  const error = new Error(message);
  error.details = details;
  throw error;
}

main().catch((error) => {
  console.error(JSON.stringify({
    status: "error",
    message: error.message,
    details: error.details || null,
  }, null, 2));
  process.exitCode = 1;
});
