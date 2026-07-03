import {
  DEG_TO_RAD,
  clamp,
  copyVec3,
  createVec3,
  crossVec3,
  distanceVec3,
  finiteNumber,
  normalizeVec3,
  scaleAndAddVec3,
  setVec3,
  subtractVec3,
} from "./math.js";

const DEFAULT_MIN_PITCH = -Math.PI * 0.48;
const DEFAULT_MAX_PITCH = Math.PI * 0.48;

export class TargetCameraState {
  constructor(options = {}) {
    this.mode = options.mode || "target";
    this.fov = finiteNumber(options.fov, 50);
    this.aspect = Math.max(1e-6, finiteNumber(options.aspect, 1));
    this.near = Math.max(1e-6, finiteNumber(options.near, 0.1));
    this.far = Math.max(this.near + 1e-6, finiteNumber(options.far, 2000));
    this.position = createVec3();
    this.target = createVec3();
    this.up = createVec3(0, 1, 0);
    this.viewport = {
      width: 1,
      height: 1,
      pixelWidth: 1,
      pixelHeight: 1,
      pixelRatio: 1,
    };
    this.minPitch = finiteNumber(options.minPitch, DEFAULT_MIN_PITCH);
    this.maxPitch = finiteNumber(options.maxPitch, DEFAULT_MAX_PITCH);
    this.minDistance = Math.max(1e-6, finiteNumber(options.minDistance, 0.05));
    this.maxDistance = Math.max(this.minDistance, finiteNumber(options.maxDistance, 10000));
    this.yaw = finiteNumber(options.yaw, 0);
    this.pitch = finiteNumber(options.pitch, 0);
    this.radius = Math.max(this.minDistance, finiteNumber(options.radius, options.distance ?? 1));

    setVec3(this.target, options.target || options.lookAt || [0, 0, 0]);
    setVec3(this.up, options.up || [0, 1, 0], [0, 1, 0]);

    if (options.viewport) {
      this.setViewport(
        options.viewport.width || options.viewport.pixelWidth,
        options.viewport.height || options.viewport.pixelHeight,
        options.viewport.pixelRatio || 1,
      );
    }

    if (options.position) {
      setVec3(this.position, options.position);
      this.syncOrbitFromPosition();
    } else {
      this.syncPositionFromOrbit();
    }
  }

  setPerspective(options = {}) {
    if (options.fov !== undefined) this.fov = finiteNumber(options.fov, this.fov);
    if (options.aspect !== undefined) this.aspect = Math.max(1e-6, finiteNumber(options.aspect, this.aspect));
    if (options.near !== undefined) this.near = Math.max(1e-6, finiteNumber(options.near, this.near));
    if (options.far !== undefined) this.far = Math.max(this.near + 1e-6, finiteNumber(options.far, this.far));
    return this;
  }

  setViewport(width, height, pixelRatio = 1) {
    const safeWidth = Math.max(1, finiteNumber(width, this.viewport.width));
    const safeHeight = Math.max(1, finiteNumber(height, this.viewport.height));
    const safeRatio = Math.max(1e-6, finiteNumber(pixelRatio, this.viewport.pixelRatio));
    this.viewport.width = safeWidth;
    this.viewport.height = safeHeight;
    this.viewport.pixelRatio = safeRatio;
    this.viewport.pixelWidth = Math.max(1, Math.round(safeWidth * safeRatio));
    this.viewport.pixelHeight = Math.max(1, Math.round(safeHeight * safeRatio));
    this.aspect = safeWidth / safeHeight;
    return this;
  }

  setDrawingBufferSize(width, height) {
    const safeWidth = Math.max(1, finiteNumber(width, this.viewport.pixelWidth));
    const safeHeight = Math.max(1, finiteNumber(height, this.viewport.pixelHeight));
    this.viewport.width = safeWidth;
    this.viewport.height = safeHeight;
    this.viewport.pixelWidth = safeWidth;
    this.viewport.pixelHeight = safeHeight;
    this.viewport.pixelRatio = 1;
    this.aspect = safeWidth / safeHeight;
    return this;
  }

  setPosition(position) {
    setVec3(this.position, position);
    this.syncOrbitFromPosition();
    return this;
  }

  setTarget(target, options = {}) {
    const previousTarget = createVec3(this.target[0], this.target[1], this.target[2]);
    setVec3(this.target, target);
    if (options.preserveOffset !== false) {
      this.position[0] += this.target[0] - previousTarget[0];
      this.position[1] += this.target[1] - previousTarget[1];
      this.position[2] += this.target[2] - previousTarget[2];
    }
    this.syncOrbitFromPosition();
    return this;
  }

  lookAt(target) {
    return this.setTarget(target, { preserveOffset: false });
  }

  setUp(up) {
    setVec3(this.up, up, [0, 1, 0]);
    return this;
  }

