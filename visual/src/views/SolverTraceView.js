import { BaseView } from "./BaseView.js";
import { VisualLayer } from "./VisualLayer.js";
import { createChannel, descriptorSource, resolveCollectionItem } from "./view-utils.js";

const TRACE_KEYS = ["residual", "objective", "loss", "value", "error"];

/**
 * SolverTraceView renders a solver/optimization trace (residuals, objective or
 * loss over iterations) exported by METRIC. It lifts the trace onto the stage as
 * a polyline so convergence reads as a falling curve, and exposes the numeric
 * trace + convergence summary for panels. It does not run a solver.
 */
export class SolverTraceView extends BaseView {
  constructor(options = {}) {
    super({ ...options, kind: "solver-trace" });
    this.series = normalizeSeries(options.series);
    this.label = options.traceLabel || options.label || "residual";
    this.logScale = Boolean(options.logScale);
    this.groundY = Number.isFinite(Number(options.groundY)) ? Number(options.groundY) : 0;
    this.width = Number.isFinite(Number(options.width)) ? Number(options.width) : 3.2;
    this.height = Number.isFinite(Number(options.height)) ? Number(options.height) : 1.8;
    this.color = options.color || [0.94, 0.62, 0.28, 1];
  }

  static fromVisualSpace(document, options = {}) {
    let series = options.series ? normalizeSeries(options.series) : null;

    if (!series) {
      const timeline = resolveCollectionItem(document, "timelines", options.timeline || options.timelineId)
        || firstTimeline(document);
      if (timeline && Array.isArray(timeline.steps)) {
        const fromSteps = timeline.steps
          .map((step, index) => ({ iteration: numberOr(step.iteration, index), value: pickTraceValue(step) }))
          .filter((entry) => Number.isFinite(entry.value));
        if (fromSteps.length > 1) series = fromSteps;
      }
    }

    if (!series) {
      const property = resolveCollectionItem(document, "properties", options.property || options.propertyId);
      const vector = firstVectorValue(property);
      if (vector) series = normalizeSeries(vector);
    }

    return new SolverTraceView({ ...options, series: series || [], datasetId: options.datasetId ?? document?.datasets?.[0]?.id });
  }

  toLayerDescriptors() {
    if (this.series.length < 2) return [];
    const summary = this.summary();
    const span = summary.domain.max - summary.domain.min || 1;
    const lastIteration = this.series[this.series.length - 1].iteration || this.series.length - 1;
    const xFor = (iteration) => -this.width / 2 + (iteration / Math.max(1, lastIteration)) * this.width;
    const yFor = (value) => {
      const scaled = this.logScale ? Math.log10(Math.max(value, 1e-12)) : value;
      const min = this.logScale ? Math.log10(Math.max(summary.domain.min, 1e-12)) : summary.domain.min;
      const max = this.logScale ? Math.log10(Math.max(summary.domain.max, 1e-12)) : summary.domain.max;
      const denom = (max - min) || 1;
      return this.groundY + ((scaled - min) / denom) * this.height;
    };

    const edgeCount = this.series.length - 1;
    const sourcePosition = new Float32Array(edgeCount * 3);
    const targetPosition = new Float32Array(edgeCount * 3);
    const color = new Float32Array(edgeCount * 4);
    for (let index = 0; index < edgeCount; index += 1) {
      const a = this.series[index];
      const b = this.series[index + 1];
      sourcePosition.set([xFor(a.iteration), yFor(a.value), 0], index * 3);
      targetPosition.set([xFor(b.iteration), yFor(b.value), 0], index * 3);
      color.set(this.color, index * 4);
    }

    return [new VisualLayer({
      id: `${this.id}:trace`,
      kind: "solver-trace",
      primitive: "RelationEdgeLayer",
      order: 4,
      source: descriptorSource(this, { viewKind: "solver-trace", traceLabel: this.label }),
      channels: {
        sourcePosition: createChannel(sourcePosition, 3, "source-position"),
        targetPosition: createChannel(targetPosition, 3, "target-position"),
        color: createChannel(color, 4, "rgba"),
      },
      geometry: { width: 2 },
      material: { alpha: 1, transparent: true, depthWrite: false },
      metadata: { ...this.metadata, role: "solver-trace", points: this.series.length, summary, logScale: this.logScale },
    }).toDescriptor()];
  }

  /** Convergence summary for panels. */
  summary() {
    const values = this.series.map((entry) => entry.value).filter((value) => Number.isFinite(value));
    if (!values.length) return { count: 0, domain: { min: 0, max: 0 }, final: null, converged: false };
    let min = Infinity;
    let max = -Infinity;
    for (const value of values) {
      min = Math.min(min, value);
      max = Math.max(max, value);
    }
    const first = values[0];
    const final = values[values.length - 1];
    return {
      count: values.length,
      domain: { min, max },
      first,
      final,
      reduction: first !== 0 ? final / first : 0,
      converged: Number.isFinite(final) && Math.abs(final) <= Math.abs(first) * 0.01,
    };
  }
}

export function createSolverTraceView(options) {
  return new SolverTraceView(options);
}

function normalizeSeries(series) {
  if (!Array.isArray(series)) return [];
  return series
    .map((entry, index) => {
      if (typeof entry === "number") return { iteration: index, value: entry };
      if (entry && typeof entry === "object") {
        return { iteration: numberOr(entry.iteration ?? entry.step ?? entry.x, index), value: Number(pickTraceValue(entry)) };
      }
      return { iteration: index, value: Number(entry) };
    })
    .filter((entry) => Number.isFinite(entry.value));
}

function pickTraceValue(entry) {
  if (entry == null) return NaN;
  if (typeof entry === "number") return entry;
  for (const key of TRACE_KEYS) {
    if (Number.isFinite(Number(entry[key]))) return Number(entry[key]);
  }
  return NaN;
}

function firstVectorValue(property) {
  if (!property || !Array.isArray(property.values)) return null;
  for (const entry of property.values) {
    const candidate = entry?.values ?? entry?.value ?? entry;
    if (Array.isArray(candidate)) return candidate;
  }
  return null;
}

function firstTimeline(document) {
  const timelines = Array.isArray(document?.timelines) ? document.timelines : [];
  return timelines[0] || null;
}

function numberOr(value, fallback) {
  const number = Number(value);
  return Number.isFinite(number) ? number : fallback;
}
