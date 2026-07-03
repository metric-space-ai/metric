const TIMELINE_CONTROL_SCHEMA = "metric.visual.timeline_control.v1";
const TIMELINE_WIDGET_STATE_SCHEMA = "metric.visual.timeline_widget_state.v1";
const STYLE_ID = "mtrc-timeline-control-style";

export class TimelineControlWidget {
  constructor(options = {}) {
    this.className = options.className || "mtrc-timeline-control";
    this.root = options.root || defaultRoot();
    this.runtime = options.runtime || null;
    this.onTimelineChange = typeof options.onTimelineChange === "function" ? options.onTimelineChange : null;
    this.clock = normalizeClock(options.clock);
    this.events = new Map();
    this.playbackHandle = null;
    this.playbackDirection = 1;
    this.lastPlaybackTime = 0;
    this.value = 0;
    this.playing = false;
    this.descriptor = null;
    this.element = options.element || createElement("section");
    if (this.element) {
      this.element.className = this.className;
      this.element.setAttribute("aria-label", options.ariaLabel || "Timeline controls");
      this.element.dataset.visible = "false";
    }
    this.setDescriptor(options.descriptor || options.control || findTimelineControlDescriptor(options.source), {
      render: false,
      preservePlayback: false,
    });
    if (options.autoAttach !== false) this.attach(this.root);
    this.render();
  }

  on(type, listener) {
    if (typeof listener !== "function") return () => {};
    if (!this.events.has(type)) this.events.set(type, new Set());
    this.events.get(type).add(listener);
    return () => this.off(type, listener);
  }

  off(type, listener) {
    this.events.get(type)?.delete(listener);
    return this;
  }

  emit(type, payload) {
    const listeners = this.events.get(type);
    if (!listeners) return this;
    for (const listener of listeners) listener(payload, this);
    return this;
  }

  attach(root = this.root) {
    if (!this.element || !root?.appendChild) return this;
    this.root = root;
    const floating = root === root.ownerDocument?.body || root === documentBody();
    this.element.dataset.floating = floating ? "true" : "false";
    if (this.element.parentNode !== root) root.appendChild(this.element);
    installTimelineControlStyle(this.element.ownerDocument || documentObject());
    return this;
  }

  setRoot(root) {
    return this.attach(root || this.root);
  }

  setRuntime(runtime) {
    this.runtime = runtime || null;
    return this;
  }

  setOnTimelineChange(listener) {
    this.onTimelineChange = typeof listener === "function" ? listener : null;
    return this;
  }

  setDescriptor(descriptor, options = {}) {
    const next = normalizeControlDescriptor(descriptor);
    const previousPlaying = this.playing;
    this.descriptor = next;
    this.value = normalizeTimelineValue(next?.value ?? next?.state?.normalized ?? 0, next);
    this.playing = options.preservePlayback !== false && previousPlaying
      ? previousPlaying
      : Boolean(playbackControl(next)?.value);
    if (!next) this.pause({ dispatch: false });
    if (options.render !== false) this.render();
    return this;
  }

