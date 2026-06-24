import { computeBoundsFromChannel } from "./channels.js";
import { disposeGLResources, resolveRenderContext } from "./gl-utils.js";

export class BaseLayer {
  constructor(descriptor = {}, rendererOrGl = null, options = {}) {
    this.renderer = null;
    this.gl = null;
    this.capabilities = null;
    this.options = options || {};
    this.resources = [];
    this.scene = null;
    this.enabled = true;
    this.visible = true;
    this.renderOrder = 0;
    this.disposed = false;
    this.initialized = false;
    this.needsUpload = true;
    this.lastError = null;

    this.setDescriptor(descriptor);
    if (rendererOrGl) {
      this.setRenderer(rendererOrGl, options);
    }
  }

  setRenderer(rendererOrGl, options = this.options) {
    const context = resolveRenderContext(rendererOrGl, options);
    this.renderer = context.renderer;
    this.gl = context.gl;
    this.capabilities = context.capabilities;
    this.options = { ...this.options, ...options };
    this.initialized = false;
    this.needsUpload = true;
    return this;
  }

  setDescriptor(descriptor = {}) {
    const previousId = this.id;
    this.descriptor = normalizeLayerDescriptor(descriptor);
    this.id = this.descriptor.id;
    this.kind = this.descriptor.kind;
    this.primitive = this.descriptor.primitive;
    this.visible = this.descriptor.visible !== false;
    this.order = this.descriptor.order;
    this.renderOrder = this.order;
    this.source = this.descriptor.source;
    this.channels = this.descriptor.channels;
    this.material = this.descriptor.material;
    this.geometry = this.descriptor.geometry;
    this.animation = this.descriptor.animation;
    this.picking = this.descriptor.picking;
    this.metadata = this.descriptor.metadata;
    this.bounds = this.descriptor.bounds;
    this.needsUpload = true;

    if (this.scene && previousId !== undefined) {
      this.scene.needsSort = true;
    }
    return this;
  }

  update() {
    return this;
  }

  render() {
    return this;
  }

  getBounds() {
    if (this.bounds) return this.bounds;
    return computeBoundsFromChannel(this.channels.position || this.channels.sourcePosition);
  }

  markNeedsUpload() {
    this.needsUpload = true;
    return this;
  }

  setVisible(visible) {
    this.visible = visible !== false;
    return this;
  }

  setOrder(order) {
    const next = Number.isFinite(Number(order)) ? Number(order) : 0;
    this.order = next;
    this.renderOrder = next;
    if (this.scene) this.scene.needsSort = true;
    return this;
  }

  track(resource) {
    if (resource) this.resources.push(resource);
    return resource;
  }

  clearResources() {
    disposeGLResources(this.gl, this.resources);
    this.resources.length = 0;
    this.initialized = false;
    return this;
  }

  dispose() {
    if (this.disposed) return;
    this.clearResources();
    this.disposed = true;
    this.enabled = false;
  }
}

export function normalizeLayerDescriptor(descriptor = {}) {
  const primitive = descriptor.primitive || descriptor.kind || "CustomLayer";
  return {
    id: descriptor.id || `${primitive}-${Math.random().toString(36).slice(2, 9)}`,
    kind: descriptor.kind || primitive,
    primitive,
    visible: descriptor.visible !== false,
    order: Number.isFinite(Number(descriptor.order)) ? Number(descriptor.order) : 0,
    source: descriptor.source || {},
    labels: descriptor.labels || descriptor.annotations || [],
    channels: descriptor.channels || {},
    geometry: descriptor.geometry || {},
    material: descriptor.material || {},
    animation: descriptor.animation || { mode: "none" },
    picking: descriptor.picking || {},
    bounds: descriptor.bounds,
    metadata: descriptor.metadata || {},
  };
}
