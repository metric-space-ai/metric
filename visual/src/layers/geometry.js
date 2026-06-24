export const CUBE_VERTEX_COUNT = 36;

export function createCubeGeometry() {
  const vertices = [
    [-0.5, -0.5, 0.5], [0.5, -0.5, 0.5], [0.5, 0.5, 0.5],
    [-0.5, -0.5, 0.5], [0.5, 0.5, 0.5], [-0.5, 0.5, 0.5],
    [0.5, -0.5, -0.5], [-0.5, -0.5, -0.5], [-0.5, 0.5, -0.5],
    [0.5, -0.5, -0.5], [-0.5, 0.5, -0.5], [0.5, 0.5, -0.5],
    [-0.5, 0.5, 0.5], [0.5, 0.5, 0.5], [0.5, 0.5, -0.5],
    [-0.5, 0.5, 0.5], [0.5, 0.5, -0.5], [-0.5, 0.5, -0.5],
    [0.5, -0.5, 0.5], [-0.5, -0.5, 0.5], [-0.5, -0.5, -0.5],
    [0.5, -0.5, 0.5], [-0.5, -0.5, -0.5], [0.5, -0.5, -0.5],
    [0.5, 0.5, 0.5], [0.5, -0.5, 0.5], [0.5, -0.5, -0.5],
    [0.5, 0.5, 0.5], [0.5, -0.5, -0.5], [0.5, 0.5, -0.5],
    [-0.5, -0.5, 0.5], [-0.5, 0.5, 0.5], [-0.5, 0.5, -0.5],
    [-0.5, -0.5, 0.5], [-0.5, 0.5, -0.5], [-0.5, -0.5, -0.5],
  ];
  const normals = [
    [0, 0, 1], [0, 0, 1], [0, 0, 1], [0, 0, 1], [0, 0, 1], [0, 0, 1],
    [0, 0, -1], [0, 0, -1], [0, 0, -1], [0, 0, -1], [0, 0, -1], [0, 0, -1],
    [0, 1, 0], [0, 1, 0], [0, 1, 0], [0, 1, 0], [0, 1, 0], [0, 1, 0],
    [0, -1, 0], [0, -1, 0], [0, -1, 0], [0, -1, 0], [0, -1, 0], [0, -1, 0],
    [1, 0, 0], [1, 0, 0], [1, 0, 0], [1, 0, 0], [1, 0, 0], [1, 0, 0],
    [-1, 0, 0], [-1, 0, 0], [-1, 0, 0], [-1, 0, 0], [-1, 0, 0], [-1, 0, 0],
  ];
  return {
    positions: new Float32Array(vertices.flat()),
    normals: new Float32Array(normals.flat()),
  };
}

export function computeVertexNormals(positions, indices = null) {
  const vertexCount = Math.floor(positions.length / 3);
  const normals = new Float32Array(vertexCount * 3);
  const triangleCount = indices ? Math.floor(indices.length / 3) : Math.floor(vertexCount / 3);

  for (let triangle = 0; triangle < triangleCount; triangle += 1) {
    const ia = indices ? indices[triangle * 3] : triangle * 3;
    const ib = indices ? indices[triangle * 3 + 1] : triangle * 3 + 1;
    const ic = indices ? indices[triangle * 3 + 2] : triangle * 3 + 2;
    const ao = ia * 3;
    const bo = ib * 3;
    const co = ic * 3;
    const abx = positions[bo] - positions[ao];
    const aby = positions[bo + 1] - positions[ao + 1];
    const abz = positions[bo + 2] - positions[ao + 2];
    const acx = positions[co] - positions[ao];
    const acy = positions[co + 1] - positions[ao + 1];
    const acz = positions[co + 2] - positions[ao + 2];
    const nx = aby * acz - abz * acy;
    const ny = abz * acx - abx * acz;
    const nz = abx * acy - aby * acx;
    addNormal(normals, ia, nx, ny, nz);
    addNormal(normals, ib, nx, ny, nz);
    addNormal(normals, ic, nx, ny, nz);
  }

  for (let index = 0; index < vertexCount; index += 1) {
    const offset = index * 3;
    const x = normals[offset];
    const y = normals[offset + 1];
    const z = normals[offset + 2];
    const length = Math.hypot(x, y, z) || 1;
    normals[offset] = x / length;
    normals[offset + 1] = y / length;
    normals[offset + 2] = z / length;
  }
  return normals;
}

