#!/usr/bin/env node
/*
 * Public miniature scene contract gate.
 *
 * This gate consumes the browser-backed public regression report and verifies
 * that public preview scenes carry the reusable photographic miniature runtime
 * contract. It does not accept heroes and it does not judge visual taste.
 *
 * Artifacts:
 *   output/visual/check-public-miniature-scene-contract/results.json
 */

import { mkdir, readFile, writeFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const DEFAULT_REGRESSION_REPORT = resolve(ROOT, "output", "visual", "check-visual-regression-public-examples", "results.json");
const DEFAULT_BRIEF_MANIFEST = resolve(ROOT, "visual", "hero-visual-briefs.manifest.json");
const OUT_DIR = resolve(process.env.METRIC_VISUAL_MINIATURE_SCENE_OUT || resolve(ROOT, "output", "visual", "check-public-miniature-scene-contract"));
const REGRESSION_REPORT = resolve(process.env.METRIC_VISUAL_REGRESSION_REPORT || DEFAULT_REGRESSION_REPORT);
const BRIEF_MANIFEST = resolve(process.env.METRIC_VISUAL_BRIEF_MANIFEST || DEFAULT_BRIEF_MANIFEST);

const REQUIRED_POSTFX = Object.freeze([
  "postFx.cameraDof",
  "postFx.miniatureFrame",
  "postFx.colorGrade",
  "postFx.vignette",
]);

const LABEL_ROLES = Object.freeze([
  "region-labels",
  "type-labels",
  "paired-space-label-anchors",
]);

async function main() {
  const regression = JSON.parse(await readFile(REGRESSION_REPORT, "utf8"));
  const manifest = JSON.parse(await readFile(BRIEF_MANIFEST, "utf8"));
  const briefs = new Map((manifest.previewBriefs || []).map((brief) => [brief.exampleId, brief]));
  const issues = [];
  const rows = [];

  if (regression.ok !== true) {
    issues.push({ code: "regression-report-not-green", report: REGRESSION_REPORT });
  }

  for (const result of regression.results || []) {
    if (result.status?.category !== "public-preview-only") continue;
    const brief = briefs.get(result.name);
    const row = inspectPreview(result, brief);
    rows.push(row);
    issues.push(...row.issues);
  }

  const report = {
    ok: issues.length === 0,
    generatedAt: new Date().toISOString(),
    regressionReport: REGRESSION_REPORT,
    regressionGeneratedAt: regression.generatedAt ?? null,
    briefManifest: BRIEF_MANIFEST,
    rowCount: rows.length,
    rows,
    issues,
  };

  await mkdir(OUT_DIR, { recursive: true });
  await writeFile(resolve(OUT_DIR, "results.json"), JSON.stringify(report, null, 2));
  console.log(JSON.stringify(report, null, 2));
  if (!report.ok) process.exitCode = 1;
}

function inspectPreview(result, brief) {
  const runtime = result.runtime?.state || {};
  const primitives = runtime.descriptorPrimitives || [];
  const roles = runtime.descriptorRoles || [];
  const postprocessPasses = runtime.postprocessPasses || [];
  const issues = [];

  if (result.render?.ok !== true || result.ok !== true) {
    issues.push({ code: "preview-regression-not-green", example: result.name });
  }

  for (const pass of REQUIRED_POSTFX) {
    if (!postprocessPasses.includes(pass)) {
      issues.push({
        code: "miniature-postfx-pass-missing",
        example: result.name,
        pass,
        postprocessPasses,
      });
    }
  }

  if (postprocessPasses.includes("TiltShiftPass") && !postprocessPasses.includes("postFx.cameraDof")) {
    issues.push({
      code: "tilt-shift-without-camera-depth-dof",
      example: result.name,
      postprocessPasses,
    });
  }

  if (!brief) {
    issues.push({ code: "missing-visual-brief", example: result.name });
  } else {
    for (const primitive of brief.requiredPrimitives || []) {
      if (!primitives.includes(primitive)) {
        issues.push({
          code: "required-miniature-primitive-missing",
          example: result.name,
          primitive,
          primitives,
        });
      }
    }
    for (const role of brief.requiredDescriptorRoles || []) {
      if (!roles.includes(role)) {
        issues.push({
          code: "required-miniature-role-missing",
          example: result.name,
          role,
          roles,
        });
      }
    }
  }

  const labelRoles = LABEL_ROLES.filter((role) => roles.includes(role));
  if (labelRoles.length && !primitives.includes("BillboardLabelLayer")) {
    issues.push({
      code: "label-role-without-billboard-label-layer",
      example: result.name,
      labelRoles,
      primitives,
    });
  }

  return {
    name: result.name,
    ok: issues.length === 0,
    expectedPrimaryVisualGrammar: brief?.expectedPrimaryVisualGrammar || result.grammar?.expected || null,
    primitives,
    roles,
    postprocessPasses,
    cameraDepthDof: postprocessPasses.includes("postFx.cameraDof"),
    miniatureFrame: postprocessPasses.includes("postFx.miniatureFrame"),
    labelRoles,
    issues,
  };
}

main().catch((error) => {
  console.error(JSON.stringify({
    ok: false,
    message: error instanceof Error ? error.message : String(error),
  }, null, 2));
  process.exitCode = 1;
});
