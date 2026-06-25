#!/usr/bin/env node

import { readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

import {
  TimelineControlWidget,
  findTimelineControlDescriptor,
} from "../src/interaction/index.js";
import { MetricVisualSurface } from "../src/metric-visual.js";
import { DynamicsView } from "../src/views/DynamicsView.js";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");

async function main() {
  const document = JSON.parse(await readFile(
    resolve(ROOT, "docs/examples/assets/dynamics-noise/metric.visual.json"),
    "utf8",
  ));
  const widgetSource = await readFile(resolve(ROOT, "visual/src/interaction/timeline-control.js"), "utf8");
  const dom = createFakeDom();
  const previousDocument = globalThis.document;
  const previousWindow = globalThis.window;
  globalThis.document = dom.document;
  globalThis.window = dom.window;

  try {
    const checks = [];
    const view = DynamicsView.fromVisualSpace(document, {
      timelineId: "reverse-reconstruction",
      timelineFieldPropertyId: "reverse-mse-to-clean",
      propertyField: "best-reconstruction-error",
    });
    const descriptors = view.toLayerDescriptors();
    const control = findTimelineControlDescriptor(descriptors);
    const root = dom.document.createElement("div");
    dom.document.body.appendChild(root);
    const changes = [];
    const widget = new TimelineControlWidget({
      root,
      descriptor: control,
      clock: {
        now: () => 0,
        request: null,
        cancel: null,
      },
      onTimelineChange: (state) => changes.push(state),
    });

    widget.setValue(0.5, { source: "check-scrub-middle", dispatch: true });
    const middleState = changes.at(-1);
    widget.setValue(0.26, { source: "check-scrub-snap", dispatch: true });
    const snappedState = changes.at(-1);
    widget.play({ startClock: false, source: "check-play" });
    const playingState = widget.getState();
    const playingDataset = widget.element.dataset.playing;
    widget.reset({ source: "check-reset" });
    const resetState = changes.at(-1);

    const surfaceRoot = dom.document.createElement("div");
    dom.document.body.appendChild(surfaceRoot);
    const canvas = dom.document.createElement("canvas");
    canvas.parentElement = surfaceRoot;
    surfaceRoot.appendChild(canvas);
    const runtime = new FakeRuntime();
    const surface = new MetricVisualSurface({
      document,
      canvas,
      runtime,
      setup: {
        stage: { grounding: { groundY: -0.58 } },
        style: {
          setStyleMotion() {},
          detachStyleMotion() {},
        },
      },
      options: {},
    });
    surface.showDynamics({
      timelineId: "reverse-reconstruction",
      timelineFieldPropertyId: "reverse-mse-to-clean",
      propertyField: "best-reconstruction-error",
      preview: false,
      timelineControl: { root: surfaceRoot },
    });
    surface.timelineControlWidget.setValue(0.5, { source: "check-surface-scrub", dispatch: true });
    const surfacePoint = surface.descriptors.find((descriptor) => descriptor.primitive === "InstancedPointLayer");
    const surfaceField = surface.descriptors.find((descriptor) => descriptor.primitive === "HeatFieldLayer"
      && descriptor.source?.viewClass === "PropertyFieldView");

    checks.push(
      ["widget discovers timeline-control descriptor from DynamicsView descriptors", control?.schema === "metric.visual.timeline_control.v1", control],
      ["widget attaches DOM element to provided root", root.children.includes(widget.element), root.children.length],
      ["widget exposes scrubber/play/reset controls", widget.rangeInput?.type === "range" && widget.playButton?.type === "button" && widget.resetButton?.type === "button", widget.element],
      ["widget scrub emits exported middle state when descriptor sample is exact", middleState?.exportedTimelineState?.activeCoordinateId === "coord-reverse-20" && middleState?.activePropertySample?.algorithmicComputation === false, middleState],
      ["widget snaps scrub values to nearest exported-step range", snappedState?.normalized === 0.25, snappedState],
      ["widget state remains exported-timeline bound", middleState?.source === "exported-timeline" && middleState?.selection === "nearest-exported-step" && middleState?.algorithmicComputation === false, middleState],
      ["widget exposes playback state", playingState.playing === true && playingDataset === "true", playingState],
      ["widget reset pauses playback and returns to exported start", resetState?.playing === false && resetState?.normalized === 0 && resetState?.activeCoordinateId === "coord-reverse-00", resetState],
      ["surface attaches runtime timeline widget for dynamics", surface.timelineControlWidget instanceof TimelineControlWidget && surfaceRoot.children.includes(surface.timelineControlWidget.element), surfaceRoot.children.length],
      ["surface scrub rebuilds dynamics through exported timeline progress", surface.views[0]?.activeStep === 20 && surfacePoint?.metadata?.activeCoordinateId === "coord-reverse-20", surfacePoint?.metadata],
      ["surface field follows exported timeline property after widget scrub", surfaceField?.source?.coordinateId === "coord-reverse-20" && surfaceField?.source?.propertyId === "best-reconstruction-error" && surfacePoint?.metadata?.timelineFieldState?.source === "exported-property" && Number(surfacePoint?.metadata?.timelineFieldState?.value).toFixed(7) === "0.0050846", { field: surfaceField?.source, timelineFieldState: surfacePoint?.metadata?.timelineFieldState }],
      ["widget source does not compute removed dynamics algorithms", !/\b(?:diffusion|Redif|redif|denoise)\b/.test(widgetSource), null],
    );

    widget.dispose();
    checks.push(["widget dispose removes DOM element", !root.children.includes(widget.element), root.children.length]);
    report(checks);
  } finally {
    globalThis.document = previousDocument;
    globalThis.window = previousWindow;
  }
}

class FakeRuntime {
  constructor() {
    this.layers = [];
    this.descriptors = [];
    this.events = [];
  }

  setLayerDescriptors(descriptors) {
    this.descriptors = descriptors.slice();
    this.layers = this.descriptors.map((descriptor) => ({ id: descriptor.id, descriptor }));
  }

  renderOnce() {}

  start() {
    this.started = true;
  }

  setCameraOptions(options) {
    this.cameraOptions = { ...options };
  }

  getState() {
    return {
      layerInstanceCount: this.layers.length,
      layerState: { count: this.layers.length },
    };
  }

  on() {
    return () => {};
  }

  emit(type, payload) {
    this.events.push({ type, payload });
  }
}

function createFakeDom() {
  const document = new FakeDocument();
  const window = {
    innerWidth: 960,
    innerHeight: 640,
    devicePixelRatio: 1,
    requestAnimationFrame: null,
    cancelAnimationFrame: null,
  };
  return { document, window };
}

class FakeDocument {
  constructor() {
    this.documentElement = new FakeElement("html", this);
    this.head = new FakeElement("head", this);
    this.body = new FakeElement("body", this);
    this.documentElement.append(this.head, this.body);
  }

  createElement(tag) {
    return new FakeElement(tag, this);
  }

  getElementById(id) {
    return findElementById(this.documentElement, id);
  }
}

class FakeElement {
  constructor(tag, ownerDocument) {
    this.ownerDocument = ownerDocument;
    this.tagName = String(tag).toUpperCase();
    this.children = [];
    this.parentNode = null;
    this.parentElement = null;
    this.dataset = {};
    this.attributes = {};
    this.listeners = new Map();
    this.className = "";
    this.textContent = "";
    this.type = "";
    this.value = "";
    this.min = "";
    this.max = "";
    this.step = "";
    this.title = "";
    this.id = "";
    this.style = {
      values: {},
      setProperty: (name, value) => {
        this.style.values[name] = value;
      },
    };
  }

  appendChild(child) {
    if (!child) return child;
    if (child.parentNode) child.remove();
    child.parentNode = this;
    child.parentElement = this;
    this.children.push(child);
    return child;
  }

  append(...children) {
    for (const child of children) this.appendChild(child);
  }

  replaceChildren(...children) {
    for (const child of this.children) {
      child.parentNode = null;
      child.parentElement = null;
    }
    this.children = [];
    this.append(...children);
  }

  remove() {
    if (!this.parentNode) return;
    const siblings = this.parentNode.children;
    const index = siblings.indexOf(this);
    if (index >= 0) siblings.splice(index, 1);
    this.parentNode = null;
    this.parentElement = null;
  }

  setAttribute(name, value) {
    this.attributes[name] = String(value);
    if (name === "id") this.id = String(value);
  }

  getAttribute(name) {
    return this.attributes[name] ?? null;
  }

  addEventListener(type, listener) {
    if (!this.listeners.has(type)) this.listeners.set(type, new Set());
    this.listeners.get(type).add(listener);
  }

  removeEventListener(type, listener) {
    this.listeners.get(type)?.delete(listener);
  }

  dispatchEvent(event) {
    event.target ||= this;
    event.currentTarget = this;
    for (const listener of this.listeners.get(event.type) || []) listener(event);
    return true;
  }

  getBoundingClientRect() {
    return { left: 0, top: 0, width: 960, height: 640 };
  }
}

function findElementById(element, id) {
  if (element.id === id) return element;
  for (const child of element.children || []) {
    const found = findElementById(child, id);
    if (found) return found;
  }
  return null;
}

function report(checks) {
  const failures = checks
    .filter(([, ok]) => !ok)
    .map(([message, , details]) => ({ message, details }));
  console.log(JSON.stringify({
    ok: failures.length === 0,
    total: checks.length,
    failed: failures.length,
    failures,
  }, null, 2));
  if (failures.length) process.exitCode = 1;
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