  render() {
    if (!this.element) return this;
    installTimelineControlStyle(this.element.ownerDocument || documentObject());
    this.element.replaceChildren?.();
    this.element.dataset.visible = this.descriptor ? "true" : "false";
    if (!this.descriptor) return this;

    const header = createElement("header");
    const title = createElement("strong");
    title.textContent = stringValue(this.descriptor.label || "Timeline");
    this.readoutElement = createElement("span");
    header.append(title, this.readoutElement);

    const row = createElement("div");
    row.className = `${this.className}__row`;

    this.playButton = createElement("button");
    this.playButton.type = "button";
    this.playButton.className = `${this.className}__button`;
    this.playButton.addEventListener("click", () => this.toggle({ source: "timeline-widget-playback" }));

    this.rangeInput = createElement("input");
    this.rangeInput.type = "range";
    this.rangeInput.className = `${this.className}__scrubber`;
    this.rangeInput.min = String(controlRange(this.descriptor).min);
    this.rangeInput.max = String(controlRange(this.descriptor).max);
    this.rangeInput.step = String(controlRange(this.descriptor).step);
    this.rangeInput.setAttribute("aria-label", rangeControl(this.descriptor)?.ariaLabel || `${this.descriptor.label || "Timeline"} position`);
    this.rangeInput.addEventListener("input", () => {
      this.setValue(this.rangeInput.value, { source: "timeline-widget-scrub", dispatch: true });
    });
    this.rangeInput.addEventListener("change", () => {
      this.dispatchTimelineChange({ source: "timeline-widget-scrub-commit", commit: true });
    });

    this.resetButton = createElement("button");
    this.resetButton.type = "button";
    this.resetButton.className = `${this.className}__button`;
    this.resetButton.textContent = resetControl(this.descriptor)?.label || "Reset";
    this.resetButton.setAttribute("aria-label", resetControl(this.descriptor)?.ariaLabel || "Reset timeline");
    this.resetButton.addEventListener("click", () => this.reset({ source: "timeline-widget-reset" }));

    row.append(this.playButton, this.rangeInput, this.resetButton);

    this.marksElement = createElement("div");
    this.marksElement.className = `${this.className}__marks`;
    this.markButtons = [];
    for (const mark of timelineMarks(this.descriptor)) {
      const button = createElement("button");
      button.type = "button";
      button.className = `${this.className}__mark`;
      button.textContent = stringValue(mark.label || mark.activeLabel || "");
      button.dataset.value = String(mark.normalized ?? mark.value ?? 0);
      button.title = timelineMarkTitle(mark);
      button.addEventListener("click", () => {
        this.setValue(mark.normalized ?? mark.value ?? 0, { source: "timeline-widget-mark", dispatch: true });
      });
      this.markButtons.push(button);
      this.marksElement.appendChild(button);
    }

    this.element.append(header, row);
    if (this.markButtons.length) this.element.appendChild(this.marksElement);
    this.syncControls();
    return this;
  }

  setValue(value, options = {}) {
    const next = normalizeTimelineValue(value, this.descriptor);
    const changed = Math.abs(next - this.value) > 1e-9;
    this.value = next;
    if (options.playing !== undefined) this.playing = Boolean(options.playing);
    this.syncControls();
    if (options.dispatch !== false && (changed || options.force)) {
      this.dispatchTimelineChange({ source: options.source || "timeline-widget", commit: options.commit === true });
    }
    return this;
  }

  reset(options = {}) {
    this.pause({ dispatch: false });
    return this.setValue(controlRange(this.descriptor).min, {
      source: options.source || "timeline-widget-reset",
      dispatch: options.dispatch !== false,
      force: true,
    });
  }

  play(options = {}) {
    if (!this.descriptor) return this;
    this.playing = true;
    this.syncControls();
    if (options.dispatch !== false) {
      this.dispatchTimelineChange({ source: options.source || "timeline-widget-play", playback: true });
    }
    if (options.startClock !== false) this.startPlaybackLoop();
    return this;
  }

  pause(options = {}) {
    const wasPlaying = this.playing;
    this.playing = false;
    this.stopPlaybackLoop();
    this.syncControls();
    if (wasPlaying && options.dispatch !== false) {
      this.dispatchTimelineChange({ source: options.source || "timeline-widget-pause", playback: true });
    }
    return this;
  }

  toggle(options = {}) {
    return this.playing ? this.pause(options) : this.play(options);
  }

  startPlaybackLoop() {
    if (this.playbackHandle != null || !this.clock.request) return this;
    this.lastPlaybackTime = this.clock.now();
    this.playbackHandle = this.clock.request((time) => this.advancePlayback(time));
    return this;
  }

  stopPlaybackLoop() {
    if (this.playbackHandle != null && this.clock.cancel) this.clock.cancel(this.playbackHandle);
    this.playbackHandle = null;
    return this;
  }

