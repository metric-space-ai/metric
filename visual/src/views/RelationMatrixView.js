import { BaseView } from "./BaseView.js";
import {
  createRelationMatrixLayerDescriptor,
  createSelectedRecordFocusDescriptor,
} from "../relational/descriptors.js";
import { resolveCollectionItem } from "./view-utils.js";

/**
 * RelationMatrixView renders an exported pair relation as a GPU matrix texture.
 * It reads relation values that METRIC already computed; it never derives new
 * pair values. The same relation source can feed this view, a graph view and a
 * neighborhood view simultaneously.
 */
export class RelationMatrixView extends BaseView {
  constructor(options = {}) {
    super({ ...options, kind: "relation-matrix" });
    this.relation = options.relation || null;
    this.recordIds = options.recordIds
      || this.relation?.record_ids
      || this.relation?.recordIds
      || [];
    this.relationType = this.relation?.relation_type || options.relationType || "relation";
    this.symmetric = options.symmetric ?? inferSymmetry(this.relation);
    this.scale = options.scale
      ?? this.relation?.metadata?.visual_scale
      ?? this.relation?.metadata?.visualScale
      ?? "quantile";
    this.palette = options.palette || "metric";
    this.rect = options.rect || [0, 0, 1, 1];
    this.recordOrder = options.recordOrder
      ?? options.record_order
      ?? this.relation?.metadata?.record_order
      ?? this.relation?.metadata?.recordOrder
      ?? null;
    this.blockRanges = options.blockRanges
      ?? options.block_ranges
      ?? this.relation?.metadata?.block_ranges
      ?? this.relation?.metadata?.blockRanges
      ?? null;
    // Material translucency (0-1). Kept distinct from the per-texel alpha byte,
    // which createRelationMatrixLayerDescriptor otherwise conflates with this.
    this.materialAlpha = Number.isFinite(Number(options.materialAlpha ?? options.alpha))
      ? Number(options.materialAlpha ?? options.alpha)
      : 1;
    // Per-texel alpha byte (0-255) for cells without relation evidence. A small
    // value renders the empty grid faintly so the matrix reads as a matrix.
    this.missingAlpha = Number.isFinite(Number(options.missingAlpha)) ? Number(options.missingAlpha) : 0;
    this.valueKey = options.valueKey || ["value", "distance", "weight", "score"];
    this.readabilityCellPixels = finiteOption(options.readabilityCellPixels ?? options.smoothingCellPixels, 4.25);
    this.smoothingStrength = finiteOption(options.smoothingStrength ?? options.valueSmoothing, 0.72);
    this.selectionAlpha = finiteOption(options.selectionAlpha, 0.54);
    this.selectionCellAlpha = finiteOption(options.selectionCellAlpha, 0.9);
    this.selectionOutlineAlpha = finiteOption(options.selectionOutlineAlpha, 1);
    this.selectionOutlinePixels = finiteOption(options.selectionOutlinePixels, 2.2);
    this.focusBackdropAlpha = finiteOption(options.focusBackdropAlpha, 0.38);
    this.focusBlockAlpha = finiteOption(options.focusBlockAlpha, 0.14);
    this.blockBandAlpha = finiteOption(options.blockBandAlpha, 0.11);
    this.blockLineAlpha = finiteOption(options.blockLineAlpha, 0.64);
    this.blockLineWidthCells = finiteOption(options.blockLineWidthCells, 1.15);
    this.tileBoundaryAlpha = finiteOption(options.tileBoundaryAlpha, 0.2);
    this.tileBoundaryWidthCells = finiteOption(options.tileBoundaryWidthCells, 0.5);
    this.outerBorderAlpha = finiteOption(options.outerBorderAlpha, 0.7);
  }

  static fromVisualSpace(document, options = {}) {
    const relationRef = options.relation || options.relationId || options.relation_id;
    const relation = resolveCollectionItem(document, "relations", relationRef, {
      required: relationRef != null,
      label: "relation",
    })
      || firstRelation(document, options);
    const spaceRef = options.space || options.spaceId;
    const space = resolveCollectionItem(document, "spaces", spaceRef, {
      required: spaceRef != null,
      label: "space",
    });
    return new RelationMatrixView({
      ...options,
      relation,
      relationId: relation?.id,
      datasetId: options.datasetId ?? relation?.dataset_id ?? space?.dataset_id,
      spaceId: options.spaceId ?? space?.id,
      recordIds: options.recordIds || relation?.record_ids || space?.record_ids,
    });
  }

