export class DisposableSet {
  constructor() {
    this.items = new Set();
    this.resizableItems = new Set();
    this.disposed = false;
  }

  add(resource, options = {}) {
    if (!resource) return resource;
    if (this.disposed) {
      throw new Error("Cannot add resources after disposal.");
    }
    this.items.add(resource);
    if (options.resizable) {
      this.resizableItems.add(resource);
    }
    return resource;
  }

  delete(resource) {
    this.resizableItems.delete(resource);
    return this.items.delete(resource);
  }

  resize(width, height) {
    for (const resource of this.resizableItems) {
      if (typeof resource.setSize === "function") {
        resource.setSize(width, height);
      } else if (typeof resource.resize === "function") {
        resource.resize(width, height);
      }
    }
  }

  dispose() {
    if (this.disposed) return;
    const resources = Array.from(this.items).reverse();
    for (const resource of resources) {
      disposeResource(resource);
    }
    this.items.clear();
    this.resizableItems.clear();
    this.disposed = true;
  }
}

export function disposeResource(resource) {
  if (!resource) return;
  if (typeof resource.dispose === "function") {
    resource.dispose();
  } else if (typeof resource.destroy === "function") {
    resource.destroy();
  }
}
