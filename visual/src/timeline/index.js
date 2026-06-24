import { clamp01, resolveEasing } from "../animation/index.js";

export const TIMELINE_INTERPOLATION_SCHEMA = "metric.visual.timeline_interpolation.v1";

export class TimelineModel {
  constructor(source = {}, options = {}) {
    this.document = isVisualDocument(source) ? source : null;
    this.timeline = resolveTimeline(source, options);
    this.id = this.timeline?.id ?? options.id ?? null;
    this.datasetId = this.timeline?.dataset_id ?? this.timeline?.datasetId ?? options.datasetId ?? options.dataset_id ?? null;
    this.steps = normalizeTimelineSteps(this.timeline?.steps || []);
    this.events = normalizeTimelineEvents(resolveEvents(source, this.timeline, options), this.steps);
    this.duration = resolveDuration(this.steps, options.duration ?? options.durationMs);
    this.currentTime = clampTime(Number(options.currentTime ?? options.time ?? 0), this.duration);
    this.previousTime = this.currentTime;
    this.playing = Boolean(options.playing);
    this.scrubbing = false;
    this.loop = Boolean(options.loop ?? this.timeline?.loop);
    this.playbackRate = Number.isFinite(Number(options.playbackRate)) ? Number(options.playbackRate) : 1;
    this.defaultEasing = options.easing || this.timeline?.easing || "linear";
    this._listeners = new Map();
    this._descriptor = createInterpolationDescriptor();
    this._eventCursor = 0;
    this._activeStepIndex = -1;
    this._eventDetail = {
      type: "",
      model: this,
      time: this.currentTime,
      previousTime: this.previousTime,
      descriptor: this._descriptor,
      event: null,
      step: null,
    };
    this.updateDescriptor();
    this._eventCursor = firstEventAfter(this.events, this.currentTime);
  }

  static fromVisualDocument(document, options = {}) {
    return new TimelineModel(document, options);
  }

  on(type, listener) {
    if (!this._listeners.has(type)) {
      this._listeners.set(type, new Set());
    }
    this._listeners.get(type).add(listener);
    return () => this.off(type, listener);
  }

  off(type, listener) {
    const listeners = this._listeners.get(type);
    if (!listeners) return false;
    return listeners.delete(listener);
  }

  play() {
    if (!this.playing) {
      this.playing = true;
      this.emit("play");
    }
    return this;
  }

  pause() {
    if (this.playing) {
      this.playing = false;
      this.emit("pause");
    }
    return this;
  }

  toggle() {
    return this.playing ? this.pause() : this.play();
  }

  seek(time) {
    this.previousTime = this.currentTime;
    this.currentTime = clampTime(time, this.duration);
    this._eventCursor = firstEventAfter(this.events, this.currentTime);
    this.updateDescriptor();
    this.emit("seek");
    return this;
  }

  startScrub(time = this.currentTime) {
    this.scrubbing = true;
    this.pause();
    this.seek(time);
    this.emit("scrubstart");
    return this;
  }

  scrub(time) {
    this.scrubbing = true;
    this.previousTime = this.currentTime;
    this.currentTime = clampTime(time, this.duration);
    this.updateDescriptor();
    this.emit("scrub");
    return this;
  }

  endScrub(options = {}) {
    if (this.scrubbing) {
      this.scrubbing = false;
      this._eventCursor = firstEventAfter(this.events, this.currentTime);
      this.emit("scrubend");
    }
    if (options.play) this.play();
    return this;
  }

  tick(renderLoopTime) {
    if (!this.playing) return this;
    const deltaMs = renderLoopTime && Number.isFinite(Number(renderLoopTime.delta))
      ? Number(renderLoopTime.delta)
      : 0;
    return this.advance(deltaMs / 1000);
  }

  advance(deltaSeconds) {
    if (!this.playing && deltaSeconds !== 0) return this;
    this.previousTime = this.currentTime;
    const delta = (Number(deltaSeconds) || 0) * this.playbackRate;
    let next = this.currentTime + delta;
    if (this.duration > 0) {
      if (this.loop) {
        next = wrapTime(next, this.duration);
      } else {
        next = clampTime(next, this.duration);
        if (next >= this.duration && delta >= 0) this.playing = false;
        if (next <= 0 && delta < 0) this.playing = false;
      }
    } else {
      next = 0;
      this.playing = false;
    }
    this.currentTime = next;
    this.updateDescriptor();
    this.dispatchTimelineEvents(this.previousTime, this.currentTime);
    this.emit("tick");
    if (!this.playing) this.emit("pause");
    return this;
  }

