#!/usr/bin/env node
import fs from "node:fs/promises";
import path from "node:path";
import { fileURLToPath } from "node:url";

import {
  createMiniatureHeroSceneBundle,
  createMiniatureHeroStage,
  createMiniaturePhotographicRigContract,
  createMiniatureRigReport,
  createMiniatureStyleMotionPreset,
  inspectMiniatureStyleContract,
  MINIATURE_CAMERA_DOF_FRAGMENT_SHADER,
  ProcessCurveSceneView,
} from "../src/index.js";

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "../..");

async function main() {
  const heroHtmlPath = path.join(root, "visual/examples/miniature-hero-frame/index.html");
  const heroHtml = await fs.readFile(heroHtmlPath, "utf8");
  assertHeroHtmlContract(heroHtml);
  assertCameraDofShaderContract(MINIATURE_CAMERA_DOF_FRAGMENT_SHADER);

  const evidencePath = path.join(root, "docs/examples/assets/process-curve-external/metric.visual.json");
  const document = JSON.parse(await fs.readFile(evidencePath, "utf8"));
  const stage = createMiniatureHeroStage();
  const view = ProcessCurveSceneView.fromVisualSpace(document, {
    datasetId: "process-curve-external",
    targetCoordinateId: "process-curve-external-landmark-3d",
    labelPropertyId: "process-role",
    relationId: "process-curve-external-aligned-metric",
    graphId: "process-curve-external-knn",
    stage,
    groundY: stage.grounding?.groundY,
    morphLoop: true,
    includeNeighborhood: true,
    includeMatrix: true,
  });
  const bundle = createMiniatureHeroSceneBundle(view.toLayerDescriptors(), {
    stage,
    fit: { targetSpan: 2.74 },
    ground: { groundY: stage.grounding?.groundY, padding: 0.66 },
    contacts: { includeSamples: false },
    motion: "miniature",
    metadata: {
      verifier: "visual/tools/verify-miniature-rig.mjs",
    },
  });
  const styleMotion = {
    enabled: true,
    motion: createMiniatureStyleMotionPreset("studio-drift"),
    hasRuntime: true,
    hasBeforeRenderHook: true,
  };
  const rigContract = createMiniaturePhotographicRigContract({
    stage,
    bundle,
    styleMotion,
  });
  const diagnostics = inspectMiniatureStyleContract({
    stage,
    bundle,
    styleMotion,
  });
  const report = createMiniatureRigReport({ stage, bundle, rigContract });

  assert(rigContract.ok, "photographic rig contract must be ready", rigContract.missing);
  assert(rigContract.sections.depthOfField.evidence.model === "camera-depth-circle-of-confusion", "DoF contract must use camera-depth circle-of-confusion", rigContract.sections.depthOfField);
  assert(rigContract.sections.depthOfField.evidence.depthMode === "raw-camera-depth", "DoF contract must require raw camera depth", rigContract.sections.depthOfField);
  assert(rigContract.sections.depthOfField.evidence.usesCameraNearFar === true, "DoF contract must reconstruct camera distance from near/far", rigContract.sections.depthOfField);
  assert(diagnostics.checks.morphRoom.ok, "same-room morph diagnostic must be ready", diagnostics.checks.morphRoom);
  assert(report.ready, "miniature rig report must be ready", report.missing);
  assert(report.camera.projection === "perspective", "miniature report must expose perspective camera", report.camera);
  assert(report.camera.coordinateSystem === "metric-xz-ground", "miniature report must expose metric x/z ground camera", report.camera);
  assert(report.camera.isOblique === true, "miniature report must expose oblique camera", report.camera);
  assert(report.light.lightCount >= 1, "miniature report must expose scene light rig", report.light);
  assert(report.ground.hasGroundLayer === true, "miniature report must expose ground layer", report.ground);
  assert(report.ground.hasProjectionLayer === true, "miniature report must expose projection/contact layer", report.ground);
  assert(report.materials.styledDescriptorCount === report.materials.descriptorCount, "all report descriptors must carry miniature material styling", report.materials);
  assert(report.materials.litDescriptorCount > 0, "report must expose lit material response", report.materials);
  assertMiniatureLabelContract(bundle);
  assert(report.motion.enabled === true, "miniature report must expose style or descriptor motion", report.motion);
  assert(report.motion.activeDomains?.focus === true, "miniature report must expose focus motion domain", report.motion);
  assert(report.motion.activeDomains?.camera === true, "miniature report must expose camera motion domain", report.motion);
  assert(report.motion.activeDomains?.postprocess === true, "miniature report must expose postprocess motion domain", report.motion);
  assert(report.motion.hasRuntime === true, "miniature report must expose motion runtime attachment", report.motion);
  assert(report.motion.hasBeforeRenderHook === true, "miniature report must expose motion before-render hook", report.motion);
  assert(report.morphRoom.morphLayerCount >= 1, "miniature report must expose same-room morph layers", report.morphRoom);

  const morphLayer = bundle.layers.find((layer) => layer.kind === "morph");
  assert(morphLayer, "process-curve hero must contain a morph layer");
  const source = endpointStats(morphLayer.channels.position);
  const target = endpointStats(morphLayer.channels.targetPosition);
  const flatY = bundle.fit?.sameRoomMorph?.flatY;
  assert(Number.isFinite(flatY), "bundle fit must expose sameRoomMorph.flatY", bundle.fit?.sameRoomMorph);
  assert(Math.abs(source.minY - flatY) < 1e-5 && Math.abs(source.maxY - flatY) < 1e-5, "2D morph endpoint must be staged on flatY", source);
  assert(target.maxY - target.minY > 0.5, "3D morph endpoint must keep y structure", target);
  assert(morphLayer.channels.position.metadata?.miniatureSameRoomMorph, "flat endpoint must carry same-room channel metadata");

  const result = {
    status: "ok",
    schema: "metric.visual.miniature_rig_verification.v1",
    rigStatus: rigContract.status,
    layerCount: bundle.layers.length,
    sameRoomMorph: bundle.fit.sameRoomMorph,
    morphRoom: rigContract.sections.morphRoom.evidence,
    depthOfField: rigContract.sections.depthOfField.evidence,
    camera: report.camera,
    light: report.light,
    ground: report.ground,
    materials: report.materials,
    motion: report.motion,
    sourceY: source,
    targetY: target,
  };
  console.log(JSON.stringify(result, null, 2));
}