  advancePlayback(time) {
    this.playbackHandle = null;
    if (!this.playing || !this.descriptor) return this;
    const now = Number.isFinite(Number(time)) ? Number(time) : this.clock.now();
    const delta = Math.max(0, now - this.lastPlaybackTime);
    this.lastPlaybackTime = now;
    const range = controlRange(this.descriptor);
    const span = Math.max(1e-9, range.max - range.min);
    const playback = this.descriptor.playback || {};
    const durationMs = timelinePlaybackDurationMs(this.descriptor);
    const rate = Number.isFinite(Number(playback.playbackRate)) ? Math.max(0.001, Number(playback.playbackRate)) : 1;
    let next = this.value + (delta / durationMs) * span * rate * this.playbackDirection;
    if (next >= range.max || next <= range.min) {
      if (playback.loop) {
        if (playback.direction === "alternate") {
          if (next >= range.max) {
            next = range.max - (next - range.max);
            this.playbackDirection = -1;
          } else {
            next = range.min + (range.min - next);
            this.playbackDirection = 1;
          }
        } else {
          next = wrapRange(next, range.min, range.max);
        }
      } else {
        next = next >= range.max ? range.max : range.min;
        this.pause({ dispatch: true, source: "timeline-widget-playback-end" });
      }
    }
    this.setValue(next, { source: "timeline-widget-playback", dispatch: true });
    if (this.playing && this.clock.request) this.playbackHandle = this.clock.request((nextTime) => this.advancePlayback(nextTime));
    return this;
  }

  syncControls() {
    if (!this.element || !this.descriptor) return this;
    const state = this.getState();
    this.element.dataset.timelineId = stringValue(state.timelineId || "");
    this.element.dataset.value = formatValue(state.normalized);
    this.element.dataset.playing = this.playing ? "true" : "false";
    this.element.style?.setProperty?.("--mtrc-timeline-progress", `${Math.round(state.normalized * 10000) / 100}%`);
    if (this.rangeInput) {
      this.rangeInput.value = formatValue(state.normalized);
      this.rangeInput.setAttribute("aria-valuetext", state.valueLabel || "");
    }
    if (this.readoutElement) this.readoutElement.textContent = state.valueLabel || "";
    if (this.playButton) {
      this.playButton.textContent = this.playing
        ? (playbackControl(this.descriptor)?.pauseLabel || "Pause")
        : (playbackControl(this.descriptor)?.label || "Play");
      this.playButton.setAttribute("aria-label", this.playing ? "Pause timeline" : "Play timeline");
      this.playButton.dataset.active = this.playing ? "true" : "false";
    }
    for (const button of this.markButtons || []) {
      const markValue = Number(button.dataset.value);
      button.dataset.active = Math.abs(markValue - state.normalized) <= controlRange(this.descriptor).step / 2 ? "true" : "false";
    }
    return this;
  }

  getState() {
    const exportedState = currentExportedState(this.descriptor, this.value);
    const mark = currentTimelineMark(this.descriptor, this.value);
    return {
      schema: TIMELINE_WIDGET_STATE_SCHEMA,
      timelineId: this.descriptor?.timelineId || null,
      datasetId: this.descriptor?.datasetId || null,
      normalized: this.value,
      value: this.value,
      playing: this.playing,
      valueLabel: timelineWidgetValueLabel(this.descriptor, this.value, exportedState, mark),
      exportedTimelineState: exportedState,
      activeCoordinateId: exportedState?.activeCoordinateId ?? mark?.activeCoordinateId ?? null,
      activePropertyId: exportedState?.activePropertyId ?? mark?.activePropertyId ?? null,
      activePropertySample: exportedState?.activePropertySample ?? mark?.activePropertySample ?? null,
      binding: rangeControl(this.descriptor)?.binding || null,
      stateTarget: rangeControl(this.descriptor)?.binding?.stateTarget || "timeline.state",
      selection: rangeControl(this.descriptor)?.binding?.selection || this.descriptor?.evidence?.selection || null,
      source: this.descriptor?.evidence?.source || rangeControl(this.descriptor)?.binding?.source || "exported-timeline",
      algorithmicComputation: false,
    };
  }