export function normalizeIndices(gl, indices, vertexCount, capabilities) {
  if (!indices) return null;
  const source = ArrayBuffer.isView(indices) ? indices : new Uint32Array(indices);
  let max = 0;
  for (const index of source) max = Math.max(max, Number(index) || 0);
  if (source instanceof Uint16Array || source instanceof Uint8Array) return source;
  if (max <= 65535) return new Uint16Array(source);
  if (capabilities?.extensions?.elementIndexUint) return source instanceof Uint32Array ? source : new Uint32Array(source);
  throw new Error(`Surface indices require Uint32 support for ${vertexCount} vertices.`);
}

export function indexTypeFor(gl, indices) {
  if (indices instanceof Uint32Array) return gl.UNSIGNED_INT;
  if (indices instanceof Uint8Array) return gl.UNSIGNED_BYTE;
  return gl.UNSIGNED_SHORT;
}

export function projectToGroundPlane(positions, geometry = {}) {
  const out = new Float32Array(positions);
  const plane = geometry.plane || (Number.isFinite(Number(geometry.groundZ)) ? "xy" : "xz");
  if (Number.isFinite(Number(geometry.groundZ)) || plane === "xy") {
    const groundZ = Number.isFinite(Number(geometry.groundZ)) ? Number(geometry.groundZ) : 0;
    for (let index = 0; index < out.length; index += 3) out[index + 2] = groundZ;
  } else {
    const groundY = Number.isFinite(Number(geometry.groundY)) ? Number(geometry.groundY) : 0;
    for (let index = 0; index < out.length; index += 3) out[index + 1] = groundY;
  }
  return out;
}

export function buildFootprintTriangles(footprints = [], geometry = {}) {
  const positions = [];
  const colors = [];
  for (let index = 0; index < footprints.length; index += 1) {
    const footprint = footprints[index];
    const vertices = footprintVertices(footprint);
    if (vertices.length < 3) continue;
    const color = normalizeFootprintColor(footprint?.color, index, footprint?.alpha);
    for (let triangle = 1; triangle < vertices.length - 1; triangle += 1) {
      pushProjectedVertex(positions, vertices[0], geometry);
      pushProjectedVertex(positions, vertices[triangle], geometry);
      pushProjectedVertex(positions, vertices[triangle + 1], geometry);
      colors.push(...color, ...color, ...color);
    }
  }
  return {
    positions: new Float32Array(positions),
    colors: new Float32Array(colors),
    count: Math.floor(positions.length / 3),
  };
}

function addNormal(normals, index, x, y, z) {
  const offset = index * 3;
  normals[offset] += x;
  normals[offset + 1] += y;
  normals[offset + 2] += z;
}

function footprintVertices(footprint) {
  const source = footprint?.vertices || footprint?.points || footprint?.polygon || footprint?.path || footprint;
  if (!Array.isArray(source)) return [];
  return source.filter((value) => Array.isArray(value) || ArrayBuffer.isView(value));
}

function pushProjectedVertex(out, vertex, geometry) {
  const x = Number(vertex[0]) || 0;
  const y = Number(vertex[1]) || 0;
  const z = Number(vertex[2]) || 0;
  if (Number.isFinite(Number(geometry.groundZ)) || geometry.plane === "xy") {
    out.push(x, y, Number.isFinite(Number(geometry.groundZ)) ? Number(geometry.groundZ) : z);
  } else {
    out.push(x, Number.isFinite(Number(geometry.groundY)) ? Number(geometry.groundY) : y, z);
  }
}

function normalizeFootprintColor(color, index, alpha = 0.22) {
  if (Array.isArray(color) || ArrayBuffer.isView(color)) {
    const divisor = Math.max(color[0] || 0, color[1] || 0, color[2] || 0) > 1 ? 255 : 1;
    return [
      clamp01((Number(color[0]) || 0) / divisor),
      clamp01((Number(color[1]) || 0) / divisor),
      clamp01((Number(color[2]) || 0) / divisor),
      clamp01(color[3] == null ? alpha : Number(color[3])),
    ];
  }
  const palette = [
    [0.16, 0.37, 0.60, alpha],
    [0.82, 0.29, 0.36, alpha],
    [0.16, 0.62, 0.56, alpha],
    [0.89, 0.52, 0.22, alpha],
  ];
  return palette[index % palette.length];
}

function clamp01(value) {
  return Math.max(0, Math.min(1, Number.isFinite(value) ? value : 0));
}

