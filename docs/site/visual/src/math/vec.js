export const EPSILON = 1e-6;

export function clamp(value, min, max) {
  return Math.min(max, Math.max(min, value));
}

export function lerp(a, b, t) {
  return a + (b - a) * t;
}

function length2(x, y) {
  return Math.hypot(x, y);
}

function length3(x, y, z) {
  return Math.hypot(x, y, z);
}

export const Vec2 = Object.freeze({
  create(x = 0, y = 0) {
    const out = new Float32Array(2);
    out[0] = x;
    out[1] = y;
    return out;
  },

  clone(a) {
    return new Float32Array(a);
  },

  set(out, x = 0, y = 0) {
    out[0] = x;
    out[1] = y;
    return out;
  },

  copy(out, a) {
    out[0] = a[0];
    out[1] = a[1];
    return out;
  },

  fromArray(out, array, offset = 0) {
    out[0] = array[offset];
    out[1] = array[offset + 1];
    return out;
  },

  toArray(out, a, offset = 0) {
    out[offset] = a[0];
    out[offset + 1] = a[1];
    return out;
  },

  add(out, a, b) {
    out[0] = a[0] + b[0];
    out[1] = a[1] + b[1];
    return out;
  },

  subtract(out, a, b) {
    out[0] = a[0] - b[0];
    out[1] = a[1] - b[1];
    return out;
  },

  multiply(out, a, b) {
    out[0] = a[0] * b[0];
    out[1] = a[1] * b[1];
    return out;
  },

  scale(out, a, scalar) {
    out[0] = a[0] * scalar;
    out[1] = a[1] * scalar;
    return out;
  },

  scaleAndAdd(out, a, b, scalar) {
    out[0] = a[0] + b[0] * scalar;
    out[1] = a[1] + b[1] * scalar;
    return out;
  },

  dot(a, b) {
    return a[0] * b[0] + a[1] * b[1];
  },

  squaredLength(a) {
    return a[0] * a[0] + a[1] * a[1];
  },

  length(a) {
    return length2(a[0], a[1]);
  },

  squaredDistance(a, b) {
    const x = a[0] - b[0];
    const y = a[1] - b[1];
    return x * x + y * y;
  },

  distance(a, b) {
    return length2(a[0] - b[0], a[1] - b[1]);
  },

  normalize(out, a) {
    const len = length2(a[0], a[1]);
    if (len > EPSILON) {
      out[0] = a[0] / len;
      out[1] = a[1] / len;
    } else {
      out[0] = 0;
      out[1] = 0;
    }
    return out;
  },

  lerp(out, a, b, t) {
    out[0] = lerp(a[0], b[0], t);
    out[1] = lerp(a[1], b[1], t);
    return out;
  },
});