  dispatchTimelineChange(detail = {}) {
    const state = {
      ...this.getState(),
      detail,
      descriptor: this.descriptor,
      control: this.descriptor,
      range: controlRange(this.descriptor),
    };
    this.runtime?.emit?.("timelinecontrolchange", {
      runtime: this.runtime,
      widget: this,
      state,
      descriptor: this.descriptor,
    });
    this.onTimelineChange?.(state, this);
    this.emit("change", state);
    return state;
  }

  dispose() {
    this.pause({ dispatch: false });
    this.events.clear();
    this.element?.remove?.();
    this.element = null;
  }
}

export function createTimelineControlWidget(options = {}) {
  return new TimelineControlWidget(options);
}

export function findTimelineControlDescriptor(source) {
  if (!source) return null;
  if (isTimelineControlDescriptor(source)) return source;
  if (Array.isArray(source)) {
    for (const entry of source) {
      const found = findTimelineControlDescriptor(entry);
      if (found) return found;
    }
    return null;
  }
  if (isTimelineControlDescriptor(source.timelineControl)) return source.timelineControl;
  if (isTimelineControlDescriptor(source.control)) return source.control;
  if (isTimelineControlDescriptor(source.metadata?.timelineControl)) return source.metadata.timelineControl;
  if (isTimelineControlDescriptor(source.animation?.control)) return source.animation.control;
  if (source.descriptors) return findTimelineControlDescriptor(source.descriptors);
  if (source.layerDescriptors) return findTimelineControlDescriptor(source.layerDescriptors);
  if (source.views) return findTimelineControlDescriptor(source.views);
  return null;
}

export function isTimelineControlDescriptor(value) {
  return Boolean(value && typeof value === "object" && value.schema === TIMELINE_CONTROL_SCHEMA);
}

function normalizeControlDescriptor(descriptor) {
  return isTimelineControlDescriptor(descriptor) ? descriptor : null;
}

function rangeControl(descriptor) {
  return Array.isArray(descriptor?.controls)
    ? descriptor.controls.find((control) => control?.role === "timeline-state") || null
    : null;
}

function playbackControl(descriptor) {
  return Array.isArray(descriptor?.controls)
    ? descriptor.controls.find((control) => control?.role === "timeline-playback") || null
    : null;
}

function resetControl(descriptor) {
  return Array.isArray(descriptor?.controls)
    ? descriptor.controls.find((control) => control?.role === "timeline-reset") || null
    : null;
}

function controlRange(descriptor) {
  const control = rangeControl(descriptor);
  const range = descriptor?.range || {};
  const min = finiteNumber(control?.min, range.min, 0);
  const max = finiteNumber(control?.max, range.max, 1);
  const step = positiveNumber(control?.step, range.step, descriptor?.step, 0.001);
  return { min, max: Math.max(min, max), step };
}

function normalizeTimelineValue(value, descriptor) {
  const range = controlRange(descriptor);
  let next = finiteNumber(value, range.min);
  next = clamp(next, range.min, range.max);
  const selection = rangeControl(descriptor)?.binding?.selection || descriptor?.evidence?.selection || "";
  if (selection === "nearest-exported-step" && range.step > 0) {
    next = range.min + Math.round((next - range.min) / range.step) * range.step;
    next = clamp(next, range.min, range.max);
  }
  return Number(next.toFixed(12));
}

function currentExportedState(descriptor, value) {
  const state = descriptor?.state;
  const range = controlRange(descriptor);
  if (state && Math.abs(Number(state.normalized) - value) <= range.step / 2) return state;
  for (const sample of descriptor?.samples || []) {
    if (Math.abs(Number(sample.normalized) - value) <= range.step / 2) return sample;
  }
  return null;
}

function currentTimelineMark(descriptor, value) {
  const range = controlRange(descriptor);
  for (const mark of timelineMarks(descriptor)) {
    const normalized = Number(mark.normalized ?? mark.value);
    if (Number.isFinite(normalized) && Math.abs(normalized - value) <= range.step / 2) return mark;
  }
  return null;
}

