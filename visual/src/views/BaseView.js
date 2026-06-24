import { VIEW_DESCRIPTOR_SCHEMA, makeId } from "./view-utils.js";

export class BaseView {
  constructor(options = {}) {
    this.schema = VIEW_DESCRIPTOR_SCHEMA;
    this.id = options.id || makeId(options.kind || "view");
    this.kind = options.kind || "view";
    this.name = options.name || this.id;
    this.datasetId = options.datasetId ?? options.dataset_id;
    this.spaceId = options.spaceId ?? options.space_id;
    this.coordinateId = options.coordinateId ?? options.coordinate_id;
    this.propertyId = options.propertyId ?? options.property_id;
    this.relationId = options.relationId ?? options.relation_id;
    this.records = options.records || [];
    this.metadata = options.metadata || {};
  }

  toViewDescriptor() {
    return {
      schema: this.schema,
      id: this.id,
      kind: this.kind,
      name: this.name,
      datasetId: this.datasetId,
      spaceId: this.spaceId,
      coordinateId: this.coordinateId,
      propertyId: this.propertyId,
      relationId: this.relationId,
      metadata: { ...this.metadata },
    };
  }

  toLayerDescriptors() {
    return [];
  }
}

