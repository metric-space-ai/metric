#!/usr/bin/env node
import { readFileSync } from "node:fs";

const runtimeSource = readFileSync(new URL("../src/runtime/runtime.js", import.meta.url), "utf8");
const stackSource = readFileSync(new URL("../src/postfx/stack.js", import.meta.url), "utf8");

const checks = [];

check(
  runtimeSource.includes("postFxOptions?.cameraDof?.enabled === true")
    && runtimeSource.includes("sceneDepthTexture: wantsCameraDepthTexture"),
  "RuntimePostProcessPipeline requests scene depth texture when cameraDof is enabled",
);

checkOrder(runtimeSource, [
  "new FxaaPass",
  "new TiltShiftPass",
  "new MetricPostFxStack",
], "native pipeline order is FXAA -> TiltShift -> METRIC postFx");

checkOrder(stackSource, [
  "\"cameraDof\"",
  "\"bloom\"",
  "\"miniatureFrame\"",
  "\"colorGrade\"",
  "\"vignette\"",
  "\"filmGrain\"",
], "METRIC postFx stack order is cameraDof -> bloom -> frame -> grade -> vignette -> filmGrain");

for (const token of [
  "nativePasses",
  "enabledNativePasses",
  "pipelineOrder",
  "pipelinePhases",
  "sceneDepthTexture",
  "sceneDepthEncoding",
  "model: this.postFxStack.cameraDof.dof?.model",
  "focusDistance: this.postFxStack.cameraDof.dof?.focusDistance",
]) {
  check(runtimeSource.includes(token), `runtime state exposes ${token}`);
}

check(
  runtimeSource.includes("function resolveCameraViewMatrix")
    && runtimeSource.includes("camera.matrixWorldInverse")
    && runtimeSource.includes("function cameraForwardDistanceToPoint"),
  "point focus resolves camera-space distance from view matrix or forward axis",
);

const cameraDistanceBody = runtimeSource.slice(
  runtimeSource.indexOf("function cameraDistanceToPoint"),
  runtimeSource.indexOf("function resolveCameraViewMatrix"),
);
check(
  !cameraDistanceBody.includes("Math.hypot(dx, dy, dz)"),
  "point focus does not fall back to Euclidean camera distance",
);

const failed = checks.filter((entry) => !entry.ok);
const result = {
  status: failed.length ? "error" : "ok",
  checks,
};

console.log(JSON.stringify(result, null, 2));
if (failed.length) process.exitCode = 1;

function check(ok, message) {
  checks.push({ ok: Boolean(ok), message });
}

function checkOrder(source, tokens, message) {
  const indexes = tokens.map((token) => source.indexOf(token));
  const ok = indexes.every((index) => index >= 0)
    && indexes.every((index, tokenIndex) => tokenIndex === 0 || indexes[tokenIndex - 1] < index);
  check(ok, message);
}