export const Vec3 = Object.freeze({
  create(x = 0, y = 0, z = 0) {
    const out = new Float32Array(3);
    out[0] = x;
    out[1] = y;
    out[2] = z;
    return out;
  },

  clone(a) {
    return new Float32Array(a);
  },

  set(out, x = 0, y = 0, z = 0) {
    out[0] = x;
    out[1] = y;
    out[2] = z;
    return out;
  },

  copy(out, a) {
    out[0] = a[0];
    out[1] = a[1];
    out[2] = a[2];
    return out;
  },

  fromArray(out, array, offset = 0) {
    out[0] = array[offset];
    out[1] = array[offset + 1];
    out[2] = array[offset + 2];
    return out;
  },

  toArray(out, a, offset = 0) {
    out[offset] = a[0];
    out[offset + 1] = a[1];
    out[offset + 2] = a[2];
    return out;
  },

  add(out, a, b) {
    out[0] = a[0] + b[0];
    out[1] = a[1] + b[1];
    out[2] = a[2] + b[2];
    return out;
  },

  subtract(out, a, b) {
    out[0] = a[0] - b[0];
    out[1] = a[1] - b[1];
    out[2] = a[2] - b[2];
    return out;
  },

  multiply(out, a, b) {
    out[0] = a[0] * b[0];
    out[1] = a[1] * b[1];
    out[2] = a[2] * b[2];
    return out;
  },

  min(out, a, b) {
    out[0] = Math.min(a[0], b[0]);
    out[1] = Math.min(a[1], b[1]);
    out[2] = Math.min(a[2], b[2]);
    return out;
  },

  max(out, a, b) {
    out[0] = Math.max(a[0], b[0]);
    out[1] = Math.max(a[1], b[1]);
    out[2] = Math.max(a[2], b[2]);
    return out;
  },

  scale(out, a, scalar) {
    out[0] = a[0] * scalar;
    out[1] = a[1] * scalar;
    out[2] = a[2] * scalar;
    return out;
  },

  scaleAndAdd(out, a, b, scalar) {
    out[0] = a[0] + b[0] * scalar;
    out[1] = a[1] + b[1] * scalar;
    out[2] = a[2] + b[2] * scalar;
    return out;
  },

  dot(a, b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
  },

  cross(out, a, b) {
    const ax = a[0];
    const ay = a[1];
    const az = a[2];
    const bx = b[0];
    const by = b[1];
    const bz = b[2];
    out[0] = ay * bz - az * by;
    out[1] = az * bx - ax * bz;
    out[2] = ax * by - ay * bx;
    return out;
  },

  squaredLength(a) {
    return a[0] * a[0] + a[1] * a[1] + a[2] * a[2];
  },

  length(a) {
    return length3(a[0], a[1], a[2]);
  },

  squaredDistance(a, b) {
    const x = a[0] - b[0];
    const y = a[1] - b[1];
    const z = a[2] - b[2];
    return x * x + y * y + z * z;
  },

  distance(a, b) {
    return length3(a[0] - b[0], a[1] - b[1], a[2] - b[2]);
  },

  normalize(out, a) {
    const len = length3(a[0], a[1], a[2]);
    if (len > EPSILON) {
      out[0] = a[0] / len;
      out[1] = a[1] / len;
      out[2] = a[2] / len;
    } else {
      out[0] = 0;
      out[1] = 0;
      out[2] = 0;
    }
    return out;
  },

  lerp(out, a, b, t) {
    out[0] = lerp(a[0], b[0], t);
    out[1] = lerp(a[1], b[1], t);
    out[2] = lerp(a[2], b[2], t);
    return out;
  },

  transformMat4(out, a, matrix) {
    const x = a[0];
    const y = a[1];
    const z = a[2];
    const w = matrix[3] * x + matrix[7] * y + matrix[11] * z + matrix[15];
    const invW = w ? 1 / w : 1;
    out[0] = (matrix[0] * x + matrix[4] * y + matrix[8] * z + matrix[12]) * invW;
    out[1] = (matrix[1] * x + matrix[5] * y + matrix[9] * z + matrix[13]) * invW;
    out[2] = (matrix[2] * x + matrix[6] * y + matrix[10] * z + matrix[14]) * invW;
    return out;
  },

  transformDirectionMat4(out, a, matrix) {
    const x = a[0];
    const y = a[1];
    const z = a[2];
    out[0] = matrix[0] * x + matrix[4] * y + matrix[8] * z;
    out[1] = matrix[1] * x + matrix[5] * y + matrix[9] * z;
    out[2] = matrix[2] * x + matrix[6] * y + matrix[10] * z;
    return Vec3.normalize(out, out);
  },
});

export const Vec4 = Object.freeze({
  create(x = 0, y = 0, z = 0, w = 0) {
    const out = new Float32Array(4);
    out[0] = x;
    out[1] = y;
    out[2] = z;
    out[3] = w;
    return out;
  },

  set(out, x = 0, y = 0, z = 0, w = 0) {
    out[0] = x;
    out[1] = y;
    out[2] = z;
    out[3] = w;
    return out;
  },

  copy(out, a) {
    out[0] = a[0];
    out[1] = a[1];
    out[2] = a[2];
    out[3] = a[3];
    return out;
  },

  transformMat4(out, a, matrix) {
    const x = a[0];
    const y = a[1];
    const z = a[2];
    const w = a[3];
    out[0] = matrix[0] * x + matrix[4] * y + matrix[8] * z + matrix[12] * w;
    out[1] = matrix[1] * x + matrix[5] * y + matrix[9] * z + matrix[13] * w;
    out[2] = matrix[2] * x + matrix[6] * y + matrix[10] * z + matrix[14] * w;
    out[3] = matrix[3] * x + matrix[7] * y + matrix[11] * z + matrix[15] * w;
    return out;
  },
});