function timelineMarks(descriptor) {
  if (Array.isArray(descriptor?.marks)) return descriptor.marks;
  const displayMarks = rangeControl(descriptor)?.display?.marks;
  return Array.isArray(displayMarks) ? displayMarks : [];
}

function timelineWidgetValueLabel(descriptor, value, exportedState, mark) {
  const source = exportedState || mark;
  if (source) {
    const step = Number.isFinite(Number(source.activeStepOrder)) ? `Step ${source.activeStepOrder}` : "";
    const label = source.activeLabel || source.label || "";
    const sample = source.activePropertySample;
    const sampleText = sample && Number.isFinite(Number(sample.value))
      ? `${sample.propertyName || sample.propertyId}: ${formatNumber(sample.value)}`
      : sample?.propertyName || sample?.propertyId || "";
    const text = [step, label, sampleText].filter(Boolean).join(" - ");
    if (text) return text;
  }
  if (Math.abs(Number(descriptor?.value) - value) <= controlRange(descriptor).step / 2) {
    return descriptor?.valueLabel || descriptor?.presentation?.readout || descriptor?.readout?.label || "";
  }
  return `${Math.round(value * 100)}%`;
}

function timelineMarkTitle(mark) {
  const parts = [
    mark.label,
    mark.activeCoordinateId,
    mark.activePropertySample?.propertyName || mark.activePropertySample?.propertyId,
  ].filter(Boolean);
  return parts.join(" / ");
}

function timelinePlaybackDurationMs(descriptor) {
  const explicit = descriptor?.playback?.durationMs ?? descriptor?.durationMs;
  if (Number.isFinite(Number(explicit))) return Math.max(100, Number(explicit));
  const duration = Number(descriptor?.duration);
  if (Number.isFinite(duration) && duration > 0) return Math.max(1000, duration * 1000);
  const stepCount = Number(descriptor?.stepCount);
  if (Number.isFinite(stepCount) && stepCount > 1) return Math.max(1000, (stepCount - 1) * 360);
  return 6800;
}

function normalizeClock(clock = {}) {
  const win = windowObject();
  return {
    now: typeof clock.now === "function"
      ? clock.now
      : () => (typeof performance !== "undefined" && performance.now ? performance.now() : Date.now()),
    request: typeof clock.request === "function"
      ? clock.request
      : win?.requestAnimationFrame?.bind(win),
    cancel: typeof clock.cancel === "function"
      ? clock.cancel
      : win?.cancelAnimationFrame?.bind(win),
  };
}

function defaultRoot() {
  return documentBody() || documentObject()?.documentElement || null;
}

function createElement(tag) {
  return documentObject()?.createElement?.(tag) || null;
}

function documentObject() {
  return typeof document !== "undefined" ? document : null;
}

function documentBody() {
  return documentObject()?.body || null;
}

function windowObject() {
  return typeof window !== "undefined" ? window : null;
}