  setOrbit(options = {}) {
    if (options.yaw !== undefined) this.yaw = finiteNumber(options.yaw, this.yaw);
    if (options.pitch !== undefined) this.pitch = clamp(finiteNumber(options.pitch, this.pitch), this.minPitch, this.maxPitch);
    if (options.radius !== undefined || options.distance !== undefined) {
      this.radius = clamp(finiteNumber(options.radius ?? options.distance, this.radius), this.minDistance, this.maxDistance);
    }
    this.syncPositionFromOrbit();
    return this;
  }

  orbit(deltaYaw = 0, deltaPitch = 0) {
    this.yaw += finiteNumber(deltaYaw);
    this.pitch = clamp(this.pitch + finiteNumber(deltaPitch), this.minPitch, this.maxPitch);
    this.syncPositionFromOrbit();
    return this;
  }

  dolly(scale) {
    const safeScale = Math.max(1e-6, finiteNumber(scale, 1));
    this.radius = clamp(this.radius * safeScale, this.minDistance, this.maxDistance);
    this.syncPositionFromOrbit();
    return this;
  }

  translate(delta) {
    this.position[0] += finiteNumber(delta[0]);
    this.position[1] += finiteNumber(delta[1]);
    this.position[2] += finiteNumber(delta[2]);
    this.target[0] += finiteNumber(delta[0]);
    this.target[1] += finiteNumber(delta[1]);
    this.target[2] += finiteNumber(delta[2]);
    this.syncOrbitFromPosition();
    return this;
  }

  panByCameraAxes(rightAmount, upAmount) {
    const basis = this.getBasis();
    const delta = createVec3();
    scaleAndAddVec3(delta, delta, basis.right, finiteNumber(rightAmount));
    scaleAndAddVec3(delta, delta, basis.up, finiteNumber(upAmount));
    return this.translate(delta);
  }

  syncOrbitFromPosition() {
    const offset = createVec3();
    subtractVec3(offset, this.position, this.target);
    this.radius = clamp(distanceVec3(this.position, this.target), this.minDistance, this.maxDistance);
    this.yaw = Math.atan2(offset[0], offset[2]);
    this.pitch = clamp(Math.asin(clamp(offset[1] / this.radius, -1, 1)), this.minPitch, this.maxPitch);
    return this;
  }

  syncPositionFromOrbit() {
    const radius = clamp(this.radius, this.minDistance, this.maxDistance);
    const pitch = clamp(this.pitch, this.minPitch, this.maxPitch);
    const cosPitch = Math.cos(pitch);
    this.radius = radius;
    this.pitch = pitch;
    this.position[0] = this.target[0] + Math.sin(this.yaw) * cosPitch * radius;
    this.position[1] = this.target[1] + Math.sin(pitch) * radius;
    this.position[2] = this.target[2] + Math.cos(this.yaw) * cosPitch * radius;
    return this;
  }

  getBasis(out = {}) {
    const forward = out.forward || createVec3();
    const right = out.right || createVec3();
    const trueUp = out.up || createVec3();

    subtractVec3(forward, this.target, this.position);
    normalizeVec3(forward, forward, [0, 0, -1]);
    crossVec3(right, forward, this.up);
    normalizeVec3(right, right, [1, 0, 0]);
    crossVec3(trueUp, right, forward);
    normalizeVec3(trueUp, trueUp, [0, 1, 0]);

    out.forward = forward;
    out.right = right;
    out.up = trueUp;
    return out;
  }

  worldUnitsPerPixel(viewportHeight = this.viewport.pixelHeight) {
    const height = Math.max(1, finiteNumber(viewportHeight, this.viewport.pixelHeight));
    return (2 * Math.tan((this.fov * DEG_TO_RAD) / 2) * this.radius) / height;
  }

  applyTo(camera) {
    camera.setPerspective({
      fov: this.fov,
      aspect: this.aspect,
      near: this.near,
      far: this.far,
    });
    camera.setUp(this.up);
    camera.setPosition(this.position);
    camera.lookAt(this.target);
    return camera;
  }

  clone() {
    return new TargetCameraState(this.toJSON());
  }

  toJSON() {
    return {
      mode: this.mode,
      fov: this.fov,
      aspect: this.aspect,
      near: this.near,
      far: this.far,
      position: Array.from(this.position),
      target: Array.from(this.target),
      up: Array.from(this.up),
      yaw: this.yaw,
      pitch: this.pitch,
      radius: this.radius,
      minPitch: this.minPitch,
      maxPitch: this.maxPitch,
      minDistance: this.minDistance,
      maxDistance: this.maxDistance,
      viewport: { ...this.viewport },
    };
  }
}

export function createTargetCameraState(options = {}) {
  return new TargetCameraState(options);
}

export function copyTargetCameraState(out, input) {
  out.mode = input.mode;
  out.setPerspective(input);
  out.minPitch = input.minPitch;
  out.maxPitch = input.maxPitch;
  out.minDistance = input.minDistance;
  out.maxDistance = input.maxDistance;
  copyVec3(out.position, input.position);
  copyVec3(out.target, input.target);
  copyVec3(out.up, input.up);
  out.yaw = input.yaw;
  out.pitch = input.pitch;
  out.radius = input.radius;
  if (input.viewport) {
    out.viewport = { ...input.viewport };
  }
  return out;
}
