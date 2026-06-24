import { clamp, finiteNumber } from "../camera/math.js";

export class FocusLineState {
  constructor(options = {}) {
    this.width = Math.max(1, finiteNumber(options.width, 1));
    this.height = Math.max(1, finiteNumber(options.height, 1));
    this.start = {
      x: finiteNumber(options.start?.x, 0),
      y: finiteNumber(options.start?.y, this.height * 0.5),
    };
    this.end = {
      x: finiteNumber(options.end?.x, this.width),
      y: finiteNumber(options.end?.y, this.height * 0.5),
    };
    this.radius = Math.max(0, finiteNumber(options.radius, this.height / 3));
    this.focusBand = Math.max(0, finiteNumber(options.focusBand, 0));
    this.blurCurve = Math.max(0.05, finiteNumber(options.blurCurve, 1));
    this.axis = options.axis === "vertical" ? "vertical" : "horizontal";
    this.version = 0;
    this.clampToViewport();
  }

  setSize(width, height, options = {}) {
    const nextWidth = Math.max(1, finiteNumber(width, this.width));
    const nextHeight = Math.max(1, finiteNumber(height, this.height));
    const preserve = options.preserveRelative !== false;
    const sx0 = preserve ? this.start.x / this.width : 0;
    const sy0 = preserve ? this.start.y / this.height : 0.5;
    const sx1 = preserve ? this.end.x / this.width : 1;
    const sy1 = preserve ? this.end.y / this.height : 0.5;
    const radiusRatio = preserve ? this.radius / this.height : 1 / 3;
    const focusBandRatio = preserve ? this.focusBand / this.height : 0;
    this.width = nextWidth;
    this.height = nextHeight;
    this.start.x = sx0 * nextWidth;
    this.start.y = sy0 * nextHeight;
    this.end.x = sx1 * nextWidth;
    this.end.y = sy1 * nextHeight;
    this.radius = Math.max(0, finiteNumber(options.radius, radiusRatio * nextHeight));
    this.focusBand = Math.max(0, finiteNumber(options.focusBand, focusBandRatio * nextHeight));
    if (options.blurCurve !== undefined) {
      this.blurCurve = Math.max(0.05, finiteNumber(options.blurCurve, this.blurCurve));
    }
    this.clampToViewport();
    this.version += 1;
    return this;
  }

  setLine(start, end, radius = this.radius, options = {}) {
    const startPoint = point2(start, this.start);
    const endPoint = point2(end, this.end);
    this.start.x = finiteNumber(startPoint.x, this.start.x);
    this.start.y = finiteNumber(startPoint.y, this.start.y);
    this.end.x = finiteNumber(endPoint.x, this.end.x);
    this.end.y = finiteNumber(endPoint.y, this.end.y);
    this.radius = Math.max(0, finiteNumber(radius, this.radius));
    if (options.focusBand !== undefined) {
      this.focusBand = Math.max(0, finiteNumber(options.focusBand, this.focusBand));
    }
    if (options.blurCurve !== undefined) {
      this.blurCurve = Math.max(0.05, finiteNumber(options.blurCurve, this.blurCurve));
    }
    if (options.axis === "vertical" || options.axis === "horizontal") {
      this.axis = options.axis;
    } else {
      this.axis = inferAxis(this.start, this.end, this.axis);
    }
    this.clampToViewport();
    this.version += 1;
    return this;
  }

  setHorizontal(y, radius = this.radius, options = {}) {
    const nextY = clamp(finiteNumber(y, this.height * 0.5), 0, this.height);
    return this.setLine({ x: 0, y: nextY }, { x: this.width, y: nextY }, radius, {
      ...options,
      axis: "horizontal",
    });
  }

  setVertical(x, radius = this.radius, options = {}) {
    const nextX = clamp(finiteNumber(x, this.width * 0.5), 0, this.width);
    return this.setLine({ x: nextX, y: 0 }, { x: nextX, y: this.height }, radius, {
      ...options,
      axis: "vertical",
    });
  }

  setOptics(options = {}) {
    if (options.focusBand !== undefined) {
      this.focusBand = Math.max(0, finiteNumber(options.focusBand, this.focusBand));
    }
    if (options.blurCurve !== undefined) {
      this.blurCurve = Math.max(0.05, finiteNumber(options.blurCurve, this.blurCurve));
    }
    this.version += 1;
    return this;
  }

  setFromPointer(pointer, options = {}) {
    const axis = options.axis || "horizontal";
    const radius = finiteNumber(options.radius, this.radius);
    if (axis === "vertical") {
      return this.setVertical(pointer.pixel?.x, radius);
    }
    return this.setHorizontal(pointer.pixel?.y, radius);
  }

  setFromNormalized(normalized, options = {}) {
    const x = (finiteNumber(normalized?.x, 0) * 0.5 + 0.5) * this.width;
    const y = (-finiteNumber(normalized?.y, 0) * 0.5 + 0.5) * this.height;
    return options.axis === "vertical"
      ? this.setVertical(x, options.radius)
      : this.setHorizontal(y, options.radius);
  }

  distanceToPixel(x, y) {
    const ax = this.start.x;
    const ay = this.start.y;
    const bx = this.end.x;
    const by = this.end.y;
    const dx = bx - ax;
    const dy = by - ay;
    const lengthSquared = dx * dx + dy * dy;
    if (lengthSquared <= 1e-6) return Math.hypot(finiteNumber(x) - ax, finiteNumber(y) - ay);
    const t = clamp(((finiteNumber(x) - ax) * dx + (finiteNumber(y) - ay) * dy) / lengthSquared, 0, 1);
    return Math.hypot(finiteNumber(x) - (ax + dx * t), finiteNumber(y) - (ay + dy * t));
  }

  toUniforms(prefix = "uFocus") {
    return {
      [`${prefix}Start`]: [this.start.x, this.start.y],
      [`${prefix}End`]: [this.end.x, this.end.y],
      [`${prefix}Radius`]: this.radius,
      [`${prefix}Band`]: this.focusBand,
      [`${prefix}BlurCurve`]: this.blurCurve,
      uTexSize: [this.width, this.height],
      uTextureSize: [this.width, this.height],
    };
  }

  toJSON() {
    return {
      width: this.width,
      height: this.height,
      start: { ...this.start },
      end: { ...this.end },
      radius: this.radius,
      focusBand: this.focusBand,
      blurCurve: this.blurCurve,
      axis: this.axis,
      version: this.version,
    };
  }

  clone() {
    return new FocusLineState(this.toJSON());
  }

  clampToViewport() {
    this.start.x = clamp(this.start.x, 0, this.width);
    this.start.y = clamp(this.start.y, 0, this.height);
    this.end.x = clamp(this.end.x, 0, this.width);
    this.end.y = clamp(this.end.y, 0, this.height);
    return this;
  }
}

export function createFocusLineState(options = {}) {
  return new FocusLineState(options);
}

function point2(value, fallback) {
  if (Array.isArray(value) || ArrayBuffer.isView(value)) {
    return {
      x: finiteNumber(value[0], fallback.x),
      y: finiteNumber(value[1], fallback.y),
    };
  }
  return {
    x: finiteNumber(value?.x, fallback.x),
    y: finiteNumber(value?.y, fallback.y),
  };
}

function inferAxis(start, end, fallback = "horizontal") {
  const dx = Math.abs(finiteNumber(end.x, 0) - finiteNumber(start.x, 0));
  const dy = Math.abs(finiteNumber(end.y, 0) - finiteNumber(start.y, 0));
  if (dy > dx) return "vertical";
  if (dx > dy) return "horizontal";
  return fallback;
}