  relationSource() {
    if (!this.relation) return { record_ids: this.recordIds, values: [] };
    return this.relation;
  }

  matrixOptions() {
    return {
      id: `${this.id}:matrix`,
      recordIds: this.recordIds,
      symmetric: this.symmetric ? "mirror" : "directed",
      scale: this.scale,
      palette: this.palette,
      rect: this.rect,
      // Present cells stay opaque (255); translucency is applied via material.
      alpha: 255,
      missingAlpha: this.missingAlpha,
      valueKey: this.valueKey,
      order: this.metadata.order ?? 0,
      recordOrder: this.recordOrder,
      blockRanges: this.blockRanges,
      relationId: this.relationId,
      relationName: this.relation?.name || null,
      materialAlpha: this.materialAlpha,
      readabilityCellPixels: this.readabilityCellPixels,
      smoothingStrength: this.smoothingStrength,
      selectionAlpha: this.selectionAlpha,
      selectionCellAlpha: this.selectionCellAlpha,
      selectionOutlineAlpha: this.selectionOutlineAlpha,
      selectionOutlinePixels: this.selectionOutlinePixels,
      focusBackdropAlpha: this.focusBackdropAlpha,
      focusBlockAlpha: this.focusBlockAlpha,
      blockBandAlpha: this.blockBandAlpha,
      blockLineAlpha: this.blockLineAlpha,
      blockLineWidthCells: this.blockLineWidthCells,
      tileBoundaryAlpha: this.tileBoundaryAlpha,
      tileBoundaryWidthCells: this.tileBoundaryWidthCells,
      outerBorderAlpha: this.outerBorderAlpha,
    };
  }

  toLayerDescriptors() {
    const descriptor = createRelationMatrixLayerDescriptor(this.relationSource(), this.matrixOptions());
    descriptor.source = { ...descriptor.source, viewId: this.id, viewKind: this.kind, relationId: this.relationId };
    descriptor.material = {
      ...descriptor.material,
      alpha: this.materialAlpha,
      focusBackdropAlpha: this.focusBackdropAlpha,
      focusBlockAlpha: this.focusBlockAlpha,
    };
    descriptor.metadata = {
      ...descriptor.metadata,
      relationType: this.relationType,
      relationId: this.relationId,
      relationName: this.relation?.name || null,
      focusModel: {
        source: "relation-matrix-view",
        selectedFeatures: ["row", "column", "cell"],
        backdrop: "dim-unfocused-cells",
        blockContext: "selected-row-column-block-bands",
        linkedGraph: "selection-presentation-graph-edge-emphasis",
        pairPreview: "runtime-selected-pair-preview",
      },
    };
    descriptor.metadata.selectionModel = {
      ...(descriptor.metadata.selectionModel || {}),
      focusPresentation: descriptor.metadata.focusModel,
    };
    return [descriptor];
  }

  /**
   * Build a focus descriptor for the selected record (row/column + neighbors).
   * Renderers use this to highlight a row/column and the matching graph edges.
   */
  focusDescriptor(selectedRecord, options = {}) {
    return createSelectedRecordFocusDescriptor(this.relationSource(), selectedRecord, {
      recordIds: this.recordIds,
      valueKey: this.valueKey,
      ...options,
    });
  }
}

export function createRelationMatrixView(options) {
  return new RelationMatrixView(options);
}

function firstRelation(document, options) {
  const relations = Array.isArray(document?.relations) ? document.relations : [];
  if (options.relationType) {
    const typed = relations.find((relation) => relation.relation_type === options.relationType);
    if (typed) return typed;
  }
  return relations[0] || null;
}

function inferSymmetry(relation) {
  if (!relation) return false;
  if (relation.metadata && typeof relation.metadata.symmetric === "boolean") return relation.metadata.symmetric;
  return relation.storage === "symmetric_dense_matrix";
}

function finiteOption(value, fallback) {
  const number = Number(value);
  return Number.isFinite(number) ? number : fallback;
}