function installTimelineControlStyle(doc = documentObject()) {
  if (!doc?.createElement || doc.getElementById?.(STYLE_ID)) return;
  const style = doc.createElement("style");
  style.id = STYLE_ID;
  style.textContent = `
    .mtrc-timeline-control {
      position: absolute;
      left: 16px;
      right: 16px;
      bottom: 16px;
      z-index: 14;
      display: grid;
      gap: 8px;
      width: min(720px, calc(100% - 32px));
      margin: 0 auto;
      padding: 10px 12px;
      border: 1px solid rgba(36, 52, 54, 0.18);
      border-radius: 8px;
      color: #263235;
      background: rgba(239, 238, 221, 0.94);
      box-shadow: 0 18px 44px rgba(38, 48, 50, 0.18);
      font: 12px/1.35 system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
      letter-spacing: 0;
      box-sizing: border-box;
    }

    .mtrc-timeline-control[data-floating="true"] {
      position: fixed;
      left: 50%;
      right: auto;
      width: min(720px, calc(100vw - 32px));
      transform: translateX(-50%);
    }

    .mtrc-timeline-control[data-visible="false"] {
      display: none;
    }

    .mtrc-timeline-control header {
      display: flex;
      align-items: baseline;
      justify-content: space-between;
      gap: 12px;
      min-width: 0;
    }

    .mtrc-timeline-control strong {
      min-width: 0;
      overflow: hidden;
      text-overflow: ellipsis;
      white-space: nowrap;
      font-size: 12px;
      font-weight: 760;
    }

    .mtrc-timeline-control header span {
      min-width: 0;
      overflow: hidden;
      text-overflow: ellipsis;
      white-space: nowrap;
      color: #5d6d70;
      font-variant-numeric: tabular-nums;
      text-align: right;
    }

    .mtrc-timeline-control__row {
      display: grid;
      grid-template-columns: minmax(62px, max-content) minmax(160px, 1fr) minmax(62px, max-content);
      gap: 9px;
      align-items: center;
      min-width: 0;
    }

    .mtrc-timeline-control__button,
    .mtrc-timeline-control__mark {
      min-width: 0;
      border: 1px solid rgba(38, 54, 56, 0.18);
      border-radius: 6px;
      color: #263235;
      background: rgba(255, 252, 235, 0.74);
      font: inherit;
      font-weight: 680;
      letter-spacing: 0;
      cursor: pointer;
    }

    .mtrc-timeline-control__button {
      height: 30px;
      padding: 0 10px;
      white-space: nowrap;
    }

    .mtrc-timeline-control__button[data-active="true"] {
      color: #12333a;
      border-color: rgba(20, 104, 114, 0.34);
      background: rgba(191, 229, 223, 0.66);
    }

    .mtrc-timeline-control__scrubber {
      width: 100%;
      min-width: 0;
      accent-color: #257784;
    }

    .mtrc-timeline-control__marks {
      display: grid;
      grid-template-columns: repeat(3, minmax(0, 1fr));
      gap: 7px;
      min-width: 0;
    }

    .mtrc-timeline-control__mark {
      height: 24px;
      padding: 0 7px;
      overflow: hidden;
      text-overflow: ellipsis;
      white-space: nowrap;
      color: #607073;
      background: rgba(255, 252, 235, 0.42);
    }

    .mtrc-timeline-control__mark[data-active="true"] {
      color: #12333a;
      border-color: rgba(20, 104, 114, 0.30);
      background: rgba(191, 229, 223, 0.58);
    }

    @media (max-width: 560px) {
      .mtrc-timeline-control {
        left: 10px;
        right: 10px;
        bottom: 10px;
        width: calc(100% - 20px);
        padding: 9px;
      }

      .mtrc-timeline-control[data-floating="true"] {
        width: calc(100vw - 20px);
      }

      .mtrc-timeline-control__row {
        grid-template-columns: minmax(54px, max-content) minmax(112px, 1fr) minmax(54px, max-content);
        gap: 7px;
      }

      .mtrc-timeline-control__button {
        padding: 0 8px;
      }
    }
  `;
  (doc.head || doc.body || doc.documentElement)?.appendChild(style);
}

function finiteNumber(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number)) return number;
  }
  return 0;
}

function positiveNumber(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number) && number > 0) return number;
  }
  return 1;
}

function clamp(value, min, max) {
  return Math.max(min, Math.min(max, value));
}

function wrapRange(value, min, max) {
  const span = Math.max(1e-9, max - min);
  return min + ((((value - min) % span) + span) % span);
}

function formatValue(value) {
  const number = Number(value);
  if (!Number.isFinite(number)) return "0";
  return String(Number(number.toFixed(12)));
}

function formatNumber(value) {
  const number = Number(value);
  if (!Number.isFinite(number)) return String(value);
  if (Math.abs(number) >= 100) return number.toFixed(0);
  if (Math.abs(number) >= 10) return number.toFixed(1);
  if (Math.abs(number) >= 1) return number.toFixed(3);
  return number.toPrecision(3);
}

function stringValue(value) {
  return value == null ? "" : String(value);
}
