import { BaseView } from "./BaseView.js";
import { VisualLayer } from "./VisualLayer.js";
import {
  colorChannelFrom,
  createChannel,
  createStringChannel,
  descriptorSource,
  extractCoordinatePositions,
  extractPropertyValues,
  flattenValues,
  flattenVectors,
  inferRecordIds,
  inferScalarDomain,
  recordsFor,
  resolveCollectionItem,
} from "./view-utils.js";

export class DenseFieldView extends BaseView {
  constructor(options = {}) {
    super({ ...options, kind: "dense-field" });
    this.mode = options.mode || (options.grid || options.matrix ? "grid" : "sampled-records");
    this.recordIds = inferRecordIds(options);
    this.positions = options.positions || options.coordinates || new Map();
    this.values = options.values || options.scalarValues || new Map();
    this.colors = options.colors;
    this.grid = options.grid || options.matrix;
    this.gridShape = options.gridShape || options.shape;
    this.cellSize = options.cellSize || [1, 1];
    this.origin = options.origin || [0, 0, 0];
    this.plane = options.plane || "xz";
    this.alpha = Number.isFinite(Number(options.alpha)) ? Number(options.alpha) : 0.72;
    this.radius = Number.isFinite(Number(options.radius)) ? Number(options.radius) : 1;
    this.material = {
      lighting: "field",
      diffuse: "scalar-ramp",
      alphaMode: "blend",
      ...(options.material || {}),
    };
  }

  static fromVisualSpace(document, options = {}) {
    const coordinate = resolveCollectionItem(document, "coordinates", options.coordinate || options.coordinateId || options.coordinate_id);
    const property = resolveCollectionItem(document, "properties", options.property || options.propertyId || options.property_id);
    const records = recordsFor(document, {
      ...options,
      datasetId: options.datasetId ?? property?.dataset_id ?? coordinate?.dataset_id,
    });
    const positions = extractCoordinatePositions(coordinate, { records, recordIds: options.recordIds });

    return new DenseFieldView({
      ...options,
      records,
      recordIds: positions.ids,
      positions: positions.positions,
      values: property ? extractPropertyValues(property, { records, recordIds: positions.ids }) : options.values,
      datasetId: options.datasetId ?? property?.dataset_id ?? property?.datasetId ?? coordinate?.dataset_id,
      spaceId: options.spaceId ?? coordinate?.space_id ?? coordinate?.spaceId,
      coordinateId: coordinate?.id,
      propertyId: property?.id,
    });
  }

  toLayerDescriptors() {
    if (this.mode === "grid") return [this.gridDescriptor()];
    return [this.sampleDescriptor()];
  }

  sampleDescriptor() {
    const scalarData = flattenValues(this.values, this.recordIds, 0);
    return new VisualLayer({
      id: `${this.id}:field`,
      kind: "dense-field",
      primitive: "HeatFieldLayer",
      source: descriptorSource(this),
      channels: {
        recordId: createStringChannel(this.recordIds, "record-id"),
        position: createChannel(flattenVectors(this.positions, this.recordIds, 3), 3, "position"),
        scalar: createChannel(scalarData, 1, "scalar", { domain: inferScalarDomain(Array.from(scalarData)) }),
        color: colorChannelFrom({
          ids: this.recordIds,
          colors: this.colors,
          scalarValues: this.values,
          alpha: this.alpha,
        }),
        radius: createChannel(new Float32Array(this.recordIds.length).fill(this.radius), 1, "influence-radius"),
        alpha: createChannel(new Float32Array(this.recordIds.length).fill(this.alpha), 1, "alpha"),
      },
      geometry: {
        mode: "sampled-records",
        plane: this.plane,
        sampleCount: this.recordIds.length,
        interpolation: "renderer-defined",
      },
      material: this.material,
      animation: { mode: "none" },
      picking: {
        mode: "record-id",
        channel: "recordId",
      },
      metadata: this.metadata,
    }).toDescriptor();
  }

  gridDescriptor() {
    const matrix = Array.isArray(this.grid) ? this.grid : [];
    const rows = matrix.length;
    const columns = rows ? matrix[0].length : 0;
    const ids = [];
    const positions = new Float32Array(rows * columns * 3);
    const scalars = new Float32Array(rows * columns);
    let offset = 0;
    for (let row = 0; row < rows; row += 1) {
      for (let column = 0; column < columns; column += 1) {
        const index = row * columns + column;
        ids.push(`${row}:${column}`);
        positions[offset] = this.origin[0] + column * this.cellSize[0];
        positions[offset + 1] = this.origin[1];
        positions[offset + 2] = this.origin[2] + row * this.cellSize[1];
        scalars[index] = Number(matrix[row][column]) || 0;
        offset += 3;
      }
    }

    return new VisualLayer({
      id: `${this.id}:grid-field`,
      kind: "dense-field",
      primitive: "HeatFieldLayer",
      source: descriptorSource(this),
      channels: {
        cellId: createStringChannel(ids, "cell-id"),
        position: createChannel(positions, 3, "cell-center"),
        scalar: createChannel(scalars, 1, "scalar", { domain: inferScalarDomain(Array.from(scalars)) }),
        color: colorChannelFrom({ ids, scalarValues: scalars, alpha: this.alpha }),
        alpha: createChannel(new Float32Array(ids.length).fill(this.alpha), 1, "alpha"),
      },
      geometry: {
        mode: "grid",
        rows,
        columns,
        cellSize: this.cellSize.slice(),
        origin: this.origin.slice(),
        plane: this.plane,
      },
      material: this.material,
      animation: { mode: "none" },
      metadata: this.metadata,
    }).toDescriptor();
  }
}

export function createDenseFieldView(options) {
  return new DenseFieldView(options);
}

