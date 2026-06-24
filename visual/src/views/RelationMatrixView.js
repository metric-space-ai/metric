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
    this.scale = options.scale || "minMax";
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
    };
  }

  toLayerDescriptors() {
    const descriptor = createRelationMatrixLayerDescriptor(this.relationSource(), this.matrixOptions());
    descriptor.source = { ...descriptor.source, viewId: this.id, viewKind: this.kind, relationId: this.relationId };
    descriptor.material = { ...descriptor.material, alpha: this.materialAlpha };
    descriptor.metadata = {
      ...descriptor.metadata,
      relationType: this.relationType,
      relationId: this.relationId,
      relationName: this.relation?.name || null,
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
