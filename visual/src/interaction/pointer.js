function readClientPosition(event) {
  const touch = event.touches?.[0] || event.changedTouches?.[0];
  return {
    x: touch ? touch.clientX : event.clientX,
    y: touch ? touch.clientY : event.clientY,
  };
}

function readElementRect(element) {
  if (element?.getBoundingClientRect) {
    const rect = element.getBoundingClientRect();
    return {
      left: rect.left,
      top: rect.top,
      width: Math.max(1, rect.width),
      height: Math.max(1, rect.height),
    };
  }
  return {
    left: 0,
    top: 0,
    width: Math.max(1, element?.clientWidth || element?.width || 1),
    height: Math.max(1, element?.clientHeight || element?.height || 1),
  };
}

export function createPointerSnapshot() {
  return {
    type: "",
    pointerId: undefined,
    buttons: 0,
    button: 0,
    client: { x: 0, y: 0 },
    css: { x: 0, y: 0, width: 1, height: 1 },
    pixel: { x: 0, y: 0, width: 1, height: 1, ratioX: 1, ratioY: 1 },
    uv: { x: 0, y: 0 },
    normalized: { x: 0, y: 0 },
    positionN: { x: 0, y: 0 },
    inside: false,
    sourceEvent: undefined,
  };
}

export function resetPointerSnapshot(out = createPointerSnapshot()) {
  out.type = "";
  out.pointerId = undefined;
  out.buttons = 0;
  out.button = 0;
  out.client.x = 0;
  out.client.y = 0;
  out.css.x = 0;
  out.css.y = 0;
  out.pixel.x = 0;
  out.pixel.y = 0;
  out.uv.x = 0;
  out.uv.y = 0;
  out.normalized.x = 0;
  out.normalized.y = 0;
  out.positionN.x = 0;
  out.positionN.y = 0;
  out.inside = false;
  out.sourceEvent = undefined;
  return out;
}

export function normalizePointerEvent(event, element, out = createPointerSnapshot()) {
  const client = readClientPosition(event);
  const rect = readElementRect(element);
  const cssX = client.x - rect.left;
  const cssY = client.y - rect.top;
  const canvasWidth = Math.max(1, element?.width || rect.width);
  const canvasHeight = Math.max(1, element?.height || rect.height);
  const ratioX = canvasWidth / rect.width;
  const ratioY = canvasHeight / rect.height;
  const uvX = cssX / rect.width;
  const uvY = cssY / rect.height;
  const normalizedX = uvX * 2 - 1;
  const normalizedY = -(uvY * 2 - 1);

  out.type = event.type || "";
  out.pointerId = event.pointerId;
  out.buttons = event.buttons || 0;
  out.button = event.button || 0;
  out.client.x = client.x;
  out.client.y = client.y;
  out.css.x = cssX;
  out.css.y = cssY;
  out.css.width = rect.width;
  out.css.height = rect.height;
  out.pixel.x = cssX * ratioX;
  out.pixel.y = cssY * ratioY;
  out.pixel.width = canvasWidth;
  out.pixel.height = canvasHeight;
  out.pixel.ratioX = ratioX;
  out.pixel.ratioY = ratioY;
  out.uv.x = uvX;
  out.uv.y = uvY;
  out.normalized.x = normalizedX;
  out.normalized.y = normalizedY;
  out.positionN.x = normalizedX;
  out.positionN.y = normalizedY;
  out.inside = uvX >= 0 && uvX <= 1 && uvY >= 0 && uvY <= 1;
  out.sourceEvent = event;
  return out;
}

export class PointerTracker {
  constructor(element, options = {}) {
    this.element = element;
    this.touch = options.touch !== false;
    this.resetOnEnd = Boolean(options.resetOnEnd);
    this.snapshot = createPointerSnapshot();
    this.listeners = false;
    this.onEnter = options.onEnter || (() => {});
    this.onMove = options.onMove || (() => {});
    this.onLeave = options.onLeave || (() => {});
    this.onDown = options.onDown || (() => {});
    this.onUp = options.onUp || (() => {});
    this.onClick = options.onClick || (() => {});
    this.handleEnter = (event) => this.emit(event, this.onEnter);
    this.handleMove = (event) => this.emit(event, this.onMove);
    this.handleLeave = (event) => {
      const snapshot = this.emit(event, this.onLeave);
      if (this.resetOnEnd) resetPointerSnapshot(snapshot);
    };
    this.handleDown = (event) => this.emit(event, this.onDown);
    this.handleUp = (event) => this.emit(event, this.onUp);
    this.handleClick = (event) => this.emit(event, this.onClick);
  }

  update(event) {
    return normalizePointerEvent(event, this.element, this.snapshot);
  }

  emit(event, callback) {
    const snapshot = this.update(event);
    callback(snapshot);
    return snapshot;
  }

  addListeners() {
    if (this.listeners || !this.element) return this;
    this.element.addEventListener("pointerenter", this.handleEnter);
    this.element.addEventListener("pointermove", this.handleMove);
    this.element.addEventListener("pointerleave", this.handleLeave);
    this.element.addEventListener("pointerdown", this.handleDown);
    this.element.addEventListener("pointerup", this.handleUp);
    this.element.addEventListener("pointercancel", this.handleLeave);
    this.element.addEventListener("click", this.handleClick);
    if (this.touch) {
      this.element.addEventListener("touchstart", this.handleEnter, { passive: true });
      this.element.addEventListener("touchmove", this.handleMove, { passive: true });
      this.element.addEventListener("touchend", this.handleLeave, { passive: true });
    }
    this.listeners = true;
    return this;
  }

  removeListeners() {
    if (!this.listeners || !this.element) return this;
    this.element.removeEventListener("pointerenter", this.handleEnter);
    this.element.removeEventListener("pointermove", this.handleMove);
    this.element.removeEventListener("pointerleave", this.handleLeave);
    this.element.removeEventListener("pointerdown", this.handleDown);
    this.element.removeEventListener("pointerup", this.handleUp);
    this.element.removeEventListener("pointercancel", this.handleLeave);
    this.element.removeEventListener("click", this.handleClick);
    this.element.removeEventListener("touchstart", this.handleEnter);
    this.element.removeEventListener("touchmove", this.handleMove);
    this.element.removeEventListener("touchend", this.handleLeave);
    this.listeners = false;
    return this;
  }
}

export function createPointerTracker(element, options = {}) {
  return new PointerTracker(element, options);
}