  updateDescriptor() {
    const descriptor = this._descriptor;
    writeBaseDescriptor(descriptor, this);
    const stepCount = this.steps.length;
    if (stepCount === 0) {
      writeEmptyDescriptor(descriptor);
      return descriptor;
    }
    if (stepCount === 1 || this.duration <= 0) {
      writeConstantDescriptor(descriptor, this.steps[0], this);
      this.emitStepChangeIfNeeded(this.steps[0].index);
      return descriptor;
    }

    const segment = findTimelineSegment(this.steps, this.currentTime);
    const from = this.steps[segment];
    const to = this.steps[Math.min(segment + 1, stepCount - 1)];
    const span = Math.max(0, to.time - from.time);
    const progress = span > 0 ? clamp01((this.currentTime - from.time) / span) : 0;
    const easingName = to.easing || from.easing || this.defaultEasing;
    const easedProgress = resolveEasing(easingName)(progress);

    descriptor.empty = false;
    descriptor.mode = from === to || span === 0 ? "constant" : "interpolate";
    descriptor.progress = progress;
    descriptor.easedProgress = easedProgress;
    descriptor.fromStepIndex = from.index;
    descriptor.toStepIndex = to.index;
    writeStepRef(descriptor.from, from);
    writeStepRef(descriptor.to, to);
    writeChannelRef(descriptor.coordinate, "coordinate", from.coordinateId, to.coordinateId, progress, easedProgress);
    writeChannelRef(descriptor.property, "property", from.propertyId, to.propertyId, progress, easedProgress);
    writeChannelRef(descriptor.relation, "relation", from.relationId, to.relationId, progress, easedProgress);
    this.emitStepChangeIfNeeded(from.index);
    return descriptor;
  }

  currentInterpolation() {
    return this._descriptor;
  }

  getCurrentInterpolation() {
    return this._descriptor;
  }

  toJSON() {
    return {
      id: this.id,
      dataset_id: this.datasetId,
      currentTime: this.currentTime,
      duration: this.duration,
      playing: this.playing,
      scrubbing: this.scrubbing,
      stepCount: this.steps.length,
      eventCount: this.events.length,
    };
  }

  emitStepChangeIfNeeded(stepIndex) {
    if (this._activeStepIndex === stepIndex) return;
    this._activeStepIndex = stepIndex;
    this._eventDetail.step = this.steps.find((step) => step.index === stepIndex) || null;
    this.emit("stepchange");
    this._eventDetail.step = null;
  }

  dispatchTimelineEvents(previousTime, currentTime) {
    if (this.events.length === 0 || previousTime === currentTime) return;
    if (currentTime > previousTime) {
      this.dispatchEventsInRange(previousTime, currentTime, 1);
      return;
    }
    if (this.loop && this.duration > 0) {
      this.dispatchEventsInRange(previousTime, this.duration, 1);
      this.dispatchEventsInRange(0, currentTime, 1);
    } else {
      this.dispatchEventsInRange(currentTime, previousTime, -1);
    }
  }

  dispatchEventsInRange(start, end, direction) {
    if (direction >= 0) {
      let index = firstEventAfter(this.events, start);
      while (index < this.events.length && this.events[index].time <= end) {
        this.emitTimelineEvent(this.events[index]);
        index += 1;
      }
      this._eventCursor = index;
      return;
    }
    let index = firstEventAtOrAfter(this.events, end) - 1;
    while (index >= 0 && this.events[index].time >= start) {
      this.emitTimelineEvent(this.events[index]);
      index -= 1;
    }
    this._eventCursor = Math.max(0, index);
  }

  emitTimelineEvent(event) {
    this._eventDetail.event = event.source;
    this.emit("event");
    this._eventDetail.event = null;
  }

  emit(type) {
    const listeners = this._listeners.get(type);
    if (!listeners || listeners.size === 0) return;
    const detail = this._eventDetail;
    detail.type = type;
    detail.time = this.currentTime;
    detail.previousTime = this.previousTime;
    detail.descriptor = this._descriptor;
    for (const listener of listeners) {
      listener(detail);
    }
  }
}

export function createTimelineModel(source, options) {
  return new TimelineModel(source, options);
}

export function normalizeTimelineSteps(steps) {
  if (!Array.isArray(steps)) return [];
  const out = new Array(steps.length);
  for (let order = 0; order < steps.length; order += 1) {
    const step = steps[order] || {};
    const index = Number.isFinite(Number(step.index ?? step.step_index ?? step.step))
      ? Number(step.index ?? step.step_index ?? step.step)
      : order;
    out[order] = {
      source: step,
      order,
      index,
      time: resolveStepTime(step, order),
      easing: step.easing,
      coordinateId: step.coordinate_id ?? step.coordinateId ?? null,
      propertyId: step.property_id ?? step.propertyId ?? null,
      relationId: step.relation_id ?? step.relationId ?? null,
      eventId: step.event_id ?? step.eventId ?? null,
      label: step.label ?? step.name ?? null,
    };
  }
  out.sort(compareSteps);
  return out;
}

