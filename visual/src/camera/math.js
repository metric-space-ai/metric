export const DEG_TO_RAD = Math.PI / 180;
export const RAD_TO_DEG = 180 / Math.PI;
export const EPSILON = 1e-6;

export function clamp(value, min, max) {
  return Math.max(min, Math.min(max, value));
}

export function finiteNumber(value, fallback = 0) {
  const number = Number(value);
  return Number.isFinite(number) ? number : fallback;
}

export function createVec3(x = 0, y = 0, z = 0) {
  return new Float32Array([finiteNumber(x), finiteNumber(y), finiteNumber(z)]);
}

export function setVec3(out, value, fallback = [0, 0, 0]) {
  if (Array.isArray(value) || ArrayBuffer.isView(value)) {
    out[0] = finiteNumber(value[0], fallback[0] || 0);
    out[1] = finiteNumber(value[1], fallback[1] || 0);
    out[2] = finiteNumber(value[2], fallback[2] || 0);
    return out;
  }
  if (value && typeof value === "object") {
    out[0] = finiteNumber(value.x, fallback[0] || 0);
    out[1] = finiteNumber(value.y, fallback[1] || 0);
    out[2] = finiteNumber(value.z, fallback[2] || 0);
    return out;
  }
  out[0] = fallback[0] || 0;
  out[1] = fallback[1] || 0;
  out[2] = fallback[2] || 0;
  return out;
}

export function setVec3Components(out, x = 0, y = 0, z = 0) {
  out[0] = finiteNumber(x);
  out[1] = finiteNumber(y);
  out[2] = finiteNumber(z);
  return out;
}

export function copyVec3(out, input) {
  out[0] = input[0];
  out[1] = input[1];
  out[2] = input[2];
  return out;
}

export function addVec3(out, a, b) {
  out[0] = a[0] + b[0];
  out[1] = a[1] + b[1];
  out[2] = a[2] + b[2];
  return out;
}

export function subtractVec3(out, a, b) {
  out[0] = a[0] - b[0];
  out[1] = a[1] - b[1];
  out[2] = a[2] - b[2];
  return out;
}

export function scaleVec3(out, input, scale) {
  out[0] = input[0] * scale;
  out[1] = input[1] * scale;
  out[2] = input[2] * scale;
  return out;
}

export function scaleAndAddVec3(out, a, b, scale) {
  out[0] = a[0] + b[0] * scale;
  out[1] = a[1] + b[1] * scale;
  out[2] = a[2] + b[2] * scale;
  return out;
}

export function lengthVec3(input) {
  return Math.hypot(input[0], input[1], input[2]);
}

export function distanceVec3(a, b) {
  return Math.hypot(a[0] - b[0], a[1] - b[1], a[2] - b[2]);
}

export function normalizeVec3(out, input, fallback = [0, 0, 1]) {
  const length = lengthVec3(input);
  if (length <= EPSILON) {
    return setVec3(out, fallback);
  }
  out[0] = input[0] / length;
  out[1] = input[1] / length;
  out[2] = input[2] / length;
  return out;
}

export function dotVec3(a, b) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

export function crossVec3(out, a, b) {
  const ax = a[0], ay = a[1], az = a[2];
  const bx = b[0], by = b[1], bz = b[2];
  out[0] = ay * bz - az * by;
  out[1] = az * bx - ax * bz;
  out[2] = ax * by - ay * bx;
  return out;
}

export function identityMat4(out = new Float32Array(16)) {
  out[0] = 1; out[4] = 0; out[8] = 0; out[12] = 0;
  out[1] = 0; out[5] = 1; out[9] = 0; out[13] = 0;
  out[2] = 0; out[6] = 0; out[10] = 1; out[14] = 0;
  out[3] = 0; out[7] = 0; out[11] = 0; out[15] = 1;
  return out;
}

