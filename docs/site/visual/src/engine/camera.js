import { Mat4, degreesToRadians } from "../math/mat4.js";
import { Vec3, Vec4, clamp } from "../math/vec.js";

export class VisualCamera {
  constructor(options = {}) {
    this.mode = options.mode || "miniaturePerspective";
    this.projectionType = options.projectionType || (this.mode === "metricMap" ? "orthographic" : "perspective");
    this.position = Vec3.create(...(options.position || [3.2, 2.2, 4.2]));
    this.target = Vec3.create(...(options.target || [0, 0, 0]));
    this.up = Vec3.create(...(options.up || [0, 1, 0]));
    this.fovDegrees = options.fovDegrees ?? 50;
    this.aspect = options.aspect || 1;
    this.near = options.near ?? 0.01;
    this.far = options.far ?? 1000;
    this.orthographicSize = options.orthographicSize || 8;
    this.viewMatrix = Mat4.create();
    this.projectionMatrix = Mat4.create();
    this.viewProjectionMatrix = Mat4.create();
    this.inverseViewProjectionMatrix = Mat4.create();
    this._clipNear = Vec4.create();
    this._clipFar = Vec4.create();
    this._worldNear = Vec4.create();
    this._worldFar = Vec4.create();
    this.updateMatrices();
  }

  setMode(mode) {
    this.mode = mode;
    if (mode === "metricMap") {
      this.projectionType = "orthographic";
    } else if (mode === "miniaturePerspective" || mode === "orbitInspect") {
      this.projectionType = "perspective";
    }
    return this.updateMatrices();
  }

  setViewport(width, height) {
    this.aspect = Math.max(1, width) / Math.max(1, height);
    return this.updateMatrices();
  }

  setPerspective(options = {}) {
    this.projectionType = "perspective";
    this.fovDegrees = options.fovDegrees ?? this.fovDegrees;
    this.aspect = options.aspect ?? this.aspect;
    this.near = options.near ?? this.near;
    this.far = options.far ?? this.far;
    return this.updateMatrices();
  }

  setOrthographic(options = {}) {
    this.projectionType = "orthographic";
    this.orthographicSize = options.size ?? options.orthographicSize ?? this.orthographicSize;
    this.aspect = options.aspect ?? this.aspect;
    this.near = options.near ?? this.near;
    this.far = options.far ?? this.far;
    return this.updateMatrices();
  }

  lookAt(position, target = this.target, up = this.up) {
    Vec3.copy(this.position, position);
    Vec3.copy(this.target, target);
    Vec3.copy(this.up, up);
    return this.updateMatrices();
  }

  setTarget(target) {
    Vec3.copy(this.target, target);
    return this.updateMatrices();
  }

  setOrbit(options = {}) {
    const target = options.target || this.target;
    const distance = options.distance ?? Vec3.distance(this.position, target);
    const yaw = options.yaw ?? Math.atan2(this.position[0] - target[0], this.position[2] - target[2]);
    const pitch = clamp(options.pitch ?? 0.5, -Math.PI / 2 + 0.001, Math.PI / 2 - 0.001);
    const cosPitch = Math.cos(pitch);

    Vec3.copy(this.target, target);
    this.position[0] = this.target[0] + Math.sin(yaw) * cosPitch * distance;
    this.position[1] = this.target[1] + Math.sin(pitch) * distance;
    this.position[2] = this.target[2] + Math.cos(yaw) * cosPitch * distance;
    return this.updateMatrices();
  }

  updateMatrices() {
    Mat4.lookAt(this.viewMatrix, this.position, this.target, this.up);

    if (this.projectionType === "orthographic") {
      const halfHeight = this.orthographicSize / 2;
      const halfWidth = halfHeight * this.aspect;
      Mat4.orthographic(
        this.projectionMatrix,
        -halfWidth,
        halfWidth,
        -halfHeight,
        halfHeight,
        this.near,
        this.far,
      );
    } else {
      Mat4.perspective(
        this.projectionMatrix,
        degreesToRadians(this.fovDegrees),
        this.aspect,
        this.near,
        this.far,
      );
    }

    Mat4.multiply(this.viewProjectionMatrix, this.projectionMatrix, this.viewMatrix);
    Mat4.invert(this.inverseViewProjectionMatrix, this.viewProjectionMatrix);
    return this;
  }

  getUniforms() {
    return {
      uViewMatrix: this.viewMatrix,
      uProjectionMatrix: this.projectionMatrix,
      uViewProjectionMatrix: this.viewProjectionMatrix,
      uCameraPosition: this.position,
    };
  }

  screenPointToRay(out, x, y, width, height) {
    const nx = (x / Math.max(1, width)) * 2 - 1;
    const ny = 1 - (y / Math.max(1, height)) * 2;
    Vec4.set(this._clipNear, nx, ny, -1, 1);
    Vec4.set(this._clipFar, nx, ny, 1, 1);
    Vec4.transformMat4(this._worldNear, this._clipNear, this.inverseViewProjectionMatrix);
    Vec4.transformMat4(this._worldFar, this._clipFar, this.inverseViewProjectionMatrix);
    divideByW(this._worldNear);
    divideByW(this._worldFar);

    Vec3.set(out.origin, this._worldNear[0], this._worldNear[1], this._worldNear[2]);
    Vec3.set(
      out.direction,
      this._worldFar[0] - this._worldNear[0],
      this._worldFar[1] - this._worldNear[1],
      this._worldFar[2] - this._worldNear[2],
    );
    Vec3.normalize(out.direction, out.direction);
    return out;
  }
}

function divideByW(point) {
  if (point[3]) {
    point[0] /= point[3];
    point[1] /= point[3];
    point[2] /= point[3];
    point[3] = 1;
  }
}
