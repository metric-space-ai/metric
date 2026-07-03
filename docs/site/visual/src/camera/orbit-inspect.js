import { DEG_TO_RAD, clamp, finiteNumber } from "./math.js";
import { orbitInspect } from "./presets.js";

export class OrbitInspectMode {
  constructor(state = orbitInspect(), options = {}) {
    this.state = state;
    this.rotateSpeed = finiteNumber(options.rotateSpeed, 0.008);
    this.panSpeed = finiteNumber(options.panSpeed, 1);
    this.dollySpeed = finiteNumber(options.dollySpeed, 0.0015);
  }

  orbitPixels(deltaX, deltaY) {
    this.state.orbit(
      finiteNumber(deltaX) * this.rotateSpeed,
      finiteNumber(deltaY) * this.rotateSpeed,
    );
    return this.state;
  }

  panPixels(deltaX, deltaY, viewportHeight = this.state.viewport.pixelHeight) {
    const worldUnitsPerPixel = this.state.worldUnitsPerPixel(viewportHeight) * this.panSpeed;
    this.state.panByCameraAxes(
      -finiteNumber(deltaX) * worldUnitsPerPixel,
      finiteNumber(deltaY) * worldUnitsPerPixel,
    );
    return this.state;
  }

  dollyWheel(deltaY) {
    this.state.dolly(Math.exp(finiteNumber(deltaY) * this.dollySpeed));
    return this.state;
  }

  frameSelection(bounds, options = {}) {
    if (!bounds) return this.state;
    const center = bounds.center || [
      (finiteNumber(bounds.min?.[0]) + finiteNumber(bounds.max?.[0])) * 0.5,
      (finiteNumber(bounds.min?.[1]) + finiteNumber(bounds.max?.[1])) * 0.5,
      (finiteNumber(bounds.min?.[2]) + finiteNumber(bounds.max?.[2])) * 0.5,
    ];
    const radius = finiteNumber(
      bounds.radius,
      Math.max(
        Math.abs(finiteNumber(bounds.max?.[0]) - finiteNumber(bounds.min?.[0])),
        Math.abs(finiteNumber(bounds.max?.[1]) - finiteNumber(bounds.min?.[1])),
        Math.abs(finiteNumber(bounds.max?.[2]) - finiteNumber(bounds.min?.[2])),
        1,
      ) * 0.5,
    );
    const fitPadding = finiteNumber(options.padding, 1.35);
    const fov = Math.max(1e-6, finiteNumber(this.state.fov, 50) * DEG_TO_RAD);
    const distance = clamp((radius * fitPadding) / Math.sin(fov * 0.5), this.state.minDistance, this.state.maxDistance);
    this.state.setTarget(center, { preserveOffset: false });
    this.state.setOrbit({ radius: distance });
    return this.state;
  }

  applyTo(camera) {
    this.state.applyTo(camera);
    return camera;
  }
}

export function createOrbitInspectMode(state, options = {}) {
  return new OrbitInspectMode(state || orbitInspect(options), options);
}