export function perspectiveMat4(out, fovDegrees, aspect, near, far) {
  const safeFov = clamp(finiteNumber(fovDegrees, 50), 0.001, 179.999) * DEG_TO_RAD;
  const safeAspect = Math.max(EPSILON, finiteNumber(aspect, 1));
  const safeNear = Math.max(EPSILON, finiteNumber(near, 0.1));
  const safeFar = Math.max(safeNear + EPSILON, finiteNumber(far, 2000));
  const f = 1 / Math.tan(safeFov / 2);
  const nf = 1 / (safeNear - safeFar);

  out[0] = f / safeAspect; out[4] = 0; out[8] = 0; out[12] = 0;
  out[1] = 0; out[5] = f; out[9] = 0; out[13] = 0;
  out[2] = 0; out[6] = 0; out[10] = (safeFar + safeNear) * nf; out[14] = 2 * safeFar * safeNear * nf;
  out[3] = 0; out[7] = 0; out[11] = -1; out[15] = 0;
  return out;
}

export function lookAtMat4(out, eye, target, up) {
  const z = createVec3();
  const x = createVec3();
  const y = createVec3();

  subtractVec3(z, eye, target);
  if (lengthVec3(z) <= EPSILON) z[2] = 1;
  normalizeVec3(z, z);

  crossVec3(x, up, z);
  if (lengthVec3(x) <= EPSILON) {
    crossVec3(x, Math.abs(z[1]) > 0.9 ? [1, 0, 0] : [0, 1, 0], z);
  }
  normalizeVec3(x, x);
  crossVec3(y, z, x);

  out[0] = x[0]; out[4] = x[1]; out[8] = x[2]; out[12] = -dotVec3(x, eye);
  out[1] = y[0]; out[5] = y[1]; out[9] = y[2]; out[13] = -dotVec3(y, eye);
  out[2] = z[0]; out[6] = z[1]; out[10] = z[2]; out[14] = -dotVec3(z, eye);
  out[3] = 0; out[7] = 0; out[11] = 0; out[15] = 1;
  return out;
}

export function multiplyMat4(out, a, b) {
  const a00 = a[0], a01 = a[1], a02 = a[2], a03 = a[3];
  const a10 = a[4], a11 = a[5], a12 = a[6], a13 = a[7];
  const a20 = a[8], a21 = a[9], a22 = a[10], a23 = a[11];
  const a30 = a[12], a31 = a[13], a32 = a[14], a33 = a[15];

  let b0 = b[0], b1 = b[1], b2 = b[2], b3 = b[3];
  out[0] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
  out[1] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
  out[2] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
  out[3] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;

  b0 = b[4]; b1 = b[5]; b2 = b[6]; b3 = b[7];
  out[4] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
  out[5] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
  out[6] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
  out[7] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;

  b0 = b[8]; b1 = b[9]; b2 = b[10]; b3 = b[11];
  out[8] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
  out[9] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
  out[10] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
  out[11] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;

  b0 = b[12]; b1 = b[13]; b2 = b[14]; b3 = b[15];
  out[12] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
  out[13] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
  out[14] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
  out[15] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;
  return out;
}

export function projectPointToPixel(out, position, viewProjectionMatrix, width, height) {
  const x = position[0], y = position[1], z = position[2];
  const clipX = viewProjectionMatrix[0] * x + viewProjectionMatrix[4] * y + viewProjectionMatrix[8] * z + viewProjectionMatrix[12];
  const clipY = viewProjectionMatrix[1] * x + viewProjectionMatrix[5] * y + viewProjectionMatrix[9] * z + viewProjectionMatrix[13];
  const clipZ = viewProjectionMatrix[2] * x + viewProjectionMatrix[6] * y + viewProjectionMatrix[10] * z + viewProjectionMatrix[14];
  const clipW = viewProjectionMatrix[3] * x + viewProjectionMatrix[7] * y + viewProjectionMatrix[11] * z + viewProjectionMatrix[15];
  const target = out || {};

  if (Math.abs(clipW) <= EPSILON) {
    target.x = 0;
    target.y = 0;
    target.depth = 0;
    target.ndcX = 0;
    target.ndcY = 0;
    target.visible = false;
    return target;
  }

  const ndcX = clipX / clipW;
  const ndcY = clipY / clipW;
  const ndcZ = clipZ / clipW;
  target.x = (ndcX * 0.5 + 0.5) * width;
  target.y = (-ndcY * 0.5 + 0.5) * height;
  target.depth = ndcZ;
  target.ndcX = ndcX;
  target.ndcY = ndcY;
  target.visible = clipW > 0 && ndcX >= -1 && ndcX <= 1 && ndcY >= -1 && ndcY <= 1 && ndcZ >= -1 && ndcZ <= 1;
  return target;
}
