import { InstancedPointLayer } from "./InstancedPointLayer.js";

export class InstancedGlyphLayer extends InstancedPointLayer {
  constructor(descriptor = {}, rendererOrGl = null, options = {}) {
    super({
      ...descriptor,
      geometry: {
        shape: "disc",
        ...(descriptor.geometry || {}),
      },
    }, rendererOrGl, {
      pointPixelScale: 10,
      ...options,
    });
  }
}

