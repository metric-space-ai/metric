#!/usr/bin/env node
/*
 * Browser-backed public visual regression gate.
 *
 * This is intentionally dependency-free: it drives an installed Chrome/Chromium
 * through the Chrome DevTools Protocol instead of requiring Playwright. The
 * gate proves load/render/evidence/grammar/interaction separately; it never
 * upgrades a page to hero-accepted because it merely loaded or rendered.
 *
 * Artifacts:
 *   output/visual/check-visual-regression-public-examples/*.png
 *   output/visual/check-visual-regression-public-examples/results.json
 */

import { createHash } from "node:crypto";
import { createServer } from "node:http";
import { inflateSync } from "node:zlib";
import { mkdtemp, mkdir, readFile, readdir, rm, stat, writeFile } from "node:fs/promises";
import { existsSync, readdirSync } from "node:fs";
import { homedir, tmpdir } from "node:os";
import { dirname, extname, join, resolve } from "node:path";
import { spawn } from "node:child_process";
import { fileURLToPath } from "node:url";

import {
  isExplicitNativeMetricVisualExport,
  isNativeMetricVisualDocument,
  isSyntheticMetricVisualEvidence,
} from "../src/data/provenance.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const SITE = resolve(ROOT, "docs", "site", "index.html");
const EXAMPLES_DIR = resolve(ROOT, "visual", "examples");
const OUT_DIR = resolve(process.env.METRIC_VISUAL_OUT || join(ROOT, "output", "visual", "check-visual-regression-public-examples"));
const VIEWPORT = parseViewport(process.env.METRIC_VISUAL_VIEWPORT || "1280x820");
const NAV_TIMEOUT_MS = Number(process.env.METRIC_VISUAL_NAV_TIMEOUT_MS || 45000);
const READY_TIMEOUT_MS = Number(process.env.METRIC_VISUAL_READY_TIMEOUT_MS || 45000);
const FRAME_SAMPLE_TARGET = Number(process.env.METRIC_VISUAL_FRAME_SAMPLES || 45);

const MIME = {
  ".html": "text/html; charset=utf-8",
  ".js": "text/javascript; charset=utf-8",
  ".mjs": "text/javascript; charset=utf-8",
  ".json": "application/json; charset=utf-8",
  ".css": "text/css; charset=utf-8",
  ".png": "image/png",
  ".jpg": "image/jpeg",
  ".jpeg": "image/jpeg",
  ".svg": "image/svg+xml",
  ".wasm": "application/wasm",
};

