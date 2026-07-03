import { BaseView } from "./BaseView.js";
import { VisualLayer } from "./VisualLayer.js";
import {
  colorChannelFrom,
  createChannel,
  descriptorSource,
  extractRelationCells,
  inferScalarDomain,
  resolveCollectionItem,
} from "./view-utils.js";

export class SurfaceView extends BaseView {
  constructor(options = {}) {
    super({ ...options, kind: "surface" });
    this.values = options.values || options.matrix || options.grid || [];
    this.colors = options.colors;
    this.xScale = Number.isFinite(Number(options.xScale)) ? Number(options.xScale) : 1;
    this.yScale = Number.isFinite(Number(options.yScale)) ? Number(options.yScale) : 1;
    this.zScale = Number.isFinite(Number(options.zScale)) ? Number(options.zScale) : 1;
    this.alpha = Number.isFinite(Number(options.alpha)) ? Number(options.alpha) : 1;
    this.material = {
      lighting: "surface",
      diffuse: "vertex-color",
      ambient: 0.48,
      pointLight: 0.42,
      specular: 0.1,
      alphaMode: this.alpha < 1 ? "blend" : "opaque",
      backFace: "double-sided",
      ...(options.material || {}),
    };
  }

  static fromVisualSpace(document, options = {}) {
    const relation = resolveCollectionItem(document, "relations", options.relation || options.relationId || options.relation_id);
    let values = options.values || options.matrix || options.grid;
    if (!values && relation) {
      const extracted = extractRelationCells(relation, options);
      values = cellsToMatrix(extracted.cells, extracted.rowIds.length, extracted.columnIds.length);
    }
    return new SurfaceView({
      ...options,
      values,
      relationId: relation?.id,
      datasetId: options.datasetId ?? relation?.dataset_id ?? relation?.datasetId,
    });
  }

  toLayerDescriptors() {
    const matrix = Array.isArray(this.values) ? this.values : [];
    const rows = matrix.length;
    const columns = rows ? matrix[0].length : 0;
    const vertexCount = rows * columns;
    const positions = new Float32Array(vertexCount * 3);
    const scalars = new Float32Array(vertexCount);
    const vertexIds = new Array(vertexCount);

    for (let row = 0; row < rows; row += 1) {
      for (let column = 0; column < columns; column += 1) {
        const index = row * columns + column;
        const value = Number(matrix[row][column]) || 0;
        vertexIds[index] = `${row}:${column}`;
        positions[index * 3] = column * this.xScale;
        positions[index * 3 + 1] = value * this.yScale;
        positions[index * 3 + 2] = row * this.zScale;
        scalars[index] = value;
      }
    }

    const indices = buildGridIndices(rows, columns);

    return [new VisualLayer({
      id: `${this.id}:surface`,
      kind: "surface",
      primitive: "SurfaceLayer",
      source: descriptorSource(this),
      channels: {
        position: createChannel(positions, 3, "vertex-position"),
        scalar: createChannel(scalars, 1, "scalar", { domain: inferScalarDomain(Array.from(scalars)) }),
        color: colorChannelFrom({
          ids: vertexIds,
          colors: this.colors,
          scalarValues: scalars,
          alpha: this.alpha,
        }),
        alpha: createChannel(new Float32Array(vertexCount).fill(this.alpha), 1, "alpha"),
      },
      geometry: {
        topology: "triangle-grid",
        rows,
        columns,
        indices,
        scale: [this.xScale, this.yScale, this.zScale],
        normalPolicy: "renderer-compute",
      },
      material: this.material,
      animation: { mode: "none" },
      picking: {
        mode: "vertex",
      },
      metadata: {
        ...this.metadata,
        vertexCount,
        triangleCount: Math.floor(indices.length / 3),
      },
    }).toDescriptor()];
  }
}

export function createSurfaceView(options) {
  return new SurfaceView(options);
}

function buildGridIndices(rows, columns) {
  const triangleCount = Math.max(0, rows - 1) * Math.max(0, columns - 1) * 2;
  const indices = new Uint32Array(triangleCount * 3);
  let offset = 0;
  for (let row = 0; row < rows - 1; row += 1) {
    for (let column = 0; column < columns - 1; column += 1) {
      const a = column + row * columns;
      const b = column + (row + 1) * columns;
      const c = column + 1 + row * columns;
      const d = column + 1 + (row + 1) * columns;
      indices[offset] = a;
      indices[offset + 1] = b;
      indices[offset + 2] = c;
      indices[offset + 3] = c;
      indices[offset + 4] = b;
      indices[offset + 5] = d;
      offset += 6;
    }
  }
  return indices;
}

function cellsToMatrix(cells, rows, columns) {
  const matrix = Array.from({ length: rows }, () => new Array(columns).fill(0));
  for (const cell of cells) {
    if (cell.row >= 0 && cell.row < rows && cell.column >= 0 && cell.column < columns) {
      matrix[cell.row][cell.column] = Number(cell.value) || 0;
    }
  }
  return matrix;
}