function assertMiniatureLabelContract(bundle) {
  const labelLayer = bundle.layers.find((layer) => layer?.primitive === "BillboardLabelLayer");
  assert(labelLayer, "miniature rig fixture must contain in-scene labels");
  assert(labelLayer.metadata?.miniature === true, "label descriptors must carry miniature styling", labelLayer.metadata);
  assert(labelLayer.metadata?.miniatureRole === "miniature-label", "label descriptors must use the miniature label role", labelLayer.metadata);
  assert(Number(labelLayer.metadata?.labelCount || labelLayer.labels?.length || 0) > 0, "label descriptors must expose label count", labelLayer.metadata);
}

function assertHeroHtmlContract(html) {
  assert(html.includes("body.metric-inspect main:hover .caption"), "hero overlays must be inspect-mode gated");
  assert(html.includes("params.get(\"inspect\") === \"1\""), "hero inspect mode must be query-param gated");
  assert(!/main:hover\\s+\\.caption/.test(html.replaceAll("body.metric-inspect main:hover .caption", "")), "hero caption must not appear on plain hover");
  assert(!/main:hover\\s+\\.status/.test(html.replaceAll("body.metric-inspect main:hover .status", "")), "hero status must not appear on plain hover");
}

function assertCameraDofShaderContract(shader) {
  assert(typeof shader === "string" && shader.length > 0, "camera DoF shader source must be exported");
  assert(shader.includes("rawDepthToCameraDistance"), "camera DoF shader must reconstruct raw depth to camera distance");
  assert(shader.includes("circleOfConfusion"), "camera DoF shader must compute circle-of-confusion");
  assert(shader.includes("uFocusDistance"), "camera DoF shader must consume focus distance");
  assert(shader.includes("uAperture"), "camera DoF shader must consume aperture");
  assert(shader.includes("uCameraNear") && shader.includes("uCameraFar"), "camera DoF shader must consume camera near/far planes");
  assert(!shader.includes("vUv.y") || shader.includes("texture2D(uDepthTexture"), "camera DoF shader must not be a screen-space y-gradient blur");
}

function endpointStats(channel) {
  const array = channel?.array;
  const itemSize = Number(channel?.itemSize || channel?.size || 3);
  assert(array && itemSize >= 3, "morph endpoint must expose 3D channel data", { channel });
  let minY = Infinity;
  let maxY = -Infinity;
  for (let index = 1; index < array.length; index += itemSize) {
    const y = Number(array[index]);
    if (!Number.isFinite(y)) continue;
    minY = Math.min(minY, y);
    maxY = Math.max(maxY, y);
  }
  assert(Number.isFinite(minY) && Number.isFinite(maxY), "morph endpoint y-range must be finite", { channel });
  return {
    minY,
    maxY,
    spanY: maxY - minY,
  };
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