export function normalizeTimelineEvents(events, steps = []) {
  if (!Array.isArray(events)) return [];
  const out = [];
  for (let index = 0; index < events.length; index += 1) {
    const event = events[index] || {};
    const time = resolveEventTime(event, steps);
    if (!Number.isFinite(time)) continue;
    out.push({
      source: event,
      order: index,
      time,
      id: event.id ?? null,
      type: event.event_type ?? event.eventType ?? event.type ?? null,
      stepIndex: event.step_index ?? event.stepIndex ?? event.step ?? null,
    });
  }
  out.sort(compareEvents);
  return out;
}

export function currentStepInterpolation(timeline, time) {
  const model = timeline instanceof TimelineModel
    ? timeline
    : new TimelineModel(timeline, { currentTime: time });
  if (timeline instanceof TimelineModel && time !== undefined) {
    model.seek(time);
  }
  return model.currentInterpolation();
}

function resolveTimeline(source, options) {
  if (!source) return null;
  if (source.steps && Array.isArray(source.steps)) return source;
  const timelines = Array.isArray(source.timelines) ? source.timelines : [];
  if (timelines.length === 0) return null;
  const id = options.timelineId ?? options.timeline_id ?? options.id;
  if (id != null) {
    const found = timelines.find((timeline) => String(timeline.id) === String(id));
    if (found) return found;
  }
  const datasetId = options.datasetId ?? options.dataset_id;
  if (datasetId != null) {
    const found = timelines.find((timeline) => String(timeline.dataset_id ?? timeline.datasetId) === String(datasetId));
    if (found) return found;
  }
  return timelines[0] || null;
}

function resolveEvents(source, timeline, options) {
  const explicit = options.events;
  const events = Array.isArray(explicit)
    ? explicit
    : Array.isArray(source?.events)
      ? source.events
      : [];
  const datasetId = timeline?.dataset_id ?? timeline?.datasetId ?? options.datasetId ?? options.dataset_id;
  const timelineId = timeline?.id ?? options.timelineId ?? options.timeline_id;
  return events.filter((event) => {
    if (!event) return false;
    const eventTimelineId = event.timeline_id ?? event.timelineId;
    if (eventTimelineId != null && timelineId != null && String(eventTimelineId) !== String(timelineId)) return false;
    const eventDatasetId = event.dataset_id ?? event.datasetId;
    if (eventDatasetId != null && datasetId != null && String(eventDatasetId) !== String(datasetId)) return false;
    return true;
  });
}

function resolveDuration(steps, override) {
  if (Number.isFinite(Number(override))) return Math.max(0, Number(override));
  if (steps.length === 0) return 0;
  const last = steps[steps.length - 1].time;
  return Math.max(0, last);
}

function resolveStepTime(step, order) {
  const explicit = step.time ?? step.t ?? step.seconds;
  if (Number.isFinite(Number(explicit))) return Number(explicit);
  const index = step.index ?? step.step_index ?? step.step;
  if (Number.isFinite(Number(index))) return Number(index);
  return order;
}

function resolveEventTime(event, steps) {
  const explicit = event.time ?? event.t ?? event.seconds;
  if (Number.isFinite(Number(explicit))) return Number(explicit);
  const stepIndex = event.step_index ?? event.stepIndex ?? event.step;
  if (Number.isFinite(Number(stepIndex))) {
    const step = steps.find((entry) => Number(entry.index) === Number(stepIndex));
    return step ? step.time : Number(stepIndex);
  }
  return NaN;
}

function findTimelineSegment(steps, time) {
  if (steps.length <= 1) return 0;
  if (time <= steps[0].time) return 0;
  const lastIndex = steps.length - 1;
  if (time >= steps[lastIndex].time) return lastIndex - 1;
  let low = 0;
  let high = lastIndex - 1;
  while (low <= high) {
    const mid = (low + high) >> 1;
    if (time < steps[mid].time) {
      high = mid - 1;
    } else if (time >= steps[mid + 1].time) {
      low = mid + 1;
    } else {
      return mid;
    }
  }
  return Math.max(0, Math.min(lastIndex - 1, low));
}

