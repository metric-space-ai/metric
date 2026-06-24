import {
  copyVec3,
  createVec3,
  finiteNumber,
  lookAtMat4,
  multiplyMat4,
  perspectiveMat4,
  projectPointToPixel,
  setVec3,
} from "./math.js";

export class PerspectiveCamera {
  constructor(options = {}) {
    this.type = "PerspectiveCamera";
    this.fov = finiteNumber(options.fov, 50);
    this.aspect = Math.max(1e-6, finiteNumber(options.aspect, 1));
    this.near = Math.max(1e-6, finiteNumber(options.near, 0.1));
    this.far = Math.max(this.near + 1e-6, finiteNumber(options.far, 2000));
    this.position = createVec3();
    this.target = createVec3();
    this.up = createVec3(0, 1, 0);
    this.projectionMatrix = new Float32Array(16);
    this.viewMatrix = new Float32Array(16);
    this.viewProjectionMatrix = new Float32Array(16);
    this.viewport = {
      width: 1,
      height: 1,
      pixelWidth: 1,
      pixelHeight: 1,
      pixelRatio: 1,
    };
    this.version = 0;
    this.matrixVersion = 0;
    this.dirty = true;

    setVec3(this.position, options.position || [0, 0, 1]);
    setVec3(this.target, options.target || options.lookAt || [0, 0, 0]);
    setVec3(this.up, options.up || [0, 1, 0], [0, 1, 0]);
    if (options.viewport) {
      this.setViewport(
        options.viewport.width || options.viewport.pixelWidth,
        options.viewport.height || options.viewport.pixelHeight,
        options.viewport.pixelRatio || 1,
      );
    }
    this.updateMatrices(true);
  }

  setPerspective(options = {}) {
    const nextFov = finiteNumber(options.fov, this.fov);
    const nextAspect = Math.max(1e-6, finiteNumber(options.aspect, this.aspect));
    const nextNear = Math.max(1e-6, finiteNumber(options.near, this.near));
    const nextFar = Math.max(nextNear + 1e-6, finiteNumber(options.far, this.far));
    if (nextFov !== this.fov || nextAspect !== this.aspect || nextNear !== this.near || nextFar !== this.far) {
      this.fov = nextFov;
      this.aspect = nextAspect;
      this.near = nextNear;
      this.far = nextFar;
      this.markDirty();
    }
    return this;
  }

  setViewport(width, height, pixelRatio = 1) {
    const safeWidth = Math.max(1, finiteNumber(width, this.viewport.width));
    const safeHeight = Math.max(1, finiteNumber(height, this.viewport.height));
    const safeRatio = Math.max(1e-6, finiteNumber(pixelRatio, this.viewport.pixelRatio));
    const pixelWidth = Math.max(1, Math.round(safeWidth * safeRatio));
    const pixelHeight = Math.max(1, Math.round(safeHeight * safeRatio));
    const aspect = safeWidth / safeHeight;

    if (
      this.viewport.width !== safeWidth ||
      this.viewport.height !== safeHeight ||
      this.viewport.pixelRatio !== safeRatio ||
      this.viewport.pixelWidth !== pixelWidth ||
      this.viewport.pixelHeight !== pixelHeight ||
      this.aspect !== aspect
    ) {
      this.viewport.width = safeWidth;
      this.viewport.height = safeHeight;
      this.viewport.pixelRatio = safeRatio;
      this.viewport.pixelWidth = pixelWidth;
      this.viewport.pixelHeight = pixelHeight;
      this.aspect = aspect;
      this.markDirty();
    }
    return this;
  }

  setDrawingBufferSize(width, height) {
    return this.setViewport(width, height, 1);
  }

  resizeToCanvas(canvas, pixelRatio = 1) {
    const rect = canvas.getBoundingClientRect ? canvas.getBoundingClientRect() : undefined;
    const cssWidth = rect?.width || canvas.clientWidth || canvas.width || 1;
    const cssHeight = rect?.height || canvas.clientHeight || canvas.height || 1;
    return this.setViewport(cssWidth, cssHeight, pixelRatio);
  }

  setPosition(positionOrX, y, z) {
    if (typeof positionOrX === "number") {
      this.position[0] = finiteNumber(positionOrX);
      this.position[1] = finiteNumber(y);
      this.position[2] = finiteNumber(z);
    } else {
      setVec3(this.position, positionOrX);
    }
    this.markDirty();
    return this;
  }

  setTarget(targetOrX, y, z) {
    if (typeof targetOrX === "number") {
      this.target[0] = finiteNumber(targetOrX);
      this.target[1] = finiteNumber(y);
      this.target[2] = finiteNumber(z);
    } else {
      setVec3(this.target, targetOrX);
    }
    this.markDirty();
    return this;
  }

  lookAt(targetOrX, y, z) {
    return this.setTarget(targetOrX, y, z);
  }

  setUp(up) {
    setVec3(this.up, up, [0, 1, 0]);
    this.markDirty();
    return this;
  }

  applyState(state) {
    this.setPerspective(state);
    this.setPosition(state.position);
    this.setUp(state.up || this.up);
    this.lookAt(state.target || state.lookAt || this.target);
    if (state.viewport) {
      this.setViewport(
        state.viewport.width || state.viewport.pixelWidth,
        state.viewport.height || state.viewport.pixelHeight,
        state.viewport.pixelRatio || 1,
      );
    }
    return this;
  }

  markDirty() {
    this.dirty = true;
    this.version += 1;
    return this;
  }

  updateMatrices(force = false) {
    if (!force && !this.dirty) return this;
    perspectiveMat4(this.projectionMatrix, this.fov, this.aspect, this.near, this.far);
    lookAtMat4(this.viewMatrix, this.position, this.target, this.up);
    multiplyMat4(this.viewProjectionMatrix, this.projectionMatrix, this.viewMatrix);
    this.dirty = false;
    this.matrixVersion += 1;
    return this;
  }

  getMatrices() {
    this.updateMatrices();
    return {
      projectionMatrix: this.projectionMatrix,
      viewMatrix: this.viewMatrix,
      viewProjectionMatrix: this.viewProjectionMatrix,
      matrixVersion: this.matrixVersion,
    };
  }

  projectToPixel(position, out = {}) {
    this.updateMatrices();
    return projectPointToPixel(out, position, this.viewProjectionMatrix, this.viewport.pixelWidth, this.viewport.pixelHeight);
  }

  clone() {
    return new PerspectiveCamera(this.toJSON());
  }

  toJSON() {
    return {
      type: this.type,
      fov: this.fov,
      aspect: this.aspect,
      near: this.near,
      far: this.far,
      position: Array.from(this.position),
      target: Array.from(this.target),
      lookAt: Array.from(this.target),
      up: Array.from(this.up),
      viewport: { ...this.viewport },
    };
  }
}

export const VisualCamera = PerspectiveCamera;

export function createPerspectiveCamera(options = {}) {
  return new PerspectiveCamera(options);
}

export function updateCameraForResize(camera, width, height, pixelRatio = 1) {
  camera.setViewport(width, height, pixelRatio);
  camera.updateMatrices();
  return camera;
}

export function copyCameraVectors(out, camera) {
  copyVec3(out.position, camera.position);
  copyVec3(out.target, camera.target);
  copyVec3(out.up, camera.up);
  return out;
}
