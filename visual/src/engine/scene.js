import { disposeResource } from "./resources.js";

export class VisualScene {
  constructor(options = {}) {
    this.children = [];
    this.backgroundColor = new Float32Array(options.backgroundColor || [0.02, 0.025, 0.03, 1]);
    this.userData = options.userData || {};
    this.needsSort = false;
    this.disposed = false;
  }

  add(child) {
    if (!child || this.children.includes(child)) return child;
    this.children.push(child);
    child.scene = this;
    this.needsSort = true;
    return child;
  }

  remove(child) {
    const index = this.children.indexOf(child);
    if (index === -1) return child;
    this.children.splice(index, 1);
    if (child.scene === this) {
      child.scene = null;
    }
    return child;
  }

  clear({ dispose = false } = {}) {
    const children = this.children.slice();
    this.children.length = 0;
    for (const child of children) {
      if (child.scene === this) child.scene = null;
      if (dispose) disposeResource(child);
    }
    return this;
  }

  setBackground(color) {
    this.backgroundColor.set(color);
    return this;
  }

  update(context) {
    if (this.needsSort) {
      this.sort();
    }
    for (const child of this.children) {
      if (child.enabled === false) continue;
      if (typeof child.update === "function") {
        child.update(context);
      }
    }
    return this;
  }

  render(context, options = {}) {
    if (this.needsSort) {
      this.sort();
    }
    const phase = options.phase || context.renderPhase || "scene";
    for (const child of this.children) {
      if (child.enabled === false || child.visible === false) continue;
      if (phase !== "all" && resolveChildRenderPhase(child) !== phase) continue;
      if (typeof child.render === "function") {
        child.render(context);
      }
    }
    return this;
  }

  sort() {
    this.children.sort((a, b) => effectiveRenderOrder(a) - effectiveRenderOrder(b));
    this.needsSort = false;
    return this;
  }

  dispose() {
    if (this.disposed) return;
    this.clear({ dispose: true });
    this.disposed = true;
  }
}

function resolveChildRenderPhase(child) {
  return child.renderPhase
    || child.descriptor?.renderPhase
    || child.descriptor?.metadata?.renderPhase
    || child.descriptor?.metadata?.postprocessGroup
    || child.descriptor?.material?.renderPhase
    || child.descriptor?.material?.postprocessGroup
    || "scene";
}

function effectiveRenderOrder(child) {
  const local = Number.isFinite(Number(child?.renderOrder)) ? Number(child.renderOrder) : 0;
  const priority = semanticRenderPriority(child);
  return priority == null ? local : priority + local * 0.001;
}

function semanticRenderPriority(child) {
  const primitive = String(child?.primitive || child?.descriptor?.primitive || child?.kind || child?.descriptor?.kind || "");
  const metadata = child?.metadata || child?.descriptor?.metadata || {};
  const role = String(metadata.role || child?.descriptor?.source?.role || "");
  const evidenceRole = String(metadata.evidenceRole || "");
  const stateRole = String(metadata.stateHistoryRole || metadata.timelineContextRole || "");
  const animationMode = String(child?.animation?.mode || child?.descriptor?.animation?.mode || "");
  const roleText = `${role} ${evidenceRole} ${stateRole}`.toLowerCase();

  if (primitive === "BillboardLabelLayer" || roleText.includes("label")) return 300;
  if (
    roleText.includes("current-timeline-state")
    || roleText.includes("current-exported-timeline-state")
    || roleText.includes("current-state")
    || animationMode === "coordinate-morph"
  ) {
    return 100;
  }
  if (
    roleText.includes("trajectory/path")
    || roleText.includes("timeline-state-history")
    || metadata.viewClass === "TrajectoryPathView"
    || primitive === "CurveRibbonLayer"
    || primitive === "CurveTubeMeshLayer"
  ) {
    return 20;
  }
  if (
    primitive === "HeatFieldLayer"
    || primitive === "GroundProjectionLayer"
    || roleText.includes("property-field")
    || roleText.includes("dominant-anomaly-field")
    || roleText.includes("projection")
    || roleText.includes("field")
  ) {
    return -120;
  }
  return null;
}