function createInterpolationDescriptor() {
  return {
    schema: TIMELINE_INTERPOLATION_SCHEMA,
    timelineId: null,
    datasetId: null,
    empty: true,
    mode: "empty",
    time: 0,
    previousTime: 0,
    duration: 0,
    progress: 0,
    easedProgress: 0,
    fromStepIndex: -1,
    toStepIndex: -1,
    from: createStepRef(),
    to: createStepRef(),
    coordinate: createChannelRef("coordinate"),
    property: createChannelRef("property"),
    relation: createChannelRef("relation"),
  };
}

function createStepRef() {
  return {
    index: -1,
    order: -1,
    time: 0,
    coordinateId: null,
    propertyId: null,
    relationId: null,
    eventId: null,
    label: null,
    source: null,
  };
}

function createChannelRef(kind) {
  return {
    kind,
    mode: "empty",
    fromId: null,
    toId: null,
    progress: 0,
    easedProgress: 0,
  };
}

function writeBaseDescriptor(descriptor, model) {
  descriptor.timelineId = model.id;
  descriptor.datasetId = model.datasetId;
  descriptor.time = model.currentTime;
  descriptor.previousTime = model.previousTime;
  descriptor.duration = model.duration;
}

function writeEmptyDescriptor(descriptor) {
  descriptor.empty = true;
  descriptor.mode = "empty";
  descriptor.progress = 0;
  descriptor.easedProgress = 0;
  descriptor.fromStepIndex = -1;
  descriptor.toStepIndex = -1;
  writeStepRef(descriptor.from, null);
  writeStepRef(descriptor.to, null);
  writeChannelRef(descriptor.coordinate, "coordinate", null, null, 0);
  writeChannelRef(descriptor.property, "property", null, null, 0);
  writeChannelRef(descriptor.relation, "relation", null, null, 0);
}

function writeConstantDescriptor(descriptor, step, model) {
  descriptor.empty = false;
  descriptor.mode = "constant";
  descriptor.progress = 0;
  descriptor.easedProgress = 0;
  descriptor.fromStepIndex = step.index;
  descriptor.toStepIndex = step.index;
  writeStepRef(descriptor.from, step);
  writeStepRef(descriptor.to, step);
  writeChannelRef(descriptor.coordinate, "coordinate", step.coordinateId, step.coordinateId, 0);
  writeChannelRef(descriptor.property, "property", step.propertyId, step.propertyId, 0);
  writeChannelRef(descriptor.relation, "relation", step.relationId, step.relationId, 0);
  writeBaseDescriptor(descriptor, model);
}

function writeStepRef(target, step) {
  target.index = step ? step.index : -1;
  target.order = step ? step.order : -1;
  target.time = step ? step.time : 0;
  target.coordinateId = step ? step.coordinateId : null;
  target.propertyId = step ? step.propertyId : null;
  target.relationId = step ? step.relationId : null;
  target.eventId = step ? step.eventId : null;
  target.label = step ? step.label : null;
  target.source = step ? step.source : null;
}

function writeChannelRef(target, kind, fromId, toId, progress, easedProgress = progress) {
  target.kind = kind;
  target.fromId = fromId ?? toId ?? null;
  target.toId = toId ?? fromId ?? null;
  target.progress = progress;
  target.easedProgress = easedProgress;
  if (target.fromId == null && target.toId == null) {
    target.mode = "empty";
  } else if (target.fromId === target.toId || fromId == null || toId == null) {
    target.mode = "constant";
  } else {
    target.mode = "interpolate";
  }
}

function compareSteps(a, b) {
  return numericCompare(a.time, b.time) || numericCompare(a.index, b.index) || numericCompare(a.order, b.order);
}

function compareEvents(a, b) {
  return numericCompare(a.time, b.time) || numericCompare(a.order, b.order);
}

function numericCompare(a, b) {
  return a < b ? -1 : a > b ? 1 : 0;
}

function clampTime(time, duration) {
  const value = Number.isFinite(Number(time)) ? Number(time) : 0;
  if (duration <= 0) return 0;
  if (value <= 0) return 0;
  if (value >= duration) return duration;
  return value;
}

function wrapTime(time, duration) {
  if (duration <= 0) return 0;
  const value = Number(time) || 0;
  return ((value % duration) + duration) % duration;
}

function firstEventAfter(events, time) {
  let low = 0;
  let high = events.length;
  while (low < high) {
    const mid = (low + high) >> 1;
    if (events[mid].time <= time) low = mid + 1;
    else high = mid;
  }
  return low;
}

function firstEventAtOrAfter(events, time) {
  let low = 0;
  let high = events.length;
  while (low < high) {
    const mid = (low + high) >> 1;
    if (events[mid].time < time) low = mid + 1;
    else high = mid;
  }
  return low;
}

function isVisualDocument(source) {
  return Boolean(source && source.schema === "metric.visual.v1" && Array.isArray(source.timelines));
}
