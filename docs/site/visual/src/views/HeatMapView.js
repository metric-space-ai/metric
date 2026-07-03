import { BaseView } from "./BaseView.js";
import { VisualLayer } from "./VisualLayer.js";
import {
  colorChannelFrom,
  createChannel,
  createStringChannel,
  descriptorSource,
  extractRelationCells,
  inferScalarDomain,
  resolveCollectionItem,
} from "./view-utils.js";

export class HeatMapView extends BaseView {
  constructor(options = {}) {
    super({ ...options, kind: "heat-map" });
    const relationCells = options.cells
      ? {
          rowIds: options.rowIds || [],
          columnIds: options.columnIds || [],
          cells: options.cells,
        }
      : extractRelationCells(options.relation || options.matrix || options.values, options);
    this.rowIds = (options.rowIds || relationCells.rowIds).map(String);
    this.columnIds = (options.columnIds || relationCells.columnIds).map(String);
    this.cells = relationCells.cells;
    this.cellSize = options.cellSize || [1, 1];
    this.origin = options.origin || [0, 0, 0];
    this.heightScale = Number.isFinite(Number(options.heightScale)) ? Number(options.heightScale) : 1;
    this.minimumHeight = Number.isFinite(Number(options.minimumHeight)) ? Number(options.minimumHeight) : 0;
    this.alpha = Number.isFinite(Number(options.alpha)) ? Number(options.alpha) : 1;
    this.renderStyle = options.renderStyle || "bars";
    this.material = {
      lighting: "data-preserving",
      diffuse: "scalar-ramp",
      ambient: 0.54,
      pointLight: 0.46,
      alphaMode: this.alpha < 1 ? "blend" : "opaque",
      ...(options.material || {}),
    };
  }

  static fromVisualSpace(document, options = {}) {
    const relation = resolveCollectionItem(document, "relations", options.relation || options.relationId || options.relation_id);
    return new HeatMapView({
      ...options,
      relation,
      relationId: relation?.id,
      datasetId: options.datasetId ?? relation?.dataset_id ?? relation?.datasetId,
    });
  }

  toLayerDescriptors() {
    const count = this.cells.length;
    const ids = new Array(count);
    const positions = new Float32Array(count * 3);
    const sizes = new Float32Array(count * 2);
    const heights = new Float32Array(count);
    const scalars = new Float32Array(count);
    const alpha = new Float32Array(count).fill(this.alpha);

    for (let index = 0; index < count; index += 1) {
      const cell = this.cells[index];
      const value = Number(cell.value) || 0;
      const height = Math.max(this.minimumHeight, value * this.heightScale);
      ids[index] = `${cell.rowId}:${cell.columnId}`;
      positions[index * 3] = this.origin[0] + (cell.column + 0.5) * this.cellSize[0];
      positions[index * 3 + 1] = this.origin[1] + height * 0.5;
      positions[index * 3 + 2] = this.origin[2] + (cell.row + 0.5) * this.cellSize[1];
      sizes[index * 2] = this.cellSize[0];
      sizes[index * 2 + 1] = this.cellSize[1];
      heights[index] = height;
      scalars[index] = value;
    }

    return [new VisualLayer({
      id: `${this.id}:heatmap`,
      kind: "heat-map",
      primitive: this.renderStyle === "field" ? "HeatFieldLayer" : "InstancedBoxLayer",
      source: descriptorSource(this),
      channels: {
        cellId: createStringChannel(ids, "cell-id"),
        position: createChannel(positions, 3, "cell-center"),
        size: createChannel(sizes, 2, "cell-size", { units: "scene" }),
        height: createChannel(heights, 1, "height", { units: "scene" }),
        scalar: createChannel(scalars, 1, "scalar", { domain: inferScalarDomain(Array.from(scalars)) }),
        color: colorChannelFrom({ ids, scalarValues: scalars, alpha: this.alpha }),
        alpha: createChannel(alpha, 1, "alpha"),
      },
      geometry: {
        mode: this.renderStyle,
        rows: this.rowIds.length,
        columns: this.columnIds.length,
        rowIds: this.rowIds.slice(),
        columnIds: this.columnIds.slice(),
        cellSize: this.cellSize.slice(),
        origin: this.origin.slice(),
        axes: { row: "z", column: "x", value: "y" },
        zeroValue: "flat-cell",
      },
      material: this.material,
      animation: { mode: "none" },
      picking: {
        mode: "cell-id",
        channel: "cellId",
      },
      metadata: {
        ...this.metadata,
        cellCount: count,
      },
    }).toDescriptor()];
  }
}

export function createHeatMapView(options) {
  return new HeatMapView(options);
}

