import { disposeResource } from "./resources.js";

export class VisualScene {
  constructor(options = {}) {
    this.children = [];
    this.backgroundColor = new Float32Array(options.backgroundColor || [0.02, 0.025, 0.03, 1]);
    this.userData = options.userData || {};
    this.needsSort = false;
    this.disposed = false;
  }

  add(child) {
    if (!child || this.children.includes(child)) return child;
    this.children.push(child);
    child.scene = this;
    this.needsSort = true;
    return child;
  }

  remove(child) {
    const index = this.children.indexOf(child);
    if (index === -1) return child;
    this.children.splice(index, 1);
    if (child.scene === this) {
      child.scene = null;
    }
    return child;
  }

  clear({ dispose = false } = {}) {
    const children = this.children.slice();
    this.children.length = 0;
    for (const child of children) {
      if (child.scene === this) child.scene = null;
      if (dispose) disposeResource(child);
    }
    return this;
  }

  setBackground(color) {
    this.backgroundColor.set(color);
    return this;
  }

  update(context) {
    if (this.needsSort) {
      this.sort();
    }
    for (const child of this.children) {
      if (child.enabled === false) continue;
      if (typeof child.update === "function") {
        child.update(context);
      }
    }
    return this;
  }

  render(context) {
    if (this.needsSort) {
      this.sort();
    }
    for (const child of this.children) {
      if (child.enabled === false || child.visible === false) continue;
      if (typeof child.render === "function") {
        child.render(context);
      }
    }
    return this;
  }

  sort() {
    this.children.sort((a, b) => (a.renderOrder || 0) - (b.renderOrder || 0));
    this.needsSort = false;
    return this;
  }

  dispose() {
    if (this.disposed) return;
    this.clear({ dispose: true });
    this.disposed = true;
  }
}
