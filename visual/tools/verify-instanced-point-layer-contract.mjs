#!/usr/bin/env node
import fs from "node:fs/promises";
import path from "node:path";
import { fileURLToPath } from "node:url";

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "../..");
const layerPath = path.join(root, "visual/src/layers/InstancedPointLayer.js");

async function main() {
  const source = await fs.readFile(layerPath, "utf8");

  assert(source.includes("uniform float uMorph"), "InstancedPointLayer must expose uMorph in the vertex shader");
  assert(source.includes('setUniform("uMorph"'), "InstancedPointLayer must upload uMorph each render");
  assert(source.includes("attribute vec3 aTargetPosition"), "InstancedPointLayer must expose aTargetPosition in the vertex shader");
  assert(source.includes('"aTargetPosition"'), "InstancedPointLayer must bind aTargetPosition");
  assert(source.includes("mix(aPosition, aTargetPosition"), "InstancedPointLayer must morph source to target positions on GPU");

  assert(source.includes("discard"), "point sprite shader must use fragment discard for a clean depth silhouette");
  assert(source.includes("function resolvePointDepthWrite"), "InstancedPointLayer must centralize point depth-write policy");
  assert(source.includes('material.depthWrite === true') && source.includes('material.alphaMode === "opaque"'), "opaque or explicit depthWrite material paths must write depth");
  assert(source.includes('material.alphaMode === "blend"') && source.includes("transparent === true"), "blend/transparent material paths must be controlled separately");
  assert(source.includes("configureDrawState(gl, drawMaterial") && source.includes("depthWrite,"), "draw state must receive resolved depthWrite");

  assertNoExternalLayerDependencies(source);

  console.log(JSON.stringify({
    status: "ok",
    schema: "metric.visual.instanced_point_layer_contract.v1",
    checks: {
      morphUniform: true,
      targetPositionAttribute: true,
      fragmentDiscard: true,
      depthWriteMaterialPath: true,
      externalDependencies: false,
    },
  }, null, 2));
}

function assertNoExternalLayerDependencies(source) {
  const imports = [
    ...Array.from(source.matchAll(/\bfrom\s+["']([^"']+)["']/g), (match) => match[1]),
    ...Array.from(source.matchAll(/^\s*import\s+["']([^"']+)["']/gm), (match) => match[1]),
  ];
  const external = imports.filter((specifier) => !specifier.startsWith("."));
  assert(external.length === 0, "InstancedPointLayer must not add external dependencies", { external });
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