const EXPECTED_GRAMMAR = {
  "grae10-metric-engine": {
    status: "hero-accepted",
    label: "protected GRAE10 60k metric-engine reference",
    sourcePattern: /metric\.visual\.grae10\.dataset\.v1|RECORD_COUNT\s*=\s*60_000/,
  },
  "condition-monitoring-hero": {
    status: "public-preview-only",
    label: "condition metric-space with density/anomaly/trajectory grammar",
    sourcePattern: /\.showConditionMonitoring\(/,
  },
  "process-curve-external-hero": {
    status: "public-preview-only",
    label: "external UCR process-curve recovery grammar",
    sourcePattern: /\.showProcessCurves\(/,
  },
  "mixed-record-hero": {
    status: "public-preview-only",
    label: "mixed-record typed glyph and cross-type relation grammar",
    sourcePattern: /\.showMixedRecords\(/,
  },
  "cross-space-dependency-hero": {
    status: "public-preview-only",
    label: "paired cross-space dependence grammar",
    sourcePattern: /\.showCrossSpace\(/,
  },
  "relation-matrix-neighborhood": {
    status: "public-preview-only",
    label: "relation-matrix plus neighborhood graph grammar",
    sourcePattern: /\.showRelationMatrixNeighborhood\(/,
  },
  "dynamics-noise-hero": {
    status: "public-preview-only",
    label: "finite-dynamics trajectory grammar",
    sourcePattern: /\.showDynamics\(/,
  },
  "mapping-dimensionality-hero": {
    status: "public-preview-only",
    label: "mapping coordinate-morph grammar",
    sourcePattern: /\.showMapping\(/,
  },
};

const INTERNAL_ENGINE_EXAMPLES = {
  "native-engine-probe": {
    status: "engine-internal",
    label: "native process-curve tube miniature probe",
    sourcePattern: /createProcessCurveMiniatureSceneBundle\(|trackMode:\s*"tube"/,
    requiresGpuCurvePicking: true,
  },
};

const EXPLICIT_NATIVE_PREVIEW_EXAMPLES = [
  "process-curve-external-hero",
];

const GPU_DIAGNOSTICS_SCRIPT = String.raw`
(() => {
  if (window.__metricVisualGpuDiagnostics) return;
  const diagnostics = {
    bufferDataCalls: 0,
    totalBufferBytes: 0,
    drawArraysCalls: 0,
    drawElementsCalls: 0,
    instancedDrawCalls: 0,
    drawCalls: 0,
    drawVertices: 0,
    drawInstances: 0,
  };
  Object.defineProperty(window, "__metricVisualGpuDiagnostics", { value: diagnostics, configurable: false });
  const byteLength = (value) => {
    if (typeof value === "number") return value;
    if (value && typeof value.byteLength === "number") return value.byteLength;
    return 0;
  };
  const patch = (proto, name, wrap) => {
    if (!proto || typeof proto[name] !== "function" || proto[name].__metricVisualPatched) return;
    const original = proto[name];
    const next = wrap(original);
    next.__metricVisualPatched = true;
    proto[name] = next;
  };
  const patchProto = (proto) => {
    patch(proto, "bufferData", (original) => function bufferDataPatched(target, dataOrSize, usage) {
      diagnostics.bufferDataCalls += 1;
      diagnostics.totalBufferBytes += byteLength(dataOrSize);
      return original.apply(this, arguments);
    });
    patch(proto, "drawArrays", (original) => function drawArraysPatched(mode, first, count) {
      diagnostics.drawArraysCalls += 1;
      diagnostics.drawCalls += 1;
      diagnostics.drawVertices += Number(count) || 0;
      return original.apply(this, arguments);
    });
    patch(proto, "drawElements", (original) => function drawElementsPatched(mode, count, type, offset) {
      diagnostics.drawElementsCalls += 1;
      diagnostics.drawCalls += 1;
      diagnostics.drawVertices += Number(count) || 0;
      return original.apply(this, arguments);
    });
    patch(proto, "drawArraysInstanced", (original) => function drawArraysInstancedPatched(mode, first, count, instanceCount) {
      diagnostics.instancedDrawCalls += 1;
      diagnostics.drawCalls += 1;
      diagnostics.drawVertices += (Number(count) || 0) * Math.max(1, Number(instanceCount) || 0);
      diagnostics.drawInstances += Number(instanceCount) || 0;
      return original.apply(this, arguments);
    });
    patch(proto, "drawElementsInstanced", (original) => function drawElementsInstancedPatched(mode, count, type, offset, instanceCount) {
      diagnostics.instancedDrawCalls += 1;
      diagnostics.drawCalls += 1;
      diagnostics.drawVertices += (Number(count) || 0) * Math.max(1, Number(instanceCount) || 0);
      diagnostics.drawInstances += Number(instanceCount) || 0;
      return original.apply(this, arguments);
    });
    patch(proto, "getExtension", (original) => function getExtensionPatched(name) {
      const extension = original.apply(this, arguments);
      if (extension && String(name).toLowerCase() === "angle_instanced_arrays") {
        patch(extension, "drawArraysInstancedANGLE", (drawOriginal) => function drawArraysInstancedAnglePatched(mode, first, count, instanceCount) {
          diagnostics.instancedDrawCalls += 1;
          diagnostics.drawCalls += 1;
          diagnostics.drawVertices += (Number(count) || 0) * Math.max(1, Number(instanceCount) || 0);
          diagnostics.drawInstances += Number(instanceCount) || 0;
          return drawOriginal.apply(this, arguments);
        });
        patch(extension, "drawElementsInstancedANGLE", (drawOriginal) => function drawElementsInstancedAnglePatched(mode, count, type, offset, instanceCount) {
          diagnostics.instancedDrawCalls += 1;
          diagnostics.drawCalls += 1;
          diagnostics.drawVertices += (Number(count) || 0) * Math.max(1, Number(instanceCount) || 0);
          diagnostics.drawInstances += Number(instanceCount) || 0;
          return drawOriginal.apply(this, arguments);
        });
      }
      return extension;
    });
  };
  patchProto(window.WebGLRenderingContext && window.WebGLRenderingContext.prototype);
  patchProto(window.WebGL2RenderingContext && window.WebGL2RenderingContext.prototype);
})();
`;

const PAGE_PROBE_SCRIPT = String.raw`
(() => {
  function dataset() {
    return Object.fromEntries(Object.entries(document.documentElement.dataset || {}));
  }
  function canvasSummaries() {
    return Array.from(document.querySelectorAll("canvas")).map((canvas, index) => {
      const rect = canvas.getBoundingClientRect();
      const style = getComputedStyle(canvas);
      return {
        index,
        id: canvas.id || null,
        ariaLabel: canvas.getAttribute("aria-label") || null,
        width: Math.round(rect.width),
        height: Math.round(rect.height),
        x: Math.max(0, rect.left),
        y: Math.max(0, rect.top),
        canvasWidth: canvas.width,
        canvasHeight: canvas.height,
        cursor: style.cursor,
        touchAction: style.touchAction,
        visible: rect.width > 0 && rect.height > 0 && style.visibility !== "hidden" && style.display !== "none",
      };
    }).sort((a, b) => (b.width * b.height) - (a.width * a.height));
  }
  function summarizeState(state) {
    if (!state || typeof state !== "object") return null;
    const runtime = state.runtime || (typeof state.layerInstanceCount === "number" ? state : null);
    const diagnostics = state.diagnostics || null;
    const descriptors = Array.isArray(state.descriptors) ? state.descriptors : [];
    const views = Array.isArray(state.views) ? state.views : [];
    return {
      recordCount: Number(diagnostics?.recordCount ?? document.documentElement.dataset.metricRecordCount) || null,
      selectedViewKind: diagnostics?.selectedViewKind || null,
      viewKinds: views.map((view) => view?.kind).filter(Boolean),
      descriptorPrimitives: descriptors.map((descriptor) => descriptor?.primitive || descriptor?.kind).filter(Boolean),
      descriptorRoles: descriptors.map((descriptor) => descriptor?.metadata?.role || descriptor?.source?.role || null).filter(Boolean),
      mappingResidualLayers: summarizeMappingResidualLayers(descriptors),
      layerDescriptorCount: Number(diagnostics?.layerDescriptorCount ?? runtime?.layerDescriptorCount ?? descriptors.length) || 0,
      runtimeLayerCount: Number(diagnostics?.runtimeLayerCount ?? runtime?.layerInstanceCount) || 0,
      runtimeLayerState: runtime?.layerState?.status || null,
      postprocessPasses: runtime?.postprocess?.pipelineOrder || [],
      warnings: (diagnostics?.warnings || runtime?.warnings || []).map((warning) => warning?.code || String(warning)),
      selectedRecordId: runtime?.selectedRecordId || null,
      selectedPair: runtime?.selectedPair || null,
      relationMatrixReadability: summarizeRelationMatrixReadability(descriptors),
      inspection: runtime?.inspection || null,
    };
  }
  function summarizeMappingResidualLayers(descriptors) {
    return descriptors
      .filter((descriptor) => descriptor?.metadata?.role === "residual/error" || descriptor?.source?.role === "residual/error")
      .map((descriptor) => ({
        id: descriptor?.id || null,
        primitive: descriptor?.primitive || descriptor?.kind || null,
        evidenceRole: descriptor?.metadata?.evidenceRole || null,
        recordCount: Number(descriptor?.metadata?.recordCount ?? descriptor?.channels?.recordId?.count) || 0,
        residualPropertyId: descriptor?.metadata?.residualPropertyId || descriptor?.source?.propertyId || null,
        residualChannelCount: Number(descriptor?.channels?.residual?.count) || 0,
        mappingEvidenceSchema: descriptor?.metadata?.mappingEvidence?.schema || null,
      }));
  }
  function summarizeRelationMatrixReadability(descriptors) {
    return descriptors
      .filter((descriptor) => (descriptor?.primitive || descriptor?.kind) === "RelationMatrixLayer")
      .map((descriptor) => {
        const matrixReadability = descriptor?.metadata?.readabilityDiagnostics
          || descriptor?.metadata?.diagnostics?.matrixReadability
          || null;
        return {
          id: descriptor?.id || null,
          relationId: descriptor?.source?.relationId || descriptor?.metadata?.relationId || matrixReadability?.relationId || null,
          blockCount: Number(matrixReadability?.blockCount ?? matrixReadability?.blocks?.count) || 0,
          blockLabelCount: Number(matrixReadability?.blockLabelCount ?? matrixReadability?.blocks?.labeledCount) || 0,
          blockLabels: Array.isArray(matrixReadability?.blocks?.labels) ? matrixReadability.blocks.labels : [],
          blockCoverageState: matrixReadability?.blocks?.coverage?.state || null,
          tileCount: Number(matrixReadability?.tileCount) || 0,
          tileSummarySource: matrixReadability?.tileSummarySource || null,
        };
      });
  }
  function runtimeHandles() {
    return Object.keys(window)
      .filter((key) => /^metric/i.test(key))
      .map((key) => {
        const handle = window[key];
        if (!handle || typeof handle !== "object") return null;
        let state = null;
        try {
          if (typeof handle.getState === "function") state = handle.getState();
          else if (typeof handle.visual?.getState === "function") state = handle.visual.getState();
          else if (typeof handle.runtime?.getState === "function") state = handle.runtime.getState();
        } catch (error) {
          return { key, error: error instanceof Error ? error.message : String(error) };
        }
        return { key, state: summarizeState(state) };
      })
      .filter(Boolean);
  }
  function runtimeObjects() {
    return Object.keys(window)
      .filter((key) => /^metric/i.test(key))
      .map((key) => {
        const handle = window[key];
        const runtime = handle?.runtime || handle?.visual?.runtime || (typeof handle?.pickAt === "function" ? handle : null);
        return runtime ? { key, runtime } : null;
      })
      .filter(Boolean);
  }
  function gpuPickingProbe() {
    for (const entry of runtimeObjects()) {
      const runtime = entry.runtime;
      if (typeof runtime.refreshPickingIndex !== "function" || typeof runtime.pickAt !== "function") continue;
      runtime.refreshPickingIndex();
      const index = runtime.pickingIndex || {};
      if (!(index.gpuLayerCount > 0) || !Array.isArray(index.recordPoints) || !index.recordPoints.length) continue;
      const pickableLayerIds = new Set((runtime.layers || [])
        .filter((layer) => layer && layer.visible !== false && (typeof layer.renderPicking === "function" || typeof layer.renderPickIds === "function"))
        .map((layer) => layer.id)
        .filter(Boolean));
      const pickableRecordPoints = index.recordPoints.filter((point) => pickableLayerIds.has(point.layerId));
      const candidates = (pickableRecordPoints.length ? pickableRecordPoints : index.recordPoints).slice(0, 160);
      const misses = [];
      for (const point of candidates) {
        const pixel = runtime.camera?.projectToPixel?.(point.position, {});
        if (!pixel || pixel.visible === false || !Number.isFinite(pixel.x) || !Number.isFinite(pixel.y)) continue;
        const result = runtime.pickAt({ x: pixel.x, y: pixel.y }, {
          gpu: true,
          relationMatrix: false,
          graph: false,
          cpuFallback: false,
          source: "gpu-contract-probe",
        });
        const sample = {
          source: result?.source || null,
          recordId: result?.recordId ?? null,
          expectedRecordId: point.recordId ?? null,
          layerId: point.layerId ?? null,
          x: Math.round(pixel.x),
          y: Math.round(pixel.y),
        };
        if (result?.source === "gpu-picking" && result?.hit === true) {
          return {
            key: entry.key,
            available: true,
            hit: true,
            source: "gpu-picking",
            recordId: result?.recordId ?? null,
            expectedRecordId: point.recordId ?? null,
            layerId: point.layerId ?? null,
            layerCount: index.gpuLayerCount,
            candidateCount: index.recordPoints.length,
            pickableCandidateCount: pickableRecordPoints.length,
            testedCandidates: misses.length + 1,
          };
        }
        if (misses.length < 8) misses.push(sample);
      }
      if (candidates.length) {
        runtime.pickAt({ x: 0, y: 0 }, {
          gpu: true,
          relationMatrix: false,
          graph: false,
          cpuFallback: false,
          gpuRadiusPx: 0,
          debugGpuScan: "always",
          source: "gpu-contract-debug-scan",
        });
      }
      return {
        key: entry.key,
        available: true,
        hit: false,
        source: "no-gpu-hit-for-visible-record-candidates",
        layerCount: index.gpuLayerCount,
        candidateCount: index.recordPoints.length,
        pickableCandidateCount: pickableRecordPoints.length,
        testedCandidates: candidates.length,
        misses,
        debug: runtime.lastGpuPickingDebug || null,
      };
    }
    return { available: false, hit: false, source: "no-gpu-pickable-runtime" };
  }
  function gpuEdgePickingProbe() {
    for (const entry of runtimeObjects()) {
      const runtime = entry.runtime;
      if (typeof runtime.refreshPickingIndex !== "function" || typeof runtime.pickAt !== "function") continue;
      runtime.refreshPickingIndex();
      const index = runtime.pickingIndex || {};
      if (!(index.gpuLayerCount > 0) || !Array.isArray(index.graphEdges) || !index.graphEdges.length) continue;
      const pickableLayerIds = new Set((runtime.layers || [])
        .filter((layer) => layer && layer.visible !== false && (typeof layer.renderPicking === "function" || typeof layer.renderPickIds === "function"))
        .map((layer) => layer.id)
        .filter(Boolean));
      const candidates = index.graphEdges
        .filter((edge) => pickableLayerIds.has(edge.layerId))
        .slice(0, 160);
      const misses = [];
      for (const edge of candidates) {
        const a = runtime.camera?.projectToPixel?.(edge.sourcePosition, {});
        const b = runtime.camera?.projectToPixel?.(edge.targetPosition, {});
        if (!a || !b || a.visible === false || b.visible === false) continue;
        if (![a.x, a.y, b.x, b.y].every(Number.isFinite)) continue;
        const x = (a.x + b.x) * 0.5;
        const y = (a.y + b.y) * 0.5;
        const result = runtime.pickAt({ x, y }, {
          gpu: true,
          relationMatrix: false,
          graph: false,
          cpuFallback: false,
          gpuRadiusPx: 10,
          source: "gpu-edge-contract-probe",
        });
        const sample = {
          source: result?.source || null,
          kind: result?.kind || null,
          edgeId: result?.edgeId ?? null,
          expectedEdgeId: edge.edgeId ?? null,
          layerId: edge.layerId ?? null,
          x: Math.round(x),
          y: Math.round(y),
        };
        if (result?.source === "gpu-picking" && result?.hit === true && result?.kind === "edge") {
          return {
            key: entry.key,
            available: true,
            hit: true,
            source: "gpu-picking",
            edgeId: result?.edgeId ?? null,
            expectedEdgeId: edge.edgeId ?? null,
            layerId: edge.layerId ?? null,
            layerCount: index.gpuLayerCount,
            candidateCount: index.graphEdges.length,
            pickableCandidateCount: candidates.length,
            testedCandidates: misses.length + 1,
          };
        }
        if (misses.length < 8) misses.push(sample);
      }
      return {
        key: entry.key,
        available: true,
        hit: false,
        source: "no-gpu-hit-for-visible-edge-candidates",
        layerCount: index.gpuLayerCount,
        candidateCount: index.graphEdges.length,
        pickableCandidateCount: candidates.length,
        testedCandidates: candidates.length,
        misses,
      };
    }
    return { available: false, hit: false, source: "no-gpu-pickable-edge-runtime" };
  }
  function gpuFieldPickingProbe() {
    for (const entry of runtimeObjects()) {
      const runtime = entry.runtime;
      if (typeof runtime.refreshPickingIndex !== "function" || typeof runtime.pickAt !== "function") continue;
      runtime.refreshPickingIndex();
      const index = runtime.pickingIndex || {};
      if (!(index.gpuLayerCount > 0) || !Array.isArray(index.recordPoints) || !index.recordPoints.length) continue;
      const fieldLayerIds = new Set((runtime.layers || [])
        .filter((layer) => layer && layer.visible !== false && (
          layer.constructor?.name === "HeatFieldLayer" ||
          layer.descriptor?.primitive === "HeatFieldLayer" ||
          layer.descriptor?.kind === "HeatFieldLayer"
        ))
        .map((layer) => layer.id)
        .filter(Boolean));
      if (!fieldLayerIds.size) continue;
      const candidates = index.recordPoints
        .filter((point) => fieldLayerIds.has(point.layerId))
        .slice(0, 160);
      const misses = [];
      for (const point of candidates) {
        const pixel = runtime.camera?.projectToPixel?.(point.position, {});
        if (!pixel || pixel.visible === false || !Number.isFinite(pixel.x) || !Number.isFinite(pixel.y)) continue;
        const result = runtime.pickAt({ x: pixel.x, y: pixel.y }, {
          gpu: true,
          relationMatrix: false,
          graph: false,
          cpuFallback: false,
          source: "gpu-field-contract-probe",
        });
        const sample = {
          source: result?.source || null,
          recordId: result?.recordId ?? null,
          expectedRecordId: point.recordId ?? null,
          layerId: point.layerId ?? null,
          x: Math.round(pixel.x),
          y: Math.round(pixel.y),
        };
        if (result?.source === "gpu-picking" && result?.hit === true && result?.layerId === point.layerId) {
          return {
            key: entry.key,
            available: true,
            hit: true,
            source: "gpu-picking",
            recordId: result?.recordId ?? null,
            expectedRecordId: point.recordId ?? null,
            layerId: point.layerId ?? null,
            layerCount: index.gpuLayerCount,
            candidateCount: index.recordPoints.length,
            pickableCandidateCount: candidates.length,
            testedCandidates: misses.length + 1,
          };
        }
        if (misses.length < 8) misses.push(sample);
      }
      return {
        key: entry.key,
        available: true,
        hit: false,
        source: "no-gpu-hit-for-visible-field-candidates",
        layerCount: index.gpuLayerCount,
        candidateCount: index.recordPoints.length,
        pickableCandidateCount: candidates.length,
        testedCandidates: candidates.length,
        misses,
      };
    }
    return { available: false, hit: false, source: "no-gpu-pickable-field-runtime" };
  }
  function gpuCurvePickingProbe() {
    for (const entry of runtimeObjects()) {
      const runtime = entry.runtime;
      if (typeof runtime.refreshPickingIndex !== "function" || typeof runtime.pickAt !== "function") continue;
      runtime.refreshPickingIndex();
      const index = runtime.pickingIndex || {};
      if (!(index.gpuLayerCount > 0)) continue;
      const curveLayers = (runtime.layers || [])
        .filter((layer) => layer && layer.visible !== false && (
          layer.constructor?.name === "CurveRibbonLayer" ||
          layer.constructor?.name === "CurveTubeMeshLayer" ||
          layer.descriptor?.primitive === "CurveRibbonLayer" ||
          layer.descriptor?.primitive === "CurveTubeMeshLayer" ||
          layer.descriptor?.kind === "CurveRibbonLayer" ||
          layer.descriptor?.kind === "CurveTubeMeshLayer"
        ))
        .filter((layer) => typeof layer.renderPicking === "function" && Array.isArray(layer.pickProbePoints) && layer.pickProbePoints.length);
      const candidates = curveLayers
        .flatMap((layer) => layer.pickProbePoints.map((point) => ({ ...point, layerId: point.layerId || layer.id })))
        .slice(0, 240);
      const curveLayerIds = new Set(curveLayers.map((layer) => layer.id).filter(Boolean));
      const misses = [];
      for (const point of candidates) {
        const pixel = runtime.camera?.projectToPixel?.(point.position, {});
        if (!pixel || pixel.visible === false || !Number.isFinite(pixel.x) || !Number.isFinite(pixel.y)) continue;
        const result = withOnlyGpuLayers(runtime, curveLayerIds, () => runtime.pickAt({ x: pixel.x, y: pixel.y }, {
          gpu: true,
          relationMatrix: false,
          graph: false,
          cpuFallback: false,
          gpuRadiusPx: 12,
          source: "gpu-curve-contract-probe",
        }));
        const sample = {
          source: result?.source || null,
          kind: result?.kind || null,
          edgeId: result?.edgeId ?? null,
          expectedEdgeId: point.edgeId ?? null,
          pathId: result?.payload?.pathId ?? result?.pathId ?? null,
          expectedPathId: point.pathId ?? null,
          layerId: result?.layerId ?? null,
          expectedLayerId: point.layerId ?? null,
          x: Math.round(pixel.x),
          y: Math.round(pixel.y),
        };
        if (result?.source === "gpu-picking" && result?.hit === true && result?.kind === "edge" && result?.layerId === point.layerId) {
          return {
            key: entry.key,
            available: true,
            hit: true,
            source: "gpu-picking",
            edgeId: result?.edgeId ?? null,
            expectedEdgeId: point.edgeId ?? null,
            pathId: result?.payload?.pathId ?? result?.pathId ?? null,
            expectedPathId: point.pathId ?? null,
            layerId: point.layerId ?? null,
            layerCount: index.gpuLayerCount,
            candidateCount: candidates.length,
            testedCandidates: misses.length + 1,
          };
        }
        if (misses.length < 8) misses.push(sample);
      }
      return {
        key: entry.key,
        available: true,
        hit: false,
        source: "no-gpu-hit-for-visible-curve-candidates",
        layerCount: index.gpuLayerCount,
        candidateCount: candidates.length,
        testedCandidates: candidates.length,
        misses,
      };
    }
    return { available: false, hit: false, source: "no-gpu-pickable-curve-runtime" };
  }
  function withOnlyGpuLayers(runtime, keepLayerIds, callback) {
    const changed = [];
    for (const layer of runtime.layers || []) {
      if (!layer || keepLayerIds.has(layer.id) || layer.visible === false) continue;
      changed.push([layer, layer.visible]);
      layer.visible = false;
    }
    try {
      return callback();
    } finally {
      for (const [layer, visible] of changed) {
        layer.visible = visible;
      }
    }
  }
  const canvases = canvasSummaries();
  const handles = runtimeHandles();
  const largestCanvas = canvases[0] || null;
  const runtimeState = handles.map((entry) => entry.state).find((state) => state && state.runtimeLayerCount > 0)
    || handles.map((entry) => entry.state).find(Boolean)
    || null;
  const data = dataset();
  return {
    title: document.title,
    url: location.href,
    data,
    canvases,
    largestCanvas,
    runtimeHandles: handles,
    runtimeState,
    gpuPickingProbe: gpuPickingProbe(),
    gpuEdgePickingProbe: gpuEdgePickingProbe(),
    gpuFieldPickingProbe: gpuFieldPickingProbe(),
    gpuCurvePickingProbe: gpuCurvePickingProbe(),
    gpuDiagnostics: window.__metricVisualGpuDiagnostics || null,
    loadingText: Array.from(document.querySelectorAll(".loading")).map((node) => node.textContent.trim()).join(" | "),
    ready: Boolean(largestCanvas?.visible) && (
      Boolean(runtimeState?.runtimeLayerCount)
      || Boolean(data.metricGrae10Engine)
      || Boolean(data.metricConditionHero || data.metricProcessCurveExternalHero || data.metricMixedHero || data.metricCrossSpaceHero || data.metricRelationHero || data.metricDynamicsHero || data.metricMappingHero)
    ),
  };
})();
`;

const FRAME_SAMPLE_SCRIPT = String.raw`
(() => new Promise((resolve) => {
  const target = Math.max(5, Number(window.__metricVisualFrameSampleTarget || 45));
  const deltas = [];
  let last = null;
  function step(now) {
    if (last != null) deltas.push(now - last);
    last = now;
    if (deltas.length < target) requestAnimationFrame(step);
    else {
      const sorted = deltas.slice().sort((a, b) => a - b);
      const median = sorted[Math.floor(sorted.length / 2)] || 0;
      const p95 = sorted[Math.floor(sorted.length * 0.95)] || sorted[sorted.length - 1] || 0;
      resolve({
        sampleCount: deltas.length,
        medianMs: Number(median.toFixed(3)),
        p95Ms: Number(p95.toFixed(3)),
        minMs: Number((sorted[0] || 0).toFixed(3)),
        maxMs: Number((sorted[sorted.length - 1] || 0).toFixed(3)),
      });
    }
  }
  requestAnimationFrame(step);
}))();
`;

async function main() {
  await mkdir(OUT_DIR, { recursive: true });
  const examples = await discoverPublicExamples();
  const syntheticFixtures = await discoverSyntheticFixtures();
  const chromeExecutable = findChromeExecutable();

  if (!chromeExecutable) {
    const report = {
      ok: false,
      reason: "browser-unavailable",
      message: "No Chrome/Chromium executable found; browser-backed visual gate is missing, not passing.",
      examples,
      syntheticFixtures,
    };
    await writeReport(report);
    console.log(JSON.stringify(report, null, 2));
    process.exitCode = 1;
    return;
  }

  let server = null;
  let browser = null;
  const results = [];
  try {
    server = await startStaticServer(ROOT);
    browser = await launchChrome(chromeExecutable);
    const baseUrl = `http://127.0.0.1:${server.address().port}`;
    for (const name of examples) {
      results.push(await checkExample(browser, baseUrl, name));
    }
  } catch (error) {
    results.push({
      name: "(runner)",
      ok: false,
      issues: [{ code: "runner-error", message: error instanceof Error ? error.message : String(error) }],
    });
  } finally {
    if (browser) await browser.close();
    if (server) server.close();
  }

  const report = {
    ok: results.every((entry) => entry.ok),
    generatedAt: new Date().toISOString(),
    chromeExecutable,
    viewport: VIEWPORT,
    outDir: OUT_DIR,
    total: results.length,
    failed: results.filter((entry) => !entry.ok).length,
    results,
    syntheticFixtures,
  };
  await writeReport(report);
  console.log(JSON.stringify(report, null, 2));
  if (!report.ok) process.exitCode = 1;
}

async function checkExample(browser, baseUrl, name) {
  const indexPath = resolve(EXAMPLES_DIR, name, "index.html");
  const indexSource = await maybeRead(indexPath);
  const expected = EXPECTED_GRAMMAR[name] || INTERNAL_ENGINE_EXAMPLES[name] || {
    status: "public-preview-only",
    label: "unclassified public visual preview",
    sourcePattern: /./,
  };
  const evidence = await summarizeEvidence(name, indexSource);
  const grammar = await summarizeGrammar(name, indexSource, evidence, expected);
  const url = `${baseUrl}/visual/examples/${name}/index.html?verify=1`;
  const issues = [];
  let page = null;
  let probe = null;
  let frameTiming = null;
  let interaction = null;
  let screenshot = null;
  let canvasScreenshot = null;
  let canvasPixels = null;

  try {
    page = await browser.newPage();
    page.consoleErrors = [];
    page.pageErrors = [];
    await page.prepare();
    await page.navigate(url);
    probe = await waitForProbe(page);
    if (process.env.METRIC_VISUAL_REGRESSION_FRAME_SAMPLE === "1") {
      try {
        await evaluate(page, `window.__metricVisualFrameSampleTarget = ${JSON.stringify(FRAME_SAMPLE_TARGET)}`, { timeoutMs: 30000 });
        frameTiming = await evaluate(page, FRAME_SAMPLE_SCRIPT, { timeoutMs: 30000 });
      } catch (error) {
        frameTiming = { error: error instanceof Error ? error.message : String(error) };
      }
    } else {
      frameTiming = { skipped: "covered-by-check-visual-performance-large-scenes" };
    }
    interaction = await performBasicInteraction(page, probe);
    probe = await evaluate(page, PAGE_PROBE_SCRIPT, { timeoutMs: 30000 });
    screenshot = await captureScreenshot(page, join(OUT_DIR, `${name}.png`));
    if (probe?.largestCanvas) {
      canvasScreenshot = await captureScreenshot(page, join(OUT_DIR, `${name}.canvas.png`), { clip: clipFromCanvas(probe.largestCanvas) });
      canvasPixels = analyzePng(await readFile(canvasScreenshot));
    }
  } catch (error) {
    issues.push({ code: "browser-check-failed", message: error instanceof Error ? error.message : String(error) });
  } finally {
    if (page) await page.close().catch(() => {});
  }

  const load = {
    ok: Boolean(probe?.title || screenshot),
    title: probe?.title || null,
    consoleErrors: page?.consoleErrors || [],
    pageErrors: page?.pageErrors || [],
  };
  const render = {
    ok: Boolean(probe?.largestCanvas?.visible) && Boolean(canvasPixels?.nonBlank),
    canvas: probe?.largestCanvas || null,
    pixelSummary: canvasPixels,
    screenshot,
    canvasScreenshot,
  };
  const runtime = {
    state: probe?.runtimeState || null,
    gpuDiagnostics: probe?.gpuDiagnostics || null,
    frameTiming,
  };
  const status = classifyStatus(name, evidence, grammar, expected);
  if (status.heroAccepted && name !== "grae10-metric-engine") {
    issues.push({
      code: "hero-acceptance-from-preview-gate",
      message: "Only the protected GRAE10 reference may be hero-accepted by this gate.",
    });
  }
  if (!load.ok) issues.push({ code: "load-failed" });
  if (load.consoleErrors.length) issues.push({ code: "console-errors", count: load.consoleErrors.length });
  if (load.pageErrors.length) issues.push({ code: "page-errors", count: load.pageErrors.length });
  if (!render.ok) issues.push({ code: "render-not-proven" });
  if (!evidence.usesNativeEvidence && name !== "grae10-metric-engine" && expected.status !== "engine-internal") {
    issues.push({ code: "missing-native-evidence" });
  }
  if (!grammar.ok) issues.push({ code: "grammar-contract-missing", expected: expected.label });
  if (!interaction?.ok) issues.push({ code: "interaction-contract-failed", reason: interaction?.reason || "not-run" });
  if (name === "relation-matrix-neighborhood" && !hasReadableRelationMatrixBlocks(runtime.state)) {
    issues.push({
      code: "relation-matrix-block-readability-missing",
      readability: runtime.state?.relationMatrixReadability || [],
    });
  }
  if (name === "relation-matrix-neighborhood" && !hasRelationMatrixNeighborhoodRoles(runtime.state)) {
    issues.push({
      code: "relation-matrix-semantic-roles-missing",
      roles: runtime.state?.descriptorRoles || [],
      primitives: runtime.state?.descriptorPrimitives || [],
    });
  }
  if (name === "mapping-dimensionality-hero" && !hasNativeMappingResidualLayer(runtime.state)) {
    issues.push({
      code: "mapping-residual-error-grammar-missing",
      residualLayers: runtime.state?.mappingResidualLayers || [],
      roles: runtime.state?.descriptorRoles || [],
    });
  }
  if (requiresGpuPicking(runtime.state) && runtime.state?.inspection?.gpuPicking?.available !== true) {
    issues.push({
      code: "gpu-picking-not-active",
      primitives: runtime.state?.descriptorPrimitives || [],
      inspection: runtime.state?.inspection || null,
    });
  }
  if (requiresGpuPicking(runtime.state) && probe?.gpuPickingProbe?.source !== "gpu-picking") {
    issues.push({
      code: "gpu-picking-probe-failed",
      primitives: runtime.state?.descriptorPrimitives || [],
      probe: probe?.gpuPickingProbe || null,
    });
  }
  if (requiresGpuEdgePicking(runtime.state) && probe?.gpuEdgePickingProbe?.source !== "gpu-picking") {
    issues.push({
      code: "gpu-edge-picking-probe-failed",
      primitives: runtime.state?.descriptorPrimitives || [],
      probe: probe?.gpuEdgePickingProbe || null,
    });
  }
  if (requiresGpuFieldPicking(runtime.state) && probe?.gpuFieldPickingProbe?.source !== "gpu-picking") {
    issues.push({
      code: "gpu-field-picking-probe-failed",
      primitives: runtime.state?.descriptorPrimitives || [],
      probe: probe?.gpuFieldPickingProbe || null,
    });
  }
  if (requiresGpuCurvePicking(runtime.state) && probe?.gpuCurvePickingProbe?.source !== "gpu-picking") {
    issues.push({
      code: "gpu-curve-picking-probe-failed",
      primitives: runtime.state?.descriptorPrimitives || [],
      probe: probe?.gpuCurvePickingProbe || null,
    });
  }
  if (expected.requiresGpuCurvePicking === true && probe?.gpuCurvePickingProbe?.source !== "gpu-picking") {
    issues.push({
      code: "required-gpu-curve-picking-probe-failed",
      expected: expected.label,
      probe: probe?.gpuCurvePickingProbe || null,
    });
  }
  if (name === "grae10-metric-engine" && !evidence.protectedGrae10?.ok) {
    issues.push({ code: "grae10-protection-failed", details: evidence.protectedGrae10 });
  }

  return {
    name,
    ok: issues.length === 0,
    status,
    load,
    render,
    evidence,
    grammar,
    interaction,
    runtime,
    gpuPickingProbe: probe?.gpuPickingProbe || null,
    gpuEdgePickingProbe: probe?.gpuEdgePickingProbe || null,
    gpuFieldPickingProbe: probe?.gpuFieldPickingProbe || null,
    gpuCurvePickingProbe: probe?.gpuCurvePickingProbe || null,
    issues,
  };
}

function requiresGpuPicking(runtimeState) {
  const primitives = runtimeState?.descriptorPrimitives || [];
  return primitives.includes("InstancedPointLayer") ||
    primitives.includes("InstancedGlyphLayer") ||
    primitives.includes("GroundProjectionLayer") ||
    primitives.includes("HeatFieldLayer");
}

function hasReadableRelationMatrixBlocks(runtimeState) {
  const matrices = runtimeState?.relationMatrixReadability || [];
  return matrices.some((matrix) => (
    matrix.blockCount >= 2 &&
    matrix.blockLabelCount >= 2 &&
    matrix.blockLabels.every((label) => typeof label === "string" && label.trim().length > 0) &&
    matrix.blockCoverageState === "full" &&
    matrix.tileCount > 0 &&
    matrix.tileSummarySource === "exported-relation-texture-downsample"
  ));
}

function hasRelationMatrixNeighborhoodRoles(runtimeState) {
  const roles = new Set(runtimeState?.descriptorRoles || []);
  return roles.has("primary-relation-matrix") &&
    roles.has("neighborhood-graph-nodes") &&
    roles.has("neighborhood-graph-edges");
}

function hasNativeMappingResidualLayer(runtimeState) {
  return (runtimeState?.mappingResidualLayers || []).some((layer) => (
    layer.primitive === "RelationEdgeLayer" &&
    layer.evidenceRole === "mapping-residual-vectors" &&
    layer.mappingEvidenceSchema === "metric.visual.mapping_motion_evidence.v1" &&
    layer.residualChannelCount > 0 &&
    layer.recordCount > 0 &&
    typeof layer.residualPropertyId === "string" &&
    layer.residualPropertyId.trim().length > 0
  ));
}

function requiresGpuEdgePicking(runtimeState) {
  const primitives = runtimeState?.descriptorPrimitives || [];
  return primitives.includes("RelationEdgeLayer") && Number(runtimeState?.inspection?.graph?.edgeCount || 0) > 0;
}

function requiresGpuFieldPicking(runtimeState) {
  const primitives = runtimeState?.descriptorPrimitives || [];
  return primitives.includes("HeatFieldLayer");
}

function requiresGpuCurvePicking(runtimeState) {
  const primitives = runtimeState?.descriptorPrimitives || [];
  return primitives.includes("CurveRibbonLayer") || primitives.includes("CurveTubeMeshLayer");
}

function classifyStatus(name, evidence, grammar, expected = {}) {
  if (name === "grae10-metric-engine") {
    return {
      category: evidence.protectedGrae10?.ok ? "hero-accepted" : "blocked-by-missing-native-evidence",
      heroAccepted: evidence.protectedGrae10?.ok === true,
      publicPreviewOnly: false,
      syntheticDevelopmentFixture: false,
      blockedByMissingNativeEvidence: evidence.protectedGrae10?.ok !== true,
      reason: evidence.protectedGrae10?.ok
        ? "Protected 60k GRAE10 reference hash and dataset shape are intact."
      : "Protected 60k GRAE10 reference is not intact.",
    };
  }
  if (expected.status === "engine-internal" && grammar.ok) {
    return {
      category: "engine-internal",
      heroAccepted: false,
      publicPreviewOnly: false,
      syntheticDevelopmentFixture: false,
      blockedByMissingNativeEvidence: false,
      reason: "Internal engine probe is checked as a reusable engine contract, not a public hero.",
    };
  }
  if (evidence.usesNativeEvidence && grammar.ok) {
    return {
      category: "public-preview-only",
      heroAccepted: false,
      publicPreviewOnly: true,
      syntheticDevelopmentFixture: false,
      blockedByMissingNativeEvidence: false,
      reason: "Native metric.visual.v1 evidence and intended preview grammar are present; screenshot review is not hero acceptance.",
    };
  }
  if (evidence.syntheticEvidence.length && !evidence.usesNativeEvidence) {
    return {
      category: "synthetic-development-fixture",
      heroAccepted: false,
      publicPreviewOnly: false,
      syntheticDevelopmentFixture: true,
      blockedByMissingNativeEvidence: true,
      reason: "The page resolves only synthetic fixture evidence.",
    };
  }
  return {
    category: "blocked-by-missing-native-evidence",
    heroAccepted: false,
    publicPreviewOnly: false,
    syntheticDevelopmentFixture: false,
    blockedByMissingNativeEvidence: true,
    reason: "Native metric.visual.v1 evidence was not found or does not qualify.",
  };
}

async function summarizeGrammar(name, indexSource, evidence, expected) {
  if (name === "grae10-metric-engine") {
    return {
      ok: expected.sourcePattern.test(indexSource) && evidence.protectedGrae10?.recordCount === 60000,
      expected: expected.label,
      sourceMatch: expected.sourcePattern.test(indexSource),
    };
  }
  return {
    ok: expected.sourcePattern.test(indexSource),
    expected: expected.label,
    sourceMatch: expected.sourcePattern.test(indexSource),
    nativeViewKinds: evidence.nativeEvidence.flatMap((entry) => entry.viewKinds || []),
  };
}

async function summarizeEvidence(name, indexSource) {
  if (name === "grae10-metric-engine") {
    return summarizeGrae10Evidence();
  }
  const targets = extractFetchTargets(indexSource);
  const resolved = [];
  const nativeEvidence = [];
  const syntheticEvidence = [];
  for (const target of targets) {
    const filePath = resolveFetchTarget(resolve(EXAMPLES_DIR, name), target);
    const entry = { target, path: relativeFromRoot(filePath), exists: Boolean(filePath && existsSync(filePath)) };
    if (entry.exists && /\.json$/i.test(filePath)) {
      try {
        const document = JSON.parse(await readFile(filePath, "utf8"));
        const provenance = document?.provenance || {};
        entry.schema = document?.schema || null;
        entry.recordCount = Array.isArray(document?.records) ? document.records.length : null;
        entry.recordTypeCount = Array.isArray(document?.records) ? countRecordTypes(document.records) : null;
        entry.relationCount = Array.isArray(document?.relations) ? document.relations.length : null;
        entry.coordinateCount = Array.isArray(document?.coordinates) ? document.coordinates.length : null;
        entry.viewKinds = (document?.views || []).map((view) => view?.kind).filter(Boolean);
        entry.diagnosticCount = Array.isArray(document?.diagnostics) ? document.diagnostics.length : 0;
        entry.synthetic = isSyntheticMetricVisualEvidence(provenance);
        entry.nativeExportExplicit = isExplicitNativeMetricVisualExport(provenance);
        entry.native = isNativeMetricVisualDocument(document);
        entry.provenance = summarizeProvenance(provenance);
        if (entry.synthetic) syntheticEvidence.push(entry);
        if (entry.native) nativeEvidence.push(entry);
      } catch (error) {
        entry.error = error instanceof Error ? error.message : String(error);
      }
    }
    resolved.push(entry);
  }
  return {
    usesNativeEvidence: nativeEvidence.length > 0,
    fetchTargets: resolved,
    nativeEvidence,
    syntheticEvidence,
  };
}

async function summarizeGrae10Evidence() {
  const indexPath = resolve(EXAMPLES_DIR, "grae10-metric-engine", "index.html");
  const dataPath = resolve(EXAMPLES_DIR, "grae10-metric-engine", "grae10-data.json");
  const baselinePath = resolve(ROOT, "visual", "regression-baselines", "grae10-metric-engine.sha256");
  const [indexBytes, expectedText, dataText] = await Promise.all([
    readFile(indexPath),
    maybeRead(baselinePath),
    maybeRead(dataPath),
  ]);
  const actualHash = createHash("sha256").update(indexBytes).digest("hex");
  const expectedHash = expectedText.trim().split(/\s+/)[0] || "";
  let recordCount = null;
  let schema = null;
  let dataOk = false;
  try {
    const data = JSON.parse(dataText);
    recordCount = data.recordCount;
    schema = data.schema;
    dataOk = schema === "metric.visual.grae10.dataset.v1"
      && recordCount === 60000
      && Array.isArray(data.p2)
      && data.p2.length === 120000
      && Array.isArray(data.p3)
      && data.p3.length === 180000
      && typeof data.labels === "string"
      && data.labels.length === 60000;
  } catch {
    dataOk = false;
  }
  return {
    usesNativeEvidence: dataOk && actualHash === expectedHash,
    fetchTargets: [{ target: "./grae10-data.json", path: relativeFromRoot(dataPath), exists: existsSync(dataPath), schema, recordCount }],
    nativeEvidence: [],
    syntheticEvidence: [],
    protectedGrae10: {
      ok: dataOk && actualHash === expectedHash,
      expectedHash,
      actualHash,
      dataOk,
      schema,
      recordCount,
    },
  };
}

function countRecordTypes(records) {
  const types = new Set();
  for (const record of records || []) {
    const type = record?.type ?? record?.record_type ?? record?.payload?.kind ?? null;
    if (type != null && String(type).trim()) types.add(String(type));
  }
  return types.size;
}

function summarizeProvenance(provenance) {
  const out = {};
  for (const key of ["writer", "generator", "runtime", "computation", "source", "source_example", "native_export", "synthetic", "synthetic_js", "status", "public_hero_ready"]) {
    if (provenance[key] !== undefined) out[key] = provenance[key];
  }
  return out;
}

function extractFetchTargets(text) {
  const constants = new Map();
  const constantPattern = /const\s+([A-Z0-9_]+)\s*=\s*["']([^"']+)["']/g;
  for (const match of text.matchAll(constantPattern)) constants.set(match[1], match[2]);
  const targets = [];
  const fetchPattern = /fetch\(\s*(?:(["'])([^"']+)\1|([A-Z0-9_]+))\s*\)/g;
  for (const match of text.matchAll(fetchPattern)) {
    if (match[2]) targets.push(match[2]);
    else if (match[3] && constants.has(match[3])) targets.push(constants.get(match[3]));
  }
  return [...new Set(targets)];
}

function resolveFetchTarget(exampleDir, target) {
  if (!target || /^https?:\/\//.test(target)) return null;
  if (target.startsWith("/")) return resolve(ROOT, `.${target}`);
  return resolve(exampleDir, target);
}

async function discoverPublicExamples() {
  const site = await maybeRead(SITE);
  const limit = process.env.METRIC_VISUAL_EXAMPLES
    ? new Set(process.env.METRIC_VISUAL_EXAMPLES.split(",").map((name) => name.trim()).filter(Boolean))
    : null;
  const internalLimit = process.env.METRIC_VISUAL_INTERNAL_EXAMPLES
    ? new Set(process.env.METRIC_VISUAL_INTERNAL_EXAMPLES.split(",").map((name) => name.trim()).filter(Boolean))
    : null;
  const matches = [...site.matchAll(/(?:src|href)="[^"]*visual\/examples\/([^/]+)\/index\.html"/g)].map((match) => match[1]);
  const internal = internalLimit ? [...internalLimit].filter((name) => INTERNAL_ENGINE_EXAMPLES[name]) : [];
  const deduped = [...new Set([...matches, ...EXPLICIT_NATIVE_PREVIEW_EXAMPLES, ...internal])];
  const existing = [];
  for (const name of deduped) {
    if (limit && !limit.has(name) && !internalLimit?.has(name)) continue;
    if (existsSync(resolve(EXAMPLES_DIR, name, "index.html"))) existing.push(name);
  }
  return existing;
}

async function discoverSyntheticFixtures() {
  const entries = await readdir(EXAMPLES_DIR, { withFileTypes: true });
  const fixtures = [];
  for (const entry of entries) {
    if (!entry.isDirectory()) continue;
    const evidencePath = resolve(EXAMPLES_DIR, entry.name, "evidence.json");
    if (!existsSync(evidencePath)) continue;
    try {
      const document = JSON.parse(await readFile(evidencePath, "utf8"));
      if (isSyntheticMetricVisualEvidence(document?.provenance)) {
        fixtures.push({
          example: entry.name,
          path: relativeFromRoot(evidencePath),
          recordCount: Array.isArray(document.records) ? document.records.length : null,
        });
      }
    } catch {
      fixtures.push({ example: entry.name, path: relativeFromRoot(evidencePath), unreadable: true });
    }
  }
  return fixtures.sort((a, b) => a.example.localeCompare(b.example));
}

async function waitForProbe(page) {
  const deadline = Date.now() + READY_TIMEOUT_MS;
  let last = null;
  let lastError = null;
  while (Date.now() < deadline) {
    try {
      last = await evaluate(page, PAGE_PROBE_SCRIPT, { timeoutMs: 30000 });
      if (last?.ready) return last;
    } catch (error) {
      lastError = error instanceof Error ? error.message : String(error);
    }
    await sleep(250);
  }
  throw new Error(`render readiness timed out after ${READY_TIMEOUT_MS}ms: ${lastError || JSON.stringify(last)}`);
}

async function performBasicInteraction(page, probe) {
  const canvas = probe?.largestCanvas;
  if (!canvas?.visible) return { ok: false, reason: "no-visible-canvas" };
  const x = clamp(canvas.x + canvas.width * 0.5, 1, VIEWPORT.width - 2);
  const y = clamp(canvas.y + canvas.height * 0.5, 1, VIEWPORT.height - 2);
  const x2 = clamp(x + Math.min(80, canvas.width * 0.18), 1, VIEWPORT.width - 2);
  const y2 = clamp(y + Math.min(40, canvas.height * 0.12), 1, VIEWPORT.height - 2);
  const before = await evaluate(page, PAGE_PROBE_SCRIPT, { timeoutMs: 30000 });
  await page.send("Input.dispatchMouseEvent", { type: "mouseMoved", x, y });
  await page.send("Input.dispatchMouseEvent", { type: "mousePressed", x, y, button: "left", clickCount: 1 });
  await page.send("Input.dispatchMouseEvent", { type: "mouseMoved", x: x2, y: y2, button: "left", buttons: 1 });
  await page.send("Input.dispatchMouseEvent", { type: "mouseReleased", x: x2, y: y2, button: "left", clickCount: 1 });
  await sleep(350);
  const after = await evaluate(page, PAGE_PROBE_SCRIPT, { timeoutMs: 30000 });
  const stillRenderable = Boolean(after?.largestCanvas?.visible)
    && (Boolean(after?.runtimeState?.runtimeLayerCount) || Boolean(after?.data?.metricGrae10Engine));
  return {
    ok: stillRenderable,
    reason: stillRenderable ? "pointer-drag-kept-scene-alive" : "scene-not-renderable-after-pointer-drag",
    before: before?.runtimeState,
    after: after?.runtimeState,
  };
}

async function captureScreenshot(page, path, options = {}) {
  const params = { format: "png", captureBeyondViewport: false, fromSurface: true };
  if (options.clip) params.clip = options.clip;
  const response = await page.send("Page.captureScreenshot", params);
  await writeFile(path, Buffer.from(response.data, "base64"));
  return path;
}

function clipFromCanvas(canvas) {
  return {
    x: Math.max(0, Math.floor(canvas.x)),
    y: Math.max(0, Math.floor(canvas.y)),
    width: Math.max(1, Math.min(VIEWPORT.width, Math.floor(canvas.width))),
    height: Math.max(1, Math.min(VIEWPORT.height, Math.floor(canvas.height))),
    scale: 1,
  };
}

function analyzePng(buffer) {
  const image = decodePng(buffer);
  const channels = image.channels;
  let count = 0;
  let sum = 0;
  let sumSq = 0;
  let alphaPixels = 0;
  const quantized = new Set();
  const step = Math.max(1, Math.floor((image.width * image.height) / 50000));
  for (let pixel = 0; pixel < image.width * image.height; pixel += step) {
    const offset = pixel * channels;
    const r = image.data[offset];
    const g = channels >= 3 ? image.data[offset + 1] : r;
    const b = channels >= 3 ? image.data[offset + 2] : r;
    const a = channels === 4 ? image.data[offset + 3] : 255;
    if (a > 8) alphaPixels += 1;
    const y = 0.299 * r + 0.587 * g + 0.114 * b;
    sum += y;
    sumSq += y * y;
    count += 1;
    quantized.add(`${r >> 4}:${g >> 4}:${b >> 4}:${a >> 6}`);
    if (quantized.size > 256) break;
  }
  const mean = count ? sum / count : 0;
  const variance = count ? Math.max(0, sumSq / count - mean * mean) : 0;
  const stddev = Math.sqrt(variance);
  return {
    width: image.width,
    height: image.height,
    sampleCount: count,
    alphaPixels,
    luminanceMean: Number(mean.toFixed(3)),
    luminanceStdDev: Number(stddev.toFixed(3)),
    quantizedColors: quantized.size,
    nonBlank: alphaPixels > 0 && (stddev >= 2.5 || quantized.size >= 12),
  };
}

function decodePng(buffer) {
  const signature = "89504e470d0a1a0a";
  if (buffer.subarray(0, 8).toString("hex") !== signature) throw new Error("not a PNG");
  let offset = 8;
  let width = 0;
  let height = 0;
  let bitDepth = 0;
  let colorType = 0;
  const idat = [];
  while (offset < buffer.length) {
    const length = buffer.readUInt32BE(offset);
    const type = buffer.subarray(offset + 4, offset + 8).toString("ascii");
    const data = buffer.subarray(offset + 8, offset + 8 + length);
    if (type === "IHDR") {
      width = data.readUInt32BE(0);
      height = data.readUInt32BE(4);
      bitDepth = data[8];
      colorType = data[9];
    } else if (type === "IDAT") {
      idat.push(data);
    } else if (type === "IEND") {
      break;
    }
    offset += 12 + length;
  }
  if (bitDepth !== 8) throw new Error(`unsupported PNG bit depth ${bitDepth}`);
  const channels = colorType === 6 ? 4 : colorType === 2 ? 3 : colorType === 0 ? 1 : 0;
  if (!channels) throw new Error(`unsupported PNG color type ${colorType}`);
  const raw = inflateSync(Buffer.concat(idat));
  const stride = width * channels;
  const out = Buffer.alloc(stride * height);
  let input = 0;
  let previous = Buffer.alloc(stride);
  for (let y = 0; y < height; y += 1) {
    const filter = raw[input++];
    const line = Buffer.alloc(stride);
    for (let x = 0; x < stride; x += 1) {
      const value = raw[input++];
      const left = x >= channels ? line[x - channels] : 0;
      const up = previous[x] || 0;
      const upLeft = x >= channels ? previous[x - channels] || 0 : 0;
      if (filter === 0) line[x] = value;
      else if (filter === 1) line[x] = (value + left) & 0xff;
      else if (filter === 2) line[x] = (value + up) & 0xff;
      else if (filter === 3) line[x] = (value + Math.floor((left + up) / 2)) & 0xff;
      else if (filter === 4) line[x] = (value + paeth(left, up, upLeft)) & 0xff;
      else throw new Error(`unsupported PNG filter ${filter}`);
    }
    line.copy(out, y * stride);
    previous = line;
  }
  return { width, height, channels, data: out };
}

function paeth(a, b, c) {
  const p = a + b - c;
  const pa = Math.abs(p - a);
  const pb = Math.abs(p - b);
  const pc = Math.abs(p - c);
  if (pa <= pb && pa <= pc) return a;
  if (pb <= pc) return b;
  return c;
}

async function startStaticServer(rootDir) {
  return new Promise((resolveServer, rejectServer) => {
    const server = createServer(async (request, response) => {
      try {
        const url = new URL(request.url, "http://127.0.0.1");
        let pathname = decodeURIComponent(url.pathname);
        if (pathname === "/favicon.ico") {
          response.writeHead(204).end();
          return;
        }
        if (pathname.endsWith("/")) pathname += "index.html";
        const filePath = resolve(rootDir, `.${pathname}`);
        if (!filePath.startsWith(rootDir)) {
          response.writeHead(403).end("forbidden");
          return;
        }
        const data = await readFile(filePath);
        response.writeHead(200, { "content-type": MIME[extname(filePath)] || "application/octet-stream" });
        response.end(data);
      } catch {
        response.writeHead(404).end("not found");
      }
    });
    server.on("error", rejectServer);
    server.listen(0, "127.0.0.1", () => resolveServer(server));
  });
}

class Browser {
  constructor(process, port, userDataDir) {
    this.process = process;
    this.port = port;
    this.userDataDir = userDataDir;
  }

  async newPage() {
    const response = await fetch(`http://127.0.0.1:${this.port}/json/new?about:blank`, { method: "PUT" });
    if (!response.ok) throw new Error(`Chrome target create failed: ${response.status}`);
    const target = await response.json();
    return new CdpPage(target.webSocketDebuggerUrl, this.port, target.id);
  }

  async close() {
    if (this.process.exitCode == null) this.process.kill("SIGTERM");
    await new Promise((resolveClose) => {
      const timer = setTimeout(() => {
        if (this.process.exitCode == null) this.process.kill("SIGKILL");
        resolveClose();
      }, 1200);
      this.process.once("exit", () => {
        clearTimeout(timer);
        resolveClose();
      });
    });
    await rm(this.userDataDir, { recursive: true, force: true });
  }
}

class CdpPage {
  constructor(webSocketUrl, port, targetId) {
    this.webSocketUrl = webSocketUrl;
    this.port = port;
    this.targetId = targetId;
    this.nextId = 1;
    this.pending = new Map();
    this.listeners = new Map();
    this.consoleErrors = [];
    this.pageErrors = [];
    this.closed = false;
    this.socket = new WebSocket(webSocketUrl);
    this.ready = new Promise((resolveReady, rejectReady) => {
      const timer = setTimeout(() => rejectReady(new Error("CDP WebSocket connect timeout")), 10000);
      this.socket.addEventListener("open", () => {
        clearTimeout(timer);
        resolveReady();
      }, { once: true });
      this.socket.addEventListener("error", () => {
        clearTimeout(timer);
        rejectReady(new Error("CDP WebSocket connection failed"));
      }, { once: true });
    });
    this.socket.addEventListener("message", (event) => this.handleMessage(event.data));
    this.socket.addEventListener("close", () => {
      this.closed = true;
      for (const { reject } of this.pending.values()) reject(new Error("CDP WebSocket closed"));
      this.pending.clear();
    });
  }

  async prepare() {
    await this.ready;
    await this.send("Page.enable");
    await this.send("Runtime.enable");
    await this.send("Network.enable");
    await this.send("Log.enable").catch(() => {});
    await this.send("Emulation.setDeviceMetricsOverride", {
      width: VIEWPORT.width,
      height: VIEWPORT.height,
      deviceScaleFactor: 1,
      mobile: false,
    });
    await this.send("Page.addScriptToEvaluateOnNewDocument", { source: GPU_DIAGNOSTICS_SCRIPT });
    this.on("Runtime.consoleAPICalled", (event) => {
      if (event.type === "error") {
        this.consoleErrors.push((event.args || []).map((arg) => arg.value || arg.description || "").join(" "));
      }
    });
    this.on("Runtime.exceptionThrown", (event) => {
      this.pageErrors.push(event.exceptionDetails?.text || event.exceptionDetails?.exception?.description || "exception");
    });
    this.on("Log.entryAdded", (event) => {
      if (event.entry?.level === "error") this.consoleErrors.push(event.entry.text);
    });
  }

  async navigate(url) {
    const domContent = waitForEvent(this, "Page.domContentEventFired", () => true, NAV_TIMEOUT_MS).catch(() => null);
    const result = await this.send("Page.navigate", { url });
    if (result.errorText) throw new Error(`navigation failed: ${result.errorText}`);
    await Promise.race([domContent, sleep(Math.min(2500, NAV_TIMEOUT_MS))]);
  }

  send(method, params = {}) {
    const id = this.nextId++;
    const message = JSON.stringify({ id, method, params });
    const promise = new Promise((resolveSend, rejectSend) => {
      this.pending.set(id, { resolve: resolveSend, reject: rejectSend, method });
    });
    this.socket.send(message);
    return promise;
  }

  on(method, handler) {
    if (!this.listeners.has(method)) this.listeners.set(method, new Set());
    this.listeners.get(method).add(handler);
    return () => this.listeners.get(method)?.delete(handler);
  }

  handleMessage(data) {
    const message = JSON.parse(typeof data === "string" ? data : Buffer.from(data).toString("utf8"));
    if (message.id) {
      const pending = this.pending.get(message.id);
      if (!pending) return;
      this.pending.delete(message.id);
      if (message.error) pending.reject(new Error(`${pending.method}: ${message.error.message}`));
      else pending.resolve(message.result || {});
      return;
    }
    const handlers = this.listeners.get(message.method);
    if (handlers) {
      for (const handler of handlers) handler(message.params || {});
    }
  }

  async close() {
    if (!this.closed) this.socket.close();
    await fetch(`http://127.0.0.1:${this.port}/json/close/${this.targetId}`).catch(() => {});
  }
}

async function launchChrome(executable) {
  const port = await reservePort();
  const userDataDir = await mkdtemp(join(tmpdir(), "metric-visual-cdp-"));
  const child = spawn(executable, [
    `--remote-debugging-port=${port}`,
    `--user-data-dir=${userDataDir}`,
    "--headless=new",
    "--no-first-run",
    "--no-default-browser-check",
    "--disable-background-networking",
    "--enable-webgl",
    "--enable-unsafe-swiftshader",
    "--ignore-gpu-blocklist",
    "--use-angle=swiftshader",
    "about:blank",
  ], { stdio: ["ignore", "ignore", "pipe"] });
  let stderr = "";
  child.stderr.on("data", (chunk) => {
    stderr += chunk.toString();
    if (stderr.length > 8000) stderr = stderr.slice(-8000);
  });
  child.once("exit", (code) => {
    if (code != null && code !== 0) {
      stderr += `\nChrome exited with code ${code}`;
    }
  });
  const deadline = Date.now() + 15000;
  while (Date.now() < deadline) {
    try {
      const response = await fetch(`http://127.0.0.1:${port}/json/version`);
      if (response.ok) return new Browser(child, port, userDataDir);
    } catch {
      // keep polling
    }
    await sleep(150);
  }
  child.kill("SIGTERM");
  await rm(userDataDir, { recursive: true, force: true });
  throw new Error(`Chrome DevTools endpoint did not start. ${stderr.trim()}`);
}

async function evaluate(page, expression, options = {}) {
  const timeoutMs = options.timeoutMs || 10000;
  let timerId = null;
  const timer = new Promise((_, reject) => {
    timerId = setTimeout(() => reject(new Error("Runtime.evaluate timed out")), timeoutMs);
  });
  const call = page.send("Runtime.evaluate", {
    expression,
    awaitPromise: true,
    returnByValue: true,
    userGesture: true,
  });
  const result = await Promise.race([call, timer]).finally(() => clearTimeout(timerId));
  if (result.exceptionDetails) {
    throw new Error(result.exceptionDetails.text || result.exceptionDetails.exception?.description || "evaluation failed");
  }
  return result.result?.value;
}

function waitForEvent(page, method, predicate, timeoutMs) {
  return new Promise((resolveWait, rejectWait) => {
    const timer = setTimeout(() => {
      unsubscribe();
      rejectWait(new Error(`${method} timed out after ${timeoutMs}ms`));
    }, timeoutMs);
    const unsubscribe = page.on(method, (params) => {
      if (!predicate(params)) return;
      clearTimeout(timer);
      unsubscribe();
      resolveWait(params);
    });
  });
}

async function reservePort() {
  return new Promise((resolvePort, rejectPort) => {
    const server = createServer();
    server.on("error", rejectPort);
    server.listen(0, "127.0.0.1", () => {
      const port = server.address().port;
      server.close(() => resolvePort(port));
    });
  });
}

function findChromeExecutable() {
  const home = homedir();
  const cacheCandidates = [
    join(home, "Library", "Caches", "ms-playwright"),
    join(home, ".cache", "ms-playwright"),
  ];
  const chromeForTesting = [];
  for (const cacheRoot of cacheCandidates) {
    if (!existsSync(cacheRoot)) continue;
    chromeForTesting.push(...findChromeForTestingExecutables(cacheRoot));
  }
  const candidates = [
    process.env.METRIC_VISUAL_CHROME,
    ...chromeForTesting,
    "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome",
    "/Applications/Chromium.app/Contents/MacOS/Chromium",
    "/Applications/Microsoft Edge.app/Contents/MacOS/Microsoft Edge",
    "/usr/bin/google-chrome",
    "/usr/bin/chromium",
    "/usr/bin/chromium-browser",
  ].filter(Boolean);
  return candidates.find((candidate) => existsSync(candidate)) || null;
}

function findChromeForTestingExecutables(cacheRoot) {
  const candidates = [];
  const stack = [cacheRoot];
  const maxEntries = 400;
  let visited = 0;
  while (stack.length && visited < maxEntries) {
    visited += 1;
    const current = stack.pop();
    let entries = [];
    try {
      entries = readdirSync(current, { withFileTypes: true });
    } catch {
      continue;
    }
    for (const entry of entries) {
      const full = join(current, entry.name);
      if (entry.isDirectory()) {
        stack.push(full);
      } else if (
        entry.isFile() &&
        (entry.name === "chrome" || entry.name === "Google Chrome for Testing") &&
        full.includes("chrome")
      ) {
        candidates.push(full);
      }
    }
  }
  return candidates.sort().reverse();
}

async function writeReport(report) {
  await writeFile(join(OUT_DIR, "results.json"), JSON.stringify(report, null, 2), "utf8");
}

async function maybeRead(path) {
  try {
    return await readFile(path, "utf8");
  } catch {
    return "";
  }
}

function relativeFromRoot(path) {
  return path ? path.replace(`${ROOT}/`, "") : null;
}

function parseViewport(value) {
  const match = /^(\d+)x(\d+)$/i.exec(value);
  return match ? { width: Number(match[1]), height: Number(match[2]) } : { width: 1280, height: 820 };
}

function clamp(value, min, max) {
  return Math.max(min, Math.min(max, value));
}

function sleep(ms) {
  return new Promise((resolveSleep) => setTimeout(resolveSleep, ms));
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
