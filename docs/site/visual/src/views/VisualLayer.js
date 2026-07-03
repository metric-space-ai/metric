import { LAYER_DESCRIPTOR_SCHEMA, makeId } from "./view-utils.js";

export class VisualLayer {
  constructor(descriptor = {}) {
    this.descriptor = {
      schema: LAYER_DESCRIPTOR_SCHEMA,
      id: descriptor.id || makeId("layer"),
      kind: descriptor.kind || descriptor.primitive || "visual-layer",
      primitive: descriptor.primitive || "CustomLayer",
      visible: descriptor.visible !== false,
      order: Number.isFinite(Number(descriptor.order)) ? Number(descriptor.order) : 0,
      source: descriptor.source || {},
      channels: descriptor.channels || {},
      geometry: descriptor.geometry || {},
      material: descriptor.material || {},
      animation: descriptor.animation || { mode: "none" },
      picking: descriptor.picking || {},
      bounds: descriptor.bounds || undefined,
      metadata: descriptor.metadata || {},
    };
    this.validate();
  }

  validate() {
    if (!this.descriptor.id) throw new Error("VisualLayer descriptor requires an id");
    if (!this.descriptor.primitive) throw new Error("VisualLayer descriptor requires a primitive");
    if (!this.descriptor.channels || typeof this.descriptor.channels !== "object") {
      throw new Error("VisualLayer descriptor requires a channels object");
    }
  }

  toDescriptor() {
    return {
      ...this.descriptor,
      channels: { ...this.descriptor.channels },
      geometry: { ...this.descriptor.geometry },
      material: { ...this.descriptor.material },
      animation: { ...this.descriptor.animation },
      picking: { ...this.descriptor.picking },
      metadata: { ...this.descriptor.metadata },
    };
  }

  withChannels(channels) {
    return new VisualLayer({
      ...this.descriptor,
      channels: {
        ...this.descriptor.channels,
        ...channels,
      },
    });
  }

  static describe(descriptor) {
    return new VisualLayer(descriptor).toDescriptor();
  }
}

export function createVisualLayer(descriptor) {
  return new VisualLayer(descriptor);
}

