export class RenderLoop {
  constructor(callback, options = {}) {
    this.callback = callback;
    this.requestFrame = options.requestAnimationFrame || defaultRequestAnimationFrame;
    this.cancelFrame = options.cancelAnimationFrame || defaultCancelAnimationFrame;
    this.now = options.now || defaultNow;
    this.running = false;
    this.frameHandle = null;
    this.maxDeltaMs = options.maxDeltaMs ?? 100;
    this.time = {
      now: 0,
      previous: 0,
      delta: 0,
      elapsed: 0,
      seconds: 0,
      frame: 0,
    };
    this._tick = (time) => this.tick(time);
    if (options.autoStart) {
      this.start();
    }
  }

  start() {
    if (this.running) return this;
    this.running = true;
    this.time.previous = this.now();
    this.frameHandle = this.requestFrame(this._tick);
    return this;
  }

  stop() {
    if (!this.running) return this;
    this.running = false;
    if (this.frameHandle != null) {
      this.cancelFrame(this.frameHandle);
      this.frameHandle = null;
    }
    return this;
  }

  tick(now = this.now()) {
    if (!this.running) return;
    this.frameHandle = this.requestFrame(this._tick);
    this.updateTime(now);
    this.callback(this.time);
  }

  renderOnce(now = this.now()) {
    this.updateTime(now);
    this.callback(this.time);
    return this;
  }

  updateTime(now) {
    const previous = this.time.previous || now;
    const rawDelta = Math.max(0, now - previous);
    const delta = Math.min(rawDelta, this.maxDeltaMs);
    this.time.now = now;
    this.time.previous = now;
    this.time.delta = delta;
    this.time.elapsed += delta;
    this.time.seconds = this.time.elapsed / 1000;
    this.time.frame += 1;
    return this.time;
  }
}

function defaultRequestAnimationFrame(callback) {
  if (typeof globalThis.requestAnimationFrame === "function") {
    return globalThis.requestAnimationFrame(callback);
  }
  return globalThis.setTimeout(() => callback(defaultNow()), 16);
}

function defaultCancelAnimationFrame(handle) {
  if (typeof globalThis.cancelAnimationFrame === "function") {
    globalThis.cancelAnimationFrame(handle);
  } else {
    globalThis.clearTimeout(handle);
  }
}

function defaultNow() {
  return globalThis.performance && typeof globalThis.performance.now === "function"
    ? globalThis.performance.now()
    : Date.now();
}
