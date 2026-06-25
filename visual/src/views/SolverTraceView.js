import { BaseView } from "./BaseView.js";
import { createTrajectoryBundleLayerDescriptor } from "../curves/index.js";
import { resolveCollectionItem } from "./view-utils.js";

const TRACE_KEYS = ["residual", "objective", "loss", "value", "error", "mse", "mse_to_clean", "dirichlet_energy", "energy", "cost"];

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
    this.timelineId = options.timelineId || options.timeline_id || options.metadata?.timelineId || null;
    this.tracePropertyId = options.tracePropertyId || options.propertyId || options.property_id || null;
  }

  static fromVisualSpace(document, options = {}) {
    let series = options.series ? normalizeSeries(options.series) : null;
    const timeline = resolveCollectionItem(document, "timelines", options.timeline || options.timelineId)
      || firstTimeline(document);
    const traceProperty = findTraceProperty(document, timeline, options);

    if (!series && traceProperty && isTimelineStepProperty(traceProperty)) {
      series = seriesFromTimelineStepProperty(traceProperty, timeline);
    }

    if (!series) {
      if (timeline && Array.isArray(timeline.steps)) {
        const fromSteps = timeline.steps
          .map((step, index) => ({ iteration: numberOr(step.iteration, index), value: pickTraceValue(step) }))
          .filter((entry) => Number.isFinite(entry.value));
        if (fromSteps.length > 1) series = fromSteps;
      }
    }

    if (!series) {
      const property = traceProperty || resolveCollectionItem(document, "properties", options.property || options.propertyId);
      const vector = firstVectorValue(property);
      if (vector) series = normalizeSeries(vector);
    }

    return new SolverTraceView({
      ...options,
      series: series || [],
      datasetId: options.datasetId ?? timeline?.dataset_id ?? document?.datasets?.[0]?.id,
      timelineId: timeline?.id,
      propertyId: traceProperty?.id ?? options.propertyId,
      tracePropertyId: traceProperty?.id ?? options.propertyId,
      metadata: {
        ...(options.metadata || {}),
        timelineId: timeline?.id,
        tracePropertyId: traceProperty?.id ?? options.propertyId ?? null,
      },
    });
  }

  toLayerDescriptors() {
    if (this.series.length < 2) return [];
    const summary = this.summary();
    const lastIteration = this.series[this.series.length - 1].iteration || this.series.length - 1;
    const xFor = (iteration) => -this.width / 2 + (iteration / Math.max(1, lastIteration)) * this.width;
    const yFor = (value) => {
      const scaled = this.logScale ? Math.log10(Math.max(value, 1e-12)) : value;
      const min = this.logScale ? Math.log10(Math.max(summary.domain.min, 1e-12)) : summary.domain.min;
      const max = this.logScale ? Math.log10(Math.max(summary.domain.max, 1e-12)) : summary.domain.max;
      const denom = (max - min) || 1;
      return this.groundY + ((scaled - min) / denom) * this.height;
    };

    const pointCount = this.series.length;
    const points = new Float32Array(pointCount * 3);
    const colors = new Float32Array(pointCount * 4);
    const widths = new Float32Array(pointCount);
    const times = new Float32Array(pointCount);
    for (let index = 0; index < pointCount; index += 1) {
      const entry = this.series[index];
      points.set([xFor(entry.iteration), yFor(entry.value), 0], index * 3);
      colors.set(this.color, index * 4);
      widths[index] = 2.4;
      times[index] = entry.iteration;
    }

    const descriptor = createTrajectoryBundleLayerDescriptor({
      paths: [{
        id: `${this.id}:trace-path`,
        points,
        colors,
        widths,
        times,
        count: pointCount,
        evidenceType: "solver-history",
        metadata: {
          traceLabel: this.label,
          timelineId: this.timelineId,
          tracePropertyId: this.tracePropertyId,
          summary,
        },
      }],
    }, {
      id: `${this.id}:trace`,
      order: 4,
      defaultWidth: 2.4,
      alpha: this.color[3] ?? 1,
      flowStrength: 0.08,
      flowScale: 1.4,
      flowSpeed: 0.12,
      ambient: 0.54,
      pointLight: 0.36,
      coreGlow: 0.1,
      depthWrite: false,
      evidenceRole: "timeline-residual-objective",
    });
    descriptor.kind = "solver-trace";
    descriptor.source = {
      ...descriptor.source,
      viewId: this.id,
      viewKind: "solver-trace",
      datasetId: this.datasetId,
      spaceId: this.spaceId,
      coordinateId: this.coordinateId,
      propertyId: this.propertyId,
      relationId: this.relationId,
      traceLabel: this.label,
      timelineId: this.timelineId,
      tracePropertyId: this.tracePropertyId,
    };
    descriptor.metadata = {
      ...descriptor.metadata,
      ...this.metadata,
      role: "solver-trace",
      evidenceRole: "timeline-residual-objective",
      motionGrammar: "solver-objective-timeline-curve",
      traceEvidence: {
        schema: "metric.visual.solver_trace_evidence.v1",
        source: this.tracePropertyId ? "exported-timeline-step-property" : "provided-series",
        viewKind: "solver-trace",
        traceLabel: this.label,
        timelineId: this.timelineId,
        tracePropertyId: this.tracePropertyId,
        seriesCount: this.series.length,
        iterationDomain: {
          min: this.series[0]?.iteration ?? 0,
          max: this.series[this.series.length - 1]?.iteration ?? 0,
        },
        valueDomain: summary.domain,
        algorithmicComputation: false,
      },
      points: this.series.length,
      segmentCount: this.series.length - 1,
      summary,
      logScale: this.logScale,
      timelineId: this.timelineId,
      tracePropertyId: this.tracePropertyId,
      algorithmicComputation: false,
    };
    return [descriptor];
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

function findTraceProperty(document, timeline, options = {}) {
  const explicit = options.property || options.propertyId || options.traceProperty || options.tracePropertyId || options.residualProperty || options.objectiveProperty || options.lossProperty;
  if (explicit) {
    return resolveCollectionItem(document, "properties", explicit, {
      required: true,
      label: "trace property",
    });
  }
  const properties = Array.isArray(document?.properties) ? document.properties : [];
  return properties.find((property) => (
    isTimelineStepProperty(property)
      && tracePropertyLooksRelevant(property)
      && propertyHasTimelineValues(property, timeline)
  )) || null;
}

function seriesFromTimelineStepProperty(property, timeline) {
  const values = Array.isArray(property?.values)
    ? property.values
    : Array.isArray(property?.record_values)
      ? property.record_values
      : [];
  if (!values.length) return null;
  const timelineId = timeline?.id ?? null;
  const stepTimes = new Map();
  const stepIndexes = new Map();
  for (let order = 0; order < (timeline?.steps || []).length; order += 1) {
    const step = timeline.steps[order] || {};
    const index = numberOr(step.index ?? step.step_index ?? step.step, order);
    const time = numberOr(step.time ?? step.t ?? step.seconds, index);
    stepTimes.set(String(index), time);
    if (step.id != null) stepTimes.set(String(step.id), time);
    if (step.coordinate_id != null) stepTimes.set(String(step.coordinate_id), time);
    if (step.id != null) stepIndexes.set(String(step.id), index);
  }
  const series = [];
  for (let order = 0; order < values.length; order += 1) {
    const entry = values[order];
    if (entry && typeof entry === "object") {
      const entryTimelineId = entry.timeline_id ?? entry.timelineId;
      if (timelineId != null && entryTimelineId != null && String(entryTimelineId) !== String(timelineId)) continue;
      const stepRef = entry.step_id ?? entry.stepId ?? entry.coordinate_id ?? entry.coordinateId;
      const index = numberOr(entry.index ?? entry.step_index ?? entry.step ?? stepIndexes.get(String(stepRef)), order);
      const iteration = numberOr(entry.time ?? entry.t ?? entry.seconds ?? stepTimes.get(String(stepRef)) ?? stepTimes.get(String(index)), index);
      const value = pickTraceValue(entry);
      if (Number.isFinite(value)) series.push({ iteration, value });
      continue;
    }
    const value = Number(entry);
    if (Number.isFinite(value)) series.push({ iteration: order, value });
  }
  series.sort((a, b) => a.iteration - b.iteration);
  return series.length > 1 ? series : null;
}

function isTimelineStepProperty(property) {
  const target = property?.target_type ?? property?.targetType ?? property?.target;
  return String(target || "").replace(/-/g, "_") === "timeline_step";
}

function tracePropertyLooksRelevant(property) {
  const text = `${property?.id || ""} ${property?.name || ""}`.toLowerCase();
  return /(residual|objective|loss|error|mse|energy|cost)/.test(text);
}

function propertyHasTimelineValues(property, timeline) {
  if (!timeline?.id) return true;
  const values = Array.isArray(property?.values) ? property.values : [];
  return values.some((entry) => {
    const entryTimelineId = entry?.timeline_id ?? entry?.timelineId;
    return entryTimelineId == null || String(entryTimelineId) === String(timeline.id);
  });
}

function firstTimeline(document) {
  const timelines = Array.isArray(document?.timelines) ? document.timelines : [];
  return timelines[0] || null;
}

function numberOr(value, fallback) {
  const number = Number(value);
  return Number.isFinite(number) ? number : fallback;
}
